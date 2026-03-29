//
// Created by Ian Parker on 18/03/2026.
//

#include "ufc/data/lnmnavdata.h"

using namespace std;
using namespace UFC;


bool LNMProcedures::init()
{
    m_findStatement = getDatabase()->prepareStatement(
    "SELECT appr.approach_id, appr.fix_ident, appr.airport_ident, appr.suffix, appr.type, rw.name FROM approach appr left join runway_end rw on rw. runway_end_id = appr.runway_end_id WHERE airport_ident=?");

    m_findApproachLegs = getDatabase()->prepareStatement(
        "SELECT type, fix_type, fix_ident, fix_lonx, fix_laty, altitude1, altitude2, speed_limit, speed_limit_type FROM approach_leg WHERE approach_id=?" );

    return true;
}

bool LNMProcedures::fetchProcedures(const std::string &airportCode)
{
    vector<shared_ptr<Procedure>> procedures;
    m_findStatement->bindString(1, airportCode);
    bool found = false;
    while (m_findStatement->step())
    {
        auto procedure = make_shared<Procedure>();
        procedure->airportCode = airportCode;

        int64_t approachId = m_findStatement->getInt64(0);
        procedure->ident = m_findStatement->getString(1);
        procedure->airportCode = m_findStatement->getString(2);
        string suffix = m_findStatement->getString(3);
        string typeName = m_findStatement->getString(4);
        procedure->runway = m_findStatement->getString(5);

        string procType;
        if (suffix == "D")
        {
            procedure->type = ProcedureType::DEPARTURE;
            procType = "Departure";
        }
        else if (suffix == "A")
        {
            procedure->type = ProcedureType::ARRIVAL;
            procType = "Arrival";
        }
        else
        {
            procedure->type = ProcedureType::APPROACH;
            procType = "Approach";
        }
        log(DEBUG, "getProcedures: %s %s type=%s, runway=%s", airportCode.c_str(), procedure->ident.c_str(), procType.c_str(), procedure->runway.c_str());

        m_findApproachLegs->bindInt64(1, approachId);
        while (m_findApproachLegs->step())
        {
            ProcedureLeg leg;
            string legType = m_findApproachLegs->getString(0);
            string fixType = m_findApproachLegs->getString(1);
            leg.ident = m_findApproachLegs->getString(2);
            leg.position.longitude = m_findApproachLegs->getDouble(3);
            leg.position.latitude = m_findApproachLegs->getDouble(4);
            leg.maxAltitude = static_cast<int>(m_findApproachLegs->getDouble(5));
            leg.minAltitude = static_cast<int>(m_findApproachLegs->getDouble(6));
            int speedLimit = m_findApproachLegs->getInt(7);
            string speedLimitType = m_findApproachLegs->getString(8);

            if (speedLimitType == "-")
            {
                leg.minSpeed = 0;
                leg.maxSpeed = speedLimit;
            }
            else if (speedLimitType == "+")
            {
                leg.minSpeed = speedLimit;
                leg.maxSpeed = -1;
            }
            else
            {
                leg.minSpeed = speedLimit;
                leg.maxSpeed = speedLimit;
            }
            /*
            log(
                DEBUG,
                "get:  -> legType=%s, fixType=%s, fixIdent=%s, fixLon=%0.2f, fixLat=%0.2f, altitude=%0.2f-%0.2f, speedLimit=%d-%d",
                legType.c_str(), fixType.c_str(), leg.ident.c_str(), leg.position.longitude, leg.position.latitude, leg.minAltitude, leg.maxAltitude, leg.minSpeed, leg.maxSpeed);
                */
            procedure->legs.push_back(leg);
        }
        m_findApproachLegs->reset();
        addProcedure(procedure);
        found = true;
    }
    m_findStatement->reset();

    return found;
}

/*
shared_ptr<Procedure> LNMProcedures::getDeparture(const string &airportCode, const string &name)
{
    return get(ProcedureType::DEPARTURE, airportCode, name);
}

shared_ptr<Procedure> LNMProcedures::getArrival(const string &airportCode, const string &name)
{
    return get(ProcedureType::ARRIVAL, airportCode, name);
}

shared_ptr<Procedure> LNMProcedures::get(
    ProcedureType type,
    const string& airportCode,
    const string& name)
{
    m_findStatement->bindString(1, type);
    m_findStatement->bindString(2, airportCode);
    m_findStatement->bindString(3, name);

    shared_ptr<Procedure> procedure = nullptr;
    if (m_findStatement->step())
    {
        int64_t approachId = m_findStatement->getInt64(0);
        string procedureType = m_findStatement->getString(1);
        log(DEBUG, "get: %s, %s ->  %ld, %s", airportCode.c_str(), name.c_str(), approachId, procedureType.c_str());

        procedure = make_shared<Procedure>();
        procedure->airportCode = airportCode;
        procedure->ident = name;


    }
    else
    {
        log(WARN, "get: Failed to find procedure: airport=%s, ident=%s", airportCode.c_str(), name.c_str());
    }

    m_findStatement->reset();

    return procedure;
}
*/
