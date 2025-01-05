//
// Created by Ian Parker on 05/01/2025.
//

#ifndef UFC_UDP_H
#define UFC_UDP_H

#include <string>
#include <netinet/in.h>

#include <ufc/data.h>
#include <ufc/logger.h>
#include <ufc/flightconnector.h>

namespace UFC
{
class UDPSocket : public UFC::Logger
{
    std::string m_host;
    int m_port = -1;
    int m_socket = -1;
    sockaddr_in m_serverAddr = {};
    int m_clientPort = -1;
    bool m_connected = false;

public:
    static std::shared_ptr<UDPSocket> connect(const std::string& host, int port);

    UDPSocket(const std::string& host, int port);
    ~UDPSocket() override;

    [[nodiscard]] bool isConnected() const
    {
        return m_connected;
    }

    Result send(void* buffer, int len);
    Result receive(std::shared_ptr<Data>& data);

    void close();

};
}

#endif // UFC_UDP_H
