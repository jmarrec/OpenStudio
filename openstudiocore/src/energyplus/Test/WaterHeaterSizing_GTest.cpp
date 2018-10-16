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

#include <gtest/gtest.h>
#include "EnergyPlusFixture.hpp"

#include "../ForwardTranslator.hpp"
#include "../ReverseTranslator.hpp"

#include "../../model/Model.hpp"

#include "../../model/WaterHeaterSizing.hpp"
#include "../../model/PlantLoop.hpp"
#include "../../model/WaterHeaterMixed.hpp"

#include <utilities/idd/WaterHeater_Sizing_FieldEnums.hxx>
#include <utilities/idd/IddEnums.hxx>

using namespace openstudio::energyplus;
using namespace openstudio::model;
using namespace openstudio;

TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_PeakDraw)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("PeakDraw");

  // PeakDraw
  sz.setTimeStorageCanMeetPeakDraw(0.538503);
  sz.setTimeforTankRecovery(0.0);
  sz.setNominalTankVolumeforAutosizingPlantConnections(1.0);

  // ResidentialHUD-FHAMinimum
  sz.setNumberofBedrooms(20);
  sz.setNumberofBathrooms(10);

  // PerPerson
  sz.setStorageCapacityperPerson(0.05);
  sz.setRecoveryCapacityperPerson(1.0);

  // PerFloorArea
  sz.setStorageCapacityperFloorArea(1.0);
  sz.setRecoveryCapacityperFloorArea(1.0);

  // PerUnit
  sz.setNumberofUnits(10.0);
  sz.setStorageCapacityperUnit(1.0);
  sz.setRecoveryCapacityperUnit(1.0);

  // PerSolarCollectorArea
  sz.setStorageCapacityperCollectorArea(0.05);

  // Other:
  sz.setHeightAspectRatio(3.0);

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  EXPECT_EQ(0u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(1u, idf_szs.size());
  IdfObject idf_sz = idf_szs[0];

  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::WaterHeaterName).get(),
            wh.name().get());

  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::DesignMode).get(),
            "PeakDraw");

  // Peak Draw
  ASSERT_TRUE(idf_sz.getDouble(WaterHeater_SizingFields::TimeStorageCanMeetPeakDraw));
  ASSERT_TRUE(idf_sz.getDouble(WaterHeater_SizingFields::TimeforTankRecovery));
  ASSERT_TRUE(idf_sz.getDouble(WaterHeater_SizingFields::NominalTankVolumeforAutosizingPlantConnections));
  EXPECT_DOUBLE_EQ(0.538503, idf_sz.getDouble(WaterHeater_SizingFields::TimeStorageCanMeetPeakDraw).get());
  EXPECT_DOUBLE_EQ(0.0, idf_sz.getDouble(WaterHeater_SizingFields::TimeforTankRecovery).get());
  EXPECT_DOUBLE_EQ(1.0, idf_sz.getDouble(WaterHeater_SizingFields::NominalTankVolumeforAutosizingPlantConnections).get());

  // ResidentialHUD-FHAMinimum
  EXPECT_FALSE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBedrooms));
  EXPECT_FALSE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBathrooms));

  // PerPerson
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperPerson));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperPerson));

  // PerFloorArea
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperFloorArea));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperFloorArea));

  // PerUnit
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::NumberofUnits));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperUnit));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityPerUnit));

  // PerSolarCollectorArea
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperCollectorArea));

  // Logic not implemented, translated if present
  EXPECT_EQ(3.0, idf_sz.getDouble(WaterHeater_SizingFields::HeightAspectRatio).get());
}

TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_PeakDraw_MissingField)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("PeakDraw");

  // PeakDraw: the first two are required
  sz.resetTimeStorageCanMeetPeakDraw();
  sz.setTimeforTankRecovery(0.0);
  sz.setNominalTankVolumeforAutosizingPlantConnections(1.0);

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  // TimeStorageCanMeetPeakDraw being missing, the WH Sizing shouldn't be translated, but the WH:Mixed should
  EXPECT_EQ(1u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(0u, idf_szs.size());

}


TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_ResidentialHUDFHAMinimum)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("ResidentialHUD-FHAMinimum");

  // PeakDraw
  sz.setTimeStorageCanMeetPeakDraw(0.538503);
  sz.setTimeforTankRecovery(0.0);
  sz.setNominalTankVolumeforAutosizingPlantConnections(1.0);

  // ResidentialHUD-FHAMinimum
  sz.setNumberofBedrooms(20);
  sz.setNumberofBathrooms(10);

  // PerPerson
  sz.setStorageCapacityperPerson(0.05);
  sz.setRecoveryCapacityperPerson(1.0);

  // PerFloorArea
  sz.setStorageCapacityperFloorArea(1.0);
  sz.setRecoveryCapacityperFloorArea(1.0);

  // PerUnit
  sz.setNumberofUnits(10.0);
  sz.setStorageCapacityperUnit(1.0);
  sz.setRecoveryCapacityperUnit(1.0);

  // PerSolarCollectorArea
  sz.setStorageCapacityperCollectorArea(0.05);

  // Other:
  sz.setHeightAspectRatio(3.0);

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  EXPECT_EQ(0u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(1u, idf_szs.size());
  IdfObject idf_sz = idf_szs[0];

  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::WaterHeaterName).get(),
            wh.name().get());

  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::DesignMode).get(),
            "ResidentialHUD-FHAMinimum");

  // Peak Draw
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::TimeStorageCanMeetPeakDraw));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::TimeforTankRecovery));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::NominalTankVolumeforAutosizingPlantConnections));

  // ResidentialHUD-FHAMinimum
  ASSERT_TRUE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBedrooms));
  ASSERT_TRUE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBathrooms));
  EXPECT_EQ(20, idf_sz.getInt(WaterHeater_SizingFields::NumberofBedrooms).get());
  EXPECT_EQ(10, idf_sz.getInt(WaterHeater_SizingFields::NumberofBathrooms).get());

  // PerPerson
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperPerson));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperPerson));

  // PerFloorArea
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperFloorArea));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperFloorArea));

  // PerUnit
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::NumberofUnits));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperUnit));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityPerUnit));

  // PerSolarCollectorArea
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperCollectorArea));

  // Logic not implemented, translated if present
  EXPECT_EQ(3.0, idf_sz.getDouble(WaterHeater_SizingFields::HeightAspectRatio).get());
}

TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_ResidentialHUDFHAMinimum_MissingField)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("ResidentialHUD-FHAMinimum");

  // ResidentialHUD-FHAMinimum: the two are required
  sz.setNumberofBedrooms(20);
  sz.resetNumberofBathrooms();

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  // NumberofBathrooms being missing, the WH Sizing shouldn't be translated, but the WH:Mixed should
  EXPECT_EQ(1u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(0u, idf_szs.size());

}


TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_PerPerson)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("PerPerson");

  // PeakDraw
  sz.setTimeStorageCanMeetPeakDraw(0.538503);
  sz.setTimeforTankRecovery(0.0);
  sz.setNominalTankVolumeforAutosizingPlantConnections(1.0);

  // ResidentialHUD-FHAMinimum
  sz.setNumberofBedrooms(20);
  sz.setNumberofBathrooms(10);

  // PerPerson
  sz.setStorageCapacityperPerson(0.05);
  sz.setRecoveryCapacityperPerson(1.0);

  // PerFloorArea
  sz.setStorageCapacityperFloorArea(1.0);
  sz.setRecoveryCapacityperFloorArea(1.0);

  // PerUnit
  sz.setNumberofUnits(10.0);
  sz.setStorageCapacityperUnit(1.0);
  sz.setRecoveryCapacityperUnit(1.0);

  // PerSolarCollectorArea
  sz.setStorageCapacityperCollectorArea(0.05);

  // Other:
  sz.setHeightAspectRatio(3.0);

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  EXPECT_EQ(0u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(1u, idf_szs.size());
  IdfObject idf_sz = idf_szs[0];

  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::WaterHeaterName).get(),
            wh.name().get());
  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::DesignMode).get(),
            "PerPerson");

  // Peak Draw
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::TimeStorageCanMeetPeakDraw));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::TimeforTankRecovery));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::NominalTankVolumeforAutosizingPlantConnections));

  // ResidentialHUD-FHAMinimum
  EXPECT_FALSE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBedrooms));
  EXPECT_FALSE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBathrooms));

  // PerPerson
  ASSERT_TRUE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperPerson));
  ASSERT_TRUE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperPerson));
  EXPECT_DOUBLE_EQ(0.05, idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperPerson).get());
  EXPECT_DOUBLE_EQ(1.00, idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperPerson).get());

  // PerFloorArea
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperFloorArea));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperFloorArea));

  // PerUnit
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::NumberofUnits));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperUnit));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityPerUnit));

  // PerSolarCollectorArea
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperCollectorArea));

  // Logic not implemented, translated if present
  EXPECT_EQ(3.0, idf_sz.getDouble(WaterHeater_SizingFields::HeightAspectRatio).get());
}

TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_PerPerson_MissingField)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("PerPerson");

  // PerPerson the two are required
  sz.setStorageCapacityperPerson(0.05);
  sz.resetRecoveryCapacityperPerson();

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  // Required field being missing, the WH Sizing shouldn't be translated, but the WH:Mixed should
  EXPECT_EQ(1u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(0u, idf_szs.size());

}


TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_PerFloorArea)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("PerFloorArea");

  // PeakDraw
  sz.setTimeStorageCanMeetPeakDraw(0.538503);
  sz.setTimeforTankRecovery(0.0);
  sz.setNominalTankVolumeforAutosizingPlantConnections(1.0);

  // ResidentialHUD-FHAMinimum
  sz.setNumberofBedrooms(20);
  sz.setNumberofBathrooms(10);

  // PerPerson
  sz.setStorageCapacityperPerson(0.05);
  sz.setRecoveryCapacityperPerson(1.0);

  // PerFloorArea
  sz.setStorageCapacityperFloorArea(0.2);
  sz.setRecoveryCapacityperFloorArea(1.0);

  // PerUnit
  sz.setNumberofUnits(10.0);
  sz.setStorageCapacityperUnit(1.0);
  sz.setRecoveryCapacityperUnit(1.0);

  // PerSolarCollectorArea
  sz.setStorageCapacityperCollectorArea(0.05);

  // Other:
  sz.setHeightAspectRatio(3.0);

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  EXPECT_EQ(0u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(1u, idf_szs.size());
  IdfObject idf_sz = idf_szs[0];

  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::WaterHeaterName).get(),
            wh.name().get());
  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::DesignMode).get(),
            "PerFloorArea");

  // Peak Draw
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::TimeStorageCanMeetPeakDraw));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::TimeforTankRecovery));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::NominalTankVolumeforAutosizingPlantConnections));

  // ResidentialHUD-FHAMinimum
  EXPECT_FALSE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBedrooms));
  EXPECT_FALSE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBathrooms));

  // PerPerson
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperPerson));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperPerson));

  // PerFloorArea
  ASSERT_TRUE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperFloorArea));
  ASSERT_TRUE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperFloorArea));
  EXPECT_DOUBLE_EQ(0.2, idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperFloorArea).get());
  EXPECT_DOUBLE_EQ(1.00, idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperFloorArea).get());

  // PerUnit
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::NumberofUnits));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperUnit));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityPerUnit));

  // PerSolarCollectorArea
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperCollectorArea));

  // Logic not implemented, translated if present
  EXPECT_EQ(3.0, idf_sz.getDouble(WaterHeater_SizingFields::HeightAspectRatio).get());
}

TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_PerFloorArea_MissingField)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("PerFloorArea");

  // PerFloorArea the two are required
  sz.setStorageCapacityperFloorArea(0.05);
  sz.resetRecoveryCapacityperFloorArea();

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  // Required field being missing, the WH Sizing shouldn't be translated, but the WH:Mixed should
  EXPECT_EQ(1u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(0u, idf_szs.size());

}



TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_PerUnit)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("PerUnit");

  // PeakDraw
  sz.setTimeStorageCanMeetPeakDraw(0.538503);
  sz.setTimeforTankRecovery(0.0);
  sz.setNominalTankVolumeforAutosizingPlantConnections(1.0);

  // ResidentialHUD-FHAMinimum
  sz.setNumberofBedrooms(20);
  sz.setNumberofBathrooms(10);

  // PerPerson
  sz.setStorageCapacityperPerson(0.05);
  sz.setRecoveryCapacityperPerson(1.0);

  // PerFloorArea
  sz.setStorageCapacityperFloorArea(0.2);
  sz.setRecoveryCapacityperFloorArea(1.0);

  // PerUnit
  sz.setNumberofUnits(10.0);
  sz.setStorageCapacityperUnit(0.50);
  sz.setRecoveryCapacityperUnit(1.0);

  // PerSolarCollectorArea
  sz.setStorageCapacityperCollectorArea(0.05);

  // Other:
  sz.setHeightAspectRatio(3.0);

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  EXPECT_EQ(0u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(1u, idf_szs.size());
  IdfObject idf_sz = idf_szs[0];

  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::WaterHeaterName).get(),
            wh.name().get());
  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::DesignMode).get(),
            "PerUnit");

  // Peak Draw
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::TimeStorageCanMeetPeakDraw));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::TimeforTankRecovery));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::NominalTankVolumeforAutosizingPlantConnections));

  // ResidentialHUD-FHAMinimum
  EXPECT_FALSE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBedrooms));
  EXPECT_FALSE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBathrooms));

  // PerPerson
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperPerson));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperPerson));

  // PerFloorArea
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperFloorArea));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperFloorArea));

  // PerUnit
  ASSERT_TRUE(idf_sz.getDouble(WaterHeater_SizingFields::NumberofUnits));
  ASSERT_TRUE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperUnit));
  ASSERT_TRUE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityPerUnit));
  EXPECT_DOUBLE_EQ(10.0, idf_sz.getDouble(WaterHeater_SizingFields::NumberofUnits).get());
  EXPECT_DOUBLE_EQ(0.5, idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperUnit).get());
  EXPECT_DOUBLE_EQ(1.00, idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityPerUnit).get());

  // PerSolarCollectorArea
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperCollectorArea));

  // Logic not implemented, translated if present
  EXPECT_EQ(3.0, idf_sz.getDouble(WaterHeater_SizingFields::HeightAspectRatio).get());
}

TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_PerUnit_MissingField)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("PerUnit");

  // PerUnit the three are required
  sz.resetNumberofUnits();
  sz.setStorageCapacityperUnit(0.05);
  sz.setRecoveryCapacityperUnit(1.0);

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  // Required field being missing, the WH Sizing shouldn't be translated, but the WH:Mixed should
  EXPECT_EQ(1u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(0u, idf_szs.size());

}


TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_PerSolarCollectorArea)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("PerSolarCollectorArea");

  // PeakDraw
  sz.setTimeStorageCanMeetPeakDraw(0.538503);
  sz.setTimeforTankRecovery(0.0);
  sz.setNominalTankVolumeforAutosizingPlantConnections(1.0);

  // ResidentialHUD-FHAMinimum
  sz.setNumberofBedrooms(20);
  sz.setNumberofBathrooms(10);

  // PerPerson
  sz.setStorageCapacityperPerson(0.05);
  sz.setRecoveryCapacityperPerson(1.0);

  // PerFloorArea
  sz.setStorageCapacityperFloorArea(0.2);
  sz.setRecoveryCapacityperFloorArea(1.0);

  // PerUnit
  sz.setNumberofUnits(10.0);
  sz.setStorageCapacityperUnit(0.50);
  sz.setRecoveryCapacityperUnit(1.0);

  // PerSolarCollectorArea
  sz.setStorageCapacityperCollectorArea(0.025);

  // Other:
  sz.setHeightAspectRatio(3.0);

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  EXPECT_EQ(0u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(1u, idf_szs.size());
  IdfObject idf_sz = idf_szs[0];

  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::WaterHeaterName).get(),
            wh.name().get());
  EXPECT_EQ(idf_sz.getString(WaterHeater_SizingFields::DesignMode).get(),
            "PerSolarCollectorArea");

  // Peak Draw
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::TimeStorageCanMeetPeakDraw));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::TimeforTankRecovery));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::NominalTankVolumeforAutosizingPlantConnections));

  // ResidentialHUD-FHAMinimum
  EXPECT_FALSE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBedrooms));
  EXPECT_FALSE(idf_sz.getInt(WaterHeater_SizingFields::NumberofBathrooms));

  // PerPerson
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperPerson));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperPerson));

  // PerFloorArea
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperFloorArea));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityperFloorArea));

  // PerUnit
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::NumberofUnits));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperUnit));
  EXPECT_FALSE(idf_sz.getDouble(WaterHeater_SizingFields::RecoveryCapacityPerUnit));

  // PerSolarCollectorArea
  ASSERT_TRUE(idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperCollectorArea));
  EXPECT_DOUBLE_EQ(0.025, idf_sz.getDouble(WaterHeater_SizingFields::StorageCapacityperCollectorArea).get());

  // Logic not implemented, translated if present
  EXPECT_EQ(3.0, idf_sz.getDouble(WaterHeater_SizingFields::HeightAspectRatio).get());
}

TEST_F(EnergyPlusFixture,ForwardTranslator_WaterHeaterSizing_PerSolarCollectorArea_MissingField)
{
  // Need WaterHeater, and a plantLoop to place the water heater probably
  Model m;

  PlantLoop p(m);
  WaterHeaterMixed wh(m);

  p.addSupplyBranchForComponent(wh);

  WaterHeaterSizing sz = wh.waterHeaterSizing();
  sz.setDesignMode("PerSolarCollectorArea");

  // PerSolarCollectorArea the one is required
  sz.resetStorageCapacityperCollectorArea();

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  // Required field being missing, the WH Sizing shouldn't be translated, but the WH:Mixed should
  EXPECT_EQ(1u, ft.errors().size());

  WorkspaceObjectVector idf_whs(w.getObjectsByType(IddObjectType::WaterHeater_Mixed));
  EXPECT_EQ(1u, idf_whs.size());

  WorkspaceObjectVector idf_szs(w.getObjectsByType(IddObjectType::WaterHeater_Sizing));
  EXPECT_EQ(0u, idf_szs.size());

}
