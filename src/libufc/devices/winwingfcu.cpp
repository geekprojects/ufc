//
// Created by Ian Parker on 14/06/2024.
//

#include "winwingfcu.h"

#include "ufc/flightconnector.h"

UFC_DEVICE(WinWingFCU, WinWingFCU)

#if 0
static void hexdump(const char* pos, int len)
{
    int i;
    for (i = 0; i < len; i += 16)
    {
        int j;
        printf("%08llx: ", (uint64_t)(i));
        for (j = 0; j < 16 && (i + j) < len; j++)
        {
            printf("%02x ", (uint8_t)pos[i + j]);
        }
        for (j = 0; j < 16 && (i + j) < len; j++)
        {
            char c = pos[i + j];
            if (!isprint(c))
            {
                c = '.';
            }
            printf("%c", c);
        }
        printf("\n");
    }
}
#endif

WinWingFCU::WinWingFCU(UFC::FlightConnector* flightConnector) : USBHIDDevice(
    flightConnector,
    "WinWingFCU",
    0x4098,
    0xBB10)
{
}

bool WinWingFCU::init()
{
    bool res = USBHIDDevice::init();
    if (!res)
    {
        return false;
    }

    wchar_t serialNumber[32];
    hid_get_serial_number_string(getDevice(), serialNumber, 32);
    printf("%ls\n", serialNumber);

    hid_set_nonblocking(getDevice(), true);

    // Set brightness
    setLED(WinWingFCULED::LCD, 0xff);

    return true;
}

void WinWingFCU::update(UFC::AircraftState state)
{
    handleInput();
    updateLCD(state);
    updateLEDs(state);
}

/*
 *    --40--
 *  04|    |20
 *    --02--
 *  01|    |10
 *    --08--
 */

uint8_t WinWingFCU::getDigit(int number, int digit)
{
    number /= powl(10, digit);
    number %= 10;

    switch (number)
    {
        case 0: return 0x40 | 0x20 | 0x10 | 0x08 | 0x1 | 0x4;
        case 1: return 0x20 | 0x10;
        case 2: return 0x40 | 0x20 | 0x02 | 0x01 | 0x08;
        case 3: return 0x40 | 0x20 | 0x02 | 0x10 | 0x08;
        case 4: return 0x04 | 0x02 | 0x20 | 0x10;
        case 5: return 0x40 | 0x04 | 0x02 | 0x10 | 0x08;
        case 6: return 0x40 | 0x04 | 0x02 | 0x10 | 0x08 | 0x01;
        case 7: return 0x40 | 0x20 | 0x10;
        case 8: return 0x7f;
        case 9: return 0x40 | 0x04 | 0x02 | 0x20 | 0x10 | 0x08;
        default: return 0;
    }
}

#define checkInput(_field, _command) \
    if (m_inputReport._field != m_previousInputReport._field && m_inputReport._field) \
    { \
        m_flightConnector->getDataSource()->command(_command); \
    }

void WinWingFCU::handleInput()
{
    // Read button/rotary state
    while (true)
    {
        m_inputReport.reportId = 0x01;
        int res = hid_read(getDevice(), reinterpret_cast<unsigned char*>(&m_inputReport), sizeof(m_inputReport));
        if (res < sizeof(m_inputReport))
        {
            break;
        }
        if (m_inputReport.reportId != 0x01)
        {
            continue;
        }

        checkInput(spdMachButton, UFC::AUTOPILOT_SPD_MACH_TOGGLE)
        checkInput(ap1Button, UFC::AUTOPILOT_AP1_TOGGLE)
        checkInput(ap2Button, UFC::AUTOPILOT_AP2_TOGGLE)
        checkInput(athrButton, UFC::AUTOPILOT_AUTO_THRUST_TOGGLE)
        checkInput(locButton, UFC::AUTOPILOT_LOCALISER_TOGGLE)
        checkInput(expedButton, UFC::AUTOPILOT_EXPEDITE_TOGGLE)
        checkInput(apprButton, UFC::AUTOPILOT_APPROACH_TOGGLE)

        checkInput(hdgVsButton, UFC::AUTOPILOT_HDG_TRK_TOGGLE)
        checkInput(hdgVsButton, UFC::AUTOPILOT_VS_FPA_TOGGLE)

        checkInput(speedDown, UFC::AUTOPILOT_AIRSPEED_DOWN)
        checkInput(speedUp, UFC::AUTOPILOT_AIRSPEED_UP)
        checkInput(speedPush, UFC::AUTOPILOT_AIRSPEED_MANAGE)
        checkInput(speedPull, UFC::AUTOPILOT_AIRSPEED_GUIDANCE)

        checkInput(headingDown, UFC::AUTOPILOT_HEADING_DOWN)
        checkInput(headingUp, UFC::AUTOPILOT_HEADING_UP)
        checkInput(headingPush, UFC::AUTOPILOT_HEADING_MANAGE)
        checkInput(headingPull, UFC::AUTOPILOT_HEADING_GUIDANCE)

        checkInput(altitudeDown, UFC::AUTOPILOT_ALTITUDE_DOWN)
        checkInput(altitudeUp, UFC::AUTOPILOT_ALTITUDE_UP)
        checkInput(altitudePush, UFC::AUTOPILOT_ALTITUDE_MANAGE)
        checkInput(altitudePull, UFC::AUTOPILOT_ALTITUDE_GUIDANCE)

        checkInput(vsDown, UFC::AUTOPILOT_VERTICAL_SPEED_DOWN)
        checkInput(vsUp, UFC::AUTOPILOT_VERTICAL_SPEED_UP)
        checkInput(vsPush, UFC::AUTOPILOT_VERTICAL_SPEED_MANAGE)
        checkInput(vsPull, UFC::AUTOPILOT_VERTICAL_SPEED_GUIDANCE)

        m_previousInputReport = m_inputReport;
    }
}

