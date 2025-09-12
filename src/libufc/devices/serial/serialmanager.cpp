//
// Created by Ian Parker on 20/08/2025.
//

#include "serial.h"
#include "ufc/utils.h"

using namespace std;
using namespace UFC;

SerialConfigManager::SerialConfigManager(FlightConnector* flightConnector) :
    Logger("SerialConfigManager"),
    m_flightConnector(flightConnector)
{
}

void SerialConfigManager::scan()
{
    auto ports = listPorts();

    // Try to find aircraft-specific definitions
    for (const auto & entry : filesystem::directory_iterator(m_flightConnector->getDataPath() + "/serial"))
    {
        if (entry.path().extension() == ".yaml")
        {
            log(DEBUG, "scan: Checking: %s", entry.path().filename().string().c_str());
            auto configFile = YAML::LoadFile(entry.path());
            auto device = checkDevice(configFile, ports);
            if (device != nullptr)
            {
                m_devices.try_emplace(device->getPort(), device);
                device->init();
                m_flightConnector->addDevice(device);
            }
        }
    }
}

std::shared_ptr<SerialConfigDevice> SerialConfigManager::checkDevice(
    const YAML::Node &node,
    const std::vector<SerialPortInfo> &ports)
{
    if (node["type"].as<std::string>() != "USB")
    {
        return nullptr;
    }

    SerialPortInfo devicePort;
    bool found = false;
    for (const auto & port : ports)
    {
        int vendorId = node["vendorId"].as<int>();
        int productId = node["productId"].as<int>();
        if (vendorId == port.vendorId && productId == port.productId)
        {
            found = true;
            devicePort = port;
            break;
        }
    }
    if (!found)
    {
        return nullptr;
    }

    auto device = make_shared<SerialConfigDevice>(m_flightConnector, node, devicePort);
    if (!device->detect())
    {
        return nullptr;
    }

    return device;
}
