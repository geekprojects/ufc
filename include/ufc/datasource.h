//
// Created by Ian Parker on 23/01/2024.
//

#ifndef UFC_DATASOURCE_H
#define UFC_DATASOURCE_H

#include <memory>
#include <map>

#include <ufc/aircraftstate.h>

#include "aircraftmapping.h"
#include "logger.h"
#include "lua.h"
#include "navdata.h"

namespace UFC
{
class Airports;
class FlightConnector;
class DataSource;

const std::string SOURCE_SIMULATOR = "Simulator";
const std::string SOURCE_XPLANE = "XPlane";

class DataSourceInit
{
 public:
    DataSourceInit() = default;
    virtual ~DataSourceInit() = default;

    virtual std::shared_ptr<DataSource> create(FlightConnector* app) { return nullptr; }

    virtual std::string getName() { return ""; }
};

class DataSourceRegistry
{
 private:
    std::map<std::string, DataSourceInit*, std::less<>> m_dataSources;

 public:
    static DataSourceRegistry* getDataSourceRegistry();
    static void registerDataSource(DataSourceInit* dataSource);

    DataSourceInit* find(const std::string& name)
    {
        auto it = m_dataSources.find(name);
        if (it != m_dataSources.end())
        {
            return it->second;
        }
        return nullptr;
    }

    const std::map<std::string, DataSourceInit*, std::less<>>& getDataSources() { return m_dataSources; }
};

#define UFC_DATA_SOURCE(_name, _class)  \
    class _name##Init : public UFC::DataSourceInit \
    { \
     private: \
        static _name##Init* const init; \
     public: \
        _name##Init() \
        { \
            UFC::DataSourceRegistry::registerDataSource(this); \
        } \
        ~_name##Init() override \
        { \
        } \
        std::string getName() override \
        { \
            return #_name; \
        } \
        std::shared_ptr<DataSource> create(UFC::FlightConnector* app) override \
        { \
            return std::shared_ptr<DataSource>(new _class(app)); \
        } \
    }; \
    _name##Init* const _name##Init::init = new _name##Init();

class DataSource : public Logger
{
 protected:
    FlightConnector* m_flightConnector;
    std::string m_name;
    int m_priority = 0;

    bool m_running = true;

    AircraftMapping m_mapping;

    UFCLua m_commandLua;
    UFCLua m_dataLua;

    float transformData(const std::shared_ptr<DataDefinition> &dataRef, float value);
    virtual void executeCommand(const std::string& command, const CommandDefinition& commandDefinition) {}

 public:
    explicit DataSource(FlightConnector* flightConnector, const std::string& name, const std::string& dataPath, int priority);

    ~DataSource() override = default;

    [[nodiscard]] std::string getName() const { return m_name; }
    [[nodiscard]] FlightConnector* getFlightConnector() const { return m_flightConnector; }

    virtual std::shared_ptr<Airports> loadAirports() { return nullptr; }
    virtual std::shared_ptr<NavAids> loadNavAids() { return nullptr; }

    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() { return false; }

    virtual bool update() = 0;

    void command(const std::string& command);
    virtual void setData(const std::string& dataName, float value) {}

    // Not all values may be updated in real time or you may not be running
    // the update thread. These can be used to retrieve values in these cases.
    virtual bool getDataInt(const std::string& dataName, int& value) { return false; }

    virtual bool getDataFloat(const std::string& dataName, float& value) { return false; };
    virtual bool getDataString(const std::string& dataName, std::string& value) { return false; };

    virtual void sendMessage(const std::string& message) {}

    UFCLua* getDataLua() { return &m_dataLua; }
    UFCLua* getCommandLua() { return &m_commandLua; }
};

}

#endif // UFC_DATASOURCE_H
