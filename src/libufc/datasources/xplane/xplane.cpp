//
// Created by Ian Parker on 23/01/2024.
//

#include "xplane.h"

#include <ufc/flightconnector.h>

#include <filesystem>
#include <unistd.h>

using namespace std;
using namespace UFC;

UFC_DATA_SOURCE(XPlane, XPlaneDataSource)

XPlaneDataSource::XPlaneDataSource(FlightConnector* flightConnector) : DataSource(flightConnector, "XPlane", 10), m_mapping("../data/x-plane")
{
    m_client = make_shared<XPlaneClient>(
        flightConnector->getConfig().xplaneHost,
        flightConnector->getConfig().xplanePort);
}

bool XPlaneDataSource::connect()
{
    auto res = m_client->connect();
    if (res != XPlaneResult::SUCCESS)
    {
        return false;
    }

    AircraftState state = m_flightConnector->getState();

    log(INFO, "init: Requesting details from X-Plane...");
    while (true)
    {
        res = m_client->readInt("sim/version/xplane_internal_version", m_xPlaneVersion);
        if (res != XPlaneResult::TIMEOUT)
        {
            break;
        }

        // Timeout! Try again in a second
        sleep(1);
    }
    if (res == XPlaneResult::FAIL || m_xPlaneVersion == 0)
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

    m_mapping.loadDefinitions("defaults.yaml");
    m_mapping.loadDefinitionsForAircraft(state.aircraftAuthor, state.aircraftICAO);

    return true;
}

void XPlaneDataSource::disconnect()
{
    m_client->disconnect();
}

bool XPlaneDataSource::update()
{
    vector<pair<int, string>> datarefs;

    int idx = 1;
    for (const auto& dataRef : m_mapping.getDataRefs())
    {
        if (!dataRef->mapping.dataRef.empty() && dataRef->mapping.dataRef != "null")
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

    return res == XPlaneResult::SUCCESS;
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
        switch (dataRef->type)
        {
            case FLOAT:
                m_mapping.writeFloat(state, dataRef, value);
                break;
            case INTEGER:
                m_mapping.writeInt(state, dataRef, (int32_t)value);
                break;
            case BOOLEAN:
                m_mapping.writeBoolean(state, dataRef, (bool)value);
                break;
        }
    }
    m_flightConnector->updateState(state);
}

void XPlaneDataSource::command(string command)
{
    auto xpcommand = m_mapping.getCommand(command);
    if (xpcommand.command.empty())
    {
        return;
    }

    string xcommand = xpcommand.command;
    printf("XPlaneDataSource::command: %s -> %s\n", command.c_str(), xcommand.c_str());

    auto idx = xcommand.find("=");
    if (idx == xcommand.npos)
    {
        m_client->sendCommand(xcommand);
    }
    else
    {
        string dataref = xcommand.substr(0, idx);
        float value = atof(xcommand.substr(idx + 1).c_str());
        log(INFO, "command: Setting data ref: %s = %0.2f", dataref.c_str(), value);
        m_client->setDataRef(dataref, value);
    }
}

