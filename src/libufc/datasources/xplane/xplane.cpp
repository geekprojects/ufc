//
// Created by Ian Parker on 23/01/2024.
//

#include "xplane.h"

#include <filesystem>
#include <fnmatch.h>

using namespace std;
using namespace UFC;

static const DataDefinition g_dataRefsInit[] =
{
    {"aircraft/indicatedAirspeed", FLOAT, offsetof(AircraftState, indicatedAirspeed)},
    {"aircraft/pitch", FLOAT, offsetof(AircraftState, pitch) },
    {"aircraft/roll", FLOAT, offsetof(AircraftState, roll)},
    {"aircraft/altitude", FLOAT, offsetof(AircraftState, altitude)},
    {"aircraft/verticalSpeed", FLOAT, offsetof(AircraftState, verticalSpeed)},
    {"aircraft/indicatedMach", FLOAT, offsetof(AircraftState, indicatedMach)},
    {"aircraft/magHeading", FLOAT, offsetof(AircraftState, magHeading)},
    {"aircraft/barometerHG", FLOAT, offsetof(AircraftState, barometerHG)},
    {"comms/com1Hz", FLOAT, offsetof(AircraftState, comms.com1Hz)},
    {"comms/com1StandbyHz", FLOAT, offsetof(AircraftState, comms.com1StandbyHz)},
    {"comms/com2Hz", FLOAT, offsetof(AircraftState, comms.com2Hz)},
    {"comms/com2StandbyHz", FLOAT, offsetof(AircraftState, comms.com2StandbyHz)},
    {"comms/nav1Hz", FLOAT, offsetof(AircraftState, comms.nav1Hz)},
    {"comms/nav1StandbyHz", FLOAT, offsetof(AircraftState, comms.nav1StandbyHz)},
    {"comms/nav2Hz", FLOAT, offsetof(AircraftState, comms.nav2Hz)},
    {"comms/nav2StandbyHz", FLOAT, offsetof(AircraftState, comms.nav2StandbyHz)},

    {"flightDirector/pitch", FLOAT, offsetof(AircraftState, flightDirector.pitch)},
    {"flightDirector/roll", FLOAT, offsetof(AircraftState, flightDirector.roll)},
    {"flightDirector/mode", INTEGER, offsetof(AircraftState, flightDirector.mode)},

    {"autopilot/speedMach", BOOLEAN, offsetof(AircraftState, autopilot.speedMach)},
    {"autopilot/speed", FLOAT, offsetof(AircraftState, autopilot.speed)},
    {"autopilot/speedWindowOpen", BOOLEAN, offsetof(AircraftState, autopilot.speedWindowOpen)},

    {"autopilot/headingWindowOpen", BOOLEAN, offsetof(AircraftState, autopilot.headingWindowOpen)},
    {"autopilot/gpssStatus", INTEGER, offsetof(AircraftState, autopilot.gpssStatus)},

    {"autopilot/altitude", FLOAT, offsetof(AircraftState, autopilot.altitude)},
    {"autopilot/fmsVnav", BOOLEAN, offsetof(AircraftState, autopilot.fmsVnav)},

    {"autopilot/heading", BOOLEAN, offsetof(AircraftState, autopilot.heading)},
    {"autopilot/verticalSpeed", BOOLEAN, offsetof(AircraftState, autopilot.verticalSpeed)},

    {"autopilot/locMode", INTEGER, offsetof(AircraftState, autopilot.locMode)},
    {"autopilot/ap1Mode", INTEGER, offsetof(AircraftState, autopilot.ap1Mode)},
    {"autopilot/ap2Mode", INTEGER, offsetof(AircraftState, autopilot.ap2Mode)},
    {"autopilot/autoThrottleMode", INTEGER, offsetof(AircraftState, autopilot.autoThrottleMode)},
    {"autopilot/approachMode", INTEGER, offsetof(AircraftState, autopilot.approachMode)},
};

XPlaneDataSource::XPlaneDataSource() : DataSource("XPlane")
{
    for (const auto& dataRef : g_dataRefsInit)
    {
        addDataRef(dataRef);
    }
}

bool XPlaneDataSource::init()
{
    bool res = m_client.connect();
    if (!res)
    {
        return false;
    }

    log(INFO, "init: Requesting details from X-Plane...");
    AircraftState state = getState();
    res = m_client.readInt("sim/version/xplane_internal_version", m_xPlaneVersion);

    if (!res || m_xPlaneVersion == 0)
    {
        log(ERROR, "init: Unable to connect to X-Plane");
        return false;
    }

    m_client.readString("sim/aircraft/view/acf_studio", 64, state.aircraftAuthor);
    if (state.aircraftAuthor.empty())
    {
        m_client.readString("sim/aircraft/view/acf_author", 64, state.aircraftAuthor);
    }
    m_client.readString("sim/aircraft/view/acf_ICAO", 10, state.aircraftICAO);

    log(INFO, "X-Plane version: %d", m_xPlaneVersion);
    log(INFO, "Author: %s", state.aircraftAuthor.c_str());
    log(INFO, "Aircraft ICAO type: %s", state.aircraftICAO.c_str());

    loadDefinitions("../data/x-plane/defaults.yaml");
    loadDefinitionsForAircraft(state.aircraftAuthor, state.aircraftICAO);

    for (auto dataRef : m_dataRefs)
    {
        log(DEBUG, "DataRef: %s -> %s", dataRef->id.c_str(), dataRef->dataRef.c_str());
    }
    for (auto commandIt : m_commandsById)
    {
        log(DEBUG, "Command: %s -> %s", commandIt.first.c_str(), commandIt.second.c_str());
    }

    return true;
}

