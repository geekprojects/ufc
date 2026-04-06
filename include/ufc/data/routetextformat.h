//
// Created by Ian Parker on 30/03/2026.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_ROUTETEXT_H
#define UNIVERSALFLIGHTCONNECTOR_ROUTETEXT_H

#include "airports.h"
#include "fpformat.h"

namespace UFC
{

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

class RouteTextFormat : public FlightPlanFormat
{
    RouteToken tokenise(
        const std::string &tokenStr,
        Coordinate &lastCoord,
        std::shared_ptr<Airport>& originAirport,
        std::shared_ptr<Airport>& destAirport);

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

    static bool parseLatLon(const std::string &ident, UFC::Coordinate &position);

public:
    RouteTextFormat(UFC::NavDataSource* navSource) : FlightPlanFormat(navSource, "RouteTextFormat") {}
    ~RouteTextFormat() override = default;

    std::shared_ptr<FlightPlan> loadString(std::string file) override;
    std::shared_ptr<FlightPlan> loadString(const std::string& routeStr, const std::string& origin, const std::string& dest);

    bool saveFile(std::shared_ptr<FlightPlan> flightPlan, std::string filename) override;
};
}

#endif //UNIVERSALFLIGHTCONNECTOR_ROUTETEXT_H
