//
// Created by Ian Parker on 06/04/2026.
//

#include "xplanewsclient.h"
#include "../../../libufc-utils/base64.h"

#include <curly.hpp/curly.hpp>
#include <nlohmann/json.hpp>

#include "curl/curl.h"

#include <cinttypes>

using namespace std;
using namespace UFC;
using namespace nlohmann;

XPlaneWebSocketClient::XPlaneWebSocketClient()
    : XPlaneClient("XPlaneWebSocketClient")
{
    m_baseRestUrl = "http://localhost:8086/api/v3";
    m_baseWebSocketUrl = "ws://localhost:8086/api/v3";
}

Result XPlaneWebSocketClient::connect()
{
    return Result::SUCCESS;
}

void XPlaneWebSocketClient::disconnect()
{
    scoped_lock lock(m_webSocketsMutex);
    for (const auto webSocket : m_webSockets)
    {
        log(DEBUG, "disconnect: Closing web socket: %p", webSocket);

        // Signal to the socket that we want to close it
        curl_ws_start_frame(webSocket->ws, CURLWS_CLOSE, 0);
    }
}

bool XPlaneWebSocketClient::isConnected() const
{
    return true;
}

Result XPlaneWebSocketClient::readString(const string &dataref, int len, string &value)
{
    json valueJson;
    if (!getDataRef(dataref, valueJson))
    {
        return Result::FAIL;
    }

    string value64 = valueJson.get<string>();

#ifdef DEBUG_XPLANE_WS
    log(DEBUG, "readString: %s = Base64: %s", dataref.c_str(), value64.c_str());
#endif

    value = base64_decode(value64);
    log(DEBUG, "readString: %s = %s", dataref.c_str(), value.c_str());

    return Result::SUCCESS;
}

Result XPlaneWebSocketClient::read(const string &dataref, float& value)
{
    json valueJson;
    if (!getDataRef(dataref, valueJson))
    {
        return Result::FAIL;
    }

    value = valueJson.get<float>();
    log(DEBUG, "read: %s = %0.2f", dataref.c_str(), value);
    return Result::SUCCESS;
}

Result XPlaneWebSocketClient::readInt(const string &dataref, int &value)
{
    json valueJson;
    if (!getDataRef(dataref, valueJson))
    {
        return Result::FAIL;
    }

    value = valueJson.get<int>();
    log(DEBUG, "readInt: %s = %d", dataref.c_str(), value);

    return Result::SUCCESS;
}

Result XPlaneWebSocketClient::setDataRef(const string &dataRef, float value)
{
    int64_t id = getDataRefId(dataRef);
    if (id == -1)
    {
        log(WARN, "setDataRef: Data ref not found: %s", dataRef.c_str());
        return Result::FAIL;
    }

    json bodyJson;
    bodyJson["data"] = value;

    string url = m_baseRestUrl + "/datarefs/" + to_string(id) + "/value";
#if 1
    log(DEBUG, "setDataRef: Sending request to: %s", url.c_str());
#endif
    auto request = curly_hpp::request_builder()
        .method(curly_hpp::http_method::PATCH)
        .url(url)
        .content(bodyJson.dump())
        .header("Accept", "application/json, text/plain, */*")
        .send();

    curly_hpp::response response;
    try
    {
        response = request.take();
    }
    catch (curly_hpp::exception& e)
    {
        log(ERROR, "setDataRef: Failed to set %s to %f: %s", dataRef.c_str(), value, e.what());
        return Result::FAIL;
    }

    int code = response.http_code();
    if (code != 200)
    {
        log(ERROR, "setDataRef: Failed to set %s to %f: code=%d", dataRef.c_str(), value, code);
        string content = response.content.as_string_copy();
        log(ERROR, "setDataRef: content: %s\n", content.c_str());
        return Result::FAIL;
    }

    return Result::SUCCESS;
}

Result XPlaneWebSocketClient::sendCommand(const string &command)
{
    auto id = getCommandId(command);
    if (id == -1)
    {
        log(WARN, "sendCommand: Command not found: %s", command.c_str());
        return Result::FAIL;
    }

    json bodyJson;
    bodyJson["duration"] = 0;

    string url = m_baseRestUrl + "/command/" + to_string(id) + "/activate";
#ifdef DEBUG_XPLANE_WS
    log(DEBUG, "sendCommand: Sending request to: %s", url.c_str());
#endif
    auto request= curly_hpp::request_builder()
        .method(curly_hpp::http_method::POST)
        .url(url)
        .content(bodyJson.dump())
        .header("Accept", "application/json, text/plain, */*")
        .send();

    curly_hpp::response response;
    try
    {
        response = request.take();
    }
    catch (curly_hpp::exception& e)
    {
        log(ERROR, "sendCommand: Failed to activate command %s: %s", command.c_str(), e.what());
        return Result::FAIL;
    }

    int code = response.http_code();
    if (code != 200)
    {
        log(ERROR, "sendCommand: Failed to activate command %s: code=%d", command.c_str(), code);
        string content = response.content.as_string_copy();
        log(ERROR, "sendCommand: content: %s\n", content.c_str());
    }

    return Result::SUCCESS;
}

