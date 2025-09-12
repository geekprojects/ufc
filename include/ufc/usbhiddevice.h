//
// Created by Ian Parker on 14/06/2024.
//

#ifndef UFC_USBHIDDEVICE_H
#define UFC_USBHIDDEVICE_H

#include <ufc/device.h>

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

    friend bool operator<(const USBIds &lhs, const USBIds &rhs)
    {
        if (lhs.vendorId < rhs.vendorId)
            return true;
        if (rhs.vendorId < lhs.vendorId)
            return false;
        return lhs.productId < rhs.productId;
    }

    friend bool operator==(const USBIds &lhs, const USBIds &rhs)
    {
        return lhs.vendorId == rhs.vendorId
               && lhs.productId == rhs.productId;
    }

    friend bool operator!=(const USBIds &lhs, const USBIds &rhs)
    {
        return !(lhs == rhs);
    }
};

class USBHIDDevice : public Device
{
 private:
    std::vector<USBIds> m_compatibleIds;

    hid_device* m_device = nullptr;

 protected:
    USBIds m_foundId;
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
