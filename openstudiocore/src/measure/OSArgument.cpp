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

#include "OSArgument.hpp"

#include "../utilities/idf/WorkspaceObject.hpp"

#include "../utilities/idd/IddObject.hpp"

#include "../utilities/units/QuantityFactory.hpp"

#include "../utilities/core/Assert.hpp"
#include "../utilities/core/Containers.hpp"
#include "../utilities/core/Compare.hpp"
#include "../utilities/core/QJson.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/functional/value_factory.hpp>

#include <sstream>

namespace openstudio {
namespace measure {

OSArgument OSArgument::clone() const {
  OSArgument result(*this);
  result.m_uuid = createUUID();
  result.m_versionUUID = createUUID();
  return result;
}

OSArgument OSArgument::makeBoolArgument(const std::string& name, bool required, bool modelDependent)
{
  OSArgument result(name, OSArgumentType::Boolean, required, modelDependent);
  result.setDomainType(OSDomainType::Enumeration);
  result.m_choices.push_back("true");
  result.m_choices.push_back("false");
  return result;
}

OSArgument OSArgument::makeDoubleArgument(const std::string& name, bool required, bool modelDependent)
{
  OSArgument result(name, OSArgumentType::Double, required, modelDependent);
  result.setDomainType(OSDomainType::Interval);
  return result;
}

// TODO: JM 2018-11-28 Remove?
//OSArgument OSArgument::makeQuantityArgument(const std::string& name, bool required, bool modelDependent)
//{
  //OSArgument result(name, OSArgumentType::Quantity, required, modelDependent);
  //result.setDomainType(OSDomainType::Interval);
  //return result;
//}

OSArgument OSArgument::makeIntegerArgument(const std::string& name, bool required, bool modelDependent)
{
  OSArgument result(name, OSArgumentType::Integer, required, modelDependent);
  result.setDomainType(OSDomainType::Interval);
  return result;
}

OSArgument OSArgument::makeStringArgument(const std::string& name, bool required, bool modelDependent)
{
  OSArgument result(name, OSArgumentType::String, required, modelDependent);
  result.setDomainType(OSDomainType::Enumeration);
  return result;
}

OSArgument OSArgument::makeChoiceArgument(const std::string& name,
                                          const std::vector<std::string>& choices,
                                          bool required, bool modelDependent)
{
  return makeChoiceArgument(name, choices, choices, required, modelDependent);
}

OSArgument OSArgument::makeChoiceArgument(const std::string& name,
                                          const std::vector<std::string>& choices,
                                          const std::vector<std::string>& displayNames,
                                          bool required, bool modelDependent)
{
  OSArgument result(name, OSArgumentType::Choice, required, modelDependent);
  result.setDomainType(OSDomainType::Enumeration);
  result.m_choices = choices;
  result.m_choiceDisplayNames = displayNames;
  return result;
}

OSArgument OSArgument::makePathArgument(const std::string& name,
                                        bool isRead,
                                        const std::string& extension,
                                        bool required, bool modelDependent)
{
  OSArgument result(name, OSArgumentType::Path, required, modelDependent);
  result.setDomainType(OSDomainType::Enumeration);
  result.m_isRead = isRead;
  result.m_extension = extension;
  return result;
}

OSArgument OSArgument::makeSeparatorArgument(const std::string& name, bool modelDependent)
{
  OSArgument result(name, OSArgumentType::Separator, false, modelDependent);
  return result;
}

//openstudio::UUID OSArgument::uuid() const {
//  return m_uuid;
//}

//openstudio::UUID OSArgument::versionUUID() const {
//  return m_versionUUID;
//}

std::string OSArgument::name() const {
  return m_name;
}

std::string OSArgument::displayName() const {
  return m_displayName;
}

boost::optional<std::string> OSArgument::description() const {
  return m_description;
}

OSArgumentType OSArgument::type() const {
  return m_type;
}

boost::optional<std::string> OSArgument::units() const {
  return m_units;
}

bool OSArgument::required() const {
  return m_required;
}

bool OSArgument::modelDependent() const {
  return m_modelDependent;
}

bool OSArgument::hasValue() const {
  // If index is 0, then its std::monostate (=empty)
  return (m_value.index() != 0);
}

bool OSArgument::valueAsBool() const
{
  if (!hasValue()) {
    LOG_AND_THROW("This argument does not have a value set.")
  }
  if (type() != OSArgumentType::Boolean) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Bool.");
  }

  return std::get<bool>(m_value);
}

double OSArgument::valueAsDouble() const
{
  if (!hasValue()) {
    LOG_AND_THROW("This argument does not have a value set.")
  }
  if (type() != OSArgumentType::Double) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Double.");
  }

  return std::get<double>(m_value);
}

