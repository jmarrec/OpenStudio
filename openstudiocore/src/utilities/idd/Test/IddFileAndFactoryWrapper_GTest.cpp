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

#include <gtest/gtest.h>
#include "IddFixture.hpp"
#include "../IddFileAndFactoryWrapper.hpp"
#include <utilities/idd/IddEnums.hxx>

using namespace openstudio;

TEST_F(IddFixture,IddFileAndFactoryWrapper_ExplicitIddFile)
{
  IddFileAndFactoryWrapper wrapper(IddFactory::instance().getIddFile(IddFileType::OpenStudio));

  EXPECT_EQ(IddFactory::instance().getIddFile(IddFileType::OpenStudio).objects().size(),wrapper.iddFile().objects().size());

  // Even though OpenStudio IddFile, it has been disembodied from IddFactory, so IddFileType
  // has been lost.
  EXPECT_TRUE(wrapper.iddFileType() == IddFileType(IddFileType::UserCustom));

  EXPECT_TRUE(wrapper.requiredObjects().size() ==
              IddFactory::instance().getRequiredObjects(IddFileType::OpenStudio).size());
  EXPECT_TRUE(wrapper.uniqueObjects().size() ==
              IddFactory::instance().getUniqueObjects(IddFileType::OpenStudio).size());

  EXPECT_TRUE(wrapper.isInFile(IddObjectType::OS_AirLoopHVAC));
  EXPECT_FALSE(wrapper.isInFile(IddObjectType::Window));
  EXPECT_FALSE(wrapper.getObject(IddObjectType::UserCustom));
}

TEST_F(IddFixture,IddFileAndFactoryWrapper_DefaultConstructor)
{
  IddFileAndFactoryWrapper wrapper;
  EXPECT_TRUE(wrapper.requiredObjects() ==
              IddFactory::instance().getRequiredObjects(IddFileType::OpenStudio));
  EXPECT_TRUE(wrapper.uniqueObjects() ==
              IddFactory::instance().getUniqueObjects(IddFileType::OpenStudio));
  EXPECT_FALSE(wrapper.getObject(IddObjectType::UserCustom));

  wrapper.setIddFile(IddFileType::UserCustom);
  EXPECT_TRUE(wrapper.requiredObjects().empty());
  EXPECT_TRUE(wrapper.uniqueObjects().empty());

  wrapper.setIddFile(IddFactory::instance().getIddFile(IddFileType::OpenStudio));
  // Even though OpenStudio IddFile, it has been disembodied from IddFactory, so IddFileType
  // has been lost.
  EXPECT_TRUE(wrapper.iddFileType() == IddFileType(IddFileType::UserCustom));
  EXPECT_TRUE(wrapper.requiredObjects().size() ==
              IddFactory::instance().getRequiredObjects(IddFileType::OpenStudio).size());
  EXPECT_TRUE(wrapper.uniqueObjects().size() ==
              IddFactory::instance().getUniqueObjects(IddFileType::OpenStudio).size());

  // no change to underlying IddFile
  wrapper.setIddFile(IddFileType::UserCustom);
  EXPECT_TRUE(wrapper.requiredObjects().size() ==
              IddFactory::instance().getRequiredObjects(IddFileType::OpenStudio).size());
  EXPECT_TRUE(wrapper.uniqueObjects().size() ==
              IddFactory::instance().getUniqueObjects(IddFileType::OpenStudio).size());
}

TEST_F(IddFixture,IddFileAndFactoryWrapper_DirectlySetUserCustom) {
  IddFileAndFactoryWrapper wrapper(IddFileType::UserCustom);
  IddFile file = wrapper.iddFile();
  EXPECT_TRUE(file.objects().empty());
  for (int typeIndex : IddObjectType::getValues()) {
    EXPECT_FALSE(wrapper.getObject(IddObjectType(typeIndex)));
  }
}
