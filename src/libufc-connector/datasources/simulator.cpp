//
// Created by Ian Parker on 29/01/2024.
//

#include "simulator.h"

#include <ufc/flightconnector.h>
#include <ufc/commands.h>

#include <unistd.h>

using namespace UFC;

UFC_DATA_SOURCE(Simulator, SimulatorDataSource)

SimulatorDataSource::SimulatorDataSource(FlightConnector* flightConnector) : DataSource(flightConnector, "Simulator", "")
{

}

bool SimulatorDataSource::connect()
{
    setRunning(true);
    return true;
}

void SimulatorDataSource::disconnect()
{
    printf("SimulatorDataSource::disconnect: Stopping...\n");
    setRunning(false);
}

bool SimulatorDataSource::update()
{
    int rollDir = 1;
    int pitchDir = 1;
    auto state = getFlightConnector()->getState();

    state->set(DATA_AIRCRAFT_PITCH, 10.0f);

    state->set(DATA_AUTOPILOT_FLIGHTDIRECTOR_PILOT_ON, m_flightDirector);
    state->set(DATA_AUTOPILOT_FLIGHTDIRECTOR_MODE, 1);
    state->set(DATA_AUTOPILOT_FLIGHTDIRECTOR_PITCH, 0.0f);
    state->set(DATA_AUTOPILOT_FLIGHTDIRECTOR_ROLL, 0.0f);

    state->set(DATA_COMMS_COM1HZ, 118900);
    state->set(DATA_COMMS_COM1STANDBYHZ, 136125);

    state->set(DATA_AUTOPILOT_SPEED, 50.0f);
    state->set(DATA_AUTOPILOT_ALTITUDE, 1000.0f);
    state->set(DATA_AUTOPILOT_HEADING, 90.0f);

    m_communication.com1Hz = 118900;
    m_communication.com1StandbyHz = 136125;

    while (isRunning())
    {
        state = getFlightConnector()->getState();

        float roll = state->getFloat(DATA_AUTOPILOT_FLIGHTDIRECTOR_ROLL);
        if (rollDir == 1)
        {
            roll += 1;
            if (roll >= 45)
            {
                rollDir = -1;
            }
        }
        else
        {
            roll -= 1;
            if (roll <= -45)
            {
                rollDir = 1;
            }
        }
        state->set(DATA_AUTOPILOT_FLIGHTDIRECTOR_ROLL, roll);

        float pitch = state->getFloat(DATA_AUTOPILOT_FLIGHTDIRECTOR_PITCH);
        if (pitchDir == 1)
        {
            pitch += 0.1f;
            if (pitch > 20.0f)
            {
                pitchDir = -1;
            }
        }
        else
        {
            pitch -= 0.1f;
            if (pitch < -20.0f)
            {
                pitchDir = 1;
            }
        }
        state->set(DATA_AUTOPILOT_FLIGHTDIRECTOR_PITCH, pitch);

        state->set(DATA_AIRCRAFT_INDICATEDAIRSPEED, state->getFloat(DATA_AIRCRAFT_INDICATEDAIRSPEED) + 0.1f);
        state->set(DATA_AIRCRAFT_ALTITUDE, state->getFloat(DATA_AIRCRAFT_ALTITUDE) + 0.5f);
        state->set(DATA_AIRCRAFT_MAGHEADING, state->getFloat(DATA_AIRCRAFT_MAGHEADING) + 0.2f);
        state->set(DATA_AIRCRAFT_VERTICALSPEED, pitch * 400.0f);

        state->set(DATA_AUTOPILOT_HEADING, m_autopilot.heading);
        state->set(DATA_AUTOPILOT_SPEED, m_autopilot.speed);
        state->set(DATA_AUTOPILOT_ALTITUDE, m_autopilot.altitude);
        state->set(DATA_AUTOPILOT_ALTITUDE_STEP_1000, m_autopilot.altitudeStep1000);
        state->set(DATA_AUTOPILOT_AP1MODE, m_autopilot.ap1Mode);
        state->set(DATA_AUTOPILOT_AP2MODE, m_autopilot.ap2Mode);

        state->set(DATA_COMMS_COM1HZ, (int)m_communication.com1Hz);
        state->set(DATA_COMMS_COM1STANDBYHZ, (int)m_communication.com1StandbyHz);

        state->set(DATA_APU_MASTER_ON, m_apu.masterOn);
        state->set(DATA_APU_STARTER_ON, m_apu.starterOn);

        state->set(DATA_COMMS_COM1HZ, (int)m_communication.com1Hz);
        state->set(DATA_COMMS_COM1STANDBYHZ, (int)m_communication.com1StandbyHz);

        state->set(DATA_AIRCRAFT_BAROMETER_PILOT_IN_HG, m_baro);
        state->set(DATA_AIRCRAFT_BAROMETER_PILOT_STD, m_baroStd);
        state->set(DATA_AIRCRAFT_BAROMETER_PILOT_MODE, m_baroMode);

        state->set(DATA_AUTOPILOT_FLIGHTDIRECTOR_PILOT_ON, m_flightDirector);
        state->set("efis/display/ls", m_ls);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return true;
}

void SimulatorDataSource::command(const std::string& command)
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
        auto hz = (int)(m_communication.com1StandbyHz % 1000);
        m_communication.com1StandbyHz -= hz;
        hz += 25;
        if (hz >= 1000)
        {
            hz -= 1000;
        }
        m_communication.com1StandbyHz = m_communication.com1StandbyHz + hz;
    }
    else if (command == COMMS_COM1_STANDBY_DOWN_FINE)
    {
        auto hz = (int)(m_communication.com1StandbyHz % 1000);
        m_communication.com1StandbyHz -= hz;
        hz -= 25;
        if (hz < 0)
        {
            hz += 1000;
        }
        m_communication.com1StandbyHz = m_communication.com1StandbyHz + hz;
    }
    else if (command == COMMS_COM1_SWAP)
    {
        uint32_t tmp = m_communication.com1Hz;
        m_communication.com1Hz = m_communication.com1StandbyHz;
        m_communication.com1StandbyHz = tmp;
    }
    else if (command == APU_MASTER_TOGGLE)
    {
        m_apu.masterOn = !m_apu.masterOn;
    }
    else if (command == APU_STARTER_TOGGLE)
    {
        m_apu.starterOn = !m_apu.starterOn;
    }
    else if (command == AIRCRAFT_BARO_PILOT_UP)
    {
        m_baro += 0.01f;
        m_baroStd = false;
    }
    else if (command == AIRCRAFT_BARO_PILOT_DOWN)
    {
        m_baro -= 0.01f;
        m_baroStd = false;
    }
    else if (command == AIRCRAFT_BARO_PILOT_STANDARD)
    {
        m_baro = 29.92f;
        m_baroStd = true;
    }
    else if (command == AIRCRAFT_BARO_PILOT_MODE_INHG)
    {
        m_baroMode = 0;
    }
    else if (command == AIRCRAFT_BARO_PILOT_MODE_HPA)
    {
        m_baroMode = 1;
    }
    else if (command == AIRCRAFT_BARO_PILOT_PUSH)
    {
        m_baroStd = false;
    }
    else if (command == "autopilot/flightDirector/toggle")
    {
        m_flightDirector = !m_flightDirector;
    }
    else if (command == "efis/ls/toggle")
    {
        m_ls = !m_ls;
    }
    else
    {
        printf("SimulatorDataSource::command: Unknown command: %s\n", command.c_str());
    }


    m_autopilot = autopilot;
}