// TODO: JM 2018-11-28 comment
//Quantity OSArgument::valueAsQuantity() const
//{
  //if (!hasValue()) {
    //LOG_AND_THROW("This argument does not have a value set.")
  //}
  //if (type() != OSArgumentType::Quantity) {
    //LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Quantity.");
  //}
  //return m_value.value<openstudio::Quantity>();
//}

int OSArgument::valueAsInteger() const
{
  if (!hasValue()) {
    LOG_AND_THROW("This argument does not have a value set.")
  }
  if (type() != OSArgumentType::Integer) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Integer.");
  }

  return std::get<int>(m_value);
}

std::string OSArgument::valueAsString() const
{
  if (!hasValue()) {
    LOG_AND_THROW("Argument " << name() << " has no value.");
  }

  return printOSArgumentVariant(m_value);
}

openstudio::path OSArgument::valueAsPath() const
{
  if (!hasValue()) {
    LOG_AND_THROW("This argument does not have a value set.")
  }
  if (type() != OSArgumentType::Path) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Path.");
  }

  return std::get<openstudio::path>(m_value);
}

// TODO: JM 2018-11-28: remove definitely?
//QVariant OSArgument::valueAsQVariant() const {
//  return m_value;
//}

bool OSArgument::hasDefaultValue() const {
  return (m_defaultValue.index() != 0);
}

bool OSArgument::defaultValueAsBool() const
{
  if (!hasDefaultValue()) {
    LOG_AND_THROW("This argument does not have a default value set.")
  }
  if (type() != OSArgumentType::Boolean) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Bool.");
  }

  return std::get<bool>(m_defaultValue);
}

double OSArgument::defaultValueAsDouble() const
{
  if (!hasDefaultValue()) {
    LOG_AND_THROW("This argument does not have a default value set.")
  }
  if (type() != OSArgumentType::Double) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Double.");
  }

  return std::get<double>(m_defaultValue);
}

//Quantity OSArgument::defaultValueAsQuantity() const
//{
  //if (!hasDefaultValue()) {
    //LOG_AND_THROW("This argument does not have a default value set.")
  //}
  //if (type() != OSArgumentType::Quantity) {
    //LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Quantity.");
  //}
  //return std::get<Quantity>(m_value);
//}

int OSArgument::defaultValueAsInteger() const
{
  if (!hasDefaultValue()) {
    LOG_AND_THROW("This argument does not have a default value set.")
  }
  if (type() != OSArgumentType::Integer) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Integer.");
  }

  return std::get<int>(m_defaultValue);
}

std::string OSArgument::defaultValueAsString() const
{
  if (!hasDefaultValue()) {
    LOG_AND_THROW("Argument " << name() << " does not have a default value.");
  }

  return printOSArgumentVariant(m_defaultValue);
}

openstudio::path OSArgument::defaultValueAsPath() const
{
  if (!hasDefaultValue()) {
    LOG_AND_THROW("This argument does not have a default value set.")
  }
  if (type() != OSArgumentType::Path) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Path.");
  }

  return std::get<openstudio::path>(m_defaultValue);
}

// TODO: JM 2018-11-28 remove
//QVariant OSArgument::defaultValueAsQVariant() const {
//  return m_defaultValue;
//}

bool OSArgument::hasDomain() const {
  return !m_domain.empty();
}

OSDomainType OSArgument::domainType() const {
  return m_domainType;
}

std::vector<bool> OSArgument::domainAsBool() const {
  if (!hasDomain()) {
    LOG_AND_THROW("No domain set for OSArgument '" << name() << "'.");
  }
  if (type() != OSArgumentType::Boolean) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Bool.");
  }

  std::vector<bool> result;

  for (const OSArgumentVariant& value: m_domain) {
    result.push_back(std::get<bool>(value));
  }
  return result;
}

std::vector<double> OSArgument::domainAsDouble() const {
  if (!hasDomain()) {
    LOG_AND_THROW("No domain set for OSArgument '" << name() << "'.");
  }
  if (type() != OSArgumentType::Double) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Double.");
  }

  std::vector<double> result;

  for (const OSArgumentVariant& value: m_domain) {
    result.push_back(std::get<double>(value));
  }
  return result;
}

// TODO: JM 2018-11-28 comment
//std::vector<Quantity> OSArgument::domainAsQuantity() const {
  //if (!hasDomain()) {
    //LOG_AND_THROW("No domain set for OSArgument '" << name() << "'.");
  //}
  //std::vector<Quantity> result;

  //for (const OSArgumentVariant& value: m_domain) {
    //result.push_back(std::get<Quantity>(value));
  //}
  //return result;
//}

std::vector<int> OSArgument::domainAsInteger() const {
  if (!hasDomain()) {
    LOG_AND_THROW("No domain set for OSArgument '" << name() << "'.");
  }
  if (type() != OSArgumentType::Integer) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Integer.");
  }

  std::vector<int> result;

  for (const OSArgumentVariant& value: m_domain) {
    result.push_back(std::get<int>(value));
  }
  return result;
}

