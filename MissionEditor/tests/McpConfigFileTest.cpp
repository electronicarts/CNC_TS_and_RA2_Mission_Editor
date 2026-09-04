// Standalone v143 Win32 test for configuration persistence, no editor needed.
#include "../McpConfigFile.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

std::string Bytes(const std::wstring& path)
{
    std::ifstream file(std::filesystem::path(path), std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

std::wstring Value(const std::wstring& path, const wchar_t* section, const wchar_t* key)
{
    wchar_t text[1024] = {};
    GetPrivateProfileStringW(section, key, L"missing", text, 1024, path.c_str());
    return text;
}

int main()
{
    wchar_t temp[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, temp);
    const std::wstring directory = std::wstring(temp) + L"fa2-mcp-config-" + std::to_wstring(GetCurrentProcessId()) + L"-\u0457";
    const std::wstring path = directory + L"\\FinalAlertMCP.ini";
    try
    {
        Check(CreateDirectoryW(directory.c_str(), nullptr) != FALSE, "Create unique test directory");
        {
            std::ofstream file(std::filesystem::path(path), std::ios::binary);
            file << "; Keep user comments\r\n[Server]\r\nEnabled=1\r\nPort=7462\r\n"
                "AuthToken=sample-test-token\r\nCustomOption=keep-me\r\n[Other]\r\nValue=untouched\r\n";
        }
        Check(McpConfigFile::Save(path, {{L"Port", L"7463"}, {L"Enabled", L"0"}}), "Save to Unicode path");
        Check(Value(path, L"Server", L"Port") == L"7463", "New port readback");
        Check(Value(path, L"Server", L"Enabled") == L"0", "Disabled state readback");
        Check(Value(path, L"Server", L"AuthToken") == L"sample-test-token", "Preserve unchanged token");
        Check(Value(path, L"Server", L"CustomOption") == L"keep-me", "Preserve unknown key");
        Check(Value(path, L"Other", L"Value") == L"untouched", "Preserve unrelated section");
        Check(Bytes(path).find("; Keep user comments") != std::string::npos, "Preserve comments");
        const auto beforeFailure = Bytes(path);
        Check(SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_READONLY) != FALSE, "Protect test file");
        Check(!McpConfigFile::Save(path, {{L"Port", L"9000"}}), "Reject read-only destination");
        Check(Bytes(path) == beforeFailure, "Failed save leaves original unchanged");
        SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
        Check(McpConfigFile::Save(path, {{L"AuthToken", L""}}), "Clear token explicitly");
        Check(Value(path, L"Server", L"AuthToken").empty(), "Cleared token readback");
        Check(!McpConfigFile::Save(directory + L"\\absent\\FinalAlertMCP.ini", {{L"Port", L"9000"}}), "Reject absent parent");
        const std::wstring fresh = directory + L"\\fresh.ini";
        Check(McpConfigFile::Save(fresh, {{L"Port", L"7462"}}), "Create missing configuration");
        Check(Value(fresh, L"Server", L"Port") == L"7462", "Fresh configuration readback");
        DeleteFileW(fresh.c_str());
        DeleteFileW(path.c_str());
        Check(RemoveDirectoryW(directory.c_str()) != FALSE, "No staging files left behind");
        std::cout << "MCP configuration persistence tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
        DeleteFileW(path.c_str());
        RemoveDirectoryW(directory.c_str());
        std::cerr << error.what() << '\n';
        return 1;
    }
}
