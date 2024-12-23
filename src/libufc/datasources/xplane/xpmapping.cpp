//
// Created by Ian Parker on 11/11/2024.
//

#include "xpmapping.h"
#include "datadefs.h"

#include <filesystem>
#include <fnmatch.h>
#include <unistd.h>

#include "ufc/utils.h"

using namespace std;
using namespace UFC;

XPMapping::XPMapping(const string& baseDir) : Logger("XPMapping"), m_baseDir(baseDir)
{
}

void XPMapping::initDefinitions()
{
    m_dataRefs.clear();
    m_dataRefsById.clear();
    m_commands.clear();

    for (const auto& dataRef : g_dataRefsInit)
    {
        addDataRef(dataRef);
    }
}

void XPMapping::loadDefinitionsForAircraft(const string& author, const string& icaoType)
{
    // Load defaults
    initDefinitions();
    loadDefinitions(YAML::LoadFile(m_baseDir + "/defaults.yaml"));

    // Try to find aircraft-specific definitions
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
            vector<string> fileICAOs;
            auto icaoNode = aircraftFile["icao"];
            if (icaoNode.IsScalar())
            {
                auto fileICAO = aircraftFile["icao"].as<string>();
                fileICAOs.push_back(fileICAO);
            }
            else
            {
                for (auto fileICAO : icaoNode)
                {
                    fileICAOs.push_back(fileICAO.as<string>());
                }
            }

            int match = fnmatch(fileAuthor.c_str(), author.c_str(), 0);
            if (match != 0)
            {
                continue;
            }

            bool foundICAO = false;
            for (const auto& fileICAO : fileICAOs)
            {
                match = fnmatch(icaoType.c_str(), fileICAO.c_str(), 0);
                if (match == 0)
                {
                    foundICAO = true;
                    break;
                }
            }

            if (!foundICAO)
            {
                continue;
            }

            log(INFO, "%s: Aircraft definition found!", entry.path().c_str());
            loadDefinitions(aircraftFile);
            break;
        }
    }
}

void XPMapping::loadDefinitions(YAML::Node config)
{
    YAML::Node dataNode = config["data"];
    for (YAML::const_iterator it=dataNode.begin(); it!=dataNode.end(); ++it)
    {
        auto categoryName = it->first.as<string>();
        for (auto dataIt = it->second.begin(); dataIt != it->second.end(); ++dataIt)
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
                    log(DEBUG, "loadDefinitions: %s -> %s", id.c_str(), value.c_str());
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
            log(DEBUG, "loadCommands: command %s -> %s", categoryName.c_str(), command.c_str());
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
    auto idx = mappingStr.find("==");

    if (idx != string::npos)
    {
        mapping.type = DataMappingType::EQUALS;
        mapping.dataRef = StringUtils::trim(mappingStr.substr(0, idx));
        mapping.operand = atoi(StringUtils::trim(mappingStr.substr(idx + 2)).c_str());
        printf("parseMapping: EQUALS: '%s' -> %d\n", mapping.dataRef.c_str(), mapping.operand);
    }
    else if (mappingStr.at(0) == '!')
    {
        mapping.type = DataMappingType::NEGATE;
        mapping.dataRef = mappingStr.substr(1);
    }
    else
    {
        mapping.dataRef = mappingStr;
    }
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
        log(DEBUG, "dump: DataRef: %s -> %s", dataRef->id.c_str(), dataRef->mapping.dataRef.c_str());
    }
    for (const auto& [id, command] : m_commands)
    {
        log(DEBUG, "dump: Command: %s -> %s", id.c_str(), command.command.c_str());
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

std::shared_ptr<DataDefinition> XPMapping::getDataRef(const std::string& id)
{
    auto it = m_dataRefsById.find(id);
    if (it != m_dataRefsById.end())
    {
        return it->second;
    }
    return nullptr;
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

void XPMapping::writeBoolean(AircraftState& state, const shared_ptr<DataDefinition> &dataDef, int32_t value)
{
    auto d = (bool*)((char*)&state + dataDef->pos);

    switch (dataDef->mapping.type)
    {
        case DataMappingType::VALUE:
            // Just use the value as-is
            value = static_cast<bool>(value);
            break;

        case DataMappingType::EQUALS:
            //printf("writeBoolean: %d -> %d\n", value, dataDef->mapping.operand);
            value = value == dataDef->mapping.operand;
            break;

        case DataMappingType::NEGATE:
            value = !static_cast<bool>(value);
            break;
    }
    *d = value;
}

void XPMapping::writeString(AircraftState& state, const shared_ptr<DataDefinition> &dataDef, string value)
{
    auto d = (string*)((char*)&state + dataDef->pos);
    *d = value;
}