std::vector<openstudio::path> OSArgument::domainAsPath() const {
  if (!hasDomain()) {
    LOG_AND_THROW("No domain set for OSArgument '" << name() << "'.");
  }
  if (type() != OSArgumentType::Path) {
    LOG_AND_THROW("This argument is of type " << type().valueName() << ", not of type Path.");
  }

  std::vector<openstudio::path> result;

  for (const OSArgumentVariant& value: m_domain) {
    result.push_back(std::get<openstudio::path>(value));
  }
  return result;
}

// TODO: should work with
std::vector<std::string> OSArgument::domainAsString() const {
  if (!hasDomain()) {
    LOG_AND_THROW("No domain set for OSArgument '" << name() << "'.");
  }

  // TODO: add check for arg type?

  std::vector<std::string> result;

  for (const OSArgumentVariant& value: m_domain) {
    result.push_back(printOSArgumentVariant(value));
  }
  return result;
}

//std::vector<QVariant> OSArgument::domainAsQVariant() const {
  //if (!hasDomain()) {
    //LOG_AND_THROW("No domain set for OSArgument '" << name() << "'.");
  //}
  //return m_domain;
//}


std::vector<std::string> OSArgument::choiceValues() const {
  return m_choices;
}

std::vector<std::string> OSArgument::choiceValueDisplayNames() const {
  return m_choiceDisplayNames;
}

std::string OSArgument::valueDisplayName() const {
  std::string valueString = valueAsString();
  int index = int(std::find(m_choices.begin(),m_choices.end(),valueString) -
                  m_choices.begin());
  if (index < int(m_choiceDisplayNames.size())) {
    return m_choiceDisplayNames[index];
  }
  return valueString;
}

std::string OSArgument::defaultValueDisplayName() const {
  std::string defaultValueString = defaultValueAsString();
  int index = int(std::find(m_choices.begin(),m_choices.end(),defaultValueString) -
                  m_choices.begin());
  if (index < int(m_choiceDisplayNames.size())) {
    return m_choiceDisplayNames[index];
  }
  return defaultValueString;
}


bool OSArgument::isRead() const {
  return m_isRead;
}

std::string OSArgument::extension() const {
  return m_extension;
}

void OSArgument::setDisplayName(const std::string& displayName) {
  m_displayName = displayName;
  onChange();
}

void OSArgument::setDescription(const std::string& description)
{
  m_description = description;
  onChange();
}

void OSArgument::setUnits(const std::string& units)
{
  m_units = units;
  onChange();
}

bool OSArgument::setValue(bool value) {
  bool result = false;
  if (m_type == OSArgumentType::Boolean) {
    m_value = value;
    OS_ASSERT(hasValue());
    onChange();
    result = true;
  }
  return result;
}

bool OSArgument::setValue(double value) {
  bool result = false;
  if (m_type == OSArgumentType::Double) {
    m_value = value;
    OS_ASSERT(hasValue());
    onChange();
    result = true;

  } else if (m_type == OSArgumentType::Integer) {
    // Let a double be assigned to an int, only if said double is really an integer expressed as a double
    int test = floor(value);
    if (test == value){
      result = setValue(test);
    } else {
      result = false;
    }
  }

  return result;
}

//bool OSArgument::setValue(const Quantity& value) {
  //bool result = false;
  //if (m_type == OSArgumentType::Quantity) {
    //m_value = value;
    //OS_ASSERT(hasValue());
    //onChange();
    //result = true;
  //}
  //return result;
//}

bool OSArgument::setValue(int value) {
  bool result = false;
  if (m_type == OSArgumentType::Integer) {
    m_value = value;
    result = true;

  } else if (m_type == OSArgumentType::Double) {
    // If we pass an int, but our type is double, we let it happen
    m_value.emplace<double>(value);
    result = true;
  }

  if (result) {
    OS_ASSERT(hasValue());
    onChange();
  }

  return result;
}

bool OSArgument::setValue(const openstudio::path& value) {
  bool result = false;
  if (m_type == OSArgumentType::Path) {
    m_value = value;
    OS_ASSERT(hasValue());
    onChange();
    result = true;
  }
  return result;
}

// TODO
bool OSArgument::setValue(const std::string& value) {
  bool result = setStringInternal(m_value, value);
  if (result) {
    OS_ASSERT(hasValue());
    onChange();
  }
  return result;
}

void OSArgument::clearValue() {
  m_value = OSArgumentVariant();
  OS_ASSERT(!hasValue());
  onChange();
}

bool OSArgument::setDefaultValue(bool defaultValue) {
  bool result = false;
  if (m_type == OSArgumentType::Boolean) {
    m_defaultValue = defaultValue;
    OS_ASSERT(hasDefaultValue());
    onChange();
    result = true;
  }
  return result;
}

