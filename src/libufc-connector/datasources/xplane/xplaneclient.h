//
// Created by Ian Parker on 19/01/2024.
//

#ifndef UFC_XPLANECLIENT_H
#define UFC_XPLANECLIENT_H

#include "ufc/utils/data.h"
#include "ufc/flightconnector.h"

#include <memory>

#include "ufc/utils/udp.h"

namespace UFC
{
class XPlaneClient : protected Logger
{
    static std::vector<XPlaneClient*> g_clients;

 public:
    explicit XPlaneClient(std::string name) : Logger(name)
    {
        g_clients.push_back(this);
    }

    ~XPlaneClient() override = default;

    virtual Result connect() = 0;
    virtual void disconnect() = 0;
    [[nodiscard]] virtual bool isConnected() const = 0;

    static void disconnectAll()
    {
        for (const auto client: g_clients)
        {
            client->disconnect();
        }
    }

    virtual Result readString(const std::string &dataref, int len, std::string& value) = 0;
    virtual Result read(const std::string& dataref, float& returnValue) = 0;
    virtual Result readInt(const std::string& dataref, int& value) = 0;
    virtual Result streamDataRefs(const std::vector<std::pair<int, std::string>> &datarefs, const std::function<void(std::map<int, float>)> &, int count = 0) = 0;

    virtual Result sendCommand(const std::string &command) = 0;

    virtual Result setDataRef(const std::string& string, float value) = 0;

    virtual void sendMessage(const std::string& string) = 0;
};
}

#endif
