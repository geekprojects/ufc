//
// Created by Ian Parker on 14/06/2024.
//

#include "winwingfcu.h"

UFC_DEVICE(WinWingFCU, WinWingFCU)

WinWingFCU::WinWingFCU(UFC::FlightConnector* flightConnector) :
    USBHIDDevice(flightConnector, "WinWingFCU", 0x4098, 0xBB10)
{
}

bool WinWingFCU::init()
{
    bool res = USBHIDDevice::init();
    if (!res)
    {
        return false;
    }

    // Stuff

    return true;
}

void WinWingFCU::update(UFC::AircraftState state)
{
}
