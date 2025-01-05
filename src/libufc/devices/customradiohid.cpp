//
// Created by Ian Parker on 28/09/2024.
//

#include "customradiohid.h"

#include <ufc/flightconnector.h>
#include <ufc/commands.h>

using namespace std;
using namespace UFC;

UFC_DEVICE(CustomRadio, CustomRadioHidDevice)

CustomRadioHidDevice::CustomRadioHidDevice(FlightConnector* flightConnector) :
    USBHIDDevice(flightConnector, "CustomRadioHidDevice", 0x1209, 0xd200)
{
}

bool CustomRadioHidDevice::init()
{
    bool res = USBHIDDevice::init();
    if (!res)
    {
        return false;
    }

    hid_set_nonblocking(getDevice(), true);
    return true;
}

void CustomRadioHidDevice::update(const AircraftState& state)
{
    while (true)
    {
        int res = hid_read(
            getDevice(),
            reinterpret_cast<unsigned char*>(&m_inputReport),
            sizeof(m_inputReport));
        if (res <= 0)
        {
            break;
        }

        checkInput(button1, COMMS_COM1_STANDBY_UP_FINE);
        checkInput(button0, COMMS_COM1_STANDBY_DOWN_FINE);
        checkInput(button3, COMMS_COM1_STANDBY_UP_COARSE);
        checkInput(button2, COMMS_COM1_STANDBY_DOWN_COARSE);
        checkInput(button4, COMMS_COM1_SWAP);

        m_previousInputReport = m_inputReport;
    }

    RadioOutputDataType outputData = {};
    outputData.value1 = state.comms.com1Hz;
    outputData.value2 = state.comms.com1StandbyHz;

    hid_write(getDevice(), reinterpret_cast<unsigned char*>(&outputData), sizeof(outputData));
}
