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
