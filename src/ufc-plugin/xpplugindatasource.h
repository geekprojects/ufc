//
// Created by Ian Parker on 11/11/2024.
//

#ifndef XPPLUGINDATASOURCE_H
#define XPPLUGINDATASOURCE_H

#include <ufc/datasource.h>
#include "../libufc/datasources/xplane/xpmapping.h"

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
    XPMapping m_dataMapping;

    XPLMDataRef m_icaoDataRef;
    XPLMDataRef m_authorDataRef;
    XPLMDataRef m_studioDataRef;

    std::map<std::string, DataRefInfo> m_commandDataRefs;

    std::string getString(XPLMDataRef ref);

    std::vector<XPLMCommandRef> m_commandQueue;
    std::mutex m_commandQueueMutex;

 public:
    XPPluginDataSource(UFC::FlightConnector* flightConnector);
    ~XPPluginDataSource() override;

    bool connect() override;

    void disconnect() override;

    bool update() override;
    bool updateDataRefs();

    void command(std::string command) override;
};



#endif //XPPLUGINDATASOURCE_H
