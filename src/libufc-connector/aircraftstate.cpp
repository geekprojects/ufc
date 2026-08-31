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

    fmsPrint(1, L"Universal");
    fmsPrint(2, L"Flight");
    fmsPrint(3, L"Connector");
}

std::shared_ptr<AircraftValue> AircraftState::getValue(const std::string& dataName)
{
    scoped_lock lock(m_mutex);
    auto it = m_valuesByName.find(dataName);
    if (it != m_valuesByName.end())
    {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<AircraftValue> AircraftState::getOrCreateValue(const std::string& dataName)
{
    scoped_lock lock(m_mutex);
    auto it = m_valuesByName.find(dataName);
    if (it != m_valuesByName.end())
    {
        return it->second;
    }

    auto value = std::make_shared<AircraftValue>();
    m_valuesByName.try_emplace(dataName, value);

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

bool AircraftState::isSet(const std::string &dataName)
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

wstring AircraftState::getString(const std::string &dataName)
{
    auto value = getValue(dataName);
    if (value == nullptr)
    {
        return L"";
    }
    return value->getString();
}

void AircraftState::fmsPrint(int row, std::wstring text, char fg, char bg)
{
    set("fmc/0/line" + to_string(row) + "/text", text);
    set("fmc/0/line" + to_string(row) + "/textColour", wstring(24, fg));
    set("fmc/0/line" + to_string(row) + "/backgroundColour", wstring(24, bg));
}

void AircraftState::dump()
{
    for (const auto&[name, value] : m_valuesByName)
    {
        log(DEBUG, "dump: %s -> %s", name.c_str(), value->getString().c_str());
    }
}
