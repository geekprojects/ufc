//
// Created by Ian Parker on 23/01/2024.
//

#ifndef UFC_DATASOURCE_H
#define UFC_DATASOURCE_H

#include <memory>
#include <map>

#include <ufc/aircraftstate.h>
#include <ufc/commands.h>

#include "logger.h"
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
    std::map<std::string, DataSourceInit*> m_dataSources;

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

    const std::map<std::string, DataSourceInit*>& getDataSources() { return m_dataSources; }
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

 public:
    explicit DataSource(FlightConnector* flightConnector, const std::string& name, int priority) :
        Logger("DataSource[" + name + "]"),
        m_flightConnector(flightConnector),
        m_name(name),
        m_priority(priority)
    {}
    ~DataSource() override = default;

    [[nodiscard]] std::string getName() const { return m_name; }

    virtual std::shared_ptr<Airports> loadAirports() { return nullptr; }
    virtual std::shared_ptr<NavData> loadNavData() { return nullptr; }

    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() { return false; }

    virtual bool update() = 0;

    virtual void command(std::string command) {}
    virtual void setData(std::string dataName, float value) {}

    // Not all values may be updated in real time or you may not be running
    // the update thread. These can be used to retrieve values in these cases.
    // Note, some Data Sources (X-Plane) do not allow you to run the update
    // thread and use these calls!
    virtual bool getDataInt(std::string dataName, int& value) { return false; };
    virtual bool getDataFloat(std::string dataName, float& value) { return false; };
    virtual bool getDataString(std::string dataName, std::string& value) { return false; };

    virtual void sendMessage(std::string message) {}
};

}

#endif //XPFD_DATASOURCE_H
