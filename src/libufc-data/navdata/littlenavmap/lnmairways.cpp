//
// Created by Ian Parker on 18/03/2026.
//

#include <cinttypes>

#include "ufc/data/lnmnavdata.h"

using namespace std;
using namespace UFC;

bool LNMAirways::init()
{
    m_findAirwayStatement = getDatabase()->prepareStatement("SELECT airway_type FROM airway WHERE airway_name=?");
    m_findNextAirwayStatement = getDatabase()->prepareStatement(
        "SELECT"
        "    aw.sequence_no, aw.from_waypoint_id, aw.to_waypoint_id, w.ident, w.lonx, w.laty"
        "  FROM airway aw "
        "  JOIN waypoint w ON w.waypoint_id = aw.to_waypoint_id "
        "  WHERE airway_name=? AND from_waypoint_id=?");

    m_findPreviousAirwayStatement = getDatabase()->prepareStatement(
        "SELECT"
        "    aw.sequence_no, aw.from_waypoint_id, aw.to_waypoint_id, w.ident, w.lonx, w.laty"
        "  FROM airway aw "
        "  JOIN waypoint w ON w.waypoint_id = aw.from_waypoint_id "
        "  WHERE airway_name=? AND to_waypoint_id=?" );

    return true;
}

bool LNMAirways::expandAirway(
    const string &ident,
    uint64_t entryWaypointId,
    uint64_t exitWaypointId,
    vector<shared_ptr<NavAid>> &navAids)
{
    printf("expandAirway: %s: %" PRId64 " -> %" PRId64 "\n", ident.c_str(), entryWaypointId, exitWaypointId);

    auto from = findNextAirwayWaypoint(ident, entryWaypointId, true);
    if (from == nullptr)
    {
        printf("expandAirway: Unable to find entry waypoint: %" PRId64 "\n", entryWaypointId);
        return false;
    }

    auto to = findNextAirwayWaypoint(ident, exitWaypointId, false);
    if (to == nullptr)
    {
        printf("expandAirway: Unable to find exit waypoint: %" PRId64 "\n", entryWaypointId);
        return false;
    }

    if (from->getSequenceNo() == to->getSequenceNo())
    {
        return true;
    }

    bool forwards = from->getSequenceNo() < to->getSequenceNo();
    uint64_t nextId;
    uint64_t endId;
    if (forwards)
    {
        printf("Forward!\n");
        nextId = from->getNextId();
        endId = to->getSourceId();
    }
    else
    {
        printf("Backwards!\n");
        nextId = from->getSourceId();
        endId = to->getNextId();
    }

    bool done = false;
    while (!done)
    {
        auto navAid = findNextAirwayWaypoint(ident, nextId, forwards);
        if (navAid == nullptr)
        {
            printf("expandAirway: Unable to find waypoint: %" PRId64 "\n", nextId);
            return false;
        }

        if (navAid->getSourceId() == endId)
        {
            break;
        }

        navAids.push_back(navAid);

        if (forwards)
        {
            nextId = navAid->getNextId();
        }
        else
        {
            nextId = navAid->getSourceId();
        }
    }
    return true;
}

bool LNMAirways::isAirway(const std::string& ident)
{
    m_findAirwayStatement->bindString(1, ident);
    bool result = m_findAirwayStatement->step();
    log(DEBUG, "isAirway: %s -> %d", ident.c_str(), result);
    m_findAirwayStatement->reset();
    return result;
}

shared_ptr<NavAid> LNMAirways::findNextAirwayWaypoint(const std::string &ident, uint64_t entryWaypointId, bool forward)
{
    PreparedStatement* preparedStatement;
    if (forward)
    {
        preparedStatement = m_findNextAirwayStatement;
    }
    else
    {
        preparedStatement = m_findPreviousAirwayStatement;
    }

    preparedStatement->bindString(1, ident);
    preparedStatement->bindInt64(2, entryWaypointId);

    shared_ptr<NavAid> navAid = nullptr;
    if (preparedStatement->step())
    {
        navAid = make_shared<NavAid>();
        navAid->setType(NavAidType::WAY_POINT);
        navAid->setSequenceNo(preparedStatement->getInt64(0));
        navAid->setSourceId(preparedStatement->getInt64(1));
        navAid->setNext_id(preparedStatement->getInt64(2));

        navAid->setId(preparedStatement->getString(3));
        navAid->setName(preparedStatement->getString(3));
        auto longitude = preparedStatement->getDouble(4);
        auto latitude = preparedStatement->getDouble(5);
        navAid->setLocation(Coordinate(latitude, longitude));
        log(
            DEBUG,
            "findNextAirwayWaypoint: %s: %lld: %llu -> %llu: %s (%f, %f)",
            ident.c_str(),
            navAid->getSequenceNo(),
            navAid->getSourceId(),
            navAid->getNextId(),
            navAid->getId().c_str(),
            navAid->getLocation().longitude,
            navAid->getLocation().latitude);
    }
    preparedStatement->reset();

    return navAid;
}
