//
// Created by Ian Parker on 23/01/2024.
//

#include "xplane.h"

#include <ufc/flightconnector.h>
#include <ufc/utils.h>

#include <unistd.h>

using namespace std;
using namespace UFC;

UFC_DATA_SOURCE(XPlane, XPlaneDataSource)

XPlaneDataSource::XPlaneDataSource(FlightConnector* flightConnector) :
    DataSource(
        flightConnector,
        "XPlane",
        flightConnector->getConfig().dataDir + "/x-plane")
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

    auto state = getFlightConnector()->getState();

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

    string aircraftAuthor;
    m_client->readString("sim/aircraft/view/acf_studio", 64, aircraftAuthor);
    if (aircraftAuthor.empty())
    {
        m_client->readString("sim/aircraft/view/acf_author", 64, aircraftAuthor);
    }
    state->set("aircraft/author", aircraftAuthor);

    string aircraftICAO;
    m_client->readString("sim/aircraft/view/acf_ICAO", 10, aircraftICAO);
    state->set("aircraft/icao", aircraftICAO);

    log(INFO, "X-Plane version: %d", m_xPlaneVersion);
    log(INFO, "Author: %s", aircraftAuthor.c_str());
    log(INFO, "Aircraft ICAO type: %s", aircraftICAO.c_str());

    getMapping().loadDefinitionsForAircraft(aircraftAuthor, aircraftICAO);

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

    auto state = getFlightConnector()->getState();
    int idx = 1;
    for (const auto& dataRef : getMapping().getDataRefs())
    {
        log(DEBUG, "update: %s -> %s", dataRef->id.c_str(), dataRef->mapping.dataRef.c_str());
        if (!dataRef->mapping.dataRef.empty() &&
            dataRef->mapping.dataRef != "null")
        {
            dataRef->value = state->getOrCreateValue(dataRef->id);
            log(DEBUG, "update: %s idx=%d, value idx=%d", dataRef->id.c_str(), idx, dataRef->value->getIndex());
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
    auto state = getFlightConnector()->getState();

    for (const auto& [idx, value] : values)
    {
        if (idx < 1)
        {
            continue;
        }

        const auto& dataRef = getMapping().getDataRefs()[idx - 1];

        auto v = transformData(dataRef, value);
        switch (dataRef->type)
        {
            case DataRefType::FLOAT:
                getMapping().writeFloat(dataRef, v);
                break;
            case DataRefType::BOOLEAN:
                getMapping().writeBoolean(dataRef, static_cast<bool>(v));
                break;
            case DataRefType::INTEGER:
                getMapping().writeInt(dataRef, static_cast<int>(v));
                break;
            default:
                if (dataRef->value != nullptr)
                {
                    getMapping().writeFloat(dataRef, v);
                }
                else
                {
                    log(WARN, "update: Unknown value for %s", dataRef->id.c_str());
                }
                break;
        }
    }
}

void XPlaneDataSource::executeCommand(const string& commandName, const CommandDefinition& commandDefinition)
{
    log(DEBUG, "executeCommand: %s", commandName.c_str());
    m_client->sendCommand(commandName);
}

void XPlaneDataSource::setData(const std::string& dataName, float value)
{
    auto dataRef = getMapping().getDataRef(dataName);
    if (dataRef == nullptr)
    {
        // Not mapped. Try setting directly
        m_client->setDataRef(dataName, value);
        return;
    }

    if (dataRef->mapping.dataRef.empty())
    {
        log(WARN, "setData: Data ref is empty for {}", dataName.c_str());
        return;
    }

    m_client->setDataRef(dataRef->mapping.dataRef, value);
}

bool XPlaneDataSource::getDataInt(const std::string& dataName, int& value)
{
    auto dataRef = getMapping().getDataRef(dataName);
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
    auto dataRef = getMapping().getDataRef(dataName);
    if (dataRef == nullptr || dataRef->mapping.dataRef.empty())
    {
        log(WARN, "getDataFloat: Unhandled data ref: %s", dataName.c_str());
        auto res = m_client->read(dataName, value);
        return res == Result::SUCCESS;
    }

    auto res = m_client->read(dataRef->mapping.dataRef, value);
    return res == Result::SUCCESS;
}

bool XPlaneDataSource::getDataString(const std::string& dataName, string& value)
{
    auto dataRef = getMapping().getDataRef(dataName);
    if (dataRef == nullptr || dataRef->mapping.dataRef.empty())
    {
        log(WARN, "getDataString: Unhandled data ref: %s", dataName.c_str());
        return false;
    }

    auto res = m_client->readString(dataRef->mapping.dataRef, dataRef->len, value);
    return res == Result::SUCCESS;
}

void XPlaneDataSource::sendMessage(const std::string& message)
{
    m_client->sendMessage(message);
}
