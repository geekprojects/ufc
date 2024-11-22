//
// Created by Ian Parker on 11/11/2024.
//

#include "xpplugindatasource.h"

#include <ufc/flightconnector.h>

#define XPLM200 1
#define XPLM210 1
#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

using namespace std;
using namespace UFC;

float updateCallback(float elapsedMe, float elapsedSim, int counter, void * refcon)
{
    ((XPPluginDataSource*)refcon)->updateDataRefs();
    return 0.1f;
}

XPPluginDataSource::XPPluginDataSource(UFC::FlightConnector* flightConnector) :
    DataSource(flightConnector, "XPPlugin", 100),
    m_dataMapping("Resources/plugins/ufc")
{
}

XPPluginDataSource::~XPPluginDataSource()
{
}

bool XPPluginDataSource::connect()
{
    AircraftState state = m_flightConnector->getState();

    m_icaoDataRef = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
    m_authorDataRef = XPLMFindDataRef("sim/aircraft/view/acf_author");
    m_studioDataRef = XPLMFindDataRef("sim/aircraft/view/acf_studio");

    state.aircraftICAO = getString(m_icaoDataRef);
    state.aircraftAuthor = getString(m_studioDataRef);
    if (state.aircraftAuthor.empty())
    {
        state.aircraftAuthor = getString(m_authorDataRef);
    }
    log(DEBUG, "UFC: ICAO: %s, author: %s", state.aircraftICAO.c_str(), state.aircraftAuthor.c_str());

    m_dataMapping.loadDefinitions("defaults.yaml");
    m_dataMapping.loadDefinitionsForAircraft(state.aircraftAuthor, state.aircraftICAO);

    for (auto& mapping : m_dataMapping.getDataRefs())
    {
        if (!mapping->mapping.dataRef.empty())
        {
            mapping->data = XPLMFindDataRef(mapping->mapping.dataRef.c_str());
        }
        else
        {
            log(DEBUG, "UFC: Unable to find data ref: %s", mapping->mapping.dataRef.c_str());
            mapping->data = nullptr;
        }
    }

    for (auto& commandMapping : m_dataMapping.getCommands())
    {
        commandMapping.second.data = XPLMFindCommand(commandMapping.second.command.c_str());
    }

    m_flightConnector->updateState(state);

    XPLMCreateFlightLoop_t createFlightLoop;
    createFlightLoop.structSize = sizeof(XPLMCreateFlightLoop_t);
    createFlightLoop.phase = xplm_FlightLoop_Phase_AfterFlightModel;
    createFlightLoop.callbackFunc = updateCallback;
    createFlightLoop.refcon = this;
    auto flightLoopId = XPLMCreateFlightLoop(&createFlightLoop);
    XPLMScheduleFlightLoop(flightLoopId, 0.1f, true);

    return true;
}

void XPPluginDataSource::disconnect()
{
}

bool XPPluginDataSource::update()
{
    // Don't do anything here, we let X-Plane tell us when to update
    return true;
}

bool XPPluginDataSource::updateDataRefs()
{
    AircraftState state = m_flightConnector->getState();
    for (const auto& mapping : m_dataMapping.getDataRefs())
    {
        if (mapping->data == nullptr)
        {
            continue;
        }
        switch (mapping->type)
        {
            case FLOAT:
                m_dataMapping.writeFloat(state, mapping, XPLMGetDataf(mapping->data));
                break;
            case BOOLEAN:
                m_dataMapping.writeBoolean(state, mapping, XPLMGetDatai(mapping->data));
                break;
            case INTEGER:
                m_dataMapping.writeInt(state, mapping, XPLMGetDatai(mapping->data));
                break;
        }
    }
    m_flightConnector->updateState(state);

    std::scoped_lock lock(m_commandQueueMutex);
    for (auto command : m_commandQueue)
    {
        XPLMCommandOnce(command);
    }
    m_commandQueue.clear();

    return true;
}

void XPPluginDataSource::command(std::string command)
{
    CommandDefinition commandDef = m_dataMapping.getCommand(command);

    string xcommand = commandDef.command;
    auto idx = xcommand.find('=');
    if (idx == string::npos)
    {
        if (commandDef.data == nullptr)
        {
            return;
        }

        std::scoped_lock lock(m_commandQueueMutex);
        m_commandQueue.push_back(commandDef.data);
    }
    else
    {
        string dataRefName = xcommand.substr(0, idx);
        float value = atof(xcommand.substr(idx + 1).c_str());
        log(INFO, "command: Setting data ref: %s = %0.2f", dataRefName.c_str(), value);

        DataRefInfo dataRefInfo;
        auto it = m_commandDataRefs.find(dataRefName);
        if (it == m_commandDataRefs.end())
        {
            dataRefInfo.dataRef = XPLMFindDataRef(dataRefName.c_str());
            if (dataRefInfo.dataRef != nullptr)
            {
                dataRefInfo.types = XPLMGetDataRefTypes(dataRefInfo.dataRef);
            }
            m_commandDataRefs.try_emplace(dataRefName, dataRefInfo);
        }
        else
        {
            dataRefInfo = it->second;
        }

        if (dataRefInfo.dataRef != nullptr)
        {
            if (!!(dataRefInfo.types & xplmType_Float))
            {
                XPLMSetDataf(dataRefInfo.dataRef, value);
            }
            else if (!!(dataRefInfo.types & xplmType_Int))
            {
                XPLMSetDatai(dataRefInfo.dataRef, (int)value);
            }
        }
    }
    //XPLMCommandOnce(commandDef.data);
}

std::string XPPluginDataSource::getString(XPLMDataRef ref)
{
    int bytes = XPLMGetDatab(ref, nullptr, 0, 0);
    char buffer[bytes + 1];
    XPLMGetDatab(ref, buffer, 0, bytes);
    buffer[bytes] = '\0';
    return string(buffer);
}

