//
// Created by Ian Parker on 19/08/2025.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_LNMNAVDATA_H
#define UNIVERSALFLIGHTCONNECTOR_LNMNAVDATA_H

#include "ufc/navdata.h"
#include "ufc/airports.h"

#include "../../utils/database.h"

#include <sqlite3.h>

class LNMAirports;

class LittleNavMapData : public UFC::NavDataSource
{
    std::shared_ptr<UFC::Database> m_database;

 public:
    explicit LittleNavMapData(UFC::FlightConnector* flightConnector) : UFC::NavDataSource(flightConnector, "LittleNavMapData")
    {
    }

    ~LittleNavMapData() override = default;

    bool init();

    std::shared_ptr<UFC::Airports> getAirports() override;

    std::shared_ptr<UFC::NavAids> getNavAids() override;

    [[nodiscard]] std::shared_ptr<UFC::Database> getDatabase() const { return m_database; }
};

class LNMAirports : public UFC::Airports
{
    UFC::PreparedStatement* m_findByCodeStatement = nullptr;

 public:
    explicit LNMAirports(LittleNavMapData* navDataSource)
        : Airports(navDataSource, "LittleNavMap")
    {
    }

    ~LNMAirports() override = default;

    bool init();

    [[nodiscard]] std::shared_ptr<UFC::Airport> findNearest(UFC::Coordinate point) const override;

    std::shared_ptr<UFC::Airport> findByCode(const std::string &code) override;
};


#endif //UNIVERSALFLIGHTCONNECTOR_LNMNAVDATA_H
