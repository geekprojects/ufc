//
// Created by Ian Parker on 27/03/2026.
//

#include "ufc/data/procedures.h"

using namespace std;
using namespace UFC;

void Procedures::checkProcedures(const std::string &airportCode)
{
    auto it = m_airportProcedures.find(airportCode);
    if (it == m_airportProcedures.end())
    {
        bool found = fetchProcedures(airportCode);
        if (!found)
        {
            m_airportProcedures.try_emplace(airportCode, vector<shared_ptr<Procedure>>());
        }
    }
}

vector<shared_ptr<Procedure>> Procedures::getProcedures(const std::string &airportCode)
{
    checkProcedures(airportCode);

    auto it = m_airportProcedures.find(airportCode);
    if (it != m_airportProcedures.end())
    {
        return it->second;
    }

    return {};
}

shared_ptr<Procedure> Procedures::getDeparture(const std::string &airportCode, const std::string &name)
{
    checkProcedures(airportCode);

    auto key = Procedure::generateKey(ProcedureType::DEPARTURE, name, airportCode);

        log(UFC::DEBUG, "getDeparture: %s", key.c_str());
    auto it = m_procedures.find(key);
    if (it != m_procedures.end())
    {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Procedure>> Procedures::getProceduresForRunway(
    ProcedureType type,
    const std::string &airportCode,
    const std::string &runway)
{
    log(DEBUG, "type=%d, airport=%s, runway=%s", type, airportCode.c_str(), runway.c_str());
    checkProcedures(airportCode);

    vector<shared_ptr<Procedure>> result;

    auto it = m_airportProcedures.find(airportCode);
    if (it != m_airportProcedures.end())
    {
        for (auto const& procedure : it->second)
        {
    log(DEBUG, " -> type=%d, runway=%s", procedure->type, procedure->runway.c_str());
            if (procedure->type == type && procedure->runway == runway)
            {
                result.push_back(procedure);
            }
        }
    }

    return result;
}

std::shared_ptr<Procedure> Procedures::getArrival(const std::string &airportCode, const std::string &name)
{
    checkProcedures(airportCode);

    auto key = Procedure::generateKey(ProcedureType::ARRIVAL, name, airportCode);
    auto it = m_procedures.find(key);
    if (it != m_procedures.end())
    {
        return it->second;
    }
    return nullptr;
}
