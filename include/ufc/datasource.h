//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_DATASOURCE_H
#define XPFD_DATASOURCE_H

#include <memory>

#include <ufc/aircraftstate.h>
#include <ufc/commands.h>

#include "logger.h"

namespace UFC
{
class Airports;
class FlightConnector;

const std::string SOURCE_SIMULATOR = "Simulator";
const std::string SOURCE_XPLANE = "XPlane";

class DataSource : public Logger
{
 protected:
    FlightConnector* m_flightConnector;
    std::string m_name;
    bool m_running = true;

 public:
    explicit DataSource(FlightConnector* flightConnector, const std::string& name) :
        Logger("DataSource[" + name + "]"),
        m_flightConnector(flightConnector),
        m_name(name)
    {}
    ~DataSource() override = default;

    virtual std::shared_ptr<Airports> loadAirports() { return nullptr; }

    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool update() = 0;

    virtual void command(std::string command) {}

};

}

#endif //XPFD_DATASOURCE_H
