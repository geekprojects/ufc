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
    data->readLine();

    shared_ptr<Airport> currentAirport = nullptr;
    double currentLat = 0.0;
    while (!data->eof())
    {
        string line = data->readLine();
        vector<string> parts = NavData::splitLine(line);
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
                if (currentAirport != nullptr)
                {
                    airportData->addAirport(currentAirport);
                }
                currentAirport = make_shared<Airport>();
                currentLat = 0.0;
                currentAirport->setName(utf82wstring(NavData::joinToEnd(parts, 5).c_str()));
                currentAirport->setElevation(stof(parts.at(1)));
                if (type == 1 || type == 16)
                {
                    currentAirport->setHasRunway(true);
                }
                break;

            /*
            case 100:
                break;
                */

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

    airportData->dump();

    //auto point = Coordinate(51.1987, 0.2764); // Tonbridge, UK
    //auto point = Coordinate(40.748333, -73.985278); // Empire State Building
    auto point = Coordinate(40.69880370388995, -73.86135782941997);
    //auto point = Coordinate(51.836826507016895, -0.18500002245520308);
    airportData->findNearest(point);

    //printf("Found %zu airports\n", airportData->m_airports.size());
    return nullptr;
}