bool OSArgument::setDefaultValue(double defaultValue) {
  bool result = false;
  if (m_type == OSArgumentType::Double){
    m_defaultValue = defaultValue;
    OS_ASSERT(hasDefaultValue());
    onChange();
    result = true;
  } else if (m_type == OSArgumentType::Integer) {
    // Let a double be assigned to an int, only if said double is really an integer expressed as a double
    int test = floor(defaultValue);
    if (test == defaultValue) {
      result = setDefaultValue(test);
    } else {
      result = false;
    }
  }
  return result;
}

//bool OSArgument::setDefaultValue(const Quantity& value) {
  //bool result = false;
  //if (m_type == OSArgumentType::Quantity) {
    //m_defaultValue = QVariant::fromValue<openstudio::Quantity>(value);
    //OS_ASSERT(hasDefaultValue());
    //onChange();
    //result = true;
  //}
  //return result;
//}

bool OSArgument::setDefaultValue(int defaultValue)
{
  bool result = false;
  if (m_type == OSArgumentType::Integer){
    m_defaultValue = defaultValue;
    result = true;

  } else if (m_type == OSArgumentType::Double) {
    // If we pass an int, but our type is double, we let it happen
    m_defaultValue.emplace<double>(defaultValue);
    result =true;
  }

  if (result) {
    OS_ASSERT(hasDefaultValue());
    onChange();
  }

  return result;
}

bool OSArgument::setDefaultValue(const openstudio::path& defaultValue) {
  bool result = false;
    if (m_type == OSArgumentType::Path) {
    m_defaultValue = defaultValue;
    OS_ASSERT(hasValue());
    onChange();
    result = true;
  }
  return result;
}

bool OSArgument::setDefaultValue(const std::string& defaultValue) {
  bool result = setStringInternal(m_defaultValue, defaultValue);
  if (result) {
    OS_ASSERT(hasValue());
    onChange();
  }
  return result;
}

bool OSArgument::setDomainType(const OSDomainType& domainType) {
  if (domainType != this->domainType()) {
    // check whether domainType makes sense for type()
    switch (domainType.value()) {
      case OSDomainType::Interval :
        switch (m_type.value()) {
          case OSArgumentType::Boolean :
          case OSArgumentType::String :
          case OSArgumentType::Choice :
          case OSArgumentType::Path :
            LOG(Info,"OSDomainType " << domainType.valueName() << " does not make sense for "
                << "arguments of type " << m_type.valueName() << ", such as " << name() << ".");
            return false;
          default:
           break;
        }
        break;
      case OSDomainType::Enumeration :
        break;
      default :
        OS_ASSERT(false);
    }

    // if so, change and clear current domain
    m_domainType = domainType;
    m_domain.clear();
    onChange();
  }
  return true;
}

bool OSArgument::setDomain(const std::vector<bool>& domain) {
  bool result(false);
  if (m_type == OSArgumentType::Boolean) {
    OS_ASSERT(m_domainType == OSDomainType::Enumeration);
    // could check for uniqueness, but pass on that for now
    m_domain.clear();
    for (bool value : domain) {
      m_domain.push_back(OSArgumentVariant(value));
    }
    onChange();
    result = true;
  }
  return result;
}

bool OSArgument::setDomain(const std::vector<double>& domain) {
  bool result(false);
  if (m_type == OSArgumentType::Double) {
    if ((m_domainType != OSDomainType::Interval) || (domain.size() == 2u)) {
      // could check for uniqueness, min < max, but pass on that for now
      m_domain.clear();
      for (double value : domain) {
        m_domain.push_back(OSArgumentVariant(value));
      }
      onChange();
      result = true;
    }
  }
  return result;
}

//bool OSArgument::setDomain(const std::vector<Quantity>& domain) {
  //bool result(false);
  //if (m_type == OSArgumentType::Quantity) {
    //if ((m_domainType != OSDomainType::Interval) || (domain.size() == 2u)) {
      //// could check for uniqueness, min < max, but pass on that for now
      //m_domain.clear();
      //for (const Quantity& value : domain) {
        //m_domain.push_back(QVariant::fromValue<openstudio::Quantity>(value));
      //}
      //onChange();
      //result = true;
    //}
  //}
  //return result;
//}

bool OSArgument::setDomain(const std::vector<int>& domain) {
  bool result(false);
  if (m_type == OSArgumentType::Integer) {
    if ((m_domainType != OSDomainType::Interval) || (domain.size() == 2u)) {
      // could check for uniqueness, min < max, but pass on that for now
      m_domain.clear();
      for (int value : domain) {
        m_domain.push_back(OSArgumentVariant(value));
      }
      onChange();
      result = true;
    }
  } else  if (m_type == OSArgumentType::Double) {
    if ((m_domainType != OSDomainType::Interval) || (domain.size() == 2u)) {
      // could check for uniqueness, min < max, but pass on that for now
      m_domain.clear();
      for (int value : domain) {
        m_domain.push_back(OSArgumentVariant(double(value)));
      }
      onChange();
      result = true;
    }
  }
  return result;
}

