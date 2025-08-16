//
// Created by Ian Parker on 26/05/2024.
//

#include <ufc/flightconnector.h>
#include <ufc/datasource.h>
#include <ufc/airports.h>

#include <cstdio>
#include <getopt.h>

using namespace std;
using namespace UFC;

const static option g_options[] =
{
    {"config", required_argument, nullptr,  'c' },
    {"data", required_argument, nullptr,  'd' },
    {"source", required_argument, nullptr,  's' },
    {"help", no_argument, nullptr,  'h' },
    {nullptr, 0, nullptr,  0 }
};

[[noreturn]] void usage()
{
    printf("UFC - Universal Flight Connector\n");
    printf("\n");
    printf("Usage: ufctool [OPTIONS]\n");
    printf("  -c, --config  Specify configuration file. (Default: %s/.config/ufc.yaml)\n", getenv("HOME"));
    printf("  -d, --data    Specify data directory (Defaults to configured directory)\n");
    printf("  -s, --source  Data Source type (Defaults to configured source)\n");
    printf("  -h, --help    This help text\n");
    exit(0);
}

int main(int argc, char** argv)
{
    Config config;

    while (true)
    {
        int option_index = 0;
        int c = getopt_long (argc, argv, "c:d:s:h", g_options, &option_index);
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
            default:
                usage();
        }
    }

    FlightConnector flightConnector;
    flightConnector.loadConfig(config);

    bool res;
    res = flightConnector.init();
    if (!res)
    {
        return 1;
    }

    res = flightConnector.initDevices();
    if (!res)
    {
        return 1;
    }

    auto dataSource = flightConnector.openDefaultDataSource();
    if (dataSource == nullptr)
    {
        return 1;
    }

    res = dataSource->connect();
    if (!res)
    {
        return 1;
    }

    flightConnector.start();
    flightConnector.wait();

    return 0;
}

