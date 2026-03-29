//
// Created by Ian Parker on 26/12/2025.
//

#ifndef BLACKBOX_ROUTEPARSER_H
#define BLACKBOX_ROUTEPARSER_H

#include <memory>

#include "ufc/data/navaids.h"
#include "ufc/data/navdata.h"
#include "ufc/utils/logger.h"

namespace UFC
{
class Airport;

struct RoutePoint
{
    std::string name;
    std::string airway;
    Coordinate position;

    NavAidType type;

    int minAltitude = -1;
    int maxAltitude = -1;
    int maxSpeed = -1;

    uint64_t sourceId = 0;

    std::vector<RoutePoint> components;
};

struct RouteToken
{
    std::string ident;
    std::string info;
    std::shared_ptr<Airport> airport = nullptr;
    std::shared_ptr<NavAid> navAid = nullptr;
    bool isAirway = false;
    bool isDeparture = false;
    bool isArrival = false;
};

class RouteParser : public Logger
{
    UFC::NavDataSource* m_navDataSource;

    RouteToken tokenise(const std::string &tokenStr, Coordinate &lastCoord, std::shared_ptr<Airport> &originAirport, std::shared_ptr<Airport> &
                          destAirport);

    bool parseRoute(
        const std::string &routeStr,
        std::shared_ptr<Airport> originAirport,
        std::shared_ptr<Airport> destAirport,
        Coordinate &lastCoord,
        std::vector<RoutePoint> &resolvedPoints);

    void expandAirways(std::vector<RoutePoint> &resolvedPoints, std::vector<std::shared_ptr<RoutePoint>> parsedPoints);

    std::vector<RoutePoint> generateGreatCirclePaths(std::vector<RoutePoint> &points);

    static void generateGreatCirclePath(
        std::vector<RoutePoint> &greatCircleRoute,
        RoutePoint const &rp,
        RoutePoint const &prev,
        double distance);

    static void addAirport(const std::shared_ptr<Airport> &originAirport, std::vector<RoutePoint> &resolvedPoints);

    std::shared_ptr<RoutePoint> parseWaypoint(
        Coordinate &lastCoord,
        const std::string &ident,
        std::shared_ptr<NavAid> navAid) const;

    void parseAirway(
        const std::vector<std::string> &route,
        std::vector<std::string>::iterator &it,
        const std::shared_ptr<RoutePoint> &rp);

 public:
    explicit RouteParser(NavDataSource* navDataSource);

    bool createRoute(const std::string& routeStr, const std::string& origin, const std::string& dest, std::vector<RoutePoint> &points);



    static bool parseLatLon(const std::string &ident, UFC::Coordinate &position);
};

}

#endif //BLACKBOX_ROUTEPARSER_H
