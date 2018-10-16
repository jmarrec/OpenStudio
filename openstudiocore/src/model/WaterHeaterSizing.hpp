/***********************************************************************************************************************
 *  OpenStudio(R), Copyright (c) 2008-2018, Alliance for Sustainable Energy, LLC. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 *  following conditions are met:
 *
 *  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 *  disclaimer.
 *
 *  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 *  following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 *  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote
 *  products derived from this software without specific prior written permission from the respective party.
 *
 *  (4) Other than as required in clauses (1) and (2), distributions in any form of modifications or other derivative
 *  works may not use the "OpenStudio" trademark, "OS", "os", or any other confusingly similar designation without
 *  specific prior written permission from Alliance for Sustainable Energy, LLC.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES GOVERNMENT, OR ANY CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **********************************************************************************************************************/

#ifndef MODEL_WATERHEATERSIZING_HPP
#define MODEL_WATERHEATERSIZING_HPP

#include <model/ModelAPI.hpp>
#include "ModelObject.hpp"

namespace openstudio {

namespace model {

// TODO: Check the following class names against object getters and setters.
class WaterHeater;

namespace detail {

  class WaterHeaterSizing_Impl;

} // detail

/** WaterHeaterSizing is a ModelObject that wraps the OpenStudio IDD object 'OS:WaterHeater:Sizing'. */
class MODEL_API WaterHeaterSizing : public ModelObject {
 public:
  /** @name Constructors and Destructors */
  //@{

  explicit WaterHeaterSizing(const Model& model);

  virtual ~WaterHeaterSizing() {}

  //@}

  static IddObjectType iddObjectType();

  static std::vector<std::string> designModeValues();

  /** @name Getters */
  //@{

  // TODO: Check return type. From object lists, some candidates are: WaterHeater.
  WaterHeater waterHeater() const;

  boost::optional<std::string> designMode() const;

  boost::optional<double> timeStorageCanMeetPeakDraw() const;

  boost::optional<double> timeforTankRecovery() const;

  boost::optional<double> nominalTankVolumeforAutosizingPlantConnections() const;

  boost::optional<int> numberofBedrooms() const;

  boost::optional<int> numberofBathrooms() const;

  boost::optional<double> storageCapacityperPerson() const;

  boost::optional<double> recoveryCapacityperPerson() const;

  boost::optional<double> storageCapacityperFloorArea() const;

  boost::optional<double> recoveryCapacityperFloorArea() const;

  boost::optional<double> numberofUnits() const;

  boost::optional<double> storageCapacityperUnit() const;

  boost::optional<double> recoveryCapacityPerUnit() const;

  boost::optional<double> storageCapacityperCollectorArea() const;

  boost::optional<double> heightAspectRatio() const;

  //@}
  /** @name Setters */
  //@{

  // TODO: Check argument type. From object lists, some candidates are: WaterHeater.
  bool setWaterHeater(const WaterHeater& waterHeater);

  bool setDesignMode(const std::string& designMode);

  void resetDesignMode();

  bool setTimeStorageCanMeetPeakDraw(double timeStorageCanMeetPeakDraw);

  void resetTimeStorageCanMeetPeakDraw();

  bool setTimeforTankRecovery(double timeforTankRecovery);

  void resetTimeforTankRecovery();

  void setNominalTankVolumeforAutosizingPlantConnections(double nominalTankVolumeforAutosizingPlantConnections);

  void resetNominalTankVolumeforAutosizingPlantConnections();

  bool setNumberofBedrooms(int numberofBedrooms);

  void resetNumberofBedrooms();

  bool setNumberofBathrooms(int numberofBathrooms);

  void resetNumberofBathrooms();

  bool setStorageCapacityperPerson(double storageCapacityperPerson);

  void resetStorageCapacityperPerson();

  bool setRecoveryCapacityperPerson(double recoveryCapacityperPerson);

  void resetRecoveryCapacityperPerson();

  bool setStorageCapacityperFloorArea(double storageCapacityperFloorArea);

  void resetStorageCapacityperFloorArea();

  bool setRecoveryCapacityperFloorArea(double recoveryCapacityperFloorArea);

  void resetRecoveryCapacityperFloorArea();

  void setNumberofUnits(double numberofUnits);

  void resetNumberofUnits();

  bool setStorageCapacityperUnit(double storageCapacityperUnit);

  void resetStorageCapacityperUnit();

  bool setRecoveryCapacityPerUnit(double recoveryCapacityPerUnit);

  void resetRecoveryCapacityPerUnit();

  bool setStorageCapacityperCollectorArea(double storageCapacityperCollectorArea);

  void resetStorageCapacityperCollectorArea();

  bool setHeightAspectRatio(double heightAspectRatio);

  void resetHeightAspectRatio();

  //@}
  /** @name Other */
  //@{

  //@}
 protected:
  /// @cond
  typedef detail::WaterHeaterSizing_Impl ImplType;

  explicit WaterHeaterSizing(std::shared_ptr<detail::WaterHeaterSizing_Impl> impl);

  friend class detail::WaterHeaterSizing_Impl;
  friend class Model;
  friend class IdfObject;
  friend class openstudio::detail::IdfObject_Impl;
  /// @endcond
 private:
  REGISTER_LOGGER("openstudio.model.WaterHeaterSizing");
};

/** \relates WaterHeaterSizing*/
typedef boost::optional<WaterHeaterSizing> OptionalWaterHeaterSizing;

/** \relates WaterHeaterSizing*/
typedef std::vector<WaterHeaterSizing> WaterHeaterSizingVector;

} // model
} // openstudio

#endif // MODEL_WATERHEATERSIZING_HPP

