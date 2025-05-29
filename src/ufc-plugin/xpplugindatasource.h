//
// Created by Ian Parker on 11/11/2024.
//

#ifndef XPPLUGINDATASOURCE_H
#define XPPLUGINDATASOURCE_H

#include <ufc/datasource.h>
#include "../../include/ufc/aircraftmapping.h"

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
 private:
    XPLMDataRef m_icaoDataRef = nullptr;
    XPLMDataRef m_authorDataRef = nullptr;
    XPLMDataRef m_studioDataRef = nullptr;

    std::map<std::string, DataRefInfo> m_dataRefInfoMap;
    std::map<std::string, XPLMCommandRef> m_commandDefs;

    static std::string getString(XPLMDataRef ref);

    std::vector<XPLMCommandRef> m_commandQueue;
    std::mutex m_commandQueueMutex;

 public:
    explicit XPPluginDataSource(UFC::FlightConnector* flightConnector);
    ~XPPluginDataSource() override;

    bool connect() override;

    XPLMCommandRef findOrRegisterCommand(
        std::string const &command);

    void disconnect() override;

    bool reloadAircraft();
    bool update() override;
    bool updateDataRefs();

    void executeCommand(const std::string& command, const CommandDefinition& commandDefinition) override;
    void setData(const std::string& dataName, float value) override;

    bool getDataInt(const std::string& dataName, int& value) override;
    bool getDataFloat(const std::string& dataName, float& value) override;
    bool getDataString(const std::string& dataName, std::string& value) override;
};



#endif //XPPLUGINDATASOURCE_H
