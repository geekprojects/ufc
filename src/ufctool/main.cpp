//
// Created by Ian Parker on 26/05/2024.
//

#include <cstdio>

#include <ufc/flightconnector.h>
#include <ufc/datasource.h>

using namespace std;
using namespace UFC;

int main(int argc, char** argv)
{
    FlightConnector flightConnector;

    flightConnector.init();
    auto dataSource = flightConnector.openDataSource(SOURCE_SIMULATOR);
    bool res = dataSource->init();
    if (!res)
    {
        return 1;
    }

    flightConnector.start();
    flightConnector.wait();

    return 0;
}

