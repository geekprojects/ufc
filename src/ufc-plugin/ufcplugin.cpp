
#include "xpplugindatasource.h"
#include "ufcplugin.h"

#include <signal.h>
#include <XPLMProcessing.h>
#include <XPLMMenus.h>
#include <XPLMPlugin.h>

#include <execinfo.h>
#include <unistd.h>
#include <sys/fcntl.h>

#include "ufc/device.h"

using namespace std;
using namespace UFC;

UFCPlugin g_ufcPlugin;

int UFCPlugin::start(char* outName, char* outSig, char* outDesc)
{
    registerCrashHandler();
    setLogPrinter(&m_logPrinter);

    fprintf(stderr, "UFC: XPluginStart: Here!\n");
    strcpy(outName, "Universal Flight Connector");
    strcpy(outSig, "com.geekprojects.ufc.plugin");
    strcpy(outDesc, "Universal Flight Connector");

    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS",1);

    m_menuContainer = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "UFC", 0, 0);
    m_menuId = XPLMCreateMenu("UFC", XPLMFindPluginsMenu(), m_menuContainer, menuCallback, this);
    XPLMAppendMenuItem(m_menuId, "Settings", (void *)-1, 1);
    XPLMAppendMenuSeparator(m_menuId);

    log(DEBUG, "UFC: XPluginStart: Creating Flight Connector...");
    m_flightConnector = new FlightConnector();
    m_flightConnector->disableExitHandler();

    char xplanePath[512];
    XPLMGetSystemPath(xplanePath);
    log(DEBUG, "XPLM Path: %s", xplanePath);

    char dirSep[2];
    strcpy(dirSep, XPLMGetDirectorySeparator());

    string pluginPath =
        string(xplanePath) +
        "Resources" +
        string(dirSep) +
        "plugins" +
        string(dirSep) +
        "ufc";

    log(DEBUG, "Plugin Path: %s", pluginPath.c_str());
    Config config;
    config.dataDir = pluginPath + "/data";
    config.configPath = pluginPath;
    m_flightConnector->setConfig(config);

    log(DEBUG, "UFC: XPluginStart: Creating Data Source...");
    m_dataSource = make_shared<XPPluginDataSource>(m_flightConnector);
    m_flightConnector->setDataSource(m_dataSource);

    log(DEBUG, "UFC: XPluginStart: Initialising Flight Connector...");
    m_flightConnector->init();
    m_flightConnector->initDevices();

    for (const auto& device : m_flightConnector->getDevices())
    {
        string name = "Device: " + device->getName();
        XPLMAppendMenuItem(m_menuId, name.c_str(), device, 1);
    }
    XPLMAppendMenuSeparator(m_menuId);
    XPLMAppendMenuItem(m_menuId, "Detect devices...", (void*)-1, 1);

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

void UFCPlugin::handlePosixSigCallback(int sig, siginfo_t *siginfo, void *context)
{
    g_ufcPlugin.handlePosixSignal(sig, siginfo, context);
}

void UFCPlugin::handleCrash()
{
#if APL || LIN
    // NOTE: This is definitely NOT production code
    // backtrace and backtrace_symbols are NOT signal handler safe and are just put in here for demonstration purposes
    // A better alternative would be to use something like libunwind here

    void *frames[64];
    int frame_count = backtrace(frames, 64);
    char **names = backtrace_symbols(frames, frame_count);

    const int fd = open("backtrace.txt", O_CREAT | O_RDWR | O_TRUNC | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd >= 0)
    {
        for(int i = 0; i < frame_count; ++ i)
        {
            write(fd, names[i], strlen(names[i]));
            write(fd, "\n", 1);
        }

        close(fd);
    }

#endif
#if IBM
    // Create a mini-dump file that can be later opened up in Visual Studio or WinDbg to do post mortem debugging
    write_mini_dump((PEXCEPTION_POINTERS)context);
#endif
}

void UFCPlugin::registerCrashHandler()
{
    m_mainThread = this_thread::get_id();
    m_pluginId = XPLMGetMyID();

#if APL || LIN
    struct sigaction sig_action = { .sa_sigaction = handlePosixSigCallback };

    sigemptyset(&sig_action.sa_mask);

#if	LIN
    static uint8_t alternate_stack[SIGSTKSZ];
    stack_t ss = {
        .ss_sp = (void*)alternate_stack,
        .ss_size = SIGSTKSZ,
        .ss_flags = 0
    };

    sigaltstack(&ss, NULL);
    sig_action.sa_flags = SA_SIGINFO | SA_ONSTACK;
#else
    sig_action.sa_flags = SA_SIGINFO;
#endif

    sigaction(SIGSEGV, &sig_action, &m_prev_sigsegv);
    sigaction(SIGABRT, &sig_action, &m_prev_sigabrt);
    sigaction(SIGFPE, &sig_action, &m_prev_sigfpe);
    sigaction(SIGINT, &sig_action, &m_prev_sigint);
    sigaction(SIGILL, &sig_action, &m_prev_sigill);
    sigaction(SIGTERM, &sig_action, &m_prev_sigterm);
#endif

#if IBM
    // Load the debug helper library into the process already, this way we don't have to hit the dynamic loader
    // in an exception context where it's potentially unsafe to do so.
    HMODULE module = ::GetModuleHandleA("dbghelp.dll");
    if(!module)
        module = ::LoadLibraryA("dbghelp.dll");

    (void)module;
    s_previous_windows_exception_handler = SetUnhandledExceptionFilter(handle_windows_exception);
#endif
}

void UFCPlugin::handlePosixSignal(int sig, siginfo_t* siginfo, void* context)
{
    if(isExecuting())
    {
        static bool has_called_out = false;

        if(!has_called_out)
        {
            has_called_out = true;
            handleCrash();
        }

        abort();
    }

    // Forward the signal to the other handlers
#define	FORWARD_SIGNAL(sigact) \
    do { \
        if((sigact)->sa_sigaction && ((sigact)->sa_flags & SA_SIGINFO)) \
            (sigact)->sa_sigaction(sig, siginfo, context); \
        else if((sigact)->sa_handler) \
            (sigact)->sa_handler(sig); \
    } while (0)

    switch(sig)
    {
        case SIGSEGV:
            FORWARD_SIGNAL(&m_prev_sigsegv);
            break;
        case SIGABRT:
            FORWARD_SIGNAL(&m_prev_sigabrt);
            break;
        case SIGFPE:
            FORWARD_SIGNAL(&m_prev_sigfpe);
            break;
        case SIGILL:
            FORWARD_SIGNAL(&m_prev_sigill);
            break;
        case SIGTERM:
            FORWARD_SIGNAL(&m_prev_sigterm);
            break;
    }
#undef FORWARD_SIGNAL

    abort();
}

bool UFCPlugin::isExecuting()
{
    const std::thread::id thread_id = std::this_thread::get_id();
    if (m_mainThread == thread_id)
    {
        return (m_pluginId == XPLMGetMyID());
    }

    return m_flightConnector->isOurThread();
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
