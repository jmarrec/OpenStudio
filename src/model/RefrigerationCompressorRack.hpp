/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) 2008-2020, Alliance for Sustainable Energy, LLC, and other contributors. All rights reserved.
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

#ifndef MODEL_REFRIGERATIONCOMPRESSORRACK_HPP
#define MODEL_REFRIGERATIONCOMPRESSORRACK_HPP

#include "ModelAPI.hpp"
#include "StraightComponent.hpp"

namespace openstudio {

namespace model {

  class Curve;
  class Schedule;
  class ThermalZone;
  class ModelObjectList;
  class RefrigerationSystem;
  class RefrigerationAirChiller;
  class RefrigerationCase;
  class RefrigerationWalkIn;

  namespace detail {

    class RefrigerationCompressorRack_Impl;

  }  // namespace detail

  /** RefrigerationCompressorRack is a StraightComponent that wraps the OpenStudio IDD object 'OS:Refrigeration:CompressorRack'. */
  class MODEL_API RefrigerationCompressorRack : public StraightComponent
  {
   public:
    /** @name Constructors and Destructors */
    //@{

    explicit RefrigerationCompressorRack(const Model& model);

    virtual ~RefrigerationCompressorRack() {}

    //@}

    static IddObjectType iddObjectType();

    static std::vector<std::string> condenserTypeValues();

    static std::vector<std::string> waterCooledLoopFlowTypeValues();

    /** @name Getters */
    //@{

    std::string heatRejectionLocation() const;

    double designCompressorRackCOP() const;

    Curve compressorRackCOPFunctionofTemperatureCurve() const;

    double designCondenserFanPower() const;

    boost::optional<Curve> condenserFanPowerFunctionofTemperatureCurve() const;

    std::string condenserType() const;

    std::string waterCooledLoopFlowType() const;

    boost::optional<Schedule> waterCooledCondenserOutletTemperatureSchedule() const;

    boost::optional<double> waterCooledCondenserDesignFlowRate() const;

    boost::optional<double> waterCooledCondenserMaximumFlowRate() const;

    double waterCooledCondenserMaximumWaterOutletTemperature() const;

    double waterCooledCondenserMinimumWaterInletTemperature() const;

    boost::optional<Schedule> evaporativeCondenserAvailabilitySchedule() const;

    double evaporativeCondenserEffectiveness() const;

    boost::optional<double> evaporativeCondenserAirFlowRate() const;

    bool isEvaporativeCondenserAirFlowRateAutocalculated() const;

    double basinHeaterCapacity() const;

    double basinHeaterSetpointTemperature() const;

    boost::optional<double> designEvaporativeCondenserWaterPumpPower() const;

    bool isDesignEvaporativeCondenserWaterPumpPowerAutocalculated() const;

    // boost::optional<HVACComponent> evaporativeWaterSupplyTank() const;

    // boost::optional<std::string> condenserAirInletNodeName() const;

    std::string endUseSubcategory() const;

    bool isEndUseSubcategoryDefaulted() const;

    boost::optional<ThermalZone> heatRejectionZone() const;

    //@}
    /** @name Setters */
    //@{

    bool setHeatRejectionLocation(std::string heatRejectionLocation);

    bool setDesignCompressorRackCOP(double designCompressorRackCOP);

    bool setCompressorRackCOPFunctionofTemperatureCurve(const Curve& curve);

    bool setDesignCondenserFanPower(double designCondenserFanPower);

    bool setCondenserFanPowerFunctionofTemperatureCurve(const Curve& curve);

    void resetCondenserFanPowerFunctionofTemperatureCurve();

    bool setCondenserType(std::string condenserType);

    bool setWaterCooledLoopFlowType(std::string waterCooledLoopFlowType);

    bool setWaterCooledCondenserOutletTemperatureSchedule(Schedule& schedule);

    void resetWaterCooledCondenserOutletTemperatureSchedule();

    bool setWaterCooledCondenserDesignFlowRate(double waterCooledCondenserDesignFlowRate);

    void resetWaterCooledCondenserDesignFlowRate();

    bool setWaterCooledCondenserMaximumFlowRate(double waterCooledCondenserMaximumFlowRate);

    void resetWaterCooledCondenserMaximumFlowRate();

    bool setWaterCooledCondenserMaximumWaterOutletTemperature(double waterCooledCondenserMaximumWaterOutletTemperature);

    bool setWaterCooledCondenserMinimumWaterInletTemperature(double waterCooledCondenserMinimumWaterInletTemperature);

    bool setEvaporativeCondenserAvailabilitySchedule(Schedule& schedule);

    void resetEvaporativeCondenserAvailabilitySchedule();

    bool setEvaporativeCondenserEffectiveness(double evaporativeCondenserEffectiveness);

    bool setEvaporativeCondenserAirFlowRate(double evaporativeCondenserAirFlowRate);

    bool autocalculateEvaporativeCondenserAirFlowRate();

    bool setBasinHeaterCapacity(double basinHeaterCapacity);

    bool setBasinHeaterSetpointTemperature(double basinHeaterSetpointTemperature);

    bool setDesignEvaporativeCondenserWaterPumpPower(double designEvaporativeCondenserWaterPumpPower);

    bool autocalculateDesignEvaporativeCondenserWaterPumpPower();

    // bool setEvaporativeWaterSupplyTank(const HVACComponent& evaporativeWaterSupplyTank);

    // void resetEvaporativeWaterSupplyTank();

    // bool setCondenserAirInletNodeName(const boost::optional<std::string>& condenserAirInletNodeName);

    // void resetCondenserAirInletNodeName();

    bool setEndUseSubcategory(std::string endUseSubcategory);

    void resetEndUseSubcategory();

    bool setHeatRejectionZone(const ThermalZone& thermalZone);

    void resetHeatRejectionZone();

    //@}
    /** @name Other */
    //@{

    boost::optional<RefrigerationSystem> system() const;

    //@}
   protected:
    /// @cond
    typedef detail::RefrigerationCompressorRack_Impl ImplType;

    explicit RefrigerationCompressorRack(std::shared_ptr<detail::RefrigerationCompressorRack_Impl> impl);

    friend class detail::RefrigerationCompressorRack_Impl;
    friend class Model;
    friend class IdfObject;
    friend class openstudio::detail::IdfObject_Impl;
    /// @endcond
   private:
    REGISTER_LOGGER("openstudio.model.RefrigerationCompressorRack");
  };

  /** \relates RefrigerationCompressorRack*/
  typedef boost::optional<RefrigerationCompressorRack> OptionalRefrigerationCompressorRack;

  /** \relates RefrigerationCompressorRack*/
  typedef std::vector<RefrigerationCompressorRack> RefrigerationCompressorRackVector;

}  // namespace model
}  // namespace openstudio

#endif  // MODEL_REFRIGERATIONCOMPRESSORRACK_HPP
