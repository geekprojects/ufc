//
// Created by Ian Parker on 19/06/2024.
//

#ifndef DATADEFS_H
#define DATADEFS_H

#include "xplane.h"

namespace UFC
{
static const DataDefinition g_dataRefsInit[] =
{
    {DATA_AIRCRAFT_INDICATEDAIRSPEED, FLOAT, offsetof(AircraftState, indicatedAirspeed)},
    {DATA_AIRCRAFT_PITCH, FLOAT, offsetof(AircraftState, pitch) },
    {DATA_AIRCRAFT_ROLL, FLOAT, offsetof(AircraftState, roll)},
    {DATA_AIRCRAFT_ALTITUDE, FLOAT, offsetof(AircraftState, altitude)},
    {DATA_AIRCRAFT_VERTICALSPEED, FLOAT, offsetof(AircraftState, verticalSpeed)},
    {DATA_AIRCRAFT_INDICATEDMACH, FLOAT, offsetof(AircraftState, indicatedMach)},
    {DATA_AIRCRAFT_MAGHEADING, FLOAT, offsetof(AircraftState, magHeading)},
    {DATA_AIRCRAFT_BAROMETERHG, FLOAT, offsetof(AircraftState, barometerHG)},
    {DATA_AIRCRAFT_PARKINGBRAKE, FLOAT, offsetof(AircraftState, parkingBrake)},
    {DATA_COMMS_COM1HZ, INTEGER, offsetof(AircraftState, comms.com1Hz)},
    {DATA_COMMS_COM1STANDBYHZ, INTEGER, offsetof(AircraftState, comms.com1StandbyHz)},
    {DATA_COMMS_COM2HZ, INTEGER, offsetof(AircraftState, comms.com2Hz)},
    {DATA_COMMS_COM2STANDBYHZ, INTEGER, offsetof(AircraftState, comms.com2StandbyHz)},
    {DATA_COMMS_NAV1HZ, INTEGER, offsetof(AircraftState, comms.nav1Hz)},
    {DATA_COMMS_NAV1STANDBYHZ, INTEGER, offsetof(AircraftState, comms.nav1StandbyHz)},
    {DATA_COMMS_NAV2HZ, INTEGER, offsetof(AircraftState, comms.nav2Hz)},
    {DATA_COMMS_NAV2STANDBYHZ, INTEGER, offsetof(AircraftState, comms.nav2StandbyHz)},

    {DATA_FLIGHTDIRECTOR_PITCH, FLOAT, offsetof(AircraftState, flightDirector.pitch)},
    {DATA_FLIGHTDIRECTOR_ROLL, FLOAT, offsetof(AircraftState, flightDirector.roll)},
    {DATA_FLIGHTDIRECTOR_MODE, BOOLEAN, offsetof(AircraftState, flightDirector.mode)},

    {DATA_AUTOPILOT_DISPLAYSPEED, BOOLEAN, offsetof(AircraftState, autopilot.displaySpeed)},
    {DATA_AUTOPILOT_SPEEDMANAGED, BOOLEAN, offsetof(AircraftState, autopilot.speedManaged)},
    {DATA_AUTOPILOT_SPEEDMACH, BOOLEAN, offsetof(AircraftState, autopilot.speedMach)},
    {DATA_AUTOPILOT_SPEED, FLOAT, offsetof(AircraftState, autopilot.speed)},
    {DATA_AUTOPILOT_SPEEDWINDOWOPEN, BOOLEAN, offsetof(AircraftState, autopilot.speedWindowOpen)},

    {DATA_AUTOPILOT_DISPLAYHEADING, BOOLEAN, offsetof(AircraftState, autopilot.displayHeading)},
    {DATA_AUTOPILOT_HEADINGMANAGED, BOOLEAN, offsetof(AircraftState, autopilot.headingManaged)},
    {DATA_AUTOPILOT_HEADINGWINDOWOPEN, BOOLEAN, offsetof(AircraftState, autopilot.headingWindowOpen)},
    {DATA_AUTOPILOT_HEADING, FLOAT, offsetof(AircraftState, autopilot.heading)},
    {DATA_AUTOPILOT_HEADINGTRKMODE, INTEGER, offsetof(AircraftState, autopilot.headingTrkMode)},
    {DATA_AUTOPILOT_GPSSSTATUS, INTEGER, offsetof(AircraftState, autopilot.gpssStatus)},

    {DATA_AUTOPILOT_DISPLAYALTITUDE, BOOLEAN, offsetof(AircraftState, autopilot.displayAltitude)},
    {DATA_AUTOPILOT_ALTITUDEMANAGED, BOOLEAN, offsetof(AircraftState, autopilot.altitudeManaged)},
    {DATA_AUTOPILOT_ALTITUDESTEP1000, BOOLEAN, offsetof(AircraftState, autopilot.altitudeStep1000)},
    {DATA_AUTOPILOT_ALTITUDE, FLOAT, offsetof(AircraftState, autopilot.altitude)},
    {DATA_AUTOPILOT_FMSVNAV, BOOLEAN, offsetof(AircraftState, autopilot.fmsVnav)},

    {DATA_AUTOPILOT_DISPLAYVERTICALSPEED, BOOLEAN, offsetof(AircraftState, autopilot.displayVerticalSpeed)},
    {DATA_AUTOPILOT_VERTICALSPEED, FLOAT, offsetof(AircraftState, autopilot.verticalSpeed)},
    {DATA_AUTOPILOT_VERTICALSPEEDFPAMODE, INTEGER, offsetof(AircraftState, autopilot.verticalSpeedFPAMode)},

    {DATA_AUTOPILOT_LOCMODE, BOOLEAN, offsetof(AircraftState, autopilot.locMode)},
    {DATA_AUTOPILOT_AP1MODE, BOOLEAN, offsetof(AircraftState, autopilot.ap1Mode)},
    {DATA_AUTOPILOT_AP2MODE, BOOLEAN, offsetof(AircraftState, autopilot.ap2Mode)},
    {DATA_AUTOPILOT_AUTOTHROTTLEMODE, BOOLEAN, offsetof(AircraftState, autopilot.autoThrottleMode)},
    {DATA_AUTOPILOT_APPROACHMODE, BOOLEAN, offsetof(AircraftState, autopilot.approachMode)},

    {DATA_CABIN_CALL, BOOLEAN, offsetof(AircraftState, cabin.call)},
    {DATA_CABIN_SEATBELTSIGN, BOOLEAN, offsetof(AircraftState, cabin.seatBeltSign)},
    {DATA_CABIN_NOSMOKINGSIGN, BOOLEAN, offsetof(AircraftState, cabin.noSmokingSign)},
    {DATA_WEIGHT_PASSENGERCOUNT, BOOLEAN, offsetof(AircraftState, weight.passengerCount)},
    {DATA_WEIGHT_PASSENGERDISTRIBUTION, BOOLEAN, offsetof(AircraftState, weight.passengerDistribution)},

    // These are not polled!
    {DATA_FLIGHTPLAN_FLIGHTNUMBER, STRING, -1, 10},
    {DATA_FLIGHTPLAN_CRUISEALTITUDE, INTEGER, -1},
    {DATA_FLIGHTPLAN_DEPARTUREAIRPORT, STRING, -1, 4},
    {DATA_FLIGHTPLAN_DESTINATIONAIRPORT, STRING, -1, 4},
    {DATA_FLIGHTPLAN_DISTANCETODEST, FLOAT, offsetof(AircraftState, flightPlan.distanceToDestination)},
};
}

#endif //DATADEFS_H
