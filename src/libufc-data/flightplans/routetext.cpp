//
// Created by Ian Parker on 26/12/2025.
//

#include "ufc/data/greatcircle.h"
#include "ufc/data/airports.h"
#include "ufc/data/airways.h"
#include "ufc/data/procedures.h"
#include "ufc/data/routetextformat.h"
#include "ufc/utils/utils.h"

using namespace std;
using namespace UFC;

enum class RouteState
{
    START,
    EN_ROUTE,
    END
};

vector<string> splitRoute(string line)
{
    vector<string> parts;

    StringUtils::trim(line);

    while (!line.empty())
    {
        size_t pos = line.find(' ');
        if (pos == string::npos)
        {
            pos = line.find('\t');
        }
        if (pos == string::npos)
        {
            pos = line.length();
            if (pos == 0)
            {
                break;
            }
        }
        if (pos >= 1)
        {
            string part = line.substr(0, pos);
            StringUtils::trim(part);
            parts.push_back(part);
        }
        if (pos == line.length())
        {
            break;
        }
        line = line.substr(pos + 1);
    }

    return parts;
}



bool RouteTextFormat::saveFile(std::shared_ptr<FlightPlan> flightPlan, string filename)
{
    return false;
}

RouteToken RouteTextFormat::tokenise(
    const std::string &tokenStr,
    Coordinate& lastCoord,
    shared_ptr<Airport>& originAirport,
    shared_ptr<Airport>& destAirport)
{
    RouteToken rt;
    auto idx = tokenStr.find('/');
    if (idx != string::npos)
    {
        rt.ident = tokenStr.substr(0, idx);
        rt.info = tokenStr.substr(idx + 1);
    }
    else
    {
        rt.ident = tokenStr;
    }

    // Figure out what we've got
    if (rt.ident.size() == 4)
    {
        rt.airport = getNavDataSource()->getAirports()->findByCode(rt.ident);
    }
    if (rt.airport == nullptr)
    {
        rt.navAid = getNavDataSource()->getNavAids()->findById(rt.ident, lastCoord);
        if (rt.navAid == nullptr)
        {
            if (rt.ident == "DCT")
            {
                rt.isAirway = true;
            }
            else if (getNavDataSource()->getAirways()->isAirway(rt.ident))
            {
                rt.isAirway = true;
            }
            if (!rt.isAirway)
            {
                if (originAirport != nullptr)
                {
                    rt.isDeparture = getNavDataSource()->getProcedures()->getDeparture(originAirport->getICAOCode(), rt.ident) != nullptr;
                }
                if (!rt.isDeparture && destAirport != nullptr)
                {
                    rt.isArrival = getNavDataSource()->getProcedures()->getArrival(destAirport->getICAOCode(), rt.ident) != nullptr;
                }
            }
        }
    }

    log(DEBUG, "tokenise: ident=%s, info=%s", rt.ident.c_str(), rt.info.c_str());
    log(DEBUG, "tokenise:   -> isAirport=%d, isNavAid=%d, isAirway=%d, isDeparture=%d, isArrival=%d",
        rt.airport != nullptr,
        rt.navAid != nullptr,
        rt.isAirway,
        rt.isDeparture,
        rt.isArrival);
    return rt;
}

