//
// Created by Ian Parker on 20/01/2024.
//

#ifndef XPFD_STATE_H
#define XPFD_STATE_H

#include <string>
#include <cstdint>

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
    uint32_t com1Hz = 0.0;
    uint32_t com1StandbyHz = 0.0;
    uint32_t com2Hz = 0.0;
    uint32_t com2StandbyHz = 0.0;
    uint32_t nav1Hz = 0.0;
    uint32_t nav1StandbyHz = 0.0;
    uint32_t nav2Hz = 0.0;
    uint32_t nav2StandbyHz = 0.0;
};

struct FlightDirectorState
{
    int mode = 0;
    float pitch = 0.0f;
    float roll = 0.0f;
};

struct AircraftState
{
    std::string aircraftAuthor;
    std::string aircraftICAO;

    bool connected = false;
    float indicatedAirspeed = 0.0f;
    float indicatedMach = 0.0f;
    float roll = 0.0f;
    float pitch = 0.0f;
    float altitude = 0.0f;
    float verticalSpeed = 0.0f;
    float magHeading = 0.0f;
    float barometerHG = 0.0f;

    CommunicationState comms;
    FlightDirectorState flightDirector;
    AutopilotState autopilot;
};

}

#endif //XPFD_STATE_H