void XPlaneWebSocketClient::sendMessage(const string &string)
{
    fprintf(stderr, "XPlaneWebSocketClient::sendMessage: NOT IMPLEMENTED: %s\n", string.c_str());
}

int64_t XPlaneWebSocketClient::getDataRefId(string dataref)
{
    if (dataref.empty())
    {
        return -1;
    }

    auto indexIdx = dataref.find_first_of('[');
    if (indexIdx != string::npos)
    {
#ifdef DEBUG_XPLANE_WS
        log(DEBUG, "getDataRefId: Stripping index off of %s...", dataref.c_str());
#endif
        dataref = dataref.substr(0, indexIdx);
    }

    auto it = m_dataRefIds.find(dataref);
    if (it != m_dataRefIds.end())
    {
#ifdef DEBUG_XPLANE_WS
        log(DEBUG, "getDataRefIds: Got: %s -> %" PRId64, dataref.c_str(), it->second);
#endif
        return it->second;
    }

    string query = "filter[name]=" + dataref;
#ifdef DEBUG_XPLANE_WS
    log(DEBUG, "getDataRefId: Querying for %s...", dataref.c_str());
#endif

    string url = m_baseRestUrl + "/datarefs?" + query;
#ifdef DEBUG_XPLANE_WS
    log(DEBUG, "getDataRefIds: Sending request to: %s", url.c_str());
#endif

    auto request= curly_hpp::request_builder()
        .method(curly_hpp::http_method::GET)
        .url(url)
        .header("Accept", "application/json, text/plain, */*")
        .send();

    curly_hpp::response response;
    try
    {
        response = request.take();
    }
    catch (const curly_hpp::exception& e)
    {
        log(ERROR, "getDataRefId: Failed to get dataref: %s", e.what());
        return -1;
    }

    int code = response.http_code();
    string content = response.content.as_string_copy();

    int64_t id = -1;
    if (code == 200)
    {
        auto json = json::parse(content);
        auto data = json["data"];
        if (data.is_array() && !data.empty())
        {
            auto datarefJson = data[0];
            const string& name = datarefJson["name"].get<string>();
            id = datarefJson["id"].get<int64_t>();
            m_dataRefIds[name] = id;
#ifdef DEBUG_XPLANE_WS
            log(DEBUG, "getDataRefId: %s -> %" PRId64, name.c_str(), id);
#endif
        }
    }
    else if (code == 404)
    {
        // Doesn't exist!
        log(WARN, "getDataRefId: %s -> doesn't exist", dataref.c_str());
    }
    else
    {
        log(WARN, "getDataRefId: Unexpected code=%d, content: %s\n", code, content.c_str());
    }
    return id;
}

bool XPlaneWebSocketClient::getDataRef(const string &dataref, json& valueJson)
{
    auto id = getDataRefId(dataref);
    if (id == -1)
    {
        return false;
    }

    string url = m_baseRestUrl + "/datarefs/" + to_string(id) + "/value";
#ifdef DEBUG_XPLANE_WS
    log(DEBUG, "getDataRef: Sending request to: %s", url.c_str());
#endif

    auto request = curly_hpp::request_builder()
        .method(curly_hpp::http_method::GET)
        .url(url)
        .header("Accept", "application/json, text/plain, */*")
        .send();

    curly_hpp::response response;
    try
    {
        response = request.take();
    }
    catch (const curly_hpp::exception& e)
    {
        log(ERROR, "getDataRef: Failed to get dataref: %s", e.what());
        return false;
    }

    int code = response.http_code();
    string content = response.content.as_string_copy();

    if (code == 200)
    {
        auto responseJson = json::parse(content);
        valueJson = responseJson["data"];

        return true;
    }
    else
    {
        log(WARN, "getDataRef: code=%d, content: %s\n", code, content.c_str());
    }
    return false;
}

