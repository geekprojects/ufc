//
// Created by Ian Parker on 23/01/2024.
//

#include "xplane.h"
#include <ufc/flightconnector.h>

#include <filesystem>
#include <fnmatch.h>

using namespace std;
using namespace UFC;

#include "datadefs.h"

XPlaneDataSource::XPlaneDataSource(FlightConnector* flightConnector) : DataSource(flightConnector, "XPlane")
{
    for (const auto& dataRef : g_dataRefsInit)
    {
        addDataRef(dataRef);
    }

    m_client = make_shared<XPlaneClient>(
        flightConnector->getConfig().xplaneHost,
        flightConnector->getConfig().xplanePort);
}

bool XPlaneDataSource::connect()
{
    bool res = m_client->connect();
    if (!res)
    {
        return false;
    }

    log(INFO, "init: Requesting details from X-Plane...");
    AircraftState state = m_flightConnector->getState();
    res = m_client->readInt("sim/version/xplane_internal_version", m_xPlaneVersion);

    if (!res || m_xPlaneVersion == 0)
    {
        log(ERROR, "init: Unable to connect to X-Plane");
        return false;
    }

    m_client->readString("sim/aircraft/view/acf_studio", 64, state.aircraftAuthor);
    if (state.aircraftAuthor.empty())
    {
        m_client->readString("sim/aircraft/view/acf_author", 64, state.aircraftAuthor);
    }
    m_client->readString("sim/aircraft/view/acf_ICAO", 10, state.aircraftICAO);

    log(INFO, "X-Plane version: %d", m_xPlaneVersion);
    log(INFO, "Author: %s", state.aircraftAuthor.c_str());
    log(INFO, "Aircraft ICAO type: %s", state.aircraftICAO.c_str());

    loadDefinitions("../data/x-plane/defaults.yaml");
    loadDefinitionsForAircraft(state.aircraftAuthor, state.aircraftICAO);

    for (const auto& dataRef : m_dataRefs)
    {
        log(DEBUG, "DataRef: %s -> %s", dataRef->id.c_str(), dataRef->mapping.dataRef.c_str());
    }
    for (const auto& [id, command] : m_commandsById)
    {
        log(DEBUG, "Command: %s -> %s", id.c_str(), command.c_str());
    }

    return true;
}

void XPlaneDataSource::disconnect()
{
    m_client->disconnect();
}

bool XPlaneDataSource::update()
{
    vector<pair<int, string>> datarefs;

    int idx = 1;
    for (const auto& dataRef : m_dataRefs)
    {
        if (!dataRef->mapping.dataRef.empty() && dataRef->mapping.dataRef != "null")
        {
            dataRef->idx = idx;
            datarefs.emplace_back(idx, dataRef->mapping.dataRef);
        }
        idx++;
    }

    return m_client->streamDataRefs(datarefs, [this](map<int, float> const& values)
    {
        update(values);
    });
}

void XPlaneDataSource::loadDefinitionsForAircraft(const string& author, const string& icaoType)
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

void XPlaneDataSource::loadDefinitions(const string&file)
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
                auto value = dataIt->second.as<string>();
                auto id = categoryName + "/" + name;
                auto dataRefIt = m_dataRefsById.find(id);
                if (dataRefIt != m_dataRefsById.end())
                {
                    dataRefIt->second->mapping = parseMapping(value);
                }
                else
                {
                    log(WARN, "Unknown data ref: %s", id.c_str());
                }
            }
        }
    }

    loadCommands(config["commands"], "");
}

void XPlaneDataSource::loadCommands(YAML::Node commandsNode, std::string id)
{
    printf("loadCommands: %s...\n", id.c_str());
    for (YAML::const_iterator it=commandsNode.begin();it!=commandsNode.end();++it)
    {
        string categoryName;
        if (!id.empty())
        {
            categoryName = id + "/";
        }
        categoryName += it->first.as<string>();
        if (it->second.Type() == YAML::NodeType::Map)
        {
            loadCommands(it->second, categoryName);
        }
        else
        {
            auto command = it->second.as<string>();
            printf("XPLANE: Loading command %s -> %s\n", command.c_str(), categoryName.c_str());
            m_commandsById.insert_or_assign(categoryName, command);
        }
    }
}

DataMapping XPlaneDataSource::parseMapping(std::string mappingStr)
{
    DataMapping mapping;
    if (mappingStr.at(0) == '!')
    {
        mapping.negate = true;
        mappingStr = mappingStr.substr(1);
    }
    mapping.dataRef = mappingStr;
    return mapping;
}

void XPlaneDataSource::update(const map<int, float>& values)
{
    AircraftState state = m_flightConnector->getState();

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
                auto d = (float*)((char*)&state + dataRef->pos);
                *d = value;
                break;
            }
            case INTEGER:
            {
                auto i = (int32_t*)((char*)&state + dataRef->pos);
                *i = (int32_t)value;
                break;
            }
            case BOOLEAN:
            {
                auto b = (bool*)((char*)&state + dataRef->pos);

                bool boolValue = value;
                if (dataRef->mapping.negate)
                {
                    boolValue = !boolValue;
                }
                *b = boolValue;
                break;
            }
        }
    }
    m_flightConnector->updateState(state);
}

void XPlaneDataSource::command(string command)
{
    auto it = m_commandsById.find(command);

    if (it == m_commandsById.end())
    {
        log(WARN, "command: Unknown command: %s", command.c_str());
        return;
    }
    string xcommand = it->second;
    printf("XPlaneDataSource::command: %s -> %s\n", command.c_str(), xcommand.c_str());

    int idx = xcommand.find("=");
    if (idx == xcommand.npos)
    {
        m_client->sendCommand(xcommand);
    }
    else
    {
        string dataref = xcommand.substr(0, idx);
        float value = atof(xcommand.substr(idx + 1).c_str());
        log(INFO, "command: Setting data ref: %s = %0.2f", dataref.c_str(), value);
        m_client->setDataRef(dataref, value);
    }
}

void XPlaneDataSource::addDataRef(const DataDefinition& dataRef)
{
    auto dataRefShared = make_shared<DataDefinition>(dataRef);
    m_dataRefs.push_back(dataRefShared);
    m_dataRefsById.insert(make_pair(dataRef.id, dataRefShared));
}
