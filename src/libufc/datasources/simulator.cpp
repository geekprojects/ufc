//
// Created by Ian Parker on 29/01/2024.
//

#include "simulator.h"
#include <ufc/flightconnector.h>

#include <unistd.h>

using namespace UFC;

UFC_DATA_SOURCE(Simulator, SimulatorDataSource)

SimulatorDataSource::SimulatorDataSource(FlightConnector* flightConnector) : DataSource(flightConnector, "Simulator", 0)
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

    m_communication.com1Hz = 118900;
    m_communication.com1StandbyHz = 136125;

    m_autopilot.speed = 100.0f;
    m_autopilot.altitude = 1000.0f;
    m_autopilot.heading = 90.0f;

    state.autopilot = m_autopilot;
    state.comms = m_communication;

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
        state.comms = m_communication;
        m_flightConnector->updateState(state);
        usleep(50000);
    }
    return true;
}

void SimulatorDataSource::command(std::string command)
{
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
    else if (command == AUTOPILOT_AIRSPEED_UP)
    {
        if (autopilot.speed <= 400)
        {
            autopilot.speed += 1.0f;
        }
    }
    else if (command == AUTOPILOT_AIRSPEED_DOWN)
    {
        if (autopilot.speed > 100.0)
        {
            autopilot.speed -= 1.0f;
        }
    }
    else if (command == AUTOPILOT_ALTITUDE_UP)
    {
        if (m_autopilot.altitudeStep1000)
        {
            autopilot.altitude += 1000.0f;
        }
        else
        {
            autopilot.altitude += 100.0f;

        }
    }
    else if (command == AUTOPILOT_ALTITUDE_DOWN)
    {
        if (m_autopilot.altitudeStep1000)
        {
            autopilot.altitude -= 1000.0f;
        }
        else
        {
            autopilot.altitude -= 100.0f;
        }
    }
    else if (command == AUTOPILOT_ALTITUDE_STEP_100)
    {
        autopilot.altitudeStep1000 = false;
    }
    else if (command == AUTOPILOT_ALTITUDE_STEP_1000)
    {
        autopilot.altitudeStep1000 = true;
    }
    else if (command == AUTOPILOT_AP1_TOGGLE)
    {
        autopilot.ap1Mode = !autopilot.ap1Mode;
    }
    else if (command == AUTOPILOT_AP2_TOGGLE)
    {
        autopilot.ap2Mode = !autopilot.ap2Mode;
    }
    else if (command == COMMS_COM1_STANDBY_UP_COARSE)
    {
        m_communication.com1StandbyHz += 1000;
    }
    else if (command == COMMS_COM1_STANDBY_DOWN_COARSE)
    {
        m_communication.com1StandbyHz -= 1000;
    }
    else if (command == COMMS_COM1_STANDBY_UP_FINE)
    {
        m_communication.com1StandbyHz += 25;
    }
    else if (command == COMMS_COM1_STANDBY_DOWN_FINE)
    {
        m_communication.com1StandbyHz -= 25;
    }
    else if (command == COMMS_COM1_SWAP)
    {
        uint32_t tmp = m_communication.com1Hz;
        m_communication.com1Hz = m_communication.com1StandbyHz;
        m_communication.com1StandbyHz = tmp;
    }
    else
    {
        printf("SimulatorDataSource::command: Unknown command: %s\n", command.c_str());
    }


    m_autopilot = autopilot;
}
