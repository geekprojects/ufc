//
// Created by Ian Parker on 28/09/2024.
//

#include "customhid.h"
#include "ufc/flightconnector.h"

#include <cmath>

using namespace std;
using namespace UFC;

UFC_DEVICE(CustomHidDevice, CustomHidDevice)



CustomHidDevice::CustomHidDevice(FlightConnector* flightConnector) :
    USBHIDDevice(flightConnector, "CustomHidDevice", 0x1209, 0xd200)
{
}

void CustomHidDevice::update(AircraftState state)
{
    hid_read(getDevice(), reinterpret_cast<unsigned char*>(&m_radioReport), sizeof(m_radioReport));

    int i = 0;
    i |= m_radioReport.button0 << 0;
    i |= m_radioReport.button1 << 1;
    i |= m_radioReport.button2 << 2;
    i |= m_radioReport.button3 << 3;
    i |= m_radioReport.button4 << 4;
    i |= m_radioReport.button5 << 5;
    i |= m_radioReport.button6 << 6;
    i |= m_radioReport.button7 << 7;
    printf("CustomHidDevice::update: i=%d\n", i);

    i++;

    RadioOutputDataType outputData;
    printf("CustomHidDevice::update: com1Hz=%0.3f\n", state.comms.com1Hz);
    outputData.value1 = (int)floor(state.comms.com1Hz * 10.0f);
    outputData.value2 = (int)floor(state.comms.com1StandbyHz * 10.0f);

    hid_write(getDevice(), reinterpret_cast<unsigned char*>(&outputData), sizeof(outputData));

    m_flightConnector->getDataSource()->command("comms/com1/standby/up/fine");
}
