//
// Created by Ian Parker on 14/09/2024.
//

#include "xplanenavdata.h"
#include "ufc/utils.h"
#include <ufc/flightconnector.h>

#include "navdata.h"
#include "ufc/data.h"

using namespace std;
using namespace UFC;

bool XPlaneAirports::init()
{
    string aptData = getFlightConnector()->getConfig().xplanePath + "/Global Scenery/Global Airports/Earth nav data/apt.dat";
    log(DEBUG, "init: Loading airports from %s", aptData.c_str());

    auto data = make_shared<Data>();
    if (!data->load(aptData))
    {
        log(ERROR, "init: Failed to load airports from %s", aptData.c_str());
        return false;
    }

    data->readLine();
    string headerStr = data->readLine();

    shared_ptr<Airport> currentAirport = nullptr;
    double currentLat = 0.0;
    RunwayRule runwayRule;
    bool hasRule = false;

    while (!data->eof())
    {
        string line = data->readLine();
        vector<string> parts = NavDataUtils::splitLine(line);
        if (parts.empty())
        {
            continue;
        }

        int type = stoi(parts.at(0));
        switch (type)
        {
            case 1:
            case 16:
            case 17:
                // New airport

                // Flush the current airport
                if (currentAirport != nullptr)
                {
                    if (hasRule)
                    {
                        currentAirport->addRule(runwayRule);
                    }
                    addAirport(currentAirport);
                }

                currentAirport = make_shared<Airport>();
                currentLat = 0.0;
                currentAirport->setName(utf82wstring(NavDataUtils::joinToEnd(parts, 5).c_str()));
                //log(DEBUG, "init: Airport: %ls", currentAirport->getName().c_str());
                currentAirport->setElevation(stof(parts.at(1)));
                if (type == 1 || type == 16)
                {
                    currentAirport->setHasRunway(true);
                }
                runwayRule = {};
                hasRule = false;
                break;

            case 100: // Runway
            {
                int ends = (parts.size() - 8) / 9;
                if (ends == 2)
                {
                    vector<Runway> runways;
                    for (int i = 0; i < ends; i++)
                    {
                        Runway runway;
                        runway.m_number = utf82wstring(parts.at(8 + (i * 9) + 0).c_str());
                        float lat = atof(parts.at(8 + (i * 9) + 1).c_str());
                        float lon = atof(parts.at(8 + (i * 9) + 2).c_str());
                        runway.m_startLocation = Coordinate(lat, lon);
                        runways.push_back(runway);
                    }

                    float length = GeoUtils::distance(runways.at(0).m_startLocation, runways.at(1).m_startLocation) * 1000.0f;
                    runways.at(0).m_length = length;
                    runways.at(1).m_length = length;

                    Coordinate r0 = runways.at(0).m_startLocation;
                    Coordinate r1 = runways.at(1).m_startLocation;
                    double angle = GeoUtils::angleFromCoordinate(r0, r1);
                    float variation = 0.0f;
                    angle = angle * (180.0 / M_PI);
                    if (r0.longitude < 0)
                    {
                        angle -= variation;
                    }
                    else
                    {
                        angle += variation;
                    }
                    angle = fmod(angle + 360, 360);
                    runways.at(0).m_bearing = angle;
                    //log(INFO, "%s %s: %ls bearing=%0.2f, variation=%0.2f", currentAirport->getICAOCode().c_str(), runways.at(0).m_number.c_str(), runways.at(0).m_startLocation.toString().c_str(), angle, variation);

                    angle = GeoUtils::angleFromCoordinate(runways.at(1).m_startLocation, runways.at(0).m_startLocation);
                    /*
                    wmm_get_magnetic_variation(
    runways.at(1).m_startLocation.latitude,
    runways.at(1).m_startLocation.longitude,
    wmmTime,
    &variation);
    */
                    angle = angle * (180.0 / M_PI);
                    angle -= variation;
                    angle = fmod(angle + 360, 360);
                    runways.at(1).m_bearing = angle;

                    vector<Runway> currentRunways = currentAirport->getRunways();
                    currentRunways.insert(currentRunways.end(), runways.begin(), runways.end());
                    currentAirport->setRunways(currentRunways);

                    //log(INFO, "%s %s: %ls bearing=%0.2f", currentAirport->getICAOCode().c_str(), runways.at(0).m_number.c_str(), runways.at(0).m_startLocation.toString().c_str(), runways.at(0).m_bearing);
                    //log(INFO, "%s %s: %ls bearing=%0.2f", currentAirport->getICAOCode().c_str(), runways.at(1).m_number.c_str(), runways.at(1).m_startLocation.toString().c_str(), runways.at(1).m_bearing);
                }
                else
                {
                    log(WARN, "Runway with %d ends!\n", ends);
                }

                break;
            }

            case 1000:
                //log(INFO, "init:  -> Runway Rule: %s", NavDataUtils::joinToEnd(parts, 1).c_str());
                if (hasRule)
                {
                    currentAirport->addRule(runwayRule);
                }
                runwayRule = {};
                hasRule = true;
                runwayRule.name = utf82wstring(NavDataUtils::joinToEnd(parts, 1).c_str());
                break;

            case 1001:
            {
                runwayRule.metarStation = utf82wstring(parts.at(1).c_str());

                RunwayWindRule windRule;
                windRule.directionMinimum = atoi(parts.at(2).c_str());
                windRule.directionMaximum = atoi(parts.at(3).c_str());

                if (windRule.directionMinimum == 1 && windRule.directionMaximum == 360)
                {
                    // These values aren't technically valid!
                    windRule.directionMinimum = 0;
                    windRule.directionMaximum = 359;
                }

                windRule.speedMaximum = atoi(parts.at(4).c_str());
                runwayRule.windRules.push_back(windRule);
                break;
            }

            case 1002:
                runwayRule.metarStation = utf82wstring(parts.at(1).c_str());
                runwayRule.ceilingMinimum = atoi(parts.at(2).c_str());
                break;

            case 1003:
                runwayRule.metarStation = utf82wstring(parts.at(1).c_str());
                runwayRule.visibilityMinimum = atof(parts.at(2).c_str());
                break;

            case 1004:
                runwayRule.timeMinimum = atoi(parts.at(1).c_str());
                runwayRule.timeMaximum = atoi(parts.at(2).c_str());
                break;

            case 1100:
            case 1110:
            {
                //log(INFO, "init:  -> Runway In Use Rule: %s", parts.at(1).c_str());
                RunwayInUse runwayInUse;
                runwayInUse.identifier = utf82wstring(parts.at(1).c_str());
                runwayInUse.channel = atoi(parts.at(2).c_str());

                string arrdep = parts.at(3);
                runwayInUse.arrival = arrdep.find("arrivals") != string::npos;
                runwayInUse.departure = arrdep.find("departures") != string::npos;

                runwayRule.runways.push_back(runwayInUse);
                break;
            }

            case 1302:
                if (parts.at(1) == "datum_lat")
                {
                    currentLat = stod(parts.at(2));
                }
                else if (parts.at(1) == "datum_lon")
                {
                    double longitude = stod(parts.at(2));
                    Coordinate coord = Coordinate(currentLat, longitude);
                    currentAirport->setLocation(coord);
                }
                else if (parts.at(1) == "icao_code")
                {
                    //printf("Adding icao_code: %s\n", parts.at(2).c_str());
                    currentAirport->setICAOCode(parts.at(2));
                }
            break;

            case 1050:
            case 1051:
            case 1052:
            case 1053:
            case 1054:
            case 1055:
            case 1056:
            {
                Controller controller;

                switch (type)
                {
                    case 1050: controller.type = ControllerType::ATIS; break;
                    case 1051: controller.type = ControllerType::UNICOM; break;
                    case 1052: controller.type = ControllerType::DELIVERY; break;
                    case 1053: controller.type = ControllerType::GROUND; break;
                    case 1054: controller.type = ControllerType::TOWER; break;
                    case 1055: controller.type = ControllerType::APPROACH; break;
                    case 1056: controller.type = ControllerType::DEPARTURE; break;
                }

                controller.channel = stoi(parts.at(1));
                controller.name = utf82wstring(NavDataUtils::joinToEnd(parts, 2).c_str());
                currentAirport->getControllers().push_back(controller);

                break;
            }

            default:
                // Ignore!
                    break;
        }
    }
    if (currentAirport != nullptr)
    {
        if (hasRule)
        {
            currentAirport->addRule(runwayRule);
        }
        addAirport(currentAirport);
    }

    //airportData->dump();

    //auto point = Coordinate(51.1987, 0.2764); // Tonbridge, UK
    //auto point = Coordinate(40.748333, -73.985278); // Empire State Building
    //auto point = Coordinate(40.69880370388995, -73.86135782941997);
    //auto point = Coordinate(51.836826507016895, -0.18500002245520308);
    //airportData->findNearest(point);

    return true;
}