int64_t XPlaneWebSocketClient::getCommandId(const string &command)
{
    if (command.empty())
    {
        return -1;
    }

    auto it = m_commandIds.find(command);
    if (it != m_commandIds.end())
    {
#ifdef DEBUG_XPLANE_WS
        log(DEBUG, "getCommandIds: Got: %s -> %" PRId64, command.c_str(), it->second);
#endif
        return it->second;
    }

    string query = "filter[name]=" + command;
#ifdef DEBUG_XPLANE_WS
    log(DEBUG, "getCommandId: Querying for %s...", command.c_str());
#endif

    string url = m_baseRestUrl + "/commands";//?" + query;
#ifdef DEBUG_XPLANE_WS
    log(DEBUG, "getCommandId: Sending request to: %s", url.c_str());
#endif
    auto request= curly_hpp::request_builder()
        .method(curly_hpp::http_method::GET)
        .qparam("filter[name]", command)
        .url(url)
        .header("Accept", "application/json, text/plain, */*")
        .send();

    curly_hpp::response response;
    try
    {
        response = request.take();
    }
    catch (const curly_hpp::exception& e)
    {
        log(ERROR, "getCommandId: Failed to get command: %s", e.what());
        return -1;
    }

    int code = response.http_code();
#ifdef DEBUG_XPLANE_WS
    log(DEBUG, "getCommandId: Got response: code=%d", code);
#endif
    string content = response.content.as_string_copy();
#ifdef DEBUG_XPLANE_WS
    log(DEBUG, "getCommandId: content: %s\n", content.c_str());
#endif

    int64_t id = -1;
    if (code == 200)
    {
        auto json = json::parse(content);
        auto data = json["data"];
        if (data.is_array() && !data.empty())
        {
            auto commandJson = data[0];
            const string& name = commandJson["name"].get<string>();
            id = commandJson["id"].get<int64_t>();
            m_commandIds[name] = id;
#ifdef DEBUG_XPLANE_WS
            log(DEBUG, "getCommandId: Got: %s -> %" PRId64 , name.c_str(), id);
#endif
        }
    }
    else if (code == 404)
    {
        log(DEBUG, "getCommandId: Command: %s -> doesn't exist", command.c_str());
    }
    else
    {
        log(WARN, "getCommandId: Failed to get command %s: code=%d, content: %s\n", command.c_str(), code, content.c_str());
    }
    return id;
}

struct read_ctx
{
    CURL* webSocket = nullptr;
    string payload;
    size_t payloadLength = 0;
    size_t bytesSent = 0;
};

static size_t read_cb(char *buf, size_t nitems, size_t buflen, void *p)
{
    auto ctx = static_cast<read_ctx*>(p);
    size_t len = nitems * buflen;
    size_t left = ctx->payloadLength - ctx->bytesSent;

    if (!ctx->bytesSent)
    {
        CURLcode result;
        /* On first call, set the FRAME information to be used (it defaults to
         * CURLWS_BINARY otherwise). */
        result = curl_ws_start_frame(ctx->webSocket, CURLWS_TEXT, static_cast<curl_off_t>(ctx->payloadLength));
        if(result != CURLE_OK)
        {
            fprintf(stderr, "error starting frame: %d\n", result);
            return CURL_READFUNC_ABORT;
        }
    }
    if (left)
    {
        if(left < len)
        {
            len = left;
        }
        memcpy(buf, ctx->payload.c_str() + ctx->bytesSent, len);
        ctx->bytesSent += len;
        return len;
    }
    return 0;
}

size_t XPlaneWebSocketClient::dataRefCallback(char* b, size_t size, size_t nitems, void *p)
{
    auto blen = nitems * size;
    auto data = static_cast<DataRefWebSocketInfo*>(p);
    auto meta = curl_ws_meta(data->ws);

    auto payload = string(b, blen);
    data->buffer += payload;

    if (meta == nullptr || !(meta->flags & CURLWS_CONT))
    {
        data->client->dataRefValues(data->buffer, data);
        data->buffer.clear();
    }

    return blen;
}

