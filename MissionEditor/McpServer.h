/*
    FinalAlert 2 MCP integration

    Copyright (C) 2026 FinalAlert 2 MCP contributors
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <Windows.h>

#include <memory>
#include <string>

class CFinalSunDlg;

constexpr UINT WM_FINALALERT_MCP_DISPATCH = WM_APP + 0x4A2;

class CMcpServer
{
public:
    struct Configuration
    {
        bool enabled = true;
        unsigned int port = 7462;
        unsigned int maxRequestBytes = 16777216;
        std::string bindAddress = "127.0.0.1";
        std::string endpoint = "/mcp";
        std::string authToken;
        std::string allowedHosts = "localhost,127.0.0.1,[::1]";
        std::string allowedOrigins = "https://chatgpt.com,https://chat.openai.com";
    };

    static Configuration LoadConfiguration();
    // Saves for the next editor launch. Never interrupts an active MCP session.
    static bool SaveConfiguration(const Configuration& configuration, std::string& error);

    CMcpServer();
    ~CMcpServer();

    CMcpServer(const CMcpServer&) = delete;
    CMcpServer& operator=(const CMcpServer&) = delete;

    bool Start(HWND targetWindow);
    void Stop();
    void ProcessPendingRequests(CFinalSunDlg& dialog);
    bool IsRunning() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
