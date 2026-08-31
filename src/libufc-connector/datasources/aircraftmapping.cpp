//
// Created by Ian Parker on 11/11/2024.
//

#include <ufc/aircraftmapping.h>
#include <ufc/datasource.h>
#include <ufc/utils/utils.h>

#include <filesystem>
#include <fnmatch.h>

#include "../lua.h"

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
}

void AircraftMapping::loadDefaults()
{
    log(DEBUG, "loadDefaults: initialising definitions...");
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
}

void AircraftMapping::loadDefinitionsForAircraft(
    const wstring& author,
    const wstring& icaoType)
{
    loadDefaults();

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
    const wstring &author,
    const wstring &icaoType,
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

    int match = fnmatch(fileAuthor.c_str(), wstring2utf8(author).c_str(), 0);
    if (match != 0)
    {
        return true;
    }

    bool foundICAO = false;
    for (const auto& fileICAO : fileICAOs)
    {
        log(DEBUG, "checkAircraft: %s == %s", fileICAO.c_str(), fileICAO.c_str());
        match = fnmatch(fileICAO.c_str(), wstring2utf8(icaoType).c_str(), 0);
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
        if (!initScript.empty() && m_dataSource != nullptr)
        {
            m_dataSource->getDataLua()->execute(initScript);
            m_dataSource->getCommandLua()->execute(initScript);
        }
    }

    YAML::Node dataNode = config["data"];
    parseDataDefinition("", dataNode);

    loadCommands(config["commands"], "");
}

void AircraftMapping::parseDataDefinition(const string& parent, YAML::Node node)
{
    for (YAML::const_iterator it = node.begin(); it!=node.end(); ++it)
    {
        auto nodeName = it->first.as<string>();
        string id;
        if (!parent.empty())
        {
            id = parent + "/";
        }
        id += nodeName;
        auto child = it->second;

        if (child.Type() == YAML::NodeType::Map)
        {
            if (child["dataRef"] || child["lua"])
            {
                addDataDefinition(id, child);
            }
            else
            {
                parseDataDefinition(id, child);
            }
        }
        else
        {
            addDataDefinition(id, child);
        }
    }

}

static void addDataRefIndex(const string& id, DataMapping& dataMapping)
{
    auto idx = id.find_last_of('[');
    if (idx != string::npos)
    {
        dataMapping.dataRef = id.substr(0, idx);
        auto arrayIdx = id.substr(idx + 1);
        dataMapping.dataRefIndex = atoi(arrayIdx.c_str());
    }
    else
    {
        dataMapping.dataRef = id;
        dataMapping.dataRefIndex = -1;
    }
}

void AircraftMapping::addDataDefinition(const string& id, YAML::Node definitionNode)
{
    shared_ptr<DataDefinition> dataRef;

    auto it = m_dataRefsById.find(id);
    if (it != m_dataRefsById.end())
    {
        dataRef = it->second;
    }
    else
    {
        dataRef = make_shared<DataDefinition>();
        dataRef->id = id;
        dataRef->idx = static_cast<int>(m_dataRefs.size()) + 1;
        m_dataRefsById.try_emplace(id, dataRef);
        m_dataRefs.push_back(dataRef);
    }

    if (definitionNode.Type() == YAML::NodeType::Map)
    {
        string dataRefNode = "";
        if (definitionNode["dataRef"])
        {
            dataRefNode = definitionNode["dataRef"].as<string>();
            addDataRefIndex(dataRefNode, dataRef->mapping);
        }
        auto lua = definitionNode["lua"].as<string>();
        dataRef->mapping.luaScript = lua;
        log(DEBUG, "addDataDefinition: %s -> %s (With lua script)", id.c_str(), dataRefNode.c_str());
    }
    else
    {
        auto value = definitionNode.as<string>();
        dataRef->mapping = parseMapping(utf82wstring(value.c_str()));
        log(DEBUG, "addDataDefinition: %s -> %s", id.c_str(), value.c_str());
    }
}

void AircraftMapping::loadCommands(YAML::Node commandsNode, const std::string& id)
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
                    auto command = commandIt.as<string>();
                    commandDefinition.commands.push_back(command);
                    log(DEBUG, "loadCommands: command %s -> %s", categoryName.c_str(), command.c_str());
                }
            }
            else
            {
                auto command = node.as<string>();
                log(DEBUG, "loadCommands: command %s -> %s", categoryName.c_str(), command.c_str());
                commandDefinition.commands.push_back(command);
            }
            m_commands.insert_or_assign(categoryName, commandDefinition);
        }
    }
}

