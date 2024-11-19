
#include "xpplugindatasource.h"
#include "ufcplugin.h"

#include <XPLMProcessing.h>

using namespace std;
using namespace UFC;

UFCPlugin g_ufcPlugin;

int UFCPlugin::start(char* outName, char* outSig, char* outDesc)
{
    setLogPrinter(&m_logPrinter);

    fprintf(stderr, "UFC: XPluginStart: Here!\n");
    strcpy(outName, "Universal Flight Connector");
    strcpy(outSig, "com.geekprojects.ufc.plugin");
    strcpy(outDesc, "Universal Flight Connector");

    log(DEBUG, "UFC: XPluginStart: Creating Flight Connector...");
    m_flightConnector = new FlightConnector();
    m_flightConnector->disableExitHandler();

    log(DEBUG, "UFC: XPluginStart: Creating Data Source...");
    m_dataSource = make_shared<XPPluginDataSource>(m_flightConnector);
    m_flightConnector->setDataSource(m_dataSource);

    log(DEBUG, "UFC: XPluginStart: Initialising Flight Connector...");
    m_flightConnector->init();

    if (!m_flightConnector->getDevices().empty())
    {
        XPLMRegisterFlightLoopCallback(initCallback, -1, this);
    }
    else
    {
        log(WARN, "UFC: No devices found. Not Starting.");
    }

    return 1;
}

void UFCPlugin::stop()
{
    if (m_flightConnector != nullptr)
    {
        m_flightConnector->stop();

        delete m_flightConnector;
        m_flightConnector = nullptr;
    }
}

int UFCPlugin::enable()
{
    return 1;
}

void UFCPlugin::disable()
{
}

float UFCPlugin::initCallback(float elapsedMe, float elapsedSim, int counter, void * refcon)
{
    return ((UFCPlugin*)refcon)->init(elapsedMe, elapsedSim, counter);
}

float UFCPlugin::init(float elapsedMe, float elapsedSim, int counter)
{
    m_dataSource->connect();
    m_flightConnector->start();
    return 0;
}

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
    return g_ufcPlugin.start(outName, outSig, outDesc);
}

PLUGIN_API void	XPluginStop()
{
    g_ufcPlugin.stop();
}

PLUGIN_API int XPluginEnable()
{
    return g_ufcPlugin.enable();
}

PLUGIN_API void XPluginDisable()
{
    g_ufcPlugin.disable();
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam)
{
}