bool OSArgument::setDomain(const std::vector<openstudio::path>& domain) {
  bool result(false);
  if (m_type == OSArgumentType::Path) {
    OS_ASSERT(m_domainType == OSDomainType::Enumeration);
    // could check for uniqueness, but pass on that for now
    m_domain.clear();
    for (const openstudio::path& value : domain) {
      m_domain.push_back(OSArgumentVariant(value));
    }
    onChange();
    result = true;
  }
  return result;
}

bool OSArgument::setDomain(const std::vector<std::string>& domain) {
  bool result(false);
  if ((m_domainType != OSDomainType::Interval) || (domain.size() == 2u)) {
    // Store the original, in case we fail to set one element
    std::vector<OSArgumentVariant> originalDomain = m_domain;
    m_domain.clear();
    for (const std::string& value : domain) {
      OSArgumentVariant newValue;
      result = setStringInternal(newValue, value);
      if (!result) {
        // Restore the original domain
        m_domain = originalDomain;
        break;
      }
      m_domain.push_back(newValue);
    }
    if (result) {
      onChange();
    }
  }
  return result;
}

void OSArgument::clearDomain() {
  m_domain.clear();
}

bool OSArgument::setMinValue(double minValue)
{
  if (m_type == OSArgumentType::Integer){
    int test = floor(minValue);
    if (test == minValue) {
      // If int expressed as double (eg: 1.0 when type = int)
      // Then we call the int overload instead
      return setMinValue(test);
    }
    return false;
  } else if (m_type != OSArgumentType::Double) {
    return false;
  }

  double maxValue = std::numeric_limits<double>::max();
  // Try to fetch the existing maxValue
  if (hasDomain() && (m_domainType == OSDomainType::Interval)){
    std::vector<double> domain = domainAsDouble();
    if (domain.size() == 2){
      maxValue = domain[1];
    }
  }

  m_domainType = OSDomainType::Interval;
  m_domain.clear();
  m_domain.push_back(OSArgumentVariant(minValue));
  m_domain.push_back(OSArgumentVariant(maxValue));

  onChange();

  return true;
}

bool OSArgument::setMinValue(int minValue)
{
  if (m_type == OSArgumentType::Double) {
    double test = (double)minValue;
    return setMinValue(test);
  } else if (m_type != OSArgumentType::Integer) {
    return false;
  }

  int maxValue = std::numeric_limits<int>::max();
  if (hasDomain() && (m_domainType == OSDomainType::Interval)){
    std::vector<int> domain = domainAsInteger();
    if (domain.size() == 2){
      maxValue = domain[1];
    }
  }

  m_domainType = OSDomainType::Interval;
  m_domain.clear();
  m_domain.push_back(OSArgumentVariant(minValue));
  m_domain.push_back(OSArgumentVariant(maxValue));

  onChange();

  return true;
}

bool OSArgument::setMaxValue(double maxValue)
{
  if (m_type == OSArgumentType::Integer){
    int test = floor(maxValue);
    if (test == maxValue){
      return setMaxValue(test);
    }
    return false;
  }
  else if (m_type != OSArgumentType::Double) {
    return false;
  }

  double minValue = std::numeric_limits<double>::min();
  if (hasDomain() && (m_domainType == OSDomainType::Interval)){
    std::vector<double> domain = domainAsDouble();
    if (domain.size() == 2){
      minValue = domain[0];
    }
  }

  m_domainType = OSDomainType::Interval;
  m_domain.clear();
  m_domain.push_back(OSArgumentVariant(minValue));
  m_domain.push_back(OSArgumentVariant(maxValue));

  onChange();

  return true;
}

bool OSArgument::setMaxValue(int maxValue)
{
  if (m_type == OSArgumentType::Double) {
    double test = (double)maxValue;
    return setMaxValue(test);
  }
  else if (m_type != OSArgumentType::Integer) {
    return false;
  }

  int minValue = std::numeric_limits<int>::min();
  if (hasDomain() && (m_domainType == OSDomainType::Interval)){
    std::vector<int> domain = domainAsInteger();
    if (domain.size() == 2){
      minValue = domain[0];
    }
  }

  m_domainType = OSDomainType::Interval;
  m_domain.clear();
  m_domain.push_back(OSArgumentVariant(minValue));
  m_domain.push_back(OSArgumentVariant(maxValue));

  onChange();

  return true;
}

