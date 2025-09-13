//
// Created by Ian Parker on 20/01/2024.
//

#ifndef UFC_STATE_H
#define UFC_STATE_H

#include <string>
#include <cstdint>
#include <map>
#include <vector>
#include <mutex>
#include <memory>

#include "logger.h"

#define DATA_AIRCRAFT_AUTHOR "aircraft/author"
#define DATA_AIRCRAFT_ICAO "aircraft/ico"
#define DATA_AIRCRAFT_INDICATEDAIRSPEED "aircraft/indicatedAirspeed"
#define DATA_AIRCRAFT_PITCH "aircraft/pitch"
#define DATA_AIRCRAFT_ROLL "aircraft/roll"
#define DATA_AIRCRAFT_ALTITUDE "aircraft/altitude"
#define DATA_AIRCRAFT_VERTICALSPEED "aircraft/verticalSpeed"
#define DATA_AIRCRAFT_INDICATEDMACH "aircraft/indicatedMach"
#define DATA_AIRCRAFT_MAGHEADING "aircraft/magHeading"
#define DATA_AIRCRAFT_BAROMETERHG "aircraft/barometerHG"
#define DATA_AIRCRAFT_PARKINGBRAKE "aircraft/parkingBrake"
#define DATA_COMMS_COM1HZ "comms/com1Hz"
#define DATA_COMMS_COM1STANDBYHZ "comms/com1StandbyHz"
#define DATA_COMMS_COM2HZ "comms/com2Hz"
#define DATA_COMMS_COM2STANDBYHZ "comms/com2StandbyHz"
#define DATA_COMMS_NAV1HZ "comms/nav1Hz"
#define DATA_COMMS_NAV1STANDBYHZ "comms/nav1StandbyHz"
#define DATA_COMMS_NAV2HZ "comms/nav2Hz"
#define DATA_COMMS_NAV2STANDBYHZ "comms/nav2StandbyHz"

#define DATA_FLIGHTDIRECTOR_PITCH "flightDirector/pitch"
#define DATA_FLIGHTDIRECTOR_ROLL "flightDirector/roll"
#define DATA_FLIGHTDIRECTOR_MODE "flightDirector/mode"

#define DATA_AUTOPILOT_DISPLAYSPEED "autopilot/displaySpeed"
#define DATA_AUTOPILOT_SPEEDMANAGED "autopilot/speedManaged"
#define DATA_AUTOPILOT_SPEEDMACH "autopilot/speedMach"
#define DATA_AUTOPILOT_SPEED "autopilot/speed"
#define DATA_AUTOPILOT_SPEEDWINDOWOPEN "autopilot/speedWindowOpen"

#define DATA_AUTOPILOT_DISPLAYHEADING "autopilot/displayHeading"
#define DATA_AUTOPILOT_HEADINGMANAGED "autopilot/headingManaged"
#define DATA_AUTOPILOT_HEADINGWINDOWOPEN "autopilot/headingWindowOpen"
#define DATA_AUTOPILOT_HEADING "autopilot/heading"
#define DATA_AUTOPILOT_HEADINGTRKMODE "autopilot/headingTrkMode"
#define DATA_AUTOPILOT_GPSSSTATUS "autopilot/gpssStatus"

#define DATA_AUTOPILOT_DISPLAYALTITUDE "autopilot/displayAltitude"
#define DATA_AUTOPILOT_ALTITUDEMANAGED "autopilot/altitudeManaged"
#define DATA_AUTOPILOT_ALTITUDESTEP1000 "autopilot/altitudeStep1000"
#define DATA_AUTOPILOT_ALTITUDE "autopilot/altitude"
#define DATA_AUTOPILOT_FMSVNAV "autopilot/fmsVnav"

#define DATA_AUTOPILOT_DISPLAYVERTICALSPEED "autopilot/displayVerticalSpeed"
#define DATA_AUTOPILOT_VERTICALSPEED "autopilot/verticalSpeed"
#define DATA_AUTOPILOT_VERTICALSPEEDFPAMODE "autopilot/verticalSpeedFPAMode"

#define DATA_AUTOPILOT_LOCMODE "autopilot/locMode"
#define DATA_AUTOPILOT_AP1MODE "autopilot/ap1Mode"
#define DATA_AUTOPILOT_AP2MODE "autopilot/ap2Mode"
#define DATA_AUTOPILOT_AUTOTHROTTLEMODE "autopilot/autoThrottleMode"
#define DATA_AUTOPILOT_APPROACHMODE "autopilot/approachMode"

#define DATA_APU_MASTER_ON "apu/master/on"
#define DATA_APU_STARTER_ON "apu/starter/on"

