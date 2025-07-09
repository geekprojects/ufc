//
// Created by Ian Parker on 19/11/2024.
//

#ifndef UFCPLUGIN_H
#define UFCPLUGIN_H

#include <XPLMMenus.h>
#include <XPLMUtilities.h>

#include <ufc/flightconnector.h>

class XPPluginDataSource;

class XPLogPrinter : public UFC::LogPrinter
{
 public:
    virtual ~XPLogPrinter() = default;

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

class UFCPlugin : public UFC::Logger
{
 private:
    XPLMPluginID m_pluginId;
    std::thread::id m_mainThread;

    sigaction m_prev_sigsegv = {};
    sigaction m_prev_sigabrt = {};
    sigaction m_prev_sigfpe = {};
    sigaction m_prev_sigint = {};
    sigaction m_prev_sigill = {};
    sigaction m_prev_sigterm = {};

    XPLogPrinter m_logPrinter;
    UFC::FlightConnector* m_flightConnector = nullptr;
    std::shared_ptr<XPPluginDataSource> m_dataSource = nullptr;

    int m_menuContainer;
    XPLMMenuID m_menuId;

    static float initCallback(float elapsedMe, float elapsedSim, int counter, void * refcon);
    float init(float elapsedMe, float elapsedSim, int counter);

    static void menuCallback(void* menuRef, void* itemRef);
    void menu(void* itemRef);

    void registerCrashHandler();
    void handlePosixSignal(int sig, siginfo_t *siginfo, void* context);
    static void handlePosixSigCallback(int sig, siginfo_t *siginfo, void *context);
    void handleCrash();
    bool isExecuting();

 public:
    UFCPlugin() : Logger("UFCPlugin") {}

    int start(char* outName, char* outSig, char* outDesc);
    void stop();
    int enable();
    void disable();

    void receiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam);
};

#endif //UFCPLUGIN_H
