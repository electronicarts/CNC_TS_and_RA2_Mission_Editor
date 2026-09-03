/*
    FinalAlert 2 MCP integration

    Copyright (C) 2026 FinalAlert 2 MCP contributors
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "stdafx.h"

#include "McpServer.h"
#include "McpConfigFile.h"
#include "functions.h"

#include "FinalSunDlg.h"
#include "McpMapTools.h"
#include "variables.h"

#include <bcrypt.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <deque>
#include <future>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

extern BOOL bNoDraw;

namespace
{
using Json = nlohmann::json;

constexpr const char* ModernProtocolVersion = "2026-07-28";
constexpr const char* LatestLegacyProtocolVersion = "2025-11-25";
constexpr const char* ServerName = "finalalert2-mcp";
constexpr const char* ServerVersion = "1.0.0";
constexpr std::size_t DefaultMaxRequestBytes = 16u * 1024u * 1024u;
constexpr std::size_t MinimumMaxRequestBytes = 1024u;
constexpr std::size_t MaximumMaxRequestBytes = 64u * 1024u * 1024u;
constexpr std::size_t MaximumOperationIdLength = 128u;
constexpr std::size_t MaximumEndpointLength = 256u;
constexpr std::size_t MaximumSessions = 128u;

const std::vector<std::string> LegacyProtocolVersions = {
    "2025-03-26",
    "2025-06-18",
    "2025-11-25"
};

std::string Trim(std::string value)
{
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](char c) { return !isSpace(static_cast<unsigned char>(c)); }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [&](char c) { return !isSpace(static_cast<unsigned char>(c)); }).base(), value.end());
    return value;
}

std::string Lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::vector<std::string> SplitList(const std::string& text)
{
    std::vector<std::string> result;
    std::stringstream stream(text);
    std::string item;
    while (std::getline(stream, item, ','))
    {
        item = Trim(item);
        if (!item.empty())
            result.push_back(item);
    }
    return result;
}

std::string ReadIniString(const std::string& path, const char* key, const char* fallback)
{
    std::vector<char> buffer(32768, 0);
    GetPrivateProfileStringA("Server", key, fallback, buffer.data(), static_cast<DWORD>(buffer.size()), path.c_str());
    return buffer.data();
}

std::string LiteralRoute(const std::string& path)
{
    // cpp-httplib interprets registered paths as regular expressions.
    std::string pattern;
    for (const char character : path)
    {
        // Also avoid httplib treating a '/:name' segment as a path parameter.
        if (character == ':')
        {
            pattern += "\\x3a";
            continue;
        }
        if (std::string(".^$|()[]{}*+?\\").find(character) != std::string::npos)
            pattern += '\\';
        pattern += character;
    }
    return pattern;
}

bool ConstantTimeEquals(const std::string& left, const std::string& right)
{
    const std::size_t maxSize = (std::max)(left.size(), right.size());
    unsigned char difference = left.size() == right.size() ? 0 : 1;
    for (std::size_t i = 0; i < maxSize; ++i)
    {
        const unsigned char a = i < left.size() ? static_cast<unsigned char>(left[i]) : 0;
        const unsigned char b = i < right.size() ? static_cast<unsigned char>(right[i]) : 0;
        difference |= static_cast<unsigned char>(a ^ b);
    }
    return difference == 0;
}

std::string RandomSessionId()
{
    unsigned char bytes[24] = {};
    if (BCryptGenRandom(nullptr, bytes, sizeof(bytes), BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0)
        return {};

    static constexpr char digits[] = "0123456789abcdef";
    std::string result;
    result.reserve(sizeof(bytes) * 2);
    for (unsigned char byte : bytes)
    {
        result.push_back(digits[byte >> 4]);
        result.push_back(digits[byte & 0x0F]);
    }
    return result;
}

Json ServerInfo()
{
    return Json{ {"name", ServerName}, {"version", ServerVersion} };
}

Json JsonRpcError(const Json& id, int code, const std::string& message, const Json& data = Json())
{
    Json error = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"error", {
            {"code", code},
            {"message", message}
        }}
    };
    if (!data.is_null())
        error["error"]["data"] = data;
    return error;
}

Json ToolFailure(const std::string& code, const std::string& message)
{
    return {
        {"content", Json::array({ Json{ {"type", "text"}, {"text", message} } })},
        {"structuredContent", Json{ {"ok", false}, {"code", code}, {"message", message} }},
        {"isError", true}
    };
}

void AddModernResultFields(Json& result)
{
    result["resultType"] = "complete";
    result["_meta"]["io.modelcontextprotocol/serverInfo"] = ServerInfo();
}

void SetJsonResponse(httplib::Response& response, const Json& body, int status = 200)
{
    response.status = status;
    response.set_content(body.dump(), "application/json; charset=utf-8");
    response.set_header("Cache-Control", "no-store");
}

bool IsSafeHeaderValue(const std::string& value)
{
    return std::all_of(value.begin(), value.end(), [](unsigned char character) {
        return character >= 0x20 && character != 0x7F && character != '\r' && character != '\n';
    });
}

bool IsSafeOperationId(const std::string& operationId)
{
    return !Trim(operationId).empty() && operationId.size() <= MaximumOperationIdLength &&
        IsSafeHeaderValue(operationId);
}

bool IsValidSessionId(const std::string& sessionId)
{
    return sessionId.size() == 48u && std::all_of(sessionId.begin(), sessionId.end(), [](unsigned char character) {
        return (character >= '0' && character <= '9') || (character >= 'a' && character <= 'f');
    });
}

bool IsValidOrigin(const std::string& value)
{
    const std::string origin = Lower(Trim(value));
    if (origin.empty() || origin == "null" || !IsSafeHeaderValue(origin) ||
        std::any_of(origin.begin(), origin.end(), [](unsigned char character) { return std::isspace(character) != 0; }))
    {
        return false;
    }

    const std::size_t schemeLength = origin.rfind("https://", 0) == 0 ? 8u :
        (origin.rfind("http://", 0) == 0 ? 7u : 0u);
    if (schemeLength == 0u || schemeLength == origin.size())
        return false;

    const std::string authority = origin.substr(schemeLength);
    return authority.front() != ':' && authority.back() != ':' &&
        origin.find_first_of("/\\@?#,", schemeLength) == std::string::npos;
}

struct ParsedAuthority
{
    std::string host;
    std::string port;
    bool hasPort = false;
};

bool ParsePort(const std::string& text)
{
    if (text.empty() || text.size() > 5 ||
        !std::all_of(text.begin(), text.end(), [](unsigned char character) { return std::isdigit(character) != 0; }))
    {
        return false;
    }

    unsigned int value = 0;
    for (const unsigned char character : text)
        value = value * 10u + static_cast<unsigned int>(character - '0');
    return value > 0u && value <= 65535u;
}

bool ParseAuthority(std::string authority, ParsedAuthority& result)
{
    authority = Lower(Trim(authority));
    if (authority.empty() || !IsSafeHeaderValue(authority) ||
        authority.find_first_of("/\\@?#,") != std::string::npos ||
        std::any_of(authority.begin(), authority.end(), [](unsigned char character) { return std::isspace(character) != 0; }))
    {
        return false;
    }

    ParsedAuthority parsed;
    if (authority.front() == '[')
    {
        const auto close = authority.find(']');
        if (close == std::string::npos || close == 1 || authority.find('[', 1) != std::string::npos ||
            authority.find(']', close + 1) != std::string::npos)
        {
            return false;
        }

        parsed.host = authority.substr(0, close + 1);
        const std::string suffix = authority.substr(close + 1);
        if (!suffix.empty())
        {
            if (suffix.front() != ':' || !ParsePort(suffix.substr(1)))
                return false;
            parsed.port = suffix.substr(1);
            parsed.hasPort = true;
        }
    }
    else
    {
        const auto colon = authority.find(':');
        if (colon != std::string::npos && authority.find(':', colon + 1) != std::string::npos)
            return false;

        parsed.host = authority.substr(0, colon);
        if (colon != std::string::npos)
        {
            parsed.port = authority.substr(colon + 1);
            parsed.hasPort = true;
            if (!ParsePort(parsed.port))
                return false;
        }

        if (parsed.host.empty() || !std::all_of(parsed.host.begin(), parsed.host.end(), [](unsigned char character) {
                return std::isalnum(character) != 0 || character == '.' || character == '-';
            }))
        {
            return false;
        }
    }

    result = std::move(parsed);
    return true;
}

bool IsLoopbackBindAddress(const std::string& address)
{
    const std::string normalized = Lower(Trim(address));
    return normalized == "localhost" || normalized == "127.0.0.1" || normalized == "::1" || normalized == "[::1]";
}

bool IsSuccessfulToolResult(const Json& result)
{
    if (!result.is_object() || result.contains("error"))
        return false;
    const auto isError = result.find("isError");
    return isError != result.end() && isError->is_boolean() && !isError->get<bool>();
}

bool IsLegacyVersion(const std::string& version)
{
    return std::find(LegacyProtocolVersions.begin(), LegacyProtocolVersions.end(), version) != LegacyProtocolVersions.end();
}

struct WindowSearchContext
{
    HWND mainWindow = nullptr;
    bool foundModal = false;
};

BOOL CALLBACK FindVisibleDialog(HWND window, LPARAM parameter)
{
    auto* context = reinterpret_cast<WindowSearchContext*>(parameter);
    if (window == context->mainWindow || !IsWindowVisible(window))
        return TRUE;

    char className[64] = {};
    GetClassNameA(window, className, static_cast<int>(sizeof(className) / sizeof(className[0])));
    if (strcmp(className, "#32770") == 0)
    {
        context->foundModal = true;
        return FALSE;
    }
    return TRUE;
}
}

struct CMcpServer::Impl
{
    struct UiRequest
    {
        std::string toolName;
        Json arguments;
        std::promise<Json> promise;
        std::atomic_bool cancelled = false;
    };

    struct Session
    {
        std::string protocolVersion;
    };

    std::unique_ptr<httplib::Server> http;
    std::thread listenerThread;
    std::atomic_bool running = false;
    std::atomic_bool stopping = false;
    std::atomic_bool executing = false;
    std::atomic<HWND> targetWindow{ nullptr };
    std::mutex lifecycleMutex;

    std::mutex queueMutex;
    std::deque<std::shared_ptr<UiRequest>> queue;

    std::mutex sessionMutex;
    std::unordered_map<std::string, Session> sessions;

    std::mutex mutationMutex;
    std::mutex operationCacheMutex;
    std::unordered_map<std::string, Json> operationCache;
    std::deque<std::string> operationOrder;

    std::string bindAddress = "127.0.0.1";
    int port = 7462;
    std::string endpoint = "/mcp";
    std::string authToken;
    std::vector<std::string> allowedHosts;
    std::vector<std::string> allowedOrigins;
    std::size_t maxRequestBytes = DefaultMaxRequestBytes;

    bool IsHostAllowed(const httplib::Request& request) const
    {
        if (request.get_header_value_count("Host") != 1)
            return false;

        ParsedAuthority requested;
        if (!ParseAuthority(request.get_header_value("Host"), requested))
            return false;

        for (const auto& configured : allowedHosts)
        {
            if (Trim(configured) == "*")
                return true;

            ParsedAuthority allowed;
            if (ParseAuthority(configured, allowed) && allowed.host == requested.host &&
                (!allowed.hasPort || (requested.hasPort && allowed.port == requested.port)))
            {
                return true;
            }
        }
        return false;
    }

    bool IsOriginAllowed(const httplib::Request& request) const
    {
        const std::size_t count = request.get_header_value_count("Origin");
        if (count == 0)
            return true;
        if (count != 1)
            return false;

        const std::string origin = Lower(Trim(request.get_header_value("Origin")));
        if (!IsValidOrigin(origin))
            return false;
        for (const auto& configured : allowedOrigins)
        {
            const std::string allowed = Lower(Trim(configured));
            if (allowed == "*" || allowed == origin)
                return true;
        }
        return false;
    }

    void ApplyCorsHeaders(const httplib::Request& request, httplib::Response& response) const
    {
        response.set_header("Vary", request.method == "OPTIONS"
            ? "Origin, Access-Control-Request-Method, Access-Control-Request-Headers"
            : "Origin");
        if (request.get_header_value_count("Origin") != 1 || !IsOriginAllowed(request))
            return;

        response.set_header("Access-Control-Allow-Origin", Trim(request.get_header_value("Origin")));
        response.set_header("Access-Control-Expose-Headers", "MCP-Session-Id");
    }

    bool Authorize(const httplib::Request& request, httplib::Response& response) const
    {
        if (!IsHostAllowed(request))
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32001, "Host is not allowed"), 403);
            return false;
        }
        if (!IsOriginAllowed(request))
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32001, "Origin is not allowed"), 403);
            return false;
        }
        if (!authToken.empty())
        {
            if (request.get_header_value_count("Authorization") != 1)
            {
                response.set_header("WWW-Authenticate", "Bearer");
                SetJsonResponse(response, JsonRpcError(nullptr, -32001, "Unauthorized"), 401);
                return false;
            }
            const std::string expected = "Bearer " + authToken;
            const std::string actual = request.get_header_value("Authorization");
            if (!ConstantTimeEquals(actual, expected))
            {
                response.set_header("WWW-Authenticate", "Bearer");
                SetJsonResponse(response, JsonRpcError(nullptr, -32001, "Unauthorized"), 401);
                return false;
            }
        }
        return true;
    }

    Json DispatchToUi(const std::string& name, const Json& arguments)
    {
        const HWND window = targetWindow.load(std::memory_order_acquire);
        if (stopping.load(std::memory_order_acquire) || !window || !IsWindow(window))
            return ToolFailure("server_stopping", "The editor is shutting down.");

        auto request = std::make_shared<UiRequest>();
        request->toolName = name;
        request->arguments = arguments;
        auto future = request->promise.get_future();
        {
            std::lock_guard lock(queueMutex);
            if (stopping.load(std::memory_order_acquire))
                return ToolFailure("server_stopping", "The editor is shutting down.");
            queue.push_back(request);
        }

        if (!PostMessage(window, WM_FINALALERT_MCP_DISPATCH, 0, 0))
        {
            request->cancelled.store(true, std::memory_order_release);
            std::lock_guard lock(queueMutex);
            const auto found = std::find(queue.begin(), queue.end(), request);
            if (found != queue.end())
                queue.erase(found);
            return ToolFailure("dispatch_failed", "Could not dispatch the command to the editor UI thread.");
        }

        if (future.wait_for(std::chrono::minutes(5)) != std::future_status::ready)
        {
            request->cancelled.store(true, std::memory_order_release);
            std::lock_guard lock(queueMutex);
            const auto found = std::find(queue.begin(), queue.end(), request);
            if (found != queue.end())
                queue.erase(found);
            return ToolFailure("editor_timeout", "The editor did not finish the command within five minutes; its final state may need to be checked.");
        }
        return future.get();
    }

    void CancelPending()
    {
        std::deque<std::shared_ptr<UiRequest>> pending;
        {
            std::lock_guard lock(queueMutex);
            pending.swap(queue);
        }
        for (const auto& request : pending)
        {
            request->cancelled.store(true, std::memory_order_release);
            try
            {
                request->promise.set_value(ToolFailure("server_stopping", "The editor is shutting down."));
            }
            catch (...)
            {
            }
        }
    }

    bool ValidateModernRequest(const httplib::Request& request, const Json& body, Json& error) const
    {
        const Json id = body.value("id", Json(nullptr));
        if (request.get_header_value_count("MCP-Protocol-Version") != 1 ||
            request.get_header_value_count("Mcp-Method") != 1)
        {
            error = JsonRpcError(id, -32020, "Exactly one MCP-Protocol-Version and Mcp-Method header is required");
            return false;
        }

        const std::string headerVersion = request.get_header_value("MCP-Protocol-Version");
        const std::string methodHeader = request.get_header_value("Mcp-Method");
        const std::string method = body.value("method", "");

        if (headerVersion != ModernProtocolVersion)
        {
            error = JsonRpcError(id, -32022, "Unsupported protocol version",
                Json{ {"supported", Json::array({ ModernProtocolVersion, LatestLegacyProtocolVersion, "2025-06-18", "2025-03-26" })} });
            return false;
        }

        if (!body.contains("params") || !body["params"].is_object() ||
            !body["params"].contains("_meta") || !body["params"]["_meta"].is_object())
        {
            error = JsonRpcError(id, -32602, "Modern MCP requests require params._meta");
            return false;
        }

        const Json& meta = body["params"]["_meta"];
        const auto metaVersion = meta.find("io.modelcontextprotocol/protocolVersion");
        if (metaVersion == meta.end() || !metaVersion->is_string() ||
            metaVersion->get<std::string>() != ModernProtocolVersion || methodHeader != method)
        {
            error = JsonRpcError(id, -32020, "Header mismatch");
            return false;
        }

        const bool nameBasedRequest = method == "tools/call" || method == "prompts/get";
        const bool uriBasedRequest = method == "resources/read";
        if (nameBasedRequest || uriBasedRequest)
        {
            const char* bodyField = uriBasedRequest ? "uri" : "name";
            if (request.get_header_value_count("Mcp-Name") != 1 ||
                !body["params"].contains(bodyField) || !body["params"][bodyField].is_string() ||
                request.get_header_value("Mcp-Name") != body["params"][bodyField].get<std::string>())
            {
                error = JsonRpcError(id, -32020, "Header mismatch");
                return false;
            }
        }
        else if (request.get_header_value_count("Mcp-Name") != 0)
        {
            error = JsonRpcError(id, -32020, "Mcp-Name is only valid for named requests");
            return false;
        }
        return true;
    }

    bool GetLegacySession(const httplib::Request& request, const Json& body, std::string& sessionId, Session& session, httplib::Response& response)
    {
        if (request.get_header_value_count("MCP-Session-Id") != 1)
        {
            SetJsonResponse(response, JsonRpcError(body.value("id", Json(nullptr)), -32000, "Exactly one MCP-Session-Id header is required"), 400);
            return false;
        }

        sessionId = request.get_header_value("MCP-Session-Id");
        if (!IsValidSessionId(sessionId))
        {
            SetJsonResponse(response, JsonRpcError(body.value("id", Json(nullptr)), -32000, "MCP-Session-Id is invalid"), 400);
            return false;
        }

        {
            std::lock_guard lock(sessionMutex);
            const auto found = sessions.find(sessionId);
            if (found == sessions.end())
            {
                SetJsonResponse(response, JsonRpcError(body.value("id", Json(nullptr)), -32000, "Unknown MCP session"), 404);
                return false;
            }
            session = found->second;
        }

        if (request.get_header_value_count("MCP-Protocol-Version") > 1)
        {
            SetJsonResponse(response, JsonRpcError(body.value("id", Json(nullptr)), -32020, "Duplicate MCP-Protocol-Version headers are not allowed"), 400);
            return false;
        }

        const std::string version = request.get_header_value("MCP-Protocol-Version");
        if (!version.empty() && version != session.protocolVersion)
        {
            SetJsonResponse(response, JsonRpcError(body.value("id", Json(nullptr)), -32022, "Protocol version does not match the session"), 400);
            return false;
        }
        response.set_header("MCP-Session-Id", sessionId);
        return true;
    }

    Json InvokeTool(const std::string& sessionScope, const std::string& name, const Json& arguments)
    {
        if (!CMcpMapTools::HasTool(name))
            return JsonRpcError(nullptr, -32602, "Unknown tool: " + name);

        const bool mutation = CMcpMapTools::IsMutation(name);
        std::unique_lock<std::mutex> mutationLock;
        std::string operationKey;
        if (mutation)
        {
            if (!arguments.contains("operationId") || !arguments["operationId"].is_string())
                return JsonRpcError(nullptr, -32602, "Mutating tools require a non-empty operationId");

            const std::string operationId = arguments["operationId"].get<std::string>();
            if (!IsSafeOperationId(operationId))
                return JsonRpcError(nullptr, -32602, "operationId must contain 1 to 128 printable characters");

            mutationLock = std::unique_lock<std::mutex>(mutationMutex);
            operationKey = sessionScope + "\n" + name + "\n" + operationId;
            {
                std::lock_guard cacheLock(operationCacheMutex);
                const auto cached = operationCache.find(operationKey);
                if (cached != operationCache.end())
                {
                    Json replay = cached->second;
                    if (!replay.contains("structuredContent") || !replay["structuredContent"].is_object())
                        replay["structuredContent"] = Json::object();
                    replay["structuredContent"]["replayed"] = true;
                    return replay;
                }
            }
        }

        Json result = DispatchToUi(name, arguments);
        if (mutation && IsSuccessfulToolResult(result))
        {
            std::lock_guard cacheLock(operationCacheMutex);
            operationCache[operationKey] = result;
            operationOrder.push_back(operationKey);
            while (operationOrder.size() > 256)
            {
                operationCache.erase(operationOrder.front());
                operationOrder.pop_front();
            }
        }
        return result;
    }

    void HandlePost(const httplib::Request& request, httplib::Response& response)
    {
        if (!Authorize(request, response))
            return;

        if (request.body.size() > maxRequestBytes)
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32003, "Request body exceeds the configured limit"), 413);
            return;
        }

        if (request.get_header_value_count("Content-Type") != 1)
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32600, "Exactly one Content-Type header is required"), 415);
            return;
        }

        const std::string contentType = Lower(Trim(request.get_header_value("Content-Type")));
        const auto contentTypeSeparator = contentType.find(';');
        const std::string mediaType = Trim(contentType.substr(0, contentTypeSeparator));
        if (mediaType != "application/json")
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32600, "Content-Type must be application/json"), 415);
            return;
        }

        Json body;
        try
        {
            body = Json::parse(request.body);
        }
        catch (const std::exception&)
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32700, "Parse error"), 400);
            return;
        }

        if (!body.is_object() || !body.contains("jsonrpc") || !body["jsonrpc"].is_string() ||
            body["jsonrpc"].get<std::string>() != "2.0" || !body.contains("method") || !body["method"].is_string())
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32600, "Invalid JSON-RPC request"), 400);
            return;
        }

        if (body.contains("id") && !(body["id"].is_null() || body["id"].is_string() || body["id"].is_number()))
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32600, "JSON-RPC id must be a string, number, or null"), 400);
            return;
        }

        const Json id = body.value("id", Json(nullptr));
        const bool notification = !body.contains("id");
        const std::string method = body["method"].get<std::string>();
        if (method.empty() || method.size() > 128 || !IsSafeHeaderValue(method))
        {
            SetJsonResponse(response, JsonRpcError(id, -32600, "Invalid JSON-RPC method"), 400);
            return;
        }

        if (request.get_header_value_count("MCP-Protocol-Version") > 1)
        {
            SetJsonResponse(response, JsonRpcError(id, -32020, "Duplicate MCP-Protocol-Version headers are not allowed"), 400);
            return;
        }

        const std::string headerVersion = request.get_header_value("MCP-Protocol-Version");
        const bool modern = headerVersion == ModernProtocolVersion;

        if (!headerVersion.empty() && headerVersion.rfind("2026-", 0) == 0 && !modern)
        {
            SetJsonResponse(response, JsonRpcError(id, -32022, "Unsupported protocol version",
                Json{ {"supported", Json::array({ ModernProtocolVersion, LatestLegacyProtocolVersion, "2025-06-18", "2025-03-26" })} }), 400);
            return;
        }

        if (modern)
        {
            Json validationError;
            if (!ValidateModernRequest(request, body, validationError))
            {
                SetJsonResponse(response, validationError, 400);
                return;
            }
        }

        std::string sessionId;
        Session session;
        if (method == "initialize" && !modern)
        {
            if (!body.contains("params") || !body["params"].is_object())
            {
                SetJsonResponse(response, JsonRpcError(id, -32602, "initialize params are required"), 400);
                return;
            }

            if (body["params"].contains("protocolVersion") && !body["params"]["protocolVersion"].is_string())
            {
                SetJsonResponse(response, JsonRpcError(id, -32602, "initialize protocolVersion must be a string"), 400);
                return;
            }

            const std::string requested = body["params"].contains("protocolVersion")
                ? body["params"]["protocolVersion"].get<std::string>()
                : std::string(LatestLegacyProtocolVersion);
            if (!headerVersion.empty() && headerVersion != requested)
            {
                SetJsonResponse(response, JsonRpcError(id, -32020, "MCP-Protocol-Version does not match initialize params"), 400);
                return;
            }
            const std::string negotiated = IsLegacyVersion(requested) ? requested : LatestLegacyProtocolVersion;
            sessionId = RandomSessionId();
            if (sessionId.empty())
            {
                SetJsonResponse(response, JsonRpcError(id, -32603, "Could not create a secure MCP session"), 500);
                return;
            }
            {
                std::lock_guard lock(sessionMutex);
                if (sessions.size() >= MaximumSessions)
                {
                    SetJsonResponse(response, JsonRpcError(id, -32000, "Too many active MCP sessions"), 503);
                    return;
                }
                sessions[sessionId] = Session{ negotiated };
            }
            response.set_header("MCP-Session-Id", sessionId);
            Json result = {
                {"protocolVersion", negotiated},
                {"capabilities", Json{ {"tools", Json{ {"listChanged", false} }} }},
                {"serverInfo", ServerInfo()},
                {"instructions", "Control the map currently open in FinalAlert 2. Read map state before editing, use unique operationId values for every mutation, prefer apply_map_plan for coordinated batches, and save to a new file until the result has been reviewed."}
            };
            SetJsonResponse(response, Json{ {"jsonrpc", "2.0"}, {"id", id}, {"result", result} });
            return;
        }

        if (!modern)
        {
            if (!GetLegacySession(request, body, sessionId, session, response))
                return;
        }

        if (method == "notifications/initialized")
        {
            if (modern)
            {
                SetJsonResponse(response, JsonRpcError(id, -32601, "initialize notifications are not part of MCP 2026-07-28"), 400);
                return;
            }
            response.status = 202;
            response.body.clear();
            return;
        }

        if (notification)
        {
            response.status = 202;
            response.body.clear();
            return;
        }

        Json result;
        if (method == "server/discover" && modern)
        {
            result = {
                {"resultType", "complete"},
                {"supportedVersions", Json::array({ ModernProtocolVersion, LatestLegacyProtocolVersion, "2025-06-18", "2025-03-26" })},
                {"capabilities", Json{ {"tools", Json::object()} }},
                {"instructions", "Control the map currently open in FinalAlert 2. Use a unique operationId for every mutation."},
                {"_meta", Json{ {"io.modelcontextprotocol/serverInfo", ServerInfo()} }}
            };
        }
        else if (method == "ping")
        {
            result = Json::object();
            if (modern)
                AddModernResultFields(result);
        }
        else if (method == "tools/list")
        {
            result = { {"tools", CMcpMapTools::ListTools()} };
            if (modern)
            {
                result["ttlMs"] = 300000;
                result["cacheScope"] = "public";
                AddModernResultFields(result);
            }
        }
        else if (method == "tools/call")
        {
            if (!body.contains("params") || !body["params"].is_object() || !body["params"].contains("name") || !body["params"]["name"].is_string())
            {
                SetJsonResponse(response, JsonRpcError(id, -32602, "tools/call requires a tool name"), 400);
                return;
            }
            const std::string name = body["params"]["name"].get<std::string>();
            if (name.empty() || name.size() > 128u || !IsSafeHeaderValue(name))
            {
                SetJsonResponse(response, JsonRpcError(id, -32602, "Invalid tool name"), 400);
                return;
            }
            Json arguments = body["params"].value("arguments", Json::object());
            if (!arguments.is_object())
            {
                SetJsonResponse(response, JsonRpcError(id, -32602, "Tool arguments must be an object"), 400);
                return;
            }

            result = InvokeTool(modern ? "modern" : sessionId, name, arguments);
            if (result.contains("error"))
            {
                result["id"] = id;
                SetJsonResponse(response, result, 400);
                return;
            }
            if (modern)
                AddModernResultFields(result);
        }
        else
        {
            SetJsonResponse(response, JsonRpcError(id, -32601, "Method not found: " + method), 404);
            return;
        }

        SetJsonResponse(response, Json{ {"jsonrpc", "2.0"}, {"id", id}, {"result", result} });
    }

    void HandleOptions(const httplib::Request& request, httplib::Response& response) const
    {
        if (!IsHostAllowed(request))
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32001, "Host is not allowed"), 403);
            return;
        }
        if (request.get_header_value_count("Origin") != 1 || !IsOriginAllowed(request))
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32001, "Origin is not allowed"), 403);
            return;
        }
        if (request.get_header_value_count("Access-Control-Request-Method") != 1)
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32600, "Access-Control-Request-Method is required"), 400);
            return;
        }

        const std::string requestedMethod = Lower(Trim(request.get_header_value("Access-Control-Request-Method")));
        if (requestedMethod != "post" && requestedMethod != "delete")
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32600, "Requested CORS method is not allowed"), 405);
            return;
        }

        if (request.get_header_value_count("Access-Control-Request-Headers") > 1)
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32600, "Duplicate Access-Control-Request-Headers headers are not allowed"), 400);
            return;
        }

        static const std::unordered_set<std::string> allowedHeaders = {
            "accept",
            "authorization",
            "content-type",
            "mcp-method",
            "mcp-name",
            "mcp-protocol-version",
            "mcp-session-id"
        };
        if (request.has_header("Access-Control-Request-Headers"))
        {
            for (const auto& header : SplitList(request.get_header_value("Access-Control-Request-Headers")))
            {
                if (allowedHeaders.find(Lower(header)) == allowedHeaders.end())
                {
                    SetJsonResponse(response, JsonRpcError(nullptr, -32600, "Requested CORS header is not allowed"), 403);
                    return;
                }
            }
        }

        response.set_header("Access-Control-Allow-Methods", "POST, DELETE, OPTIONS");
        response.set_header("Access-Control-Allow-Headers", "Accept, Content-Type, Authorization, MCP-Protocol-Version, MCP-Session-Id, Mcp-Method, Mcp-Name");
        response.set_header("Access-Control-Max-Age", "600");
        response.status = 204;
    }

    void HandleDelete(const httplib::Request& request, httplib::Response& response)
    {
        if (!Authorize(request, response))
            return;
        if (request.get_header_value_count("MCP-Protocol-Version") > 1)
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32020, "Duplicate MCP-Protocol-Version headers are not allowed"), 400);
            return;
        }
        if (request.get_header_value("MCP-Protocol-Version") == ModernProtocolVersion)
        {
            response.status = 405;
            response.set_header("Allow", "POST, OPTIONS");
            return;
        }
        if (request.get_header_value_count("MCP-Session-Id") != 1)
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32000, "Exactly one MCP-Session-Id header is required"), 400);
            return;
        }
        const std::string sessionId = request.get_header_value("MCP-Session-Id");
        if (!IsValidSessionId(sessionId))
        {
            SetJsonResponse(response, JsonRpcError(nullptr, -32000, "MCP-Session-Id is invalid"), 400);
            return;
        }
        {
            std::lock_guard lock(sessionMutex);
            if (sessions.erase(sessionId) == 0)
            {
                SetJsonResponse(response, JsonRpcError(nullptr, -32000, "Unknown MCP session"), 404);
                return;
            }
        }

        {
            const std::string prefix = sessionId + "\n";
            std::lock_guard cacheLock(operationCacheMutex);
            for (auto iterator = operationCache.begin(); iterator != operationCache.end();)
            {
                if (iterator->first.rfind(prefix, 0) == 0)
                    iterator = operationCache.erase(iterator);
                else
                    ++iterator;
            }
            operationOrder.erase(std::remove_if(operationOrder.begin(), operationOrder.end(), [&](const std::string& key) {
                return key.rfind(prefix, 0) == 0;
            }), operationOrder.end());
        }
        response.status = 204;
    }
};

CMcpServer::Configuration CMcpServer::LoadConfiguration()
{
    const std::string path = std::string(AppPath) + "FinalAlertMCP.ini";
    Configuration config;
    config.enabled = GetPrivateProfileIntA("Server", "Enabled", 1, path.c_str()) != 0;
    config.port = GetPrivateProfileIntA("Server", "Port", 7462, path.c_str());
    config.maxRequestBytes = GetPrivateProfileIntA("Server", "MaxRequestBytes", static_cast<UINT>(DefaultMaxRequestBytes), path.c_str());
    config.bindAddress = Trim(ReadIniString(path, "BindAddress", "127.0.0.1"));
    config.endpoint = Trim(ReadIniString(path, "Endpoint", "/mcp"));
    config.authToken = Trim(ReadIniString(path, "AuthToken", ""));
    config.allowedHosts = ReadIniString(path, "AllowedHosts", "localhost,127.0.0.1,[::1]");
    config.allowedOrigins = ReadIniString(path, "AllowedOrigins", "https://chatgpt.com,https://chat.openai.com");
    return config;
}

bool CMcpServer::SaveConfiguration(const Configuration& config, std::string& error)
{
    error.clear();
    if (config.port == 0 || config.port > 65535)
        error = "Invalid MCP port. Use a number from 1 to 65535.";
    else if (config.bindAddress != "127.0.0.1")
        error = "MCP must listen on 127.0.0.1.";
    else if (config.endpoint.empty() || config.endpoint.front() != '/' || config.endpoint == "/health" ||
        config.endpoint.size() > MaximumEndpointLength ||
        config.endpoint.find_first_of("?#\\\r\n\t ") != std::string::npos ||
        !IsSafeHeaderValue(config.endpoint))
        error = "Invalid MCP endpoint. Use an absolute path other than /health, without spaces, query or fragment.";
    else if (config.authToken.size() > 32760 || std::any_of(config.authToken.begin(), config.authToken.end(),
        [](unsigned char ch) { return ch < 33 || ch > 126 || ch == '\"' || ch == '\''; }))
        error = "Invalid MCP authentication token. Use printable ASCII characters without spaces or quotes.";
    else if (config.maxRequestBytes < MinimumMaxRequestBytes || config.maxRequestBytes > MaximumMaxRequestBytes)
        error = "MCP request limit must be between 1024 and 67108864 bytes.";
    if (!error.empty()) return false;

    const auto hosts = SplitList(config.allowedHosts);
    if (hosts.empty() || config.allowedHosts.size() > 32760 || !IsSafeHeaderValue(config.allowedHosts))
        error = "Invalid MCP allowed hosts.";
    for (const auto& host : hosts)
    {
        ParsedAuthority authority;
        if (host != "*" && !ParseAuthority(host, authority))
            error = "Invalid MCP allowed hosts.";
    }
    if (!error.empty()) return false;
    if (config.allowedOrigins.size() > 32760 ||
        (!config.allowedOrigins.empty() && !IsSafeHeaderValue(config.allowedOrigins)))
        error = "Invalid MCP allowed origins.";
    for (const auto& origin : SplitList(config.allowedOrigins))
        if (origin != "*" && !IsValidOrigin(origin))
            error = "Invalid MCP allowed origins.";
    if (!error.empty()) return false;

    // Values are validated before any file is changed; never log the token.
    const bool saved = McpConfigFile::Save(utf8ToUtf16(std::string(AppPath) + "FinalAlertMCP.ini"), {
        {L"Enabled", config.enabled ? L"1" : L"0"},
        {L"BindAddress", L"127.0.0.1"},
        {L"Port", std::to_wstring(config.port)},
        {L"Endpoint", utf8ToUtf16(config.endpoint)},
        {L"AuthToken", utf8ToUtf16(config.authToken)},
        {L"AllowedHosts", utf8ToUtf16(config.allowedHosts)},
        {L"AllowedOrigins", utf8ToUtf16(config.allowedOrigins)},
        {L"MaxRequestBytes", std::to_wstring(config.maxRequestBytes)}
    });
    if (!saved) error = "Could not save MCP settings. Check write access to the editor folder.";
    return saved;
}

CMcpServer::CMcpServer()
    : m_impl(std::make_unique<Impl>())
{
}

CMcpServer::~CMcpServer()
{
    Stop();
}

bool CMcpServer::Start(HWND targetWindow)
{
    if (!m_impl || !targetWindow || !IsWindow(targetWindow))
        return false;

    std::lock_guard lifecycleLock(m_impl->lifecycleMutex);
    if (m_impl->running.load(std::memory_order_acquire))
    {
        m_impl->targetWindow.store(targetWindow, std::memory_order_release);
        return true;
    }

    if (m_impl->listenerThread.joinable())
        m_impl->listenerThread.join();
    m_impl->http.reset();

    const Configuration config = LoadConfiguration();
    if (!config.enabled) return false;
    const std::string& bindAddress = config.bindAddress;
    const UINT configuredPort = config.port;
    std::string endpoint = config.endpoint;
    if (endpoint.empty() || endpoint.front() != '/')
        endpoint.insert(endpoint.begin(), '/');
    const std::string& authToken = config.authToken;
    const std::vector<std::string> allowedHosts = SplitList(config.allowedHosts);
    const std::vector<std::string> allowedOrigins = SplitList(config.allowedOrigins);
    const UINT configuredMaxRequestBytes = config.maxRequestBytes;

    if (bindAddress.empty() || configuredPort == 0 || configuredPort > 65535 || allowedHosts.empty() ||
        endpoint.size() > MaximumEndpointLength || endpoint.find_first_of("?#\r\n\t ") != std::string::npos || endpoint == "/health")
    {
        return false;
    }
    if (!authToken.empty() && (!IsSafeHeaderValue(authToken) ||
        std::any_of(authToken.begin(), authToken.end(), [](unsigned char character) { return std::isspace(character) != 0; })))
    {
        return false;
    }
    if (!IsLoopbackBindAddress(bindAddress) && authToken.empty())
        return false;

    for (const auto& configured : allowedHosts)
    {
        ParsedAuthority parsed;
        if (Trim(configured) != "*" && !ParseAuthority(configured, parsed))
            return false;
    }
    for (const auto& configured : allowedOrigins)
    {
        const std::string origin = Lower(Trim(configured));
        if (origin != "*" && !IsValidOrigin(origin))
            return false;
    }

    m_impl->bindAddress = bindAddress;
    m_impl->port = static_cast<int>(configuredPort);
    m_impl->endpoint = std::move(endpoint);
    m_impl->authToken = authToken;
    m_impl->allowedHosts = allowedHosts;
    m_impl->allowedOrigins = allowedOrigins;
    m_impl->maxRequestBytes = configuredMaxRequestBytes >= MinimumMaxRequestBytes && configuredMaxRequestBytes <= MaximumMaxRequestBytes
        ? static_cast<std::size_t>(configuredMaxRequestBytes)
        : DefaultMaxRequestBytes;
    m_impl->targetWindow.store(targetWindow, std::memory_order_release);
    m_impl->stopping.store(false, std::memory_order_release);
    m_impl->executing.store(false, std::memory_order_release);

    m_impl->http = std::make_unique<httplib::Server>();
    m_impl->http->set_payload_max_length(m_impl->maxRequestBytes);
    m_impl->http->set_read_timeout(30, 0);
    m_impl->http->set_write_timeout(30, 0);
    m_impl->http->set_idle_interval(0, 100000);
    m_impl->http->set_error_handler(httplib::Server::HandlerWithResponse([](const httplib::Request&, httplib::Response& response) {
        const std::string contentType = Lower(response.get_header_value("Content-Type"));
        if (contentType.rfind("application/json", 0) == 0)
            return httplib::Server::HandlerResponse::Unhandled;

        const int status = response.status > 0 ? response.status : 500;
        const int code = status == 413 ? -32003 : (status == 404 ? -32601 : -32600);
        const char* message = status == 413 ? "Request body exceeds the configured limit" :
            (status == 404 ? "Endpoint not found" : "HTTP request failed");
        SetJsonResponse(response, JsonRpcError(nullptr, code, message), status);
        return httplib::Server::HandlerResponse::Handled;
    }));
    m_impl->http->set_exception_handler([](const httplib::Request&, httplib::Response& response, std::exception_ptr) {
        SetJsonResponse(response, JsonRpcError(nullptr, -32603, "Internal server error"), 500);
    });
    m_impl->http->set_post_routing_handler([impl = m_impl.get()](const httplib::Request& request, httplib::Response& response) {
        impl->ApplyCorsHeaders(request, response);
    });

    const std::string route = LiteralRoute(m_impl->endpoint);
    m_impl->http->Post(route, [impl = m_impl.get()](const httplib::Request& request, httplib::Response& response) {
        impl->HandlePost(request, response);
    });
    m_impl->http->Get(route, [impl = m_impl.get()](const httplib::Request& request, httplib::Response& response) {
        if (!impl->Authorize(request, response))
            return;
        response.status = 405;
        response.set_header("Allow", "POST, DELETE, OPTIONS");
    });
    m_impl->http->Delete(route, [impl = m_impl.get()](const httplib::Request& request, httplib::Response& response) {
        impl->HandleDelete(request, response);
    });
    m_impl->http->Options(route, [impl = m_impl.get()](const httplib::Request& request, httplib::Response& response) {
        impl->HandleOptions(request, response);
    });
    m_impl->http->Get("/health", [impl = m_impl.get()](const httplib::Request& request, httplib::Response& response) {
        if (!impl->Authorize(request, response))
            return;
        SetJsonResponse(response, Json{ {"status", "ok"}, {"server", ServerName}, {"version", ServerVersion}, {"mcpEndpoint", impl->endpoint} });
    });

    if (!m_impl->http->bind_to_port(m_impl->bindAddress, m_impl->port))
    {
        m_impl->http.reset();
        m_impl->targetWindow.store(nullptr, std::memory_order_release);
        return false;
    }

    m_impl->running.store(true, std::memory_order_release);
    httplib::Server* const boundServer = m_impl->http.get();
    try
    {
        m_impl->listenerThread = std::thread([impl = m_impl.get(), boundServer]() {
            boundServer->listen_after_bind();
            impl->running.store(false, std::memory_order_release);
        });
    }
    catch (...)
    {
        m_impl->running.store(false, std::memory_order_release);
        boundServer->stop();
        m_impl->http.reset();
        m_impl->targetWindow.store(nullptr, std::memory_order_release);
        return false;
    }
    return true;
}

void CMcpServer::Stop()
{
    if (!m_impl)
        return;

    std::lock_guard lifecycleLock(m_impl->lifecycleMutex);
    m_impl->stopping.store(true, std::memory_order_release);
    m_impl->targetWindow.store(nullptr, std::memory_order_release);
    m_impl->CancelPending();
    if (m_impl->http)
        m_impl->http->stop();
    if (m_impl->listenerThread.joinable())
        m_impl->listenerThread.join();
    m_impl->http.reset();
    m_impl->running.store(false, std::memory_order_release);
    m_impl->executing.store(false, std::memory_order_release);
    {
        std::lock_guard lock(m_impl->sessionMutex);
        m_impl->sessions.clear();
    }
    {
        std::lock_guard lock(m_impl->operationCacheMutex);
        m_impl->operationCache.clear();
        m_impl->operationOrder.clear();
    }
}

void CMcpServer::ProcessPendingRequests(CFinalSunDlg& dialog)
{
    std::deque<std::shared_ptr<Impl::UiRequest>> requests;
    {
        std::lock_guard lock(m_impl->queueMutex);
        requests.swap(m_impl->queue);
    }

    for (const auto& request : requests)
    {
        if (request->cancelled.load(std::memory_order_acquire))
            continue;

        Json result;
        try
        {
            WindowSearchContext context{ dialog.GetSafeHwnd(), false };
            EnumThreadWindows(GetCurrentThreadId(), FindVisibleDialog, reinterpret_cast<LPARAM>(&context));
            const bool editorBusy = !dialog.IsWindowEnabled() || context.foundModal || bNoDraw ||
                (dialog.m_view.m_isoview && dialog.m_view.m_isoview->b_IsLoading);
            const bool alreadyExecuting = m_impl->executing.exchange(true, std::memory_order_acq_rel);

            if (editorBusy || alreadyExecuting)
            {
                result = ToolFailure("editor_busy", "FinalAlert 2 is busy or a modal dialog is open. Close the dialog and retry; mutations must reuse the same operationId.");
            }
            else
            {
                result = CMcpMapTools::Execute(request->toolName, request->arguments, dialog);
                m_impl->executing.store(false, std::memory_order_release);
            }
            if (!alreadyExecuting)
                m_impl->executing.store(false, std::memory_order_release);
        }
        catch (const std::exception& exception)
        {
            m_impl->executing.store(false, std::memory_order_release);
            result = ToolFailure("internal_error", exception.what());
        }
        catch (...)
        {
            m_impl->executing.store(false, std::memory_order_release);
            result = ToolFailure("internal_error", "An unknown native exception occurred while executing the tool.");
        }

        try
        {
            request->promise.set_value(std::move(result));
        }
        catch (...)
        {
        }
    }
}

bool CMcpServer::IsRunning() const
{
    return m_impl && m_impl->running.load(std::memory_order_acquire);
}
