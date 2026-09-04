#pragma once

#include <windows.h>
#include <string>
#include <utility>
#include <vector>

namespace McpConfigFile
{
// Profile APIs preserve comments and unrelated settings. Stage the complete
// update next to the destination, so a failed write leaves the live file intact.
inline bool Save(const std::wstring& path, const std::vector<std::pair<std::wstring, std::wstring>>& values)
{
    const auto separator = path.find_last_of(L"\\/");
    if (separator == std::wstring::npos) return false;
    const std::wstring directory = path.substr(0, separator + 1);
    wchar_t temporary[MAX_PATH] = {};
    if (!GetTempFileNameW(directory.c_str(), L"mcp", 0, temporary)) return false;
    struct Cleanup
    {
        const wchar_t* path;
        ~Cleanup() { DeleteFileW(path); }
    } cleanup{ temporary };

    const DWORD attributes = GetFileAttributesW(path.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES)
    {
        if ((attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY)) ||
            !CopyFileW(path.c_str(), temporary, FALSE)) return false;
    }
    else if (GetLastError() != ERROR_FILE_NOT_FOUND)
        return false;

    for (const auto& value : values)
        if (!WritePrivateProfileStringW(L"Server", value.first.c_str(), value.second.c_str(), temporary))
            return false;
    WritePrivateProfileStringW(nullptr, nullptr, nullptr, temporary);
    if (!MoveFileExW(temporary, path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        return false;
    WritePrivateProfileStringW(nullptr, nullptr, nullptr, path.c_str());
    return true;
}
}
