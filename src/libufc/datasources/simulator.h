//
// Created by Ian Parker on 29/01/2024.
//

#ifndef XPFD_SIMULATOR_H
#define XPFD_SIMULATOR_H

#include <ufc/datasource.h>

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

    void executeCommand(const std::string& command, const CommandDefinition& commandDefinition) override;
};

}

#endif //XPFD_SIMULATOR_H
