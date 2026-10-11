// Sketch v3: collapse OpenStudio's 5-level DefaultConstructionSet inheritance into the 2 levels E+ ConstructionAssignmentSet
// supports (Space, Building).
//
// Compared to the other sketches:
//   * v1 (ConvertDCSToEnergyPlus.cpp):  chain = every level that has a DCS, merged sets keyed by DCS handles.
//   * v2 (ConvertDCSToEnergyPlus2.cpp): chain = levels that win at least one *existing* defaulted surface (geometry-dependent).
//   * v3 (this file):                   chain = levels that fill at least one *slot* not already filled by a higher level
//                                       (geometry-independent), merged sets de-duplicated by resolved slot content.
//
// Why slots rather than surfaces: the emitted sets keep OpenStudio's meaning for ANY surface, including one added to the IDF
// afterwards. The result also doesn't change shape when a window is added to the model.
//
// OS inheritance order (cf Space_Impl::getDefaultConstructionWithSearchDistance):
//
//   Space -> SpaceType (explicit only) -> BuildingStory -> Building -> Building SpaceType
//   \__________________ ___________________________/     \_______________ _____________/
//                      V                                                 V
//                 Space Group                                     Building Group
//
// Meant to run on the FT's model copy, in translateModelPrivate:
//   AFTER  resolveMatched(Sub)SurfaceConstructionConflicts  (they need the original 1..5 search distances)
//   BEFORE the "remove unused space types" loop             (it can delete the Building's SpaceType, and its DCS with it)
//
// Algorithm, per group (Building once, then each Space):
//   1. Collect the levels that have a DCS, in priority order, skipping repeated handles.
//   2. Prune: walk the chain keeping a mask of filled slots. Drop a DCS that fills no slot outside the mask.
//   3. 0 left -> reset. 1 left -> reuse that DCS as is.
//      2+ left -> compute the merged slot content (first non-empty wins, same as merge()) WITHOUT creating anything, and look it
//      up among existing originals and merged sets created so far. Only clone + merge on a miss.
//   4. Reset all SpaceType and BuildingStory DCSs: the OS lookup is now Space -> Building, same as E+.
//
// Invariants:
//   * Originals are never mutated.
//   * Every PlanarSurface (existing or not) resolves to the same construction after as before: a pruned DCS only fills slots a
//     higher one already fills, and "first level with the slot wins" is preserved by the merge order.
//   * Each group only reads its own levels' DCSs (Space group never reads the Building's), and SpaceType / Story DCSs are only
//     reset at the end, so assigning as we go is safe.
//
// Not handled here:
//   * ShadingSurfaces: E+ has no shading slot, their constructions must be written explicitly. Shading slots are still counted
//     in the masks so the OS model stays consistent. If shading constructions get hard-assigned BEFORE this runs, set
//     countShadingSlots = false to prune more.
//   * Boundary conditions where the OS and E+ slot differ (GroundFCfactorMethod, Foundation, OtherSideCoefficients, ...):
//     handled by writing the construction explicitly on those surfaces.

#include "Building.hpp"
#include "BuildingStory.hpp"
#include "BuildingStory_Impl.hpp"
#include "ConstructionBase.hpp"
#include "DefaultConstructionSet.hpp"
#include "DefaultConstructionSet_Impl.hpp"
#include "DefaultSubSurfaceConstructions.hpp"
#include "DefaultSurfaceConstructions.hpp"
#include "Model.hpp"
#include "Space.hpp"
#include "Space_Impl.hpp"
#include "SpaceType.hpp"
#include "SpaceType_Impl.hpp"

#include "../utilities/core/Logger.hpp"

#include <boost/optional.hpp>

#include <array>
#include <bitset>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace openstudio {

namespace model {

  namespace {

    constexpr auto logChannel = "openstudio.model.ConvertDCSToEnergyPlus";

    // 3 surface categories x 3 types, 2 sub-surface categories x 8 types, then interior partition, 3 shading, adiabatic
    constexpr size_t nSlots = (3 * 3) + (2 * 8) + 5;
    constexpr size_t firstShadingSlot = (3 * 3) + (2 * 8) + 1;  // space, building, site shading
    constexpr size_t nShadingSlots = 3;
    constexpr bool countShadingSlots = true;

    using SlotConstructions = std::array<boost::optional<ConstructionBase>, nSlots>;
    using SlotMask = std::bitset<nSlots>;
    using ContentKey = std::array<Handle, nSlots>;  // null Handle for an empty slot

