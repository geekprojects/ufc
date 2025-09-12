//
// Created by Ian Parker on 24/10/2024.
//

#ifndef UFC_NAVAIDS_H
#define UFC_NAVAIDS_H

#include "geoutils.h"
#include "logger.h"

#include <map>

namespace UFC
{
class FlightConnector;
class Airports;
class NavAids;

class NavDataSource : public Logger
{
    FlightConnector* m_flightConnector;

public:
    explicit NavDataSource(FlightConnector* flightConnector, std::string name) : Logger(name), m_flightConnector(flightConnector)
    {
    }

    virtual ~NavDataSource() = default;

    FlightConnector* getFlightConnector() { return m_flightConnector; }

    virtual std::shared_ptr<Airports> getAirports() { return nullptr; }
    virtual std::shared_ptr<NavAids> getNavAids() { return nullptr; }
};

struct NavDataHeader
{
    int version;
    int cycle;
    int build;
    std::string type;
    std::string copyright;
};

class NavData : public Logger
{
 protected:
    NavDataSource* m_navDataSource;
    NavDataHeader m_header = {};

    [[nodiscard]] FlightConnector* getFlightConnector() const { return m_navDataSource->getFlightConnector(); }
 public:
    explicit NavData(NavDataSource* navdataSource, std::string const& name) :
        Logger("NavData[" + name + "]"),
        m_navDataSource(navdataSource)
    {}

    void setHeader(const NavDataHeader& header) { m_header = header; }
    [[nodiscard]] int getCycle() const { return m_header.cycle; }
};

enum class NavAidType
{
    WAY_POINT, // Unnnamed location
    FIX, // Named fix
    NDB,
    VOR,
    LOC,
    DME,
};

class NavAid : public Locationable
{
 private:
    std::string m_id;
    NavAidType m_type = NavAidType::WAY_POINT;
    Coordinate m_location;
    int m_elevation = 0;
    int m_frequency = 0;

 public:
    NavAid() = default;

    [[nodiscard]] std::string getId() const
    {
        return m_id;
    }

    void setId(const std::string& id)
    {
        m_id = id;
    }

    [[nodiscard]] NavAidType getType() const
    {
        return m_type;
    }

    void setType(NavAidType type)
    {
        m_type = type;
    }

    [[nodiscard]] Coordinate getLocation() const override { return m_location; }
    void setLocation(const Coordinate& location) { m_location = location; }

    [[nodiscard]] int getElevation() const
    {
        return m_elevation;
    }
    void setElevation(int elevation)
    {
        m_elevation = elevation;
    }

    [[nodiscard]] int getFrequency() const
    {
        return m_frequency;
    }

    void setFrequency(int frequency)
    {
        m_frequency = frequency;
    }
};

class NavAids : public NavData
{
 private:
    std::shared_ptr<QuadTree<NavAid>> m_navAids;
    std::map<std::string, std::vector<std::shared_ptr<NavAid>>> m_navAidsById;

 protected:
    void addNavAid(std::shared_ptr<NavAid> navaid)
    {
        m_navAids->insert(navaid);

        auto it = m_navAidsById.find(navaid->getId());
        if (it == m_navAidsById.end())
        {
            std::vector<std::shared_ptr<NavAid>> v;
            v.push_back(navaid);
            m_navAidsById.try_emplace(navaid->getId(), v);
        }
        else
        {
            it->second.push_back(navaid);
        }
    }

 public:
    explicit NavAids(NavDataSource* navDataSource, std::string const& name) : NavData(navDataSource, name)
    {
        m_navAids = std::make_shared<QuadTree<NavAid>>(-180.0f, -180.0f, 360.0f);
    }

    ~NavAids() override = default;

    [[nodiscard]] virtual std::shared_ptr<NavAid> findNearest(const Coordinate point) const
    {
        return m_navAids->findNearest(point, [](const std::shared_ptr<NavAid>&)
        {
            return true;
        });
    }

    [[nodiscard]] virtual std::vector<std::shared_ptr<NavAid>> findById(const std::string& id) const
    {
        auto it = m_navAidsById.find(id);
        if (it != m_navAidsById.end())
        {
            return it->second;
        }
        return {};
    }
};


};

#endif //NAVAIDS_H
