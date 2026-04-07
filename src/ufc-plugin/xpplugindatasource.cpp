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

XPPluginDataSource::XPPluginDataSource(UFC::FlightConnector* flightConnector) :
    DataSource(flightConnector, "XPPlugin", "Resources/plugins/ufc/data/x-plane")
{
}

XPPluginDataSource::~XPPluginDataSource() = default;

bool XPPluginDataSource::connect()
{

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
    log(DEBUG, "reloadAircraft: Checking aircraft...");
    m_icaoDataRef = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
    m_authorDataRef = XPLMFindDataRef("sim/aircraft/view/acf_author");
    m_studioDataRef = XPLMFindDataRef("sim/aircraft/view/acf_studio");

    string aircraftICAO = getString(m_icaoDataRef);
    string aircraftAuthor = getString(m_studioDataRef);
    if (aircraftAuthor.empty())
    {
        aircraftAuthor = getString(m_authorDataRef);
    }

    auto state = getFlightConnector()->getState();
    if (aircraftICAO == state->getString(DATA_AIRCRAFT_ICAO) &&
        aircraftAuthor == state->getString(DATA_AIRCRAFT_AUTHOR))
    {
        // Aircraft definitions are already loaded, nothing to do!
        return true;
    }

    state->set(DATA_AIRCRAFT_AUTHOR, aircraftAuthor);
    state->set(DATA_AIRCRAFT_ICAO, aircraftICAO);

    log(DEBUG, "reloadAircraft: New Aircraft: ICAO: %s, author: %s", aircraftICAO.c_str(), aircraftAuthor.c_str());

    getMapping().loadDefinitionsForAircraft(aircraftAuthor, aircraftICAO);

    for (auto& mapping : getMapping().getDataRefs())
    {
        mapping->value = state->getOrCreateValue(mapping->id);
        if (!mapping->mapping.dataRef.empty())
        {
            mapping->data = XPLMFindDataRef(mapping->mapping.dataRef.c_str());
            auto type = XPLMGetDataRefTypes(mapping->data);
            if (!!(type & xplmType_Float) || !!(type & xplmType_Double))
            {
                mapping->type = DataRefType::FLOAT;
            }
            else if (!!(type & xplmType_Int))
            {
                mapping->type = DataRefType::INTEGER;
            }
            else
            {
                log(WARN, "reloadAircraft: Unhandled type for dataRef %s: %d", mapping->mapping.dataRef.c_str(), type);
                mapping->data = nullptr;
            }
        }
        else
        {
            mapping->data = nullptr;
        }
        log(DEBUG, "reloadAircraft: Data: %s -> %s (%p)", mapping->id.c_str(), mapping->mapping.dataRef.c_str(), mapping->data);
    }

    for (const auto& commandMapping : getMapping().getCommands())
    {
        for (auto const& command : commandMapping.second.commands)
        {
            if (!command.empty() && !command.starts_with("lua:"))
            {
                findOrRegisterCommand(command);
            }
        }
    }

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

float XPPluginDataSource::updateCallback(
    [[maybe_unused]] float elapsedMe,
    [[maybe_unused]] float elapsedSim,
    [[maybe_unused]] int counter,
    XPPluginDataSource* refcon)
{
    refcon->checkReload();
    refcon->updateDataRefs();
    refcon->executeCommands();
    refcon->updateValues();

    return 0.1f;
}

void XPPluginDataSource::checkReload()
{
    if (m_checkAircraft)
    {
        reloadAircraft();
        m_checkAircraft = false;
    }
}

bool XPPluginDataSource::updateDataRefs()
{
    auto state = getFlightConnector()->getState();
    for (const auto& mapping : getMapping().getDataRefs())
    {
        if (mapping->mapping.type == DataMappingType::STATIC)
        {
            getMapping().writeValue(mapping, mapping->mapping.value);
            continue;
        }
        if (mapping->data == nullptr)
        {
            continue;
        }

        switch (mapping->type)
        {
            case DataRefType::BOOLEAN:
            {
                auto i = XPLMGetDatai(mapping->data);
                i = transformData(mapping, i);
                getMapping().writeBoolean(mapping, i);
                break;
            }

            case DataRefType::INTEGER:
            {
                auto i = XPLMGetDatai(mapping->data);
                i = transformData(mapping, i);
                getMapping().writeInt(mapping, i);
                break;
            }

            case DataRefType::FLOAT:
            default:
            {
                auto f = XPLMGetDataf(mapping->data);
                f = transformData(mapping, f);
                getMapping().writeFloat(mapping, f);
                break;
            }
        }
    }


    return true;
}

void XPPluginDataSource::executeCommands()
{
    scoped_lock lock(m_commandQueueMutex);
    for (auto command : m_commandQueue)
    {
        XPLMCommandOnce(command);
    }
    m_commandQueue.clear();
}

void XPPluginDataSource::updateValues()
{
    scoped_lock lock(m_dataQueueMutex);
    for (auto& data : m_dataQueue)
    {
        executeSetData(data.first, data.second);
    }
    m_dataQueue.clear();
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
    scoped_lock lock(m_dataQueueMutex);
    m_dataQueue.emplace(dataName, value);
}

void XPPluginDataSource::executeSetData(const std::string &dataName, float value)
{
    log(DEBUG, "setData: %s -> %f", dataName.c_str(), value);
    string dataRefName = dataName;

    auto dataRef = getMapping().getDataRef(dataName);
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

std::string XPPluginDataSource::getString(const XPLMDataRef ref)
{
    log(DEBUG, "getString: ref=%p", ref);
    int bytes = XPLMGetDatab(ref, nullptr, 0, 0);
    if (bytes == 0)
    {
        return "";
    }

    string buffer(bytes, '\0');
    XPLMGetDatab(ref, buffer.data(), 0, bytes);
    return buffer;
}
