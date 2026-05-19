//
// Created by Ian Parker on 06/04/2026.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_XPLANEUDPCLIENT_H
#define UNIVERSALFLIGHTCONNECTOR_XPLANEUDPCLIENT_H

#include "ufc/utils/data.h"
#include "ufc/utils/udp.h"
#include "ufc/flightconnector.h"

#include "xplaneclient.h"

#include <memory>

namespace UFC
{
struct XPlaneAlertMessage
{
    char header[5];
    char message1[240];	// needs to be multiple of 8 for the align to work out perfect for the copy?
    char message2[240];	// needs to be long enough to hold the strings!
    char message3[240];
    char message4[240];
};

class XPlaneUDPClient : public XPlaneClient
{
    std::string m_host;
    int m_port;
    std::shared_ptr<UDPSocket> m_dataSocket;

    [[nodiscard]] std::shared_ptr<UDPSocket> createConnection() const;

    static Result sendRREF(const std::shared_ptr<UDPSocket> &socket, std::vector<std::shared_ptr<DataDefinition>> datarefs, int freq);

    Result streamDataRefsInternal(
        const std::shared_ptr<UDPSocket> &socket,
        const std::vector<std::shared_ptr<DataDefinition>> &datarefs,
        const std::function<void(std::map<int, float>)>& func,
        int count);

 public:
    XPlaneUDPClient();
    XPlaneUDPClient(const std::string &host, int port);

    ~XPlaneUDPClient() override = default;

    Result connect() override;
    void disconnect() override;
    [[nodiscard]] bool isConnected() const override { return m_dataSocket->isConnected(); }

    Result readString(const std::string &dataref, int len, std::string& value) override;
    Result read(const std::string& dataref, float& returnValue) override;
    Result readInt(const std::string& dataref, int& value) override
    {
        float d;
        auto res = read(dataref, d);
        if (res != Result::SUCCESS)
        {
            return res;
        }
        value = (int)d;
        return Result::SUCCESS;
    }

    Result streamDataRefs(
        const std::vector<std::shared_ptr<DataDefinition>> &datarefs,
        const std::function<void(std::map<int, float>)> &,
        int count) override;

    Result sendCommand(const std::string &command) override;

    Result setDataRef(const std::string& string, float value) override;

    void sendMessage(const std::string& string) override;
};
}

#endif //UNIVERSALFLIGHTCONNECTOR_XPLANEUDPCLIENT_H
