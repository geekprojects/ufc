//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_XPLANE_H
#define XPFD_XPLANE_H

#include <ufc/datasource.h>
#include "xplaneclient.h"

#include <yaml-cpp/yaml.h>

#include <map>

namespace UFC
{

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
};

struct CommandDefinition
{
    std::string id;
    std::string command;
};

class XPlaneDataSource : public DataSource
{
    private:
    std::shared_ptr<XPlaneClient> m_client;

    int m_xPlaneVersion = 0;

    std::vector<std::shared_ptr<DataDefinition>> m_dataRefs;
    std::map<std::string, std::shared_ptr<DataDefinition>> m_dataRefsById;

    std::map<std::string, std::string> m_commandsById;

    void addDataRef(const DataDefinition& dataRef);

    void loadDefinitionsForAircraft(const std::string &author, const std::string &icaoType);
    void loadDefinitions(const std::string &file);
    void loadDefinitions(YAML::Node config);
    void loadCommands(YAML::Node node, std::string id);

    DataMapping parseMapping(std::string mapping);

    void update(const std::map<int, float>& values);

    public:
    explicit XPlaneDataSource(FlightConnector* flightConnector);
    ~XPlaneDataSource() override = default;

    std::shared_ptr<Airports> loadAirports() override;

    bool connect() override;
    void disconnect() override;

    bool update() override;

    void command(std::string command) override;

    int getXPlaneVersion() { return m_xPlaneVersion; }
};

}

#endif //XPFD_XPLANE_H
