// Sketch v2: collapse OpenStudio's 5-level DefaultConstructionSet inheritance into the 2 levels E+ ConstructionAssignmentSet
// supports (Space, Building), only merging levels that actually contribute, and only cloning when 2+ levels meet.
//
// OS inheritance order (cf Space_Impl::getDefaultConstructionWithSearchDistance):
//
//   Space -> SpaceType (explicit only) -> BuildingStory -> Building -> Building SpaceType
//   \__________________ ___________________________/     \_______________ _____________/
//                      V                                                 V
//                 Space Group                                     Building Group
//
// Meant to run on the FT's model copy, in translateModelPrivate:
//   AFTER  resolveMatched(Sub)SurfaceConstructionConflicts  (they need the original 1..5 search distances, and they make
//                                                            matched surfaces explicit so they are not considered here)
//   BEFORE the "remove unused space types" loop             (it can delete the Building's SpaceType, and its DCS with it)
//
// Algorithm:
//   1. Read-only pass. For every defaulted PlanarSurface (Surface, SubSurface, InteriorPartitionSurface) of every Space, record
//      which level wins it. A level "contributes" if it wins at least one surface.
//   2. Building group: chain = contributing levels among [Building, Building SpaceType].
//      Space group, per Space: chain = contributing levels among [Space, SpaceType, BuildingStory].
//      0 levels -> reset, 1 level -> reuse that DCS as is, 2+ levels -> merged clone, cached by chain so Spaces with the same
//      contributing levels share one.
//   3. Reset all SpaceType and BuildingStory DCSs: the OS lookup is now Space -> Building, same as E+.
//
// Invariants:
//   * Originals are never mutated: a SpaceType's or Story's DCS has directUseCount() == 1 but can still serve many Spaces.
//   * Every defaulted PlanarSurface resolves to the same construction after as before. Levels that contribute nothing are
//     fully shadowed (or have no matching slot) for every existing surface, so dropping them changes nothing. Merge order
//     preserves "first level with the slot wins".
//   * Pass 1 is done entirely before any mutation, since changing the Building's DCS would change results for other Spaces.
//
// Not handled here:
//   * ShadingSurfaces: E+ has no shading slot in ConstructionAssignmentSet, their constructions must be written explicitly.
//   * Boundary conditions where the OS and E+ slot differ (GroundFCfactorMethod, Foundation, OtherSideCoefficients, ...):
//     handled by writing the construction explicitly on those surfaces.

#include "Building.hpp"
#include "BuildingStory.hpp"
#include "BuildingStory_Impl.hpp"
#include "ConstructionBase.hpp"
#include "DefaultConstructionSet.hpp"
#include "DefaultConstructionSet_Impl.hpp"
#include "InteriorPartitionSurface.hpp"
#include "InteriorPartitionSurfaceGroup.hpp"
#include "Model.hpp"
#include "PlanarSurface.hpp"
#include "Space.hpp"
#include "Space_Impl.hpp"
#include "SpaceType.hpp"
#include "SpaceType_Impl.hpp"
#include "SubSurface.hpp"
#include "Surface.hpp"

#include "../utilities/core/Logger.hpp"

#include <boost/optional.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace openstudio {

namespace model {

  namespace {

    constexpr auto logChannel = "openstudio.model.ConvertDCSToEnergyPlus";

    // Search distances returned by Space::getDefaultConstructionWithSearchDistance
    enum class ConstructionDistance
    {
      Space = 1,
      SpaceType = 2,
      BuildingStory = 3,
      Building = 4,
      BuildingSpaceType = 5
    };

    std::vector<PlanarSurface> defaultedPlanarSurfaces(const Space& space) {
      std::vector<PlanarSurface> result;
      for (const Surface& surface : space.surfaces()) {
        if (surface.isConstructionDefaulted()) {
          result.push_back(surface);
        }
        for (const SubSurface& subSurface : surface.subSurfaces()) {
          if (subSurface.isConstructionDefaulted()) {
            result.push_back(subSurface);
          }
        }
      }
      for (const InteriorPartitionSurfaceGroup& group : space.interiorPartitionSurfaceGroups()) {
        for (const InteriorPartitionSurface& partition : group.interiorPartitionSurfaces()) {
          if (partition.isConstructionDefaulted()) {
            result.push_back(partition);
          }
        }
      }
      return result;
    }

    struct SpaceContributions
    {
      Space space;
      std::array<bool, 3> contributes{};  // Space, SpaceType, BuildingStory
    };

  }  // namespace

