//
// Created by Ian Parker on 16/10/2024.
//

#include "clock.h"

#include <ctime>
#include <unistd.h>

#include <ufc/flightconnector.h>
#include <ufc/commands.h>

using namespace std;
using namespace UFC;

UFC_DATA_SOURCE(Clock, ClockDataSource)

ClockDataSource::ClockDataSource(FlightConnector* fc) :
    DataSource(fc, "Clock", "", 0)
{
}

bool ClockDataSource::connect()
{
    return true;
}

void ClockDataSource::disconnect()
{
    m_running = false;
}

bool ClockDataSource::update()
{
    while (m_running)
    {
        AircraftState state = m_flightConnector->getState();

        state.connected = true;

        time_t timestamp;
        time(&timestamp);

        struct tm local_time = {};
        localtime_r(&timestamp, &local_time);

        int hour = local_time.tm_hour;
        if (!m_24Hour && hour > 12)
        {
            hour -= 12;
            state.autopilot.speedManaged = true;
        }
        else
        {
            state.autopilot.speedManaged = false;
        }

        int minutes = local_time.tm_min;

        if (m_showSeconds)
        {
            state.autopilot.speed = hour * 10;
            state.autopilot.speed += minutes / 10;

            state.autopilot.heading = (minutes % 10) * 100;
            state.autopilot.heading += local_time.tm_sec;
        }
        else
        {
            state.autopilot.speed = hour;
            state.autopilot.heading = minutes;
        }
        state.autopilot.headingManaged = !(local_time.tm_sec % 2);

        if (m_showYear)
        {
            state.autopilot.altitude = local_time.tm_year + 1900;

            time_t diff = timestamp - m_showYearTime;
            if (diff >= 5)
            {
                m_showYear = false;
            }
        }
        else
        {
            state.autopilot.altitude = local_time.tm_mday;
        }
        state.autopilot.verticalSpeed = (local_time.tm_mon + 1) * 100;

        state.comms.com1Hz = (hour * 10000) + (minutes * 100) + (local_time.tm_sec);
        state.comms.com1StandbyHz = (local_time.tm_mday * 10000) + ((local_time.tm_mon + 1) * 100) + ((local_time.tm_year + 1900) % 100);

        m_flightConnector->updateState(state);

        this_thread::sleep_for(chrono::milliseconds(100));
    }

    return true;
}

void ClockDataSource::executeCommand(const std::string& command, const CommandDefinition& commandDefinition)
{
    log(DEBUG, "command: %s", command.c_str());
    if (command == AUTOPILOT_ALTITUDE_MANAGE)
    {
        m_showYear = true;
        m_showYearTime = time(nullptr);
    }
    else if (command == AUTOPILOT_HEADING_MANAGE)
    {
        m_showSeconds = !m_showSeconds;
    }
    else if (command == AUTOPILOT_AIRSPEED_MANAGE)
    {
        m_24Hour = !m_24Hour;
    }
}
