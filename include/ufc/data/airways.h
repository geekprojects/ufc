//
// Created by Ian Parker on 27/03/2026.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_AIRWAYS_H
#define UNIVERSALFLIGHTCONNECTOR_AIRWAYS_H

#include <ufc/data/navdata.h>

namespace UFC
{
class NavAid;

struct Airway
{
    std::string ident;
    std::vector<std::shared_ptr<NavAid>> m_navAids;
};

class Airways : public NavData
{
    std::map<std::string, std::vector<std::shared_ptr<Airway>>> m_airwaysById;

protected:
    void addAirway(const std::shared_ptr<Airway>& airway)
    {
        m_airwaysById.try_emplace(airway->ident, std::vector{airway});
    }

public:
    explicit Airways(NavDataSource* navDataSource, std::string const& name) : NavData(navDataSource, name)
    {
    }

    ~Airways() override = default;

    virtual bool init() { return true; }

    virtual bool expandAirway(
        const std::string &ident,
        uint64_t entryWaypointId,
        uint64_t exitWaypointId,
        std::vector<std::shared_ptr<NavAid>> &navAids) { return false; }

    virtual bool isAirway(const std::string & next) { return m_airwaysById.find(next) != m_airwaysById.end(); }
    virtual std::shared_ptr<NavAid> findNextAirwayWaypoint(const std::string &ident, uint64_t entryWaypointId, bool forward) { return nullptr; }
};

}

#endif //UNIVERSALFLIGHTCONNECTOR_AIRWAYS_H
