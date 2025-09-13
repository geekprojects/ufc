//
// Created by Ian Parker on 05/06/2024.
//

#ifndef UFC_CONSOLE_OUTPUT_H
#define UFC_CONSOLE_OUTPUT_H

#include <ufc/device.h>

namespace UFC
{
class ConsoleOutput : public Device
{
 private:
    bool m_connected = true;
 public:
    explicit ConsoleOutput(FlightConnector* flightConnector);
    ~ConsoleOutput() override = default;

    void update(std::shared_ptr<AircraftState> state) override;

    bool detect() override { return false; }
};
}

#endif
