//
// Created by Ian Parker on 11/11/2024.
//

#ifndef XPPLUGINDATASOURCE_H
#define XPPLUGINDATASOURCE_H

#include <ufc/datasource.h>
#include <ufc/aircraftmapping.h>

#define XPLM200 1
#define XPLM210 1
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>

struct DataRefInfo
{
    XPLMDataRef dataRef = nullptr;
    XPLMDataTypeID types = 0;
};

class XPPluginDataSource : public UFC::DataSource
{
    XPLMDataRef m_icaoDataRef = nullptr;
    XPLMDataRef m_authorDataRef = nullptr;
    XPLMDataRef m_studioDataRef = nullptr;

    std::map<std::string, DataRefInfo> m_dataRefInfoMap;
    std::map<std::string, XPLMCommandRef> m_commandDefs;

    std::string getString(XPLMDataRef ref);

    std::vector<XPLMCommandRef> m_commandQueue;
    std::mutex m_commandQueueMutex;

    std::map<std::string, float> m_dataQueue;
    std::mutex m_dataQueueMutex;

    bool m_checkAircraft = true;

    static float updateCallback(float elapsedMe, float elapsedSim, int counter, XPPluginDataSource* refcon);
    bool updateDataRefs();
    void executeCommands();
    void updateValues();
    void executeSetData(const std::string& dataName, float value);

    void checkReload();

 public:
    explicit XPPluginDataSource(UFC::FlightConnector* flightConnector);
    ~XPPluginDataSource() override;

    bool connect() override;

    XPLMCommandRef findOrRegisterCommand(
        std::string const &command);

    void disconnect() override;

    bool reloadAircraft();
    bool update() override;

    void executeCommand(const std::string& command, const CommandDefinition& commandDefinition) override;
    void setData(const std::string& dataName, float value) override;
};

#endif //XPPLUGINDATASOURCE_H