bool RouteTextFormat::parseRoute(
    const std::string& routeStr,
    shared_ptr<Airport> originAirport,
    shared_ptr<Airport> destAirport,
    Coordinate& lastCoord,
    std::vector<RoutePoint>& resolvedPoints)
{
    RouteState state = RouteState::START;
    vector<std::shared_ptr<RoutePoint>> parsedPoints;
    auto route = splitRoute(routeStr);
    shared_ptr<RoutePoint> previousPoint = nullptr;

    for (auto it = route.begin(); it != route.end(); ++it)
    {
        RouteToken token = tokenise(*it, lastCoord, originAirport, destAirport);

        if (token.isAirway)
        {
            log(DEBUG, "parseRoute: Unexpected Airway: %s, skipping", token.ident.c_str());
            state = RouteState::EN_ROUTE;
            continue;
        }

        shared_ptr<RoutePoint> rp = nullptr;
        if (state == RouteState::START)
        {
            if (token.airport != nullptr)
            {
                if (token.ident != originAirport->getICAOCode())
                {
                    log(
                        WARN,
                        "parseRoute: Starting with different airport: %s != %s",
                        token.ident.c_str(),
                        originAirport->getICAOCode().c_str());
                    originAirport = token.airport;
                }

                if (originAirport != nullptr)
                {
                    log(
                        DEBUG,
                        "parseRoute: Origin: %s (%s) Runway: %s",
                        originAirport->getICAOCode().c_str(),
                        token.ident.c_str(),
                        token.info.c_str());
                }
                else
                {
                    log(WARN, "parseRoute: Invalid airport: %s", token.ident.c_str());
                    state = RouteState::EN_ROUTE;
                    it--;
                }
            }
            else
            {
                if (originAirport == nullptr)
                {
                    log(WARN, "parseRoute: No origin");
                    state = RouteState::EN_ROUTE;
                }
                else
                {
                    if (token.isDeparture)
                    {
                        log(DEBUG, "parseRoute: Departure: %s", token.ident.c_str());
                    }
                    else
                    {
                        // Not a departure, probably part of the route
                        state = RouteState::EN_ROUTE;
                    }
                }
            }
        }
        if (state == RouteState::EN_ROUTE)
        {
            rp = parseWaypoint(lastCoord, token.ident, token.navAid);

            if (rp != nullptr)
            {
                log(
                    DEBUG,
                    "parseRoute: Waypoint: %s: %ls",
                    rp->name.c_str(),
                    lastCoord.toString().c_str());
                parsedPoints.push_back(rp);
                parseAirway(route, it, rp);
            }
            else
            {
                if (token.isArrival)
                {
                    printf("parseRoute: Arrival: %s\n", token.ident.c_str());
                    state = RouteState::END;
                }
                else if (token.airport != nullptr)
                {
                    if (token.ident != destAirport->getICAOCode())
                    {
                        auto dest = getNavDataSource()->getAirports()->findByCode(token.ident);
                        if (dest != nullptr)
                        {
                            log(
                                DEBUG,
                                "parseRoute: Destination airport doesn't match!: %s != %s",
                                dest->getICAOCode().c_str(),
                                destAirport->getICAOCode().c_str());
                            destAirport = dest;
                            state = RouteState::END;
                        }
                        else
                        {
                            log(WARN, "parseRoute: Unknown waypoint: %s", token.ident.c_str());
                        }
                    }
                    else
                    {
                        state = RouteState::END;
                    }
                }
            }
        }
    }

    log(
        DEBUG,
        "parseRoute: Origin: %s: %s",
        originAirport->getICAOCode().c_str(),
        originAirport->getName().c_str());
    addAirport(originAirport, resolvedPoints);

    expandAirways(resolvedPoints, parsedPoints);
    addAirport(destAirport, resolvedPoints);

    log(
        DEBUG,
        "parseRoute: Destination: %s: %s",
        destAirport->getICAOCode().c_str(),
        destAirport->getName().c_str());

    return true;
}

void RouteTextFormat::expandAirways(
    std::vector<RoutePoint> &resolvedPoints,
    vector<std::shared_ptr<RoutePoint>> parsedPoints)
{
    for (auto it = parsedPoints.begin(); it != parsedPoints.end(); ++it)
    {
        const auto& point = *it;
        resolvedPoints.push_back(*point);
        if ((it + 1) != parsedPoints.end())
        {
            auto nextPoint = *(it + 1);
            log( DEBUG, "parseRoute: %s -> %s", point->name.c_str(), nextPoint->name.c_str());
            if (point->type == NavAidType::WAY_POINT &&
                point->sourceId != 0 &&
                nextPoint->type == NavAidType::WAY_POINT &&
                nextPoint->sourceId != 0)
            {
                if (!point->airway.empty())
                {
                    log(DEBUG, "parseRoute:  -> Airway: %s", point->airway.c_str());
                    vector<shared_ptr<NavAid>> airwayPoints;
                    getNavDataSource()->getAirways()->expandAirway(
                        point->airway,
                        point->sourceId,
                        nextPoint->sourceId,
                        airwayPoints);
                    for (auto const& ap : airwayPoints)
                    {
                        RoutePoint rp;
                        rp.name = ap->getId();
                        rp.position = ap->getLocation();
                        rp.type = ap->getType();
                        resolvedPoints.push_back(rp);
                    }
                }
                else
                {
                    log(DEBUG, "parseRoute:  -> No airway connecting to next point");
                }
            }
        }
    }
}

