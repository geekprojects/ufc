//
// Created by Ian Parker on 27/05/2024.
//

#ifndef UFC_FLIGHTCONNECTOR_H
#define UFC_FLIGHTCONNECTOR_H

#include <thread>
#include <mutex>
#include <vector>
#include <memory>

#include <ufc/datasource.h>

#include "logger.h"


namespace UFC
{

class UFCLua;

enum class Result
{
    SUCCESS,
    TIMEOUT,
    FAIL
};

class DataSource;
class Device;
class Airports;
class USBHIDConfigManager;
class SerialConfigManager;

struct Config
{
    /**
     * Path to the configuration file
     */
    std::string configPath;

    /**
     * Path to libufc's data, such as aircraft definitions etc
     */
    std::string dataDir;

    /**
     * Name of the default DataSource
     */
    std::string dataSource;

    std::string navDataSource;

    // X-Plane specific
    std::string xplanePath;
    std::string xplaneHost;
    int xplanePort = 0;

    // Arduino specific
    std::string arduinoDevice;

    // Little Nav Map Nav Data
    std::string littleNavMapPath;

    void dump() const
    {
        printf("Config::dump: configPath=%s\n", configPath.c_str());
        printf("Config::dump: dataPath=%s\n", dataDir.c_str());
        printf("Config::dump: dataSource=%s\n", dataSource.c_str());
        printf("Config::dump: X-Plane path=%s\n", xplanePath.c_str());
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

    std::vector<std::shared_ptr<Device>> m_devices;
    std::shared_ptr<USBHIDConfigManager> m_usbhidConfigManager;
    std::shared_ptr<SerialConfigManager> m_serialConfigManager;

    std::shared_ptr<NavDataSource> m_navDataSource;

    std::mutex m_stateMutex;
    AircraftState m_state;

    bool m_running = false;
    std::shared_ptr<std::thread> m_updateDeviceThread = nullptr;
    std::shared_ptr<std::thread> m_updateDataSourceThread = nullptr;
    bool m_exitHandler = true;

    std::mutex m_threadsMutex;
    std::set<std::thread::id> m_threads;

    static void updateDeviceThread(FlightConnector* flightConnector);
    void updateDeviceMain();

    static void updateDataSourceThread(FlightConnector* flightConnector);
    void updateDataSourceMain();


 public:
    FlightConnector();
    ~FlightConnector() override;

    void loadConfig(const Config &config);
    void setConfig(const Config &config);
    void writeConfig();

    /**
     * Initialise this Flight Connector
     *
     * @return true if successfully initialised, otherwise false.
     */
    bool init();

    /**
     * Detect and initialise devices.
     *
     * This only needs to be called if you intend to use these devices.
     * If you are using libufc as an API to talk to your simulator, you probably
     * don't need to call this.
     *
     * @return true if devices were successfully initialised.
     */
    bool initDevices();

    /**
     * Find and open the default data source set in configuration
     *
     * @return The DataSource
     */
    std::shared_ptr<DataSource> openDefaultDataSource();

    /**
     * Open a specific DataSource
     *
     * @param name Name of the Data Source
     * @return The DataSource
     */
    std::shared_ptr<DataSource> openDataSource(const std::string &name);

    /**
     *
     * @return The currently opened DataSource or nullptr if none.
     */
    std::shared_ptr<DataSource> getDataSource() { return m_dataSource; }

    /**
     * Set the opened CataSource to a custom DataSource that libufc doesn't
     * manage.
     *
     * @param dataSource The DataSource to set
     */
    void setDataSource(std::shared_ptr<DataSource> dataSource);

    /**
     * Start threads that manage communication with the DataSource and devices
     */
    void start();

    /**
     * Stop threads that manage the DataSource and devices.
     */
    void stop();

    /**
     * Wait for the DataSource and device threads to end.
     */
    void wait() const;

    /**
     *
     * @return The current configuration
     */
    [[nodiscard]] const Config& getConfig() const { return m_config; }

    [[nodiscard]] std::string getDataPath() const
    {
        return m_config.dataDir;
    }

    /**
     *
     * @return A copy of the current aircraft state from the simulator
     */
    AircraftState getState()
    {
        std::scoped_lock lock(m_stateMutex);
        return m_state;
    }

    /**
     *
     * @param state The Aircraft state data to update with
     */
    void updateState(const AircraftState& state)
    {
        std::scoped_lock lock(m_stateMutex);
        m_state = state;
    }

    /**
     * Stop all currently running FlightConnectors
     */
    static void exit();

    void addDevice(std::shared_ptr<Device> device)
    {
        m_devices.push_back(device);
    }

    /**
     * Poll devices for updates once
     */
    void updateDevices();

    /**
     * @return All currently discovered and initialised Devices.
     */
    const std::vector<std::shared_ptr<Device>>& getDevices() { return m_devices; }

    /**
     * Disables the exit handling that is usually set by the FlightConnector.
     * Use this when using libufc in other environments that already handle
     * signals, such as within a simulator plugin.
     */
    void disableExitHandler() { m_exitHandler = false; }

    void registerThread()
    {
        std::scoped_lock lock(m_threadsMutex);
        m_threads.insert(std::this_thread::get_id());
    }

    void unregisterThread()
    {
        std::scoped_lock lock(m_threadsMutex);
        m_threads.erase(std::this_thread::get_id());
    }

    bool isOurThread()
    {
        std::scoped_lock lock(m_threadsMutex);
        const std::thread::id thread_id = std::this_thread::get_id();
        return m_threads.contains(thread_id);
    }

    std::shared_ptr<NavDataSource> getNavDataSource();
};

}

#endif //FLIGHTCONNECTOR_H
