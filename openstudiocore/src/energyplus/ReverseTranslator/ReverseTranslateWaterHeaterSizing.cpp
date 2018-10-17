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

#include "../ReverseTranslator.hpp"

#include "../../model/WaterHeaterSizing.hpp"

#include "../../model/WaterHeaterMixed.hpp"
#include "../../model/WaterHeaterMixed_Impl.hpp"
#include "../../model/WaterHeaterStratified.hpp"
#include "../../model/WaterHeaterStratified_Impl.hpp"

#include <utilities/idd/WaterHeater_Sizing_FieldEnums.hxx>
#include <utilities/idd/IddEnums.hxx>

using namespace openstudio::model;

namespace openstudio {

namespace energyplus {

OptionalModelObject ReverseTranslator::translateWaterHeaterSizing( const WorkspaceObject & workspaceObject )
{
  boost::optional<ModelObject> result;
  boost::optional<WorkspaceObject> _wo;
  boost::optional<ModelObject> _mo;

  // Note JM 2018-10-17: We don't know if we can actually instantiate this one...
  // the WaterHeaters are the only ones who can

  boost::optional<openstudio::model::WaterHeaterSizing> _modelObject;
  // WaterHeater Name: Required Object
  if ((_wo = workspaceObject.getTarget(WaterHeater_SizingFields::WaterHeaterName))) {
    if( (_mo = translateAndMapWorkspaceObject(_wo.get())) ) {
      // Can only be a WaterHeater:Mixed, or WaterHeater:Stratified, and these two are the only ones that can instantiate a WaterHeater:Sizing object
      if (boost::optional<WaterHeaterMixed> _waterHeater = _mo->optionalCast<WaterHeaterMixed>()) {
        _modelObject = _waterHeater.get().waterHeaterSizing();
      } else if (boost::optional<WaterHeaterStratified> _waterHeater = _mo->optionalCast<WaterHeaterStratified>()) {
        _modelObject = _waterHeater.get().waterHeaterSizing();
      } else {
        LOG(Error, workspaceObject.briefDescription() << " has a wrong type for 'WaterHeater Name'");
        return result;
      }
    } else {
      LOG(Error, "For " << workspaceObject.briefDescription() << ", cannot reverse translate required object 'WaterHeater Name'");
      return result;
    }
  } else {
    LOG(Error, "For " << workspaceObject.briefDescription() << ", cannot find required object 'WaterHeater Name'");
    return result;
  }

  // Note JM 2018-10-17:
  // Design Mode: Optional String in IDD, but actually really mandatory
  if (boost::optional<std::string> _designMode = workspaceObject.getString(WaterHeater_SizingFields::DesignMode)) {
    _modelObject->setDesignMode(_designMode.get());
  } else {
    LOG(Error, "For " << workspaceObject.briefDescription() << ", cannot find required property 'Design Mode'");
    return result;
  }

  // Time Storage Can Meet Peak Draw: Optional Double
  if (boost::optional<double> _timeStorageCanMeetPeakDraw = workspaceObject.getDouble(WaterHeater_SizingFields::TimeStorageCanMeetPeakDraw)) {
    _modelObject->setTimeStorageCanMeetPeakDraw(_timeStorageCanMeetPeakDraw.get());
  }

  // Time for Tank Recovery: Optional Double
  if (boost::optional<double> _timeforTankRecovery = workspaceObject.getDouble(WaterHeater_SizingFields::TimeforTankRecovery)) {
    _modelObject->setTimeforTankRecovery(_timeforTankRecovery.get());
  }

  // Nominal Tank Volume for Autosizing Plant Connections: Optional Double
  if (boost::optional<double> _nominalTankVolumeforAutosizingPlantConnections = workspaceObject.getDouble(WaterHeater_SizingFields::NominalTankVolumeforAutosizingPlantConnections)) {
    _modelObject->setNominalTankVolumeforAutosizingPlantConnections(_nominalTankVolumeforAutosizingPlantConnections.get());
  }

  // Number of Bedrooms: Optional Integer
  if (boost::optional<int> _numberofBedrooms = workspaceObject.getInt(WaterHeater_SizingFields::NumberofBedrooms)) {
    _modelObject->setNumberofBedrooms(_numberofBedrooms.get());
  }

  // Number of Bathrooms: Optional Integer
  if (boost::optional<int> _numberofBathrooms = workspaceObject.getInt(WaterHeater_SizingFields::NumberofBathrooms)) {
    _modelObject->setNumberofBathrooms(_numberofBathrooms.get());
  }

  // Storage Capacity per Person: Optional Double
  if (boost::optional<double> _storageCapacityperPerson = workspaceObject.getDouble(WaterHeater_SizingFields::StorageCapacityperPerson)) {
    _modelObject->setStorageCapacityperPerson(_storageCapacityperPerson.get());
  }

  // Recovery Capacity per Person: Optional Double
  if (boost::optional<double> _recoveryCapacityperPerson = workspaceObject.getDouble(WaterHeater_SizingFields::RecoveryCapacityperPerson)) {
    _modelObject->setRecoveryCapacityperPerson(_recoveryCapacityperPerson.get());
  }

  // Storage Capacity per Floor Area: Optional Double
  if (boost::optional<double> _storageCapacityperFloorArea = workspaceObject.getDouble(WaterHeater_SizingFields::StorageCapacityperFloorArea)) {
    _modelObject->setStorageCapacityperFloorArea(_storageCapacityperFloorArea.get());
  }

  // Recovery Capacity per Floor Area: Optional Double
  if (boost::optional<double> _recoveryCapacityperFloorArea = workspaceObject.getDouble(WaterHeater_SizingFields::RecoveryCapacityperFloorArea)) {
    _modelObject->setRecoveryCapacityperFloorArea(_recoveryCapacityperFloorArea.get());
  }

  // Number of Units: Optional Double
  if (boost::optional<double> _numberofUnits = workspaceObject.getDouble(WaterHeater_SizingFields::NumberofUnits)) {
    _modelObject->setNumberofUnits(_numberofUnits.get());
  }

  // Storage Capacity per Unit: Optional Double
  if (boost::optional<double> _storageCapacityperUnit = workspaceObject.getDouble(WaterHeater_SizingFields::StorageCapacityperUnit)) {
    _modelObject->setStorageCapacityperUnit(_storageCapacityperUnit.get());
  }

  // Recovery Capacity PerUnit: Optional Double
  if (boost::optional<double> _recoveryCapacityperUnit = workspaceObject.getDouble(WaterHeater_SizingFields::RecoveryCapacityPerUnit)) {
    _modelObject->setRecoveryCapacityperUnit(_recoveryCapacityperUnit.get());
  }

  // Storage Capacity per Collector Area: Optional Double
  if (boost::optional<double> _storageCapacityperCollectorArea = workspaceObject.getDouble(WaterHeater_SizingFields::StorageCapacityperCollectorArea)) {
    _modelObject->setStorageCapacityperCollectorArea(_storageCapacityperCollectorArea.get());
  }

  // Height Aspect Ratio: Optional Double
  if (boost::optional<double> _heightAspectRatio = workspaceObject.getDouble(WaterHeater_SizingFields::HeightAspectRatio)) {
    _modelObject->setHeightAspectRatio(_heightAspectRatio.get());
  }

  result = _modelObject;
  return result;
}

} // end namespace energyplus

} // end namespace openstudio