vector<RoutePoint> RouteTextFormat::generateGreatCirclePaths(std::vector<RoutePoint> &points)
{
    vector<RoutePoint> greatCircleRoute;
    for (auto it = points.begin(); it != points.end(); ++it)
    {
        RoutePoint const& rp = *it;
        bool added = false;
        if (it != points.begin())
        {
            auto const& prev = *(it - 1);
            auto distance = GeoUtils::distance(prev.position, rp.position);
            log(
                DEBUG,
                "parseRoute: Distance from %s to %s: %0.2f",
                prev.name.c_str(),
                rp.name.c_str(),
                distance);
            if (distance > 300.0f)
            {
                generateGreatCirclePath(greatCircleRoute, rp, prev, distance);
                added = true;
            }
        }
        if (!added)
        {
            greatCircleRoute.push_back(rp);
        }
    }
    return greatCircleRoute;
}

void RouteTextFormat::generateGreatCirclePath(
    vector<RoutePoint>& greatCircleRoute,
    RoutePoint const &rp,
    RoutePoint const &prev,
    double distance)
{
    GreatCircle gc(prev.position, rp.position);
    auto arcPoints = gc.arc(static_cast<int>(distance / 100.0f));
    for (auto arcIt = arcPoints.begin(); arcIt != arcPoints.end(); ++arcIt)
    {
        auto const& p = *arcIt;
        RoutePoint gcPoint;
        if ((arcIt + 1) == arcPoints.end())
        {
            gcPoint.name = rp.name;
        }
        gcPoint.position = p;
        greatCircleRoute.push_back(gcPoint);
    }
}

void RouteTextFormat::addAirport(const shared_ptr<Airport> &originAirport, std::vector<RoutePoint> &resolvedPoints)
{
    //if (originAirport.hasCoordinates)
    {
        RoutePoint rp;
        rp.name = originAirport->getICAOCode();
        rp.position = originAirport->getLocation();
        rp.type = NavAidType::AIRPORT;
        resolvedPoints.push_back(rp);
    }
}

std::shared_ptr<RoutePoint> RouteTextFormat::parseWaypoint(UFC::Coordinate& lastCoord, const string& ident, shared_ptr<NavAid> navAid) const
{
    shared_ptr<RoutePoint> rp = nullptr;

    if (navAid != nullptr)
    {
        rp = make_shared<RoutePoint>();
        lastCoord = navAid->getLocation();
        if (navAid->getId() != navAid->getName())
        {
            rp->name = navAid->getId() + " (" + navAid->getName() + ")";
        }
        else
        {
            rp->name = navAid->getId();
        }
        rp->position = navAid->getLocation();
        rp->type = navAid->getType();
        rp->sourceId = navAid->getSourceId();
    }
    else
    {
        Coordinate pos;
        if (parseLatLon(ident, pos))
        {
            lastCoord = pos;
            rp = make_shared<RoutePoint>();
            rp->name = ident;
            rp->position = pos;
            rp->type = NavAidType::WAY_POINT;
        }
    }
    return rp;
}

