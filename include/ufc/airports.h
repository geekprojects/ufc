//
// Created by Ian Parker on 28/08/2024.
//

#ifndef UFC_AIRPORTS_H
#define UFC_AIRPORTS_H

#include <string>
#include <map>

#include "geoutils.h"
#include "navdata.h"

namespace UFC
{

struct Runway
{
    std::string m_number;
    Coordinate m_startLocation;
    float m_length;
    float m_bearing;
};

class Airport : public Locationable
{
 private:
    std::wstring m_name;
    Coordinate m_location;
    std::string m_icaoCode;

    std::vector<Runway> m_runways;

    bool m_hasRunway = false;

    float m_elevation = 0.0f;

 public:
    Airport() = default;
    Airport(std::wstring name, Coordinate location);
    ~Airport() override = default;

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

    [[nodiscard]] const std::vector<Runway>& getRunways() const
    {
        return m_runways;
    }

    void setRunways(const std::vector<Runway>& runways)
    {
        m_runways = runways;
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

class Airports : public NavData
{
    std::shared_ptr<QuadTree<Airport>> m_airports;
    std::vector<std::shared_ptr<Airport>> m_airportList;
    std::map<std::string, std::shared_ptr<Airport>, std::less<>> m_airportsByCode;

 protected:
    void addAirport(std::shared_ptr<Airport> airport);


 public:
    Airports(NavDataSource* navDataSource, std::string const& name);
    virtual ~Airports();

    virtual std::shared_ptr<Airport> findNearest(Coordinate point) const
    {
        return m_airports->findNearest(point, [](const std::shared_ptr<Airport>& airport)
        {
            return airport->isHasRunway();
        });
    }

    virtual std::shared_ptr<Airport> findByCode(const std::string& code);
};

}

#endif //AIRPORTS_H
