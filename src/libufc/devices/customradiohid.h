//
// Created by Ian Parker on 28/09/2024.
//

#ifndef CUSTOMHID_H
#define CUSTOMHID_H

#include <ufc/usbhiddevice.h>

namespace UFC
{
struct RadioReportDataType
{
    uint8_t button0 : 1;
    uint8_t button1 : 1;
    uint8_t button2 : 1;
    uint8_t button3 : 1;
    uint8_t button4 : 1;
    uint8_t button5 : 1;
    uint8_t button6 : 1;
    uint8_t button7 : 1;
    uint8_t button8 : 1;
    uint8_t button9 : 1;
} __attribute__((packed));

struct RadioOutputDataType
{
    uint32_t value1;
    uint32_t value2;
} __attribute__((packed));

class CustomRadioHidDevice : public USBHIDDevice
{
 private:
    RadioReportDataType m_inputReport = {};
    RadioReportDataType m_previousInputReport = {};

 public:
    explicit CustomRadioHidDevice(FlightConnector* flightConnector);
    ~CustomRadioHidDevice() override = default;

    bool init() override;
    void update(const AircraftState& state) override;
};
}

#endif //CUSTOMHID_H
