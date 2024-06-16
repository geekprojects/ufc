//
// Created by Ian Parker on 14/06/2024.
//

#ifndef WINWINGFCU_H
#define WINWINGFCU_H
#include "ufc/usbhiddevice.h"

class WinWingFCU : public UFC::USBHIDDevice
{
 public:
    WinWingFCU(UFC::FlightConnector* flightConnector) : USBHIDDevice(flightConnector, "WinWingFCU", 0x4098, 0xBB10)
    {
    }

    ~WinWingFCU() = default;

    bool init() override;

    void update(UFC::AircraftState state) override;
};



#endif //WINWINGFCU_H
