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

#ifndef MODEL_MASSLESSOPAQUEMATERIAL_IMPL_HPP
#define MODEL_MASSLESSOPAQUEMATERIAL_IMPL_HPP

#include "ModelAPI.hpp"
#include "OpaqueMaterial_Impl.hpp"

#include "../utilities/units/Quantity.hpp"
#include "../utilities/units/OSOptionalQuantity.hpp"

namespace openstudio {
namespace model {

namespace detail {

  /** MasslessOpaqueMaterial_Impl is a OpaqueMaterial_Impl that is the implementation class for MasslessOpaqueMaterial.*/
  class MODEL_API MasslessOpaqueMaterial_Impl : public OpaqueMaterial_Impl {
    Q_OBJECT;

    Q_PROPERTY(std::string roughness READ roughness WRITE setRoughness);
    Q_PROPERTY(std::vector<std::string> roughnessValues READ roughnessValues);

    Q_PROPERTY(double thermalResistance READ thermalResistance WRITE setThermalResistance);
    Q_PROPERTY(openstudio::Quantity thermalResistance_SI READ thermalResistance_SI WRITE setThermalResistance);
    Q_PROPERTY(openstudio::Quantity thermalResistance_IP READ thermalResistance_IP WRITE setThermalResistance);

    Q_PROPERTY(double thermalAbsorptance READ thermalAbsorptance WRITE setThermalAbsorptance RESET resetThermalAbsorptance);
    Q_PROPERTY(openstudio::Quantity thermalAbsorptance_SI READ thermalAbsorptance_SI WRITE setThermalAbsorptance RESET resetThermalAbsorptance);
    Q_PROPERTY(openstudio::Quantity thermalAbsorptance_IP READ thermalAbsorptance_IP WRITE setThermalAbsorptance RESET resetThermalAbsorptance);
    Q_PROPERTY(bool isThermalAbsorptanceDefaulted READ isThermalAbsorptanceDefaulted);

    Q_PROPERTY(double solarAbsorptance READ solarAbsorptance WRITE setSolarAbsorptance RESET resetSolarAbsorptance);
    Q_PROPERTY(openstudio::Quantity solarAbsorptance_SI READ solarAbsorptance_SI WRITE setSolarAbsorptance RESET resetSolarAbsorptance);
    Q_PROPERTY(openstudio::Quantity solarAbsorptance_IP READ solarAbsorptance_IP WRITE setSolarAbsorptance RESET resetSolarAbsorptance);
    Q_PROPERTY(bool isSolarAbsorptanceDefaulted READ isSolarAbsorptanceDefaulted);

    Q_PROPERTY(double visibleAbsorptance READ visibleAbsorptance WRITE setVisibleAbsorptance RESET resetVisibleAbsorptance);
    Q_PROPERTY(openstudio::Quantity visibleAbsorptance_SI READ visibleAbsorptance_SI WRITE setVisibleAbsorptance RESET resetVisibleAbsorptance);
    Q_PROPERTY(openstudio::Quantity visibleAbsorptance_IP READ visibleAbsorptance_IP WRITE setVisibleAbsorptance RESET resetVisibleAbsorptance);
    Q_PROPERTY(bool isVisibleAbsorptanceDefaulted READ isVisibleAbsorptanceDefaulted);

    Q_PROPERTY(double conductivity READ conductivity WRITE setConductivity);
    Q_PROPERTY(openstudio::Quantity conductivity_SI READ conductivity_SI WRITE setConductivity);
    Q_PROPERTY(openstudio::Quantity conductivity_IP READ conductivity_IP WRITE setConductivity);

    Q_PROPERTY(double density READ density WRITE setDensity);
    Q_PROPERTY(openstudio::Quantity density_SI READ density_SI WRITE setDensity);
    Q_PROPERTY(openstudio::Quantity density_IP READ density_IP WRITE setDensity);

    Q_PROPERTY(double specificHeat READ specificHeat WRITE setSpecificHeat);
    Q_PROPERTY(openstudio::Quantity specificHeat_SI READ specificHeat_SI WRITE setSpecificHeat);
    Q_PROPERTY(openstudio::Quantity specificHeat_IP READ specificHeat_IP WRITE setSpecificHeat);

    // TODO: Add relationships for objects related to this one, but not pointed to by the underlying data.
    //       Such relationships can be generated by the GenerateRelationships.rb script.
   public:
    /** @name Constructors and Destructors */
    //@{

    MasslessOpaqueMaterial_Impl(const IdfObject& idfObject,
                                Model_Impl* model,
                                bool keepHandle);

    MasslessOpaqueMaterial_Impl(const openstudio::detail::WorkspaceObject_Impl& other,
                                Model_Impl* model,
                                bool keepHandle);

    MasslessOpaqueMaterial_Impl(const MasslessOpaqueMaterial_Impl& other,
                                Model_Impl* model,
                                bool keepHandle);

