//
// Created by Ian Parker on 26/05/2024.
//

#include <ufc/flightconnector.h>
#include <ufc/datasource.h>

#include <cstdio>
#include <getopt.h>

using namespace std;
using namespace UFC;

static option g_options[] =
{
    {"config", required_argument, 0,  'c' },
    {"data", required_argument, 0,  'd' },
    {"source", required_argument, 0,  's' },
    {"xplane-port", required_argument, 0,  'p' },
    {"help", no_argument, 0,  'h' },
    {0, 0, 0,  0 }
};

int main(int argc, char** argv)
{
    Config config;

    while (true)
    {
        int option_index = 0;
        int c = getopt_long (argc, argv, "c:d:s:p:", g_options, &option_index);
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
            case 'c':
                config.configPath = optarg;
                break;
            case 'd':
                config.dataDir = optarg;
                break;
            case 's':
                config.dataSource = optarg;
                break;
            case 'p':
                config.xplanePort = atoi(optarg);
                break;
        }
    }

    FlightConnector flightConnector(config);

    flightConnector.init();
    auto dataSource = flightConnector.openDefaultDataSource();
    bool res = dataSource->init();
    if (!res)
    {
        return 1;
    }

    flightConnector.start();
    flightConnector.wait();

    return 0;
}

