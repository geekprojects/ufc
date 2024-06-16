//
// Created by Ian Parker on 14/06/2024.
//

#include "winwingfcu.h"

UFC_DEVICE(WinWingFCU, WinWingFCU)

bool WinWingFCU::init()
{
    return USBHIDDevice::init();
}

void WinWingFCU::update(UFC::AircraftState state)
{
}
