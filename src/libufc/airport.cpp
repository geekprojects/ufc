//
// Created by Ian Parker on 28/08/2024.
//

#include <float.h>
#include <cmath>

#include <ufc/airports.h>
#include <ufc/geoutils.h>

using namespace std;
using namespace UFC;

Airport::Airport(wstring name, Coordinate location) : m_name(name), m_location(location)
{
}

Airports::Airports()
{
    m_airports = make_shared<QuadTree>(-180.0f, -180.0f, 360.0f);
/*
    auto airport1 = make_shared<Airport>(L"Gatwick", Coordinate( 51.148056, -0.190278));
    m_airports->insert(airport1);
    auto airport2 = make_shared<Airport>(L"Heathrow", Coordinate( 51.4775, -0.461389));
    m_airports->insert(airport2);
    auto airport3 = make_shared<Airport>(L"JFK", Coordinate( 40.639722, -73.778889));
    m_airports->insert(airport3);
    auto airport4 = make_shared<Airport>(L"Singapore Changi", Coordinate( 1.359167, 103.989444));
    m_airports->insert(airport4);
    auto airport5 = make_shared<Airport>(L"Sydney", Coordinate( -33.946111, 151.177222));
    m_airports->insert(airport5);

    m_airports->dump();

    auto point = Coordinate(51.1987, 0.2764);
    m_airports->findNearest(point);
*/
    //auto dist = GeoUtils::distance(airport1->getLocation(), airport2->getLocation());
    //printf("Distance: %0.2f\n", dist);

/*
    geos::index::strtree::TemplateSTRtree<Airport*> searchTree;
    searchTree.insert(search);
    vector<Airport*> results;
    m_index.query(*pos->getEnvelopeInternal(), results);
    for (auto result : results)
    {
        printf("result: %ls\n", result->getName().c_str());
    }
    */
}

Airports::~Airports() = default;

void Airports::addAirport(std::shared_ptr<Airport> shared)
{
    if (fabs(shared->getLocation().latitude) < DBL_EPSILON && fabs(shared->getLocation().longitude) < DBL_EPSILON)
    {
        return;
    }
    m_airports->insert(shared);
    //m_airportList.push_back(shared);
    if (!shared->getICAOCode().empty())
    {
        m_airportsByCode.try_emplace(shared->getICAOCode(), shared);
    }
}

std::shared_ptr<Airport> Airports::findByCode(const std::string& code)
{
    auto it = m_airportsByCode.find(code);
    if (it != m_airportsByCode.end())
    {
        return it->second;
    }
    return nullptr;
}

