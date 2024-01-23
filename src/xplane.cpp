#include <string>
#include <cstdio>
#include <cstdlib>
#include <csignal>

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "xplane.h"

#include <geek/core-data.h>

using namespace std;
using namespace Geek;

std::vector<XPlaneClient*> XPlaneClient::g_clients;

static void exitHandler(int signal)
{
    printf("exitHandler: signal=%d\n", signal);
    XPlaneClient::disconnectAll();
}

void XPlaneClient::disconnectAll()
{
    for (auto client : g_clients)
    {
        client->disconnect();
    }
}

XPlaneClient::XPlaneClient() :
    m_port(49000)
{
    m_host = "127.0.0.1";
    g_clients.push_back(this);
}

XPlaneClient::~XPlaneClient() = default;

bool XPlaneClient::connect()
{
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(m_port);
    m_serverAddr.sin_addr.s_addr = inet_addr(m_host.c_str());

    sockaddr_in recvaddr = {};
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_addr.s_addr = INADDR_ANY;
    recvaddr.sin_port = 0;

    int res = ::bind(m_socket, (struct sockaddr*)&recvaddr, sizeof(recvaddr));
    if (res == -1)
    {
        printf("XPlaneClient::connect: Failed to bind\n");
        return false;
    }

    struct sockaddr_in name = {};
    socklen_t len = sizeof(name);
    getsockname(m_socket, (struct sockaddr *)&name, &len);
    m_clientPort = name.sin_port;

    // Set socket timeout period for sendUDP to 1 millisecond
    // Without this, playback may become choppy due to process blocking
    // Set socket timeout to 1 millisecond = 1,000 microseconds to make it the same as Windows (0 makes it blocking)
    timeval timeout = {};
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;
    res = setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    if (res < 0)
    {
        printf("Failed to set timeout\n");
        return false;
    }

    struct sigaction sigIntHandler = {};
    sigIntHandler.sa_handler = exitHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, nullptr);
    sigaction(SIGTERM, &sigIntHandler, nullptr);

    m_connected = true;

    return true;
}

bool XPlaneClient::disconnect()
{
    m_connected = false;
    stopDataRefs();
    return true;
}

bool XPlaneClient::send(void* buffer, int len) const
{
    ssize_t result = sendto(m_socket, buffer, len, 0, (const struct sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
    if (result < 0)
    {
        printf("XPlaneClient::send: Failed to send data: %zd\n", result);
        return false;
    }
    return true;
}

shared_ptr<Geek::Data> XPlaneClient::receive() const
{
    fd_set stReadFDS;
    fd_set stExceptFDS;
    timeval timeout{};

    // Setup for Select
    FD_ZERO(&stReadFDS);
    FD_SET(m_socket, &stReadFDS);
    FD_ZERO(&stExceptFDS);
    FD_SET(m_socket, &stExceptFDS);

    // 50 milliseconds
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    int res = select(m_socket + 1, &stReadFDS, nullptr, &stExceptFDS, &timeout);
    if (res < 0)
    {
        printf("XPlaneClient::receive: Failed to select\n");
        return nullptr;
    }
    else if (res == 0)
    {
        return make_shared<Data>();
    }

    char buffer[4096];
    res = recv(m_socket, buffer, 4096, 0);
    if (res <= 0)
    {
        printf("XPlaneClient::receive: Failed to read data?\n");
        return nullptr;
    }

    auto data = make_shared<Geek::Data>(res);
    memcpy(data->getData(), buffer, res);

    return data;
}

bool XPlaneClient::getPosition(Position& position)
{
    char buffer[8] = "RPOS\001\0";

    bool res = send(buffer, 8);
    if (!res)
    {
        return false;
    }

    shared_ptr<Geek::Data> data = receive();
    if (!data)
    {
        return false;
    }
    if (data->getLength() < sizeof(Position))
    {
        printf("XPlaneClient::getPosition: not received enough data!\n");
        return false;
    }

    // Turn off updates
    buffer[6] = '0';
    res = send(buffer, 8);
    if (!res)
    {
        return false;
    }

    data->readString(4);
    data->skip(1);

    //Lat, Lon, Alt, Pitch, Roll, Yaw, Gear
    position.dat_lon = data->readDouble();
    position.dat_lat = data->readDouble();
    position.dat_ele = data->readDouble();
    position.y_agl_mtr = data->readFloat();
    position.veh_the_loc = data->readFloat();
    position.veh_psi_loc = data->readFloat();
    position.veh_phi_loc = data->readFloat();
    position.vx_wrl = data->readFloat();
    position.vy_wrl = data->readFloat();
    position.vz_wrl = data->readFloat();
    position.Prad = data->readFloat();
    position.Qrad = data->readFloat();
    position.Rrad = data->readFloat();
    return true;
}

bool XPlaneClient::sendConnection()
{
    char buffer[32] = "CONN ";
    memcpy(&buffer[5], &m_clientPort, 2);

    return false;
}

struct dref_struct_in
{
    char header[4];
    uint8_t pad = 0;
    uint32_t dref_freq;
    uint32_t dref_sender_index;	// the index the customer is using to define this dataref
    char dref_string[400];
} __attribute__((packed));

bool XPlaneClient::streamDataRefs(vector<pair<int, string>> datarefs, function<void(map<int, float>)> callback)
{
    bool res;
    m_currentDataRefs = datarefs;
    res = sendRREF(datarefs, 20);
    if (!res)
    {
        return false;
    }

    while (m_connected)
    {
        auto packet = receive();
        if (!packet)
        {
            printf("XXX: No data?\n");
            break;
        }
        if (packet->getLength() == 0)
        {
            continue;
        }
        uint32_t header = packet->read32();
        if (header != 0x46455252)
        {
            printf("XPlaneClient::streamDataRefs: Not an RREF packet?\n");
            continue;
        }
        packet->read8(); // null
        map<int, float> values;
        while (!packet->eof())
        {
            int idx = packet->read32();
            float value = packet->readFloat();
            values.insert(make_pair(idx, value));
        }
        callback(values);
    }
    stopDataRefs();
    return true;
}

bool XPlaneClient::sendRREF(std::vector<std::pair<int, std::string>> datarefs, int freq)
{
    for (int i = 0; i < datarefs.size(); i += 100)
    {
        for (int j = 0; (i + j) < datarefs.size(); j++)
        {
            dref_struct_in drefRequest;

            memcpy(drefRequest.header, "RREF", 4);

            int idx = datarefs[i + j].first;
            string dataref = datarefs[i + j].second;
            printf("streamDataRefs: %d,%d: %d: %s\n", i, j, idx, dataref.c_str());
            drefRequest.dref_freq = freq;
            drefRequest.dref_sender_index = idx;
            strncpy(drefRequest.dref_string, dataref.c_str(), 400);
            bool res = send(&drefRequest, sizeof(drefRequest));
            if (!res)
            {
                return false;
            }
        }
    }
    return true;
}

void XPlaneClient::stopDataRefs()
{
    printf("XXX: stopDataRefs: Stopping data...\n");
    sendRREF(m_currentDataRefs, 0);
}
