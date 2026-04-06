//
// Created by Ian Parker on 05/06/2024.
//

#include "consoleoutput.h"

#include "ufc/flightconnector.h"

using namespace UFC;

UFC_DEVICE(Console, ConsoleOutput)

ConsoleOutput::ConsoleOutput(FlightConnector* flightConnector) : Device(flightConnector, "Console")
{
    log(DEBUG, "Here!");
}

void ConsoleOutput::update(std::shared_ptr<AircraftState> state)
{
    if (!getFlightConnector()->getDataSource()->isConnected() && m_connected)
    {
        printf("ConsoleOutput::update: Not connected\n");
        m_connected = false;
        return;
    }
    //printf("ConsoleOutput::update altitude=%0.2f\n", state.altitude);
}
