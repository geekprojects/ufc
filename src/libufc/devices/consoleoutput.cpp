//
// Created by Ian Parker on 05/06/2024.
//

#include "consoleoutput.h"

using namespace UFC;

UFC_DEVICE(ConsoleOutput, Consoleoutput)

ConsoleOutput::ConsoleOutput(FlightConnector* flightConnector) : Device(flightConnector, "Console")
{
    log(DEBUG, "Here!");
}

void ConsoleOutput::update(const AircraftState& state)
{
    if (!state.connected)
    {
        printf("ConsoleOutput::update: Not connected\n");
        return;
    }
    //printf("ConsoleOutput::update altitude=%0.2f\n", state.altitude);
}
