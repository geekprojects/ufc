//
// Created by Ian Parker on 14/06/2024.
//

#ifndef WINWINGFCU_H
#define WINWINGFCU_H
#include "ufc/usbhiddevice.h"

enum class WinWingFCULED
{
    LCD = 0x1,
    LOC = 0x3,
    AP1 = 0x5,
    AP2 = 0x7,
    ATHR = 0x9,
    EXPED = 0xb,
    APPR = 0xd
};

struct WinWingFCUInputReport
{
    uint8_t reportId;

    uint8_t spdMachButton : 1;
    uint8_t locButton : 1;
    uint8_t hdgVsButton : 1;
    uint8_t ap1Button : 1;
    uint8_t ap2Button : 1;
    uint8_t athrButton : 1;
    uint8_t expedButton : 1;
    uint8_t metricAltButton : 1;

    uint8_t apprButton : 1;
    uint8_t speedDown : 1;
    uint8_t speedUp : 1;
    uint8_t speedPush : 1;
    uint8_t speedPull : 1;
    uint8_t headingDown : 1;
    uint8_t headingUp : 1;
    uint8_t headingPush : 1;

    uint8_t headingPull : 1;
    uint8_t altitudeDown : 1;
    uint8_t altitudeUp : 1;
    uint8_t altitudePush : 1;
    uint8_t altitudePull : 1;
    uint8_t vsDown : 1;
    uint8_t vsUp : 1;
    uint8_t vsPush : 1;
    uint8_t vsPull : 1;
    uint8_t altitude100 : 1;
    uint8_t altitude1000 : 1;

    uint8_t padding2[12];

    // Not actually used!
    uint16_t x_axis;
    uint16_t y_axis;
    uint16_t z_axis;
    uint16_t rx_axis;
    uint16_t ry_axis;
    uint16_t rz_axis;
} __attribute__((packed));

struct WinWingFCUOutputReport
{
    uint8_t reportId;
    uint8_t vendor_data[13];  // Vendor-specific output data (13 bytes)
} __attribute__((packed));

struct WinWingFCUFeatureReport {
    uint8_t reportId;
    uint8_t vendor_data[63];  // Vendor-specific feature data (63 bytes)
} __attribute__((packed));

struct WinWingFCULCDData
{
    uint8_t featureId = 0xf0;

    // The magic header
    uint8_t header[24] =
    {
        0x00, 0x05, 0x31, 0x10, 0xbb, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x1d, 0x12, 0x20, 0x10,
        0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    unsigned char speed1Point : 1;
    uint8_t speed1 : 7;
    unsigned char speed2Point : 1;
    uint8_t speed2 : 7;
    unsigned char speed3Point : 1;
    uint8_t speed3 : 7;
    unsigned char padding1 : 1;
    unsigned char speedDot : 1;
    unsigned char machSign : 1;
    unsigned char spdSign : 1;
    unsigned char heading1Point : 1;
    unsigned char heading1 : 7;
    unsigned char heading2Point : 1;
    unsigned char heading2 : 7;
    unsigned char heading3Point : 1;
    unsigned char heading3 : 7;

    unsigned char headingDot : 1;
    unsigned char latSign : 1;
    unsigned char trkSign : 1;
    unsigned char headingSign : 1;
    unsigned char fpaSign : 1;
    unsigned char trkSign2 : 1;
    unsigned char vsSign : 1;
    unsigned char headingSign2 : 1;

    unsigned char altitude1Dot : 1;
    unsigned char altitude1 : 7;
    unsigned char altSign : 1;
    unsigned char altitude2 : 7;
    unsigned char lvlChSign1 : 1;
    unsigned char altitude3 : 7;
    unsigned char lvlChSign2 : 1;
    unsigned char altitude4 : 7;
    unsigned char lvlChSign3 : 1;
    unsigned char altitude5 : 7;

    unsigned char vsNegative : 1;
    unsigned char vs1 : 7;
    unsigned char vs2Dot : 1;
    unsigned char vs2 : 7;
    unsigned char vsPositive : 1;
    unsigned char vs3 : 7;
    unsigned char altitudeDot : 1;
    unsigned char vs4 : 7;

    unsigned char padding2 : 1;
    unsigned char padding3 : 1;
    unsigned char vsSign2 : 1;
    unsigned char fpaSign2 : 1;
    unsigned char padding6 : 1;
    unsigned char padding7 : 1;
    unsigned char padding8 : 1;
    unsigned char padding9 : 1;

    uint8_t unknown[21] = {};
} __attribute__((packed));

class WinWingFCU : public UFC::USBHIDDevice
{
 private:
    WinWingFCUInputReport m_previousInputReport = {};
    WinWingFCUInputReport m_inputReport = {};
    //WinWingFCUOutputReport m_outputReport = {};
    WinWingFCUFeatureReport m_featureReport = {};

    void updateLCD(UFC::AircraftState state);
    void updateLEDs(UFC::AircraftState state);
    void setLED(WinWingFCULED led, int state);

    static uint8_t getDigit(int number, int digit);

 public:
    explicit WinWingFCU(UFC::FlightConnector* flightConnector);
    ~WinWingFCU() override = default;

    bool init() override;

    void handleInput();

    void update(UFC::AircraftState state) override;
};

#endif //WINWINGFCU_H