    SlotConstructions slotConstructions(const DefaultConstructionSet& dcs) {
      SlotConstructions result;
      size_t i = 0;

      for (const auto& d_ :
           {dcs.defaultExteriorSurfaceConstructions(), dcs.defaultInteriorSurfaceConstructions(), dcs.defaultGroundContactSurfaceConstructions()}) {
        if (d_) {
          result[i] = d_->floorConstruction();
          result[i + 1] = d_->wallConstruction();
          result[i + 2] = d_->roofCeilingConstruction();
        }
        i += 3;
      }

      for (const auto& d_ : {dcs.defaultExteriorSubSurfaceConstructions(), dcs.defaultInteriorSubSurfaceConstructions()}) {
        if (d_) {
          result[i] = d_->fixedWindowConstruction();
          result[i + 1] = d_->operableWindowConstruction();
          result[i + 2] = d_->doorConstruction();
          result[i + 3] = d_->glassDoorConstruction();
          result[i + 4] = d_->overheadDoorConstruction();
          result[i + 5] = d_->skylightConstruction();
          result[i + 6] = d_->tubularDaylightDomeConstruction();
          result[i + 7] = d_->tubularDaylightDiffuserConstruction();
        }
        i += 8;
      }

      result[i++] = dcs.interiorPartitionConstruction();
      result[i++] = dcs.spaceShadingConstruction();
      result[i++] = dcs.buildingShadingConstruction();
      result[i++] = dcs.siteShadingConstruction();
      result[i++] = dcs.adiabaticSurfaceConstruction();

      return result;
    }

    SlotMask filledSlots(const SlotConstructions& slots) {
      SlotMask mask;
      for (size_t i = 0; i < nSlots; ++i) {
        mask[i] = slots[i].has_value();
      }
      if constexpr (!countShadingSlots) {
        for (size_t i = firstShadingSlot; i < firstShadingSlot + nShadingSlots; ++i) {
          mask[i] = false;
        }
      }
      return mask;
    }

    ContentKey contentKey(const SlotConstructions& slots) {
      ContentKey key{};
      for (size_t i = 0; i < nSlots; ++i) {
        if (slots[i]) {
          key[i] = slots[i]->handle();
        }
      }
      return key;
    }

    struct ChainItem
    {
      DefaultConstructionSet dcs;
      std::string owner;  // e.g. "SpaceType 'Office'", for logging
      SlotConstructions slots;
    };

    class Resolver
    {
     public:
      explicit Resolver(Model& model) : m_model(model) {
        // Register originals by content, so a merge that happens to equal an existing DCS reuses it
        // TODO: I don't think I want this
        // Example 1: matching an unrelated original set

        // The model has these sets:
        // - Lab Set on Space 104: exterior wall = Wall-A
        // - L1 Set on Story L1: ground floor = Slab-150
        // - Retail Set on SpaceType Retail: exterior wall = Wall-A, ground floor = Slab-150

        // Space 104 has no Retail SpaceType. Its chain is Lab Set, then L1 Set, and merging them gives exterior wall = Wall-A and ground floor = Slab-150.

        // The Resolver constructor registers every original set by its contents. So resolve finds that Retail Set holds exactly the same constructions and returns it rather than creating Lab
        // Set + L1 Set. In the IDF, Space 104 then points at ConstructionAssignmentSet Retail Set, even though nothing Retail is involved. Every surface still gets the right construction;
        // only the name misleads
        // for (const auto& dcs : model.getConcreteModelObjects<DefaultConstructionSet>()) {
        //   m_byContent.emplace(contentKey(slotConstructions(dcs)), dcs);
        // }
      }

      void push(std::vector<ChainItem>& chain, const boost::optional<DefaultConstructionSet>& dcs_, std::string owner) const {
        if (!dcs_) {
          return;
        }
        for (const auto& item : chain) {
          if (item.dcs.handle() == dcs_->handle()) {
            return;  // same DCS higher up already provides everything this one would
          }
        }
        chain.push_back({*dcs_, std::move(owner), slotConstructions(*dcs_)});
      }

