//
// Created by Ian Parker on 28/08/2024.
//

#ifndef AIRPORTS_H
#define AIRPORTS_H

#include <string>

#include "geoutils.h"

namespace UFC
{

class Airport : public Locationable
{
 private:
    std::wstring m_name;
    Coordinate m_location;
    std::string m_icaoCode;
    bool m_hasRunway = false;

    float m_elevation;

 public:
    Airport() = default;
    Airport(std::wstring name, Coordinate location);
    ~Airport() = default;

    [[nodiscard]] std::wstring getName() const
    {
        return m_name;
    }

    void setName(const std::wstring& name)
    {
        m_name = name;
    }

    [[nodiscard]] Coordinate getLocation() const override
    {
        return m_location;
    }

    void setLocation(const Coordinate& location)
    {
        m_location = location;
    }

    [[nodiscard]] float getElevation() const
    {
        return m_elevation;
    }

    void setElevation(const float elevation)
    {
        m_elevation = elevation;
    }

    [[nodiscard]] std::string getICAOCode() const
    {
        return m_icaoCode;
    }

    void setICAOCode(const std::string& icaoCode)
    {
        m_icaoCode = icaoCode;
    }

    [[nodiscard]] bool isHasRunway() const
    {
        return m_hasRunway;
    }

    void setHasRunway(bool hasRunway)
    {
        m_hasRunway = hasRunway;
    }

    std::wstring toString() const override
    {
        return m_name + L" " +
            (wchar_t)m_icaoCode[0] +
            (wchar_t)m_icaoCode[1] +
            (wchar_t)m_icaoCode[2] +
            (wchar_t)m_icaoCode[3] +
            L" [" + m_location.toString() + L"]";
    }
};

class Airports
{
 private:
    std::shared_ptr<QuadTree> m_airports;
    std::vector<std::shared_ptr<Airport>> m_airportList;

 public:
    Airports();
    ~Airports();

    void addAirport(const std::shared_ptr<Airport>& shared);

    void dump()
    {
        printf("Airports: Loaded %lu airports\n", m_airportList.size());
        //m_airports->dump();
    }

    std::shared_ptr<Locationable> findNearest(Coordinate point)
    {
        return m_airports->findNearest(point, [](const std::shared_ptr<Locationable>& shared)
        {
            Airport* airport = dynamic_cast<Airport*>(shared.get());
            return airport->isHasRunway();
        });
    }
};

}

#endif //AIRPORTS_H