std::string OSArgument::print() const {
  std::stringstream ss;

  // name
  ss << name();
  if (!displayName().empty()) {
    ss << " (" << displayName() << ")";
  }
  ss << std::endl;

  // type and required
  ss << type().valueName() << ", ";
  if (required()) {
    ss << "Required";
  }
  else {
    ss << "Optional";
  }
  ss << std::endl;

  // value
  ss << "Value: ";
  if (hasValue()) {
    ss << printValue(false) << " ";
  }
  if (hasDefaultValue()) {
    ss << "(" << printDefaultValue() << ")";
  }
  ss << std::endl;

  if (m_type.value() == OSArgumentType::Choice) {
    ss << "Choices:" << std::endl;
    int dnn = m_choiceDisplayNames.size();
    for (int i = 0, n = m_choices.size(); i < n; ++i) {
      ss << "  " << m_choices[i];
      if ((i < dnn) && (!m_choiceDisplayNames[i].empty())) {
        ss << " (" << m_choiceDisplayNames[i] << ")";
      }
      ss << std::endl;
    }
  }

  if (hasDomain()) {
    ss << m_domainType.valueName() << " Domain: ";
    if (m_domainType == OSDomainType::Interval) {
      OS_ASSERT(m_domain.size() == 2u);
      ss << "[" << printOSArgumentVariant(m_domain[0]) << ", " << printOSArgumentVariant(m_domain[1]) << "]" << std::endl;
    }
    else {
      ss << std::endl;
      for (const OSArgumentVariant& value : m_domain) {
        ss << "  " << printOSArgumentVariant(value) << std::endl;
      }
    }
  }

  return ss.str();

}

std::string OSArgument::printValue(bool printDefault) const {
  std::string result;

  OSArgumentVariant toPrint;
  if (hasValue()) {
    toPrint = m_value;
  }
  else if (printDefault && hasDefaultValue()) {
    toPrint = m_defaultValue;
  }

  // If not std::monostate (= empty)
  if (toPrint.index() != 0) {
    result = printOSArgumentVariant(toPrint);
  }

  return result;
}

std::string OSArgument::printDefaultValue() const {
  std::string result;

  if (hasDefaultValue()) {
    result = printOSArgumentVariant(m_defaultValue);
  }

  return result;
}

OSArgument::OSArgument()
  : m_uuid(createUUID()), m_versionUUID(createUUID())
{}

OSArgument::OSArgument(const std::string& name,
                       const OSArgumentType& type,
                       bool required, bool modelDependent)
  : m_uuid(createUUID()),
    m_versionUUID(createUUID()),
    m_name(name),
    m_displayName(name),
    m_type(type),
    m_required(required),
    m_modelDependent(modelDependent),
    m_isRead(false)
{}

bool OSArgument::setStringInternal(OSArgumentVariant& variant, const std::string& value) {
  bool result = false;

  if (m_type == OSArgumentType::Boolean) {
    if (value == "true") {
      variant = true;
      result = true;
    }
    else if (value == "false") {
      variant = false;
      result = true;
    }
  } else if (m_type == OSArgumentType::Double) {
    try {
      auto const double_val = std::stod(value, nullptr);
      variant = double_val;
      result = true;
    } catch (std::exception e) { }

  /*
   *} else if (m_type == OSArgumentType::Quantity) {
   *  OptionalQuantity oq = createQuantity(value);
   *  if (oq) {
   *    variant = QVariant::fromValue<openstudio::Quantity>(*oq);
   *    result = true;
   *  }
   */
  } else if (m_type == OSArgumentType::Integer) {
    try {
      auto const int_val = std::stoi(value, nullptr);
      variant = int_val;
      result = true;
    } catch (std::exception e) { }
  } else if (m_type == OSArgumentType::String) {
    variant = value;
    result = true;
  } else if (m_type == OSArgumentType::Choice) {
    if (std::find(m_choices.begin(), m_choices.end(), value) != m_choices.end()){
      variant = value;
      result = true;
    } else {
      // can also set using display name
      StringVector::const_iterator it = std::find(m_choiceDisplayNames.begin(), m_choiceDisplayNames.end(), value);
      if (it != m_choiceDisplayNames.end()) {
        int index = int(it - m_choiceDisplayNames.begin());
        if (index < int(m_choices.size())) {
          variant = m_choices[index];
          result = true;
        }
      }
    }
  } else if (m_type == OSArgumentType::Path) {
    openstudio::path temp = toPath(value);
      variant = temp;
      result = true;
  }

  return result;
}

std::string OSArgument::printOSArgumentVariant(const OSArgumentVariant& toPrint) const {
  OS_ASSERT(toPrint.index() != 0);

  std::stringstream ss;
  // We use std::visit, filtering out the case where it's monostate
  // Aside from monostate, every possible type is streamable
  std::visit(
      [&ss](const auto& val){
        //Needed to properly compare the types
      using T = std::remove_cv_t<std::remove_reference_t<decltype(val)>>;
        if constexpr (!std::is_same_v<T, std::monostate>) {
          ss << val;
        }
      },
      toPrint);

  return ss.str();
}

void OSArgument::onChange() {
  m_versionUUID = createUUID();
}

