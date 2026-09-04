/*
    FinalAlert 2 MCP integration

    Copyright (C) 2026 FinalAlert 2 MCP contributors
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "McpServer.h"
#include "resource.h"

class CMcpSettingsDlg : public CDialog
{
public:
    explicit CMcpSettingsDlg(bool running, CWnd* parent = nullptr);

    enum { IDD = IDD_MCP_SETTINGS };

protected:
    void DoDataExchange(CDataExchange* exchange) override;
    BOOL OnInitDialog() override;
    void OnOK() override;
    afx_msg void OnEndpointChanged();
    DECLARE_MESSAGE_MAP()

private:
    CMcpServer::Configuration m_configuration;
    BOOL m_enabled;
    bool m_running;
    bool m_initialized = false;
};
