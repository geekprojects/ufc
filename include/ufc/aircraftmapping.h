//
// Created by Ian Parker on 11/11/2024.
//

#ifndef MAPPING_H
#define MAPPING_H

#include "ufc/logger.h"

#include <string>
#include <vector>
#include <map>

#include <yaml-cpp/yaml.h>
#include <__filesystem/directory_entry.h>

#include "ufc/aircraftstate.h"

namespace UFC
{
class DataSource;
}

enum DataRefType
{
    FLOAT,
    BOOLEAN,
    INTEGER,
    STRING,
};

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
    DataRefType type;
    int pos;
    int len = 1;

    DataMapping mapping;
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
 private:
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

    void addDataDefinition(std::string id, YAML::Node definitionNode);

    void addDataRef(const DataDefinition& dataRef);

    static DataMapping parseMapping(std::string mapping);

    void loadDefinitions(YAML::Node config);
    void loadCommands(YAML::Node node, std::string id);

 public:
    AircraftMapping(UFC::DataSource* dataSource, const std::string &baseDir);

    void loadDefinitionsForAircraft(const std::string &author, const std::string &icaoType);

    const CommandDefinition& getCommand(std::string command);

    std::vector<std::shared_ptr<DataDefinition>>& getDataRefs() { return m_dataRefs; }
    std::map<std::string, CommandDefinition>& getCommands() { return m_commands; }
    std::shared_ptr<DataDefinition> getDataRef(const std::string &id);

    void writeFloat(UFC::AircraftState& state, const std::shared_ptr<DataDefinition> &dataDef, float value);
    void writeInt(UFC::AircraftState& state, const std::shared_ptr<DataDefinition> &dataDef, int32_t value);
    void writeBoolean(UFC::AircraftState &state, const std::shared_ptr<DataDefinition> &dataDef, int32_t value);
    void writeString(UFC::AircraftState& state, const std::shared_ptr<DataDefinition> &dataDef, std::string value);
};

#endif //MAPPING_H