#define DATA_LIGHTS_LANDING_LEFT_ON "lights/landing/left/on"
#define DATA_LIGHTS_LANDING_RIGHT_ON "lights/landing/right/on"
#define DATA_LIGHTS_LANDING_LEFT_EXTENDED "lights/landing/left/extended"
#define DATA_LIGHTS_LANDING_RIGHT_EXTENDED "lights/landing/right/extended"

#define DATA_CABIN_CALL "cabin/call"
#define DATA_CABIN_SEATBELTSIGN "cabin/seatBeltSign"
#define DATA_CABIN_NOSMOKINGSIGN "cabin/noSmokingSign"
#define DATA_WEIGHT_PASSENGERCOUNT "weight/passengerCount"
#define DATA_WEIGHT_PASSENGERDISTRIBUTION "weight/passengerDistribution"

#define DATA_FLIGHTPLAN_FLIGHTNUMBER "flightplan/flightNumber"
#define DATA_FLIGHTPLAN_CRUISEALTITUDE "flightplan/cruiseAltitude"
#define DATA_FLIGHTPLAN_DEPARTUREAIRPORT "flightplan/departureAirport"
#define DATA_FLIGHTPLAN_DESTINATIONAIRPORT "flightplan/destinationAirport"
#define DATA_FLIGHTPLAN_DISTANCETODEST "flightplan/distanceToDestination"

namespace UFC
{

enum class DataRefType
{
    UNKNOWN,
    FLOAT,
    BOOLEAN,
    INTEGER,
    STRING,
};

class AircraftValue
{
    int m_index = -1;
    DataRefType m_type = DataRefType::UNKNOWN;
    union
    {
        int i;
        float f;
    } m_value;
    std::string m_string;

 public:
    explicit AircraftValue(int idx)
    {
        m_value.i = 0;
        m_index = idx;
    }

    [[nodiscard]] int getIndex() const
    {
        return m_index;
    }

    void set(bool b)
    {
        m_type = DataRefType::BOOLEAN;
        m_value.i = b;
    }

    void set(int i)
    {
        m_type = DataRefType::INTEGER;
        m_value.i = i;
    }

    void set(float f)
    {
        m_type = DataRefType::FLOAT;
        m_value.f = f;
    }

    void set(std::string const& str)
    {
        m_type = DataRefType::STRING;
        m_string = str;
    }

    [[nodiscard]] int getInt() const
    {
        switch (m_type)
        {
            case DataRefType::BOOLEAN:
            case DataRefType::INTEGER:
                return m_value.i;
            case DataRefType::FLOAT:
                return (int)m_value.f;
            default:
                return 0;
        }
    }

    [[nodiscard]] float getFloat() const
    {
        switch (m_type)
        {
            case DataRefType::BOOLEAN:
            case DataRefType::INTEGER:
                return (float)m_value.i;
            case DataRefType::FLOAT:
                return m_value.f;
            default:
                return 0.0;
        }
    }

    [[nodiscard]] std::string getString() const
    {
        switch (m_type)
        {
            case DataRefType::BOOLEAN:
            case DataRefType::INTEGER:
                return std::to_string(m_value.i);
            case DataRefType::FLOAT:
                return std::to_string(m_value.f);
            case DataRefType::STRING:
                return m_string;
            default:
                return "";
        }
    }
};

class AircraftState : public Logger
{
    int m_nextIndex = 1;
    std::mutex m_mutex;

    std::map<std::string, std::shared_ptr<AircraftValue>, std::less<>> valuesByName;
    std::map<int, std::shared_ptr<AircraftValue>> valuesByIndex;

 public:
    AircraftState() : Logger("AircraftState") {}

    int getIndex(const std::string& name);
    std::shared_ptr<AircraftValue> getOrCreateValue(const std::string &dataName);
    std::shared_ptr<AircraftValue> getValue(const std::string &dataName);

    void init();

    bool isSet(const std::string &dataName)
    {
        return getValue(dataName) != nullptr;
    }

    void set(int idx, bool b)
    {
        valuesByIndex[idx]->set(b);
    }

    void set(int idx, int i)
    {
        valuesByIndex[idx]->set(i);
    }

    void set(int idx, float f)
    {
        valuesByIndex[idx]->set(f);
    }

    void set(int idx, const std::string& str)
    {
        valuesByIndex[idx]->set(str);
    }

    void set(std::string const& name, bool b)
    {
        getOrCreateValue(name)->set(b);
    }

    void set(std::string const& name, int i)
    {
        getOrCreateValue(name)->set(i);
    }

    void set(std::string const& name, float f)
    {
        getOrCreateValue(name)->set(f);
    }

    void set(std::string const& name, const std::string& str)
    {
        getOrCreateValue(name)->set(str);
    }

    float getFloat(const std::string& dataName);
    int getInt(const std::string& dataName);
    std::string getString(const std::string& dataName);

    void dump();
};

}

#endif //XPFD_STATE_H
