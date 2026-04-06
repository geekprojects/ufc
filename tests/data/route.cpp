//
// Created by Ian Parker on 18/03/2026.
//

#include <gtest/gtest.h>

#include "ufc/datasource.h"
#include "ufc/data/airports.h"
#include "ufc/data/airways.h"
#include "ufc/data/lnmnavdata.h"
#include "ufc/data/procedures.h"
#include "ufc/data/routetextformat.h"

using namespace std;
using namespace UFC;

class TestAirports : public Airports
{
 public:
    TestAirports(NavDataSource* navDataSource) : Airports(navDataSource, "TestAirports")
    {
        auto heathrow = make_shared<Airport>();
        heathrow->setICAOCode("EGKK");
        heathrow->setName(L"London Gatwick");
        heathrow->setLocation(Coordinate(51.148055556, -0.190277778));
        addAirport(heathrow);

        auto jfk = make_shared<Airport>();
        jfk->setICAOCode("LFPG");
        jfk->setName(L"Paris - Charles De Gaulle");
        jfk->setLocation(Coordinate(49.009747222, 2.547819444));
        addAirport(jfk);
    }
    ~TestAirports() override = default;
};

class TestNavAids : public NavAids
{
 public:
    TestNavAids(NavDataSource* navDataSource) : NavAids(navDataSource, "TestNavAids")
    {
        auto navAid = make_shared<NavAid>();
        navAid->setId("SFD");
        navAid->setName("SFD");
        navAid->setLocation(Coordinate(50.760688889, 0.121913889));
        addNavAid(navAid);

        navAid = make_shared<NavAid>();
        navAid->setId("SAM");
        navAid->setName("SAM");
        navAid->setLocation(Coordinate(50.955250000, -1.345055556));
        addNavAid(navAid);

        navAid = make_shared<NavAid>();
        navAid->setId("XIDIL");
        navAid->setName("XIDIL");
        navAid->setLocation(Coordinate(50.351708333, 0.641491667));
        addNavAid(navAid);

        navAid = make_shared<NavAid>();
        navAid->setId("BIBAX");
        navAid->setName("BIBAX");
        navAid->setLocation(Coordinate(50.057500000, 1.008055556));
        addNavAid(navAid);
    }
};

class TestAirways : public Airways
{
 public:
    TestAirways(NavDataSource* navDataSource) : Airways(navDataSource, "TestAirways")
    {
        auto airway = make_shared<Airway>();
        airway->ident = "M605";
        addAirway(airway);

        airway = make_shared<Airway>();
        airway->ident = "UM605";
        addAirway(airway);
    }
};

class TestProcedures : public Procedures
{
 public:
    TestProcedures(NavDataSource* navDataSource) : Procedures(navDataSource, "TestProcedures") {}

    shared_ptr<Procedure> getDeparture(const std::string &airportCode, const std::string &name) override
    {
        if (name == "SFD4Z")
        {
            return make_shared<Procedure>();
        }
        return nullptr;
    }

    shared_ptr<Procedure> getArrival(const std::string &airportCode, const std::string &name) override
    {
        if (name == "BIBA9W")
        {
            return make_shared<Procedure>();
        }
        return nullptr;
    }
};

class TestNavDataSource : public NavDataSource
{
    shared_ptr<Airports> m_airports;
    shared_ptr<NavAids> m_navAids;
    shared_ptr<Procedures> m_procedures;
    shared_ptr<Airways> m_airways;
 public:
    TestNavDataSource(FlightConnector* flightConnector)
        : NavDataSource(flightConnector, "TestData")
    {
        m_airports = make_shared<TestAirports>(this);
        m_navAids = make_shared<TestNavAids>(this);
        m_procedures = make_shared<TestProcedures>(this);
        m_airways = make_shared<TestAirways>(this);
    }

    ~TestNavDataSource() override = default;

    std::shared_ptr<Airports> getAirports() override { return m_airports; }
    std::shared_ptr<NavAids> getNavAids() override { return m_navAids; }
    std::shared_ptr<Procedures> getProcedures() override { return m_procedures; }

    std::shared_ptr<Airways> getAirways() override { return m_airways; }
};

TEST(RouteParser, Route)
{
    //TestNavDataSource testNavDataSource(nullptr);
    LittleNavMapData lnmDataSource(nullptr);
    lnmDataSource.init();

    RouteTextFormat routeTextFormat(&lnmDataSource);

    routeTextFormat.loadString(
        //"EGKK/08R SFD4Z SFD M605 XIDIL UM605 BIBAX BIBA9W LFPG/09L",
        //"SFD M605 XIDIL UM605 BIBAX BIBA9W",
        "SFD M605 XIDIL UM605 BIBAX",
        "EGKK",
        "LFPG"
        );
}

