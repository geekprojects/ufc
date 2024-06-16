//
// Created by Ian Parker on 27/05/2024.
//

#include <unistd.h>
#include <ufc/flightconnector.h>

#include "datasources/simulator.h"
#include "datasources/xplane/xplane.h"
#include "ufc/device.h"

using namespace std;
using namespace UFC;

FlightConnector::FlightConnector() : Logger("FlightConnector")
{
}

FlightConnector::~FlightConnector()
{
}

bool FlightConnector::init()
{
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
    return openDataSource(SOURCE_SIMULATOR);
}

std::shared_ptr<DataSource> FlightConnector::openDataSource(std::string name)
{
    if (name == SOURCE_XPLANE)
    {
        m_dataSource = make_shared<XPlaneDataSource>();
    }
    if (name == SOURCE_SIMULATOR)
    {
        m_dataSource = make_shared<SimulatorDataSource>();
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
    m_running = false;
}

void FlightConnector::wait()
{
    if (!m_updateDeviceThread)
    {
        // Nothing to wait for
        return;
    }

    m_updateDeviceThread->join();
    m_updateDataSourceThread->join();
}

void FlightConnector::updateDeviceMain()
{
    auto dataSource = getDataSource();

    while (m_running)
    {
        auto state = dataSource->getState();
        for (auto device : m_devices)
        {
            device->update(state);
        }
        usleep(200000);
    }
}

void FlightConnector::updateDataSourceMain()
{
    auto dataSource = getDataSource();
    dataSource->update();
}
