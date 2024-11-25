
#include "xpplugindatasource.h"
#include "ufcplugin.h"

#include <XPLMProcessing.h>
#include <XPLMMenus.h>
#include <XPLMPlugin.h>

#include "ufc/device.h"

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

    m_menuContainer = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "UFC", 0, 0);
    m_menuId = XPLMCreateMenu("UFC", XPLMFindPluginsMenu(), m_menuContainer, menuCallback, this);
    XPLMAppendMenuItem(m_menuId, "Settings", (void *)-1, 1);
    XPLMAppendMenuSeparator(m_menuId);

    log(DEBUG, "UFC: XPluginStart: Creating Flight Connector...");
    m_flightConnector = new FlightConnector();
    m_flightConnector->disableExitHandler();

    log(DEBUG, "UFC: XPluginStart: Creating Data Source...");
    m_dataSource = make_shared<XPPluginDataSource>(m_flightConnector);
    m_flightConnector->setDataSource(m_dataSource);

    log(DEBUG, "UFC: XPluginStart: Initialising Flight Connector...");
    m_flightConnector->init();

    for (const auto& device : m_flightConnector->getDevices())
    {
        string name = "Device: " + device->getName();
        XPLMAppendMenuItem(m_menuId, name.c_str(), device, 1);
    }

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

void UFCPlugin::receiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam)
{
#if 0
    switch (inMsg)
    {
        case XPLM_MSG_PLANE_LOADED:
        {
            int index = (int)(intptr_t)inParam;
            log(DEBUG, "receiveMessage: XPLM_MSG_PLANE_LOADED: index=%d", index);
            if (index == 0)
            {
                m_dataSource->reloadAircraft();
            }
            break;
        }
        default:
            log(DEBUG, "receiveMessage: Unhandled message: %d", inMsg);
    }
#endif
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

void UFCPlugin::menuCallback(void* menuRef, void* itemRef)
{
    if (menuRef != nullptr)
    {
        ((UFCPlugin*)menuRef)->menu(itemRef);
    }
}

void UFCPlugin::menu(void* itemRef)
{
    log(DEBUG, "menu: itemRef=%p", itemRef);
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
    g_ufcPlugin.receiveMessage(inFrom, inMsg, inParam);
}
