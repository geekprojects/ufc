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

#include "ufc/aircraftstate.h"

enum DataRefType
{
    FLOAT,
    BOOLEAN,
    INTEGER,
};

struct DataMapping
{
    std::string dataRef;
    bool negate = false;
};

struct DataDefinition
{
    std::string id;
    DataRefType type;
    int pos;
    DataMapping mapping;
    int idx;
    void* data;
};

struct CommandDefinition
{
    std::string id;
    std::string command;
    void* data = nullptr;
};

class XPMapping : UFC::Logger
{
 private:
    std::string m_baseDir;

    std::vector<std::shared_ptr<DataDefinition>> m_dataRefs;
    std::map<std::string, std::shared_ptr<DataDefinition>> m_dataRefsById;

    std::map<std::string, CommandDefinition> m_commands;

    void initDefinitions();
    void addDataRef(const DataDefinition& dataRef);
    DataMapping parseMapping(std::string mapping);

    void loadDefinitions(YAML::Node config);
    void loadCommands(YAML::Node node, std::string id);

 public:
    XPMapping(std::string baseDir);

    void loadDefinitionsForAircraft(const std::string &author, const std::string &icaoType);

    void dump();

    const CommandDefinition& getCommand(std::string command);

    std::vector<std::shared_ptr<DataDefinition>>& getDataRefs() { return m_dataRefs; }
    std::map<std::string, CommandDefinition>& getCommands() { return m_commands; }

    void writeFloat(UFC::AircraftState& state, const std::shared_ptr<DataDefinition> &dataDef, float value);
    void writeInt(UFC::AircraftState& state, const std::shared_ptr<DataDefinition> &dataDef, int32_t value);
    void writeBoolean(UFC::AircraftState& state, const std::shared_ptr<DataDefinition> &dataDef, bool value);
    void writeString(UFC::AircraftState& state, const std::shared_ptr<DataDefinition> &dataDef, std::string value);
};

#endif //MAPPING_H
