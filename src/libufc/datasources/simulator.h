//
// Created by Ian Parker on 29/01/2024.
//

#ifndef XPFD_SIMULATOR_H
#define XPFD_SIMULATOR_H

#include <ufc/datasource.h>

#include "ufc/flightconnector.h"

namespace UFC
{

class SimulatorDataSource : public DataSource
{
    AutopilotState m_autopilot;
    CommunicationState m_communication;

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
