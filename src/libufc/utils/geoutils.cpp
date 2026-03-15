//
// Created by Ian Parker on 12/08/2024.
//

#include <cmath>
#include <ufc/geoutils.h>

using namespace std;
using namespace UFC;

float degreesToRadians(float degrees)
{
    return degrees * M_PI / 180.0f;
}

double GeoUtils::distance(Coordinate c1, Coordinate c2)
{
    constexpr float earthRadiusKm = 6371;

    const auto dLat = degreesToRadians(c2.latitude-c1.latitude);
    const auto dLon = degreesToRadians(c2.longitude-c1.longitude);

    const float lat1 = degreesToRadians(c1.latitude);
    const float lat2 = degreesToRadians(c2.latitude);

    const auto a =
        sinf(dLat/2.0f) * sinf(dLat/2.0f) +
        sinf(dLon/2.0f) * sinf(dLon/2.0f) *
        cosf(lat1) * cosf(lat2);
    const auto c = 2.0f * atan2f(sqrtf(a), sqrtf(1-a));
    return earthRadiusKm * c;
}

double GeoUtils::angleFromCoordinate(Coordinate coord1, Coordinate coord2)
{
    double phi1 = degreesToRadians(coord1.latitude);
    double phi2 = degreesToRadians(coord2.latitude);

    double dLon = degreesToRadians(coord2.longitude - coord1.longitude);
    double y = sin(dLon) * cos(phi2);
    double x =
        cos(phi1) * sin(phi2) -
        sin(phi1) * cos(phi2) *
        cos(dLon);

    return atan2(y, x);
}

void Polygon::updateCentre()
{
    centre = Coordinate();
    for (auto point : points)
    {
        centre.latitude += point.latitude;
        centre.longitude += point.longitude;
    }

    centre.latitude /= static_cast<float>(points.size());
    centre.longitude /= static_cast<float>(points.size());
}

bool Polygon::intersects(Coordinate point) const
{
    const auto num_vertices = points.size();
    const auto x = point.x;
    const auto y = point.y;
    bool inside = false;

    Coordinate p1 = points[0];
    for (size_t i = 1; i <= num_vertices; i++)
    {
        Coordinate p2 = points[i % num_vertices];

        // Check if the point is above the minimum y coordinate of the edge
        // Check if the point is below the maximum y coordinate of the edge
        // Check if the point is to the left of the maximum x coordinate of the edge
        if (y > min(p1.y, p2.y) && y <= max(p1.y, p2.y) && x <= max(p1.x, p2.x))
        {
            // Calculate the x-intersection of the line connecting the point to the edge
            double xIntersection = (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;

            // Check if the point is on the same line as the edge or to the left of the x-intersection
            if (p1.x == p2.x || x <= xIntersection)
            {
                // Flip the inside flag
                inside = !inside;
            }
        }

        p1 = p2;
    }

    return inside;
}

template <class T> void QuadTreeNode<T>::dump(int level) const
{
    const auto spaces = string(level * 2, ' ');
    printf("dump: %s%ls\n", spaces.c_str(), m_boundary.toString().c_str());
    for (const auto& object : m_objects)
    {
        printf("dump: %s  -> Object: %ls\n", spaces.c_str(), object->toString().c_str());
    }
    for (const auto& child : m_children)
    {
        if (!child->m_children.empty() || !child->m_objects.empty())
        {
            child->dump(level + 1);
        }
    }
}

template <class T> void QuadTree<T>::dump() const
{
    m_root->dump(0);
}
