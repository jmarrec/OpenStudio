void ForwardTranslator::translateThermalZoneSpacesWhenCombinedSpaces(model::ThermalZone& modelObject, IdfObject& idfObject) {

  std::string tzName = modelObject.nameString();

  std::vector<Space> spaces = modelObject.spaces();

  // EXCLUDE SPACE TRANSLATION (old behavior)
  // Note, when m_excludeSpaceTranslation = true, when you reach this point of the forward translator thermalZone.combineSpaces()
  // has already been called. This happens in ForwardTranslator::translateModelPrivate. As a result, each zone has 0 or 1 space only
  if (spaces.empty()) {
    LOG(Warn, "ThermalZone " << tzName << " does not have any geometry or loads associated with it. It will not be translated");
    return;  // boost::none;
  }

  OS_ASSERT(spaces.size() == 1);
  auto& space = spaces[0];

  idfObject.setDouble(openstudio::ZoneFields::DirectionofRelativeNorth, spaces[0].directionofRelativeNorth());
  idfObject.setDouble(openstudio::ZoneFields::XOrigin, space.xOrigin());
  idfObject.setDouble(openstudio::ZoneFields::YOrigin, space.yOrigin());
  idfObject.setDouble(openstudio::ZoneFields::ZOrigin, space.zOrigin());

  if (space.partofTotalFloorArea()) {
    idfObject.setString(openstudio::ZoneFields::PartofTotalFloorArea, "Yes");
  } else {
    idfObject.setString(openstudio::ZoneFields::PartofTotalFloorArea, "No");
  }

  // translate the space now
  translateAndMapModelObject(space);

  // Translate all Space-specific loads
  auto translateSpaceLoads = [this](auto loads) {
    std::sort(loads.begin(), loads.end(), WorkspaceObjectNameLess());
    for (auto& load : loads) {
      translateAndMapModelObject(load);
    }
  };

  // TODO: THIS SHOULD HAPPEN. The translateSpace should do it!
  translateSpaceLoads(space.shadingSurfaceGroups());
  translateSpaceLoads(space.interiorPartitionSurfaceGroups());
  translateSpaceLoads(space.surfaces());

  translateSpaceLoads(space.internalMass());
  translateSpaceLoads(space.lights());
  translateSpaceLoads(space.luminaires());
  translateSpaceLoads(space.people());
  translateSpaceLoads(space.electricEquipment());
  translateSpaceLoads(space.electricEquipmentITEAirCooled());
  translateSpaceLoads(space.gasEquipment());
  translateSpaceLoads(space.hotWaterEquipment());
  translateSpaceLoads(space.steamEquipment());
  translateSpaceLoads(space.otherEquipment());
  // END TODO

  // Technically this stuff maps to a thermal zone, always (can't map to a Space/SpaceList), so it's perfectly appropriate here
  translateSpaceLoads(space.spaceInfiltrationDesignFlowRates());
  translateSpaceLoads(space.spaceInfiltrationEffectiveLeakageAreas());
  translateSpaceLoads(space.spaceInfiltrationFlowCoefficients());

  // translate daylighting controls
  boost::optional<DaylightingControl> primaryDaylightingControl = modelObject.primaryDaylightingControl();
  if (primaryDaylightingControl) {
    boost::optional<DaylightingControl> secondaryDaylightingControl = modelObject.secondaryDaylightingControl();

    IdfObject daylightingControlObject(openstudio::IddObjectType::Daylighting_Controls);
    // Name it like the Zone name + " DaylightingControls"
    daylightingControlObject.setName(tzName + " DaylightingControls");
    m_idfObjects.push_back(daylightingControlObject);

    // Zone Name
    daylightingControlObject.setString(Daylighting_ControlsFields::ZoneorSpaceName, tzName);

    // Availability Schedule Name
    if (boost::optional<Schedule> sched = modelObject.daylightingControlsAvailabilitySchedule()) {
      if (boost::optional<IdfObject> idfo = translateAndMapModelObject(sched.get())) {
        daylightingControlObject.setString(Daylighting_ControlsFields::AvailabilityScheduleName, idfo->nameString());
      }
    }

    // Primary Control
    IdfObject primaryReferencePoint(openstudio::IddObjectType::Daylighting_ReferencePoint);
    // Name it like the OS:Daylighting:Control corresponding to the Primary Reference Point
    primaryReferencePoint.setName(primaryDaylightingControl->nameString());
    m_idfObjects.push_back(primaryReferencePoint);

    OptionalSpace refSpace = primaryDaylightingControl->space();
    if (refSpace) {
      OptionalThermalZone refThermalZone = refSpace->thermalZone();
      // TODO: I think we should catch the case when refThermalZone != modelObject...
      if (refThermalZone) {
        primaryReferencePoint.setString(Daylighting_ReferencePointFields::ZoneorSpaceName, refThermalZone->nameString());
      }
    }

    primaryReferencePoint.setDouble(Daylighting_ReferencePointFields::XCoordinateofReferencePoint, primaryDaylightingControl->positionXCoordinate());
    primaryReferencePoint.setDouble(Daylighting_ReferencePointFields::YCoordinateofReferencePoint, primaryDaylightingControl->positionYCoordinate());
    primaryReferencePoint.setDouble(Daylighting_ReferencePointFields::ZCoordinateofReferencePoint, primaryDaylightingControl->positionZCoordinate());

    double primaryFrac = modelObject.fractionofZoneControlledbyPrimaryDaylightingControl();
    if (istringEqual("None", primaryDaylightingControl->lightingControlType())) {
      if (primaryFrac > 0.0) {
        primaryFrac = 0.0;
        LOG(Warn, "Fraction of Zone Controlled by Primary Daylight Control is "
                    << primaryFrac << " but lighting control type is 'None'. Reseting Primary Fraction to " << 0.0);
      }
    }
    std::string fractionofZoneControlledbyFirstReferencePoint = toString(primaryFrac);
    std::string illuminanceSetpointatFirstReferencePoint = toString(primaryDaylightingControl->illuminanceSetpoint());

    std::vector<std::string> firstGroup;
    firstGroup.push_back(primaryReferencePoint.nameString());
    firstGroup.push_back(fractionofZoneControlledbyFirstReferencePoint);
    firstGroup.push_back(illuminanceSetpointatFirstReferencePoint);
    daylightingControlObject.pushExtensibleGroup(firstGroup);

    // Secondary Control
    if (secondaryDaylightingControl) {
      IdfObject secondaryReferencePoint(openstudio::IddObjectType::Daylighting_ReferencePoint);
      // Name it like the OS:Daylighting:Control corresponding to the Secondary Reference Point
      secondaryReferencePoint.setName(secondaryDaylightingControl->nameString());
      m_idfObjects.push_back(secondaryReferencePoint);

      refSpace = secondaryDaylightingControl->space();
      if (refSpace) {
        OptionalThermalZone refThermalZone = refSpace->thermalZone();
        // TODO: catch if refThermalZone != modelObject
        if (refThermalZone) {
          secondaryReferencePoint.setString(Daylighting_ReferencePointFields::ZoneorSpaceName, refThermalZone->nameString());
        }
      }

      secondaryReferencePoint.setString(Daylighting_ReferencePointFields::ZoneorSpaceName, tzName);
      secondaryReferencePoint.setDouble(Daylighting_ReferencePointFields::XCoordinateofReferencePoint,
                                        secondaryDaylightingControl->positionXCoordinate());
      secondaryReferencePoint.setDouble(Daylighting_ReferencePointFields::YCoordinateofReferencePoint,
                                        secondaryDaylightingControl->positionYCoordinate());
      secondaryReferencePoint.setDouble(Daylighting_ReferencePointFields::ZCoordinateofReferencePoint,
                                        secondaryDaylightingControl->positionZCoordinate());

      double secondaryFrac = modelObject.fractionofZoneControlledbySecondaryDaylightingControl();
      if (istringEqual("None", secondaryDaylightingControl->lightingControlType())) {
        if (secondaryFrac > 0.0) {
          secondaryFrac = 0.0;
          LOG(Warn, "Fraction of Zone Controlled by Secondary Daylight Control is "
                      << secondaryFrac << " but lighting control type is 'None'. Reseting Secondary Fraction to " << 0.0);
        }
      } else {
        if ((primaryFrac + secondaryFrac) > 1) {
          // Reset secondary to 1 - Primary
          secondaryFrac = std::max(1.0 - primaryFrac, 0.0);
          LOG(Warn, "Fraction of Zone Controlled by Primary Daylight Control is " << primaryFrac << " while Secondary Fraction is " << secondaryFrac
                                                                                  << ". Reseting Secondary Fraction to " << secondaryFrac);
        }
      }
      std::string fractionofZoneControlledbySecondReferencePoint = toString(secondaryFrac);
      std::string illuminanceSetpointatSecondReferencePoint = toString(secondaryDaylightingControl->illuminanceSetpoint());

      std::vector<std::string> secondGroup;
      secondGroup.push_back(secondaryReferencePoint.nameString());
      secondGroup.push_back(fractionofZoneControlledbySecondReferencePoint);
      secondGroup.push_back(illuminanceSetpointatSecondReferencePoint);
      daylightingControlObject.pushExtensibleGroup(secondGroup);
    }

    // Shared Data
    std::string lightingControlType = primaryDaylightingControl->lightingControlType();
    if (istringEqual("None", lightingControlType)) {
      // fraction for first point have been set to 0, try to get control type from second point
      if (secondaryDaylightingControl) {
        lightingControlType = secondaryDaylightingControl->lightingControlType();
      }
    }

    if (istringEqual("None", lightingControlType)) {
      // both fractions are 0
    } else if (istringEqual("Continuous", lightingControlType)) {
      daylightingControlObject.setString(Daylighting_ControlsFields::LightingControlType, "Continuous");
    } else if (istringEqual("Stepped", lightingControlType)) {
      daylightingControlObject.setString(Daylighting_ControlsFields::LightingControlType, "Stepped");
    } else if (istringEqual("Continuous/Off", lightingControlType)) {
      daylightingControlObject.setString(Daylighting_ControlsFields::LightingControlType, "ContinuousOff");
    } else {
      LOG(Warn,
          "Unknown lighting control type '" << lightingControlType << "' for OS:Daylighting:Control " << primaryDaylightingControl->nameString());
    }

    if (primaryDaylightingControl->psiRotationAroundXAxis() != 0.0) {
      LOG(Warn, "Rotation of " << primaryDaylightingControl->psiRotationAroundXAxis()
                               << " degrees about X axis not mapped for OS:Daylighting:Control " << primaryDaylightingControl->nameString());
    }

    if (primaryDaylightingControl->thetaRotationAroundYAxis() != 0.0) {
      LOG(Warn, "Rotation of " << primaryDaylightingControl->thetaRotationAroundYAxis()
                               << " degrees about Y axis not mapped for OS:Daylighting:Control " << primaryDaylightingControl->nameString());
    }

    // glare
    double glareAngle = primaryDaylightingControl->phiRotationAroundZAxis();
    daylightingControlObject.setDouble(Daylighting_ControlsFields::GlareCalculationAzimuthAngleofViewDirectionClockwisefromZoneyAxis, glareAngle);

    if (OptionalDouble d = primaryDaylightingControl->maximumAllowableDiscomfortGlareIndex()) {
      daylightingControlObject.setDouble(Daylighting_ControlsFields::MaximumAllowableDiscomfortGlareIndex, *d);
    }

    daylightingControlObject.setString(Daylighting_ControlsFields::GlareCalculationDaylightingReferencePointName, primaryReferencePoint.nameString());

    daylightingControlObject.setDouble(Daylighting_ControlsFields::MinimumInputPowerFractionforContinuousorContinuousOffDimmingControl,
                                       primaryDaylightingControl->minimumInputPowerFractionforContinuousDimmingControl());

    daylightingControlObject.setDouble(Daylighting_ControlsFields::MinimumLightOutputFractionforContinuousorContinuousOffDimmingControl,
                                       primaryDaylightingControl->minimumLightOutputFractionforContinuousDimmingControl());

    daylightingControlObject.setInt(Daylighting_ControlsFields::NumberofSteppedControlSteps,
                                    primaryDaylightingControl->numberofSteppedControlSteps());

    daylightingControlObject.setDouble(Daylighting_ControlsFields::ProbabilityLightingwillbeResetWhenNeededinManualSteppedControl,
                                       primaryDaylightingControl->probabilityLightingwillbeResetWhenNeededinManualSteppedControl());
  }

  // translate illuminance map
  boost::optional<IlluminanceMap> illuminanceMap = modelObject.illuminanceMap();
  if (illuminanceMap) {
    if (!primaryDaylightingControl) {
      LOG(Warn, "Daylighting:Controls object is required to trigger daylighting calculations in EnergyPlus, adding a minimal one to Zone " << tzName);

      IdfObject referencePoint(openstudio::IddObjectType::Daylighting_ReferencePoint);
      referencePoint.setName(tzName + " Daylighting Reference Point");
      m_idfObjects.push_back(referencePoint);
      referencePoint.setString(Daylighting_ReferencePointFields::ZoneorSpaceName, tzName);
      referencePoint.setDouble(Daylighting_ReferencePointFields::XCoordinateofReferencePoint,
                               illuminanceMap->originXCoordinate() + 0.5 * illuminanceMap->xLength());
      referencePoint.setDouble(Daylighting_ReferencePointFields::YCoordinateofReferencePoint,
                               illuminanceMap->originYCoordinate() + 0.5 * illuminanceMap->yLength());
      referencePoint.setDouble(Daylighting_ReferencePointFields::ZCoordinateofReferencePoint, illuminanceMap->originZCoordinate());

      IdfObject daylightingControlObject(openstudio::IddObjectType::Daylighting_Controls);
      // Name it like the Zone name + " DaylightingControls"
      daylightingControlObject.setName(tzName + " DaylightingControls");
      m_idfObjects.push_back(daylightingControlObject);

      daylightingControlObject.setString(Daylighting_ControlsFields::ZoneorSpaceName, tzName);
      std::vector<std::string> group{referencePoint.nameString(),  // ref point name
                                     "0.0",                        // fraction controlled
                                     ""};                          // illuminance setpoint
      daylightingControlObject.pushExtensibleGroup(group);
    }

    IdfObject illuminanceMapObject(openstudio::IddObjectType::Output_IlluminanceMap);
    m_idfObjects.push_back(illuminanceMapObject);

    illuminanceMapObject.setString(Output_IlluminanceMapFields::Name, illuminanceMap->nameString());

    illuminanceMapObject.setString(Output_IlluminanceMapFields::ZoneName, tzName);

    illuminanceMapObject.setDouble(Output_IlluminanceMapFields::Zheight, illuminanceMap->originZCoordinate());

    illuminanceMapObject.setDouble(Output_IlluminanceMapFields::XMinimumCoordinate, illuminanceMap->originXCoordinate());
    illuminanceMapObject.setDouble(Output_IlluminanceMapFields::XMaximumCoordinate, illuminanceMap->originXCoordinate() + illuminanceMap->xLength());
    illuminanceMapObject.setInt(Output_IlluminanceMapFields::NumberofXGridPoints, illuminanceMap->numberofXGridPoints());

    illuminanceMapObject.setDouble(Output_IlluminanceMapFields::YMinimumCoordinate, illuminanceMap->originYCoordinate());
    illuminanceMapObject.setDouble(Output_IlluminanceMapFields::YMaximumCoordinate, illuminanceMap->originYCoordinate() + illuminanceMap->yLength());
    illuminanceMapObject.setInt(Output_IlluminanceMapFields::NumberofYGridPoints, illuminanceMap->numberofYGridPoints());

    if (illuminanceMap->psiRotationAroundXAxis() != 0.0) {
      LOG(Warn, "Rotation of " << illuminanceMap->psiRotationAroundXAxis() << " degrees about X axis not mapped for OS:IlluminanceMap "
                               << illuminanceMap->nameString());
    }

    if (illuminanceMap->thetaRotationAroundYAxis() != 0.0) {
      LOG(Warn, "Rotation of " << illuminanceMap->thetaRotationAroundYAxis() << " degrees about Y axis not mapped for OS:IlluminanceMap "
                               << illuminanceMap->nameString());
    }

    if (illuminanceMap->phiRotationAroundZAxis() != 0.0) {
      LOG(Warn, "Rotation of " << illuminanceMap->phiRotationAroundZAxis() << " degrees about Z axis not mapped for OS:IlluminanceMap "
                               << illuminanceMap->nameString());
    }
  }

  // translate zone mixing objects which supply air to this zone
  ZoneMixingVector supplyZoneMixing = modelObject.supplyZoneMixing();
  std::sort(supplyZoneMixing.begin(), supplyZoneMixing.end(), WorkspaceObjectNameLess());
  for (ZoneMixing& mixing : supplyZoneMixing) {
    translateAndMapModelObject(mixing);
  }

  auto zoneEquipment = modelObject.equipment();

  // In OS ZoneVentilationDesignFlowRate and ZoneVentilationWindandStackOpenArea are considered zone equipment,
  // but for the E+ perspective it is not so we have to remove them and treat them differently.
  auto isZoneVentilationObject = [](const ModelObject& mo) {
    return ((mo.iddObjectType() == ZoneVentilationDesignFlowRate::iddObjectType())
            || (mo.iddObjectType() == ZoneVentilationWindandStackOpenArea::iddObjectType()));
  };

  std::vector<model::ModelObject> zoneVentilationObjects;
  std::copy_if(zoneEquipment.begin(), zoneEquipment.end(), std::back_inserter(zoneVentilationObjects), isZoneVentilationObject);

  auto zoneVentilationBegin = std::remove_if(zoneEquipment.begin(), zoneEquipment.end(), isZoneVentilationObject);
  zoneEquipment.erase(zoneVentilationBegin, zoneEquipment.end());

  // translate thermostat and/or humidistat
  if ((!zoneEquipment.empty()) || modelObject.useIdealAirLoads()) {
    // Thermostat
    if (auto thermostat = modelObject.thermostat()) {
      if (thermostat->iddObjectType() == ZoneControlThermostatStagedDualSetpoint::iddObjectType()) {
        // This one we translate already
        translateAndMapModelObject(thermostat.get());
      } else {

        // This is a OS:ThermostatSetpoint:DualSetpoint as it's the only other choice.
        auto dualSetpoint = thermostat->cast<ThermostatSetpointDualSetpoint>();

        auto createZoneControlThermostat = [&]() {
          IdfObject zoneControlThermostat(openstudio::IddObjectType::ZoneControl_Thermostat);
          zoneControlThermostat.setString(ZoneControl_ThermostatFields::Name, tzName + " Thermostat");
          zoneControlThermostat.setString(ZoneControl_ThermostatFields::ZoneorZoneListName, tzName);
          m_idfObjects.push_back(zoneControlThermostat);

          // Need to handle the control type base don thermostat type (1: Single heating, 2: single cooling, 4: Dual setpoint)
          IdfObject scheduleCompact(openstudio::IddObjectType::Schedule_Compact);
          scheduleCompact.setName(tzName + " Thermostat Schedule");
          m_idfObjects.push_back(scheduleCompact);
          scheduleCompact.setString(1, tzName + " Thermostat Schedule Type Limits");
          scheduleCompact.setString(2, "Through: 12/31");
          scheduleCompact.setString(3, "For: AllDays");
          scheduleCompact.setString(4, "Until: 24:00");
          scheduleCompact.setString(5, "4");

          IdfObject scheduleTypeLimits(openstudio::IddObjectType::ScheduleTypeLimits);
          scheduleTypeLimits.setName(tzName + " Thermostat Schedule Type Limits");
          m_idfObjects.push_back(scheduleTypeLimits);
          scheduleTypeLimits.setString(1, "0");
          scheduleTypeLimits.setString(2, "4");
          scheduleTypeLimits.setString(3, "DISCRETE");

          zoneControlThermostat.setString(ZoneControl_ThermostatFields::ControlTypeScheduleName, scheduleCompact.nameString());

          if (boost::optional<IdfObject> idfThermostat = translateAndMapModelObject(dualSetpoint)) {
            // TODO: JM 2019-09-04 switch back to an extensible object once/if https://github.com/NREL/EnergyPlus/issues/7484 is addressed and the
            // 'Temperature Difference Between Cutout And Setpoint' field is moved before the extensible fields
            // For now, we revert to a non extensible object, so we can still write that field

            //StringVector values(zoneControlThermostat.iddObject().properties().numExtensible);
            //values[ZoneControl_ThermostatExtensibleFields::ControlObjectType] = idfThermostat->iddObject().name();
            //values[ZoneControl_ThermostatExtensibleFields::ControlName] = idfThermostat->nameString();
            //IdfExtensibleGroup eg = zoneControlThermostat.pushExtensibleGroup(values);

            zoneControlThermostat.setString(ZoneControl_ThermostatFields::Control1ObjectType, idfThermostat->iddObject().name());
            zoneControlThermostat.setString(ZoneControl_ThermostatFields::Control1Name, idfThermostat->nameString());

            if (idfThermostat->iddObject().name() == "ThermostatSetpoint:SingleHeating") {
              scheduleCompact.setString(5, "1");
            } else if (idfThermostat->iddObject().name() == "ThermostatSetpoint:SingleCooling") {
              scheduleCompact.setString(5, "2");
            } else {
              // DualSetpoint
              scheduleCompact.setString(5, "4");
            }

            // Thermostat's Temperature Difference Between Cutout And Setpoint is placed here on the ZoneControl:Thermostat
            zoneControlThermostat.setDouble(ZoneControl_ThermostatFields::TemperatureDifferenceBetweenCutoutAndSetpoint,
                                            dualSetpoint.temperatureDifferenceBetweenCutoutAndSetpoint());
          }
        };

        // Only translate ThermostatSetpointDualSetpoint if there is at least one schedule attached
        // The translation to SingleHeating, SingleCooling, or DualSetpoint as appropriate is handled in ForwardTranslateThermostatSetpointDualSetpoint
        if (dualSetpoint.heatingSetpointTemperatureSchedule() || dualSetpoint.coolingSetpointTemperatureSchedule()) {
          createZoneControlThermostat();
        }
      }
    }

    // Humidistat
    if (boost::optional<ZoneControlHumidistat> humidistat = modelObject.zoneControlHumidistat()) {
      if (boost::optional<IdfObject> idfHumidistat = translateAndMapModelObject(humidistat.get())) {
        idfHumidistat->setString(ZoneControl_HumidistatFields::ZoneName, tzName);
      }
    }

    // ZoneControlContaminantController
    if (auto controller = modelObject.zoneControlContaminantController()) {
      translateAndMapModelObject(controller.get());
    }
  }

  // Ideal air loads
  if (modelObject.useIdealAirLoads()) {
    IdfObject idealLoadsAirSystem(IddObjectType::HVACTemplate_Zone_IdealLoadsAirSystem);

    idealLoadsAirSystem.setString(HVACTemplate_Zone_IdealLoadsAirSystemFields::ZoneName, tzName);

    m_idfObjects.push_back(idealLoadsAirSystem);
  }

  // ZoneVentilationDesignFlowRate and ZoneVentilationWindandStackOpenArea do not go on equipment connections or associated list
  for (auto& zone_vent : zoneVentilationObjects) {
    translateAndMapModelObject(zone_vent);
  }

  bool zoneHVACIdealWorkaround = false;
  boost::optional<ZoneHVACIdealLoadsAirSystem> ideal;
  if (zoneEquipment.size() == 1) {
    ideal = zoneEquipment.front().optionalCast<model::ZoneHVACIdealLoadsAirSystem>();
    if (ideal) {
      auto returnPlenum = ideal->returnPlenum();
      if (returnPlenum) {
        auto allIdealHVAC = returnPlenum->getImpl<model::detail::AirLoopHVACReturnPlenum_Impl>()->zoneHVACIdealLoadsAirSystems();
        if (!allIdealHVAC.empty()) {
          zoneHVACIdealWorkaround = true;
        }
      }
    }
  }

  if (zoneHVACIdealWorkaround) {
    // ZoneHVAC_EquipmentConnections
    IdfObject connectionsObject(openstudio::IddObjectType::ZoneHVAC_EquipmentConnections);
    m_idfObjects.push_back(connectionsObject);

    connectionsObject.setString(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneName, tzName);

    //set the inlet port list
    PortList inletPortList = modelObject.inletPortList();
    if (!inletPortList.modelObjects().empty()) {
      boost::optional<IdfObject> _inletNodeList = translateAndMapModelObject(inletPortList);
      if (_inletNodeList) {
        _inletNodeList->setName(tzName + " Inlet Node List");
        connectionsObject.setString(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneAirInletNodeorNodeListName, _inletNodeList->nameString());
      }
    }

    //set the zone air node
    Node node = modelObject.zoneAirNode();
    connectionsObject.setString(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneAirNodeName, node.nameString());

    // Use the exhaust node as the zone return node in this workaround
    //set the zone return air node
    auto exhaustPortList = modelObject.exhaustPortList();
    auto exhaustNodes = subsetCastVector<model::Node>(exhaustPortList.modelObjects());
    OS_ASSERT(exhaustNodes.size() == 1);
    connectionsObject.setString(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneReturnAirNodeorNodeListName, exhaustNodes.front().nameString());

    // ZoneHVAC_EquipmentList
    ZoneHVACEquipmentList equipmentList = modelObject.getImpl<model::detail::ThermalZone_Impl>()->zoneHVACEquipmentList();
    boost::optional<IdfObject> _equipmentList = translateAndMapModelObject(equipmentList);

    if (_equipmentList) {
      connectionsObject.setString(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneConditioningEquipmentListName, _equipmentList->nameString());
    }
  } else if (!zoneEquipment.empty()) {
    // ZoneHVAC_EquipmentConnections
    IdfObject connectionsObject(openstudio::IddObjectType::ZoneHVAC_EquipmentConnections);
    m_idfObjects.push_back(connectionsObject);

    connectionsObject.setString(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneName, tzName);

    //set the inlet port list
    PortList inletPortList = modelObject.inletPortList();
    if (!inletPortList.modelObjects().empty()) {
      boost::optional<IdfObject> _inletNodeList = translateAndMapModelObject(inletPortList);
      if (_inletNodeList) {
        connectionsObject.setString(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneAirInletNodeorNodeListName, _inletNodeList->nameString());
      }
    }

    //set the exhaust port list
    PortList exhaustPortList = modelObject.exhaustPortList();
    if (!exhaustPortList.modelObjects().empty()) {
      boost::optional<IdfObject> _exhaustNodeList = translateAndMapModelObject(exhaustPortList);
      if (_exhaustNodeList) {
        _exhaustNodeList->setName(tzName + " Exhaust Node List");
        connectionsObject.setString(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneAirExhaustNodeorNodeListName,
                                    _exhaustNodeList->nameString());
      }
    }

    //set the zone air node
    Node node = modelObject.zoneAirNode();
    connectionsObject.setString(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneAirNodeName, node.nameString());

    //set the zone return air node
    auto returnPortList = modelObject.returnPortList();
    if (!returnPortList.modelObjects().empty()) {
      auto _returnNodeList = translateAndMapModelObject(returnPortList);
      if (_returnNodeList) {
        _returnNodeList->setName(tzName + " Return Node List");
        connectionsObject.setString(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneReturnAirNodeorNodeListName, _returnNodeList->nameString());
      }
    }

    // ZoneHVAC_EquipmentList

    ZoneHVACEquipmentList equipmentList = modelObject.getImpl<model::detail::ThermalZone_Impl>()->zoneHVACEquipmentList();

    boost::optional<IdfObject> _equipmentList = translateAndMapModelObject(equipmentList);

    if (_equipmentList) {
      connectionsObject.setString(openstudio::ZoneHVAC_EquipmentConnectionsFields::ZoneConditioningEquipmentListName, _equipmentList->nameString());
    }
  }

  // SizingZone

  if ((!zoneEquipment.empty()) || modelObject.useIdealAirLoads()) {
    // get sizing period objects from the model
    std::vector<SizingPeriod> sizingPeriod = modelObject.model().getModelObjects<SizingPeriod>();
    // map the sizing object only if a sizing period object exists
    boost::optional<IdfObject> sizingZoneIdf;
    if (!sizingPeriod.empty()) {
      SizingZone sizingZone = modelObject.sizingZone();
      sizingZoneIdf = translateAndMapModelObject(sizingZone);
      OS_ASSERT(sizingZoneIdf);
    }

    // map the design specification outdoor air
    boost::optional<DesignSpecificationOutdoorAir> designSpecificationOutdoorAir;
    if (!spaces.empty()) {
      designSpecificationOutdoorAir = spaces[0].designSpecificationOutdoorAir();
      if (designSpecificationOutdoorAir) {

        translateAndMapModelObject(*designSpecificationOutdoorAir);

        // point the sizing object to the outdoor air spec
        if (sizingZoneIdf) {
          sizingZoneIdf->setString(Sizing_ZoneFields::DesignSpecificationOutdoorAirObjectName, designSpecificationOutdoorAir->nameString());
        }

        // create zone ventilation if needed
        // TODO: we could remove all this code if we used ZoneHVAC:IdealLoadsAirSystem instead of HVACTemplate:Zone:IdealLoadsAirSystem
        if (zoneEquipment.empty()) {

          double outdoorAirFlowperPerson = designSpecificationOutdoorAir->outdoorAirFlowperPerson();
          double outdoorAirFlowperFloorArea = designSpecificationOutdoorAir->outdoorAirFlowperFloorArea();
          double outdoorAirFlowRate = designSpecificationOutdoorAir->outdoorAirFlowRate();
          double outdoorAirFlowAirChangesperHour = designSpecificationOutdoorAir->outdoorAirFlowAirChangesperHour();

          std::string outdoorAirMethod = designSpecificationOutdoorAir->outdoorAirMethod();
          if (istringEqual(outdoorAirMethod, "Max")) {

            double rateForPeople = spaces[0].numberOfPeople() * outdoorAirFlowperPerson;
            double rateForArea = spaces[0].floorArea() * outdoorAirFlowperFloorArea;
            double rate = outdoorAirFlowRate;
            double rateForVolume = spaces[0].volume() * outdoorAirFlowAirChangesperHour;

            double biggestRate = std::max(rateForPeople, std::max(rateForArea, std::max(rate, rateForVolume)));

            if (rateForPeople == biggestRate) {
              //outdoorAirFlowperPerson = 0;
              outdoorAirFlowperFloorArea = 0;
              outdoorAirFlowRate = 0;
              outdoorAirFlowAirChangesperHour = 0;
            } else if (rateForArea == biggestRate) {
              outdoorAirFlowperPerson = 0;
              //outdoorAirFlowperFloorArea = 0;
              outdoorAirFlowRate = 0;
              outdoorAirFlowAirChangesperHour = 0;
            } else if (rate == biggestRate) {
              outdoorAirFlowperPerson = 0;
              outdoorAirFlowperFloorArea = 0;
              //outdoorAirFlowRate = 0;
              outdoorAirFlowAirChangesperHour = 0;
            } else {
              //rateForVolume == biggestRate
              outdoorAirFlowperPerson = 0;
              outdoorAirFlowperFloorArea = 0;
              outdoorAirFlowRate = 0;
              //outdoorAirFlowAirChangesperHour = 0;
            }

          } else {
            // sum
          }

          if (outdoorAirFlowperPerson > 0) {

            // TODO: improve this?
            // find first people schedule
            std::vector<People> allPeople;
            for (const People& people : space.people()) {
              allPeople.push_back(people);
            }
            if (space.spaceType()) {
              for (const People& people : space.spaceType()->people()) {
                allPeople.push_back(people);
              }
            }

            boost::optional<Schedule> peopleSchedule;
            for (const People& people : allPeople) {
              peopleSchedule = people.numberofPeopleSchedule();
              if (peopleSchedule) {
                break;
              }
            }

            if (peopleSchedule) {
              IdfObject zoneVentilation(IddObjectType::ZoneVentilation_DesignFlowRate);
              zoneVentilation.setName(tzName + " Ventilation per Person");
              zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::ZoneorZoneListName, tzName);
              zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::ScheduleName, peopleSchedule->nameString());
              zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::DesignFlowRateCalculationMethod, "Flow/Person");
              zoneVentilation.setDouble(ZoneVentilation_DesignFlowRateFields::FlowRateperPerson, outdoorAirFlowperPerson);
              m_idfObjects.push_back(zoneVentilation);
            } else {
              LOG(Warn, "No People found in ThermalZone '" << tzName << "', outdoor air per person will not be added");
            }
          }

          if (outdoorAirFlowperFloorArea > 0) {
            IdfObject zoneVentilation(IddObjectType::ZoneVentilation_DesignFlowRate);
            zoneVentilation.setName(tzName + " Ventilation per Floor Area");
            zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::ZoneorZoneListName, tzName);
            zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::ScheduleName, this->alwaysOnSchedule().nameString());
            zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::DesignFlowRateCalculationMethod, "Flow/Area");
            zoneVentilation.setDouble(ZoneVentilation_DesignFlowRateFields::FlowRateperZoneFloorArea, outdoorAirFlowperFloorArea);
            m_idfObjects.push_back(zoneVentilation);
          }

          if (outdoorAirFlowRate > 0) {
            IdfObject zoneVentilation(IddObjectType::ZoneVentilation_DesignFlowRate);
            zoneVentilation.setName(tzName + " Ventilation Rate");
            zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::ZoneorZoneListName, tzName);
            zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::ScheduleName, this->alwaysOnSchedule().nameString());
            zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::DesignFlowRateCalculationMethod, "Flow/Zone");
            zoneVentilation.setDouble(ZoneVentilation_DesignFlowRateFields::DesignFlowRate, outdoorAirFlowRate);
            m_idfObjects.push_back(zoneVentilation);
          }

          if (outdoorAirFlowAirChangesperHour > 0) {
            IdfObject zoneVentilation(IddObjectType::ZoneVentilation_DesignFlowRate);
            zoneVentilation.setName(tzName + " Ventilation Air Changes per Hour");
            zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::ZoneorZoneListName, tzName);
            zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::ScheduleName, this->alwaysOnSchedule().nameString());
            zoneVentilation.setString(ZoneVentilation_DesignFlowRateFields::DesignFlowRateCalculationMethod, "AirChanges/Hour");
            zoneVentilation.setDouble(ZoneVentilation_DesignFlowRateFields::AirChangesperHour, outdoorAirFlowAirChangesperHour);
            m_idfObjects.push_back(zoneVentilation);
          }
        }
      }
    }
  }
}