void XPlaneWebSocketClient::dataRefValues(const string &body, DataRefWebSocketInfo* dataRefWebSocketData)
{
    json dataRefValuesJson;
    try
    {
        dataRefValuesJson = json::parse(body);
    }
    catch (const json::parse_error& e)
    {
        log(ERROR, "dataRefValues: Failed to parse JSON: %s", e.what());
        log(ERROR, "dataRefValues: Received: %s", body.c_str());
        return;
    }
#ifdef DEBUG_XPLANE_WS
    log(INFO, "dataRefValues: Received: %s", body.c_str());
#endif

    json data = dataRefValuesJson["data"];
    map<int, float> values;
    for (auto const& value : data.items())
    {
        int64_t id = atoll(value.key().c_str());
        auto dataRefs = dataRefWebSocketData->dataRefIdx.at(id);

        for (const auto& dataRef : dataRefs)
        {
            float v = 0;
            if (value.value().is_array())
            {
                size_t index = 0;
                if (dataRef->mapping.dataRefIndex != -1)
                {
                    index = dataRef->mapping.dataRefIndex;
                }
                if (index < value.value().size())
                {
                    v = value.value().at(index).get<float>();
                }
            }
            else
            {
                v = value.value().get<float>();
            }

#ifdef DEBUG_XPLANE_WS
            log(DEBUG, "dataRefValues: %llu = %s = %s = %f", id, dataRef->id.c_str(), dataRef->mapping.dataRef.c_str(), v);
#endif
            values.insert(make_pair(dataRef->idx, v));
        }
    }
    dataRefWebSocketData->func->operator()(values);
}

Result XPlaneWebSocketClient::streamDataRefs(
    const vector<shared_ptr<DataDefinition>>& dataRefs,
    const function<void(map<int, float>)>& func,
    int count)
{
    json dataRefsJson;
    bool hasDataRef = false;
    auto dataRefWebSocketData = new DataRefWebSocketInfo();
    dataRefWebSocketData->client = this;
    dataRefWebSocketData->func = new function(func);
    for (const auto& dataref : dataRefs)
    {
        int64_t id = getDataRefId(dataref->mapping.dataRef);
        if (id != -1)
        {
            json dataRefJson;
            dataRefJson["id"] = id;
            dataRefsJson.push_back(dataRefJson);

            dataRefWebSocketData->dataRefIdx[id].push_back(dataref);
            hasDataRef = true;
       }
    }

#ifdef DEBUG_XPLANE_WS
    for (auto const& i : dataRefWebSocketData->dataRefIdx)
    {
        log(DEBUG, "streamDataRefs: dataRef: %llu: %d", i.first, i.second.size());
    }
#endif

    if (!hasDataRef)
    {
        log(WARN, "streamDataRefs: No valid datarefs to stream");
        return Result::FAIL;
    }

    json paramsJson;
    paramsJson["datarefs"] = dataRefsJson;

    json requestJson;
    requestJson["req_id"] = m_requestId++;
    requestJson["type"] = "dataref_subscribe_values";
    requestJson["params"] = paramsJson;

#ifdef DEBUG_XPLANE_WS
    log(DEBUG, "streamDataRefs: Sending request: %s", requestJson.dump().c_str());
#endif
    string url = m_baseWebSocketUrl;
#ifdef DEBUG_XPLANE_WS
    log(DEBUG, "streamDataRefs: Sending request to: %s", url.c_str());
#endif

    // curly_hpp doesn't support Web Sockets :-(
    CURL* webSocket = curl_easy_init();
    if (webSocket == nullptr)
    {
        log(ERROR, "streamDataRefs: Failed to initialize CURL");
        return Result::FAIL;
    }

    {
        scoped_lock lock(m_webSocketsMutex);
        dataRefWebSocketData->ws = webSocket;
        m_webSockets.insert(dataRefWebSocketData);
    }

    read_ctx rctx;
    rctx.webSocket = webSocket;
    rctx.payload = requestJson.dump();
    rctx.payloadLength = rctx.payload.size();
    rctx.bytesSent = 0;

    curl_easy_setopt(webSocket, CURLOPT_URL, url.c_str());
    curl_easy_setopt(webSocket, CURLOPT_WRITEFUNCTION, dataRefCallback);
    curl_easy_setopt(webSocket, CURLOPT_WRITEDATA, dataRefWebSocketData);
    curl_easy_setopt(webSocket, CURLOPT_READFUNCTION, read_cb);
    curl_easy_setopt(webSocket, CURLOPT_READDATA, &rctx);
    curl_easy_setopt(webSocket, CURLOPT_UPLOAD, 1L);

    CURLcode curlResult = curl_easy_perform(webSocket);
#ifdef DEBUG_XPLANE_WS
    log(INFO, "streamDataRefs: perform result=%d", curlResult);
#endif

    Result result = Result::SUCCESS;
    if (curlResult != CURLE_OK)
    {
        log(ERROR, "streamDataRefs: Failed to perform CURL request: %s", curl_easy_strerror(curlResult));
        result = Result::FAIL;
    }

    curl_easy_cleanup(webSocket);

    {
        scoped_lock lock(m_webSocketsMutex);
        m_webSockets.erase(dataRefWebSocketData);
    }

    delete dataRefWebSocketData;
#ifdef DEBUG_XPLANE_WS
    log(INFO, "streamDataRefs: Finished!");
#endif

    return result;
}
