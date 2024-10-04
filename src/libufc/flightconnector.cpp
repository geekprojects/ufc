//
// Created by Ian Parker on 27/05/2024.
//

#include <fstream>
#include <signal.h>
#include <unistd.h>
#include <ufc/flightconnector.h>

#include "datasources/simulator.h"
#include "datasources/xplane/xplane.h"
#include "ufc/device.h"

using namespace std;
using namespace UFC;

#define STRINGIFY(x) XSTRINGIFY(x)
#define XSTRINGIFY(x) #x

static void exitHandler(int signal)
{
    FlightConnector::exit();
}

vector<FlightConnector*> g_flightConnectors;

FlightConnector::FlightConnector() :
    Logger("FlightConnector")
{
    Config config = {};
    loadConfig(config);
    g_flightConnectors.push_back(this);
}

FlightConnector::FlightConnector(const Config& config) :
    Logger("FlightConnector")
{
    loadConfig(config);
    g_flightConnectors.push_back(this);
}

FlightConnector::~FlightConnector()
{
    for (auto it = g_flightConnectors.begin(); it != g_flightConnectors.end(); it++)
    {
        if (*it == this)
        {
            g_flightConnectors.erase(it);
            break;
        }
    }
}

bool FlightConnector::init()
{
    struct sigaction sigIntHandler = {};
    sigIntHandler.sa_handler = exitHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, nullptr);
    sigaction(SIGTERM, &sigIntHandler, nullptr);

    DeviceRegistry* deviceRegistry = DeviceRegistry::getDeviceRegistry();
    for (const auto& dev : deviceRegistry->getDevices())
    {
        Device* device = dev.second->create(this);
        if (device->detect())
        {
            log(INFO, "init: Detected device: %s", dev.first.c_str());
            if (device->init())
            {
                m_devices.push_back(device);
            }
            else
            {
                delete device;
            }
        }
        else
        {
            delete device;
        }
    }
    return true;
}

shared_ptr<DataSource> FlightConnector::openDefaultDataSource()
{
    return openDataSource(m_config.dataSource);
}

std::shared_ptr<DataSource> FlightConnector::openDataSource(const std::string& name)
{
    log(DEBUG, "openDataSource: Opening %s", name.c_str());
    if (name == SOURCE_XPLANE)
    {
        m_dataSource = make_shared<XPlaneDataSource>(this);
    }
    else if (name == SOURCE_SIMULATOR)
    {
        m_dataSource = make_shared<SimulatorDataSource>(this);
    }
    else
    {
        log(ERROR, "openDataSource: Unrecognised data source: %s", name.c_str());
        m_dataSource = nullptr;
    }
    return m_dataSource;
}

void FlightConnector::updateDeviceThread(FlightConnector* flightConnector)
{
    flightConnector->updateDeviceMain();
}
void FlightConnector::updateDataSourceThread(FlightConnector* flightConnector)
{
    flightConnector->updateDataSourceMain();
}

void FlightConnector::start()
{
    m_running = true;
    m_updateDeviceThread = make_shared<thread>(updateDeviceThread, this);
    m_updateDataSourceThread = make_shared<thread>(updateDataSourceThread, this);
}

void FlightConnector::stop()
{
    log(INFO, "Stopping data source...");
    m_running = false;
    if (m_dataSource != nullptr)
    {
        m_dataSource->disconnect();
    }
    log(INFO, "Stopped");
}

void FlightConnector::wait() const
{
    if (!m_updateDeviceThread)
    {
        // Nothing to wait for
        return;
    }

    m_updateDeviceThread->join();
    m_updateDataSourceThread->join();
}

void FlightConnector::exit()
{
    for (auto fc : g_flightConnectors)
    {
        fc->stop();
    }
}

void FlightConnector::updateDeviceMain()
{
    auto dataSource = getDataSource();

    while (m_running)
    {
        const auto state = getState();
        for (auto device : m_devices)
        {
            device->update(state);
        }
        usleep(20000);
    }
}

void FlightConnector::updateDataSourceMain()
{
    auto dataSource = getDataSource();
    dataSource->update();
}

void FlightConnector::loadConfig(const Config& config)
{
    bool writeConfig = false;
    if (config.configPath.empty())
    {
        m_config.configPath = string(getenv("HOME")) + "/.config/ufc.yml";
        writeConfig = true;
    }
    else
    {
        m_config.configPath = config.configPath;
    }

    // Set defaults
    m_config.dataDir = STRINGIFY(DATADIR);
    m_config.dataSource = "Simulator";
    m_config.xplaneHost = "127.0.0.1";
    m_config.xplanePort = 49000;
    m_config.arduinoDevice = "";

    // Set values from config file
    if (access(m_config.configPath.c_str(), R_OK) == 0)
    {
        log(INFO, "Loading configuration from: %s", m_config.configPath.c_str());
        auto configNode = YAML::LoadFile(m_config.configPath);

        if (configNode["dataDir"])
        {
            m_config.dataDir = configNode["dataDir"].as<string>();
        }
        if (configNode["dataSource"])
        {
            m_config.dataSource = configNode["dataSource"].as<string>();
        }

        if (configNode["xplane"])
        {
            auto xplaneNode = configNode["xplane"];
            if (xplaneNode["path"])
            {
                m_config.xplanePath = xplaneNode["path"].as<string>();
            }
            if (xplaneNode["host"])
            {
                m_config.xplaneHost = xplaneNode["host"].as<string>();
            }
            if (xplaneNode["port"])
            {
                m_config.xplanePort = xplaneNode["port"].as<int>();
            }
        }

        if (configNode["arduino"])
        {
            auto arduinoNode = configNode["arduino"];
            if (arduinoNode["device"])
            {
                m_config.arduinoDevice = arduinoNode["device"].as<string>();
            }
        }
        writeConfig = false;
    }

    // Now override with any command line config
    if (!config.dataDir.empty())
    {
        m_config.dataDir = config.dataDir;
    }
    if (!config.dataSource.empty())
    {
        m_config.dataSource = config.dataSource;
    }
    if (!config.xplaneHost.empty())
    {
        m_config.xplaneHost = config.xplaneHost;
    }
    if (config.xplanePort != 0)
    {
        m_config.xplanePort = config.xplanePort;
    }
    if (config.arduinoDevice.empty())
    {
        m_config.arduinoDevice = config.arduinoDevice;
    }

    if (writeConfig)
    {
        YAML::Node configNode;
        configNode["dataDir"] = m_config.dataDir;
        configNode["dataSource"] = m_config.dataSource;

        YAML::Node xplaneNode;
        xplaneNode["path"] = m_config.xplanePath;
        xplaneNode["host"] = m_config.xplaneHost;
        xplaneNode["port"] = m_config.xplanePort;
        configNode["xplane"] = xplaneNode;

        YAML::Node arduinoNode;
        arduinoNode["device"] = m_config.arduinoDevice;
        configNode["arduino"] = arduinoNode;

        ofstream configStream(m_config.configPath);
        configStream << configNode;
        configStream.close();
    }

    m_config.dump();
}

