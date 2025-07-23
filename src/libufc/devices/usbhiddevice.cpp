//
// Created by Ian Parker on 14/06/2024.
//

#include <ufc/usbhiddevice.h>

using namespace std;
using namespace UFC;

USBHIDDevice::USBHIDDevice(
    FlightConnector* flightConnector,
    const string &name,
    uint16_t vendorId,
    uint16_t productId)
: Device(flightConnector, name)
{
    m_compatibleIds.emplace_back(vendorId, productId);
}

USBHIDDevice::~USBHIDDevice() = default;

bool USBHIDDevice::detect()
{
    bool found = false;
    for (auto id : m_compatibleIds)
    {
        hid_device* handle = hid_open(id.vendorId, id.productId, nullptr);
        if (handle != nullptr)
        {
            hid_close(handle);
            log(INFO, "Found USB HID device: %0x:%0x", id.vendorId, id.productId);
            m_foundId = id;
            found = true;
            break;
        }
    }
    return found;
}

bool USBHIDDevice::init()
{
    if (m_foundId.vendorId == 0 && m_foundId.productId == 0)
    {
        log(ERROR, "init: No compatible device found");
        return false;
    }

    m_device = hid_open(m_foundId.vendorId, m_foundId.productId, nullptr);
    if (m_device == nullptr)
    {
        log(ERROR, "init: Could not open USB HID device: %s", hid_error(nullptr));
        return false;
    }
    return true;
}

void USBHIDDevice::close()
{
    if (m_device != nullptr)
    {
        hid_close(m_device);
        m_device = nullptr;
    }
}
