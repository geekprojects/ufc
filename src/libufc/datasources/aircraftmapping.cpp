//
// Created by Ian Parker on 11/11/2024.
//

#include <ufc/aircraftmapping.h>
#include <ufc/datasource.h>

#include <filesystem>
#include <fnmatch.h>

#include "../lua.h"
#include "ufc/utils.h"

#include "datadefs.h"

using namespace std;
using namespace UFC;

AircraftMapping::AircraftMapping(DataSource* dataSource, const string& baseDir) :
    Logger("AircraftMapping"),
    m_dataSource(dataSource),
    m_baseDir(baseDir)
{
}

void AircraftMapping::initDefinitions()
{
    m_dataRefs.clear();
    m_dataRefsById.clear();
    m_commands.clear();

    for (const auto& dataRef : g_dataRefsInit)
    {
        addDataRef(dataRef);
    }
}


void AircraftMapping::loadDefinitionsForAircraft(
    const string& author,
    const string& icaoType)
{
    // Load defaults
    log(DEBUG, "loadDefinitionsForAircraft: initialising definitions...");
    initDefinitions();

    string defaultsFile = m_baseDir + "/defaults.yaml";
    YAML::Node defaults;
    try
    {
        defaults = YAML::LoadFile(defaultsFile);
    }
    catch (std::exception& e)
    {
        log(ERROR, "loadDefinitionsForAircraft: Failed to load default definitions: %s", e.what());
        return;
    }
    loadDefinitions(defaults);

    // Try to find aircraft-specific definitions
    for (const auto & entry : filesystem::directory_iterator(m_baseDir + "/aircraft"))
    {
        if (entry.path().extension() == ".yaml")
        {
            auto aircraftFile = YAML::LoadFile(entry.path());
            if (checkAircraft(author, icaoType, entry, aircraftFile))
            {
                continue;
            }

            log(INFO, "%s: Aircraft definition found!", entry.path().c_str());
            loadDefinitions(aircraftFile);
            break;
        }
    }
}

bool AircraftMapping::checkAircraft(
    const string &author,
    const string &icaoType,
    const filesystem::directory_entry &entry,
    YAML::Node aircraftFile)
{
    if (!aircraftFile["author"] || !aircraftFile["icao"])
    {
        log(ERROR, "%s: Not a valid aircraft definition", entry.path().c_str());
        return true;
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
        return true;
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
        return true;
    }
    return false;
}


void AircraftMapping::loadDefinitions(YAML::Node config)
{
    if (config["init"])
    {
        auto initScript = config["init"].as<string>();
        if (initScript.starts_with("lua:"))
        {
            initScript = initScript.substr(4);
        }
        initScript = StringUtils::trim(initScript);
        if (!initScript.empty())
        {
            m_dataSource->getDataLua()->execute(initScript);
            m_dataSource->getCommandLua()->execute(initScript);
        }
    }

    YAML::Node dataNode = config["data"];
    for (YAML::const_iterator it=dataNode.begin(); it!=dataNode.end(); ++it)
    {
        auto categoryName = it->first.as<string>();
        for (auto dataIt = it->second.begin(); dataIt != it->second.end(); ++dataIt)
        {
            if (dataIt->second)
            {
                auto name = dataIt->first.as<string>();
                auto id = categoryName + "/" + name;
                auto definitionNode = dataIt->second;

                addDataDefinition(id, definitionNode);
            }
        }
    }

    loadCommands(config["commands"], "");
}

