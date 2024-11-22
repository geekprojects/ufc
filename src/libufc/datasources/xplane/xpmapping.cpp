//
// Created by Ian Parker on 11/11/2024.
//

#include "xpmapping.h"
#include "datadefs.h"

#include <filesystem>
#include <fnmatch.h>
#include <unistd.h>

using namespace std;
using namespace UFC;

XPMapping::XPMapping(string baseDir) : Logger("XPMapping"), m_baseDir(baseDir)
{
    for (const auto& dataRef : g_dataRefsInit)
    {
        addDataRef(dataRef);
    }
}

void XPMapping::loadDefinitionsForAircraft(const string& author, const string& icaoType)
{
    for (const auto & entry : filesystem::directory_iterator(m_baseDir + "/aircraft"))
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

void XPMapping::loadDefinitions(const string& file)
{
    loadDefinitions(YAML::LoadFile(m_baseDir + "/" + file));
}

void XPMapping::loadDefinitions(YAML::Node config)
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

void XPMapping::loadCommands(YAML::Node commandsNode, std::string id)
{
    log(DEBUG, "loadCommands: %s...", id.c_str());
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
            log(DEBUG, "loadCommands: command %s -> %s", command.c_str(), categoryName.c_str());
            CommandDefinition commandDefinition;
            commandDefinition.command = command;
            commandDefinition.id = id;
            m_commands.insert_or_assign(categoryName, commandDefinition);
        }
    }
}


DataMapping XPMapping::parseMapping(std::string mappingStr)
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

void XPMapping::addDataRef(const DataDefinition& dataRef)
{
    auto dataRefShared = make_shared<DataDefinition>(dataRef);
    m_dataRefs.push_back(dataRefShared);
    m_dataRefsById.insert(make_pair(dataRef.id, dataRefShared));
}

void XPMapping::dump()
{
    for (const auto& dataRef : m_dataRefs)
    {
        log(DEBUG, "DataRef: %s -> %s", dataRef->id.c_str(), dataRef->mapping.dataRef.c_str());
    }
    for (const auto& [id, command] : m_commands)
    {
        log(DEBUG, "Command: %s -> %s", id.c_str(), command.command.c_str());
    }
}

const CommandDefinition nullCommand = {};
const CommandDefinition& XPMapping::getCommand(std::string command)
{
    auto it = m_commands.find(command);

    if (it == m_commands.end())
    {
        log(WARN, "command: Unknown command: %s", command.c_str());
        return nullCommand;
    }

    return it->second;
}

void XPMapping::writeFloat(AircraftState& state, const shared_ptr<DataDefinition> &dataDef, float value)
{
    auto d = (float*)((char*)&state + dataDef->pos);
    *d = value;
}

void XPMapping::writeInt(AircraftState& state, const shared_ptr<DataDefinition> &dataDef, int32_t value)
{
    auto d = (int32_t*)((char*)&state + dataDef->pos);
    *d = value;
}

void XPMapping::writeBoolean(AircraftState& state, const shared_ptr<DataDefinition> &dataDef, bool value)
{
    auto d = (bool*)((char*)&state + dataDef->pos);

    if (dataDef->mapping.negate)
    {
        value = !value;
    }
    *d = value;
}
