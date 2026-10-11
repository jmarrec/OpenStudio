// Throwaway sketch: collapse OpenStudio's 5-level DefaultConstructionSet inheritance into the
// 2 levels E+ ConstructionAssignmentSet supports (Space, Building).
//
// Runs on the FT's model copy, in translateModelPrivate:
//   AFTER  resolveMatched(Sub)SurfaceConstructionConflicts  (they need the original 0..5 search distances)
//   BEFORE the "remove unused space types" loop             (it can delete the Building's SpaceType, and its DCS with it)
//   BEFORE ThermalZone::combineSpaces                        (fine either way, it hard-applies constructions, but keep it simple)
//
// Post-conditions on the model copy:
//   * space.defaultConstructionSet()    == the E+ Space-level set (or empty)
//   * building.defaultConstructionSet() == the E+ Building-level set (or empty)
//   * no SpaceType / BuildingStory has a DefaultConstructionSet anymore
// => Every surface resolves to the same construction as before (per slot, first hit wins is preserved by merge order),
//    and the OS lookup now mirrors E+'s Space -> Building lookup exactly. ForwardTranslateSpace / ForwardTranslateBuilding
//    then just translate whatever DCS they point to.

#include "Building.hpp"
#include "BuildingStory.hpp"
#include "BuildingStory_Impl.hpp"
#include "DefaultConstructionSet.hpp"
#include "DefaultConstructionSet_Impl.hpp"
#include "Model.hpp"
#include "Space.hpp"
#include "Space_Impl.hpp"
#include "SpaceType.hpp"
#include "SpaceType_Impl.hpp"

#include <boost/optional.hpp>

#include <algorithm>
#include <initializer_list>
#include <map>
#include <string>
#include <vector>

namespace openstudio {

namespace model {

  namespace {
    /// Ordered, de-duplicated chain of sets, highest priority first.
    std::vector<DefaultConstructionSet> makeChain(std::initializer_list<boost::optional<DefaultConstructionSet>> candidates) {
      std::vector<DefaultConstructionSet> chain;
      for (const auto& dcs_ : candidates) {
        if (!dcs_) {
          continue;
        }
        const bool alreadyIn = std::ranges::any_of(chain, [&dcs_](const DefaultConstructionSet& c) { return c.handle() == dcs_->handle(); });
        if (!alreadyIn) {
          chain.push_back(*dcs_);
        }
      }
      return chain;
    }
  }  // namespace

  void resolveDefaultConstructionSets(model::Model& model) {

    using Key = std::vector<Handle>;
    std::map<Key, DefaultConstructionSet> mergedCache;

    // 0 sets -> none, 1 set -> reuse as is (no clone), 2+ sets -> one merged clone per unique chain
    auto getOrCreateMerged = [&model, &mergedCache](const std::vector<DefaultConstructionSet>& chain) -> boost::optional<DefaultConstructionSet> {
      if (chain.empty()) {
        return boost::none;
      }
      if (chain.size() == 1) {
        return chain.front();
      }

      Key key;
      key.reserve(chain.size());
      std::ranges::transform(chain, std::back_inserter(key), [](const auto& c) { return c.handle(); });
      if (auto it = mergedCache.find(key); it != mergedCache.end()) {
        return it->second;
      }

      // Never merge into an original: it may be reused as-is by another single-set chain.
      // The clone still points at the original Default(Sub)SurfaceConstructions; merge() only clones one of those when it
      // actually has to fill it from another partial one (numSources() > 1), and simply points at the other's when it has none.
      // So sub-object cloning is already minimal.
      auto merged = chain.front().clone(model).cast<DefaultConstructionSet>();
      std::string name = chain.front().nameString();
      for (auto it = std::next(chain.cbegin()); it != chain.cend(); ++it) {
        merged.merge(*it);
        name += " + " + it->nameString();
      }
      merged.setName(name);

      mergedCache.emplace(std::move(key), merged);
      return merged;
    };

    // Building level: Building DCS, then Building SpaceType DCS (rungs 4, 5)
    // Note: model.building() (not getUniqueModelObject) so we don't instantiate a Building
    if (auto building_ = model.building()) {
      boost::optional<DefaultConstructionSet> bldgSpaceTypeDCS_;
      if (auto st_ = building_->spaceType()) {
        bldgSpaceTypeDCS_ = st_->defaultConstructionSet();
      }
      const auto chain = makeChain({building_->defaultConstructionSet(), bldgSpaceTypeDCS_});
      if (auto dcs_ = getOrCreateMerged(chain)) {
        building_->setDefaultConstructionSet(*dcs_);
      }
    }

    // Space level: Space DCS, then *explicit* SpaceType DCS, then BuildingStory DCS (rungs 1, 2, 3)
    // Safe to assign as we go: each space's chain only reads its own Space field, plus SpaceType/Story fields which we
    // leave untouched until the end.
    for (auto& space : model.getConcreteModelObjects<model::Space>()) {
      boost::optional<DefaultConstructionSet> spaceTypeDCS_;
      if (!space.isSpaceTypeDefaulted()) {  // defaulted == Building's SpaceType, which is rung 5, already in the Building set
        if (auto st_ = space.spaceType()) {
          spaceTypeDCS_ = st_->defaultConstructionSet();
        }
      }
      boost::optional<DefaultConstructionSet> storyDCS_;
      if (auto story_ = space.buildingStory()) {
        storyDCS_ = story_->defaultConstructionSet();
      }

      const auto chain = makeChain({space.defaultConstructionSet(), spaceTypeDCS_, storyDCS_});
      if (auto dcs_ = getOrCreateMerged(chain)) {
        space.setDefaultConstructionSet(*dcs_);  // no-op when chain == {space's own}
      }
    }

    // Everything now lives on Space / Building. Drop the intermediate levels so the OS lookup matches E+'s.
    // (This includes the Building's SpaceType, which is just a SpaceType.)
    for (auto& spaceType : model.getConcreteModelObjects<model::SpaceType>()) {
      spaceType.resetDefaultConstructionSet();
    }
    for (auto& story : model.getConcreteModelObjects<model::BuildingStory>()) {
      story.resetDefaultConstructionSet();
    }

    // Original DCSs that ended up unreferenced are simply not translated (translate DCS only when Space/Building points at it),
    // so nothing to clean up here.
  }

  // Debug check worth keeping while developing: snapshot every PlanarSurface's construction() before, compare after.
  // They must be identical. Search distances will differ (2,3 -> 1 and 5 -> 4), which is exactly why the matched-surface
  // conflict resolution must run first.

}  // namespace model

}  // namespace openstudio
