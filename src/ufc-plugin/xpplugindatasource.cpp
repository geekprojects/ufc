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

float updateCallback(
    [[maybe_unused]] float elapsedMe,
    [[maybe_unused]] float elapsedSim,
    [[maybe_unused]] int counter,
    XPPluginDataSource* refcon)
{
    refcon->updateDataRefs();
    return 0.1f;
}

XPPluginDataSource::XPPluginDataSource(UFC::FlightConnector* flightConnector) :
    DataSource(flightConnector, "XPPlugin", "Resources/plugins/ufc", 100)
{
}

XPPluginDataSource::~XPPluginDataSource() = default;

bool XPPluginDataSource::connect()
{
    bool res = reloadAircraft();
    if (!res)
    {
        return false;
    }

    XPLMCreateFlightLoop_t createFlightLoop;
    createFlightLoop.structSize = sizeof(XPLMCreateFlightLoop_t);
    createFlightLoop.phase = xplm_FlightLoop_Phase_AfterFlightModel;
    createFlightLoop.callbackFunc = (XPLMFlightLoop_f)updateCallback;
    createFlightLoop.refcon = this;
    auto flightLoopId = XPLMCreateFlightLoop(&createFlightLoop);
    XPLMScheduleFlightLoop(flightLoopId, 0.1f, true);

    return true;
}

XPLMCommandRef XPPluginDataSource::findOrRegisterCommand(string const &command)
{
    auto it = m_commandDefs.find(command);
    XPLMCommandRef commandRef;
    if (it == m_commandDefs.end())
    {
        commandRef = XPLMFindCommand(command.c_str());
        m_commandDefs.try_emplace(command, commandRef);
        if (commandRef != nullptr)
        {
#if 0
            log(DEBUG, "UFC: Command: %s -> %s (%p)", command.c_str(), command.c_str(), commandRef);
#endif
        }
        else
        {
            log(WARN, "UFC: Command: Unable to find command %s", command.c_str());
        }
    }
    else
    {
        commandRef = it->second;
    }
    return commandRef;
}

bool XPPluginDataSource::reloadAircraft()
{
    m_icaoDataRef = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
    m_authorDataRef = XPLMFindDataRef("sim/aircraft/view/acf_author");
    m_studioDataRef = XPLMFindDataRef("sim/aircraft/view/acf_studio");

    string aircraftICAO = getString(m_icaoDataRef);
    string aircraftAuthor = getString(m_studioDataRef);
    if (aircraftAuthor.empty())
    {
        aircraftAuthor = getString(m_authorDataRef);
    }

    AircraftState state = m_flightConnector->getState();
    if (aircraftICAO == state.aircraftICAO && aircraftAuthor == state.aircraftAuthor)
    {
        // Aircraft definitions are already loaded, nothing to do!
        return true;
    }

    state.aircraftICAO = aircraftICAO;
    state.aircraftAuthor = aircraftAuthor;

    log(DEBUG, "UFC: New Aircraft: ICAO: %s, author: %s", state.aircraftICAO.c_str(), state.aircraftAuthor.c_str());

    m_mapping.loadDefinitionsForAircraft(state.aircraftAuthor, state.aircraftICAO);

    for (auto& mapping : m_mapping.getDataRefs())
    {
        if (!mapping->mapping.dataRef.empty())
        {
            mapping->data = XPLMFindDataRef(mapping->mapping.dataRef.c_str());
        }
        else
        {
            log(WARN, "UFC: Unable to find data ref: %s", mapping->mapping.dataRef.c_str());
            mapping->data = nullptr;
        }
        log(DEBUG, "UFC: Data: %s -> %s (%p)", mapping->id.c_str(), mapping->mapping.dataRef.c_str(), mapping->data);
    }

    for (auto& commandMapping : m_mapping.getCommands())
    {
        for (auto const& command : commandMapping.second.commands)
        {
            if (!command.empty() && !command.starts_with("lua:"))
            {
                findOrRegisterCommand(command);
            }
        }
    }

    m_flightConnector->updateState(state);

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
    for (const auto& mapping : m_mapping.getDataRefs())
    {
        if (mapping->data == nullptr)
        {
            continue;
        }
        switch (mapping->type)
        {
            case FLOAT:
                m_mapping.writeFloat(state, mapping, XPLMGetDataf(mapping->data));
                break;
            case BOOLEAN:
                m_mapping.writeBoolean(state, mapping, XPLMGetDatai(mapping->data));
                break;
            case INTEGER:
                m_mapping.writeInt(state, mapping, XPLMGetDatai(mapping->data));
                break;
            case STRING:
                // Unhandled
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

void XPPluginDataSource::executeCommand(const std::string& command, const CommandDefinition& commandDefinition)
{
    log(DEBUG, "executeCommand: %s", command.c_str());
    XPLMCommandRef commandRef = findOrRegisterCommand(command);
    if (commandRef != nullptr)
    {
        std::scoped_lock lock(m_commandQueueMutex);
        m_commandQueue.push_back(commandRef);
    }
}

void XPPluginDataSource::setData(const std::string &dataName, float value)
{
    log(DEBUG, "setData: %s -> %f", dataName.c_str(), value);
    string dataRefName = dataName;

    auto dataRef = m_mapping.getDataRef(dataName);
    if (dataRef != nullptr && !dataRef->mapping.dataRef.empty())
    {
        dataRefName = dataRef->mapping.dataRef;
    }

    DataRefInfo dataRefInfo;
    auto it = m_dataRefInfoMap.find(dataRefName);
    if (it == m_dataRefInfoMap.end())
    {
        dataRefInfo.dataRef = XPLMFindDataRef(dataRefName.c_str());
        if (dataRefInfo.dataRef != nullptr)
        {
            dataRefInfo.types = XPLMGetDataRefTypes(dataRefInfo.dataRef);
        }
        m_dataRefInfoMap.try_emplace(dataRefName, dataRefInfo);
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

bool XPPluginDataSource::getDataInt(const std::string &dataName, int &value)
{
    auto dataRef = m_mapping.getDataRef(dataName);
    if (dataRef == nullptr || dataRef->mapping.dataRef.empty())
    {
        log(WARN, "getDataInt: Unhandled data ref: %s", dataName.c_str());
        return false;
    }

    value = XPLMGetDatai(dataRef->data);
    return true;
}

bool XPPluginDataSource::getDataFloat(const std::string &dataName, float &value)
{
    auto dataRef = m_mapping.getDataRef(dataName);
    if (dataRef == nullptr || dataRef->mapping.dataRef.empty())
    {
        log(WARN, "getDataInt: Unhandled data ref: %s", dataName.c_str());
        return false;
    }

    value = XPLMGetDataf(dataRef->data);
    return true;
}

bool XPPluginDataSource::getDataString(const std::string &dataName, std::string &value)
{
    auto dataRef = m_mapping.getDataRef(dataName);
    if (dataRef == nullptr || dataRef->mapping.dataRef.empty())
    {
        log(WARN, "getDataInt: Unhandled data ref: %s", dataName.c_str());
        return false;
    }

    value = getString(dataRef->data);
    return true;
}

std::string XPPluginDataSource::getString(const XPLMDataRef ref)
{
    int bytes = XPLMGetDatab(ref, nullptr, 0, 0);
    char buffer[bytes + 1];
    XPLMGetDatab(ref, buffer, 0, bytes);
    buffer[bytes] = '\0';
    return string(buffer);
}

