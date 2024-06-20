//
// Created by Ian Parker on 27/05/2024.
//

#ifndef FLIGHTCONNECTOR_H
#define FLIGHTCONNECTOR_H

#include <thread>
#include <mutex>
#include <vector>

#include <ufc/datasource.h>

#include "logger.h"

namespace UFC
{

class DataSource;
class Device;

struct Config
{
    std::string configPath;
    std::string dataDir;

    std::string dataSource;

    // X-Plane specific
    std::string xplaneHost;
    int xplanePort = 0;


    // Arduino specific
    std::string arduinoDevice;

    void dump() const
    {
        printf("Config::dump: configPath=%s\n", configPath.c_str());
        printf("Config::dump: dataPath=%s\n", dataDir.c_str());
        printf("Config::dump: dataSource=%s\n", dataSource.c_str());
        printf("Config::dump: X-Plane host=%s\n", xplaneHost.c_str());
        printf("Config::dump: X-Plane port=%d\n", xplanePort);
        printf("Config::dump: Arduino device=%s\n", arduinoDevice.c_str());
    }
};

class FlightConnector final : public Logger
{
 private:
    Config m_config;
    std::shared_ptr<DataSource> m_dataSource;
    std::vector<Device*> m_devices;

    std::mutex m_stateMutex;
    AircraftState m_state;

    bool m_running = false;
    std::shared_ptr<std::thread> m_updateDeviceThread;
    std::shared_ptr<std::thread> m_updateDataSourceThread;

    static void updateDeviceThread(FlightConnector* flightConnector);
    void updateDeviceMain();

    static void updateDataSourceThread(FlightConnector* flightConnector);
    void updateDataSourceMain();

    void loadConfig(const Config &config);

 public:
    FlightConnector();
    explicit FlightConnector(const Config &config);
    ~FlightConnector() override;

    bool init();

    std::shared_ptr<DataSource> openDefaultDataSource();
    std::shared_ptr<DataSource> openDataSource(const std::string &name);
    std::shared_ptr<DataSource> getDataSource() { return m_dataSource; }

    void start();
    void stop();
    void wait() const;

    [[nodiscard]] const Config& getConfig() const { return m_config; }

    AircraftState getState()
    {
        std::scoped_lock lock(m_stateMutex);
        return m_state;
    }

    void updateState(const AircraftState& state)
    {
        std::scoped_lock lock(m_stateMutex);
        m_state = state;
    }
};

}

#endif //FLIGHTCONNECTOR_H
