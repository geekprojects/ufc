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

struct DataDefinition
{
    std::string id;
    DataRefType type;
    int pos;
    std::string dataRef;
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
    XPlaneClient m_client;

    int m_xPlaneVersion = 0;

    std::vector<std::shared_ptr<DataDefinition>> m_dataRefs;
    std::map<std::string, std::shared_ptr<DataDefinition>> m_dataRefsById;

    std::map<std::string, std::string> m_commandsById;

    void addDataRef(const DataDefinition& dataRef);

    void loadDefinitionsForAircraft(std::string author, std::string icaoType);
    void loadDefinitions(std::string file);
    void loadDefinitions(YAML::Node config);

    void update(const std::map<int, float>& values);

    public:
    explicit XPlaneDataSource();
    ~XPlaneDataSource() override = default;

    bool init() override;
    void close() override;

    bool update() override;

    void command(std::string command) override;

    int getXPlaneVersion() { return m_xPlaneVersion; }
};

}

#endif //XPFD_XPLANE_H