void RouteTextFormat::parseAirway(const std::vector<std::string>& route, std::vector<std::string>::iterator& it, const shared_ptr<RoutePoint> &rp)
{
    log(DEBUG, "parseAirway: Parsing airway");
    if ((it + 1) != route.end())
    {
        string next = *(it + 1);
        log(DEBUG, "parseAirway: next=%s", next.c_str());

        bool isAirway = false;
        if (next == "DCT")
        {
            isAirway = true;
            rp->airway = "DCT";
            log(DEBUG, "parseRoute: Airway: DCT");
        }
        if (!isAirway)
        {
            isAirway = getNavDataSource()->getAirways()->isAirway(next);
        }

        if (isAirway)
        {
            log(DEBUG, "parseRoute: Airway: %s", next.c_str());
            rp->airway = next;
            it++;
        }
    }
}

std::shared_ptr<FlightPlan> RouteTextFormat::loadString(std::string routeStr)
{
    return loadString(routeStr, "", "");
}

shared_ptr<FlightPlan> RouteTextFormat::loadString(const string& routeStr, const string& origin, const string& dest)
{
    shared_ptr<Airport> originAirport = nullptr;
    shared_ptr<Airport> destAirport = nullptr;
    Coordinate lastCoord = {};

    if (!origin.empty())
    {
        originAirport = getNavDataSource()->getAirports()->findByCode(origin);
        if (originAirport != nullptr)
        {
            lastCoord = originAirport->getLocation();
        }
        else
        {
            log(WARN, "parseRoute: Invalid origin: %s", origin.c_str());
        }
    }
    else
    {
        log(INFO, "parseRoute: No origin set");
    }

    if (!dest.empty())
    {
        destAirport = getNavDataSource()->getAirports()->findByCode(dest);
        if (destAirport == nullptr)
        {
            log(WARN, "parseRoute: Invalid destination: %s", dest.c_str());
        }
    }

    auto flightPlan = make_shared<FlightPlan>();
    vector<RoutePoint> points;
    parseRoute(routeStr, originAirport, destAirport, lastCoord, points);

    vector<RoutePoint> greatCircleRoute = generateGreatCirclePaths(points);
    flightPlan->setRoute(greatCircleRoute);

    return flightPlan;
}

bool RouteTextFormat::parseLatLon(const std::string& ident, UFC::Coordinate& position)
{
    if (ident.length() == 7)
    {
        // 51N025E
        int latDeg;
        int lonDeg;
        char latDir;
        char lonDir;

        sscanf(ident.c_str(), "%d%c%d%c", &latDeg, &latDir, &lonDeg, &lonDir);

        if ((latDir != 'N' && latDir != 'S') || (lonDir != 'E' && lonDir != 'W'))
        {
            return false;
        }

        if (latDir == 'S')
        {
            latDeg = -latDeg;
        }
        if (lonDir == 'W')
        {
            lonDeg = -lonDeg;
        }

        //printf("parseLonLat(7): %d %c %d %c\n", latDeg, latDir, lonDeg, lonDir);
        position.latitude = static_cast<float>(latDeg);
        position.longitude = static_cast<float>(lonDeg);
        return true;
    }

    if (ident.length() == 11)
    {
        //5220N03305E
        int latDeg;
        int lonDeg;
        char latDir;
        char lonDir;
        sscanf(ident.c_str(), "%d%c%d%c", &latDeg, &latDir, &lonDeg, &lonDir);
        if ((latDir != 'N' && latDir != 'S') || (lonDir != 'E' && lonDir != 'W'))
        {
            return false;
        }

        const int latMins = latDeg % 100;
        const int lonMins = lonDeg % 100;

        latDeg -= latMins;
        lonDeg -= lonMins;
        latDeg /= 100;
        lonDeg /= 100;

        if (latDir == 'S')
        {
            latDeg = -latDeg;
        }
        if (lonDir == 'W')
        {
            lonDeg = -lonDeg;
        }

        const float lat = (float)latDeg + ((float)latMins / 60.0f);
        const float lon = (float)lonDeg + ((float)lonMins / 60.0f);

        //printf("parseLonLat(11): %d %c %d %c -> %f, %f\n", latDeg, latDir, lonDeg, lonDir, lat, lon);
        position.latitude = lat;
        position.longitude = lon;
        return true;
    }

    return false;
}
