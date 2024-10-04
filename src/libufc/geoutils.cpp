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
    const float earthRadiusKm = 6371;

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

shared_ptr<QuadTreeNode> QuadTreeNode::findNearestNode(Coordinate point)
{
    if (!m_boundary.contains(point))
    {
        return nullptr;
    }

    for (const auto& child : m_children)
    {
        if (child->m_boundary.contains(point))
        {
            return child->findNearestNode(point);
        }
    }
    return shared_from_this();
}

bool QuadTreeNode::insert(const shared_ptr<Locationable> &object)
{
    if (!m_boundary.contains(object->getLocation()))
    {
        return false;
    }

    if (m_objects.size() < QT_NODE_CAPACITY && m_children.empty())
    {
       // printf("XXX: QuadTree::insert: %ls: Adding as object: %ls\n", m_boundary.toString().c_str(), object->toString().c_str());
        m_objects.push_back(object);
        return true;
    }

    if (m_children.empty())
    {
        subdivide();
    }

    for (auto child : m_children)
    {
        if (child->insert(object))
        {
            return true;
        }
    }
    printf("XXX: QuadTree::insert: This shouldn't happen!\n");
    abort();
}

set<shared_ptr<Locationable>> QuadTreeNode::getAllObjects() const
{
    set<shared_ptr<Locationable>> results;
    if (!m_children.empty())
    {
        for (const auto& child : m_children)
        {
            auto objs = child->getAllObjects();
            results.insert(objs.begin(), objs.end());
        }
    }
    else
    {
        results.insert(m_objects.begin(), m_objects.end());
    }
    return results;
}


void QuadTreeNode::subdivide()
{
    float halfSize = m_boundary.size / 2.0f;

    // North West
    auto northWest = make_shared<QuadTreeNode>(shared_from_this(), m_boundary.minLatitude(), m_boundary.minLongitude(), halfSize);
    m_children.push_back(northWest);

    // North East
    auto northEast = make_shared<QuadTreeNode>(shared_from_this(), m_boundary.minLatitude(), m_boundary.minLongitude() + halfSize, halfSize);
    m_children.push_back(northEast);

    // South East
    auto southEast = make_shared<QuadTreeNode>(shared_from_this(), m_boundary.minLatitude() + halfSize, m_boundary.minLongitude() + halfSize, halfSize);
    m_children.push_back(southEast);

    // South West
    auto southWest = make_shared<QuadTreeNode>(shared_from_this(), m_boundary.minLatitude() + halfSize, m_boundary.minLongitude(), halfSize);
    m_children.push_back(southWest);

    for (const auto& object : m_objects)
    {
        for (const auto& child : m_children)
        {
            if (child->insert(object))
            {
                break;
            }
        }
    }
    m_objects.clear();
}

void QuadTreeNode::dump(int level) const
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

QuadTree::QuadTree(float latitude, float longitude, float size)
{
    m_root = make_shared<QuadTreeNode>(latitude, longitude, size);
}

bool QuadTree::insert(const shared_ptr<Locationable> &object)
{
    return m_root->insert(object);
}

shared_ptr<Locationable> QuadTree::findNearest(Coordinate point, const function<bool(shared_ptr<Locationable>)>& filter) const
{
    auto node = m_root->findNearestNode(point);

    printf("QuadTree::findNearest: Closest node: %ls\n", node->toString().c_str());
    auto parent = node->getParent();
    printf("QuadTree::findNearest: Parent node: %ls\n", parent->toString().c_str());

    set<shared_ptr<Locationable>> results;
    auto r = parent->getAllObjects();
    results.insert(r.begin(), r.end());

    {
        auto northCoord = Coordinate(parent->getBoundary().minLatitude() - 0.01, parent->getBoundary().midLongitude());
        auto north = m_root->findNearestNode(northCoord)->getParent();
        printf("QuadTree::findNearest: north: %ls\n", north->toString().c_str());
        r = north->getAllObjects();
        results.insert(r.begin(), r.end());
    }
    {
        auto southCoord = Coordinate(parent->getBoundary().maxLatitude() + 0.01, parent->getBoundary().midLongitude());
        auto south = m_root->findNearestNode(southCoord)->getParent();
        printf("QuadTree::findNearest: south: %ls\n", south->toString().c_str());
        r = south->getAllObjects();
        results.insert(r.begin(), r.end());
    }
    {
        auto eastCoord = Coordinate(parent->getBoundary().midLatitude(), parent->getBoundary().maxLongitude() + 0.01);
        auto east = m_root->findNearestNode(eastCoord)->getParent();
        printf("QuadTree::findNearest: east: %ls\n", east->toString().c_str());
        r = east->getAllObjects();
        results.insert(r.begin(), r.end());
    }
    {
        auto westCoord = Coordinate(parent->getBoundary().midLatitude(), parent->getBoundary().minLongitude() - 0.01);
        auto west = m_root->findNearestNode(westCoord)->getParent();
        printf("QuadTree::findNearest: west: %ls\n", west->toString().c_str());
        r = west->getAllObjects();
        results.insert(r.begin(), r.end());
    }

    float nearestDistance = numeric_limits<float>::max();
    shared_ptr<Locationable> nearestObject = nullptr;
    for (auto object : results)
    {
        if (!filter(object))
        {
            continue;
        }
        float d = GeoUtils::distance(object->getLocation(), point);
        printf("QuadTreeNode::findNearestNode: Possible: %ls (distance=%0.2f)\n", object->toString().c_str(), d);
        if (d < nearestDistance)
        {
            nearestDistance = d;
            nearestObject = object;
        }
    }
    printf("QuadTreeNode::findNearestNode: Nearest: %ls (distance=%0.2f)\n", nearestObject->toString().c_str(), nearestDistance);

    return nullptr;
}

void QuadTree::dump() const
{
    m_root->dump(0);
}
