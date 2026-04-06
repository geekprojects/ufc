//
// Created by Ian Parker on 19/08/2025.
//

#include "ufc/data/lnmnavdata.h"

using namespace std;
using namespace UFC;

LittleNavMapData::LittleNavMapData(UFC::FlightConnector* flightConnector) : UFC::NavDataSource(flightConnector, "LittleNavMapData")
{
}

bool LittleNavMapData::init()
{
    sqlite3_initialize();

    string homeDir = getenv("HOME");
    string m_path = homeDir + "/.config/ABarthel/little_navmap_db/little_navmap_navigraph.sqlite";
    m_database = make_shared<Database>(m_path);
    return m_database->open();
}

shared_ptr<Airports> LittleNavMapData::getAirports()
{
    if (m_airports == nullptr)
    {
        m_airports = make_shared<LNMAirports>(this);
        m_airports->init();
    }
    return m_airports;
}

shared_ptr<NavAids> LittleNavMapData::getNavAids()
{
    if (m_navAids == nullptr)
    {
        m_navAids = make_shared<LNMNavAids>(this);
        m_navAids->init();
    }
    return m_navAids;
}

shared_ptr<Procedures> LittleNavMapData::getProcedures()
{
    if (m_procedures == nullptr)
    {
        m_procedures = make_shared<LNMProcedures>(this);
        m_procedures->init();
    }
    return m_procedures;
}

shared_ptr<Airways> LittleNavMapData::getAirways()
{
    if (m_airways == nullptr)
    {
        m_airways = make_shared<LNMAirways>(this);
        m_airways->init();
    }
    return m_airways;
}
