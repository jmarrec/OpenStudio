// Sketch v4: collapse OpenStudio's 5-level DefaultConstructionSet inheritance into the 2 levels E+ ConstructionAssignmentSet
// supports (Space, Building).
//
// Compared to the other sketches:
//   * v1: chain = every level that has a DCS, merged sets keyed by DCS handles.
//   * v2: chain = levels that win at least one *existing* defaulted surface (geometry-dependent).
//   * v3: chain = levels that fill a slot no higher level fills, merged sets de-duplicated by resolved slot content.
//   * v4 (this file): same chain as v3, merged sets keyed by the handles of the objects that OWN the DCSs
//     (Space, SpaceType, BuildingStory / Building, Building SpaceType). One merged set per combination of owners, e.g.
//     "SpaceType Office @ BuildingStory L1", and only if Story L1's DCS fills a slot that Office's doesn't.
//     More sets than v1/v3 when different owners share a DCS, but every name says exactly where the set came from.
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
//   1. Collect the levels that have a DCS, in priority order, with their owner.
//   2. Prune: walk the chain keeping a mask of filled slots. Drop a DCS that fills no slot outside the mask. This also drops a
//      DCS repeated lower down (e.g. a SpaceType and its Story pointing at the same DCS).
//   3. 0 left -> reset. 1 left -> reuse that DCS as is, no clone.
//      2+ left -> look up the kept owners' handles; clone + merge only on a miss.
//   4. Reset all SpaceType and BuildingStory DCSs: the OS lookup is now Space -> Building, same as E+.
//
// Invariants:
//   * Originals are never mutated.
//   * Every PlanarSurface (existing or not) resolves to the same construction after as before: a pruned DCS only fills slots a
//     higher one already fills, and "first level with the slot wins" is preserved by the merge order.
//   * Same owners => same DCSs => same merged content, so sharing by owners is always correct.
//   * Each group only reads its own levels' DCSs (the Space group never reads the Building's), and SpaceType / Story DCSs are
//     only reset at the end, so assigning as we go is safe.
//
// Not handled here:
//   * ShadingSurfaces: E+ has no shading slot, their constructions must be written explicitly. With
//     countShadingSlots = true, shading slots count when pruning, so OS shading surfaces keep resolving the same. Once shading
//     constructions get hard-assigned BEFORE this runs, set it to false to prune more.
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

    // Set to false once shading constructions are hard-assigned before this runs: E+ has no shading slot, so a DCS that only
    // contributes shading constructions shouldn't keep a level in the chain
    constexpr bool countShadingSlots = true;

    // 3 surface categories x 3 types, 2 sub-surface categories x 8 types, interior partition, [space, building, site shading], adiabatic
    constexpr size_t nSlots = (3 * 3) + (2 * 8) + 1 + (countShadingSlots ? 3 : 0) + 1;

    using SlotMask = std::bitset<nSlots>;

    SlotMask filledSlots(const DefaultConstructionSet& dcs) {
      SlotMask mask;
      size_t i = 0;

      for (const auto& d_ :
           {dcs.defaultExteriorSurfaceConstructions(), dcs.defaultInteriorSurfaceConstructions(), dcs.defaultGroundContactSurfaceConstructions()}) {
        if (d_) {
          mask[i] = d_->floorConstruction().has_value();
          mask[i + 1] = d_->wallConstruction().has_value();
          mask[i + 2] = d_->roofCeilingConstruction().has_value();
        }
        i += 3;
      }

      for (const auto& d_ : {dcs.defaultExteriorSubSurfaceConstructions(), dcs.defaultInteriorSubSurfaceConstructions()}) {
        if (d_) {
          mask[i] = d_->fixedWindowConstruction().has_value();
          mask[i + 1] = d_->operableWindowConstruction().has_value();
          mask[i + 2] = d_->doorConstruction().has_value();
          mask[i + 3] = d_->glassDoorConstruction().has_value();
          mask[i + 4] = d_->overheadDoorConstruction().has_value();
          mask[i + 5] = d_->skylightConstruction().has_value();
          mask[i + 6] = d_->tubularDaylightDomeConstruction().has_value();
          mask[i + 7] = d_->tubularDaylightDiffuserConstruction().has_value();
        }
        i += 8;
      }

      mask[i++] = dcs.interiorPartitionConstruction().has_value();
      if constexpr (countShadingSlots) {
        mask[i++] = dcs.spaceShadingConstruction().has_value();
        mask[i++] = dcs.buildingShadingConstruction().has_value();
        mask[i++] = dcs.siteShadingConstruction().has_value();
      }
      mask[i++] = dcs.adiabaticSurfaceConstruction().has_value();

      return mask;
    }

    struct ChainItem
    {
      DefaultConstructionSet dcs;
      Handle ownerHandle;  // the Space, SpaceType, BuildingStory, or Building that points at dcs
      std::string owner;   // e.g. "SpaceType Office", used for the merged set name and logging
    };

    void push(std::vector<ChainItem>& chain, const boost::optional<DefaultConstructionSet>& dcs_, const ModelObject& owner,
              const std::string& ownerKind) {
      if (dcs_) {
        chain.push_back({*dcs_, owner.handle(), ownerKind + " " + owner.nameString()});
      }
    }

    class Resolver
    {
     public:
      explicit Resolver(const Model& model) : m_model(model) {}

      boost::optional<DefaultConstructionSet> resolve(const std::string& groupName, std::vector<ChainItem> chain) {
        prune(groupName, chain);

        if (chain.empty()) {
          return boost::none;
        }
        if (chain.size() == 1) {
          return chain.front().dcs;
        }

        std::vector<Handle> key;
        key.reserve(chain.size());
        for (const auto& item : chain) {
          key.push_back(item.ownerHandle);
        }
        if (auto it = m_byOwners.find(key); it != m_byOwners.end()) {
          return it->second;
        }

        // Miss: create it. The clone still points at the original Default(Sub)SurfaceConstructions; merge() only clones one of
        // those when it has to fill it from another partial one, so sub-object cloning is already minimal.
        auto merged = chain.front().dcs.clone(m_model).cast<DefaultConstructionSet>();
        std::string name = chain.front().owner;
        for (auto it = std::next(chain.cbegin()); it != chain.cend(); ++it) {
          merged.merge(it->dcs);
          name += " @ " + it->owner;
        }
        merged.setName(name);
        LOG_FREE(Info, logChannel, groupName << ": created merged DefaultConstructionSet '" << merged.nameString() << "'");

        m_byOwners.emplace(std::move(key), merged);
        return merged;
      }

     private:
      static void prune(const std::string& groupName, std::vector<ChainItem>& chain) {
        SlotMask covered;
        std::vector<ChainItem> kept;
        kept.reserve(chain.size());
        for (auto& item : chain) {
          const SlotMask filled = filledSlots(item.dcs);
          if ((filled & ~covered).none()) {
            LOG_FREE(Trace, logChannel,
                     groupName << ": ignoring " << item.owner << "'s DefaultConstructionSet '" << item.dcs.nameString()
                               << "', every slot it fills is already filled higher up");
            continue;
          }
          covered |= filled;
          kept.push_back(std::move(item));
        }
        chain = std::move(kept);
      }

      Model m_model;
      std::map<std::vector<Handle>, DefaultConstructionSet> m_byOwners;
    };

  }  // namespace

  void resolveDefaultConstructionSets(Model& model) {

    Resolver resolver(model);

    // Building group. model.building(), not getUniqueModelObject, so we don't instantiate a Building
    if (auto building_ = model.building()) {
      std::vector<ChainItem> chain;
      push(chain, building_->defaultConstructionSet(), *building_, "Building");
      if (auto st_ = building_->spaceType()) {
        push(chain, st_->defaultConstructionSet(), *st_, "SpaceType");
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
      push(chain, space.defaultConstructionSet(), space, "Space");
      if (!space.isSpaceTypeDefaulted()) {  // defaulted == Building's SpaceType, which is in the Building group
        if (auto st_ = space.spaceType()) {
          push(chain, st_->defaultConstructionSet(), *st_, "SpaceType");
        }
      }
      if (auto story_ = space.buildingStory()) {
        push(chain, story_->defaultConstructionSet(), *story_, "BuildingStory");
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
  //   * Two Spaces with SpaceType Office on Story L1 share one "SpaceType Office @ BuildingStory L1" set.
  //   * A Space with SpaceType OpenOffice on L1, where OpenOffice uses the same DCS as Office, gets its own
  //     "SpaceType OpenOffice @ BuildingStory L1" set.
  //   * If L1's DCS only fills slots Office's already fills, Office's spaces on L1 reuse Office's DCS directly, no clone.

}  // namespace model

}  // namespace openstudio