void XPlaneDataSource::close()
{
    m_client.disconnect();
}

bool XPlaneDataSource::update()
{
    vector<pair<int, string>> datarefs;

    int idx = 1;
    for (auto dataRef : m_dataRefs)
    {
        if (!dataRef->dataRef.empty() && dataRef->dataRef != "null")
        {
            dataRef->idx = idx;
            datarefs.emplace_back(idx, dataRef->dataRef);
        }
        idx++;
    }

    return m_client.streamDataRefs(datarefs, [this](map<int, float> const& values)
    {
        update(values);
    });
}

void XPlaneDataSource::loadDefinitionsForAircraft(string author, string icaoType)
{
    for (const auto & entry : filesystem::directory_iterator("../data/x-plane/aircraft"))
    {
        if (entry.path().extension() == ".yaml")
        {
            auto aircraftFile = YAML::LoadFile(entry.path());
            if (!aircraftFile["author"] || !aircraftFile["icao"])
            {
                log(ERROR, "%s: Not a valid aircraft definition", entry.path().c_str());
                continue;
            }
            auto fileAuthor = aircraftFile["author"].as<string>();
            auto fileICAO = aircraftFile["icao"].as<string>();

            int match = fnmatch(fileAuthor.c_str(), author.c_str(), 0);
            if (match != 0)
            {
                continue;
            }
            match = fnmatch(icaoType.c_str(), fileICAO.c_str(), 0);
            if (match != 0)
            {
                continue;
            }
            log(INFO, "%s: Aircraft definition found!", entry.path().c_str());
            loadDefinitions(aircraftFile);
            break;
        }
    }
}

void XPlaneDataSource::loadDefinitions(string file)
{
    loadDefinitions(YAML::LoadFile(file));
}

void XPlaneDataSource::loadDefinitions(YAML::Node config)
{
    YAML::Node dataNode = config["data"];
    for (YAML::const_iterator it=dataNode.begin();it!=dataNode.end();++it)
    {
        auto categoryName = it->first.as<string>();
        for (YAML::const_iterator dataIt=it->second.begin();dataIt!=it->second.end();++dataIt)
        {
            if (dataIt->second)
            {
                auto name = dataIt->first.as<string>();
                auto dataRef = dataIt->second.as<string>();
                auto id = categoryName + "/" + name;
                auto dataRefIt = m_dataRefsById.find(id);
                if (dataRefIt != m_dataRefsById.end())
                {
                    dataRefIt->second->dataRef = dataRef;
                }
                else
                {
                    log(WARN, "Unknown data ref: %s", id.c_str());
                }
            }
        }
    }

    YAML::Node commandsNode = config["commands"];
    for (YAML::const_iterator it=commandsNode.begin();it!=commandsNode.end();++it)
    {
        auto categoryName = it->first.as<string>();
        for (YAML::const_iterator dataIt=it->second.begin();dataIt!=it->second.end();++dataIt)
        {
            auto name = dataIt->first.as<string>();
            auto command = dataIt->second.as<string>();
            auto id = categoryName + "/" + name;
            m_commandsById.insert_or_assign(id, command);
        }
    }
}

void XPlaneDataSource::update(const map<int, float>& values)
{
    AircraftState state = getState();

    state.connected = true;

    for (const auto& [idx, value] : values)
    {
        if (idx < 1)
        {
            continue;
        }

        const auto& dataRef = m_dataRefs[idx - 1];
        switch (dataRef->type)
        {
            case FLOAT:
            {
                float* d = (float*)((char*)&state + dataRef->pos);
                *d = value;
                break;
            }
            case INTEGER:
            {
                int32_t* i = (int32_t*)((char*)&state + dataRef->pos);
                *i = (int32_t)value;
                break;
            }
            case BOOLEAN:
            {
                bool* b = (bool*)((char*)&state + dataRef->pos);
                *b = (bool)value;
                break;
            }
        }
    }
    updateState(state);
}

void XPlaneDataSource::command(string command)
{
    auto it = m_commandsById.find(command);

    if (it == m_commandsById.end())
    {
        log(WARN, "command: Unknown command: %s", command.c_str());
        return;
    }

    m_client.sendCommand(it->second);
}

void XPlaneDataSource::addDataRef(const DataDefinition& dataRef)
{
    auto dataRefShared = make_shared<DataDefinition>(dataRef);
    m_dataRefs.push_back(dataRefShared);
    m_dataRefsById.insert(make_pair(dataRef.id, dataRefShared));
}
