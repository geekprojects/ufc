//
// Created by Ian Parker on 19/08/2025.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_XPLANENAVDATA_H
#define UNIVERSALFLIGHTCONNECTOR_XPLANENAVDATA_H

#include "ufc/navdata.h"
#include "ufc/airports.h"

namespace UFC
{
class XPlaneNavDataSource : public NavDataSource
{

    NavDataHeader loadHeader(std::string headerStr);
    void loadFixes(const std::shared_ptr<NavAids> &navData);
    void loadNavAidData(const std::shared_ptr<NavAids>& shared);

public:
    explicit XPlaneNavDataSource(FlightConnector* flightConnector);

    std::shared_ptr<Airports> loadAirports() override;
    std::shared_ptr<NavAids> loadNavAids() override;

};
}

#endif //UNIVERSALFLIGHTCONNECTOR_XPLANENAVDATA_H
