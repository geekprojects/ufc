//
// Created by Ian Parker on 19/11/2024.
//

#ifndef UFCPLUGIN_H
#define UFCPLUGIN_H

#include <XPLMMenus.h>
#include <XPLMUtilities.h>

#include <ufc/flightconnector.h>

#include <cstdarg>
#include <csignal>

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
    XPLMPluginID m_pluginId = -1;
    std::thread::id m_mainThread;

    struct sigaction m_prev_sigsegv = {};
    struct sigaction m_prev_sigabrt = {};
    struct sigaction m_prev_sigfpe = {};
    struct sigaction m_prev_sigint = {};
    struct sigaction m_prev_sigill = {};
    struct sigaction m_prev_sigterm = {};

    XPLogPrinter m_logPrinter;
    UFC::FlightConnector* m_flightConnector = nullptr;
    std::shared_ptr<XPPluginDataSource> m_dataSource = nullptr;

    int m_menuContainer = -1;
    XPLMMenuID m_menuId = nullptr;

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
