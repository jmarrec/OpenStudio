/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) 2008-2021, Alliance for Sustainable Energy, LLC, and other contributors. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
*  disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote products
*  derived from this software without specific prior written permission from the respective party.
*
*  (4) Other than as required in clauses (1) and (2), distributions in any form of modifications or other derivative works
*  may not use the "OpenStudio" trademark, "OS", "os", or any other confusingly similar designation without specific prior
*  written permission from Alliance for Sustainable Energy, LLC.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) AND ANY CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER(S), ANY CONTRIBUTORS, THE UNITED STATES GOVERNMENT, OR THE UNITED
*  STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
*  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
*  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************************************************************/

#include "../ForwardTranslator.hpp"

#include "../../model/Model.hpp"
#include "../../model/DaylightingControl.hpp"
#include "../../model/Space.hpp"
#include "../../model/ThermalZone.hpp"

#include "../../utilities/idd/IddEnums.hpp"
#include <utilities/idd/IddFactory.hxx>

using namespace openstudio::model;

using namespace std;

namespace openstudio {

namespace energyplus {

  boost::optional<IdfObject> ForwardTranslator::translateDaylightingControl(model::DaylightingControl& modelObject) {
    // real work is done in translateThermalZone
    // TODO: move the 'real' work here
    // TODO: make transateSpace call this one
    // In this one, do like SpaceLoad, figure out if we write to a Space or Zone
    // TODO: should we deprecate ThermalZone::setPrimary/SecondaryDaylightingControl. E+ doesn't seem to have any limitations on the number of
    // Reference Points. The thing is that the
    //

    boost::optional<Space> space_ = modelObject.space();
    if (!space_) {
      LOG(Warn, modelObject.briefDescription() << " isn't assigned to a Space, it will not be translated");
      return boost::none;
    }

    boost::optional<ThermalZone> thermalZone_ = space_->thermalZone();
    boost::optional<Schedule> availabilitySchedule;

    if (thermalZone_) {
      availabilitySchedule = thermalZone_->daylightingControlsAvailabilitySchedule();
    } else if (m_excludeSpaceTranslation) {
      LOG(Warn, modelObject.briefDescription() << " is assigned to a Space that has no thermal zone. It will not be translated");
      OS_ASSERT(false);  // THis shouldn't happen, translateSpace is responsible to trigger this one
      return boost::none;
    }

    OptionalIdfObject relatedIdfObject;

    if (boost::optional<Space> space = modelObject.space()) {
      if (m_excludeSpaceTranslation) {
        if (auto thermalZone_ = space->thermalZone()) {
          relatedIdfObject = translateAndMapModelObject(thermalZone_.get());
        } else {
          OS_ASSERT(false);  // This shouldn't happen, since we removed all orphaned spaces earlier in the FT
        }
      } else {
        relatedIdfObject = translateAndMapModelObject(*space);
      }
    }

    OS_ASSERT(relatedIdfObject);

    return boost::none;
  }

}  // namespace energyplus

}  // namespace openstudio
