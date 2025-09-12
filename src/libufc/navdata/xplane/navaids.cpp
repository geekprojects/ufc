//
// Created by Ian Parker on 14/04/2024.
//

#include "xplanenavdata.h"

#include <ufc/utils.h>

#include <cstring>
#include <regex>

#include "navdata.h"
#include "ufc/data.h"
#include "ufc/flightconnector.h"

using namespace std;
using namespace UFC;


bool XPlaneNavAids::init()
{
    if (!loadFixes())
    {
        return false;
    }
    if (!loadNavAidData())
    {
        return false;
    }

    return true;
}


bool XPlaneNavAids::loadFixes()
{
    string fixPath = getFlightConnector()->getConfig().xplanePath + "/Custom Data/earth_fix.dat";

    auto fixData = make_shared<Data>();
    fixData->load(fixPath);
    fixData->readLine();
    string headerStr = fixData->readLine();

    auto header = NavDataUtils::parseHeader(headerStr);
    if (header.version != 1200)
    {
        log(ERROR, "Unsupported version of earth_fix: version=%d", header.version);
        return false;
    }
    setHeader(header);

    while (!fixData->eof())
    {
        string lineStr = fixData->readLine();
        lineStr = StringUtils::trim(lineStr);
        if (lineStr.size() == 0)
        {
            continue;
        }
        auto line = NavDataUtils::splitLine(lineStr);
        if (line.size() < 6)
        {
            continue;
        }

        double lat = atof(line[0].c_str());
        double lng = atof(line[1].c_str());
        string id = line[2];
        //string region = line[3];
        //int type = atoi(line[4].c_str());

        auto navAid = make_shared<NavAid>();
        navAid->setLocation(Coordinate(lat, lng));
        navAid->setId(id);
        navAid->setType(NavAidType::FIX);
        addNavAid(navAid);
    }
    return true;
}

bool XPlaneNavAids::loadNavAidData()
{
    string fixPath = getFlightConnector()->getConfig().xplanePath + "/Custom Data/earth_nav.dat";

    auto fixData = make_shared<Data>();
    fixData->load(fixPath);
    fixData->readLine();
    string headerStr = fixData->readLine();

    auto header = NavDataUtils::parseHeader(headerStr);
    if (header.version != 1200)
    {
        log(ERROR, "Unsupported version of earth_nav: version=%d", header.version);
        return false;
    }
    setHeader(header);

    while (!fixData->eof())
    {
        string lineStr = fixData->readLine();
        StringUtils::trim(lineStr);
        if (lineStr.size() == 0)
        {
            continue;
        }
        auto line = NavDataUtils::splitLine(lineStr);
        if (line.size() < 11)
        {
            continue;
        }

        int type = atoi(line[0].c_str());
        if (!(type == 2 || type == 3 || /* type == 4 || type == 5 || */ type == 12 || type == 13))
        {
            continue;
        }

        NavAidType navAidType = NavAidType::WAY_POINT;
        switch (type)
        {
            case 2: navAidType = NavAidType::NDB; break;
            case 3: navAidType = NavAidType::VOR; break;
            //case 4:
            //case 5: navAidType = NavAidType::LOC; break;
            case 12:
            case 13: navAidType = NavAidType::DME; break;
        }

        double lat = atof(line[1].c_str());
        double lng = atof(line[2].c_str());
        string id = line[7];
        int elevation = atoi(line[3].c_str());
        int frequency = atoi(line[4].c_str());
        // printf("loadNavAids: type=%d -> %d, lat=%0.2f, lng=%0.2f, id=%s\n", type, (int)navAidType, lat, lng, id.c_str());
        auto navAid = make_shared<NavAid>();
        navAid->setLocation(Coordinate(lat, lng));
        navAid->setId(id);
        navAid->setType(navAidType);
        navAid->setElevation(elevation);
        navAid->setFrequency(frequency);
        addNavAid(navAid);
    }

    return true;
}

