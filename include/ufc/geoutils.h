//
// Created by Ian Parker on 12/08/2024.
//

#ifndef UFC_GEOUTILS_H
#define UFC_GEOUTILS_H

#include <memory>
#include <vector>
#include <set>

namespace UFC
{

struct Coordinate
{
    float latitude = 0.0f;
    float longitude = 0.0f;
    float altitude = 0.0f;

    Coordinate() = default;
    Coordinate(float latitude, float longitude) : latitude(latitude), longitude(longitude) {}
    Coordinate(float latitude, float longitude, float altitude) : latitude(latitude), longitude(longitude), altitude(altitude) {}

    std::wstring toString() const
    {
        return std::to_wstring(latitude) + L", " + std::to_wstring(longitude) + L", " + std::to_wstring(altitude);
    }
};

class Locationable
{
 public:
    virtual ~Locationable() = default;
    virtual Coordinate getLocation() const = 0;

    virtual std::wstring toString() const
    {
        return L"[Locationable " + getLocation().toString() + L"]";
    }
};

class GeoUtils
{
private:

public:
    static double distance(Coordinate c1, Coordinate c2);
};

struct Box
{
    Coordinate position;
    float size;

    Box() = default;
    Box(float latitude, float longitude, float size) : position(Coordinate(latitude, longitude)), size(size) {}
    Box(Coordinate position, float size) : position(position), size(size) {}

    std::wstring toString() const
    {
        return
            std::to_wstring(minLatitude()) + L", " +
            std::to_wstring(minLongitude()) + L" -> " +
            std::to_wstring(maxLatitude()) + L", " +
            std::to_wstring(maxLongitude());
    }

    float minLatitude() const
    {
        return position.latitude;
    }

    float minLongitude() const
    {
        return position.longitude;
    }

    float maxLatitude() const
    {
        return position.latitude + size;
    }

    float maxLongitude() const
    {
        return position.longitude + size;
    }

    float midLatitude() const
    {
        return position.latitude + (size / 2);
    }

    float midLongitude() const
    {
        return position.longitude + (size / 2);
    }

    bool contains(Coordinate c) const
    {
        bool res = (
            c.latitude >= position.latitude &&
            c.latitude < position.latitude + size &&
            c.longitude >= position.longitude &&
            c.longitude < position.longitude + size);
        return res;
    }

    bool intersects(Box box) const
    {
        return !(minLatitude() >= box.maxLatitude() || maxLatitude() <= box.minLatitude() ||
            minLongitude() >= box.maxLongitude() || maxLongitude() <= box.minLongitude());
    }
};

class QuadTreeNode : public std::enable_shared_from_this<QuadTreeNode>
{
 private:
    const int QT_NODE_CAPACITY = 4;

    /*
    const int QT_NODE_NORTHWEST = 0;
    const int QT_NODE_NORTHEAST = 1;
    const int QT_NODE_SOUTHWEST = 2;
    const int QT_NODE_SOUTHEAST = 3;
    */

    std::shared_ptr<QuadTreeNode> m_parent;

    Box m_boundary;
    std::vector<std::shared_ptr<QuadTreeNode>> m_children;
    std::vector<std::shared_ptr<Locationable>> m_objects;

    void subdivide();

 public:
    QuadTreeNode(float latitude, float longitude, float size) : m_boundary(Coordinate(latitude, longitude), size) {}
    QuadTreeNode( std::shared_ptr<QuadTreeNode> parent, float latitude, float longitude, float size) :
        m_parent(parent),
        m_boundary(Coordinate(latitude, longitude), size) {}

    std::shared_ptr<QuadTreeNode> findNearestNode(Coordinate point);
    bool insert(const std::shared_ptr<Locationable> &object);

    std::set<std::shared_ptr<Locationable>> getAllObjects() const;

    [[nodiscard]] std::shared_ptr<QuadTreeNode> getParent() const
    {
        return m_parent;
    }

    [[nodiscard]] Box getBoundary() const
    {
        return m_boundary;
    }

    [[nodiscard]] std::vector<std::shared_ptr<QuadTreeNode>> getChildren() const
    {
        return m_children;
    }

    [[nodiscard]] std::vector<std::shared_ptr<Locationable>> getObjects() const
    {
        return m_objects;
    }

    void dump(int level = 0) const;

    std::wstring toString() const
    {
        return
            L"[QuadTreeNode: " + m_boundary.toString() +
            L", children=" + std::to_wstring(m_children.size()) +
            L", objects=" + std::to_wstring(m_objects.size()) + L"]";
    }
};

class QuadTree
{
private:
    std::shared_ptr<QuadTreeNode> m_root;

public:
    QuadTree(float latitude, float longitude, float size);
    bool insert(const std::shared_ptr<Locationable> &object);

    std::shared_ptr<Locationable> findNearest(Coordinate point, const std::function<bool(std::shared_ptr<Locationable>)> &filter) const;

    void dump() const;
};

}

#endif //GEOUTILS_H
