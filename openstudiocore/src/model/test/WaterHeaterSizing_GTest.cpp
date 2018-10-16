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

#include <gtest/gtest.h>

#include <model/test/ModelFixture.hpp>

#include "../WaterHeaterSizing.hpp"
#include "../WaterHeaterSizing_Impl.hpp"

#include "../WaterHeaterMixed.hpp"
#include "../WaterHeaterStratified.hpp"

#include "../HeatExchangerFluidToFluid.hpp"


using namespace openstudio;
using namespace openstudio::model;


TEST_F(ModelFixture,WaterHeaterSizing) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  ASSERT_EXIT (
  {
     model::Model m;
     model::WaterHeaterMixed wh(m);
     model::WaterHeaterSizing sz(m, wh);

     exit(0);
  } ,
    ::testing::ExitedWithCode(0), "" );
}

TEST_F(ModelFixture,WaterHeaterSizing_BadCtor) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  ASSERT_THROW (
  {
     model::Model m;
     // Another WaterToWaterComponent, but inappropriate
     model::HeatExchangerFluidToFluid hx(m);
     model::WaterHeaterSizing sz(m, hx);

  } , openstudio::Exception);
}

TEST_F(ModelFixture,WaterHeaterSizing_ConstructorDefaults) {

  model::Model m;
  model::WaterHeaterMixed wh(m);
  model::WaterHeaterSizing sz(m, wh);

  EXPECT_EQ("PeakDraw", sz.designMode());

  // Test default values
  EXPECT_TRUE(sz.timeStorageCanMeetPeakDraw());
  EXPECT_DOUBLE_EQ(0.538503, sz.timeStorageCanMeetPeakDraw().get());
  EXPECT_TRUE(sz.timeforTankRecovery());
  EXPECT_DOUBLE_EQ(0.0, sz.timeforTankRecovery().get());
  EXPECT_TRUE(sz.nominalTankVolumeforAutosizingPlantConnections());
  EXPECT_DOUBLE_EQ(1.0, sz.nominalTankVolumeforAutosizingPlantConnections().get());


  EXPECT_FALSE(sz.numberofBedrooms());
  EXPECT_FALSE(sz.numberofBathrooms());
  EXPECT_FALSE(sz.storageCapacityperPerson());
  EXPECT_FALSE(sz.recoveryCapacityperPerson());
  EXPECT_FALSE(sz.storageCapacityperFloorArea());
  EXPECT_FALSE(sz.recoveryCapacityperFloorArea());
  EXPECT_FALSE(sz.numberofUnits());
  EXPECT_FALSE(sz.storageCapacityperUnit());
  EXPECT_FALSE(sz.recoveryCapacityperUnit());
  EXPECT_FALSE(sz.storageCapacityperCollectorArea());
  EXPECT_FALSE(sz.heightAspectRatio());

}

