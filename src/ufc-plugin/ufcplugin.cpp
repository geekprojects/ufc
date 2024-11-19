
#include <XPLMDefs.h>
#include <XPLMProcessing.h>

#include <ufc/flightconnector.h>

#include "xpplugindatasource.h"

using namespace std;
using namespace UFC;

FlightConnector* g_flightConnector = nullptr;
shared_ptr<XPPluginDataSource> g_dataSource;

float deferredInit(float elapsedMe, float elapsedSim, int counter, void * refcon);

class XPLogPrinter : public LogPrinter
{
 public:
    void printf(const char* message, ...) override
    {
        va_list va;
        va_start(va, message);

        char buf[4096];
        vsnprintf(buf, 4094, message, va);
        XPLMDebugString(buf);

        va_end(va);
    }
};

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
    XPLogPrinter* logPrinter = new XPLogPrinter();
    Logger::setLogPrinter(logPrinter);

    fprintf(stderr, "UFC: XPluginStart: Here!\n");
    strcpy(outName, "Universal Flight Connector");
    strcpy(outSig, "com.geekprojects.ufc.plugin");
    strcpy(outDesc, "Universal Flight Connector");

    logPrinter->printf("UFC: XPluginStart: Creating Flight Connector...\n");
    g_flightConnector = new FlightConnector();
    g_flightConnector->disableExitHandler();

    logPrinter->printf("UFC: XPluginStart: Creating Data Source...\n");
    g_dataSource = make_shared<XPPluginDataSource>(g_flightConnector);
    g_flightConnector->setDataSource(g_dataSource);

    logPrinter->printf("UFC: XPluginStart: Initialising Flight Connector...\n");
    g_flightConnector->init();

    if (!g_flightConnector->getDevices().empty())
    {
        XPLMRegisterFlightLoopCallback(deferredInit, -1, nullptr);
    }
    else
    {
        logPrinter->printf("UFC: No devices found. Not Starting.");
    }

    return 1;
}

PLUGIN_API void	XPluginStop(void)
{
    if (g_flightConnector != nullptr)
    {
        g_flightConnector->stop();
    }
}

PLUGIN_API void XPluginDisable(void)
{

}

PLUGIN_API int  XPluginEnable(void)
{
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam)
{
}

float deferredInit(float elapsedMe, float elapsedSim, int counter, void * refcon)
{
    g_dataSource->connect();
    g_flightConnector->start();
    return 0;
}
