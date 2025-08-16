//
// Created by Ian Parker on 22/07/2025.
//

#include <ufc/usbhidconfig.h>
#include <ufc/flightconnector.h>

#include <filesystem>
#include <hidapi.h>
#include <yaml-cpp/yaml.h>

using namespace std;
using namespace UFC;

USBHIDConfigManager::USBHIDConfigManager(FlightConnector* flightConnector) :
    Logger("USBHIDConfigManager"),
    m_flightConnector(flightConnector),
    m_baseDir(flightConnector->getConfig().dataDir)
{}

void USBHIDConfigManager::scan()
{
    filesystem::path defDir(m_baseDir + "/usbhid");
    if (!exists(defDir))
    {
        log(WARN, "scan: Unable to find data definition directory: %s", defDir.string().c_str());
        return;
    }

    // Try to find aircraft-specific definitions
    for (const auto & entry : filesystem::directory_iterator(m_baseDir + "/usbhid"))
    {
        log(DEBUG, "scan: Checking: %s", entry.path().filename().string().c_str());
        if (entry.path().extension() == ".yaml")
        {
            auto configFile = YAML::LoadFile(entry.path());
            USBIds id;
            if (checkDevice(configFile, id))
            {
                auto it = m_devices.find(id);
                if (it != m_devices.end())
                {
                    // We've already found this device!
                    continue;
                }
                openDevice(configFile, id);
            }
        }
    }
}

bool USBHIDConfigManager::checkDevice(YAML::Node node, USBIds& id)
{
    const auto vendorId = node["vendorId"].as<uint16_t>();
    const auto productId = node["productId"].as<uint16_t>();
    auto handle = hid_open(vendorId, productId, nullptr);
    if (handle != nullptr)
    {
        auto name = node["name"].as<string>();
        log(INFO, "USBHIDConfigManager::checkDevice: Found: %s\n", name.c_str());
        hid_close(handle);

        id.productId = productId;
        id.vendorId = vendorId;
        return true;
    }
    return false;
}

void USBHIDConfigManager::openDevice(const YAML::Node &node, USBIds id)
{
    string name = node["name"].as<string>();
    USBHIDConfigDevice* device = new USBHIDConfigDevice(
        m_flightConnector,
        name,
        id.vendorId,
        id.productId);
    bool res = device->loadConfig(node);
    if (!res)
    {
        return;
    }

    res = device->init();
    if (!res)
    {
        return;
    }
    m_devices.insert(make_pair(id, device));

    m_flightConnector->addDevice(device);
}
