//
// Created by Ian Parker on 24/10/2024.
//

#ifndef UFC_NAVAIDS_H
#define UFC_NAVAIDS_H

#include <ufc/utils/geoutils.h>
#include <ufc/utils/logger.h>

#include <map>

namespace UFC
{

class FlightConnector;
class Airports;
class NavAids;
class Procedures;
class Airways;

class NavDataSource : public Logger
{
    FlightConnector* m_flightConnector;

public:
    explicit NavDataSource(FlightConnector* flightConnector, std::string name) : Logger(name), m_flightConnector(flightConnector)
    {
    }

    ~NavDataSource() override = default;

    [[nodiscard]] FlightConnector* getFlightConnector() const { return m_flightConnector; }

    virtual std::shared_ptr<Airports> getAirports() { return nullptr; }
    virtual std::shared_ptr<NavAids> getNavAids() { return nullptr; }
    virtual std::shared_ptr<Procedures> getProcedures() { return nullptr; }
    virtual std::shared_ptr<Airways> getAirways() { return nullptr; }
};

struct NavDataHeader
{
    int version;
    int cycle;
    int build;
    std::string type;
    std::string copyright;
};

class NavData : public Logger
{
 protected:
    NavDataSource* m_navDataSource;
    NavDataHeader m_header = {};

    [[nodiscard]] FlightConnector* getFlightConnector() const { return m_navDataSource->getFlightConnector(); }

 public:
    explicit NavData(NavDataSource* navdataSource, std::string const& name) :
        Logger("NavData[" + name + "]"),
        m_navDataSource(navdataSource)
    {}

    void setHeader(const NavDataHeader& header) { m_header = header; }
    [[nodiscard]] int getCycle() const { return m_header.cycle; }
};

};

#endif //NAVAIDS_H
