//
// Created by Ian Parker on 18/03/2026.
//

#include "ufc/data/lnmnavdata.h"

using namespace std;
using namespace UFC;

bool LNMAirways::init()
{
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
    printf("expandAirway: %s: %llu -> %llu\n", ident.c_str(), entryWaypointId, exitWaypointId);

    auto from = findNextAirwayWaypoint(ident, entryWaypointId, true);
    if (from == nullptr)
    {
        printf("expandAirway: Unable to find entry waypoint: %llu\n", entryWaypointId);
        return false;
    }

    auto to = findNextAirwayWaypoint(ident, exitWaypointId, false);
    if (to == nullptr)
    {
        printf("expandAirway: Unable to find exit waypoint: %llu\n", entryWaypointId);
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
            printf("expandAirway: Unable to find waypoint: %llu\n", nextId);
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

bool LNMAirways::isAirway(const std::string &next)
{
    return true;
}

shared_ptr<NavAid> LNMAirways::findNextAirwayWaypoint(const std::string &ident, uint64_t entryWaypointId, bool forward)
{
    return nullptr;
}
