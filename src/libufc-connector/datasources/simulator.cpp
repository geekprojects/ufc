//
// Created by Ian Parker on 29/01/2024.
//

#include "simulator.h"

#include <ufc/flightconnector.h>
#include <ufc/aircraftcommands.h>
#include <ufc/aircraftdata.h>

#include <unistd.h>

#include "ufc/utils/utils.h"

using namespace std;
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

        float roll = state->getFloat(DATA_AIRCRAFT_ROLL);
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
        state->set(DATA_AIRCRAFT_ROLL, roll);

        float pitch = state->getFloat(DATA_AIRCRAFT_PITCH);
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
        state->set(DATA_AIRCRAFT_PITCH, pitch);

        state->set(DATA_AIRCRAFT_INDICATEDAIRSPEED, state->getFloat(DATA_AIRCRAFT_INDICATEDAIRSPEED) + 0.1f);
        state->set(DATA_AIRCRAFT_ALTITUDE, state->getFloat(DATA_AIRCRAFT_ALTITUDE) + 0.5f);
        state->set(DATA_AIRCRAFT_MAGHEADING, state->getFloat(DATA_AIRCRAFT_MAGHEADING) + 0.2f);
        state->set(DATA_AIRCRAFT_VERTICALSPEED, pitch * 400.0f);

        state->set(DATA_AUTOPILOT_HEADING, m_autopilot.heading);
        state->set(DATA_AUTOPILOT_SPEED, m_autopilot.speed);
        state->set(DATA_AUTOPILOT_ALTITUDE, m_autopilot.altitude);
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

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        wstring title = L"⬡UFC FMS°";
        int spaces = (24 - title.length()) / 2;
        title = wstring(spaces, ' ') + title;
        while (title.length() < 24)
        {
            title += L" ";
        }

        std::wstringstream oss;
        oss << std::put_time(&tm, L"%d-%m-%Y %H:%M:%S");

        fmsPrint(state, 1, title, 'b', 'g');
        fmsPrint(state, 2, oss.str());

        fmsPrint(state, 3, L"< Detatch Engine");
        fmsPrint(state, 5, L"< Increase Turbulence");
        fmsPrint(state, 7, L"< Deploy Chemtrails");
        fmsPrint(state, 9, L"< Fire Missile");
        fmsPrint(state, 11, L"< Eject Random Passenger");
        fmsPrint(state, 13, L"< Inflate Otto Pilot");

        wstring scratchPad = m_scratchPad;
        if ((t % 2) == 0)
        {
            scratchPad += L"_";
        }
        else
        {
            scratchPad += L" ";
        }
        while (scratchPad.length() < 24)
        {
            scratchPad += L" ";
        }
        fmsPrint(state, 14, scratchPad, 'b', 'w');

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return true;
}

void SimulatorDataSource::fmsPrint(shared_ptr<AircraftState> state, int row, std::wstring text, char fg, char bg)
{
    state->set("fmc/0/line" + to_string(row) + "/text", text);
    state->set("fmc/0/line" + to_string(row) + "/textColour", wstring(24, fg));
    state->set("fmc/0/line" + to_string(row) + "/backgroundColour", wstring(24, bg));
}

