//
// Created by Ian Parker on 10/06/2024.
//

#include "ufc/device.h"

using namespace std;
using namespace UFC;

static DeviceRegistry* g_deviceRegistry = nullptr;

Device::Device(FlightConnector* flightConnector, const string& name) :
    Logger("Device[" + name + "]"),
    m_flightConnector(flightConnector),
    m_name(name)
{
}

DeviceRegistry* DeviceRegistry::getDeviceRegistry()
{
    if (g_deviceRegistry == nullptr)
    {
        g_deviceRegistry = new DeviceRegistry();
    }
    return g_deviceRegistry;
}

void DeviceRegistry::registerDevice(DeviceInit* device)
{
    getDeviceRegistry()->m_devices.try_emplace(device->getName(), device);
}

