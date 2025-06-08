//
// Created by Ian Parker on 20/01/2024.
//

#ifndef UFC_STATE_H
#define UFC_STATE_H

#include <string>
#include <cstdint>

#define DATA_AIRCRAFT_INDICATEDAIRSPEED "aircraft/indicatedAirspeed"
#define DATA_AIRCRAFT_PITCH "aircraft/pitch"
#define DATA_AIRCRAFT_ROLL "aircraft/roll"
#define DATA_AIRCRAFT_ALTITUDE "aircraft/altitude"
#define DATA_AIRCRAFT_VERTICALSPEED "aircraft/verticalSpeed"
#define DATA_AIRCRAFT_INDICATEDMACH "aircraft/indicatedMach"
#define DATA_AIRCRAFT_MAGHEADING "aircraft/magHeading"
#define DATA_AIRCRAFT_BAROMETERHG "aircraft/barometerHG"
#define DATA_AIRCRAFT_PARKINGBRAKE "aircraft/parkingBrake"
#define DATA_COMMS_COM1HZ "comms/com1Hz"
#define DATA_COMMS_COM1STANDBYHZ "comms/com1StandbyHz"
#define DATA_COMMS_COM2HZ "comms/com2Hz"
#define DATA_COMMS_COM2STANDBYHZ "comms/com2StandbyHz"
#define DATA_COMMS_NAV1HZ "comms/nav1Hz"
#define DATA_COMMS_NAV1STANDBYHZ "comms/nav1StandbyHz"
#define DATA_COMMS_NAV2HZ "comms/nav2Hz"
#define DATA_COMMS_NAV2STANDBYHZ "comms/nav2StandbyHz"

#define DATA_FLIGHTDIRECTOR_PITCH "flightDirector/pitch"
#define DATA_FLIGHTDIRECTOR_ROLL "flightDirector/roll"
#define DATA_FLIGHTDIRECTOR_MODE "flightDirector/mode"

#define DATA_AUTOPILOT_DISPLAYSPEED "autopilot/displaySpeed"
#define DATA_AUTOPILOT_SPEEDMANAGED "autopilot/speedManaged"
#define DATA_AUTOPILOT_SPEEDMACH "autopilot/speedMach"
#define DATA_AUTOPILOT_SPEED "autopilot/speed"
#define DATA_AUTOPILOT_SPEEDWINDOWOPEN "autopilot/speedWindowOpen"

#define DATA_AUTOPILOT_DISPLAYHEADING "autopilot/displayHeading"
#define DATA_AUTOPILOT_HEADINGMANAGED "autopilot/headingManaged"
#define DATA_AUTOPILOT_HEADINGWINDOWOPEN "autopilot/headingWindowOpen"
#define DATA_AUTOPILOT_HEADING "autopilot/heading"
#define DATA_AUTOPILOT_HEADINGTRKMODE "autopilot/headingTrkMode"
#define DATA_AUTOPILOT_GPSSSTATUS "autopilot/gpssStatus"

#define DATA_AUTOPILOT_DISPLAYALTITUDE "autopilot/displayAltitude"
#define DATA_AUTOPILOT_ALTITUDEMANAGED "autopilot/altitudeManaged"
#define DATA_AUTOPILOT_ALTITUDESTEP1000 "autopilot/altitudeStep1000"
#define DATA_AUTOPILOT_ALTITUDE "autopilot/altitude"
#define DATA_AUTOPILOT_FMSVNAV "autopilot/fmsVnav"

#define DATA_AUTOPILOT_DISPLAYVERTICALSPEED "autopilot/displayVerticalSpeed"
#define DATA_AUTOPILOT_VERTICALSPEED "autopilot/verticalSpeed"
#define DATA_AUTOPILOT_VERTICALSPEEDFPAMODE "autopilot/verticalSpeedFPAMode"

