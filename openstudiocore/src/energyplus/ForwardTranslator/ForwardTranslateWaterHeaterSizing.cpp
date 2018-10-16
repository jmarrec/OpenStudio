/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) 2008-2018, Alliance for Sustainable Energy, LLC. All rights reserved.
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

#include "../../model/WaterHeaterSizing.hpp"
#include "../../model/WaterHeaterSizing_Impl.hpp"
#include "../../model/WaterToWaterComponent.hpp"


#include <utilities/idd/WaterHeater_Sizing_FieldEnums.hxx>
#include "../../utilities/idd/IddEnums.hpp"
#include <utilities/idd/IddEnums.hxx>

using namespace openstudio::model;

namespace openstudio {

namespace energyplus {

boost::optional<IdfObject> ForwardTranslator::translateWaterHeaterSizing( WaterHeaterSizing & modelObject )
{
  boost::optional<double> _d;
  boost::optional<int> _i;

  // IdfObject idfObject = createRegisterAndNameIdfObject(openstudio::IddObjectType::WaterHeater_Sizing, modelObject);

  // Note JM 2018-10-16: We create an idfObject, but don't register it just yet
  // We'll have plenty of cases in which we won't translate because this object
  // has a choice field + many optional double/int fields that are required depending
  // onthe value of that choice field (design mode)
  IdfObject idfObject(openstudio::IddObjectType::WaterHeater_Sizing);

  // Name
  if (boost::optional<std::string> moName = modelObject.name()) {
    idfObject.setName(*moName);
  }

  // Water Heater Name
  // Note JM 2018-10-16: we only call translation of the WaterHeaterSizing inside - and right at the end - of the FT
  // for WaterHeaterMixed and WaterHeaterStratified, so we shouldn't get problems here.
  // Plus, we can call translateAndMap(wh) because it'll create a recursion.
  WaterToWaterComponent wh = modelObject.waterHeater();
  idfObject.setString(WaterHeater_SizingFields::WaterHeaterName, wh.nameString());

  // Design Mode
  std::string designMode = modelObject.designMode();
  idfObject.setString(WaterHeater_SizingFields::DesignMode, designMode);

    /***************************************************************************
  *                                PeakDraw                                 *
  ***************************************************************************/
  if (istringEqual("PeakDraw", designMode)) {

    // Time Storage Can Meet Peak Draw
    if ((_d = modelObject.timeStorageCanMeetPeakDraw())) {
      idfObject.setDouble(WaterHeater_SizingFields::TimeStorageCanMeetPeakDraw, _d.get());
    } else {
      LOG(Error, modelObject.briefDescription() << " is missing required field 'Time Storage Can Meet Peak Draw' and will not be translated");
      return boost::none;
    }


    // Time for Tank Recovery
    if ((_d = modelObject.timeforTankRecovery())) {
      idfObject.setDouble(WaterHeater_SizingFields::TimeforTankRecovery, _d.get());
    } else {
      LOG(Error, modelObject.briefDescription() << " is missing required field 'Time for Tank Recovery' and will not be translated");
      return boost::none;
    }


    // Nominal Tank Volume for Autosizing Plant Connections: only required for some other condition
    if ((_d = modelObject.nominalTankVolumeforAutosizingPlantConnections())) {
      idfObject.setDouble(WaterHeater_SizingFields::NominalTankVolumeforAutosizingPlantConnections, _d.get());
    }

  }
  /***************************************************************************
  *                        ResidentialHUD-FHAMinimum                        *
  ***************************************************************************/
  else if (istringEqual("ResidentialHUD-FHAMinimum", designMode)) {

    // Number of Bedrooms (int)
    if ((_i = modelObject.numberofBedrooms())) {
      idfObject.setInt(WaterHeater_SizingFields::NumberofBedrooms, _i.get());
    } else {
      LOG(Error, modelObject.briefDescription() << " is missing required field 'Number of Bedrooms' and will not be translated");
      return boost::none;
    }

    // Number of Bathrooms (int)
    if ((_i = modelObject.numberofBathrooms())) {
      idfObject.setInt(WaterHeater_SizingFields::NumberofBathrooms, _i.get());
    } else {
      LOG(Error, modelObject.briefDescription() << " is missing required field 'Number of Bathrooms' and will not be translated");
      return boost::none;
    }

  }
  /***************************************************************************
  *                                 PerPerson                                        *
  ***************************************************************************/
  else if (istringEqual("PerPerson", designMode)) {

    // Storage Capacity per Person
    if ((_d = modelObject.storageCapacityperPerson())) {
      idfObject.setDouble(WaterHeater_SizingFields::StorageCapacityperPerson, _d.get());
    } else {
      LOG(Error, modelObject.briefDescription() << " is missing required field 'Storage Capacity per Person' and will not be translated");
      return boost::none;
    }

    // Recovery Capacity per Person
    if ((_d = modelObject.recoveryCapacityperPerson())) {
      idfObject.setDouble(WaterHeater_SizingFields::RecoveryCapacityperPerson, _d.get());
    } else {
      LOG(Error, modelObject.briefDescription() << " is missing required field 'Recovery Capacity per Person' and will not be translated");
      return boost::none;
    }

  }
  /***************************************************************************
  *                                 PerFloorArea                                        *
  ***************************************************************************/
  else if (istringEqual("PerFloorArea", designMode)) {
    // Storage Capacity per Floor Area
    if ((_d = modelObject.storageCapacityperFloorArea())) {
      idfObject.setDouble(WaterHeater_SizingFields::StorageCapacityperFloorArea, _d.get());
    } else {
      LOG(Error, modelObject.briefDescription() << " is missing required field 'Storage Capacity per Floor Area' and will not be translated");
      return boost::none;
    }

    // Recovery Capacity per Floor Area
    if ((_d = modelObject.recoveryCapacityperFloorArea())) {
      idfObject.setDouble(WaterHeater_SizingFields::RecoveryCapacityperFloorArea, _d.get());
    } else {
      LOG(Error, modelObject.briefDescription() << " is missing required field 'Recovery Capacity per Floor Area' and will not be translated");
      return boost::none;
    }
  }
  /***************************************************************************
  *                                 PerUnit                                        *
  ***************************************************************************/
  else if (istringEqual("PerUnit", designMode)) {
    //  Number of Units

    //  Storage Capacity per Unit
    if ((_d = modelObject.storageCapacityperUnit())) {
      idfObject.setDouble(WaterHeater_SizingFields::StorageCapacityperUnit, _d.get());
    } else {
      LOG(Error, modelObject.briefDescription() << " is missing required field 'Storage Capacity per Unit' and will not be translated");
      return boost::none;
    }

    //  Recovery Capacity PerUnit
    if ((_d = modelObject.recoveryCapacityperUnit())) {
      // Note JM 2018-10-16: E+ Idd as of 9.0.1 has a capital 'P' in Per for this field, which differs from the others
      idfObject.setDouble(WaterHeater_SizingFields::RecoveryCapacityPerUnit, _d.get());
    } else {
      LOG(Error, modelObject.briefDescription() << " is missing required field 'Recovery Capacity per Unit' and will not be translated");
      return boost::none;
    }

  }
  /***************************************************************************
  *                          PerSolarCollectorArea                          *
  ***************************************************************************/
  else if (istringEqual("PerSolarCollectorArea", designMode)) {
    //  Storage Capacity per Collector Area
    if ((_d = modelObject.storageCapacityperCollectorArea())) {
      idfObject.setDouble(WaterHeater_SizingFields::StorageCapacityperCollectorArea, _d.get());
    } else {
      LOG(Error, modelObject.briefDescription() << " is missing required field 'Storage Capacity per Solar Collector Area' and will not be translated");
      return boost::none;
    }

  }
  else {
    // Shouldn't ever get here
    LOG(Error, "For " << modelObject.briefDescription() << ", unknown 'Design Mode' = " << designMode << ".");
    OS_ASSERT(false);

  } // End if statements on designMode

  //  Height Aspect Ratio
  if ((_d = modelObject.heightAspectRatio())) {
    // Note JM 2018-10-16: Technically we could also ensure that the attached wh is a WaterHeater:Mixed, and its tank shape is Vertical Cylinder
    // But I don't think it's worth the trouble
    idfObject.setDouble(WaterHeater_SizingFields::HeightAspectRatio, _d.get());
  }

  // At this point, our object is valid, so now we do register it!
  m_idfObjects.push_back(idfObject);
  m_map.insert(std::make_pair(modelObject.handle(), idfObject));

  return boost::optional<IdfObject>(idfObject);
}

} // energyplus

} // openstudio

