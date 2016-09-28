/***********************************************************************************************************************
 *  OpenStudio(R), Copyright (c) 2008-2016, Alliance for Sustainable Energy, LLC. All rights reserved.
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

#ifndef MODEL_DESIGNSPECIFICATIONOUTDOORAIR_IMPL_HPP
#define MODEL_DESIGNSPECIFICATIONOUTDOORAIR_IMPL_HPP

#include "ModelAPI.hpp"
#include "ResourceObject_Impl.hpp"

#include "../utilities/units/Quantity.hpp"
#include "../utilities/units/OSOptionalQuantity.hpp"

namespace openstudio {
namespace model {

class Schedule;

namespace detail {

  /** DesignSpecificationOutdoorAir_Impl is a ModelObject_Impl that is the
   *  implementation class for DesignSpecificationOutdoorAir.*/
  class MODEL_API DesignSpecificationOutdoorAir_Impl : public ResourceObject_Impl {
    Q_OBJECT;

    Q_PROPERTY(std::string outdoorAirMethod READ outdoorAirMethod WRITE setOutdoorAirMethod RESET resetOutdoorAirMethod);
    Q_PROPERTY(bool isOutdoorAirMethodDefaulted READ isOutdoorAirMethodDefaulted);
    Q_PROPERTY(std::vector<std::string> outdoorAirMethodValues READ outdoorAirMethodValues);

    Q_PROPERTY(double outdoorAirFlowperPerson READ outdoorAirFlowperPerson WRITE setOutdoorAirFlowperPerson RESET resetOutdoorAirFlowperPerson);
    Q_PROPERTY(openstudio::Quantity outdoorAirFlowperPerson_SI READ outdoorAirFlowperPerson_SI WRITE setOutdoorAirFlowperPerson RESET resetOutdoorAirFlowperPerson);
    Q_PROPERTY(openstudio::Quantity outdoorAirFlowperPerson_IP READ outdoorAirFlowperPerson_IP WRITE setOutdoorAirFlowperPerson RESET resetOutdoorAirFlowperPerson);
    Q_PROPERTY(bool isOutdoorAirFlowperPersonDefaulted READ isOutdoorAirFlowperPersonDefaulted);

    Q_PROPERTY(double outdoorAirFlowperFloorArea READ outdoorAirFlowperFloorArea WRITE setOutdoorAirFlowperFloorArea RESET resetOutdoorAirFlowperFloorArea);
    Q_PROPERTY(openstudio::Quantity outdoorAirFlowperFloorArea_SI READ outdoorAirFlowperFloorArea_SI WRITE setOutdoorAirFlowperFloorArea RESET resetOutdoorAirFlowperFloorArea);
    Q_PROPERTY(openstudio::Quantity outdoorAirFlowperFloorArea_IP READ outdoorAirFlowperFloorArea_IP WRITE setOutdoorAirFlowperFloorArea RESET resetOutdoorAirFlowperFloorArea);
    Q_PROPERTY(bool isOutdoorAirFlowperFloorAreaDefaulted READ isOutdoorAirFlowperFloorAreaDefaulted);

    Q_PROPERTY(double outdoorAirFlowRate READ outdoorAirFlowRate WRITE setOutdoorAirFlowRate RESET resetOutdoorAirFlowRate);
    Q_PROPERTY(openstudio::Quantity outdoorAirFlowRate_SI READ outdoorAirFlowRate_SI WRITE setOutdoorAirFlowRate RESET resetOutdoorAirFlowRate);
    Q_PROPERTY(openstudio::Quantity outdoorAirFlowRate_IP READ outdoorAirFlowRate_IP WRITE setOutdoorAirFlowRate RESET resetOutdoorAirFlowRate);
    Q_PROPERTY(bool isOutdoorAirFlowRateDefaulted READ isOutdoorAirFlowRateDefaulted);

    Q_PROPERTY(double outdoorAirFlowAirChangesperHour READ outdoorAirFlowAirChangesperHour WRITE setOutdoorAirFlowAirChangesperHour RESET resetOutdoorAirFlowAirChangesperHour);
    Q_PROPERTY(openstudio::Quantity outdoorAirFlowAirChangesperHour_SI READ outdoorAirFlowAirChangesperHour_SI WRITE setOutdoorAirFlowAirChangesperHour RESET resetOutdoorAirFlowAirChangesperHour);
    Q_PROPERTY(openstudio::Quantity outdoorAirFlowAirChangesperHour_IP READ outdoorAirFlowAirChangesperHour_IP WRITE setOutdoorAirFlowAirChangesperHour RESET resetOutdoorAirFlowAirChangesperHour);
    Q_PROPERTY(bool isOutdoorAirFlowAirChangesperHourDefaulted READ isOutdoorAirFlowAirChangesperHourDefaulted);

    Q_PROPERTY(boost::optional<openstudio::model::ModelObject> outdoorAirFlowRateFractionSchedule READ outdoorAirFlowRateFractionScheduleAsModelObject WRITE setOutdoorAirFlowRateFractionScheduleAsModelObject RESET resetOutdoorAirFlowRateFractionSchedule);
   public:
    /** @name Constructors and Destructors */
    //@{

    DesignSpecificationOutdoorAir_Impl(const IdfObject& idfObject, Model_Impl* model, bool keepHandle);

    DesignSpecificationOutdoorAir_Impl(const openstudio::detail::WorkspaceObject_Impl& other,
                                       Model_Impl* model,
                                       bool keepHandle);

    DesignSpecificationOutdoorAir_Impl(const DesignSpecificationOutdoorAir_Impl& other,
                                       Model_Impl* model,
                                       bool keepHandle);

    virtual ~DesignSpecificationOutdoorAir_Impl() {}

    //@}
    /** @name Virtual Methods */
    //@{

    virtual const std::vector<std::string>& outputVariableNames() const override;

    virtual IddObjectType iddObjectType() const override;

    virtual std::vector<ScheduleTypeKey> getScheduleTypeKeys(const Schedule& schedule) const override;

    //@}
    /** @name Getters */
    //@{

    std::string outdoorAirMethod() const;

    bool isOutdoorAirMethodDefaulted() const;

    double outdoorAirFlowperPerson() const;

    Quantity getOutdoorAirFlowperPerson(bool returnIP=false) const;

    bool isOutdoorAirFlowperPersonDefaulted() const;

    double outdoorAirFlowperFloorArea() const;

    Quantity getOutdoorAirFlowperFloorArea(bool returnIP=false) const;

    bool isOutdoorAirFlowperFloorAreaDefaulted() const;

    double outdoorAirFlowRate() const;

    Quantity getOutdoorAirFlowRate(bool returnIP=false) const;

    bool isOutdoorAirFlowRateDefaulted() const;

    double outdoorAirFlowAirChangesperHour() const;

    Quantity getOutdoorAirFlowAirChangesperHour(bool returnIP=false) const;

    bool isOutdoorAirFlowAirChangesperHourDefaulted() const;

    boost::optional<Schedule> outdoorAirFlowRateFractionSchedule() const;

    //@}
    /** @name Setters */
    //@{

    bool setOutdoorAirMethod(std::string outdoorAirMethod);

    void resetOutdoorAirMethod();

    bool setOutdoorAirFlowperPerson(double outdoorAirFlowperPerson);

    bool setOutdoorAirFlowperPerson(const Quantity& outdoorAirFlowperPerson);

    void resetOutdoorAirFlowperPerson();

    bool setOutdoorAirFlowperFloorArea(double outdoorAirFlowperFloorArea);

    bool setOutdoorAirFlowperFloorArea(const Quantity& outdoorAirFlowperFloorArea);

    void resetOutdoorAirFlowperFloorArea();

    bool setOutdoorAirFlowRate(double outdoorAirFlowRate);

    bool setOutdoorAirFlowRate(const Quantity& outdoorAirFlowRate);

    void resetOutdoorAirFlowRate();

    bool setOutdoorAirFlowAirChangesperHour(double outdoorAirFlowAirChangesperHour);

    bool setOutdoorAirFlowAirChangesperHour(const Quantity& outdoorAirFlowAirChangesperHour);

    void resetOutdoorAirFlowAirChangesperHour();

    bool setOutdoorAirFlowRateFractionSchedule(Schedule& schedule);

    void resetOutdoorAirFlowRateFractionSchedule();

    //@}
    /** @name Other */
    //@{

    //@}
   protected:
   private:
    REGISTER_LOGGER("openstudio.model.DesignSpecificationOutdoorAir");

    std::vector<std::string> outdoorAirMethodValues() const;
    openstudio::Quantity outdoorAirFlowperPerson_SI() const;
    openstudio::Quantity outdoorAirFlowperPerson_IP() const;
    openstudio::Quantity outdoorAirFlowperFloorArea_SI() const;
    openstudio::Quantity outdoorAirFlowperFloorArea_IP() const;
    openstudio::Quantity outdoorAirFlowRate_SI() const;
    openstudio::Quantity outdoorAirFlowRate_IP() const;
    openstudio::Quantity outdoorAirFlowAirChangesperHour_SI() const;
    openstudio::Quantity outdoorAirFlowAirChangesperHour_IP() const;

    boost::optional<ModelObject> outdoorAirFlowRateFractionScheduleAsModelObject() const;

    bool setOutdoorAirFlowRateFractionScheduleAsModelObject(const boost::optional<ModelObject>& modelObject);
  };

} // detail

} // model
} // openstudio

#endif // MODEL_DESIGNSPECIFICATIONOUTDOORAIR_IMPL_HPP

