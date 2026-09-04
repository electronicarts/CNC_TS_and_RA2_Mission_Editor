/*
    FinalAlert 2 MCP integration

    Copyright (C) 2026 FinalAlert 2 MCP contributors
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "stdafx.h"
#include "McpSettingsDlg.h"
#include "functions.h"

namespace
{
void ExchangeString(CDataExchange* exchange, int control, std::string& value)
{
    CString text(value.c_str());
    DDX_Text(exchange, control, text);
    if (exchange->m_bSaveAndValidate) value = text.GetString();
}

void ExchangeUnsigned(CDataExchange* exchange, int control, unsigned int& value,
    unsigned int minimum, unsigned int maximum, const char* error)
{
    CString text;
    text.Format("%u", value);
    DDX_Text(exchange, control, text);
    if (!exchange->m_bSaveAndValidate) return;

    unsigned int parsed = 0;
    bool valid = !text.IsEmpty();
    for (int index = 0; valid && index < text.GetLength(); ++index)
    {
        const char character = text[index];
        if (character < '0' || character > '9')
        {
            valid = false;
            break;
        }
        const unsigned int digit = character - '0';
        if (parsed > (maximum - digit) / 10)
        {
            valid = false;
            break;
        }
        parsed = parsed * 10 + digit;
    }
    if (!valid || parsed < minimum || parsed > maximum)
    {
        AfxMessageBox(TranslateStringACP(error), MB_OK | MB_ICONEXCLAMATION);
        exchange->PrepareEditCtrl(control);
        exchange->Fail();
        return;
    }
    value = parsed;
}
}

CMcpSettingsDlg::CMcpSettingsDlg(bool running, CWnd* parent)
    : CDialog(IDD, parent)
    , m_configuration(CMcpServer::LoadConfiguration())
    , m_enabled(m_configuration.enabled ? TRUE : FALSE)
    , m_running(running)
{
    // The settings UI cannot expose the editor outside this computer.
    m_configuration.bindAddress = "127.0.0.1";
}

void CMcpSettingsDlg::DoDataExchange(CDataExchange* exchange)
{
    CDialog::DoDataExchange(exchange);
    DDX_Check(exchange, IDC_MCP_ENABLED, m_enabled);
    ExchangeUnsigned(exchange, IDC_MCP_PORT, m_configuration.port, 1, 65535,
        "Invalid MCP port. Use a number from 1 to 65535.");
    ExchangeString(exchange, IDC_MCP_ENDPOINT, m_configuration.endpoint);
    ExchangeString(exchange, IDC_MCP_AUTH_TOKEN, m_configuration.authToken);
    ExchangeString(exchange, IDC_MCP_ALLOWED_HOSTS, m_configuration.allowedHosts);
    ExchangeString(exchange, IDC_MCP_ALLOWED_ORIGINS, m_configuration.allowedOrigins);
    ExchangeUnsigned(exchange, IDC_MCP_MAX_REQUEST_BYTES, m_configuration.maxRequestBytes,
        1024, 67108864, "MCP request limit must be between 1024 and 67108864 bytes.");
    if (exchange->m_bSaveAndValidate) m_configuration.enabled = m_enabled != FALSE;
}

BEGIN_MESSAGE_MAP(CMcpSettingsDlg, CDialog)
    ON_EN_CHANGE(IDC_MCP_PORT, OnEndpointChanged)
    ON_EN_CHANGE(IDC_MCP_ENDPOINT, OnEndpointChanged)
END_MESSAGE_MAP()

BOOL CMcpSettingsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetDlgItemText(IDC_MCP_BIND_ADDRESS, "127.0.0.1");
    SetDlgItemText(IDC_MCP_STATUS, TranslateStringACP(
        m_running ? "MCP server is running." : "MCP server is stopped."));
    m_initialized = true;
    OnEndpointChanged();
    return TRUE;
}

void CMcpSettingsDlg::OnEndpointChanged()
{
    if (!m_initialized) return;
    CString port;
    CString endpoint;
    GetDlgItemText(IDC_MCP_PORT, port);
    GetDlgItemText(IDC_MCP_ENDPOINT, endpoint);
    CString url;
    url.Format("http://127.0.0.1:%s%s", port.GetString(), endpoint.GetString());
    SetDlgItemText(IDC_MCP_URL, url);
}

void CMcpSettingsDlg::OnOK()
{
    if (!UpdateData(TRUE)) return;
    std::string error;
    if (!CMcpServer::SaveConfiguration(m_configuration, error))
    {
        AfxMessageBox(TranslateStringACP(error.c_str()), MB_OK | MB_ICONEXCLAMATION);
        return;
    }
    // Keep the current listener and client operation caches alive until restart.
    EndDialog(IDOK);
}
