//
// Created by Ian Parker on 17/03/2026.
//

#include "ufc/data/lnmnavdata.h"

using namespace std;
using namespace UFC;

bool LNMAirports::init()
{
    auto db = getDatabase();
    m_findAllStatement = db->prepareStatement("SELECT lonx, laty, name, ident, altitude FROM airport");
    m_findByCodeStatement = db->prepareStatement("SELECT lonx, laty, name, ident, altitude FROM airport WHERE ident=?");

    while (m_findAllStatement->step())
    {
        auto airport = createAirportFromQuery(m_findAllStatement);
        addAirport(airport);
    }
    m_findAllStatement->reset();

    return true;
}
/*
shared_ptr<Airport> LNMAirports::findNearest(UFC::Coordinate point) const
{

    return nullptr;
}
*/
shared_ptr<Airport> LNMAirports::createAirportFromQuery(PreparedStatement* statement) const
{
    auto airport = make_shared<UFC::Airport>();
    airport->setLocation(Coordinate(statement->getDouble(1), statement->getDouble(0)));
    airport->setName(statement->getWString(2));
    airport->setICAOCode(statement->getString(3));
    airport->setElevation(statement->getDouble(4));

    return airport;
}
/*
shared_ptr<Airport> LNMAirports::findByCode(const string &code)
{
    if (code.empty())
    {
        return nullptr;
    }

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
        airport = createAirportFromQuery(m_findByCodeStatement);
        m_airportCache.try_emplace(code, airport);
    }
    m_findByCodeStatement->reset();

    return airport;
}
*/
