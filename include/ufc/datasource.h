//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_DATASOURCE_H
#define XPFD_DATASOURCE_H

#include <ufc/aircraftstate.h>

#include <mutex>

#include "logger.h"

namespace UFC
{

const std::string AUTOPILOT_AIRSPEED_UP = "autopilot/airspeedUp";
const std::string AUTOPILOT_AIRSPEED_DOWN = "autopilot/airspeedDown";
const std::string AUTOPILOT_HEADING_UP = "autopilot/headingUp";
const std::string AUTOPILOT_HEADING_DOWN = "autopilot/headingDown";
const std::string AUTOPILOT_ALTITUDE_UP = "autopilot/altitudeUp";
const std::string AUTOPILOT_ALTITUDE_DOWN = "autopilot/altitudeDown";

const std::string SOURCE_SIMULATOR = "sim";
const std::string SOURCE_XPLANE = "xplane";

class DataSource : public Logger
{
 protected:
    std::string m_name;
    std::mutex m_stateMutex;
    AircraftState m_state;

 public:
    explicit DataSource(std::string name) : Logger("DataSource[" + name + "]"), m_name(name) {}
    ~DataSource() override = default;

    AircraftState getState()
    {
        std::scoped_lock lock(m_stateMutex);
        return m_state;
    }

    void updateState(AircraftState& state)
    {
        std::scoped_lock lock(m_stateMutex);
        m_state = state;
    }

    virtual bool init() = 0;
    virtual void close() = 0;
    virtual bool update() = 0;

    virtual void command(std::string command) {}
};

}

#endif //XPFD_DATASOURCE_H
