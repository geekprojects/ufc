//
// Created by Ian Parker on 05/01/2025.
//

#include <unistd.h>
#include <cstring>

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include <ufc/udp.h>

using namespace std;
using namespace UFC;

UDPSocket::UDPSocket(const string &host, int port) : Logger("UDPSocket")
{
    m_host = host;
    m_port = port;

    m_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(port);
    m_serverAddr.sin_addr.s_addr = inet_addr(host.c_str());
}

UDPSocket::~UDPSocket()
{
    close();
}

Result UDPSocket::send(void* buffer, int len)
{
    ssize_t result = sendto(m_socket, buffer, len, 0, (const struct sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
    if (result < 0)
    {
        log(ERROR, "UDPSocket::send: Failed to send data: %z", result);
        return Result::FAIL;
    }
    return Result::SUCCESS;
}

Result UDPSocket::receive(std::shared_ptr<Data>& data)
{
    fd_set stReadFDS;
    fd_set stExceptFDS;
    timeval timeout{};

    // Setup for Select
    FD_ZERO(&stReadFDS);
    FD_SET(m_socket, &stReadFDS);
    FD_ZERO(&stExceptFDS);
    FD_SET(m_socket, &stExceptFDS);

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    int res = select(m_socket + 1, &stReadFDS, nullptr, &stExceptFDS, &timeout);
    if (res < 0)
    {
        log(ERROR, "receive: Failed to select");
        return Result::FAIL;
    }
    else if (res == 0)
    {
        log(WARN, "receive: Timeout");
        return Result::TIMEOUT;
    }

    char buffer[4096];
    ssize_t len = ::recv(m_socket, buffer, 4096, 0);
    if (len <= 0)
    {
        log(ERROR, "receive: Failed to read data?");
        return Result::FAIL;
    }

    data = make_shared<Data>(len);
    memcpy(data->getData(), buffer, len);

    return Result::SUCCESS;
}

void UDPSocket::close()
{
    if (m_socket != -1)
    {
        ::close(m_socket);
        m_connected = false;
    }
}

std::shared_ptr<UDPSocket> UDPSocket::connect(const string& host, int port)
{
    auto socket = std::make_shared<UDPSocket>(host, port);

    sockaddr_in receiveAddr = {};
    receiveAddr.sin_family = AF_INET;
    receiveAddr.sin_addr.s_addr = INADDR_ANY;
    receiveAddr.sin_port = 0;

    int res = ::bind(socket->m_socket, (struct sockaddr*)&receiveAddr, sizeof(receiveAddr));
    if (res == -1)
    {
        socket->log(ERROR, "connect: Failed to bind to port %d", port);
        return nullptr;
    }

    struct sockaddr_in name = {};
    socklen_t len = sizeof(name);
    getsockname(socket->m_socket, (struct sockaddr *)&name, &len);
    socket->m_clientPort = name.sin_port;

    // Set socket timeout period for sendUDP to 1 millisecond
    // Without this, playback may become choppy due to process blocking
    // Set socket timeout to 1 millisecond = 1,000 microseconds to make it the same as Windows (0 makes it blocking)
    timeval timeout = {};
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;
    res = setsockopt(socket->m_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    if (res < 0)
    {
        socket->log(ERROR, "Failed to set timeout");
        return nullptr;
    }

    socket->m_connected = true;

    return socket;
}
