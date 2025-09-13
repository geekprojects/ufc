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
    DataSource(fc, "Clock", "")
{
}

bool ClockDataSource::connect()
{
    setRunning(true);
    return true;
}

void ClockDataSource::disconnect()
{
    setRunning(false);
}

bool ClockDataSource::update()
{
    while (isRunning())
    {
        auto state = getFlightConnector()->getState();

        time_t timestamp;
        time(&timestamp);

        struct tm local_time = {};
        localtime_r(&timestamp, &local_time);

        int hour = local_time.tm_hour;
        if (!m_24Hour && hour > 12)
        {
            hour -= 12;
            state->set(DATA_AUTOPILOT_SPEEDMANAGED, true);
        }
        else
        {
            state->set(DATA_AUTOPILOT_SPEEDMANAGED, false);
        }

        int minutes = local_time.tm_min;

        int speed;
        int heading;
        if (m_showSeconds)
        {
            speed = hour * 10;
            speed += minutes / 10;

            heading = (minutes % 10) * 100;
            heading += local_time.tm_sec;
        }
        else
        {
            speed = hour;
            heading = minutes;
        }
        state->set(DATA_AUTOPILOT_SPEED, speed);
        state->set(DATA_AUTOPILOT_HEADING, heading);


        state->set(DATA_AUTOPILOT_HEADINGMANAGED, !(local_time.tm_sec % 2));

        if (m_showYear)
        {
            state->set(DATA_AUTOPILOT_ALTITUDE, local_time.tm_year + 1900);

            time_t diff = timestamp - m_showYearTime;
            if (diff >= 5)
            {
                m_showYear = false;
            }
        }
        else
        {
            state->set(DATA_AUTOPILOT_ALTITUDE, local_time.tm_mday);
        }
        state->set(DATA_AUTOPILOT_VERTICALSPEED, (local_time.tm_mon + 1) * 100);

        state->set(DATA_COMMS_COM1HZ, (hour * 10000) + (minutes * 100) + (local_time.tm_sec));
        state->set(DATA_COMMS_COM1STANDBYHZ, (local_time.tm_mday * 10000) + ((local_time.tm_mon + 1) * 100) + ((local_time.tm_year + 1900) % 100));

        this_thread::sleep_for(chrono::milliseconds(100));
    }

    return true;
}

void ClockDataSource::command(const std::string& command)
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
