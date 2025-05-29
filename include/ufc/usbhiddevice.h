//
// Created by Ian Parker on 14/06/2024.
//

#ifndef UFC_USBHIDDEVICE_H
#define UFC_USBHIDDEVICE_H

#include "device.h"

#include <vector>

#include <hidapi.h>

#define checkInput(_field, _command) \
    if (m_inputReport._field != m_previousInputReport._field && m_inputReport._field) \
    { \
        m_flightConnector->getDataSource()->command(_command); \
    }

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

 protected:
    hid_device* getDevice() { return m_device; }

 public:
    USBHIDDevice(FlightConnector* flightConnector, const std::string &name, uint16_t vendorId, uint16_t productId);
    ~USBHIDDevice() override;

    bool detect() override;

    bool init() override;
    void close() override;
};

}

#endif //USBHIDDEVICE_H
