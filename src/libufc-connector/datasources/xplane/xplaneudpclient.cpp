#include <string>
#include <cstring>

#include <arpa/inet.h>

#include "xplaneudpclient.h"

#include "ufc/utils/data.h"

using namespace std;
using namespace UFC;

XPlaneUDPClient::XPlaneUDPClient() :
    XPlaneClient("XPlaneUDPClient"),
    m_port(49000)
{
    m_host = "127.0.0.1";
}

XPlaneUDPClient::XPlaneUDPClient(const string &host, const int port) :
    XPlaneClient("XPlaneUDPClient"),
    m_host(host),
    m_port(port)
{
}

Result XPlaneUDPClient::connect()
{
    m_dataSocket = createConnection();
    if (m_dataSocket == nullptr)
    {
        return Result::FAIL;
    }

    return Result::SUCCESS;
}

shared_ptr<UDPSocket> XPlaneUDPClient::createConnection() const
{
    return UDPSocket::connect(m_host, m_port);
}

void XPlaneUDPClient::disconnect()
{
    if (m_dataSocket != nullptr)
    {
        m_dataSocket->close();
    }
}

struct dref_struct_in
{
    char header[4] = {};
    [[maybe_unused]] uint8_t pad = 0;
    uint32_t dref_freq = 0;
    uint32_t dref_sender_index = 0;	// the index the customer is using to define this dataref
    char dref_string[400] = {};
} __attribute__((packed));

Result XPlaneUDPClient::streamDataRefs(
    const vector<shared_ptr<DataDefinition>>& datarefs,
    const function<void(map<int, float>)>& callback,
    int count)
{
    auto streamSocket = createConnection();
    if (streamSocket == nullptr)
    {
        return Result::FAIL;
    }

    return streamDataRefsInternal(streamSocket, datarefs, callback, count);
}

Result XPlaneUDPClient::streamDataRefsInternal(
    const shared_ptr<UDPSocket>& socket,
    const vector<shared_ptr<DataDefinition>>& datarefs,
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
            auto idx = static_cast<int>(packet->read32());
            float value = packet->readFloat();
            values.insert(make_pair(idx, value));
        }
        callback(values);
        c++;
    }

    sendRREF(socket, datarefs, 0);

    return result;
}

Result XPlaneUDPClient::sendRREF(
    const shared_ptr<UDPSocket> &socket,
    vector<shared_ptr<DataDefinition>> datarefs,
    int freq)
{
    for (unsigned int i = 0; i < datarefs.size(); i += 100)
    {
        for (unsigned int j = 0; (i + j) < datarefs.size(); j++)
        {
            dref_struct_in drefRequest;

            const auto rref = "RREF";
            memcpy(drefRequest.header, rref, 4);

            const auto& dataref = datarefs[i + j];
            drefRequest.dref_freq = freq;
            drefRequest.dref_sender_index = dataref->idx;
            string dataRefName = dataref->mapping.dataRef;
            if (dataref->mapping.dataRefIndex != -1)
            {
                dataRefName += "[" + to_string(dataref->mapping.dataRefIndex) + "]";
            }
            strncpy(drefRequest.dref_string, dataRefName.c_str(), 399);
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

Result XPlaneUDPClient::sendCommand(const string& command)
{
    const auto len = command.length() + 5;
    char buffer[len];
    const auto cmnd = "CMND\00";
    memcpy(buffer, cmnd, 5);
    memcpy(buffer + 5, command.c_str(), command.length());
    return m_dataSocket->send(buffer, len);
}

Result XPlaneUDPClient::setDataRef(const string& dataRef, float value)
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

Result XPlaneUDPClient::readString(const string& dataref, int len, string& value)
{
    vector<shared_ptr<DataDefinition>> datarefs;
    for (int i = 0; i < len; i++)
    {
        auto datadef = make_shared<DataDefinition>();
        datadef->idx = i + READ_OFFSET;
        datadef->mapping.dataRef = dataref + "[" + to_string(i) + "]";
        datarefs.push_back(datadef);
    }

    char buffer[len + 1];
    memset(buffer, 0, len + 1);
    auto res = streamDataRefsInternal(m_dataSocket, datarefs, [&buffer, &len](map<int, float> const& values)
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

Result XPlaneUDPClient::read(const string& dataref, float& returnValue)
{
    vector<shared_ptr<DataDefinition>> datarefs;
    auto datadef = make_shared<DataDefinition>();
    datadef->idx = 1;
    datadef->mapping.dataRef = dataref;
    datarefs.push_back(datadef);

    return streamDataRefsInternal(m_dataSocket, datarefs, [&returnValue](map<int, float> const& values)
    {
        for (const auto& [idx, value] : values)
        {
            returnValue = value;
        }
    }, 1);
}

void XPlaneUDPClient::sendMessage(const string& string)
{
    XPlaneAlertMessage message;

    const auto alrt = "ALRT";
    memcpy(message.header, alrt, 4);
    message.header[4] = 0;
    strcpy(message.message1, string.c_str());
    message.message2[0] = 0;
    message.message3[0] = 0;
    message.message4[0] = 0;
    m_dataSocket->send(&message, sizeof(message));
}