  void resolveDefaultConstructionSets(Model& model) {

    // ---------------------------------------------------------------------------------------------------------------------
    // 1. Read-only pass: who wins what
    // ---------------------------------------------------------------------------------------------------------------------
    std::vector<SpaceContributions> spaceContributions;
    bool buildingContributes = false;
    bool buildingSpaceTypeContributes = false;

    for (const auto& space : model.getConcreteModelObjects<Space>()) {
      SpaceContributions sc{space, {}};
      for (const auto& planarSurface : defaultedPlanarSurfaces(space)) {
        auto cwsd_ = space.getDefaultConstructionWithSearchDistance(planarSurface);
        if (!cwsd_) {
          continue;
        }
        switch (static_cast<ConstructionDistance>(cwsd_->second)) {
          case ConstructionDistance::Space:
          case ConstructionDistance::SpaceType:
          case ConstructionDistance::BuildingStory:
            sc.contributes[cwsd_->second - 1] = true;
            break;
          case ConstructionDistance::Building:
            buildingContributes = true;
            break;
          case ConstructionDistance::BuildingSpaceType:
            buildingSpaceTypeContributes = true;
            break;
        }
      }
      spaceContributions.push_back(std::move(sc));
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // 2. Assign one DCS per Space and one for the Building
    // ---------------------------------------------------------------------------------------------------------------------
    std::map<std::vector<Handle>, DefaultConstructionSet> mergedCache;

    // chain is in priority order, only contributing levels. A level only contributes if it wins over every level above it,
    // so the chain can't hold the same DCS twice.
    auto resolveChain = [&model, &mergedCache](const std::vector<DefaultConstructionSet>& chain) -> boost::optional<DefaultConstructionSet> {
      if (chain.empty()) {
        return boost::none;
      }
      if (chain.size() == 1) {
        return chain.front();
      }

      std::vector<Handle> key;
      key.reserve(chain.size());
      std::ranges::transform(chain, std::back_inserter(key), [](const auto& dcs) { return dcs.handle(); });
      if (auto it = mergedCache.find(key); it != mergedCache.end()) {
        return it->second;
      }

      // Always start from a clone. The clone still points at the original Default(Sub)SurfaceConstructions; merge() only
      // clones one of those when it has to fill it from another partial one, so sub-object cloning is already minimal.
      auto merged = chain.front().clone(model).cast<DefaultConstructionSet>();
      std::string name = chain.front().nameString();
      for (auto it = std::next(chain.cbegin()); it != chain.cend(); ++it) {
        merged.merge(*it);
        name += " + " + it->nameString();
      }
      merged.setName(name);
      LOG_FREE(Info, logChannel, "Created merged DefaultConstructionSet '" << merged.nameString() << "'");

      mergedCache.emplace(std::move(key), merged);
      return merged;
    };

    // Building group. model.building(), not getUniqueModelObject, so we don't instantiate a Building
    if (auto building_ = model.building()) {
      std::vector<DefaultConstructionSet> chain;
      if (buildingContributes) {
        chain.push_back(*building_->defaultConstructionSet());
      }
      if (buildingSpaceTypeContributes) {
        chain.push_back(*building_->spaceType()->defaultConstructionSet());
      }
      if (auto dcs_ = resolveChain(chain)) {
        building_->setDefaultConstructionSet(*dcs_);
      } else {
        building_->resetDefaultConstructionSet();
      }
    }

    // Space group. Only reads SpaceType / Story DCSs, which are untouched until step 3
    for (auto& [space, contributes] : spaceContributions) {
      std::vector<DefaultConstructionSet> chain;
      if (contributes[0]) {
        chain.push_back(*space.defaultConstructionSet());
      }
      if (contributes[1]) {
        chain.push_back(*space.spaceType()->defaultConstructionSet());
      }
      if (contributes[2]) {
        chain.push_back(*space.buildingStory()->defaultConstructionSet());
      }

      if (auto dcs_ = resolveChain(chain)) {
        space.setDefaultConstructionSet(*dcs_);  // no-op when chain == {Space's own}
        LOG_FREE(Trace, logChannel, "Space '" << space.nameString() << "' uses DefaultConstructionSet '" << dcs_->nameString() << "'");
      } else {
        // Nothing comes from the Space group: everything defaulted resolves at the Building level (or not at all)
        space.resetDefaultConstructionSet();
      }
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // 3. Drop the intermediate levels so the OS lookup matches E+'s (includes the Building's SpaceType)
    // ---------------------------------------------------------------------------------------------------------------------
    for (auto& spaceType : model.getConcreteModelObjects<SpaceType>()) {
      spaceType.resetDefaultConstructionSet();
    }
    for (auto& story : model.getConcreteModelObjects<BuildingStory>()) {
      story.resetDefaultConstructionSet();
    }

    // Original DCSs that are no longer referenced are simply not translated (translate a DCS only when a Space or the Building
    // points at it).
  }

  // Test idea: snapshot construction() of every defaulted PlanarSurface before, call resolveDefaultConstructionSets, compare
  // after: must be identical. Also assert the number of DefaultConstructionSets created (clone only when 2+ levels meet,
  // shared across Spaces with the same contributing levels).

}  // namespace model

}  // namespace openstudio