bool is_number(const std::wstring& s)
{
    std::wstring::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

DataMapping AircraftMapping::parseMapping(std::wstring mappingStr)
{
    DataMapping mapping;

    vector<wstring> mappingParts = splitString(mappingStr, ' ');
    if (mappingParts.empty())
    {
        log(WARN, "parseMapping: Mapping string is empty");
        mapping.type = DataMappingType::STATIC;
        mapping.value.set(false);
        log(DEBUG, "parseMapping: Invalid value, defaulting to false");
        return mapping;
    }

    if (mappingParts.size() == 1)
    {
        if (mappingParts.at(0) == L"true" || mappingParts.at(0) == L"false")
        {
            mapping.type = DataMappingType::STATIC;
            mapping.value.set(mappingParts.at(0) == L"true");
            log(DEBUG, "parseMapping: STATIC: %d", mapping.value.getInt());
            return mapping;
        }

        if (is_number(mappingParts.at(0)))
        {
            mapping.type = DataMappingType::STATIC;
            mapping.value.set((int)wcstol(mappingParts.at(0).c_str(), nullptr, 10));
            log(DEBUG, "parseMapping: STATIC: %d", mapping.value.getInt());
            return mapping;
        }

        if (mappingParts.at(0).starts_with(L"static:"))
        {
            mapping.type = DataMappingType::STATIC;
            mapping.value.set(mappingParts.at(0).substr(7));
            log(DEBUG, "parseMapping: STATIC: %s", mapping.value.getString().c_str());
            return mapping;
        }
    }

    addDataRefIndex(wstring2utf8(mappingParts.at(0)), mapping);
    if (mapping.dataRef.at(0) == '!')
    {
        mapping.type = DataMappingType::NEGATE;
        addDataRefIndex(mapping.dataRef.substr(1), mapping);
        log(DEBUG, "parseMapping: NEGATE: %s", mapping.dataRef.c_str());
    }

    if (mappingParts.size() == 3)
    {
        wstring comparison = mappingParts.at(1);
        wstring operand = mappingParts.at(2);
        if (comparison == L"==")
        {
            mapping.type = DataMappingType::EQUALS;
            mapping.operand = wcstol(operand.c_str(), nullptr, 10);
            log(DEBUG, "parseMapping: EQUALS: '%s' -> %d", mapping.dataRef.c_str(), mapping.operand);
        }
        else if (comparison == L">")
        {
            mapping.type = DataMappingType::GREATER_THAN;
            mapping.operand = wcstol(operand.c_str(), nullptr, 10);
            log(DEBUG, "parseMapping: GREATER_THAN: '%s' -> %d", mapping.dataRef.c_str(), mapping.operand);
        }
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
const CommandDefinition& AircraftMapping::getCommand(const std::string &command)
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

void AircraftMapping::writeFloat(const shared_ptr<DataDefinition> &dataDef, float value)
{
    switch (dataDef->mapping.type)
    {
        case DataMappingType::VALUE:
            // Just use the value as-is
            break;

        case DataMappingType::EQUALS:
            value = value == static_cast<float>(dataDef->mapping.operand);
            break;

        case DataMappingType::GREATER_THAN:
            value = value > static_cast<float>(dataDef->mapping.operand);
            break;

        case DataMappingType::NEGATE:
            value = !static_cast<bool>(value);
            break;

        case DataMappingType::STATIC:
            value = dataDef->mapping.value.getFloat();
            break;
    }
    dataDef->value->set(value);
}

void AircraftMapping::writeInt(const shared_ptr<DataDefinition> &dataDef, int32_t value)
{
    switch (dataDef->mapping.type)
    {
        case DataMappingType::VALUE:
            // Just use the value as-is
            break;

        case DataMappingType::EQUALS:
            value = value == dataDef->mapping.operand;
            break;

        case DataMappingType::GREATER_THAN:
            value = value > dataDef->mapping.operand;
            break;

        case DataMappingType::NEGATE:
            value = !static_cast<bool>(value);
            break;

        case DataMappingType::STATIC:
            value = dataDef->mapping.value.getInt();
            break;
    }
    dataDef->value->set(value);
}

void AircraftMapping::writeBoolean(const shared_ptr<DataDefinition>& dataDef, int32_t value)
{
    writeInt(dataDef, value);
}

void AircraftMapping::writeString(
    [[maybe_unused]] const shared_ptr<DataDefinition>& dataDef,
    [[maybe_unused]] const wstring& value)
{
    if (dataDef->mapping.type == DataMappingType::STATIC)
    {
        dataDef->value->set(dataDef->mapping.value);
    }
    else
    {
        dataDef->value->set(value);
    }
}

void AircraftMapping::writeArray(
    [[maybe_unused]] const shared_ptr<DataDefinition>& dataDef,
    [[maybe_unused]] const vector<int>& value)
{
    if (dataDef->mapping.type == DataMappingType::STATIC)
    {
        dataDef->value->set(dataDef->mapping.value);
    }
    else
    {
        dataDef->value->set(value);
    }
}

void AircraftMapping::writeValue(const std::shared_ptr<DataDefinition> &dataDef, const UFC::AircraftValue &value)
{
    switch (value.getType())
    {
        case DataRefType::BOOLEAN:
            writeBoolean(dataDef, value.getInt());
            break;
        case DataRefType::INTEGER:
            writeInt(dataDef, value.getInt());
            break;
        case DataRefType::FLOAT:
            writeFloat(dataDef, value.getFloat());
            break;
        case DataRefType::STRING:
            writeString(dataDef, value.getString());
            break;
        case DataRefType::INT_ARRAY:
            writeArray(dataDef, value.getArray());
            break;
        default:
            break;
    }
}
