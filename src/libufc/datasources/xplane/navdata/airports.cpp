//
// Created by Ian Parker on 14/09/2024.
//

#include "navdata.h"
#include "../xplane.h"
#include <ufc/flightconnector.h>
#include <ufc/airports.h>

#include "ufc/utils.h"

using namespace std;
using namespace UFC;

shared_ptr<Airports> XPlaneDataSource::loadAirports()
{
    string aptData = m_flightConnector->getConfig().xplanePath + "/Global Scenery/Global Airports/Earth nav data/apt.dat";

    auto data = make_shared<Data>();
    data->load(aptData);

    auto airportData = make_shared<Airports>();

    data->readLine();
    string headerStr = data->readLine();

    shared_ptr<Airport> currentAirport = nullptr;
    double currentLat = 0.0;
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
                    airportData->addAirport(currentAirport);
                }

                currentAirport = make_shared<Airport>();
                currentLat = 0.0;
                currentAirport->setName(utf82wstring(NavDataUtils::joinToEnd(parts, 5).c_str()));
                currentAirport->setElevation(stof(parts.at(1)));
                if (type == 1 || type == 16)
                {
                    currentAirport->setHasRunway(true);
                }
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
                        runway.m_number = parts.at(8 + (i * 9) + 0);
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

                    //log(INFO, "%s %s: %ls bearing=%0.2f", currentAirport->getICAOCode().c_str(), runways.at(1).m_number.c_str(), runways.at(1).m_startLocation.toString().c_str(), angle);
                }
                else
                {
                    log(WARN, "Runway with %d ends!\n", ends);
                }

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

            default:
                // Ignore!
                    break;
        }
    }
    if (currentAirport != nullptr)
    {
        airportData->addAirport(currentAirport);
    }

    //airportData->dump();

    //auto point = Coordinate(51.1987, 0.2764); // Tonbridge, UK
    //auto point = Coordinate(40.748333, -73.985278); // Empire State Building
    //auto point = Coordinate(40.69880370388995, -73.86135782941997);
    //auto point = Coordinate(51.836826507016895, -0.18500002245520308);
    //airportData->findNearest(point);

    //printf("Found %zu airports\n", airportData->m_airports.size());
    return airportData;
}
