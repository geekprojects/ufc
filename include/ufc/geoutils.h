//
// Created by Ian Parker on 12/08/2024.
//

#ifndef UFC_GEOUTILS_H
#define UFC_GEOUTILS_H

#include <memory>
#include <vector>
#include <set>
#include <functional>
#include <cmath>

#ifdef DEBUG_QUADTREE
#undef DEBUG_QUADTREE
#endif

namespace UFC
{
struct Coordinate
{
    float latitude = 0.0f;
    float longitude = 0.0f;
    float altitude = 0.0f;

    Coordinate() = default;

    Coordinate(float latitude, float longitude) : latitude(latitude), longitude(longitude)
    {
    }

    Coordinate(float latitude, float longitude, float altitude) :
        latitude(latitude),
        longitude(longitude),
        altitude(altitude)
    {
    }

    [[nodiscard]] std::wstring toString() const
    {
        return std::to_wstring(latitude) + L", " + std::to_wstring(longitude) + L", " + std::to_wstring(altitude);
    }
};

class Locationable
{
public:
    virtual ~Locationable() = default;

    [[nodiscard]] virtual Coordinate getLocation() const = 0;

    [[nodiscard]] virtual std::wstring toString() const
    {
        return L"[Locationable " + getLocation().toString() + L"]";
    }
};

class GeoUtils
{
 public:
    static double distance(Coordinate c1, Coordinate c2);

    static double angleFromCoordinate(Coordinate coord1, Coordinate coord2);
};

struct Box
{
    Coordinate position;
    float size = 0.0f;

    Box() = default;

    Box(const float latitude, const float longitude, const float size) :
        position(Coordinate(latitude, longitude)),
        size(size)
    {
    }

    Box(const Coordinate position, const float size) : position(position), size(size)
    {
    }

    [[nodiscard]] std::wstring toString() const
    {
        return
            std::to_wstring(minLatitude()) + L", " +
            std::to_wstring(minLongitude()) + L" -> " +
            std::to_wstring(maxLatitude()) + L", " +
            std::to_wstring(maxLongitude());
    }

    [[nodiscard]] float minLatitude() const
    {
        return position.latitude;
    }

    [[nodiscard]] float minLongitude() const
    {
        return position.longitude;
    }

    [[nodiscard]] float maxLatitude() const
    {
        return position.latitude + size;
    }

    [[nodiscard]] float maxLongitude() const
    {
        return position.longitude + size;
    }

    [[nodiscard]] float midLatitude() const
    {
        return position.latitude + (size / 2);
    }

    [[nodiscard]] float midLongitude() const
    {
        return position.longitude + (size / 2);
    }

    [[nodiscard]] bool contains(Coordinate c) const
    {
        bool res = (
            c.latitude >= position.latitude &&
            c.latitude < position.latitude + size &&
            c.longitude >= position.longitude &&
            c.longitude < position.longitude + size);
        return res;
    }

    [[nodiscard]] bool intersects(Box box) const
    {
        return !(minLatitude() >= box.maxLatitude() || maxLatitude() <= box.minLatitude() ||
                 minLongitude() >= box.maxLongitude() || maxLongitude() <= box.minLongitude());
    }
};

template<class T>
class QuadTreeNode : public std::enable_shared_from_this<QuadTreeNode<T> >
{
    const int QT_NODE_CAPACITY = 4;

    std::shared_ptr<QuadTreeNode> m_parent;

    Box m_boundary;
    std::vector<std::shared_ptr<QuadTreeNode> > m_children;
    std::vector<std::shared_ptr<T> > m_objects;

    void subdivide()
    {
        float halfSize = m_boundary.size / 2.0f;

        // North West
        auto northWest = make_shared<QuadTreeNode>(
            this->shared_from_this(),
            m_boundary.minLatitude(),
            m_boundary.minLongitude(),
            halfSize);
        m_children.push_back(northWest);

        // North East
        auto northEast = make_shared<QuadTreeNode>(
            this->shared_from_this(),
            m_boundary.minLatitude(),
            m_boundary.minLongitude() + halfSize,
            halfSize);
        m_children.push_back(northEast);

        // South East
        auto southEast = make_shared<QuadTreeNode>(
            this->shared_from_this(),
            m_boundary.minLatitude() + halfSize,
            m_boundary.minLongitude() + halfSize,
            halfSize);
        m_children.push_back(southEast);

        // South West
        auto southWest = make_shared<QuadTreeNode>(
            this->shared_from_this(),
            m_boundary.minLatitude() + halfSize,
            m_boundary.minLongitude(),
            halfSize);
        m_children.push_back(southWest);

        for (const auto &object: m_objects)
        {
            for (const auto &child: m_children)
            {
                if (child->insert(object))
                {
                    break;
                }
            }
        }
        m_objects.clear();
    }

public:
    QuadTreeNode(float latitude, float longitude, float size) : m_boundary(Coordinate(latitude, longitude), size)
    {
    }

