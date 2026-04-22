//
// Created by Ian Parker on 22/07/2025.
//

#include <ufc/aircraftstate.h>
#include <ufc/aircraftdata.h>

using namespace std;
using namespace UFC;

void AircraftState::init()
{
    set(DATA_AIRCRAFT_BAROMETER_PILOT_IN_HG, 29.92f);
}

int AircraftState::getIndex(const std::string &name)
{
    auto value = getValue(name);
    if (value == nullptr)
    {
        return -1;
    }
    return value->getIndex();
}

std::shared_ptr<AircraftValue> AircraftState::getValue(const std::string& dataName)
{
    scoped_lock lock(m_mutex);
    auto it = valuesByName.find(dataName);
    if (it != valuesByName.end())
    {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<AircraftValue> AircraftState::getOrCreateValue(const std::string& dataName)
{
    scoped_lock lock(m_mutex);
    auto it = valuesByName.find(dataName);
    if (it != valuesByName.end())
    {
        return it->second;
    }

    int index = m_nextIndex;
    m_nextIndex++;
    auto value = std::make_shared<AircraftValue>(index);
    valuesByName.try_emplace(dataName, value);
    valuesByIndex.try_emplace(index, value);

    return value;
}

int AircraftState::getInt(const std::string& dataName)
{
    auto value = getValue(dataName);
    if (value == nullptr)
    {
        return 0;
    }
    return value->getInt();
}

bool AircraftState::hasValue(const std::string &dataName)
{
    auto value = getValue(dataName);
    if (value == nullptr)
    {
        return false;
    }
    return value->hasValue();
}

float AircraftState::getFloat(const std::string &dataName)
{
    auto value = getValue(dataName);
    if (value == nullptr)
    {
        return 0.0;
    }
    return value->getFloat();
}

std::string AircraftState::getString(const std::string &dataName)
{
    auto value = getValue(dataName);
    if (value == nullptr)
    {
        return "";
    }
    return value->getString();
}

void AircraftState::dump()
{
    for (const auto&[name, value] : valuesByName)
    {
        log(DEBUG, "dump: %d: %s -> %s", value->getIndex(), name.c_str(), value->getString().c_str());
    }
}
