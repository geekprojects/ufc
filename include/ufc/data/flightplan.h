//
// Created by Ian Parker on 30/03/2026.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_FLIGHTPLAN_H
#define UNIVERSALFLIGHTCONNECTOR_FLIGHTPLAN_H

#include <string>

#include "airports.h"
#include "ufc/data/navaids.h"
#include "ufc/utils/geoutils.h"

namespace UFC
{

enum class FlightPlanType
{
    VFR,
    IFR
};

struct RoutePoint
{
    std::string name;
    std::string airway;
    Coordinate position;

    NavAidType type;

    int altitude;

    int minAltitude = -1;
    int maxAltitude = -1;
    int maxSpeed = -1;

    uint64_t sourceId = 0;

    std::vector<RoutePoint> components;
};

class FlightPlan
{
    int m_cycle;
    FlightPlanType m_type = FlightPlanType::VFR;

    std::shared_ptr<Airport> m_origin;
    std::shared_ptr<Airport> m_destination;

    int m_cruisingAltitude = 0;
    std::vector<RoutePoint> m_route;

 public:
    void setCycle(int cycle) { m_cycle = cycle; }
    int getCycle() const { return m_cycle; }
    void setType(FlightPlanType type) { m_type = type; }
    FlightPlanType getType() const { return m_type; }

    void setOrigin(const std::shared_ptr<Airport>& origin) { m_origin = origin; }
    const std::shared_ptr<Airport>& getOrigin() const { return m_origin; }

    void setDestination(const std::shared_ptr<Airport>& destination) { m_destination = destination; }
    const std::shared_ptr<Airport>& getDestination() const { return m_destination; }

    void setCruisingAltitude(int altitude) { m_cruisingAltitude = altitude; }
    int getCruisingAltitude() const { return m_cruisingAltitude; }

    void setRoute(const std::vector<RoutePoint>& points) { m_route = points; }
    const std::vector<RoutePoint>& getRoute() const { return m_route; }
};

}

#endif //UNIVERSALFLIGHTCONNECTOR_ROUTE_H
