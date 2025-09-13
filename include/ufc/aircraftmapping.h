//
// Created by Ian Parker on 11/11/2024.
//

#ifndef MAPPING_H
#define MAPPING_H

#include "ufc/logger.h"

#include <string>
#include <vector>
#include <map>
#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "ufc/aircraftstate.h"

namespace UFC
{
class DataSource;
}


enum class DataMappingType
{
    VALUE,
    EQUALS,
    NEGATE
};

struct DataMapping
{
    DataMappingType type = DataMappingType::VALUE;
    int32_t operand = 0;
    std::string dataRef;
    std::string luaScript;
};

struct DataDefinition
{
    std::string id;
    UFC::DataRefType type;
    int len = 1;

    DataMapping mapping;
    std::shared_ptr<UFC::AircraftValue> value;
    int idx;
    void* data;
};

struct CommandDefinition
{
    std::string id;
    std::vector<std::string> commands;
    void* data = nullptr;
};

class AircraftMapping : UFC::Logger
{
    UFC::DataSource* m_dataSource;
    std::string m_baseDir;

    std::vector<std::shared_ptr<DataDefinition>> m_dataRefs;
    std::map<std::string, std::shared_ptr<DataDefinition>> m_dataRefsById;

    std::map<std::string, CommandDefinition> m_commands;

    void initDefinitions();

    bool checkAircraft(
        const std::string &author,
        const std::string &icaoType,
        const std::filesystem::directory_entry &entry,
        YAML::Node aircraftFile);

    void parseDataDefinition(const std::string& parent, YAML::Node definitionNode);
    void addDataDefinition(const std::string &id, YAML::Node definitionNode);

    void addDataRef(const DataDefinition& dataRef);

    static DataMapping parseMapping(std::string mapping);

    void loadDefinitions(YAML::Node config);
    void loadCommands(YAML::Node node, const std::string& id);

 public:
    AircraftMapping(UFC::DataSource* dataSource, const std::string &baseDir);

    void loadDefinitionsForAircraft(const std::string &author, const std::string &icaoType);

    const CommandDefinition& getCommand(const std::string &command);

    std::vector<std::shared_ptr<DataDefinition>>& getDataRefs() { return m_dataRefs; }
    std::map<std::string, CommandDefinition>& getCommands() { return m_commands; }
    std::shared_ptr<DataDefinition> getDataRef(const std::string &id);

    void writeFloat(const std::shared_ptr<DataDefinition> &dataDef, float value);
    void writeInt(const std::shared_ptr<DataDefinition> &dataDef, int32_t value);
    void writeBoolean(const std::shared_ptr<DataDefinition> &dataDef, int32_t value);
    void writeString(const std::shared_ptr<DataDefinition> &dataDef, const std::string& value);
};

#endif //MAPPING_H
