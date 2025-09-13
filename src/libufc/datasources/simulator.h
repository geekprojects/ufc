//
// Created by Ian Parker on 29/01/2024.
//

#ifndef XPFD_SIMULATOR_H
#define XPFD_SIMULATOR_H

#include <ufc/datasource.h>

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

struct APUState
{
    bool masterOn = false;
    bool starterOn = false;
};

class SimulatorDataSource : public DataSource
{
    AutopilotState m_autopilot;
    CommunicationState m_communication;
    APUState m_apu;

 public:
    explicit SimulatorDataSource(FlightConnector* flightConnector);
    ~SimulatorDataSource() override = default;

    bool connect() override;
    void disconnect() override;


    bool update() override;

    void command(const std::string& command) override;
};

}

#endif //XPFD_SIMULATOR_H
