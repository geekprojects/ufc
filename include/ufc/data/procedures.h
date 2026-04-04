//
// Created by Ian Parker on 27/03/2026.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_PROCEDURES_H
#define UNIVERSALFLIGHTCONNECTOR_PROCEDURES_H

#include <ufc/data/navdata.h>
#include <ufc/utils/geoutils.h>

namespace UFC
{

enum class ProcedureType
{
    DEPARTURE,
    ARRIVAL,
    APPROACH
};

struct ProcedureLeg
{
    std::string ident;
    bool isTransition;
    Coordinate position;
    int minAltitude;
    int maxAltitude;

    int minSpeed;
    int maxSpeed;
};

struct Procedure
{
    ProcedureType type;
    std::string ident;
    std::string procedureIdent;
    std::string transitionIdent;
    std::string airportCode;
    std::string runway;
    int direction;

    std::vector<ProcedureLeg> legs;

    static std::string generateKey(ProcedureType type, const std::string &ident, const std::string &airportCode)
    {
        return std::to_string(static_cast<int>(type)) + ":" + ident + ":" + airportCode;
    }
};

class Procedures : public NavData
{
 protected:
    std::map<std::string, std::shared_ptr<Procedure>> m_procedures;
    std::map<std::string, std::vector<std::shared_ptr<Procedure>>> m_airportProcedures;

    void addProcedure(std::shared_ptr<Procedure> procedure)
    {
        std::string key = Procedure::generateKey(procedure->type, procedure->ident, procedure->airportCode);
        log(UFC::DEBUG, "Adding procedure: %s", key.c_str());
        if (m_procedures.find(key) == m_procedures.end())
        {
            m_procedures.try_emplace(key, procedure);
            m_airportProcedures[procedure->airportCode].push_back(procedure);
        }
    }

    virtual bool fetchProcedures(const std::string &airportCode) { return {}; }
    void checkProcedures(const std::string &airportCode);

 public:
    Procedures(NavDataSource* navDataSource, std::string const& name) : NavData(navDataSource, name)
    {
    }
    ~Procedures() override = default;

    virtual bool init() { return true; }

    virtual std::vector<std::shared_ptr<Procedure>> getProcedures(const std::string &airportCode);

    virtual std::shared_ptr<Procedure> getDeparture(const std::string &airportCode, const std::string &name);
    virtual std::vector<std::shared_ptr<Procedure>> getProceduresForRunway(
        ProcedureType type,
        const std::string &airportCode,
        const std::string &runway);
    virtual std::shared_ptr<Procedure> getArrival(const std::string &airportCode, const std::string &name);
};

}

#endif //UNIVERSALFLIGHTCONNECTOR_PROCEDURES_H