#define DATA_AUTOPILOT_LOCMODE "autopilot/locMode"
#define DATA_AUTOPILOT_AP1MODE "autopilot/ap1Mode"
#define DATA_AUTOPILOT_AP2MODE "autopilot/ap2Mode"
#define DATA_AUTOPILOT_AUTOTHROTTLEMODE "autopilot/autoThrottleMode"
#define DATA_AUTOPILOT_APPROACHMODE "autopilot/approachMode"

#define DATA_CABIN_CALL "cabin/call"
#define DATA_CABIN_SEATBELTSIGN "cabin/seatBeltSign"
#define DATA_CABIN_NOSMOKINGSIGN "cabin/noSmokingSign"
#define DATA_WEIGHT_PASSENGERCOUNT "weight/passengerCount"
#define DATA_WEIGHT_PASSENGERDISTRIBUTION "weight/passengerDistribution"

#define DATA_FLIGHTPLAN_FLIGHTNUMBER "flightplan/flightNumber"
#define DATA_FLIGHTPLAN_CRUISEALTITUDE "flightplan/cruiseAltitude"
#define DATA_FLIGHTPLAN_DEPARTUREAIRPORT "flightplan/departureAirport"
#define DATA_FLIGHTPLAN_DESTINATIONAIRPORT "flightplan/destinationAirport"
#define DATA_FLIGHTPLAN_DISTANCETODEST "flightplan/distanceToDestination"

namespace UFC
{

struct AutopilotState
{
    bool displaySpeed = true;
    bool speedManaged = false;
    float speed = 0.0f;
    bool speedMach = false;

    bool displayHeading = true;
    bool headingManaged = false;
    bool headingTrkMode = false;
    bool headingWindowOpen = false;
    float heading = 0.0f;

    bool displayAltitude = true;
    bool altitudeManaged = false;
    bool altitudeStep1000 = false;
    float altitude = 0.0f;

    bool displayVerticalSpeed = true;
    bool verticalSpeedFPAMode = false;
    float verticalSpeed = 0.0f;

    bool fmsVnav = false;
    int gpssStatus = 0;
    bool speedWindowOpen = false;

    int locMode = 0;
    int ap1Mode = 0;
    int ap2Mode = 0;
    int autoThrottleMode = 0;
    int approachMode = 0;
};

struct CommunicationState
{
    uint32_t com1Hz = 0;
    uint32_t com1StandbyHz = 0;
    uint32_t com2Hz = 0;
    uint32_t com2StandbyHz = 0;
    uint32_t nav1Hz = 0;
    uint32_t nav1StandbyHz = 0;
    uint32_t nav2Hz = 0;
    uint32_t nav2StandbyHz = 0;
};

struct FlightDirectorState
{
    int mode = 0;
    float pitch = 0.0f;
    float roll = 0.0f;
};

struct CabinState
{
    bool call = false;
    bool seatBeltSign = false;
    bool noSmokingSign = false;
};

struct WeightState
{
    int passengerCount = 0;
    float passengerDistribution = 0.0;
};

struct FlightPlanState
{
    std::string flightNumber;
    std::string departureAirport;
    int cruiseAltitude = 0;
    std::string destinationAirport;
    float distanceToDestination = 0.0f;
};

struct InstrumentState
{
    float v1;
    float vr;
    float v2;
    float vfeNext;

    float radioAltitude;
};

struct AircraftState
{
    std::string dataSource;
    std::string dataSourceVersion;
    bool connected = false;

    std::string aircraftAuthor;
    std::string aircraftICAO;

    float indicatedAirspeed = 0.0f;
    float indicatedMach = 0.0f;
    float roll = 0.0f;
    float pitch = 0.0f;
    float altitude = 0.0f;
    float verticalSpeed = 0.0f;
    float magHeading = 0.0f;
    float barometerHG = 0.0f;
    bool parkingBrake = false;

    CommunicationState comms;
    FlightDirectorState flightDirector;
    AutopilotState autopilot;
    InstrumentState instrument;
    CabinState cabin;
    WeightState weight;
    FlightPlanState flightPlan;
};

}

#endif //XPFD_STATE_H
