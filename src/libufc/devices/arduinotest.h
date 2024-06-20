//
// Created by Ian Parker on 10/06/2024.
//

#ifndef ARDUINOTEST_H
#define ARDUINOTEST_H

#include <ufc/device.h>

#include <thread>

class ArduinoTest : public UFC::Device
{
 private:
    int m_fd = -1;

    std::thread* m_readThread = nullptr;

    void handleLine(const std::string &line) const;

    void writeNumber(int id, int value);

 public:
    ArduinoTest(UFC::FlightConnector* flightConnector);
    ~ArduinoTest() override;

    bool detect() override;
    bool init() override;

    std::string getName() override;

    void update(UFC::AircraftState state) override;

    void readMain();
};

#endif //ARDUINOTEST_H
