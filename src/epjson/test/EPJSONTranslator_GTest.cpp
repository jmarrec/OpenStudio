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

#include <gtest/gtest.h>
#include "EPJSONFixture.hpp"
#include "../EPJSONTranslator.hpp"
#include "../../utilities/idf/IdfFile.hpp"
#include "../../utilities/core/ApplicationPathHelpers.hpp"
#include "../../utilities/core/PathHelpers.hpp"
#include <fmt/format.h>
#include <json/json.h>
#include <resources.hxx>



openstudio::path setupIdftoEPJSONTest(const openstudio::path &location)
{
  const auto basename = openstudio::toPath(openstudio::filesystem::basename(location));
  const auto working_directory = openstudio::filesystem::complete(openstudio::toPath("epjson_tests") / basename);
  const auto idf_path = working_directory / openstudio::toPath("eplus.idf");
  openstudio::filesystem::create_directories(working_directory);
  openstudio::filesystem::copy_file(location, idf_path, openstudio::filesystem::copy_option::overwrite_if_exists);
  return idf_path;
}

std::pair<bool, Json::Value> translateIdfToEPJSONWithEP(const openstudio::path& location) {

  std::system(fmt::format("{} --output-directory {} --convert-only {}", openstudio::getEnergyPlusExecutable().native(), location.parent_path().native(),
                          location.native())
                .c_str());

  Json::Value root;

  std::ifstream ifs;
  const auto epJSONFile = openstudio::setFileExtension(location, "epJSON", true);
  ifs.open(openstudio::toString(epJSONFile));
  Json::CharReaderBuilder builder;
  JSONCPP_STRING errs;

  bool success = parseFromStream(builder, ifs, &root, &errs);

  if (success) {
    const auto outputLocation = location.parent_path() / openstudio::toPath("eplus-rewritten.epJSON");
    std::ofstream ofs(openstudio::toString(outputLocation), std::ofstream::trunc);
    ofs << root.toStyledString() << '\n';
  }
  return {success, root};
}

std::pair<bool, Json::Value> translateIdfToEPJSONWithOS(const openstudio::path &location) {
  auto idf = openstudio::IdfFile::load(location);

  if (!idf) {
    return {false, Json::Value{}};
  }

  auto result = openstudio::EPJSON::toJSON(*idf);

  const auto outputLocation = location.parent_path() / openstudio::toPath("os.epJSON");
  std::ofstream ofs(openstudio::toString(outputLocation), std::ofstream::trunc);
  ofs << result.toStyledString() << '\n';
  return {true, result};
}

void makeDoubles(Json::Value &value) {
  if (value.isNumeric()) {
    value = value.asDouble();
  } else {
    for (auto &child : value) {
      makeDoubles(child);
    }
  }
}

bool equal(const Json::Value &lhs, const Json::Value &rhs)
{
  auto doubledLhs = lhs;
  makeDoubles(doubledLhs);

  auto doubledRhs = rhs;
  makeDoubles(doubledRhs);

  return doubledLhs == doubledRhs;
}

TEST_F(EPJSONFixture, TranslateIDFToEPJSON) {
  const auto idfToTest = openstudio::getEnergyPlusDirectory() / openstudio::toPath("ExampleFiles") / openstudio::toPath("RefBldgMediumOfficeNew2004_Chicago.idf");
  const auto setupIdf = setupIdftoEPJSONTest(idfToTest);

  const auto epTranslation = translateIdfToEPJSONWithEP(setupIdf);
  const auto osTranslation = translateIdfToEPJSONWithOS(setupIdf);

  ASSERT_TRUE(epTranslation.first);
  ASSERT_TRUE(osTranslation.first);

  const auto are_equal = equal(epTranslation.second, osTranslation.second);

  EXPECT_TRUE(are_equal);


}