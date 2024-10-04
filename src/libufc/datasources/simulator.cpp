//
// Created by Ian Parker on 29/01/2024.
//

#include "simulator.h"
#include <ufc/flightconnector.h>

#include <unistd.h>

using namespace UFC;

SimulatorDataSource::SimulatorDataSource(FlightConnector* flightConnector) : DataSource(flightConnector, "Simulator")
{

}

bool SimulatorDataSource::connect()
{
    AircraftState state = m_flightConnector->getState();
    state.connected = true;
    m_flightConnector->updateState(state);
    return true;
}

void SimulatorDataSource::disconnect()
{
    printf("SimulatorDataSource::disconnect: Stopping...\n");
    m_running = false;
}

bool SimulatorDataSource::update()
{
    int rollDir = 1;
    int pitchDir = 1;
    AircraftState state = m_flightConnector->getState();
    state.connected = true;

    state.pitch = 10.0f;

    state.flightDirector.mode = 1;
    state.flightDirector.pitch = 0.0f;
    state.flightDirector.roll = 0.0f;

    state.comms.com1Hz = 234.432;
    state.comms.com1StandbyHz = 123.123;

    m_flightConnector->updateState(state);

    while (m_running)
    {
        state = m_flightConnector->getState();
        state.connected = true;
        if (rollDir == 1)
        {
            state.roll += 1;
            if (state.roll >= 45)
            {
                rollDir = -1;
            }
        }
        else
        {
            state.roll -= 1;
            if (state.roll <= -45)
            {
                rollDir = 1;
            }
        }
        if (pitchDir == 1)
        {
            state.pitch += 0.1;
            if (state.pitch > 20.0f)
            {
                pitchDir = -1;
            }
        }
        else
        {
            state.pitch -= 0.1;
            if (state.pitch < -20.0f)
            {
                pitchDir = 1;
            }
        }
        state.autopilot = m_autopilot;
        m_flightConnector->updateState(state);
        usleep(50000);
    }
    return true;
}

void SimulatorDataSource::command(std::string command)
{
    printf("SimulatorDataSource::command: %s\n", command.c_str());
    AutopilotState autopilot = m_autopilot;
    if (command == AUTOPILOT_HEADING_UP)
    {
        autopilot.heading += 1.0f;
        if (autopilot.heading >= 360.0f)
        {
            autopilot.heading = 1.0f;
        }
    }
    else if (command == AUTOPILOT_HEADING_DOWN)
    {
        autopilot.heading -= 1.0f;
        if (autopilot.heading <= 1.0f)
        {
            autopilot.heading = 365.0f;
        }
    }
    m_autopilot = autopilot;
}
