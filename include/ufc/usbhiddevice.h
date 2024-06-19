//
// Created by Ian Parker on 14/06/2024.
//

#ifndef USBHIDDEVICE_H
#define USBHIDDEVICE_H
#include "device.h"

#include <hidapi/hidapi.h>

namespace UFC
{

struct USBIds
{
    uint16_t vendorId = 0;
    uint16_t productId = 0;

    USBIds() = default;

    USBIds(const uint16_t v, const uint16_t p)
    {
        vendorId = v;
        productId = p;
    }
};

class USBHIDDevice : public Device
{
 private:
    std::vector<USBIds> m_compatibleIds;
    USBIds m_foundId;

    hid_device* m_device = nullptr;

 public:
    USBHIDDevice(FlightConnector* flightConnector, const std::string &name, uint16_t vendorId, uint16_t productId);
    ~USBHIDDevice() override;

    bool detect() override;

    bool init() override;
};

}

#endif //USBHIDDEVICE_H