TEST_F(ModelFixture,WaterHeaterSizing_GettersSetters) {

  model::Model m;
  model::WaterHeaterMixed wh(m);
  model::WaterHeaterSizing sz(m, wh);

  EXPECT_EQ(wh.handle(), sz.waterHeater().handle());

  // Another WaterToWaterComponent, but inappropriate
  model::HeatExchangerFluidToFluid hx(m);
  EXPECT_FALSE(sz.setWaterHeater(hx));
  EXPECT_EQ(wh.handle(), sz.waterHeater().handle());


  // Name

  // Design Mode
  EXPECT_TRUE(sz.setDesignMode("PerPerson"));
  EXPECT_EQ("PerPerson", sz.designMode());
  EXPECT_FALSE(sz.setDesignMode("Gibberish"));
  EXPECT_EQ("PerPerson", sz.designMode());

  /***************************************************************************
  *                                PeakDraw                                 *
  ***************************************************************************/

  /**
   * Time Storage Can Meet Peak Draw
   */
  // Reset first, so that we don't depend on Ctor at all
  sz.resetTimeStorageCanMeetPeakDraw();
  EXPECT_FALSE(sz.timeStorageCanMeetPeakDraw());

  // Set
  EXPECT_TRUE(sz.setTimeStorageCanMeetPeakDraw(1.0));
  ASSERT_TRUE(sz.timeStorageCanMeetPeakDraw());
  EXPECT_DOUBLE_EQ(1.0, sz.timeStorageCanMeetPeakDraw().get());

  // reset
  sz.resetTimeStorageCanMeetPeakDraw();
  EXPECT_FALSE(sz.timeStorageCanMeetPeakDraw());

  /**
   * Time for Tank Recovery
   */
  // Reset first, so that we don't depend on Ctor at all
  sz.resetTimeforTankRecovery();
  EXPECT_FALSE(sz.timeforTankRecovery());

  // Set
  EXPECT_TRUE(sz.setTimeforTankRecovery(1.0));
  ASSERT_TRUE(sz.timeforTankRecovery());
  EXPECT_DOUBLE_EQ(1.0, sz.timeforTankRecovery().get());

  // reset
  sz.resetTimeforTankRecovery();
  EXPECT_FALSE(sz.timeforTankRecovery());


  /**
   * Nominal Tank Volume for Autosizing Plant Connections
   */
  // Reset first, so that we don't depend on Ctor at all
  sz.resetNominalTankVolumeforAutosizingPlantConnections();
  EXPECT_FALSE(sz.nominalTankVolumeforAutosizingPlantConnections());

  // Set
  EXPECT_TRUE(sz.setNominalTankVolumeforAutosizingPlantConnections(1.0));
  ASSERT_TRUE(sz.nominalTankVolumeforAutosizingPlantConnections());
  EXPECT_DOUBLE_EQ(1.0, sz.nominalTankVolumeforAutosizingPlantConnections().get());

  // reset
  sz.resetNominalTankVolumeforAutosizingPlantConnections();
  EXPECT_FALSE(sz.nominalTankVolumeforAutosizingPlantConnections());

  /***************************************************************************
  *                        ResidentialHUD-FHAMinimum                        *
  ***************************************************************************/


  /**
   * Number of Bedrooms (int)
   */
  // Reset first, so that we don't depend on Ctor at all
  sz.resetNumberofBedrooms();
  EXPECT_FALSE(sz.numberofBedrooms());

  // Set
  EXPECT_TRUE(sz.setNumberofBedrooms(1));
  ASSERT_TRUE(sz.numberofBedrooms());
  EXPECT_EQ(1, sz.numberofBedrooms().get());

  // reset
  sz.resetNumberofBedrooms();
  EXPECT_FALSE(sz.numberofBedrooms());
  /**
   * Number of Bathrooms (int)
   */
  // Reset first, so that we don't depend on Ctor at all
  sz.resetNumberofBathrooms();
  EXPECT_FALSE(sz.numberofBathrooms());

  // Set
  EXPECT_TRUE(sz.setNumberofBathrooms(1));
  ASSERT_TRUE(sz.numberofBathrooms());
  EXPECT_EQ(1, sz.numberofBathrooms().get());

  // reset
  sz.resetNumberofBathrooms();
  EXPECT_FALSE(sz.numberofBathrooms());

  /***************************************************************************
  *                                 PerPerson                                        *
  ***************************************************************************/

  /**
   * Storage Capacity per Person
   */
  // Reset first, so that we don't depend on Ctor at all
  sz.resetStorageCapacityperPerson();
  EXPECT_FALSE(sz.storageCapacityperPerson());

  // Set
  EXPECT_TRUE(sz.setStorageCapacityperPerson(0.05));
  ASSERT_TRUE(sz.storageCapacityperPerson());
  EXPECT_DOUBLE_EQ(0.05, sz.storageCapacityperPerson().get());

  // reset
  sz.resetStorageCapacityperPerson();
  EXPECT_FALSE(sz.storageCapacityperPerson());

  /**
   * Recovery Capacity per Person
   */
  // Reset first, so that we don't depend on Ctor at all
  sz.resetRecoveryCapacityperPerson();
  EXPECT_FALSE(sz.recoveryCapacityperPerson());

  // Set
  EXPECT_TRUE(sz.setRecoveryCapacityperPerson(1.0));
  ASSERT_TRUE(sz.recoveryCapacityperPerson());
  EXPECT_DOUBLE_EQ(1.0, sz.recoveryCapacityperPerson().get());

  // reset
  sz.resetRecoveryCapacityperPerson();
  EXPECT_FALSE(sz.recoveryCapacityperPerson());

  /***************************************************************************
  *                                 PerFloorArea                                        *
  ***************************************************************************/

  /**
   * Storage Capacity per Floor Area
   */
  sz.resetStorageCapacityperFloorArea();
  EXPECT_FALSE(sz.storageCapacityperFloorArea());

  // Set
  EXPECT_TRUE(sz.setStorageCapacityperFloorArea(0.05));
  ASSERT_TRUE(sz.storageCapacityperFloorArea());
  EXPECT_DOUBLE_EQ(0.05, sz.storageCapacityperFloorArea().get());

  // reset
  sz.resetStorageCapacityperFloorArea();
  EXPECT_FALSE(sz.storageCapacityperFloorArea());

  /**
   * Recovery Capacity per Floor Area
   */
  sz.resetRecoveryCapacityperFloorArea();
  EXPECT_FALSE(sz.recoveryCapacityperFloorArea());

  // Set
  EXPECT_TRUE(sz.setRecoveryCapacityperFloorArea(1.0));
  ASSERT_TRUE(sz.recoveryCapacityperFloorArea());
  EXPECT_DOUBLE_EQ(1.0, sz.recoveryCapacityperFloorArea().get());

  // reset
  sz.resetRecoveryCapacityperFloorArea();
  EXPECT_FALSE(sz.recoveryCapacityperFloorArea());

  /***************************************************************************
  *                                 perUnit                                        *
  ***************************************************************************/

  /**
   * Number of Units
   */

  /**
   * Storage Capacity per Unit
   */
    // Reset first, so that we don't depend on Ctor at all
  sz.resetStorageCapacityperUnit();
  EXPECT_FALSE(sz.storageCapacityperUnit());

  // Set
  EXPECT_TRUE(sz.setStorageCapacityperUnit(0.05));
  ASSERT_TRUE(sz.storageCapacityperUnit());
  EXPECT_DOUBLE_EQ(0.05, sz.storageCapacityperUnit().get());

  // reset
  sz.resetStorageCapacityperUnit();
  EXPECT_FALSE(sz.storageCapacityperUnit());

  /**
   * Recovery Capacity perUnit
   */
  // Reset first, so that we don't depend on Ctor at all
  sz.resetRecoveryCapacityperUnit();
  EXPECT_FALSE(sz.recoveryCapacityperUnit());

  // Set
  EXPECT_TRUE(sz.setRecoveryCapacityperUnit(1.0));
  ASSERT_TRUE(sz.recoveryCapacityperUnit());
  EXPECT_DOUBLE_EQ(1.0, sz.recoveryCapacityperUnit().get());

  // reset
  sz.resetRecoveryCapacityperUnit();
  EXPECT_FALSE(sz.recoveryCapacityperUnit());

  /***************************************************************************
  *                          PerSolarCollectorArea                          *
  ***************************************************************************/

  /**
   * Storage Capacity per Collector Area
   */
    // Reset first, so that we don't depend on Ctor at all
  sz.resetStorageCapacityperCollectorArea();
  EXPECT_FALSE(sz.storageCapacityperCollectorArea());

  // Set
  EXPECT_TRUE(sz.setStorageCapacityperCollectorArea(0.05));
  ASSERT_TRUE(sz.storageCapacityperCollectorArea());
  EXPECT_DOUBLE_EQ(0.05, sz.storageCapacityperCollectorArea().get());

  // reset
  sz.resetStorageCapacityperCollectorArea();
  EXPECT_FALSE(sz.storageCapacityperCollectorArea());

  /**
   * Height Aspect Ratio
   */
  // Reset first, so that we don't depend on Ctor at all
  sz.resetHeightAspectRatio();
  EXPECT_FALSE(sz.heightAspectRatio());

  // Set
  EXPECT_TRUE(sz.setHeightAspectRatio(1.0));
  ASSERT_TRUE(sz.heightAspectRatio());
  EXPECT_DOUBLE_EQ(1.0, sz.heightAspectRatio().get());

  // reset
  sz.resetHeightAspectRatio();
  EXPECT_FALSE(sz.heightAspectRatio());

}
