//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_DATASOURCE_H
#define XPFD_DATASOURCE_H

#include <ufc/aircraftstate.h>

#include <mutex>

#include "flightconnector.h"
#include "logger.h"

namespace UFC
{

class FlightConnector;

const std::string AUTOPILOT_AIRSPEED_UP = "autopilot/airspeedUp";
const std::string AUTOPILOT_AIRSPEED_DOWN = "autopilot/airspeedDown";
const std::string AUTOPILOT_HEADING_UP = "autopilot/headingUp";
const std::string AUTOPILOT_HEADING_DOWN = "autopilot/headingDown";
const std::string AUTOPILOT_ALTITUDE_UP = "autopilot/altitudeUp";
const std::string AUTOPILOT_ALTITUDE_DOWN = "autopilot/altitudeDown";

const std::string SOURCE_SIMULATOR = "Simulator";
const std::string SOURCE_XPLANE = "XPlane";

class DataSource : public Logger
{
 protected:
    FlightConnector* m_flightConnector;
    std::string m_name;

 public:
    explicit DataSource(FlightConnector* flightConnector, std::string name) :
        Logger("DataSource[" + name + "]"),
        m_flightConnector(flightConnector),
        m_name(name)
    {}
    ~DataSource() override = default;


    virtual bool init() = 0;
    virtual void close() = 0;
    virtual bool update() = 0;

    virtual void command(std::string command) {}
};

}

#endif //XPFD_DATASOURCE_H
