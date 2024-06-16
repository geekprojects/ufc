//
// Created by Ian Parker on 05/06/2024.
//

#ifndef CONSOLEOUTPUT_H
#define CONSOLEOUTPUT_H

#include <ufc/device.h>

namespace UFC
{
class ConsoleOutput : public Device
{
 private:
 public:
    ConsoleOutput(FlightConnector* flightConnector);
    ~ConsoleOutput() override = default;

    void update(AircraftState state) override;

    bool detect() override { return true; }
};
}

#endif //CONSOLEOUTPUT_H
