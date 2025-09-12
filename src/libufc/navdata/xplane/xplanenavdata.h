//
// Created by Ian Parker on 19/08/2025.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_XPLANENAVDATA_H
#define UNIVERSALFLIGHTCONNECTOR_XPLANENAVDATA_H

#include "ufc/navdata.h"
#include "ufc/airports.h"

namespace UFC
{
class XPlaneAirports;
class XPlaneNavAids;

class XPlaneNavDataSource : public NavDataSource
{
    std::shared_ptr<XPlaneAirports> m_airports;
    std::shared_ptr<XPlaneNavAids> m_navAids;

public:
    explicit XPlaneNavDataSource(FlightConnector* flightConnector);

    std::shared_ptr<Airports> getAirports() override;
    std::shared_ptr<NavAids> getNavAids() override;

};

class XPlaneAirports: public Airports
{

 public:
    explicit XPlaneAirports(XPlaneNavDataSource* navDataSource) : Airports(navDataSource, "XPlane") {}

    bool init();
};

class XPlaneNavAids : public NavAids
{
    bool loadFixes();
    bool loadNavAidData();

public:
    XPlaneNavAids(XPlaneNavDataSource* navDataSource) : NavAids(navDataSource, "XPlane") {}

    bool init();
};

}

#endif //UNIVERSALFLIGHTCONNECTOR_XPLANENAVDATA_H
