#include <string>
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

XPlaneResult XPlaneClient::connect()
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
        return XPlaneResult::FAIL;
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
        return XPlaneResult::FAIL;
    }

    m_connected = true;

    return XPlaneResult::SUCCESS;
}

bool XPlaneClient::disconnect()
{
    m_connected = false;
    stopDataRefs();
    return true;
}

XPlaneResult XPlaneClient::send(void* buffer, int len)
{
    ssize_t result = sendto(m_socket, buffer, len, 0, (const struct sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
    if (result < 0)
    {
        log(ERROR, "XPlaneClient::send: Failed to send data: %z", result);
        return XPlaneResult::FAIL;
    }
    return XPlaneResult::SUCCESS;
}

XPlaneResult XPlaneClient::receive(shared_ptr<Data>& data)
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
        return XPlaneResult::FAIL;
    }
    else if (res == 0)
    {
        log(WARN, "receive: Timeout");
        return XPlaneResult::TIMEOUT;
    }

    char buffer[4096];
    ssize_t len = ::recv(m_socket, buffer, 4096, 0);
    if (len <= 0)
    {
        log(ERROR, "receive: Failed to read data?");
        return XPlaneResult::FAIL;
    }

    data = make_shared<Data>(len);
    memcpy(data->getData(), buffer, len);

    return XPlaneResult::SUCCESS;
}

XPlaneResult XPlaneClient::getPosition(Position& position)
{
    char buffer[8] = "RPOS\001\0";

    auto res = send(buffer, 8);
    if (res != XPlaneResult::SUCCESS)
    {
        return res;
    }

    shared_ptr<Data> data;
    res = receive(data);
    if (res != XPlaneResult::SUCCESS)
    {
        return res;
    }
    if (data->getLength() < sizeof(Position))
    {
        log(ERROR, "getPosition: not received enough data!");
        return XPlaneResult::FAIL;
    }

    // Turn off updates
    buffer[6] = '0';
    res = send(buffer, 8);
    if (res != XPlaneResult::SUCCESS)
    {
        return res;
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

    return XPlaneResult::SUCCESS;
}

XPlaneResult XPlaneClient::sendConnection()
{
    char buffer[32] = "CONN ";
    memcpy(&buffer[5], &m_clientPort, 2);

    return XPlaneResult::SUCCESS;
}

struct dref_struct_in
{
    char header[4] = {};
    uint8_t pad = 0;
    uint32_t dref_freq = 0;
    uint32_t dref_sender_index = 0;	// the index the customer is using to define this dataref
    char dref_string[400] = {};
} __attribute__((packed));

XPlaneResult XPlaneClient::streamDataRefs(
    const vector<pair<int, string>>& datarefs,
    const function<void(map<int, float>)>& callback,
    const int count)
{
    m_currentDataRefs = datarefs;
    auto res = sendRREF(datarefs, 20);
    if (res != XPlaneResult::SUCCESS)
    {
        return res;
    }

    int c = 0;
    XPlaneResult result = XPlaneResult::SUCCESS;
    while (m_connected && (count == 0 || c < count))
    {
        shared_ptr<Data> packet;
        res = receive(packet);
        if (res != XPlaneResult::SUCCESS)
        {
            // Unable to receive data
            result = res;
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
    return result;
}

XPlaneResult XPlaneClient::sendRREF(std::vector<std::pair<int, std::string>> datarefs, int freq)
{
    for (unsigned int i = 0; i < datarefs.size(); i += 100)
    {
        for (unsigned int j = 0; (i + j) < datarefs.size(); j++)
        {
            dref_struct_in drefRequest;

            const auto rref = "RREF";
            memcpy(drefRequest.header, rref, 4);

            const int idx = datarefs[i + j].first;
            string dataref = datarefs[i + j].second;
            drefRequest.dref_freq = freq;
            drefRequest.dref_sender_index = idx;
            strncpy(drefRequest.dref_string, dataref.c_str(), 399);
            drefRequest.dref_string[399] = 0;
            auto res = send(&drefRequest, sizeof(drefRequest));
            if (res != XPlaneResult::SUCCESS)
            {
                return res;
            }
        }
    }
    return XPlaneResult::SUCCESS;
}

void XPlaneClient::stopDataRefs()
{
    sendRREF(m_currentDataRefs, 0);
    m_currentDataRefs.clear();
}

XPlaneResult XPlaneClient::sendCommand(const std::string& command)
{
    const int len = 5 + command.length();
    char buffer[len];
    const auto cmnd = "CMND\00";
    memcpy(buffer, cmnd, 5);
    memcpy(buffer + 5, command.c_str(), command.length());
    return send(buffer, len);
}

XPlaneResult XPlaneClient::setDataRef(const std::string& dataRef, float value)
{
    int len = 5 + 4 + 500;
    char buffer[len];
    memset(buffer, 0, 5 + 4 + 500);
    const auto cmnd = "DREF\00";
    memcpy(buffer, cmnd, 5);
    memcpy(buffer + 5, &value, 4);
    memcpy(buffer + 9, dataRef.c_str(), dataRef.length());
    return send(buffer, len);
}

XPlaneResult XPlaneClient::readString(const std::string& dataref, int len, string& value)
{
    vector<pair<int, string>> datarefs;
    for (int i = 0; i < len; i++)
    {
        datarefs.emplace_back(i, dataref + "[" + to_string(i) + "]");
    }

    char buffer[len + 1];
    memset(buffer, 0, len + 1);
    auto res = streamDataRefs(datarefs, [&buffer, &len](map<int, float> const& values)
    {
        for (const auto& [idx, value] : values)
        {
            if (idx < len)
            {
                buffer[idx] = (char)value;
            }
        }
    }, 1);

    if (res == XPlaneResult::SUCCESS)
    {
        value = string(buffer);
    }

    return res;
}

XPlaneResult XPlaneClient::read(const std::string& dataref, double& returnValue)
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
