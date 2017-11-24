/***********************************************************************************************************************
 *  OpenStudio(R), Copyright (c) 2008-2017, Alliance for Sustainable Energy, LLC. All rights reserved.
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

#ifndef MODEL_SCHEDULETYPELIMITS_IMPL_HPP
#define MODEL_SCHEDULETYPELIMITS_IMPL_HPP

#include "ModelAPI.hpp"
#include "ResourceObject_Impl.hpp"

#include "../utilities/units/Quantity.hpp"
#include "../utilities/units/OSOptionalQuantity.hpp"

namespace openstudio {
namespace model {

namespace detail {

  /** ScheduleTypeLimits_Impl is a ResourceObject_Impl that is the implementation class for ScheduleTypeLimits.*/
  class MODEL_API ScheduleTypeLimits_Impl : public ResourceObject_Impl {

















    // TODO: Add relationships for objects related to this one, but not pointed to by the underlying data.
    //       Such relationships can be generated by the GenerateRelationships.rb script.
   public:
    /** @name Constructors and Destructors */
    //@{

    ScheduleTypeLimits_Impl(const IdfObject& idfObject,
                            Model_Impl* model,
                            bool keepHandle);

    ScheduleTypeLimits_Impl(const openstudio::detail::WorkspaceObject_Impl& other,
                            Model_Impl* model,
                            bool keepHandle);

    ScheduleTypeLimits_Impl(const ScheduleTypeLimits_Impl& other,
                            Model_Impl* model,
                            bool keepHandle);

    virtual ~ScheduleTypeLimits_Impl() {}

    //@}

    /** @name Virtual Methods */
    //@{

    virtual const std::vector<std::string>& outputVariableNames() const override;

    virtual IddObjectType iddObjectType() const override;

    //@}
    /** @name Getters */
    //@{

    boost::optional<double> lowerLimitValue() const;

    OSOptionalQuantity getLowerLimitValue(bool returnIP=false) const;

    boost::optional<double> upperLimitValue() const;

    OSOptionalQuantity getUpperLimitValue(bool returnIP=false) const;

    boost::optional<std::string> numericType() const;

    std::string unitType() const;

    bool isUnitTypeDefaulted() const;

    boost::optional<Unit> units(bool returnIP=false) const;

    //@}
    /** @name Setters */
    //@{

    bool setLowerLimitValue(boost::optional<double> lowerLimitValue);

    bool setLowerLimitValue(const OSOptionalQuantity& lowerLimitValue);

    void resetLowerLimitValue();

    bool setUpperLimitValue(boost::optional<double> upperLimitValue);

    bool setUpperLimitValue(const OSOptionalQuantity& upperLimitValue);

    void resetUpperLimitValue();

    bool setNumericType(boost::optional<std::string> numericType);

    void resetNumericType();

    bool setUnitType(std::string unitType);

    void resetUnitType();

    //@}
    /** @name Other */
    //@{

    //@}
   protected:
    virtual OSOptionalQuantity getQuantityFromDouble(unsigned index,
                                                     boost::optional<double> value,
                                                     bool returnIP) const override;

    virtual boost::optional<double> getDoubleFromQuantity(unsigned index, Quantity q) const;

   private:
    REGISTER_LOGGER("openstudio.model.ScheduleTypeLimits");

    openstudio::OSOptionalQuantity lowerLimitValue_SI() const;
    openstudio::OSOptionalQuantity lowerLimitValue_IP() const;
    openstudio::OSOptionalQuantity upperLimitValue_SI() const;
    openstudio::OSOptionalQuantity upperLimitValue_IP() const;
    std::vector<std::string> numericTypeValues() const;
    std::vector<std::string> unitTypeValues() const;
  };

} // detail

} // model
} // openstudio

#endif // MODEL_SCHEDULETYPELIMITS_IMPL_HPP
