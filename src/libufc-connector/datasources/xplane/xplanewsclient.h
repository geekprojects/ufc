//
// Created by Ian Parker on 06/04/2026.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_XPLANEWSCLIENT_H
#define UNIVERSALFLIGHTCONNECTOR_XPLANEWSCLIENT_H

#include <nlohmann/json_fwd.hpp>

#include "xplaneclient.h"
#include "curly.hpp/curly.hpp"

namespace UFC {

class XPlaneWebSocketClient;
typedef void CURL;

struct DataRefWebSocketInfo
{
    CURL* ws;
    std::string buffer;
    XPlaneWebSocketClient* client;
    std::map<int64_t, int64_t> dataRefIdx;
    const std::function<void(std::map<int, float>)>* func;
};

class XPlaneWebSocketClient : public XPlaneClient
{
    curly_hpp::performer m_performer;
    int m_requestId = 1;

    std::string m_baseRestUrl;
    std::string m_baseWebSocketUrl;
    std::map<std::string, int64_t> m_dataRefIds;
    std::map<std::string, int64_t> m_commandIds;
    std::set<DataRefWebSocketInfo*> m_webSockets;

    int64_t getDataRefId(std::string dataref);
    int64_t getCommandId(const std::string& command);

    bool getDataRef(const std::string &dataref, nlohmann::json &valueJson);

    static size_t dataRefCallback(char *b, size_t size, size_t nitems, void *p);
    size_t dataRefValues(std::string body, DataRefWebSocketInfo* info);

 public:
    explicit XPlaneWebSocketClient();
    ~XPlaneWebSocketClient() override = default;

    Result connect() override;

    void disconnect() override;

    [[nodiscard]] bool isConnected() const override;

    Result readString(const std::string &dataref, int len, std::string &value) override;

    Result read(const std::string &dataref, float &returnValue) override;

    Result readInt(const std::string &dataref, int &value) override;

    Result streamDataRefs(
        const std::vector<std::pair<int, std::string>> &datarefs,
        const std::function<void(std::map<int, float>)> &,
        int count) override;

    Result sendCommand(const std::string &command) override;

    Result setDataRef(const std::string &string, float value) override;

    void sendMessage(const std::string &string) override;
};
}

#endif //UNIVERSALFLIGHTCONNECTOR_XPLANEWSCLIENT_H