void AircraftMapping::addDataDefinition(string id, YAML::Node definitionNode)
{
    auto dataRefIt = m_dataRefsById.find(id);
    if (dataRefIt == m_dataRefsById.end())
    {
        log(WARN, "addDataDefinition: Unknown data ref: %s", id.c_str());
        return;
    }

    if (definitionNode.Type() == YAML::NodeType::Map)
    {
        auto dataRef = definitionNode["dataRef"].as<string>();
        auto lua = definitionNode["lua"].as<string>();
        dataRefIt->second->mapping.dataRef = dataRef;
        dataRefIt->second->mapping.luaScript = lua;
        log(DEBUG, "addDataDefinition: %s -> %s (With lua script)", id.c_str(), dataRef.c_str());
    }
    else
    {
        auto value = definitionNode.as<string>();
        dataRefIt->second->mapping = parseMapping(value);
        log(DEBUG, "addDataDefinition: %s -> %s", id.c_str(), value.c_str());
    }
}

void AircraftMapping::loadCommands(YAML::Node commandsNode, std::string id)
{
    for (YAML::const_iterator it=commandsNode.begin();it!=commandsNode.end();++it)
    {
        string categoryName;
        if (!id.empty())
        {
            categoryName = id + "/";
        }
        categoryName += it->first.as<string>();
        YAML::Node node = it->second;
        if (node.Type() == YAML::NodeType::Map)
        {
            loadCommands(node, categoryName);
        }
        else
        {
            CommandDefinition commandDefinition;
            commandDefinition.id = id;

            vector<string> commands;
            if (node.Type() == YAML::NodeType::Sequence)
            {
                for (auto commandIt : node)
                {
                    string command = commandIt.as<string>();
                    commandDefinition.commands.push_back(command);
                    log(DEBUG, "loadCommands: command %s -> %s", categoryName.c_str(), command.c_str());
                }
            }
            else
            {
                string command = node.as<string>();
                log(DEBUG, "loadCommands: command %s -> %s", categoryName.c_str(), command.c_str());
                commandDefinition.commands.push_back(command);
            }
            m_commands.insert_or_assign(categoryName, commandDefinition);
        }
    }
}

DataMapping AircraftMapping::parseMapping(std::string mappingStr)
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

void AircraftMapping::addDataRef(const DataDefinition& dataRef)
{
    auto dataRefShared = make_shared<DataDefinition>(dataRef);
    m_dataRefs.push_back(dataRefShared);
    m_dataRefsById.try_emplace(dataRef.id, dataRefShared);
}

const CommandDefinition nullCommand = {};
const CommandDefinition& AircraftMapping::getCommand(std::string command)
{
    auto it = m_commands.find(command);

    if (it == m_commands.end())
    {
        log(WARN, "command: Unknown command: %s", command.c_str());
        return nullCommand;
    }

    return it->second;
}

std::shared_ptr<DataDefinition> AircraftMapping::getDataRef(const std::string& id)
{
    auto it = m_dataRefsById.find(id);
    if (it != m_dataRefsById.end())
    {
        return it->second;
    }
    return nullptr;
}

void AircraftMapping::writeFloat(AircraftState& state, const shared_ptr<DataDefinition> &dataDef, float value)
{
    auto d = (float*)((char*)&state + dataDef->pos);
    *d = value;
}

void AircraftMapping::writeInt(AircraftState& state, const shared_ptr<DataDefinition> &dataDef, int32_t value)
{
    auto d = (int32_t*)((char*)&state + dataDef->pos);
    *d = value;
}

void AircraftMapping::writeBoolean(AircraftState& state, const shared_ptr<DataDefinition> &dataDef, int32_t value)
{
    auto d = (bool*)((char*)&state + dataDef->pos);

    switch (dataDef->mapping.type)
    {
        case DataMappingType::VALUE:
            // Just use the value as-is
            value = static_cast<bool>(value);
            break;

        case DataMappingType::EQUALS:
            value = value == dataDef->mapping.operand;
            break;

        case DataMappingType::NEGATE:
            value = !static_cast<bool>(value);
            break;
    }
    *d = value;
}

void AircraftMapping::writeString(AircraftState& state, const shared_ptr<DataDefinition> &dataDef, string value)
{
    auto d = (string*)((char*)&state + dataDef->pos);
    *d = value;
}
