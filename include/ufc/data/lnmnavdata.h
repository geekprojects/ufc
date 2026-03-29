//
// Created by Ian Parker on 19/08/2025.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_LNMNAVDATA_H
#define UNIVERSALFLIGHTCONNECTOR_LNMNAVDATA_H

//#include <ufc/data/navdata.h>

#include <ufc/data/airways.h>
#include <ufc/data/navaids.h>
#include <ufc/data/procedures.h>
#include <ufc/data/airports.h>

#include <ufc/utils/database.h>

namespace UFC
{
class LNMAirports;
class LNMNavAids;
class LNMProcedures;
class LNMAirways;

class LittleNavMapData : public NavDataSource
{
    std::shared_ptr<Database> m_database;
    std::shared_ptr<LNMAirports> m_airports;
    std::shared_ptr<LNMNavAids> m_navAids;
    std::shared_ptr<LNMProcedures> m_procedures;
    std::shared_ptr<LNMAirways> m_airways;

public:
    explicit LittleNavMapData(UFC::FlightConnector* flightConnector);

    ~LittleNavMapData() override = default;

    bool init();

    std::shared_ptr<Airports> getAirports() override;

    std::shared_ptr<NavAids> getNavAids() override;

    std::shared_ptr<Procedures> getProcedures() override;

    std::shared_ptr<Airways> getAirways() override;

    [[nodiscard]] std::shared_ptr<Database> getDatabase() const
    {
        return m_database;
    }
};

class LNMAirports : public UFC::Airports
{
    UFC::PreparedStatement* m_findByCodeStatement = nullptr;

    std::map<std::string, std::shared_ptr<UFC::Airport> > m_airportCache;

    [[nodiscard]] std::shared_ptr<UFC::Database> getDatabase() const
    {
        return dynamic_cast<LittleNavMapData*>(m_navDataSource)->getDatabase();
    }

    [[nodiscard]] std::shared_ptr<UFC::Airport> createAirportFromQuery(const std::string &code) const;

public:
    explicit LNMAirports(LittleNavMapData* navDataSource) : Airports(navDataSource, "LittleNavMap")
    {
    }

    ~LNMAirports() override = default;

    bool init() override;

    [[nodiscard]] std::shared_ptr<UFC::Airport> findNearest(UFC::Coordinate point) const override;

    std::shared_ptr<UFC::Airport> findByCode(const std::string &code) override;
};

class LNMNavAids : public UFC::NavAids
{
    UFC::PreparedStatement* m_findNavStatement = nullptr;

    [[nodiscard]] std::shared_ptr<UFC::Database> getDatabase() const
    {
        return dynamic_cast<LittleNavMapData*>(m_navDataSource)->getDatabase();
    }

public:
    explicit LNMNavAids(LittleNavMapData* navDataSource) : NavAids(navDataSource, "LittleNavMap")
    {
    }

    bool init() override;

    [[nodiscard]] std::vector<std::shared_ptr<UFC::NavAid> > findById(const std::string &id) const override;
};

class LNMProcedures : public UFC::Procedures
{
    UFC::PreparedStatement* m_findStatement = nullptr;
    UFC::PreparedStatement* m_findApproachLegs = nullptr;

    [[nodiscard]] std::shared_ptr<UFC::Database> getDatabase() const
    {
        return dynamic_cast<LittleNavMapData*>(m_navDataSource)->getDatabase();
    }

    bool fetchProcedures(const std::string &airportCode) override;

public:
    explicit LNMProcedures(LittleNavMapData* navDataSource) : Procedures(navDataSource, "LittleNavMap")
    {
    }

    bool init() override;
};

class LNMAirways : public UFC::Airways
{
    UFC::PreparedStatement* m_findNextAirwayStatement = nullptr;
    UFC::PreparedStatement* m_findPreviousAirwayStatement = nullptr;

    [[nodiscard]] std::shared_ptr<UFC::Database> getDatabase() const
    {
        return dynamic_cast<LittleNavMapData*>(m_navDataSource)->getDatabase();
    }

public:
    explicit LNMAirways(LittleNavMapData* navDataSource) : Airways(navDataSource, "LittleNavMap")
    {
    }

    ~LNMAirways() override = default;

    bool init() override;

    bool expandAirway(
        const std::string &ident,
        uint64_t entryWaypointId,
        uint64_t exitWaypointId,
        std::vector<std::shared_ptr<UFC::NavAid>> &navAids) override;

    bool isAirway(const std::string &next) override;

    std::shared_ptr<UFC::NavAid> findNextAirwayWaypoint(const std::string &ident, uint64_t entryWaypointId, bool forward) override;
};
}

#endif //UNIVERSALFLIGHTCONNECTOR_LNMNAVDATA_H
