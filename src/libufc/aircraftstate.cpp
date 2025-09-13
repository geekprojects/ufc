//
// Created by Ian Parker on 22/07/2025.
//

#include <ufc/aircraftstate.h>

using namespace std;
using namespace UFC;

void AircraftState::init()
{
    set(DATA_AIRCRAFT_INDICATEDAIRSPEED, 0.0f);
    set(DATA_AIRCRAFT_PITCH, 0.0f);
    set(DATA_AIRCRAFT_ROLL, 0.0f);
    set(DATA_AIRCRAFT_ALTITUDE, 0.0f);
    set(DATA_AIRCRAFT_VERTICALSPEED, 0.0f);
    set(DATA_AIRCRAFT_INDICATEDMACH, 0.0f);
    set(DATA_AIRCRAFT_MAGHEADING, 0.0f);
    set(DATA_AIRCRAFT_BAROMETERHG, 0.0f);
    set(DATA_AIRCRAFT_PARKINGBRAKE, 0.0f);
    set(DATA_COMMS_COM1HZ, 0);
    set(DATA_COMMS_COM1STANDBYHZ, 0);
    set(DATA_COMMS_COM2HZ, 0);
    set(DATA_COMMS_COM2STANDBYHZ, 0);
    set(DATA_COMMS_NAV1HZ, 0);
    set(DATA_COMMS_NAV1STANDBYHZ, 0);
    set(DATA_COMMS_NAV2HZ, 0);
    set(DATA_COMMS_NAV2STANDBYHZ, 0);

    set(DATA_FLIGHTDIRECTOR_PITCH, 0.0f);
    set(DATA_FLIGHTDIRECTOR_ROLL, 0.0f);
    set(DATA_FLIGHTDIRECTOR_MODE, false);

    set(DATA_AUTOPILOT_DISPLAYSPEED, true);
    set(DATA_AUTOPILOT_SPEEDMANAGED, false);
    set(DATA_AUTOPILOT_SPEEDMACH, false);
    set(DATA_AUTOPILOT_SPEED, 0.0f);
    set(DATA_AUTOPILOT_SPEEDWINDOWOPEN, false);

    set(DATA_AUTOPILOT_DISPLAYHEADING, true);
    set(DATA_AUTOPILOT_HEADINGMANAGED, false);
    set(DATA_AUTOPILOT_HEADINGWINDOWOPEN, false);
    set(DATA_AUTOPILOT_HEADING, 0.0f);
    set(DATA_AUTOPILOT_HEADINGTRKMODE, 0);
    set(DATA_AUTOPILOT_GPSSSTATUS, 0);

    set(DATA_AUTOPILOT_DISPLAYALTITUDE, true);
    set(DATA_AUTOPILOT_ALTITUDEMANAGED, false);
    set(DATA_AUTOPILOT_ALTITUDESTEP1000, false);
    set(DATA_AUTOPILOT_ALTITUDE, 0.0f);
    set(DATA_AUTOPILOT_FMSVNAV, false);

    set(DATA_AUTOPILOT_DISPLAYVERTICALSPEED, true);
    set(DATA_AUTOPILOT_VERTICALSPEED, 0.0f);
    set(DATA_AUTOPILOT_VERTICALSPEEDFPAMODE, 0);

    set(DATA_AUTOPILOT_LOCMODE, false);
    set(DATA_AUTOPILOT_AP1MODE, false);
    set(DATA_AUTOPILOT_AP2MODE, false);
    set(DATA_AUTOPILOT_AUTOTHROTTLEMODE, false);
    set(DATA_AUTOPILOT_APPROACHMODE, false);

    set(DATA_APU_MASTER_ON, false);
    set(DATA_APU_STARTER_ON, false);

    set(DATA_LIGHTS_LANDING_LEFT_ON, false);
    set(DATA_LIGHTS_LANDING_RIGHT_ON, false);
    set(DATA_LIGHTS_LANDING_LEFT_EXTENDED, false);
    set(DATA_LIGHTS_LANDING_RIGHT_EXTENDED, false);

    set(DATA_CABIN_CALL, false);
    set(DATA_CABIN_SEATBELTSIGN, false);
    set(DATA_CABIN_NOSMOKINGSIGN, false);
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