    QuadTreeNode(std::shared_ptr<QuadTreeNode> parent, float latitude, float longitude, float size) : m_parent(parent),
        m_boundary(Coordinate(latitude, longitude), size)
    {
    }

    std::shared_ptr<QuadTreeNode> findNearestNode(Coordinate point)
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
        return this->shared_from_this();
    }

    bool insert(const std::shared_ptr<T> &object)
    {
        if (!m_boundary.contains(object->getLocation()))
        {
            return false;
        }

        if ((int) m_objects.size() < QT_NODE_CAPACITY && m_children.empty())
        {
            // printf("XXX: QuadTree::insert: %ls: Adding as object: %ls\n", m_boundary.toString().c_str(), object->toString().c_str());
            m_objects.push_back(object);
            return true;
        }

        if (m_children.empty())
        {
            subdivide();
        }

        for (const auto &child: m_children)
        {
            if (child->insert(object))
            {
                return true;
            }
        }
        printf("XXX: QuadTree::insert: This shouldn't happen!\n");
        abort();
    }


    std::set<std::shared_ptr<T> > getAllObjects() const
    {
        std::set<std::shared_ptr<T> > results;
        if (!m_children.empty())
        {
            for (const auto &child: m_children)
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


    [[nodiscard]] std::shared_ptr<QuadTreeNode> getParent() const
    {
        return m_parent;
    }

    [[nodiscard]] Box getBoundary() const
    {
        return m_boundary;
    }

    [[nodiscard]] std::vector<std::shared_ptr<QuadTreeNode> > getChildren() const
    {
        return m_children;
    }

    [[nodiscard]] std::vector<std::shared_ptr<T> > getObjects() const
    {
        return m_objects;
    }

    void dump(int level = 0) const;

    [[nodiscard]] std::wstring toString() const
    {
        return
            L"[QuadTreeNode: " + m_boundary.toString() +
            L", children=" + std::to_wstring(m_children.size()) +
            L", objects=" + std::to_wstring(m_objects.size()) + L"]";
    }
};

template<class T>
class QuadTree
{
    std::shared_ptr<QuadTreeNode<T> > m_root;

 public:
    QuadTree(float latitude, float longitude, float size) :
        m_root(std::make_shared<QuadTreeNode<T> >(latitude, longitude, size))
    {
    }

    bool insert(const std::shared_ptr<T> &object)
    {
        return m_root->insert(object);
    }

    void checkParent(auto parent, std::set<std::shared_ptr<T>> results, auto& r, float latOffset, float longOffset) const
    {
        auto northCoord = Coordinate(
            parent->getBoundary().minLatitude() + latOffset,
            parent->getBoundary().midLongitude() + longOffset);
        auto nearestNode = m_root->findNearestNode(northCoord);
        if (nearestNode != nullptr)
        {
            auto north = nearestNode->getParent();
            if (north != nullptr)
            {
#ifdef DEBUG_QUADTREE
                printf("QuadTree::findNearest: north: %ls\n", north->toString().c_str());
#endif
                r = north->getAllObjects();
                results.insert(r.begin(), r.end());
            }
        }
    }

    std::shared_ptr<T> findNearest(Coordinate point, const std::function<bool(std::shared_ptr<T>)> &filter) const
    {
        auto node = m_root->findNearestNode(point);

#ifdef DEBUG_QUADTREE
        printf("QuadTree::findNearest: Closest node: %ls\n", node->toString().c_str());
#endif
        auto parent = node->getParent();
#ifdef DEBUG_QUADTREE
        printf("QuadTree::findNearest: Parent node: %ls\n", parent->toString().c_str());
#endif

        std::set<std::shared_ptr<T> > results;
        auto r = parent->getAllObjects();
        results.insert(r.begin(), r.end());

        checkParent(parent, results, r, -0.01, 0.0f);
        checkParent(parent, results, r, +0.01, 0.0f);
        checkParent(parent, results, r, 0.0f, -0.01);
        checkParent(parent, results, r, 0.0f, +0.01);

        float nearestDistance = std::numeric_limits<float>::max();
        std::shared_ptr<T> nearestObject = nullptr;
        for (auto object: results)
        {
            if (!filter(object))
            {
                continue;
            }
            float d = GeoUtils::distance(object->getLocation(), point);
#ifdef DEBUG_QUADTREE
            printf("QuadTreeNode::findNearestNode: Possible: %ls (distance=%0.2f)\n", object->toString().c_str(), d);
#endif
            if (d < nearestDistance)
            {
                nearestDistance = d;
                nearestObject = object;
            }
        }
#ifdef DEBUG_QUADTREE
        printf(
            "QuadTreeNode::findNearestNode: Nearest: %ls (distance=%0.2f)\n",
            nearestObject->toString().c_str(),
            nearestDistance);
#endif

        return nearestObject;
    }


    void dump() const;
};
}

#endif //GEOUTILS_H