    virtual ~MasslessOpaqueMaterial_Impl() {}

    //@}

    /** @name Virtual Methods */
    //@{

    virtual const std::vector<std::string>& outputVariableNames() const override;

    virtual IddObjectType iddObjectType() const override;

    //@}
    /** @name Getters */
    //@{

    std::string roughness() const;

    /** The conductivitiy of the material in W/m*K. */
    virtual double thermalConductivity() const override;

    /** The conductance of the material in W/m^2*K. */
    virtual double thermalConductance() const override;

    /** The resistivity of the material in m*K/W. */
    virtual double thermalResistivity() const override;

    /** The resistance of the material in m^2*K/W. */
    virtual double thermalResistance() const override;

    virtual double thermalAbsorptance() const override;

    virtual boost::optional<double> thermalReflectance() const override;

    virtual double solarAbsorptance() const override;

    virtual boost::optional<double> solarReflectance() const override;

    virtual double visibleAbsorptance() const override;

    virtual boost::optional<double> visibleReflectance() const override;

    Quantity getThermalResistance(bool returnIP=false) const;

    Quantity getThermalAbsorptance(bool returnIP=false) const;

    bool isThermalAbsorptanceDefaulted() const;

    Quantity getSolarAbsorptance(bool returnIP=false) const;

    bool isSolarAbsorptanceDefaulted() const;

    Quantity getVisibleAbsorptance(bool returnIP=false) const;

    bool isVisibleAbsorptanceDefaulted() const;

    double conductivity() const;

    Quantity getConductivity(bool returnIP=false) const;

    double density() const;

    Quantity getDensity(bool returnIP=false) const;

    double specificHeat() const;

    Quantity getSpecificHeat(bool returnIP=false) const;

    //@}
    /** @name Setters */
    //@{

    bool setRoughness(std::string roughness);

    /** Set thickness to value (m). */
    virtual bool setThickness(double value) override;

    /** Sets the conductivity of the material in W/m*K, if possible. */
    virtual bool setThermalConductivity(double value) override;

    /** Sets the conductance of the material in W/m^2*K, if possible. */
    virtual bool setThermalConductance(double value) override;

    /** Sets the resistivity of the material in m*K/W, if possible. */
    virtual bool setThermalResistivity(double value) override;

    /** Sets the resistance of the material in m^2*K/W, if possible. */
    virtual bool setThermalResistance(double value) override;

    virtual bool setThermalAbsorptance(boost::optional<double> value);

    virtual bool setThermalReflectance(boost::optional<double> value) override;

    virtual bool setSolarAbsorptance(boost::optional<double> value) override;

    virtual bool setSolarReflectance(boost::optional<double> value) override;

    virtual bool setVisibleAbsorptance(boost::optional<double> value) override;

    virtual bool setVisibleReflectance(boost::optional<double> value) override;

    bool setThermalResistance(const Quantity& thermalResistance);

    bool setThermalAbsorptance(double thermalAbsorptance) override;

    bool setThermalAbsorptance(const Quantity& thermalAbsorptance);

    void resetThermalAbsorptance();

    bool setSolarAbsorptance(double solarAbsorptance);

    bool setSolarAbsorptance(const Quantity& solarAbsorptance);

    void resetSolarAbsorptance();

    bool setVisibleAbsorptance(double visibleAbsorptance);

    bool setVisibleAbsorptance(const Quantity& visibleAbsorptance);

    void resetVisibleAbsorptance();

    bool setConductivity(double conductivity);

    bool setConductivity(const Quantity& conductivity);

    bool setDensity(double density);

    bool setDensity(const Quantity& density);

    bool setSpecificHeat(double specificHeat);

    bool setSpecificHeat(const Quantity& specificHeat);

    //@}
    /** @name Other */
    //@{

    //@}
   protected:
   private:
    REGISTER_LOGGER("openstudio.model.MasslessOpaqueMaterial");

    std::vector<std::string> roughnessValues() const;
    openstudio::Quantity thermalResistance_SI() const;
    openstudio::Quantity thermalResistance_IP() const;
    openstudio::Quantity thermalAbsorptance_SI() const;
    openstudio::Quantity thermalAbsorptance_IP() const;
    openstudio::Quantity solarAbsorptance_SI() const;
    openstudio::Quantity solarAbsorptance_IP() const;
    openstudio::Quantity visibleAbsorptance_SI() const;
    openstudio::Quantity visibleAbsorptance_IP() const;
    openstudio::Quantity conductivity_SI() const;
    openstudio::Quantity conductivity_IP() const;
    openstudio::Quantity density_SI() const;
    openstudio::Quantity density_IP() const;
    openstudio::Quantity specificHeat_SI() const;
    openstudio::Quantity specificHeat_IP() const;

  };

} // detail

} // model
} // openstudio

#endif // MODEL_MASSLESSOPAQUEMATERIAL_IMPL_HPP

