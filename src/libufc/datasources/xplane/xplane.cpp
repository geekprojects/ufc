//
// Created by Ian Parker on 23/01/2024.
//

#include "xplane.h"

#include <ufc/flightconnector.h>
#include <ufc/lua.h>

#include <filesystem>
#include <unistd.h>

#include "ufc/utils.h"

using namespace std;
using namespace UFC;

UFC_DATA_SOURCE(XPlane, XPlaneDataSource)

XPlaneDataSource::XPlaneDataSource(FlightConnector* flightConnector) :
    DataSource(flightConnector, "XPlane", 10),
    m_mapping(this, "../data/x-plane")
{
    m_client = make_shared<XPlaneClient>(
        flightConnector->getConfig().xplaneHost,
        flightConnector->getConfig().xplanePort);
}

bool XPlaneDataSource::connect()
{
    auto res = m_client->connect();
    if (res != Result::SUCCESS)
    {
        return false;
    }

    AircraftState state = m_flightConnector->getState();

    log(INFO, "init: Requesting details from X-Plane...");
    while (true)
    {
        res = m_client->readInt("sim/version/xplane_internal_version", m_xPlaneVersion);
        if (res != Result::TIMEOUT)
        {
            break;
        }

        // Timeout! Try again in a second
        sleep(1);
    }

    if (res == Result::FAIL || m_xPlaneVersion == 0)
    {
        log(ERROR, "init: Unable to connect to X-Plane");
        return false;
    }

    m_client->readString("sim/aircraft/view/acf_studio", 64, state.aircraftAuthor);
    if (state.aircraftAuthor.empty())
    {
        m_client->readString("sim/aircraft/view/acf_author", 64, state.aircraftAuthor);
    }
    m_client->readString("sim/aircraft/view/acf_ICAO", 10, state.aircraftICAO);

    log(INFO, "X-Plane version: %d", m_xPlaneVersion);
    log(INFO, "Author: %s", state.aircraftAuthor.c_str());
    log(INFO, "Aircraft ICAO type: %s", state.aircraftICAO.c_str());

    m_mapping.loadDefinitionsForAircraft(state.aircraftAuthor, state.aircraftICAO);

    return true;
}

void XPlaneDataSource::disconnect()
{
    m_client->disconnect();
}

bool XPlaneDataSource::isConnected()
{
    return m_client->isConnected();
}

bool XPlaneDataSource::update()
{
    vector<pair<int, string>> datarefs;

    int idx = 1;
    for (const auto& dataRef : m_mapping.getDataRefs())
    {
        if (!dataRef->mapping.dataRef.empty() &&
            dataRef->mapping.dataRef != "null" &&
            dataRef->pos != -1)
        {
            dataRef->idx = idx;
            datarefs.emplace_back(idx, dataRef->mapping.dataRef);
        }
        idx++;
    }

    auto res = m_client->streamDataRefs(datarefs, [this](map<int, float> const& values)
    {
        update(values);
    });

    return res == Result::SUCCESS;
}

void XPlaneDataSource::update(const map<int, float>& values)
{
    AircraftState state = m_flightConnector->getState();

    state.connected = true;

    for (const auto& [idx, value] : values)
    {
        if (idx < 1)
        {
            continue;
        }

        const auto& dataRef = m_mapping.getDataRefs()[idx - 1];

        float v = value;
        if (!dataRef->mapping.luaScript.empty())
        {
            v = m_dataLua.execute(dataRef->mapping.luaScript, "value", value);
        }

        switch (dataRef->type)
        {
            case FLOAT:
                m_mapping.writeFloat(state, dataRef, v);
                break;
            case INTEGER:
                m_mapping.writeInt(state, dataRef, (int32_t)v);
                break;
            case BOOLEAN:
                m_mapping.writeBoolean(state, dataRef, (int32_t)v);
                break;
            case STRING:
                // Ignored!
                break;
        }
    }
    m_flightConnector->updateState(state);
}

void XPlaneDataSource::command(const string& commandName)
{
    auto commandDefinition = m_mapping.getCommand(commandName);
    if (commandDefinition.commands.empty())
    {
        return;
    }

    for (auto commandStr : commandDefinition.commands)
    {
        printf("XPlaneDataSource::command: %s -> %s\n", commandName.c_str(), commandStr.c_str());

        if (commandStr.starts_with("lua:"))
        {
            string luaScript = commandStr.substr(4);
            m_commandLua.execute(luaScript);
            continue;
        }

        auto idx = commandStr.find('=');
        if (idx != string::npos)
        {
            string dataref = StringUtils::trim(commandStr.substr(0, idx));
            string valueStr = StringUtils::trim(commandStr.substr(idx + 1));
            auto value = (float)atof(valueStr.c_str());
            log(INFO, "command: Setting data ref: %s = %0.2f", dataref.c_str(), value);
            m_client->setDataRef(dataref, value);
            continue;
        }

        m_client->sendCommand(commandStr);
    }
}

void XPlaneDataSource::setData(const std::string& dataName, float value)
{
    auto dataRef = m_mapping.getDataRef(dataName);
    if (dataRef == nullptr || dataRef->mapping.dataRef.empty())
    {
        log(WARN, "setData: Unhandled data ref: %s", dataName.c_str());
        return;
    }

    m_client->setDataRef(dataRef->mapping.dataRef, value);
}

bool XPlaneDataSource::getDataInt(const std::string& dataName, int& value)
{
    auto dataRef = m_mapping.getDataRef(dataName);
    if (dataRef == nullptr || dataRef->mapping.dataRef.empty())
    {
        log(WARN, "getDataInt: Unhandled data ref: %s", dataName.c_str());
        return false;
    }

    auto res = m_client->readInt(dataRef->mapping.dataRef, value);
    return res == Result::SUCCESS;
}

bool XPlaneDataSource::getDataFloat(const std::string& dataName, float& value)
{
    auto dataRef = m_mapping.getDataRef(dataName);
    if (dataRef == nullptr || dataRef->mapping.dataRef.empty())
    {
        log(WARN, "getDataInt: Unhandled data ref: %s", dataName.c_str());
        return false;
    }

    auto res = m_client->read(dataRef->mapping.dataRef, value);
    return res == Result::SUCCESS;
}

bool XPlaneDataSource::getDataString(const std::string& dataName, string& value)
{
    auto dataRef = m_mapping.getDataRef(dataName);
    if (dataRef == nullptr || dataRef->mapping.dataRef.empty())
    {
        log(WARN, "getDataInt: Unhandled data ref: %s", dataName.c_str());
        return false;
    }

    auto res = m_client->readString(dataRef->mapping.dataRef, dataRef->len, value);
    return res == Result::SUCCESS;
}

void XPlaneDataSource::sendMessage(const std::string& message)
{
    m_client->sendMessage(message);
}
