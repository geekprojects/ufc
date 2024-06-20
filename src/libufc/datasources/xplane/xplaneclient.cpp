#include <string>
#include <cstdio>
#include <csignal>
#include <cstring>

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "xplaneclient.h"

#include <ufc/data.h>

using namespace std;
using namespace UFC;

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
    Logger("XPlaneClient"),
    m_port(49000)
{
    m_host = "127.0.0.1";
    g_clients.push_back(this);
}

XPlaneClient::XPlaneClient(const string &host, const int port) :
    Logger("XPlaneClient"),
    m_port(port)
{
    m_host = host;
    g_clients.push_back(this);
}

XPlaneClient::~XPlaneClient() = default;

bool XPlaneClient::connect()
{
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(m_port);
    m_serverAddr.sin_addr.s_addr = inet_addr(m_host.c_str());

    sockaddr_in receiveAddr = {};
    receiveAddr.sin_family = AF_INET;
    receiveAddr.sin_addr.s_addr = INADDR_ANY;
    receiveAddr.sin_port = 0;

    int res = ::bind(m_socket, (struct sockaddr*)&receiveAddr, sizeof(receiveAddr));
    if (res == -1)
    {
        log(ERROR, "connect: Failed to bind to port %d", m_port);
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
        log(ERROR, "Failed to set timeout");
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

bool XPlaneClient::send(void* buffer, int len)
{
    ssize_t result = sendto(m_socket, buffer, len, 0, (const struct sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
    if (result < 0)
    {
        log(ERROR, "XPlaneClient::send: Failed to send data: %z", result);
        return false;
    }
    return true;
}

shared_ptr<Data> XPlaneClient::receive()
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
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    int res = select(m_socket + 1, &stReadFDS, nullptr, &stExceptFDS, &timeout);
    if (res < 0)
    {
        log(ERROR, "receive: Failed to select");
        return nullptr;
    }
    else if (res == 0)
    {
        log(WARN, "receive: Timeout");
        return nullptr;
    }

    char buffer[4096];
    ssize_t len = ::recv(m_socket, buffer, 4096, 0);
    if (len <= 0)
    {
        log(ERROR, "receive: Failed to read data?");
        return nullptr;
    }

    auto data = make_shared<Data>(len);
    memcpy(data->getData(), buffer, len);

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

    shared_ptr<Data> data = receive();
    if (!data)
    {
        return false;
    }
    if (data->getLength() < sizeof(Position))
    {
        log(ERROR, "getPosition: not received enough data!");
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
    char header[4] = {};
    uint8_t pad = 0;
    uint32_t dref_freq = 0;
    uint32_t dref_sender_index = 0;	// the index the customer is using to define this dataref
    char dref_string[400] = {};
} __attribute__((packed));

bool XPlaneClient::streamDataRefs(
    const vector<pair<int, string>>& datarefs,
    const function<void(map<int, float>)>& callback,
    const int count)
{
    m_currentDataRefs = datarefs;
    bool res = sendRREF(datarefs, 20);
    if (!res)
    {
        return false;
    }

    int c = 0;
    while (m_connected && (count == 0 || c < count))
    {
        auto packet = receive();
        if (!packet)
        {
            // Unable to receive data
            break;
        }
        if (packet->getLength() == 0)
        {
            continue;
        }
        uint32_t header = packet->read32();
        if (header != 0x46455252)
        {
            log(ERROR, "streamDataRefs: Not an RREF packet?");
            continue;
        }
        packet->read8(); // null
        map<int, float> values;
        while (!packet->eof())
        {
            int idx = (int)packet->read32();
            float value = packet->readFloat();
            values.insert(make_pair(idx, value));
        }
        callback(values);
        c++;
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

            const auto rref = "RREF";
            memcpy(drefRequest.header, rref, 4);

            const int idx = datarefs[i + j].first;
            string dataref = datarefs[i + j].second;
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
    sendRREF(m_currentDataRefs, 0);
    m_currentDataRefs.clear();
}

void XPlaneClient::sendCommand(const std::string& command)
{
    const int len = 5 + command.length();
    char buffer[len];
    const auto cmnd = "CMND\00";
    memcpy(buffer, cmnd, 5);
    memcpy(buffer + 5, command.c_str(), command.length());
    send(buffer, len);
}

bool XPlaneClient::readString(const std::string& dataref, int len, string& value)
{
    vector<pair<int, string>> datarefs;
    for (int i = 0; i < len; i++)
    {
        datarefs.emplace_back(i, dataref + "[" + to_string(i) + "]");
    }

    char buffer[len + 1];
    memset(buffer, 0, len + 1);
    streamDataRefs(datarefs, [&buffer, &len](map<int, float> const& values)
    {
        for (const auto& [idx, value] : values)
        {
            if (idx < len)
            {
                buffer[idx] = (char)value;
            }
        }
    }, 1);

    value = string(buffer);

    return true;
}

bool XPlaneClient::read(const std::string& dataref, double& returnValue)
{
    vector<pair<int, string>> datarefs;
    datarefs.emplace_back(0, dataref);

    return streamDataRefs(datarefs, [&returnValue](map<int, float> const& values)
    {
        for (const auto& [idx, value] : values)
        {
            returnValue = value;
        }
    }, 1);
}
