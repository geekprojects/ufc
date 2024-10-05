//
// Created by Ian Parker on 19/06/2024.
//

#ifndef DATADEFS_H
#define DATADEFS_H

#include "xplane.h"

static const DataDefinition g_dataRefsInit[] =
{
    {"aircraft/indicatedAirspeed", FLOAT, offsetof(AircraftState, indicatedAirspeed)},
    {"aircraft/pitch", FLOAT, offsetof(AircraftState, pitch) },
    {"aircraft/roll", FLOAT, offsetof(AircraftState, roll)},
    {"aircraft/altitude", FLOAT, offsetof(AircraftState, altitude)},
    {"aircraft/verticalSpeed", FLOAT, offsetof(AircraftState, verticalSpeed)},
    {"aircraft/indicatedMach", FLOAT, offsetof(AircraftState, indicatedMach)},
    {"aircraft/magHeading", FLOAT, offsetof(AircraftState, magHeading)},
    {"aircraft/barometerHG", FLOAT, offsetof(AircraftState, barometerHG)},
    {"comms/com1Hz", FLOAT, offsetof(AircraftState, comms.com1Hz)},
    {"comms/com1StandbyHz", FLOAT, offsetof(AircraftState, comms.com1StandbyHz)},
    {"comms/com2Hz", FLOAT, offsetof(AircraftState, comms.com2Hz)},
    {"comms/com2StandbyHz", FLOAT, offsetof(AircraftState, comms.com2StandbyHz)},
    {"comms/nav1Hz", FLOAT, offsetof(AircraftState, comms.nav1Hz)},
    {"comms/nav1StandbyHz", FLOAT, offsetof(AircraftState, comms.nav1StandbyHz)},
    {"comms/nav2Hz", FLOAT, offsetof(AircraftState, comms.nav2Hz)},
    {"comms/nav2StandbyHz", FLOAT, offsetof(AircraftState, comms.nav2StandbyHz)},

    {"flightDirector/pitch", FLOAT, offsetof(AircraftState, flightDirector.pitch)},
    {"flightDirector/roll", FLOAT, offsetof(AircraftState, flightDirector.roll)},
    {"flightDirector/mode", INTEGER, offsetof(AircraftState, flightDirector.mode)},

    {"autopilot/displaySpeed", BOOLEAN, offsetof(AircraftState, autopilot.displaySpeed)},
    {"autopilot/speedManaged", BOOLEAN, offsetof(AircraftState, autopilot.speedManaged)},
    {"autopilot/speedMach", BOOLEAN, offsetof(AircraftState, autopilot.speedMach)},
    {"autopilot/speed", FLOAT, offsetof(AircraftState, autopilot.speed)},
    {"autopilot/speedWindowOpen", BOOLEAN, offsetof(AircraftState, autopilot.speedWindowOpen)},

    {"autopilot/displayHeading", BOOLEAN, offsetof(AircraftState, autopilot.displayHeading)},
    {"autopilot/headingManaged", BOOLEAN, offsetof(AircraftState, autopilot.headingManaged)},
    {"autopilot/headingWindowOpen", BOOLEAN, offsetof(AircraftState, autopilot.headingWindowOpen)},
    {"autopilot/heading", FLOAT, offsetof(AircraftState, autopilot.heading)},
    {"autopilot/headingTrkMode", INTEGER, offsetof(AircraftState, autopilot.headingTrkMode)},
    {"autopilot/gpssStatus", INTEGER, offsetof(AircraftState, autopilot.gpssStatus)},

    {"autopilot/displayAltitude", BOOLEAN, offsetof(AircraftState, autopilot.displayAltitude)},
    {"autopilot/altitudeManaged", BOOLEAN, offsetof(AircraftState, autopilot.altitudeManaged)},
    {"autopilot/altitudeStep1000", BOOLEAN, offsetof(AircraftState, autopilot.altitudeStep1000)},
    {"autopilot/altitude", FLOAT, offsetof(AircraftState, autopilot.altitude)},
    {"autopilot/fmsVnav", BOOLEAN, offsetof(AircraftState, autopilot.fmsVnav)},

    {"autopilot/displayVerticalSpeed", BOOLEAN, offsetof(AircraftState, autopilot.displayVerticalSpeed)},
    {"autopilot/verticalSpeed", FLOAT, offsetof(AircraftState, autopilot.verticalSpeed)},
    {"autopilot/verticalSpeedFPAMode", INTEGER, offsetof(AircraftState, autopilot.verticalSpeedFPAMode)},

    {"autopilot/locMode", INTEGER, offsetof(AircraftState, autopilot.locMode)},
    {"autopilot/ap1Mode", INTEGER, offsetof(AircraftState, autopilot.ap1Mode)},
    {"autopilot/ap2Mode", INTEGER, offsetof(AircraftState, autopilot.ap2Mode)},
    {"autopilot/autoThrottleMode", INTEGER, offsetof(AircraftState, autopilot.autoThrottleMode)},
    {"autopilot/approachMode", INTEGER, offsetof(AircraftState, autopilot.approachMode)},
};

#endif //DATADEFS_H
