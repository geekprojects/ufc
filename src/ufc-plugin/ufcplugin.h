//
// Created by Ian Parker on 19/11/2024.
//

#ifndef UFCPLUGIN_H
#define UFCPLUGIN_H

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
    XPLogPrinter m_logPrinter;
    UFC::FlightConnector* m_flightConnector = nullptr;
    std::shared_ptr<XPPluginDataSource> m_dataSource = nullptr;

    static float initCallback(float elapsedMe, float elapsedSim, int counter, void * refcon);
    float init(float elapsedMe, float elapsedSim, int counter);

 public:
    UFCPlugin() : Logger("UFCPlugin") {}

    int start(char* outName, char* outSig, char* outDesc);
    void stop();
    int enable();
    void disable();
};

#endif //UFCPLUGIN_H
