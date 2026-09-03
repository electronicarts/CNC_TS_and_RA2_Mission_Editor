/*
    FinalAlert 2 MCP map tools

    Copyright (C) 2026 FinalAlert 2 MCP contributors
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nlohmann/json.hpp>
#include <string>

class CFinalSunDlg;

class CMcpMapTools
{
public:
    using Json = nlohmann::json;

    static Json ListTools();
    static bool HasTool(const std::string& name);
    static bool IsMutation(const std::string& name);
    static Json Execute(const std::string& name, const Json& arguments, CFinalSunDlg& dialog);
};
