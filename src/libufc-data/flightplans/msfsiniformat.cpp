//
// Created by Ian Parker on 23/10/2024.
//

#include "ufc/data/msfsiniformat.h"

#include "ufc/utils/utils.h"

using namespace std;
using namespace UFC;

float dmmToDegrees(string dmm)
{
    char d = dmm.at(0);
    dmm = dmm.substr(1);

    auto degrees = (float)atof(dmm.c_str());
    auto idx = dmm.find(' ');
    dmm = dmm.substr(idx + 1);

    auto dm = (float)atof(dmm.c_str());
    degrees += (dm / 60.0f);
    if (d == 'S' || d == 'W')
    {
        degrees = -degrees;
    }

    return degrees;
}

/*
bool MSFSIniFormat::check(std::string fileName, const std::vector<std::vector<std::string>>& text)
{
    return text.size() > 0 && text.at(0).at(0) == "[flightplan]";
}
*/

shared_ptr<FlightPlan> MSFSIniFormat::loadFile(string filename)
{
    auto text = readTextFile(filename, false);

    if (text.at(0).at(0) != "[flightplan]")
    {
        printf("MSFSFormat::load: Not a MSFS Flight Plan\n");
    }

    auto flightPlan = make_shared<FlightPlan>();
    vector<RoutePoint> points;
    for (auto lineVec : text)
    {
        string line = lineVec.at(0);
        string key;
        string value;
        auto idx = line.find('=');
        if (idx != string::npos)
        {
            key = line.substr(0, idx);
            value = line.substr(idx + 1);
        }
        StringUtils::trim(key);
        StringUtils::trim(value);

        if (key == "type")
        {
            printf("MSFSFormat::load: Type=%s\n", value.c_str());
            if (value == "VFR")
            {
                flightPlan->setType(FlightPlanType::VFR);
            }
            else if (value == "IFR")
            {
                flightPlan->setType(FlightPlanType::IFR);
            }
        }
        else if (key == "departure_id")
        {
            idx = value.find(',');
            if (idx != string::npos)
            {
                value = value.substr(0, idx);
            }
            StringUtils::trim(value);
            printf("MSFSFormat::load: Departure Airport=%s\n", value.c_str());
            flightPlan->setOrigin(getNavDataSource()->getAirports()->findByCode(value));
        }
        else if (key == "departure_position")
        {
            printf("MSFSFormat::load: Departure Runway=%s\n", value.c_str());
            //flightPlan->m_departureRunway = value.c_str();
        }
        else if (key == "destination_id")
        {
            idx = value.find(',');
            if (idx != string::npos)
            {
                value = value.substr(0, idx);
            }
            StringUtils::trim(value);
            printf("MSFSFormat::load: Destination Airport=%s\n", value.c_str());
            flightPlan->setDestination(getNavDataSource()->getAirports()->findByCode(value));
        }
        else if (key == "destination_position")
        {
            printf("MSFSFormat::load: Destination Runway=%s\n", value.c_str());
            //flightPlan->m_destinationRunway = value.c_str();
        }
        else if (key == "cruising_altitude")
        {
            flightPlan->setCruisingAltitude(atoi(value.c_str()));
            printf("MSFSFormat::load: Cruising Altitude=%d\n", flightPlan->getCruisingAltitude());
        }
        else if (key.starts_with("waypoint."))
        {
            printf("MSFSFormat::load: Waypoint: %s\n", value.c_str());
            auto waypointLine = splitString(value, ',');
            string name;
            string typeStr;
            string latStr;
            string lonStr;

            // There doesn't seem to be any specification for this file, but I've seen
            // two different waypoint types!
            if (waypointLine.size() == 5)
            {
                name = waypointLine.at(0);
                typeStr = waypointLine.at(1);
                latStr = waypointLine.at(2);
                lonStr = waypointLine.at(3);
            }
            else
            {
                name = waypointLine.at(0);
                typeStr = waypointLine.at(3);
                latStr = waypointLine.at(4);
                lonStr = waypointLine.at(5);
            }
            printf("MSFSFormat::load: Waypoint: -> name: %s\n", name.c_str());

            StringUtils::trim(typeStr);
            RoutePoint wayPoint;
            if (typeStr == "A")
            {
                // Airport, skip it
                printf("MSFSFormat::load: Waypoint: -> Skipping airport\n");
                continue;
            }

            if (typeStr == "V")
            {
                wayPoint.type = NavAidType::VOR;
            }
            else if (typeStr == "I")
            {
                wayPoint.type = NavAidType::FIX;
            }

            StringUtils::trim(latStr);
            float lat = dmmToDegrees(latStr);
            StringUtils::trim(lonStr);
            float lon = dmmToDegrees(lonStr);
            printf("MSFSFormat::load: Waypoint: -> %0.5f, %0.5f\n", lat, lon);
            wayPoint.name = name;
            wayPoint.position = Coordinate(lat, lon);
            //addWayPoint(flightPlan, wayPoint);
            points.push_back(wayPoint);
        }
        else
        {
            printf("MSFSFormat::load: Unhandled line: %s\n", line.c_str());
        }
    }
    flightPlan->setRoute(points);

    return flightPlan;
}
