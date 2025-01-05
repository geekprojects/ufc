#include <string>
#include <cstring>

#include <arpa/inet.h>

#include "xplaneclient.h"

#include <ufc/data.h>

using namespace std;
using namespace UFC;

std::vector<XPlaneClient*> XPlaneClient::g_clients;

void XPlaneClient::disconnectAll()
{
    for (const auto client : g_clients)
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
    m_host(host),
    m_port(port)
{
    g_clients.push_back(this);
}

XPlaneClient::~XPlaneClient() = default;

Result XPlaneClient::connect()
{
    m_dataSocket = createConnection();
    if (m_dataSocket == nullptr)
    {
        return Result::FAIL;
    }

    return Result::SUCCESS;
}

std::shared_ptr<UDPSocket> XPlaneClient::createConnection() const
{
    return UDPSocket::connect(m_host, m_port);
}

void XPlaneClient::disconnect() const
{
    if (m_dataSocket != nullptr)
    {
        m_dataSocket->close();
    }
}

Result XPlaneClient::getPosition(Position& position)
{
    char buffer[8] = "RPOS\001\0";

    auto res = m_dataSocket->send(buffer, 8);
    if (res != Result::SUCCESS)
    {
        return res;
    }

    shared_ptr<Data> data;
    res = m_dataSocket->receive(data);
    if (res != Result::SUCCESS)
    {
        return res;
    }
    if (data->getLength() < sizeof(Position))
    {
        log(ERROR, "getPosition: not received enough data!");
        return Result::FAIL;
    }

    // Turn off updates
    buffer[6] = '0';
    res = m_dataSocket->send(buffer, 8);
    if (res != Result::SUCCESS)
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

    return Result::SUCCESS;
}

struct dref_struct_in
{
    char header[4] = {};
    uint8_t pad = 0;
    uint32_t dref_freq = 0;
    uint32_t dref_sender_index = 0;	// the index the customer is using to define this dataref
    char dref_string[400] = {};
} __attribute__((packed));

Result XPlaneClient::streamDataRefs(
    const std::vector<std::pair<int, std::string>>& datarefs,
    const std::function<void(std::map<int, float>)>& callback,
    int count)
{
    auto streamSocket = createConnection();
    if (streamSocket == nullptr)
    {
        return Result::FAIL;
    }

    return streamDataRefs(streamSocket, datarefs, callback, count);
}

Result XPlaneClient::streamDataRefs(
    std::shared_ptr<UDPSocket> socket,
    const vector<pair<int, string>>& datarefs,
    const function<void(map<int, float>)>& callback,
    const int count)
{
    auto res = sendRREF(socket, datarefs, 20);
    if (res != Result::SUCCESS)
    {
        return res;
    }

    int c = 0;
    Result result = Result::SUCCESS;
    while (m_dataSocket->isConnected() && socket->isConnected() && (count == 0 || c < count))
    {
        shared_ptr<Data> packet;
        res = socket->receive(packet);
        if (res != Result::SUCCESS)
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

    sendRREF(socket, datarefs, 0);

    return result;
}

Result XPlaneClient::sendRREF(const std::shared_ptr<UDPSocket> &socket, std::vector<std::pair<int, std::string>> datarefs, int freq)
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
            auto res = socket->send(&drefRequest, sizeof(drefRequest));
            if (res != Result::SUCCESS)
            {
                return res;
            }
        }
    }
    return Result::SUCCESS;
}

Result XPlaneClient::sendCommand(const std::string& command)
{
    const int len = 5 + command.length();
    char buffer[len];
    const auto cmnd = "CMND\00";
    memcpy(buffer, cmnd, 5);
    memcpy(buffer + 5, command.c_str(), command.length());
    return m_dataSocket->send(buffer, len);
}

Result XPlaneClient::setDataRef(const std::string& dataRef, float value)
{
    int len = 5 + 4 + 500;
    char buffer[len];
    memset(buffer, 0, 5 + 4 + 500);
    const auto cmnd = "DREF\00";
    memcpy(buffer, cmnd, 5);
    memcpy(buffer + 5, &value, 4);
    memcpy(buffer + 9, dataRef.c_str(), dataRef.length());
    return m_dataSocket->send(buffer, len);
}

#define READ_OFFSET 10000

Result XPlaneClient::readString(const std::string& dataref, int len, string& value)
{
    vector<pair<int, string>> datarefs;
    for (int i = 0; i < len; i++)
    {
        datarefs.emplace_back(i + READ_OFFSET, dataref + "[" + to_string(i) + "]");
    }

    char buffer[len + 1];
    memset(buffer, 0, len + 1);
    auto res = streamDataRefs(m_dataSocket, datarefs, [&buffer, &len](map<int, float> const& values)
    {
        for (const auto& [idx, v] : values)
        {
            if (idx < READ_OFFSET)
            {
                continue;
            }
            int i = idx - READ_OFFSET;
            if (i < len)
            {
                buffer[i] = (char)v;
            }
        }
    }, 1);

    if (res == Result::SUCCESS)
    {
        value = string(buffer);
    }

    return res;
}

Result XPlaneClient::read(const std::string& dataref, float& returnValue)
{
    vector<pair<int, string>> datarefs;
    datarefs.emplace_back(0, dataref);

    return streamDataRefs(m_dataSocket, datarefs, [&returnValue](map<int, float> const& values)
    {
        for (const auto& [idx, value] : values)
        {
            returnValue = value;
        }
    }, 1);
}


void XPlaneClient::sendMessage(const std::string& string)
{
    XPlaneAlertMessage message;

    const auto alrt = "ALRT";
    memcpy(message.header, alrt, 4);
    message.header[4] = 0;
    strcpy(message.message1, string.c_str());
    m_dataSocket->send(&message, sizeof(message));
}