std::ostream& operator<<(std::ostream& os, const OSArgument& arg) {
  os << arg.print();
  return os;
}

OSArgument makeChoiceArgumentOfWorkspaceObjects(const std::string& name,
                                                const IddObjectType& iddObjectType,
                                                const Workspace& workspace,
                                                bool required)
{
  std::vector< std::pair<std::string, std::string> > intermediate;

  std::vector<WorkspaceObject> objects = workspace.getObjectsByType(iddObjectType);
  for (const WorkspaceObject& object : objects){
    std::string objectName;
    if (object.name()) {
      objectName = object.name().get();
    }
    else {
      objectName = object.iddObject().type().valueName();
    }
    intermediate.push_back(std::pair<std::string,std::string>(toString(object.handle()),
                                                              objectName));
  }

  std::sort(intermediate.begin(),
            intermediate.end(),
            SecondOfPairLess< std::pair<std::string,std::string> >());

  int n = intermediate.size();
  StringVector choices(n), displayNames(n);
  for (int i = 0; i < n; ++i) {
    choices[i] = intermediate[i].first;
    displayNames[i] = intermediate[i].second;
  }

  return OSArgument::makeChoiceArgument(name,
                                        choices,
                                        displayNames,
                                        required);
}

OSArgument makeChoiceArgumentOfWorkspaceObjects(const std::string& name,
                                                const std::string& referenceName,
                                                const Workspace& workspace,
                                                bool required)
{
  std::vector< std::pair<std::string, std::string> > intermediate;

  std::vector<WorkspaceObject> objects = workspace.getObjectsByReference(referenceName);
  for (const WorkspaceObject& object : objects){
    intermediate.push_back(std::pair<std::string,std::string>(toString(object.handle()),
                                                              object.name().get()));
  }

  std::sort(intermediate.begin(),
            intermediate.end(),
            SecondOfPairLess< std::pair<std::string,std::string> >());

  int n = intermediate.size();
  StringVector choices(n), displayNames(n);
  for (int i = 0; i < n; ++i) {
    choices[i] = intermediate[i].first;
    displayNames[i] = intermediate[i].second;
  }

  return OSArgument::makeChoiceArgument(name,
                                        choices,
                                        displayNames,
                                        required);
}

std::map<std::string,OSArgument> convertOSArgumentVectorToMap(const std::vector<OSArgument>& arguments)
{
  std::map<std::string, OSArgument> argMap;
  for (const OSArgument& arg : arguments) {
    argMap.insert(std::make_pair(arg.name(), arg.clone()));
  }
  return argMap;
}

namespace detail {

  //QVariant toVariant(const OSArgument& argument) {
    //QVariantMap argumentData;

    //argumentData["uuid"] = toQString(removeBraces(argument.uuid()));
    //argumentData["version_uuid"] = toQString(removeBraces(argument.versionUUID()));
    //argumentData["name"] = toQString(argument.name());
    //if (!argument.displayName().empty()) {
      //argumentData["display_name"] = toQString(argument.displayName());
    //}
    //if (argument.description() && !argument.description()->empty()) {
      //argumentData["description"] = toQString(argument.description().get());
    //}
    //OSArgumentType type = argument.type();
    //if (argument.units() && !argument.units()->empty()) {
      //argumentData["units"] = toQString(argument.units().get());
    //}
    //argumentData["type"] = toQString(type.valueName());
    //argumentData["required"] = argument.required();
    //argumentData["model_dependent"] = argument.modelDependent();
    //if (argument.hasValue()) {
      //if (type == OSArgumentType::Quantity) {
        //Quantity value = argument.valueAsQuantity();
        //argumentData["value"] = value.value();
        //argumentData["value_units"] = toQString(value.units().standardString());
      //}
      //else {
        //// use QVariant directly
        //argumentData["value"] = argument.valueAsQVariant();
      //}
    //}
    //if (argument.hasDefaultValue()) {
      //if (type == OSArgumentType::Quantity) {
        //Quantity defaultValue = argument.defaultValueAsQuantity();
        //argumentData["default_value"] = defaultValue.value();
        //argumentData["default_value_units"] = toQString(defaultValue.units().standardString());
      //}
      //else {
        //// use QVariant directly
        //argumentData["default_value"] = argument.defaultValueAsQVariant();
      //}
    //}
    //argumentData["domain_type"] = toQString(argument.domainType().valueName());
    //if (argument.hasDomain()) {
      //QVariantList domainList;
      //int index(0);
      //for (const QVariant& dval : argument.domainAsQVariant()) {
        //QVariantMap domainValueMap;
        //domainValueMap["domain_value_index"] = index;
        //if (type == OSArgumentType::Quantity) {
          //Quantity q = dval.value<openstudio::Quantity>();
          //domainValueMap["value"] = q.value();
          //domainValueMap["units"] = toQString(q.units().standardString());
        //}
        //else {
          //domainValueMap["value"] = dval;
        //}
        //domainList.push_back(domainValueMap);
        //++index;
      //}
      //argumentData["domain"] = domainList;
    //}
    //if (type == OSArgumentType::Choice) {
      //QVariantList choicesList;
      //StringVector displayNames = argument.choiceValueDisplayNames();
      //int index(0), displayNamesN(displayNames.size());
      //for (const std::string& choice : argument.choiceValues()) {
        //QVariantMap choiceMap;
        //choiceMap["choice_index"] = index;
        //choiceMap["value"] = toQString(choice);
        //if (index < displayNamesN) {
          //choiceMap["display_name"] = toQString(displayNames[index]);
        //}
        //choicesList.push_back(choiceMap);
        //++index;
      //}
      //argumentData["choices"] = QVariant(choicesList);
    //}
    //if (type == OSArgumentType::Path) {
      //argumentData["is_read"] = argument.isRead();
      //argumentData["extension"] = toQString(argument.extension());
    //}

