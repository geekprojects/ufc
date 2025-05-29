//
// Created by Ian Parker on 14/10/2024.
//

#include <ufc/datasource.h>

#include "ufc/utils.h"

using namespace std;
using namespace UFC;

static DataSourceRegistry* g_dataSourceRegistry = nullptr;

DataSource::DataSource(FlightConnector* flightConnector, const std::string& name, const std::string& dataPath, int priority) :
    Logger("DataSource[" + name + "]"),
    m_flightConnector(flightConnector),
    m_name(name),
    m_priority(priority),
    m_mapping(this, dataPath),
    m_commandLua(flightConnector),
    m_dataLua(flightConnector)
{}

void DataSource::command(const std::string& commandName)
{
    auto commandDefinition = m_mapping.getCommand(commandName);
    if (commandDefinition.commands.empty())
    {
        return;
    }

    for (auto const& commandStr : commandDefinition.commands)
    {
#if 0
        log(DEBUG, "command: %s -> %s", commandName.c_str(), commandStr.c_str());
#endif

        if (commandStr.starts_with("lua:"))
        {
            string luaScript = commandStr.substr(4);
            m_commandLua.execute(luaScript);
            continue;
        }

        auto idx = commandStr.find('=');
        if (idx != string::npos)
        {
            string dataref = StringUtils::trim(commandStr.substr(0, idx));
            string valueStr = StringUtils::trim(commandStr.substr(idx + 1));
            auto value = (float)atof(valueStr.c_str());
#if 0
            log(INFO, "command: Setting data ref: %s = %0.2f", dataref.c_str(), value);
#endif
            setData(dataref, value);
            continue;
        }

        executeCommand(commandStr, commandDefinition);
    }
}

float DataSource::transformData(const shared_ptr<DataDefinition>& dataRef, float value)
{
    if (!dataRef->mapping.luaScript.empty())
    {
        value = m_dataLua.execute(dataRef->mapping.luaScript, "value", value);
    }
    return value;
}

DataSourceRegistry* DataSourceRegistry::getDataSourceRegistry()
{
    if (g_dataSourceRegistry == nullptr)
    {
        g_dataSourceRegistry = new DataSourceRegistry();
    }
    return g_dataSourceRegistry;
}

void DataSourceRegistry::registerDataSource(DataSourceInit* dataSource)
{
    getDataSourceRegistry()->m_dataSources.insert(make_pair(dataSource->getName(), dataSource));
}