void WinWingFCU::updateLCD(UFC::AircraftState state)
{
    WinWingFCULCDData data;
    int speed;
    if (!state.autopilot.speedMach)
    {
        speed = (int) state.autopilot.speed;
        data.speed2Point = false;
        data.speed1 = getDigit(speed, 2);
        data.speed2 = getDigit(speed, 1);
        data.speed3 = getDigit(speed, 0);
    }
    else
    {
        speed = (int) (state.autopilot.speed * 1000.0f);
        data.speed1 = getDigit(0, 0);
        data.speed2 = getDigit(speed, 2);
        data.speed3 = getDigit(speed, 1);
    }

    data.speed1Point = 0;
    data.speed3Point = 0;
    data.speedDot = 0;
    data.machSign = state.autopilot.speedMach;
    data.spdSign = !state.autopilot.speedMach;

    int heading = (int) ceil(state.autopilot.heading);
    data.heading1 = getDigit(heading, 2);
    data.heading2 = getDigit(heading, 1);
    data.heading3 = getDigit(heading, 0);
    data.latSign = true;

if (!state.autopilot.headingTrkMode)
{
    data.headingSign = true;
    data.headingSign2 = true;
    data.trkSign = false;
    data.trkSign2 = false;
}
else
{
    data.headingSign = false;
    data.headingSign2 = false;
    data.trkSign = true;
    data.trkSign2 = true;
}

    int altitude = (int) state.autopilot.altitude;
    data.altitude1 = getDigit(altitude, 4);
    data.altitude2 = getDigit(altitude, 3);
    data.altitude3 = getDigit(altitude, 2);
    data.altitude4 = getDigit(altitude, 1);
    data.altitude5 = getDigit(altitude, 0);
    data.altSign = true;
    data.altitudeDot = 0;

    data.vsNegative = true;
    if (state.autopilot.verticalSpeed < 0)
{
    data.vsPositive = false;
}
else
{
    data.vsPositive = true;
}


        int vs = abs((int)state.autopilot.verticalSpeed);
        data.vs1 = getDigit(vs, 3);
        data.vs2 = getDigit(vs, 2);
    if (!state.autopilot.verticalSpeedFPAMode)
    {
        data.vsSign = true;
        data.vsSign2 = true;
        data.fpaSign = false;
        data.fpaSign2 = false;

        data.vs3 = 0x1b;
        data.vs4 = 0x1b;
    }
    else
    {
        data.vsSign = false;
        data.vsSign2 = false;
        data.fpaSign = true;
        data.fpaSign2 = true;

    data.vs2Dot = true;
    data.vs3 = 0x00;
    data.vs4 = 0x00;
    }

    data.lvlChSign1 = true;
    data.lvlChSign2 = true;
    data.lvlChSign3 = true;

    hid_write(getDevice(), reinterpret_cast<unsigned char*>(&data), sizeof(data));

    // I'm not sure what this does but the LCD won't update without these magic bytes
    uint8_t data5[] =
    {
        0x00, 0x06, 0x11, 0x10, 0xbb, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x1d, 0x12, 0x20, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    m_featureReport.reportId = 0xf0;
    for (int i = 0; i < sizeof(data5); i++)
    {
        m_featureReport.vendor_data[i] = data5[i];
    }
    hid_write(getDevice(), reinterpret_cast<unsigned char*>(&m_featureReport), sizeof(m_featureReport));
}

void WinWingFCU::updateLEDs(UFC::AircraftState state)
{
    setLED(WinWingFCULED::LOC, state.autopilot.locMode);
    setLED(WinWingFCULED::AP1, state.autopilot.ap1Mode);
    setLED(WinWingFCULED::AP2, state.autopilot.ap2Mode);
    setLED(WinWingFCULED::ATHR, state.autopilot.autoThrottleMode);
    setLED(WinWingFCULED::APPR, state.autopilot.approachMode);
}

void WinWingFCU::setLED(WinWingFCULED led, int state)
{
    uint8_t data[] =
    {
        0x02, 0x10, 0xbb, 0x00, 0x00, 0x03, 0x49, static_cast<uint8_t>(led), (uint8_t)state, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    hid_write(getDevice(), data, sizeof(data));
}
