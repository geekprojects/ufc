//
// Created by Ian Parker on 17/03/2026.
//

#include "ufc/data/lnmnavdata.h"

using namespace std;
using namespace UFC;

bool LNMAirports::init()
{
    auto db = getDatabase();
    m_findByCodeStatement = db->prepareStatement("SELECT lonx, laty, name, altitude FROM airport WHERE ident=?");
    return true;
}

shared_ptr<Airport> LNMAirports::findNearest(UFC::Coordinate point) const
{
    return nullptr;
}

shared_ptr<Airport> LNMAirports::createAirportFromQuery(const string &code) const
{
    auto airport = make_shared<UFC::Airport>();
    airport->setICAOCode(code);
    airport->setLocation(Coordinate(m_findByCodeStatement->getDouble(1), m_findByCodeStatement->getDouble(0)));
    airport->setName(m_findByCodeStatement->getWString(2));
    airport->setElevation(m_findByCodeStatement->getDouble(3));

    return airport;
}

shared_ptr<Airport> LNMAirports::findByCode(const string &code)
{
    auto it = m_airportCache.find(code);
    if (it != m_airportCache.end())
    {
        return it->second;
    }

    m_findByCodeStatement->bindString(1, code);
    m_findByCodeStatement->executeQuery();
    shared_ptr<Airport> airport = nullptr;
    if (m_findByCodeStatement->step())
    {
        airport = createAirportFromQuery(code);
        m_airportCache.try_emplace(code, airport);
    }
    m_findByCodeStatement->reset();

    return airport;
}