      boost::optional<DefaultConstructionSet> resolve(const std::string& groupName, std::vector<ChainItem> chain) {
        prune(groupName, chain);

        if (chain.empty()) {
          return boost::none;
        }
        if (chain.size() == 1) {
          return chain.front().dcs;
        }

        // What merge() would produce: first non-empty slot wins
        SlotConstructions mergedSlots = chain.front().slots;
        for (auto it = std::next(chain.cbegin()); it != chain.cend(); ++it) {
          for (size_t i = 0; i < nSlots; ++i) {
            if (!mergedSlots[i] && it->slots[i]) {
              mergedSlots[i] = it->slots[i];
            }
          }
        }
        auto key = contentKey(mergedSlots);
        if (auto it = m_byContent.find(key); it != m_byContent.end()) {
          LOG_FREE(Trace, logChannel, groupName << ": reusing DefaultConstructionSet '" << it->second.nameString() << "' (same content)");
          return it->second;
        }

        // Miss: create it. The clone still points at the original Default(Sub)SurfaceConstructions; merge() only clones one of
        // those when it has to fill it from another partial one, so sub-object cloning is already minimal.
        auto merged = chain.front().dcs.clone(m_model).cast<DefaultConstructionSet>();
        std::string name = chain.front().dcs.nameString();
        for (auto it = std::next(chain.cbegin()); it != chain.cend(); ++it) {
          merged.merge(it->dcs);
          name += " + " + it->dcs.nameString();
        }
        merged.setName(name);

        std::string owners = chain.front().owner;
        for (auto it = std::next(chain.cbegin()); it != chain.cend(); ++it) {
          owners += ", " + it->owner;
        }
        LOG_FREE(Info, logChannel, groupName << ": created merged DefaultConstructionSet '" << merged.nameString() << "' from " << owners);

        m_byContent.emplace(std::move(key), merged);
        return merged;
      }

     private:
      static void prune(const std::string& groupName, std::vector<ChainItem>& chain) {
        SlotMask covered;
        std::vector<ChainItem> kept;
        kept.reserve(chain.size());
        for (auto& item : chain) {
          const SlotMask filled = filledSlots(item.slots);
          if ((filled & ~covered).none()) {
            LOG_FREE(Trace, logChannel,
                     groupName << ": ignoring " << item.owner << " DefaultConstructionSet '" << item.dcs.nameString()
                               << "', every slot it fills is already filled higher up");
            continue;
          }
          covered |= filled;
          kept.push_back(std::move(item));
        }
        chain = std::move(kept);
      }

      Model& m_model;
      std::map<ContentKey, DefaultConstructionSet> m_byContent;
    };

  }  // namespace

  void resolveDefaultConstructionSets(Model& model) {

    Resolver resolver(model);

    // Building group. model.building(), not getUniqueModelObject, so we don't instantiate a Building
    if (auto building_ = model.building()) {
      std::vector<ChainItem> chain;
      resolver.push(chain, building_->defaultConstructionSet(), "Building");
      if (auto st_ = building_->spaceType()) {
        resolver.push(chain, st_->defaultConstructionSet(), "Building SpaceType '" + st_->nameString() + "'");
      }
      if (auto dcs_ = resolver.resolve("Building", std::move(chain))) {
        building_->setDefaultConstructionSet(*dcs_);
      } else {
        building_->resetDefaultConstructionSet();
      }
    }

    // Space group, per Space
    for (auto& space : model.getConcreteModelObjects<Space>()) {
      const std::string groupName = "Space '" + space.nameString() + "'";
      std::vector<ChainItem> chain;
      resolver.push(chain, space.defaultConstructionSet(), "Space");
      if (!space.isSpaceTypeDefaulted()) {  // defaulted == Building's SpaceType, which is in the Building group
        if (auto st_ = space.spaceType()) {
          resolver.push(chain, st_->defaultConstructionSet(), "SpaceType '" + st_->nameString() + "'");
        }
      }
      if (auto story_ = space.buildingStory()) {
        resolver.push(chain, story_->defaultConstructionSet(), "BuildingStory '" + story_->nameString() + "'");
      }

      if (auto dcs_ = resolver.resolve(groupName, std::move(chain))) {
        space.setDefaultConstructionSet(*dcs_);  // no-op when it's the Space's own
      } else {
        space.resetDefaultConstructionSet();  // no DCS in the group, or only empty ones
      }
    }

    // Drop the intermediate levels so the OS lookup matches E+'s (includes the Building's SpaceType)
    for (auto& spaceType : model.getConcreteModelObjects<SpaceType>()) {
      spaceType.resetDefaultConstructionSet();
    }
    for (auto& story : model.getConcreteModelObjects<BuildingStory>()) {
      story.resetDefaultConstructionSet();
    }

    // Original DCSs that are no longer referenced are simply not translated (translate a DCS only when a Space or the Building
    // points at it).
  }

  // Test ideas:
  //   * Snapshot construction() of every PlanarSurface before, call resolveDefaultConstructionSets, compare after: identical.
  //   * Count DefaultConstructionSets created: a lower DCS that only repeats higher slots must not trigger a clone, and two
  //     Spaces whose chains differ but merge to the same content must share one set.

}  // namespace model

}  // namespace openstudio
