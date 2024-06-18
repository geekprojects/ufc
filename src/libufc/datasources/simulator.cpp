//
// Created by Ian Parker on 29/01/2024.
//

#include "simulator.h"

#include <unistd.h>

using namespace UFC;

SimulatorDataSource::SimulatorDataSource(FlightConnector* flightConnector) : DataSource(flightConnector, "Simulator")
{

}

bool SimulatorDataSource::init()
{
    AircraftState state = m_flightConnector->getState();
    state.connected = true;
    m_flightConnector->updateState(state);
    return true;
}

void SimulatorDataSource::close()
{
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

    m_flightConnector->updateState(state);

    while (true)
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
        m_flightConnector->updateState(state);
        usleep(50000);
    }
    return true;
}