    //return QVariant(argumentData);
  //}

  //OSArgument toOSArgument(const QVariant& variant, const VersionString& version) {
    //QVariantMap map = variant.toMap();

    //OSArgumentType type(map["type"].toString().toStdString());

    //QVariant value, defaultValue;
    //OS_ASSERT(value.isNull() && defaultValue.isNull());
    //if (map.contains("value")) {
      //if (type == OSArgumentType::Quantity) {
        //value = toQuantityQVariant(map,"value","value_units");
      //}
      //else {
        //value = map["value"];
      //}
    //}
    //if (map.contains("default_value")) {
      //if (type == OSArgumentType::Quantity) {
        //defaultValue = toQuantityQVariant(map,"default_value","default_value_units");
      //}
      //else {
        //defaultValue = map["default_value"];
      //}
    //}

    //std::vector<QVariant> domain;
    //if (map.contains("domain")) {
      //if (type == OSArgumentType::Quantity) {
        //domain = deserializeOrderedVector(
              //map["domain"].toList(),
              //"domain_value_index",
              //std::function<QVariant (QVariant*)>(std::bind(
                                                            //toQuantityQVariant,
                                                            //std::bind(&QVariant::toMap,std::placeholders::_1),
                                                            //"value",
                                                            //"units")));
      //}
      //else {
        //domain = deserializeOrderedVector(
              //map["domain"].toList(),
              //"value",
              //"domain_value_index",
              //std::function<QVariant (const QVariant&)>(std::bind(boost::value_factory<QVariant>(),std::placeholders::_1)));
      //}
    //}

    //StringVector choices, choiceDisplayNames;
    //if (map.contains("choices")) {
      //QVariantList choicesList = map["choices"].toList();
      //choices = deserializeOrderedVector(
            //choicesList,
            //"value",
            //"choice_index",
            //std::function<std::string (QVariant*)>(std::bind(&QString::toStdString,
                                                                 //std::bind(&QVariant::toString,std::placeholders::_1))));
      //if (!choicesList.empty() && choicesList[0].toMap().contains("display_name")) {
        //try {
          //choiceDisplayNames = deserializeOrderedVector(
                //choicesList,
                //"display_name",
                //"choice_index",
                //std::function<std::string (QVariant*)>(std::bind(&QString::toStdString,
                                                                     //std::bind(&QVariant::toString,std::placeholders::_1))));
        //}
        //catch (...) {
          //LOG_FREE(Warn,"openstudio.measure.OSArgument","Unable to deserialize partial list of choice display names.");
        //}
      //}
    //}

    //return OSArgument(toUUID(map["uuid"].toString().toStdString()),
                      //toUUID(map["version_uuid"].toString().toStdString()),
                      //map["name"].toString().toStdString(),
                      //map.contains("display_name") ? map["display_name"].toString().toStdString() : std::string(),
                      //map.contains("description") ? map["description"].toString().toStdString() : boost::optional<std::string>(),
                      //type,
                      //map.contains("units") ? map["units"].toString().toStdString() : boost::optional<std::string>(),
                      //map["required"].toBool(),
                      //map["modelDependent"].toBool(),
                      //value,
                      //defaultValue,
                      //OSDomainType(map["domain_type"].toString().toStdString()),
                      //domain,
                      //choices,
                      //choiceDisplayNames,
                      //map.contains("is_read") ? map["is_read"].toBool() : false,
                      //map.contains("extension") ? map["extension"].toString().toStdString() : std::string());
  //}

  //QVariant toQuantityQVariant(const QVariantMap& map,
                              //const std::string& valueKey,
                              //const std::string& unitsKey)
  //{
    //Quantity q = createQuantity(map[toQString(valueKey)].toDouble(),map[toQString(unitsKey)].toString().toStdString()).get();
    //return QVariant::fromValue<openstudio::Quantity>(q);
  //}
} // detail

} // measure
} // openstudio