void SimulatorDataSource::command(const std::string& command)
{
    AutopilotState autopilot = m_autopilot;
    if (command == COMMAND_AUTOPILOT_HEADING_UP)
    {
        autopilot.heading += 1.0f;
        if (autopilot.heading >= 360.0f)
        {
            autopilot.heading = 1.0f;
        }
    }
    else if (command == COMMAND_AUTOPILOT_HEADING_DOWN)
    {
        autopilot.heading -= 1.0f;
        if (autopilot.heading <= 1.0f)
        {
            autopilot.heading = 365.0f;
        }
    }
    else if (command == COMMAND_AUTOPILOT_AIRSPEED_UP)
    {
        if (autopilot.speed <= 400)
        {
            autopilot.speed += 1.0f;
        }
    }
    else if (command == COMMAND_AUTOPILOT_AIRSPEED_DOWN)
    {
        if (autopilot.speed > 100.0)
        {
            autopilot.speed -= 1.0f;
        }
    }
    else if (command == COMMAND_AUTOPILOT_ALTITUDE_UP)
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
    else if (command == COMMAND_AUTOPILOT_ALTITUDE_DOWN)
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
    else if (command == COMMAND_AUTOPILOT_ALTITUDE_STEP_100)
    {
        autopilot.altitudeStep1000 = false;
    }
    else if (command == COMMAND_AUTOPILOT_ALTITUDE_STEP_1000)
    {
        autopilot.altitudeStep1000 = true;
    }
    else if (command == COMMAND_AUTOPILOT_AP1_TOGGLE)
    {
        autopilot.ap1Mode = !autopilot.ap1Mode;
    }
    else if (command == COMMAND_AUTOPILOT_AP2_TOGGLE)
    {
        autopilot.ap2Mode = !autopilot.ap2Mode;
    }
    else if (command == COMMAND_COMMS_COM1_STANDBY_COARSE_UP)
    {
        m_communication.com1StandbyHz += 1000;
    }
    else if (command == COMMAND_COMMS_COM1_STANDBY_COARSE_DOWN)
    {
        m_communication.com1StandbyHz -= 1000;
    }
    else if (command == COMMAND_COMMS_COM1_STANDBY_FINE_UP)
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
    else if (command == COMMAND_COMMS_COM1_STANDBY_FINE_DOWN)
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
    else if (command == COMMAND_COMMS_COM1_SWAP)
    {
        uint32_t tmp = m_communication.com1Hz;
        m_communication.com1Hz = m_communication.com1StandbyHz;
        m_communication.com1StandbyHz = tmp;
    }
    else if (command == COMMAND_APU_MASTER_TOGGLE)
    {
        m_apu.masterOn = !m_apu.masterOn;
    }
    else if (command == COMMAND_APU_STARTER_TOGGLE)
    {
        m_apu.starterOn = !m_apu.starterOn;
    }
    else if (command == COMMAND_AIRCRAFT_BAROMETER_PILOT_UP)
    {
        m_baro += 0.01f;
        m_baroStd = false;
    }
    else if (command == COMMAND_AIRCRAFT_BAROMETER_PILOT_DOWN)
    {
        m_baro -= 0.01f;
        m_baroStd = false;
    }
    else if (command == COMMAND_AIRCRAFT_BAROMETER_PILOT_STD)
    {
        if (m_baroStd)
        {
            m_baroStd = false;
        }
        else
        {
            m_baro = 29.92f;
            m_baroStd = true;
        }
    }
    else if (command == COMMAND_AIRCRAFT_BAROMETER_PILOT_MODE_INHG)
    {
        m_baroMode = 0;
    }
    else if (command == COMMAND_AIRCRAFT_BAROMETER_PILOT_MODE_HPA)
    {
        m_baroMode = 1;
    }
    else if (command == "aircraf/barometer/pilot/push")
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
    else if (command.starts_with("fmc/0/"))
    {
        if (command.starts_with("fmc/0/key") || command == ("fmc/0/space"))
        {
            wstring key = L" ";
            if (command.starts_with("fmc/0/key"))
            {
                key = utf82wstring(command.substr(9).c_str());
            }
            m_scratchPad += key;
            if (m_scratchPad.length() > 23)
            {
                m_scratchPad = m_scratchPad.substr(1);
            }
        }
        else if (command.starts_with("fmc/0/clear"))
        {
            if (!m_scratchPad.empty())
            {
                m_scratchPad = m_scratchPad.substr(0, m_scratchPad.length() - 1);
            }
        }
        else
        {
            m_scratchPad = utf82wstring(command.c_str());
            if (m_scratchPad.length() > 23)
            {
                m_scratchPad = m_scratchPad.substr(0, 23);
            }
        }
    }
    else
    {
        printf("SimulatorDataSource::command: Unknown command: %s\n", command.c_str());
    }


    m_autopilot = autopilot;
}

