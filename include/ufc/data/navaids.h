//
// Created by Ian Parker on 27/03/2026.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_NAVAID_H
#define UNIVERSALFLIGHTCONNECTOR_NAVAID_H

#include <ufc/data/navdata.h>
#include <ufc/utils/geoutils.h>

namespace UFC
{
enum class NavAidType
{
    AIRPORT,
    WAY_POINT, // Unnnamed location
    FIX, // Named fix
    NDB,
    VOR,
    LOC,
    DME,
    AIRWAY,
    DEPARTURE,
    ARRIVAL,
};

class NavAid : public UFC::Locationable
{
    std::string m_id;
    std::string m_name;
    NavAidType m_type = NavAidType::WAY_POINT;
    Coordinate m_location;
    int m_elevation = 0;
    int m_frequency = 0;

    // Airway
    uint64_t m_sequenceNo;

    // Internal data source ids
    uint64_t m_sourceId;
    uint64_t m_nextId;

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

    [[nodiscard]] std::string getName() const
    {
        return m_name;
    }

    void setName(const std::string& name)
    {
        m_name = name;
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

    [[nodiscard]] uint64_t getSequenceNo() const
    {
        return m_sequenceNo;
    }

    void setSequenceNo(uint64_t sequence_no)
    {
        m_sequenceNo = sequence_no;
    }

    [[nodiscard]] uint64_t getSourceId() const
    {
        return m_sourceId;
    }

    void setSourceId(uint64_t source_id)
    {
        m_sourceId = source_id;
    }

    [[nodiscard]] uint64_t getNextId() const
    {
        return m_nextId;
    }

    void setNext_id(uint64_t next_id)
    {
        m_nextId = next_id;
    }
};

class NavAids : public NavData
{
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

    virtual bool init() { return true; }

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

    [[nodiscard]] virtual std::shared_ptr<NavAid> findById(const std::string& id, Coordinate& near) const
    {
        auto navaids = findById(id);

        std::shared_ptr<NavAid> nearset = nullptr;
        double distance = 0.0;
        for (auto const& navaid : navaids)
        {
            double d = GeoUtils::distance(navaid->getLocation(), near);
            if (nearset == nullptr || d < distance)
            {
                nearset = navaid;
                distance = d;
            }
        }
        return nearset;
    }
};

}

#endif //UNIVERSALFLIGHTCONNECTOR_NAVAID_H
