/*
    FinalAlert 2 MCP map tools

    Copyright (C) 2026 FinalAlert 2 MCP contributors
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "stdafx.h"

#include "McpMapTools.h"

#include "FinalSunDlg.h"
#include "MapCode.h"
#include "MapData.h"
#include "functions.h"
#include "variables.h"

#include <wincodec.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace
{
using Json = nlohmann::json;

thread_local int BatchDepth = 0;

std::string LowerAscii(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

class ToolInputError : public std::runtime_error
{
public:
    explicit ToolInputError(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

enum class RefreshKind
{
    Cells,
    Objects,
    Full
};

Json ToolSuccess(const std::string& message, Json data = Json::object())
{
    if (!data.is_object())
        data = Json{ {"value", std::move(data)} };
    data["ok"] = true;
    return {
        {"content", Json::array({ Json{ {"type", "text"}, {"text", message} } })},
        {"structuredContent", std::move(data)},
        {"isError", false}
    };
}

Json ToolFailure(const std::string& code, const std::string& message, Json details = Json::object())
{
    if (!details.is_object())
        details = Json{ {"details", std::move(details)} };
    details["ok"] = false;
    details["code"] = code;
    details["message"] = message;
    return {
        {"content", Json::array({ Json{ {"type", "text"}, {"text", message} } })},
        {"structuredContent", std::move(details)},
        {"isError", true}
    };
}

bool IsToolError(const Json& result)
{
    return result.value("isError", false);
}

Json StringSchema(const std::string& description, const std::vector<std::string>& values = {})
{
    Json schema = { {"type", "string"}, {"description", description} };
    if (!values.empty())
        schema["enum"] = values;
    return schema;
}

Json IntegerSchema(const std::string& description, int minimum = (std::numeric_limits<int>::min)(), int maximum = (std::numeric_limits<int>::max)())
{
    Json schema = { {"type", "integer"}, {"description", description} };
    if (minimum != (std::numeric_limits<int>::min)())
        schema["minimum"] = minimum;
    if (maximum != (std::numeric_limits<int>::max)())
        schema["maximum"] = maximum;
    return schema;
}

Json NumberSchema(const std::string& description, double minimum, double maximum)
{
    return { {"type", "number"}, {"description", description}, {"minimum", minimum}, {"maximum", maximum} };
}

Json BooleanSchema(const std::string& description)
{
    return { {"type", "boolean"}, {"description", description} };
}

Json OperationIdSchema()
{
    return StringSchema("Unique idempotency key (UUID recommended). Reuse it only when retrying exactly the same mutation.");
}

Json MakeTool(
    const std::string& name,
    const std::string& title,
    const std::string& description,
    Json properties,
    std::vector<std::string> required,
    bool readOnly,
    bool destructive,
    bool idempotent,
    bool openWorld = false)
{
    if (!readOnly)
    {
        properties["operationId"] = OperationIdSchema();
        required.push_back("operationId");
    }

    Json inputSchema = {
        {"$schema", "https://json-schema.org/draft/2020-12/schema"},
        {"type", "object"},
        {"properties", std::move(properties)},
        {"additionalProperties", false}
    };
    if (!required.empty())
        inputSchema["required"] = required;

    return {
        {"name", name},
        {"title", title},
        {"description", description},
        {"inputSchema", std::move(inputSchema)},
        {"annotations", {
            {"title", title},
            {"readOnlyHint", readOnly},
            {"destructiveHint", destructive},
            {"idempotentHint", idempotent},
            {"openWorldHint", openWorld}
        }}
    };
}

const std::vector<std::string>& ToolNames()
{
    static const std::vector<std::string> names = {
        "finalalert.get_status",
        "finalalert.get_map_info",
        "finalalert.get_cells",
        "finalalert.list_catalog",
        "finalalert.list_objects",
        "finalalert.get_preview",
        "finalalert.validate_map",
        "finalalert.get_ini_section",
        "finalalert.create_map",
        "finalalert.import_semantic_image",
        "finalalert.open_map",
        "finalalert.save_map",
        "finalalert.set_map_metadata",
        "finalalert.set_cells",
        "finalalert.paint_region",
        "finalalert.place_resources",
        "finalalert.place_waypoint",
        "finalalert.set_player_spawns",
        "finalalert.place_object",
        "finalalert.delete_at",
        "finalalert.set_ini_value",
        "finalalert.apply_map_plan",
        "finalalert.undo_terrain",
        "finalalert.redo_terrain"
    };
    return names;
}

const std::unordered_set<std::string>& ReadOnlyTools()
{
    static const std::unordered_set<std::string> names = {
        "finalalert.get_status",
        "finalalert.get_map_info",
        "finalalert.get_cells",
        "finalalert.list_catalog",
        "finalalert.list_objects",
        "finalalert.get_preview",
        "finalalert.validate_map",
        "finalalert.get_ini_section"
    };
    return names;
}

void RequireMap()
{
    if (!Map || Map->GetIsoSize() == 0)
        throw ToolInputError("No map is loaded. Call finalalert.create_map or finalalert.open_map first.");
}

int RequireInteger(const Json& arguments, const char* name, int minimum, int maximum)
{
    if (!arguments.contains(name) || !arguments[name].is_number_integer())
        throw ToolInputError(std::string("'") + name + "' must be an integer.");
    const int value = arguments[name].get<int>();
    if (value < minimum || value > maximum)
        throw ToolInputError(std::string("'") + name + "' must be between " + std::to_string(minimum) + " and " + std::to_string(maximum) + ".");
    return value;
}

int OptionalInteger(const Json& arguments, const char* name, int fallback, int minimum, int maximum)
{
    if (!arguments.contains(name))
        return fallback;
    return RequireInteger(arguments, name, minimum, maximum);
}

double OptionalNumber(const Json& arguments, const char* name, double fallback, double minimum, double maximum)
{
    if (!arguments.contains(name))
        return fallback;
    if (!arguments[name].is_number())
        throw ToolInputError(std::string("'") + name + "' must be a number.");
    const double value = arguments[name].get<double>();
    if (value < minimum || value > maximum)
        throw ToolInputError(std::string("'") + name + "' is out of range.");
    return value;
}

std::string RequireString(const Json& arguments, const char* name)
{
    if (!arguments.contains(name) || !arguments[name].is_string())
        throw ToolInputError(std::string("'") + name + "' must be a string.");
    const std::string value = arguments[name].get<std::string>();
    if (value.empty())
        throw ToolInputError(std::string("'") + name + "' must not be empty.");
    return value;
}

std::string OptionalString(const Json& arguments, const char* name, const std::string& fallback = {})
{
    if (!arguments.contains(name))
        return fallback;
    return RequireString(arguments, name);
}

bool OptionalBoolean(const Json& arguments, const char* name, bool fallback)
{
    if (!arguments.contains(name))
        return fallback;
    if (!arguments[name].is_boolean())
        throw ToolInputError(std::string("'") + name + "' must be a boolean.");
    return arguments[name].get<bool>();
}

CString ToCString(int value)
{
    CString result;
    result.Format("%d", value);
    return result;
}

std::string ToUtf8(const CString& value)
{
    return std::string(value.GetString());
}

bool IsPlayableCell(int x, int y)
{
    if (!Map)
        return false;
    const int iso = static_cast<int>(Map->GetIsoSize());
    const int width = Map->GetWidth();
    const int height = Map->GetHeight();
    return x >= 1 && y >= 1 && x < iso && y < iso &&
        x + y >= width + 1 &&
        x + y <= width + height * 2 &&
        !(y + 1 > width && x - 1 < y - width) &&
        !(x + 1 > width && y + width - 1 < x);
}

DWORD RequirePosition(const Json& arguments, int& x, int& y, bool playable = true)
{
    RequireMap();
    const int iso = static_cast<int>(Map->GetIsoSize());
    x = RequireInteger(arguments, "x", 0, iso - 1);
    y = RequireInteger(arguments, "y", 0, iso - 1);
    if (playable && !IsPlayableCell(x, y))
        throw ToolInputError("The requested coordinate is outside the playable diamond.");
    return static_cast<DWORD>(x + y * iso);
}

void RefreshEditor(CFinalSunDlg& dialog, RefreshKind kind)
{
    if (BatchDepth > 0)
        return;
    if (!Map || Map->GetIsoSize() == 0)
        return;

    if (kind == RefreshKind::Full)
    {
        if (dialog.m_view.m_isoview)
            dialog.m_view.m_isoview->UpdateDialog(TRUE);
        dialog.UpdateDialogs();
        return;
    }

    Map->RedrawMinimap();
    if (dialog.m_view.m_isoview)
    {
        dialog.m_view.m_isoview->UpdateDialog(FALSE);
        dialog.m_view.m_isoview->RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
    }
    if (kind == RefreshKind::Objects)
        dialog.UpdateDialogs(FALSE, TRUE);
    dialog.m_view.m_minimap.UpdateView();
    dialog.m_view.m_minimap.RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}

void BeginCellTransaction()
{
    RequireMap();
    Map->TakeSnapshot(TRUE);
}

void CommitCellTransaction()
{
    Map->TakeSnapshot(TRUE);
    Map->Undo();
}

bool SectionContainsValue(const CIniFileSection& section, const CString& value)
{
    for (const auto& pair : section.values)
    {
        if (pair.second.CompareNoCase(value) == 0 || pair.first.CompareNoCase(value) == 0)
            return true;
    }
    return false;
}

bool TypeExists(const CString& sectionName, const CString& type)
{
    const auto rulesSection = rules.GetSection(sectionName);
    if (rulesSection && SectionContainsValue(*rulesSection, type))
        return true;
    if (Map)
    {
        const auto mapSection = Map->GetIniFile().GetSection(sectionName);
        if (mapSection && SectionContainsValue(*mapSection, type))
            return true;
    }
    return false;
}

bool HouseExists(const CString& house)
{
    if (house.CompareNoCase("Neutral") == 0 || house.CompareNoCase("Special") == 0)
        return true;
    if (const auto section = rules.GetSection(HOUSES); section && SectionContainsValue(*section, house))
        return true;
    if (Map)
    {
        if (const auto section = Map->GetIniFile().GetSection(MAPHOUSES); section && SectionContainsValue(*section, house))
            return true;
    }
    return false;
}

bool IsTargetRa2()
{
    if (!Map)
        return false;
    CString target = Map->GetIniFile().GetValueByName("FinalAlertMCP", "TargetGame", "auto");
    return target.CompareNoCase("ra2") == 0;
}

bool IsYrOnlyType(const std::string& kind, const CString& type)
{
    CString section;
    if (kind == "structure") section = "YRBuildings";
    else if (kind == "unit") section = "YRUnits";
    else if (kind == "infantry") section = "YRInfantry";
    else if (kind == "aircraft") section = "YRAircraft";
    else if (kind == "terrain") section = "YRTerrain";
    else return false;
    const auto found = g_data.GetSection(section);
    return found && (found->values.find(type) != found->values.end() || SectionContainsValue(*found, type));
}

DWORD ResolveTerrainTile(const std::string& terrain, int tileIndex = 0)
{
    RequireMap();
    if (!tiles || !tiledata || !tiledata_count || !*tiledata || *tiledata_count == 0)
        throw ToolInputError("The theater tile catalog is not loaded.");

    static const std::unordered_map<std::string, const char*> keys = {
        {"clear", "ClearTile"},
        {"green", "GreenTile"},
        {"rough", "RoughTile"},
        {"sand", "SandTile"},
        {"pave", "PaveTile"},
        {"water", "WaterSet"}
    };
    const auto found = keys.find(terrain);
    if (found == keys.end())
        throw ToolInputError("Unknown terrain semantic. Use clear, green, rough, sand, pave, or water.");
    const int tileSet = atoi(tiles->GetValueByName("General", found->second, "-1"));
    if (tileSet < 0)
        throw ToolInputError("This theater does not define the requested terrain semantic.");
    if (terrain == "water" && tileIndex == 0)
        tileIndex = 8;
    DWORD tileId = Map->GetTileID(tileSet, tileIndex);
    if (tileId == 0xFFFFFFFF || tileId >= *tiledata_count)
    {
        tileId = Map->GetTileID(tileSet, 0);
        if (tileId == 0xFFFFFFFF || tileId >= *tiledata_count)
            throw ToolInputError("Could not resolve the requested theater tile set.");
    }
    return tileId;
}

void ValidateTile(DWORD tileId, DWORD subTile)
{
    if (!tiledata || !tiledata_count || !*tiledata || tileId >= *tiledata_count)
        throw ToolInputError("tileId is outside the loaded theater catalog.");
    if (subTile >= (*tiledata)[tileId].wTileCount)
        throw ToolInputError("subTile is outside the selected tile.");
}

std::string Base64Encode(const unsigned char* data, std::size_t size)
{
    static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    result.reserve(((size + 2) / 3) * 4);
    for (std::size_t i = 0; i < size; i += 3)
    {
        const unsigned value = (static_cast<unsigned>(data[i]) << 16) |
            (i + 1 < size ? static_cast<unsigned>(data[i + 1]) << 8 : 0) |
            (i + 2 < size ? static_cast<unsigned>(data[i + 2]) : 0);
        result.push_back(alphabet[(value >> 18) & 63]);
        result.push_back(alphabet[(value >> 12) & 63]);
        result.push_back(i + 1 < size ? alphabet[(value >> 6) & 63] : '=');
        result.push_back(i + 2 < size ? alphabet[value & 63] : '=');
    }
    return result;
}

std::vector<unsigned char> Base64Decode(std::string text)
{
    const auto comma = text.find(',');
    if (text.rfind("data:", 0) == 0 && comma != std::string::npos)
        text = text.substr(comma + 1);

    std::array<int, 256> table;
    table.fill(-1);
    const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (int i = 0; i < static_cast<int>(alphabet.size()); ++i)
        table[static_cast<unsigned char>(alphabet[i])] = i;

    std::vector<unsigned char> result;
    int value = 0;
    int bits = -8;
    for (unsigned char c : text)
    {
        if (std::isspace(c))
            continue;
        if (c == '=')
            break;
        if (table[c] < 0)
            throw ToolInputError("imageBase64 contains invalid base64 data.");
        value = (value << 6) | table[c];
        bits += 6;
        if (bits >= 0)
        {
            result.push_back(static_cast<unsigned char>((value >> bits) & 0xFF));
            bits -= 8;
        }
    }
    if (result.empty())
        throw ToolInputError("imageBase64 decoded to an empty image.");
    return result;
}

std::vector<unsigned char> BuildMinimapBmp(int& width, int& height)
{
    RequireMap();
    Map->RedrawMinimap();
    BYTE* pixels = nullptr;
    BITMAPINFO info = {};
    int pitch = 0;
    Map->GetMinimap(&pixels, &info, &pitch);
    width = info.bmiHeader.biWidth;
    height = std::abs(info.bmiHeader.biHeight);
    if (!pixels || width <= 0 || height <= 0 || pitch <= 0)
        throw ToolInputError("The current map has no renderable minimap.");

    const std::size_t pixelBytes = static_cast<std::size_t>(pitch) * height;
    BITMAPFILEHEADER fileHeader = {};
    fileHeader.bfType = 0x4D42;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHeader.bfSize = static_cast<DWORD>(fileHeader.bfOffBits + pixelBytes);

    std::vector<unsigned char> result(fileHeader.bfSize);
    memcpy(result.data(), &fileHeader, sizeof(fileHeader));
    memcpy(result.data() + sizeof(fileHeader), &info.bmiHeader, sizeof(BITMAPINFOHEADER));
    memcpy(result.data() + fileHeader.bfOffBits, pixels, pixelBytes);
    return result;
}

uint32_t CoordinateHash(int x, int y, uint32_t seed)
{
    uint32_t value = seed ^ (static_cast<uint32_t>(x) * 0x9E3779B1u) ^ (static_cast<uint32_t>(y) * 0x85EBCA77u);
    value ^= value >> 16;
    value *= 0x7FEB352Du;
    value ^= value >> 15;
    value *= 0x846CA68Bu;
    return value ^ (value >> 16);
}

void SetBasicMetadata(const Json& arguments)
{
    RequireMap();
    CIniFile& ini = Map->GetIniFile();
    if (arguments.contains("name"))
        ini.sections["Basic"].values["Name"] = RequireString(arguments, "name").c_str();
    if (arguments.contains("author"))
        ini.sections["Basic"].values["Author"] = RequireString(arguments, "author").c_str();
    if (arguments.contains("description"))
        ini.sections["Basic"].values["Description"] = RequireString(arguments, "description").c_str();
    if (arguments.contains("gameMode"))
        ini.sections["Basic"].values["GameMode"] = RequireString(arguments, "gameMode").c_str();
    if (arguments.contains("minPlayers"))
        ini.sections["Basic"].values["MinPlayer"] = ToCString(RequireInteger(arguments, "minPlayers", 1, 8));
    if (arguments.contains("maxPlayers"))
        ini.sections["Basic"].values["MaxPlayer"] = ToCString(RequireInteger(arguments, "maxPlayers", 1, 8));
    if (arguments.contains("targetGame"))
    {
        const std::string game = RequireString(arguments, "targetGame");
        if (game != "ra2" && game != "yr" && game != "auto")
            throw ToolInputError("targetGame must be ra2, yr, or auto.");
        ini.sections["FinalAlertMCP"].values["TargetGame"] = game.c_str();
    }
    if (arguments.contains("multiplayer"))
    {
        const bool multiplayer = OptionalBoolean(arguments, "multiplayer", false);
        ini.sections["Basic"].values["MultiplayerOnly"] = multiplayer ? "1" : "0";
        if (multiplayer)
            ini.sections["Basic"].values.erase("Player");
        else
            ini.sections["Basic"].values["Player"] = OptionalString(arguments, "playerHouse", "Player").c_str();
    }
}

void PrepareMultiplayerHeader()
{
    if (!Map || !Map->IsMultiplayer())
        return;
    CIniFile& ini = Map->GetIniFile();
    int count = 0;
    int xValues[8] = {};
    int yValues[8] = {};
    for (DWORD index = 0; index < Map->GetWaypointCount(); ++index)
    {
        CString id;
        DWORD position = 0;
        Map->GetWaypointData(index, &id, &position);
        const int numericId = atoi(id);
        if (numericId >= 0 && numericId < 8 && count < 8)
        {
            const int x = position % Map->GetIsoSize();
            const int y = position / Map->GetIsoSize();
            xValues[count] = calcXPos(y, x);
            yValues[count] = calcYPos(y, x);
            ++count;
        }
    }

    CIniFileSection& header = ini.sections["Header"];
    header.values["NumberStartingPoints"] = ToCString(count);
    for (int i = 0; i < 8; ++i)
    {
        CString key;
        key.Format("Waypoint%d", i + 1);
        CString value;
        value.Format("%d,%d", xValues[i], yValues[i]);
        header.values[key] = value;
    }
    int startX = 0;
    int startY = 0;
    int width = 0;
    int height = 0;
    MC_GetHeaderRect(startX, startY, width, height);
    header.values["StartX"] = ToCString(startX);
    header.values["StartY"] = ToCString(startY);
    header.values["Width"] = ToCString(width);
    header.values["Height"] = ToCString(height);
    ini.sections["Basic"].values["MinPlayer"] = ini.sections["Basic"].GetValueByName("MinPlayer", "2");
    ini.sections["Basic"].values["MaxPlayer"] = ToCString((std::max)(2, count));
}

bool AtomicWriteMap(const std::string& path, bool includePreview, bool compressed, bool createDirectories, std::string& error)
{
    namespace fs = std::filesystem;
    fs::path target = fs::u8path(path);
    if (!target.is_absolute())
    {
        error = "save path must be absolute";
        return false;
    }
    const std::string extension = LowerAscii(utf16ToUtf8(target.extension().wstring()));
    if (extension != ".map" && extension != ".mpr" && extension != ".yrm")
    {
        error = "save path must end in .map, .mpr, or .yrm";
        return false;
    }
    std::error_code filesystemError;
    if (createDirectories)
        fs::create_directories(target.parent_path(), filesystemError);
    if (filesystemError || !fs::exists(target.parent_path()))
    {
        error = "destination directory does not exist or could not be created";
        return false;
    }

    PrepareMultiplayerHeader();
    DWORD flags = MAPDATA_UPDATE_TO_INI_ALL;
    if (includePreview)
        flags |= MAPDATA_UPDATE_TO_INI_ALL_PREVIEW;
    if (compressed)
        flags |= MAPDATA_UPDATE_TO_INI_ALL_COMPRESSED;
    Map->UpdateIniFile(flags);
    CIniFile& ini = Map->GetIniFile();
    ini.DeleteLeadingSpaces(TRUE, TRUE);
    ini.DeleteEndingSpaces(TRUE, TRUE);

    std::ostringstream text;
    text << "; Map created with FinalAlert 2 MCP Editor\n";
    text << "; Generated through validated in-process map tools\n\n";
    for (const auto& sectionPair : ini.sections)
    {
        if (sectionPair.first.IsEmpty() || sectionPair.second.values.empty())
            continue;
        text << '[' << sectionPair.first.GetString() << "]\n";
        for (const auto& valuePair : sectionPair.second.values)
            text << valuePair.first.GetString() << '=' << valuePair.second.GetString() << "\n";
        text << "\n";
    }
    const std::string bytes = text.str();

    fs::path temporary = target;
    temporary += L".mcp-tmp-" + std::to_wstring(GetCurrentProcessId());
    const std::wstring temporaryWide = temporary.wstring();
    HANDLE file = CreateFileW(temporaryWide.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        error = "could not create the temporary map file";
        return false;
    }

    DWORD written = 0;
    const bool writeOk = bytes.size() <= MAXDWORD &&
        WriteFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) &&
        written == bytes.size() && FlushFileBuffers(file);
    CloseHandle(file);
    if (!writeOk)
    {
        DeleteFileW(temporaryWide.c_str());
        error = "writing the temporary map file failed";
        return false;
    }

    if (!MoveFileExW(temporaryWide.c_str(), target.wstring().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        DeleteFileW(temporaryWide.c_str());
        error = "could not atomically replace the destination map";
        return false;
    }
    return true;
}

std::string CStringToUtf8(const CString& value)
{
    return std::string(value.GetString(), value.GetLength());
}

Json CoordinateJson(int x, int y)
{
    return { {"x", x}, {"y", y} };
}

Json CurrentMapInfo()
{
    RequireMap();
    CIniFile& ini = Map->GetIniFile();
    const CString name = ini.GetValueByName("Basic", "Name", "Unnamed");
    const CString author = ini.GetValueByName("Basic", "Author", "");
    const CString description = ini.GetValueByName("Basic", "Description", "");
    const CString target = ini.GetValueByName("FinalAlertMCP", "TargetGame", Map->IsYRMap() ? "yr" : "ra2");
    return {
        {"loaded", true},
        {"width", Map->GetWidth()},
        {"height", Map->GetHeight()},
        {"isoSize", Map->GetIsoSize()},
        {"theater", CStringToUtf8(Map->GetTheater())},
        {"name", CStringToUtf8(name)},
        {"author", CStringToUtf8(author)},
        {"description", CStringToUtf8(description)},
        {"targetGame", CStringToUtf8(target)},
        {"isYuriMap", Map->IsYRMap() != FALSE},
        {"multiplayer", Map->IsMultiplayer() != FALSE},
        {"money", Map->CalcMoneyOnMap()},
        {"waypoints", Map->GetWaypointCount()},
        {"structures", Map->GetStructureCount()},
        {"units", Map->GetUnitCount()},
        {"infantry", Map->GetInfantryCount()},
        {"aircraft", Map->GetAircraftCount()},
        {"terrainObjects", Map->GetTerrainCount()},
        {"currentFile", std::string(currentMapFile)}
    };
}

bool HasObjectAt(DWORD position, const std::string& kind)
{
    if (kind == "structure") return Map->GetStructureAt(position) >= 0;
    if (kind == "unit") return Map->GetUnitAt(position) >= 0;
    if (kind == "aircraft") return Map->GetAirAt(position) >= 0;
    if (kind == "infantry") return Map->GetInfantryAt(position) >= 0;
    if (kind == "terrain") return Map->GetTerrainAt(position) >= 0;
    return false;
}

bool AnyBlockingObjectAt(DWORD position)
{
    return Map->GetStructureAt(position) >= 0 || Map->GetUnitAt(position) >= 0 ||
        Map->GetAirAt(position) >= 0 || Map->GetTerrainAt(position) >= 0;
}

CString CatalogSectionForKind(const std::string& kind)
{
    if (kind == "structure" || kind == "building") return "BuildingTypes";
    if (kind == "unit" || kind == "vehicle") return "VehicleTypes";
    if (kind == "infantry") return "InfantryTypes";
    if (kind == "aircraft") return "AircraftTypes";
    if (kind == "terrain") return "TerrainTypes";
    if (kind == "house") return "Houses";
    throw ToolInputError("kind must be structure, unit, infantry, aircraft, terrain, or house.");
}

std::vector<CString> CatalogValues(const CString& sectionName)
{
    std::vector<CString> result;
    std::unordered_set<std::string> seen;
    const auto append = [&](const CIniFileSection* section) {
        if (!section)
            return;
        for (const auto& pair : section->values)
        {
            const CString& value = pair.second;
            const std::string key = LowerAscii(CStringToUtf8(value));
            if (!value.IsEmpty() && seen.insert(key).second)
                result.push_back(value);
        }
    };
    append(rules.GetSection(sectionName));
    if (Map)
        append(Map->GetIniFile().GetSection(sectionName));
    return result;
}

struct SemanticColor
{
    int r = 0;
    int g = 0;
    int b = 0;
};

SemanticColor ReadColor(COLORREF value)
{
    return { GetRValue(value), GetGValue(value), GetBValue(value) };
}

int ColorDistance(const SemanticColor& left, int r, int g, int b)
{
    return std::abs(left.r - r) + std::abs(left.g - g) + std::abs(left.b - b);
}

enum class ImageMarker
{
    None,
    Spawn,
    Ore,
    Gems,
    Location
};

ImageMarker DetectMarker(const SemanticColor& color, int tolerance)
{
    if (ColorDistance(color, 255, 0, 0) <= tolerance * 3) return ImageMarker::Spawn;
    if (ColorDistance(color, 255, 128, 0) <= tolerance * 3) return ImageMarker::Ore;
    if (ColorDistance(color, 255, 0, 255) <= tolerance * 3) return ImageMarker::Gems;
    if (ColorDistance(color, 0, 255, 255) <= tolerance * 3) return ImageMarker::Location;
    return ImageMarker::None;
}

std::string ClassifyTerrain(const SemanticColor& color)
{
    if (color.b > color.r + 35 && color.b > color.g + 20)
        return "water";
    if (color.r > 150 && color.g > 125 && color.b < 120)
        return "sand";
    if (std::abs(color.r - color.g) < 22 && std::abs(color.g - color.b) < 22 && color.r > 90)
        return "pave";
    if (color.g > color.r + 15 && color.g > color.b + 15)
        return color.g < 125 ? "rough" : "green";
    return "clear";
}

struct DecodedImage
{
    int width = 0;
    int height = 0;
    std::vector<unsigned char> rgba;

    SemanticColor GetPixel(int x, int y) const
    {
        const std::size_t offset = (static_cast<std::size_t>(y) * width + x) * 4;
        return { rgba[offset], rgba[offset + 1], rgba[offset + 2] };
    }
};

template<typename T>
void ReleaseCom(T*& value)
{
    if (value)
    {
        value->Release();
        value = nullptr;
    }
}

bool LoadImageArgument(const Json& arguments, DecodedImage& image, std::string& error)
{
    const bool hasPath = arguments.contains("imagePath");
    const bool hasBase64 = arguments.contains("imageBase64");
    if (hasPath == hasBase64)
    {
        error = "Provide exactly one of imagePath or imageBase64.";
        return false;
    }

    IWICImagingFactory* factory = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    IWICStream* stream = nullptr;
    HRESULT loadResult = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory));
    if (FAILED(loadResult))
    {
        error = "Windows Imaging Component is unavailable in the editor UI thread.";
        return false;
    }
    if (hasPath)
    {
        const std::string path = RequireString(arguments, "imagePath");
        const std::filesystem::path source = std::filesystem::u8path(path);
        if (!source.is_absolute() || !std::filesystem::is_regular_file(source))
        {
            error = "imagePath must point to an existing absolute image file.";
            ReleaseCom(factory);
            return false;
        }
        loadResult = factory->CreateDecoderFromFilename(source.wstring().c_str(), nullptr, GENERIC_READ,
            WICDecodeMetadataCacheOnLoad, &decoder);
    }
    else
    {
        const std::vector<unsigned char> bytes = Base64Decode(RequireString(arguments, "imageBase64"));
        if (bytes.size() > 64u * 1024u * 1024u)
        {
            error = "Decoded image exceeds the 64 MiB limit.";
            ReleaseCom(factory);
            return false;
        }
        if (bytes.size() > MAXDWORD)
            loadResult = E_INVALIDARG;
        else
            loadResult = factory->CreateStream(&stream);
        if (SUCCEEDED(loadResult))
            loadResult = stream->InitializeFromMemory(const_cast<BYTE*>(bytes.data()), static_cast<DWORD>(bytes.size()));
        if (SUCCEEDED(loadResult))
            loadResult = factory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder);
    }

    if (SUCCEEDED(loadResult))
        loadResult = decoder->GetFrame(0, &frame);
    UINT width = 0;
    UINT height = 0;
    if (SUCCEEDED(loadResult))
        loadResult = frame->GetSize(&width, &height);
    if (SUCCEEDED(loadResult))
        loadResult = factory->CreateFormatConverter(&converter);
    if (SUCCEEDED(loadResult))
        loadResult = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (SUCCEEDED(loadResult) && width > 0 && height > 0 &&
        static_cast<std::uint64_t>(width) * height <= 64u * 1024u * 1024u)
    {
        image.width = static_cast<int>(width);
        image.height = static_cast<int>(height);
        image.rgba.resize(static_cast<std::size_t>(width) * height * 4);
        loadResult = converter->CopyPixels(nullptr, width * 4, static_cast<UINT>(image.rgba.size()), image.rgba.data());
    }
    else if (SUCCEEDED(loadResult))
        loadResult = E_INVALIDARG;

    ReleaseCom(converter);
    ReleaseCom(frame);
    ReleaseCom(decoder);
    ReleaseCom(stream);
    ReleaseCom(factory);
    if (FAILED(loadResult) || image.width <= 0 || image.height <= 0 || image.rgba.empty())
    {
        error = "Windows Imaging Component could not decode the supplied BMP, PNG, or JPEG image.";
        return false;
    }
    return true;
}

bool CoordinatesFarEnough(const std::vector<std::pair<int, int>>& points, int x, int y, int distance)
{
    for (const auto& point : points)
        if (std::abs(point.first - x) + std::abs(point.second - y) < distance)
            return false;
    return true;
}

std::filesystem::path MakeCheckpointPath()
{
    wchar_t temporaryFolder[MAX_PATH + 1] = {};
    wchar_t temporaryFile[MAX_PATH + 1] = {};
    if (!GetTempPathW(_countof(temporaryFolder), temporaryFolder) ||
        !GetTempFileNameW(temporaryFolder, L"F2M", 0, temporaryFile))
        throw ToolInputError("Could not create a transaction checkpoint path.");
    DeleteFileW(temporaryFile);
    std::filesystem::path result(temporaryFile);
    result.replace_extension(L".map");
    return result;
}

class BatchGuard
{
public:
    BatchGuard() { ++BatchDepth; }
    ~BatchGuard() { --BatchDepth; }
};
}

namespace
{
Json HandleGetStatus()
{
    Json status = {
        {"editor", "FinalAlert 2 MCP Editor"},
        {"version", "1.0.0"},
        {"uiThread", static_cast<unsigned long>(GetCurrentThreadId())},
        {"mapLoaded", Map && Map->GetIsoSize() > 0},
        {"capabilities", {
            "create-map", "semantic-image", "terrain", "height", "resources",
            "waypoints", "player-spawns", "objects", "metadata", "ini", "preview", "save"
        }}
    };
    if (Map && Map->GetIsoSize() > 0)
        status["map"] = CurrentMapInfo();
    return ToolSuccess("FinalAlert 2 MCP is ready.", std::move(status));
}

Json HandleGetMapInfo()
{
    return ToolSuccess("Current map information returned.", CurrentMapInfo());
}

Json HandleGetCells(const Json& arguments)
{
    RequireMap();
    const int iso = static_cast<int>(Map->GetIsoSize());
    const int x0 = OptionalInteger(arguments, "x", 0, 0, iso - 1);
    const int y0 = OptionalInteger(arguments, "y", 0, 0, iso - 1);
    const int width = OptionalInteger(arguments, "width", iso - x0, 1, iso - x0);
    const int height = OptionalInteger(arguments, "height", iso - y0, 1, iso - y0);
    const bool playableOnly = OptionalBoolean(arguments, "playableOnly", true);
    const int limit = OptionalInteger(arguments, "limit", 4096, 1, 20000);

    Json cells = Json::array();
    bool truncated = false;
    for (int y = y0; y < y0 + height && !truncated; ++y)
    {
        for (int x = x0; x < x0 + width; ++x)
        {
            const bool playable = IsPlayableCell(x, y);
            if (playableOnly && !playable)
                continue;
            if (static_cast<int>(cells.size()) >= limit)
            {
                truncated = true;
                break;
            }
            const DWORD position = static_cast<DWORD>(x + y * iso);
            const FIELDDATA* field = Map->GetFielddataAt(position);
            cells.push_back({
                {"x", x}, {"y", y}, {"playable", playable},
                {"height", field->bHeight}, {"tileId", field->wGround}, {"subTile", field->bSubTile},
                {"overlay", field->overlay}, {"overlayData", field->overlaydata},
                {"structure", field->structure}, {"unit", field->unit}, {"infantry", Map->GetInfantryAt(position)},
                {"aircraft", field->aircraft}, {"terrainObject", field->terrain}, {"waypoint", field->waypoint}
            });
        }
    }
    return ToolSuccess("Map cells returned.", {
        {"cells", std::move(cells)}, {"truncated", truncated}, {"isoSize", iso}
    });
}

Json HandleListCatalog(const Json& arguments)
{
    const std::string kind = LowerAscii(RequireString(arguments, "kind"));
    const int offset = OptionalInteger(arguments, "offset", 0, 0, 1000000);
    const int limit = OptionalInteger(arguments, "limit", 250, 1, 2000);
    if (kind == "tile")
    {
        RequireMap();
        Json items = Json::array();
        const int total = tiledata && tiledata_count && *tiledata ? static_cast<int>(*tiledata_count) : 0;
        for (int id = offset; id < total && static_cast<int>(items.size()) < limit; ++id)
        {
            const TILEDATA& tile = (*tiledata)[id];
            items.push_back({ {"id", id}, {"tileSet", tile.wTileSet}, {"subTiles", tile.wTileCount},
                {"width", tile.cx}, {"height", tile.cy}, {"allowResources", tile.bAllowTiberium != FALSE},
                {"allowPlacement", tile.bAllowToPlace != FALSE}, {"morphable", tile.bMorphable != FALSE} });
        }
        return ToolSuccess("Tile catalog entries returned.", {
            {"kind", kind}, {"items", std::move(items)}, {"total", total}, {"offset", offset}, {"hasMore", offset + limit < total}
        });
    }
    if (kind == "overlay")
    {
        Json all = Json::array();
        all.push_back({ {"id", 0x1E}, {"name", "Gems"}, {"resource", true}, {"yrOnly", false} });
        for (int id = 0x68; id <= 0x6F; ++id)
            all.push_back({ {"id", id}, {"name", "Ore"}, {"resource", true}, {"yrOnly", false} });
        for (int index = 0; index < overlay_count; ++index)
            all.push_back({ {"id", overlay_number[index]}, {"name", CStringToUtf8(overlay_name[index])},
                {"resource", false}, {"yrOnly", yr_only[index] != FALSE} });
        Json items = Json::array();
        for (int index = offset; index < static_cast<int>(all.size()) && static_cast<int>(items.size()) < limit; ++index)
            items.push_back(all[index]);
        return ToolSuccess("Overlay catalog entries returned.", {
            {"kind", kind}, {"items", std::move(items)}, {"total", all.size()}, {"offset", offset},
            {"hasMore", offset + limit < static_cast<int>(all.size())}
        });
    }
    const CString sectionName = CatalogSectionForKind(kind);
    const std::vector<CString> values = CatalogValues(sectionName);
    Json items = Json::array();
    for (int i = offset; i < static_cast<int>(values.size()) && static_cast<int>(items.size()) < limit; ++i)
    {
        Json item = { {"id", CStringToUtf8(values[i])}, {"index", i} };
        if (kind != "house")
        {
            const CString uiName = rules.GetValueByName(values[i], "UIName", values[i]);
            item["uiName"] = CStringToUtf8(uiName);
            item["yrOnly"] = IsYrOnlyType(kind == "building" ? "structure" : kind, values[i]);
        }
        items.push_back(std::move(item));
    }
    return ToolSuccess("Catalog entries returned.", {
        {"kind", kind}, {"items", std::move(items)}, {"total", values.size()},
        {"offset", offset}, {"hasMore", offset + limit < static_cast<int>(values.size())}
    });
}

Json HandleListObjects(const Json& arguments)
{
    RequireMap();
    const std::string requestedKind = LowerAscii(OptionalString(arguments, "kind", "all"));
    const int limit = OptionalInteger(arguments, "limit", 2000, 1, 10000);
    Json objects = Json::array();
    bool truncated = false;
    const auto append = [&](Json object) {
        if (static_cast<int>(objects.size()) >= limit)
        {
            truncated = true;
            return false;
        }
        objects.push_back(std::move(object));
        return true;
    };

    if (requestedKind == "all" || requestedKind == "structure")
    {
        for (DWORD i = 0; i < Map->GetStructureCount() && !truncated; ++i)
        {
            STRUCTURE value = {};
            Map->GetStructureData(i, &value);
            append({ {"kind", "structure"}, {"index", i}, {"type", CStringToUtf8(value.type)},
                {"house", CStringToUtf8(value.house)}, {"x", atoi(value.x)}, {"y", atoi(value.y)},
                {"strength", atoi(value.strength)}, {"direction", atoi(value.direction)} });
        }
    }
    if (requestedKind == "all" || requestedKind == "unit")
    {
        for (DWORD i = 0; i < Map->GetUnitCount() && !truncated; ++i)
        {
            UNIT value = {};
            Map->GetUnitData(i, &value);
            append({ {"kind", "unit"}, {"index", i}, {"type", CStringToUtf8(value.type)},
                {"house", CStringToUtf8(value.house)}, {"x", atoi(value.x)}, {"y", atoi(value.y)},
                {"strength", atoi(value.strength)}, {"direction", atoi(value.direction)} });
        }
    }
    if (requestedKind == "all" || requestedKind == "aircraft")
    {
        for (DWORD i = 0; i < Map->GetAircraftCount() && !truncated; ++i)
        {
            AIRCRAFT value = {};
            Map->GetAircraftData(i, &value);
            append({ {"kind", "aircraft"}, {"index", i}, {"type", CStringToUtf8(value.type)},
                {"house", CStringToUtf8(value.house)}, {"x", atoi(value.x)}, {"y", atoi(value.y)},
                {"strength", atoi(value.strength)}, {"direction", atoi(value.direction)} });
        }
    }
    if (requestedKind == "all" || requestedKind == "infantry")
    {
        for (DWORD i = 0; i < Map->GetInfantryCount() && !truncated; ++i)
        {
            INFANTRY value = {};
            Map->GetInfantryData(i, &value);
            if (value.deleted)
                continue;
            append({ {"kind", "infantry"}, {"index", i}, {"type", CStringToUtf8(value.type)},
                {"house", CStringToUtf8(value.house)}, {"x", atoi(value.x)}, {"y", atoi(value.y)},
                {"subPosition", atoi(value.pos)}, {"strength", atoi(value.strength)}, {"direction", atoi(value.direction)} });
        }
    }
    if (requestedKind == "all" || requestedKind == "terrain")
    {
        for (DWORD i = 0; i < Map->GetTerrainCount() && !truncated; ++i)
        {
            TERRAIN value = {};
            Map->GetTerrainData(i, &value);
            if (value.deleted)
                continue;
            append({ {"kind", "terrain"}, {"index", i}, {"type", CStringToUtf8(value.type)},
                {"x", value.x}, {"y", value.y} });
        }
    }
    if (requestedKind == "all" || requestedKind == "waypoint")
    {
        for (DWORD i = 0; i < Map->GetWaypointCount() && !truncated; ++i)
        {
            CString id;
            DWORD position = 0;
            Map->GetWaypointData(i, &id, &position);
            append({ {"kind", "waypoint"}, {"index", i}, {"id", CStringToUtf8(id)},
                {"x", position % Map->GetIsoSize()}, {"y", position / Map->GetIsoSize()} });
        }
    }

    return ToolSuccess("Map objects returned.", {
        {"objects", std::move(objects)}, {"truncated", truncated}, {"requestedKind", requestedKind}
    });
}

Json HandleGetPreview(const Json& arguments)
{
    int width = 0;
    int height = 0;
    const std::vector<unsigned char> bmp = BuildMinimapBmp(width, height);
    const bool includeBase64 = OptionalBoolean(arguments, "includeBase64", true);
    Json data = { {"format", "image/bmp"}, {"width", width}, {"height", height}, {"byteLength", bmp.size()} };
    Json content = Json::array({ Json{ {"type", "text"}, {"text", "Current map preview rendered."} } });
    if (includeBase64)
    {
        const std::string encoded = Base64Encode(bmp.data(), bmp.size());
        data["base64"] = encoded;
        content.push_back({ {"type", "image"}, {"mimeType", "image/bmp"}, {"data", encoded} });
    }
    data["ok"] = true;
    return { {"content", std::move(content)}, {"structuredContent", std::move(data)}, {"isError", false} };
}

Json HandleValidateMap()
{
    RequireMap();
    Json issues = Json::array();
    int spawnCount = 0;
    std::set<int> spawnIds;
    for (DWORD i = 0; i < Map->GetWaypointCount(); ++i)
    {
        CString id;
        DWORD position = 0;
        Map->GetWaypointData(i, &id, &position);
        const int numeric = atoi(id);
        if (numeric >= 0 && numeric <= 7)
        {
            spawnIds.insert(numeric);
            ++spawnCount;
            const int x = position % Map->GetIsoSize();
            const int y = position / Map->GetIsoSize();
            if (!IsPlayableCell(x, y))
                issues.push_back({ {"severity", "error"}, {"code", "spawn_outside_map"}, {"waypoint", numeric} });
        }
    }
    for (int expected = 0; expected < spawnCount; ++expected)
        if (!spawnIds.contains(expected))
            issues.push_back({ {"severity", "error"}, {"code", "non_contiguous_spawns"}, {"missing", expected} });

    int steepEdges = 0;
    const int iso = static_cast<int>(Map->GetIsoSize());
    for (int y = 1; y < iso; ++y)
        for (int x = 1; x < iso; ++x)
            if (IsPlayableCell(x, y))
            {
                const int height = Map->GetHeightAt(static_cast<DWORD>(x + y * iso));
                if (x + 1 < iso && IsPlayableCell(x + 1, y) && std::abs(height - Map->GetHeightAt(static_cast<DWORD>(x + 1 + y * iso))) > 2)
                    ++steepEdges;
                if (y + 1 < iso && IsPlayableCell(x, y + 1) && std::abs(height - Map->GetHeightAt(static_cast<DWORD>(x + (y + 1) * iso))) > 2)
                    ++steepEdges;
            }
    if (steepEdges)
        issues.push_back({ {"severity", "warning"}, {"code", "steep_height_edges"}, {"count", steepEdges} });
    if (Map->IsMultiplayer() && spawnCount < 2)
        issues.push_back({ {"severity", "error"}, {"code", "too_few_player_spawns"}, {"count", spawnCount} });
    if (IsTargetRa2() && Map->IsYRMap())
        issues.push_back({ {"severity", "error"}, {"code", "yr_content_in_ra2_map"} });

    int errors = 0;
    int warnings = 0;
    for (const auto& issue : issues)
        issue.value("severity", "warning") == "error" ? ++errors : ++warnings;
    return ToolSuccess(errors ? "Map validation found errors." : "Map validation completed.", {
        {"valid", errors == 0}, {"errors", errors}, {"warnings", warnings}, {"issues", std::move(issues)},
        {"spawnCount", spawnCount}, {"money", Map->CalcMoneyOnMap()}
    });
}

Json HandleGetIniSection(const Json& arguments)
{
    RequireMap();
    const std::string sectionName = RequireString(arguments, "section");
    const int offset = OptionalInteger(arguments, "offset", 0, 0, 1000000);
    const int limit = OptionalInteger(arguments, "limit", 1000, 1, 5000);
    CIniFile& ini = Map->GetIniFile();
    const CIniFileSection* section = ini.GetSection(sectionName.c_str());
    if (!section)
        return ToolFailure("section_not_found", "The requested map INI section does not exist.");
    Json values = Json::object();
    int index = 0;
    for (const auto& pair : section->values)
    {
        if (index >= offset && index < offset + limit)
            values[CStringToUtf8(pair.first)] = CStringToUtf8(pair.second);
        ++index;
    }
    return ToolSuccess("INI section returned.", {
        {"section", sectionName}, {"values", std::move(values)}, {"total", section->values.size()},
        {"offset", offset}, {"hasMore", offset + limit < static_cast<int>(section->values.size())}
    });
}

void ValidateCreateArguments(const Json& arguments, int& width, int& height, int& startingHeight, std::string& theater, std::string& targetGame)
{
    width = RequireInteger(arguments, "width", 16, 400);
    height = RequireInteger(arguments, "height", 16, 400);
    if (width + height > 512)
        throw ToolInputError("width + height must not exceed 512.");
    startingHeight = OptionalInteger(arguments, "startingHeight", 0, 0, 14);
    theater = OptionalString(arguments, "theater", "TEMPERATE");
    std::transform(theater.begin(), theater.end(), theater.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    static const std::set<std::string> theaters = { "TEMPERATE", "SNOW", "URBAN", "NEWURBAN", "LUNAR", "DESERT" };
    if (!theaters.contains(theater))
        throw ToolInputError("theater must be TEMPERATE, SNOW, URBAN, NEWURBAN, LUNAR, or DESERT.");
    targetGame = LowerAscii(OptionalString(arguments, "targetGame", "yr"));
    if (targetGame != "ra2" && targetGame != "yr")
        throw ToolInputError("targetGame must be ra2 or yr.");
    if (targetGame == "ra2" && (theater == "NEWURBAN" || theater == "LUNAR" || theater == "DESERT"))
        throw ToolInputError("NEWURBAN, LUNAR, and DESERT are Yuri's Revenge-only theaters.");
}

Json HandleCreateMap(const Json& arguments, CFinalSunDlg& dialog)
{
    int width = 0;
    int height = 0;
    int startingHeight = 0;
    std::string theater;
    std::string targetGame;
    ValidateCreateArguments(arguments, width, height, startingHeight, theater, targetGame);
    Map->CreateMap(width, height, theater.c_str(), startingHeight);
    Json metadata = arguments;
    metadata["targetGame"] = targetGame;
    SetBasicMetadata(metadata);
    currentMapFile[0] = 0;
    RefreshEditor(dialog, RefreshKind::Full);
    return ToolSuccess("New map created.", CurrentMapInfo());
}

Json HandleOpenMap(const Json& arguments, CFinalSunDlg& dialog)
{
    const std::string path = RequireString(arguments, "path");
    const std::filesystem::path source = std::filesystem::u8path(path);
    if (!source.is_absolute() || !std::filesystem::is_regular_file(source))
        return ToolFailure("file_not_found", "path must point to an existing absolute map file.");
    const std::string extension = LowerAscii(utf16ToUtf8(source.extension().wstring()));
    if (extension != ".map" && extension != ".mpr" && extension != ".yrm")
        return ToolFailure("unsupported_extension", "Only .map, .mpr, and .yrm are accepted by noninteractive MCP open.");
    Map->LoadMap(path);
    if (!Map->CheckMapPackData())
        return ToolFailure("invalid_map_pack", "The map pack could not be decoded.");
    if (path.size() <= MAX_PATH)
        strcpy_s(currentMapFile, path.c_str());
    else
        currentMapFile[0] = 0;
    RefreshEditor(dialog, RefreshKind::Full);
    return ToolSuccess("Map opened.", CurrentMapInfo());
}

Json HandleSaveMap(const Json& arguments)
{
    RequireMap();
    const std::string path = RequireString(arguments, "path");
    const bool includePreview = OptionalBoolean(arguments, "includePreview", true);
    const bool compressed = OptionalBoolean(arguments, "compressed", true);
    const bool createDirectories = OptionalBoolean(arguments, "createDirectories", false);
    std::string error;
    if (!AtomicWriteMap(path, includePreview, compressed, createDirectories, error))
        return ToolFailure("save_failed", error);
    if (path.size() <= MAX_PATH)
        strcpy_s(currentMapFile, path.c_str());
    return ToolSuccess("Map saved atomically without opening UI dialogs.", {
        {"path", path}, {"includePreview", includePreview}, {"compressed", compressed}
    });
}

Json HandleSetMetadata(const Json& arguments, CFinalSunDlg& dialog)
{
    SetBasicMetadata(arguments);
    if (Map->IsMultiplayer())
        PrepareMultiplayerHeader();
    RefreshEditor(dialog, RefreshKind::Objects);
    return ToolSuccess("Map metadata updated.", CurrentMapInfo());
}

Json HandleImportSemanticImage(const Json& arguments, CFinalSunDlg& dialog)
{
    int width = 0;
    int height = 0;
    int startingHeight = 0;
    std::string theater;
    std::string targetGame;
    ValidateCreateArguments(arguments, width, height, startingHeight, theater, targetGame);

    DecodedImage image;
    std::string imageError;
    if (!LoadImageArgument(arguments, image, imageError))
        return ToolFailure("image_decode_failed", imageError);

    const std::string heightMode = LowerAscii(OptionalString(arguments, "heightMode", "none"));
    if (heightMode != "none" && heightMode != "luminance" && heightMode != "red")
        throw ToolInputError("heightMode must be none, luminance, or red.");
    const int maxHeight = OptionalInteger(arguments, "maxHeight", 8, 0, 14);
    const int markerTolerance = OptionalInteger(arguments, "markerTolerance", 12, 0, 64);
    const bool markersEnabled = OptionalBoolean(arguments, "markers", true);
    const bool autoShore = OptionalBoolean(arguments, "autoShore", true);
    const bool autoSmooth = OptionalBoolean(arguments, "autoSmooth", true);

    Map->CreateMap(width, height, theater.c_str(), startingHeight);
    Json metadata = arguments;
    metadata["targetGame"] = targetGame;
    SetBasicMetadata(metadata);

    const int iso = static_cast<int>(Map->GetIsoSize());
    const int previewWidth = width * 2;
    const int previewHeight = height;
    const int sourceWidth = image.width;
    const int sourceHeight = image.height;
    std::map<std::string, DWORD> terrainTiles;
    std::vector<std::pair<int, int>> spawnPoints;
    std::vector<std::pair<int, int>> locationPoints;
    std::vector<DWORD> orePoints;
    std::vector<DWORD> gemPoints;
    int painted = 0;

    BeginCellTransaction();
    for (int y = 1; y < iso; ++y)
    {
        for (int x = 1; x < iso; ++x)
        {
            if (!IsPlayableCell(x, y))
                continue;
            const CPoint mini = Map->GetMiniMapPos(MapCoords(static_cast<short>(x), static_cast<short>(y)));
            if (mini.x < 0 || mini.y < 0 || mini.x >= previewWidth || mini.y >= previewHeight)
                continue;
            const int sourceX = (std::clamp)(static_cast<int>(mini.x) * sourceWidth / (std::max)(1, previewWidth), 0, sourceWidth - 1);
            const int sourceY = (std::clamp)(static_cast<int>(mini.y) * sourceHeight / (std::max)(1, previewHeight), 0, sourceHeight - 1);
            const SemanticColor color = image.GetPixel(sourceX, sourceY);
            const ImageMarker marker = markersEnabled ? DetectMarker(color, markerTolerance) : ImageMarker::None;
            std::string terrain = marker == ImageMarker::None ? ClassifyTerrain(color) : "clear";
            if (!terrainTiles.contains(terrain))
                terrainTiles[terrain] = ResolveTerrainTile(terrain, static_cast<int>(CoordinateHash(x, y, 0xFA2u) % 4));
            const DWORD position = static_cast<DWORD>(x + y * iso);
            const DWORD tile = terrainTiles[terrain];
            ValidateTile(tile, 0);
            Map->SetTileAt(position, tile, 0);

            if (heightMode != "none")
            {
                const int channel = heightMode == "red" ? color.r : (color.r * 299 + color.g * 587 + color.b * 114) / 1000;
                const int cellHeight = (std::clamp)(startingHeight + (channel * maxHeight + 127) / 255, 0, 14);
                Map->SetHeightAt(position, static_cast<BYTE>(cellHeight));
            }

            if (marker == ImageMarker::Spawn && spawnPoints.size() < 8 && CoordinatesFarEnough(spawnPoints, x, y, 6))
                spawnPoints.emplace_back(x, y);
            else if (marker == ImageMarker::Ore)
                orePoints.push_back(position);
            else if (marker == ImageMarker::Gems)
                gemPoints.push_back(position);
            else if (marker == ImageMarker::Location && CoordinatesFarEnough(locationPoints, x, y, 3))
                locationPoints.emplace_back(x, y);
            ++painted;
        }
    }

    for (DWORD position : orePoints)
        Map->SetOverlayAt(position, static_cast<BYTE>(0x68 + CoordinateHash(position % iso, position / iso, 17) % 8));
    for (DWORD position : gemPoints)
        Map->SetOverlayAt(position, 0x1E);
    if (autoShore)
        Map->CreateShore(0, 0, iso, iso);
    if (autoSmooth)
        for (int y = 1; y < iso; ++y)
            for (int x = 1; x < iso; ++x)
                if (IsPlayableCell(x, y))
                    Map->SmoothAllAt(static_cast<DWORD>(x + y * iso));
    CommitCellTransaction();

    for (DWORD index = Map->GetWaypointCount(); index > 0; --index)
    {
        CString id;
        DWORD position = 0;
        Map->GetWaypointData(index - 1, &id, &position);
        const int numeric = atoi(id);
        if ((numeric >= 0 && numeric <= 7) || numeric == 98 || numeric == 99)
            Map->DeleteWaypoint(index - 1);
    }
    for (int index = 0; index < static_cast<int>(spawnPoints.size()); ++index)
    {
        const DWORD position = static_cast<DWORD>(spawnPoints[index].first + spawnPoints[index].second * iso);
        Map->AddWaypoint(ToCString(index), position);
    }
    for (int index = 0; index < static_cast<int>(locationPoints.size()); ++index)
    {
        const DWORD position = static_cast<DWORD>(locationPoints[index].first + locationPoints[index].second * iso);
        Map->AddWaypoint(ToCString(100 + index), position);
    }
    if (!spawnPoints.empty())
    {
        Json multiplayer = { {"multiplayer", true}, {"minPlayers", 2},
            {"maxPlayers", (std::max)(2, static_cast<int>(spawnPoints.size()))} };
        SetBasicMetadata(multiplayer);
        PrepareMultiplayerHeader();
    }
    currentMapFile[0] = 0;
    RefreshEditor(dialog, RefreshKind::Full);

    Json spawns = Json::array();
    for (const auto& point : spawnPoints)
        spawns.push_back(CoordinateJson(point.first, point.second));
    Json locations = Json::array();
    for (int i = 0; i < static_cast<int>(locationPoints.size()); ++i)
        locations.push_back({ {"id", 100 + i}, {"x", locationPoints[i].first}, {"y", locationPoints[i].second} });
    return ToolSuccess("Semantic image converted into a map.", {
        {"map", CurrentMapInfo()}, {"sourceWidth", sourceWidth}, {"sourceHeight", sourceHeight},
        {"paintedCells", painted}, {"oreCells", orePoints.size()}, {"gemCells", gemPoints.size()},
        {"spawns", std::move(spawns)}, {"locations", std::move(locations)}
    });
}

struct CellMutation
{
    DWORD position = 0;
    int x = 0;
    int y = 0;
    bool hasHeight = false;
    int height = 0;
    bool hasTile = false;
    DWORD tile = 0;
    DWORD subTile = 0;
    bool hasOverlay = false;
    BYTE overlay = 0xFF;
    bool hasOverlayData = false;
    BYTE overlayData = 0;
};

CellMutation ParseCellMutation(const Json& cell)
{
    if (!cell.is_object())
        throw ToolInputError("Each cells entry must be an object.");
    CellMutation change;
    change.position = RequirePosition(cell, change.x, change.y);
    if (cell.contains("height"))
    {
        change.hasHeight = true;
        change.height = RequireInteger(cell, "height", 0, 14);
    }
    if (cell.contains("terrain"))
    {
        change.hasTile = true;
        const std::string terrain = LowerAscii(RequireString(cell, "terrain"));
        change.tile = ResolveTerrainTile(terrain, OptionalInteger(cell, "tileIndex", 0, 0, 255));
        change.subTile = 0;
    }
    else if (cell.contains("tileId"))
    {
        change.hasTile = true;
        change.tile = static_cast<DWORD>(RequireInteger(cell, "tileId", 0, static_cast<int>(*tiledata_count) - 1));
        change.subTile = static_cast<DWORD>(OptionalInteger(cell, "subTile", 0, 0, 255));
    }
    if (change.hasTile)
        ValidateTile(change.tile, change.subTile);
    if (cell.contains("overlay"))
    {
        change.hasOverlay = true;
        change.overlay = static_cast<BYTE>(RequireInteger(cell, "overlay", 0, 255));
    }
    if (cell.contains("overlayData"))
    {
        change.hasOverlayData = true;
        change.overlayData = static_cast<BYTE>(RequireInteger(cell, "overlayData", 0, 255));
        const int overlay = change.hasOverlay ? change.overlay : Map->GetOverlayAt(change.position);
        if ((overlay >= RIPARIUS_BEGIN && overlay <= RIPARIUS_END) ||
            (overlay >= CRUENTUS_BEGIN && overlay <= CRUENTUS_END) ||
            (overlay >= VINIFERA_BEGIN && overlay <= VINIFERA_END) ||
            (overlay >= ABOREUS_BEGIN && overlay <= ABOREUS_END))
            throw ToolInputError("overlayData cannot be set for resource overlays; control their amount by cell count.");
    }
    if (!change.hasHeight && !change.hasTile && !change.hasOverlay && !change.hasOverlayData)
        throw ToolInputError("Each cells entry must change height, terrain/tileId, overlay, or overlayData.");
    return change;
}

void ApplyCellMutations(const std::vector<CellMutation>& changes, bool createSlopes, bool smooth)
{
    BeginCellTransaction();
    for (const CellMutation& change : changes)
    {
        if (change.hasTile)
            Map->SetTileAt(change.position, change.tile, change.subTile);
        if (change.hasHeight)
            Map->SetHeightAt(change.position, static_cast<BYTE>(change.height));
        if (change.hasOverlay)
            Map->SetOverlayAt(change.position, change.overlay);
        if (change.hasOverlayData)
            Map->SetOverlayDataAt(change.position, change.overlayData);
    }
    if (createSlopes)
        for (const CellMutation& change : changes)
            Map->CreateSlopesAt(change.position);
    if (smooth)
        for (const CellMutation& change : changes)
            Map->SmoothAllAt(change.position);
    CommitCellTransaction();
}

Json HandleSetCells(const Json& arguments, CFinalSunDlg& dialog)
{
    RequireMap();
    if (!arguments.contains("cells") || !arguments["cells"].is_array() || arguments["cells"].empty())
        throw ToolInputError("cells must be a non-empty array.");
    if (arguments["cells"].size() > 20000)
        throw ToolInputError("A single set_cells call is limited to 20,000 entries.");
    std::vector<CellMutation> changes;
    changes.reserve(arguments["cells"].size());
    for (const Json& cell : arguments["cells"])
        changes.push_back(ParseCellMutation(cell));
    const bool slopes = OptionalBoolean(arguments, "createSlopes", false);
    const bool smooth = OptionalBoolean(arguments, "smooth", false);
    ApplyCellMutations(changes, slopes, smooth);
    RefreshEditor(dialog, RefreshKind::Cells);
    return ToolSuccess("Map cells updated.", {
        {"changedCells", changes.size()}, {"createSlopes", slopes}, {"smooth", smooth}
    });
}

std::vector<std::pair<int, int>> RegionCells(const Json& arguments)
{
    RequireMap();
    const int iso = static_cast<int>(Map->GetIsoSize());
    const std::string shape = LowerAscii(OptionalString(arguments, "shape", "rectangle"));
    std::vector<std::pair<int, int>> result;
    if (shape == "circle")
    {
        const int centerX = RequireInteger(arguments, "x", 0, iso - 1);
        const int centerY = RequireInteger(arguments, "y", 0, iso - 1);
        const int radius = RequireInteger(arguments, "radius", 0, iso);
        for (int y = (std::max)(0, centerY - radius); y <= (std::min)(iso - 1, centerY + radius); ++y)
            for (int x = (std::max)(0, centerX - radius); x <= (std::min)(iso - 1, centerX + radius); ++x)
                if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) <= radius * radius && IsPlayableCell(x, y))
                    result.emplace_back(x, y);
    }
    else if (shape == "rectangle")
    {
        const int x0 = RequireInteger(arguments, "x", 0, iso - 1);
        const int y0 = RequireInteger(arguments, "y", 0, iso - 1);
        const int width = RequireInteger(arguments, "width", 1, iso - x0);
        const int height = RequireInteger(arguments, "height", 1, iso - y0);
        for (int y = y0; y < y0 + height; ++y)
            for (int x = x0; x < x0 + width; ++x)
                if (IsPlayableCell(x, y))
                    result.emplace_back(x, y);
    }
    else
        throw ToolInputError("shape must be rectangle or circle.");
    if (result.size() > 100000)
        throw ToolInputError("The selected region exceeds 100,000 playable cells.");
    return result;
}

Json HandlePaintRegion(const Json& arguments, CFinalSunDlg& dialog)
{
    const std::vector<std::pair<int, int>> cells = RegionCells(arguments);
    if (cells.empty())
        return ToolFailure("empty_region", "The selected shape contains no playable cells.");
    const bool hasHeight = arguments.contains("height");
    const bool hasDelta = arguments.contains("deltaHeight");
    const bool hasTerrain = arguments.contains("terrain");
    const bool hasTile = arguments.contains("tileId");
    if (!hasHeight && !hasDelta && !hasTerrain && !hasTile)
        throw ToolInputError("paint_region requires terrain/tileId, height, or deltaHeight.");
    if (hasHeight && hasDelta)
        throw ToolInputError("Use either height or deltaHeight, not both.");
    DWORD tile = 0;
    DWORD subTile = static_cast<DWORD>(OptionalInteger(arguments, "subTile", 0, 0, 255));
    if (hasTerrain)
        tile = ResolveTerrainTile(LowerAscii(RequireString(arguments, "terrain")), OptionalInteger(arguments, "tileIndex", 0, 0, 255));
    else if (hasTile)
        tile = static_cast<DWORD>(RequireInteger(arguments, "tileId", 0, static_cast<int>(*tiledata_count) - 1));
    if (hasTerrain || hasTile)
        ValidateTile(tile, subTile);
    const int absoluteHeight = hasHeight ? RequireInteger(arguments, "height", 0, 14) : 0;
    const int deltaHeight = hasDelta ? RequireInteger(arguments, "deltaHeight", -14, 14) : 0;

    std::vector<CellMutation> changes;
    changes.reserve(cells.size());
    const int iso = static_cast<int>(Map->GetIsoSize());
    for (const auto& coordinate : cells)
    {
        CellMutation change;
        change.x = coordinate.first;
        change.y = coordinate.second;
        change.position = static_cast<DWORD>(change.x + change.y * iso);
        change.hasTile = hasTerrain || hasTile;
        change.tile = tile;
        change.subTile = subTile;
        change.hasHeight = hasHeight || hasDelta;
        if (change.hasHeight)
            change.height = hasHeight ? absoluteHeight : (std::clamp)(Map->GetHeightAt(change.position) + deltaHeight, 0, 14);
        changes.push_back(change);
    }
    const bool slopes = OptionalBoolean(arguments, "createSlopes", false);
    const bool smooth = OptionalBoolean(arguments, "smooth", false);
    ApplyCellMutations(changes, slopes, smooth);
    RefreshEditor(dialog, RefreshKind::Cells);
    return ToolSuccess("Region painted.", {
        {"changedCells", changes.size()}, {"shape", LowerAscii(OptionalString(arguments, "shape", "rectangle"))}
    });
}

Json HandlePlaceResources(const Json& arguments, CFinalSunDlg& dialog)
{
    const std::string resource = LowerAscii(RequireString(arguments, "resource"));
    if (resource != "ore" && resource != "gems" && resource != "clear")
        throw ToolInputError("resource must be ore, gems, or clear.");
    std::vector<std::pair<int, int>> region = RegionCells(arguments);
    const bool respectTerrain = OptionalBoolean(arguments, "respectTerrain", true);
    const uint32_t seed = static_cast<uint32_t>(OptionalInteger(arguments, "seed", 1, 0, (std::numeric_limits<int>::max)()));
    std::vector<DWORD> candidates;
    const int iso = static_cast<int>(Map->GetIsoSize());
    for (const auto& coordinate : region)
    {
        const DWORD position = static_cast<DWORD>(coordinate.first + coordinate.second * iso);
        const FIELDDATA* field = Map->GetFielddataAt(position);
        if (resource != "clear" && respectTerrain &&
            (!tiledata || !tiledata_count || field->wGround >= *tiledata_count || !(*tiledata)[field->wGround].bAllowTiberium))
            continue;
        candidates.push_back(position);
    }
    std::sort(candidates.begin(), candidates.end(), [seed, iso](DWORD left, DWORD right) {
        return CoordinateHash(left % iso, left / iso, seed) < CoordinateHash(right % iso, right / iso, seed);
    });
    int targetCells = 0;
    if (arguments.contains("targetCells"))
        targetCells = RequireInteger(arguments, "targetCells", 0, static_cast<int>(candidates.size()));
    else
    {
        const double density = OptionalNumber(arguments, "density", resource == "clear" ? 1.0 : 0.35, 0.0, 1.0);
        targetCells = static_cast<int>(std::round(candidates.size() * density));
    }
    const int targetCredits = OptionalInteger(arguments, "targetCredits", 0, 0, (std::numeric_limits<int>::max)());
    const int moneyBefore = Map->CalcMoneyOnMap();
    BeginCellTransaction();
    int placed = 0;
    for (DWORD position : candidates)
    {
        if (placed >= targetCells && (!targetCredits || Map->CalcMoneyOnMap() - moneyBefore >= targetCredits))
            break;
        BYTE overlay = 0xFF;
        if (resource == "ore")
            overlay = static_cast<BYTE>(0x68 + CoordinateHash(position % iso, position / iso, seed ^ 0x51u) % 8);
        else if (resource == "gems")
            overlay = 0x1E;
        Map->SetOverlayAt(position, overlay);
        ++placed;
    }
    CommitCellTransaction();
    RefreshEditor(dialog, RefreshKind::Cells);
    const int moneyAfter = Map->CalcMoneyOnMap();
    return ToolSuccess(resource == "clear" ? "Resources cleared in region." : "Resources placed.", {
        {"resource", resource}, {"changedCells", placed}, {"eligibleCells", candidates.size()},
        {"moneyBefore", moneyBefore}, {"moneyAfter", moneyAfter}, {"creditDelta", moneyAfter - moneyBefore}, {"seed", seed}
    });
}

Json HandlePlaceWaypoint(const Json& arguments, CFinalSunDlg& dialog)
{
    int x = 0;
    int y = 0;
    const DWORD position = RequirePosition(arguments, x, y);
    const std::string id = RequireString(arguments, "id");
    if (id.size() > 32 || id.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-.") != std::string::npos)
        throw ToolInputError("id must be at most 32 characters and contain only letters, digits, _, -, or .");
    if (!Map->AddWaypoint(id.c_str(), position))
        return ToolFailure("waypoint_failed", "FinalAlert rejected the waypoint.");
    RefreshEditor(dialog, RefreshKind::Objects);
    return ToolSuccess("Waypoint placed or replaced.", { {"id", id}, {"x", x}, {"y", y} });
}

Json HandleSetPlayerSpawns(const Json& arguments, CFinalSunDlg& dialog)
{
    RequireMap();
    if (!arguments.contains("spawns") || !arguments["spawns"].is_array())
        throw ToolInputError("spawns must be an array.");
    if (arguments["spawns"].size() < 2 || arguments["spawns"].size() > 8)
        throw ToolInputError("A multiplayer map requires between 2 and 8 spawns.");
    std::vector<std::pair<int, int>> points;
    for (const Json& spawn : arguments["spawns"])
    {
        if (!spawn.is_object())
            throw ToolInputError("Each spawn must be an object with x and y.");
        int x = 0;
        int y = 0;
        RequirePosition(spawn, x, y);
        if (!CoordinatesFarEnough(points, x, y, 2))
            throw ToolInputError("Player spawns must not occupy the same or immediately adjacent cell.");
        points.emplace_back(x, y);
    }

    for (DWORD index = Map->GetWaypointCount(); index > 0; --index)
    {
        CString id;
        DWORD position = 0;
        Map->GetWaypointData(index - 1, &id, &position);
        const int numeric = atoi(id);
        if ((numeric >= 0 && numeric <= 7) || numeric == 98 || numeric == 99)
            Map->DeleteWaypoint(index - 1);
    }
    const int iso = static_cast<int>(Map->GetIsoSize());
    for (int index = 0; index < static_cast<int>(points.size()); ++index)
        Map->AddWaypoint(ToCString(index), static_cast<DWORD>(points[index].first + points[index].second * iso));
    Json metadata = {
        {"multiplayer", true},
        {"minPlayers", OptionalInteger(arguments, "minPlayers", 2, 1, static_cast<int>(points.size()))},
        {"maxPlayers", OptionalInteger(arguments, "maxPlayers", static_cast<int>(points.size()), 2, static_cast<int>(points.size()))}
    };
    SetBasicMetadata(metadata);
    PrepareMultiplayerHeader();
    RefreshEditor(dialog, RefreshKind::Objects);
    Json output = Json::array();
    for (int index = 0; index < static_cast<int>(points.size()); ++index)
        output.push_back({ {"id", index}, {"x", points[index].first}, {"y", points[index].second} });
    return ToolSuccess("Multiplayer starting positions replaced.", { {"spawns", std::move(output)} });
}

void ValidateObjectPlacement(const std::string& kind, const CString& type, const CString& house, int x, int y, DWORD position, bool allowOverlap)
{
    const CString section = CatalogSectionForKind(kind);
    if (!TypeExists(section, type))
        throw ToolInputError("Unknown object type for the selected kind. Use finalalert.list_catalog first.");
    if (kind != "terrain" && !HouseExists(house))
        throw ToolInputError("Unknown owner house. Use finalalert.list_catalog with kind=house first.");
    if (IsTargetRa2() && IsYrOnlyType(kind, type))
        throw ToolInputError("The selected object type is Yuri's Revenge-only, but this map targets Red Alert 2.");

    if (kind == "infantry")
    {
        if (Map->GetInfantryCountAt(position) >= SUBPOS_COUNT)
            throw ToolInputError("All infantry subpositions at this cell are occupied.");
        if (!allowOverlap && (Map->GetStructureAt(position) >= 0 || Map->GetTerrainAt(position) >= 0))
            throw ToolInputError("The target cell is blocked by a structure or terrain object.");
        return;
    }

    if (kind == "structure")
    {
        const int buildingId = Map->GetBuildingID(type);
        if (buildingId < 0 || buildingId >= 0x0F00)
            throw ToolInputError("The building catalog is not initialized for this structure type.");
        const int footprintX = (std::max)(1, static_cast<int>(buildinginfo[buildingId].h));
        const int footprintY = (std::max)(1, static_cast<int>(buildinginfo[buildingId].w));
        const int iso = static_cast<int>(Map->GetIsoSize());
        for (int dy = 0; dy < footprintY; ++dy)
            for (int dx = 0; dx < footprintX; ++dx)
            {
                const int cellX = x + dx;
                const int cellY = y + dy;
                if (!IsPlayableCell(cellX, cellY))
                    throw ToolInputError("The structure footprint extends outside the playable map.");
                if (!allowOverlap && AnyBlockingObjectAt(static_cast<DWORD>(cellX + cellY * iso)))
                    throw ToolInputError("The structure footprint overlaps another blocking object.");
            }
        return;
    }

    if (!allowOverlap && AnyBlockingObjectAt(position))
        throw ToolInputError("The target cell already contains a blocking object.");
}

Json HandlePlaceObject(const Json& arguments, CFinalSunDlg& dialog)
{
    const std::string kind = LowerAscii(RequireString(arguments, "kind"));
    if (kind != "structure" && kind != "unit" && kind != "infantry" && kind != "aircraft" && kind != "terrain")
        throw ToolInputError("kind must be structure, unit, infantry, aircraft, or terrain.");
    int x = 0;
    int y = 0;
    const DWORD position = RequirePosition(arguments, x, y);
    const CString type(RequireString(arguments, "type").c_str());
    const CString house(kind == "terrain" ? "" : OptionalString(arguments, "house", "Neutral").c_str());
    const bool allowOverlap = OptionalBoolean(arguments, "allowOverlap", false);
    ValidateObjectPlacement(kind, type, house, x, y, position, allowOverlap);

    BOOL result = FALSE;
    const CString suggestedId(OptionalString(arguments, "id", "").c_str());
    if (kind == "structure") result = Map->AddStructure(nullptr, type, house, position, suggestedId);
    else if (kind == "unit") result = Map->AddUnit(nullptr, type, house, position, suggestedId);
    else if (kind == "infantry") result = Map->AddInfantry(nullptr, type, house, position);
    else if (kind == "aircraft") result = Map->AddAircraft(nullptr, type, house, position, suggestedId);
    else if (kind == "terrain") result = Map->AddTerrain(type, position);
    if (!result)
        return ToolFailure("object_rejected", "FinalAlert rejected the object after validation.");
    RefreshEditor(dialog, RefreshKind::Objects);
    return ToolSuccess("Map object placed.", {
        {"kind", kind}, {"type", CStringToUtf8(type)}, {"house", CStringToUtf8(house)}, {"x", x}, {"y", y}
    });
}

Json HandleDeleteAt(const Json& arguments, CFinalSunDlg& dialog)
{
    int x = 0;
    int y = 0;
    const DWORD position = RequirePosition(arguments, x, y);
    const std::string kind = LowerAscii(OptionalString(arguments, "kind", "all"));
    const bool all = kind == "all";
    static const std::set<std::string> supported = { "all", "structure", "unit", "infantry", "aircraft", "terrain", "waypoint", "overlay" };
    if (!supported.contains(kind))
        throw ToolInputError("Unsupported kind for delete_at.");
    int deleted = 0;
    if (all || kind == "structure")
    {
        const int index = Map->GetStructureAt(position);
        if (index >= 0) { Map->DeleteStructure(index); ++deleted; }
    }
    if (all || kind == "unit")
    {
        const int index = Map->GetUnitAt(position);
        if (index >= 0) { Map->DeleteUnit(index); ++deleted; }
    }
    if (all || kind == "aircraft")
    {
        const int index = Map->GetAirAt(position);
        if (index >= 0) { Map->DeleteAircraft(index); ++deleted; }
    }
    if (all || kind == "infantry")
    {
        std::vector<int> indexes;
        for (int sub = 0; sub < SUBPOS_COUNT; ++sub)
        {
            const int index = Map->GetInfantryAt(position, sub);
            if (index >= 0)
                indexes.push_back(index);
        }
        for (int index : indexes) { Map->DeleteInfantry(index); ++deleted; }
    }
    if (all || kind == "terrain")
    {
        const int index = Map->GetTerrainAt(position);
        if (index >= 0) { Map->DeleteTerrain(index); ++deleted; }
    }
    if (all || kind == "waypoint")
    {
        const int index = Map->GetWaypointAt(position);
        if (index >= 0) { Map->DeleteWaypoint(index); ++deleted; }
    }
    if (all || kind == "overlay")
    {
        if (Map->GetOverlayAt(position) != 0xFF) { Map->SetOverlayAt(position, 0xFF); ++deleted; }
    }
    RefreshEditor(dialog, all ? RefreshKind::Objects : (kind == "overlay" ? RefreshKind::Cells : RefreshKind::Objects));
    return ToolSuccess("Objects removed from the selected cell.", { {"x", x}, {"y", y}, {"deleted", deleted}, {"kind", kind} });
}

bool IsProtectedIniSection(const std::string& section)
{
    const std::string lower = LowerAscii(section);
    static const std::set<std::string> protectedSections = {
        "map", "isomappack5", "overlaypack", "overlaydatapack", "preview", "previewpack",
        "structures", "units", "infantry", "aircraft", "terrain", "waypoints", "celltags",
        "tubes", "houses", "countries"
    };
    return protectedSections.contains(lower) || lower.ends_with("types");
}

Json HandleSetIniValue(const Json& arguments, CFinalSunDlg& dialog)
{
    RequireMap();
    const std::string section = RequireString(arguments, "section");
    const std::string key = RequireString(arguments, "key");
    if (section.size() > 128 || key.size() > 128 || section.find_first_of("[]\r\n") != std::string::npos || key.find_first_of("=\r\n") != std::string::npos)
        throw ToolInputError("section or key contains invalid INI characters or is too long.");
    if (IsProtectedIniSection(section))
        throw ToolInputError("This derived/object section is protected. Use a typed MCP tool instead.");
    const bool remove = OptionalBoolean(arguments, "delete", false);
    CIniFile& ini = Map->GetIniFile();
    if (remove)
    {
        if (CIniFileSection* found = ini.GetSection(section.c_str()))
            found->values.erase(key.c_str());
    }
    else
    {
        const std::string value = RequireString(arguments, "value");
        if (value.size() > 16384 || value.find_first_of("\r\n") != std::string::npos)
            throw ToolInputError("value contains a newline or exceeds 16 KiB.");
        ini.sections[section.c_str()].values[key.c_str()] = value.c_str();
    }
    RefreshEditor(dialog, RefreshKind::Objects);
    return ToolSuccess(remove ? "INI value deleted." : "INI value set.", {
        {"section", section}, {"key", key}, {"deleted", remove}
    });
}

Json ExecutePlanStep(const std::string& tool, Json arguments, CFinalSunDlg& dialog)
{
    arguments["operationId"] = "nested-map-plan-step";
    return CMcpMapTools::Execute(tool, arguments, dialog);
}

Json HandleApplyMapPlan(const Json& arguments, CFinalSunDlg& dialog)
{
    if (!arguments.contains("plan") || !arguments["plan"].is_object())
        throw ToolInputError("plan must be an object.");
    const Json& plan = arguments["plan"];
    std::filesystem::path checkpoint;
    bool hasCheckpoint = false;
    if (Map && Map->GetIsoSize() > 0)
    {
        checkpoint = MakeCheckpointPath();
        std::string checkpointError;
        if (!AtomicWriteMap(utf16ToUtf8(checkpoint.wstring()), true, true, false, checkpointError))
            return ToolFailure("checkpoint_failed", "Could not start atomic map plan: " + checkpointError);
        hasCheckpoint = true;
    }

    Json steps = Json::array();
    Json failure;
    bool failed = false;
    {
        BatchGuard batch;
        const auto run = [&](const std::string& tool, const Json& stepArguments) {
            Json result = ExecutePlanStep(tool, stepArguments, dialog);
            steps.push_back({ {"tool", tool}, {"result", result.value("structuredContent", Json::object())} });
            if (IsToolError(result))
            {
                failure = result;
                failed = true;
            }
        };

        if (plan.contains("create") && !failed)
            run("finalalert.create_map", plan["create"]);
        if (plan.contains("metadata") && !failed)
            run("finalalert.set_map_metadata", plan["metadata"]);
        if (plan.contains("cells") && !failed)
            run("finalalert.set_cells", Json{ {"cells", plan["cells"]} });
        if (plan.contains("regions"))
        {
            if (!plan["regions"].is_array()) throw ToolInputError("plan.regions must be an array.");
            for (const Json& item : plan["regions"]) { if (failed) break; run("finalalert.paint_region", item); }
        }
        if (plan.contains("resources"))
        {
            if (!plan["resources"].is_array()) throw ToolInputError("plan.resources must be an array.");
            for (const Json& item : plan["resources"]) { if (failed) break; run("finalalert.place_resources", item); }
        }
        if (plan.contains("waypoints"))
        {
            if (!plan["waypoints"].is_array()) throw ToolInputError("plan.waypoints must be an array.");
            for (const Json& item : plan["waypoints"]) { if (failed) break; run("finalalert.place_waypoint", item); }
        }
        if (plan.contains("spawns") && !failed)
            run("finalalert.set_player_spawns", Json{ {"spawns", plan["spawns"]} });
        if (plan.contains("objects"))
        {
            if (!plan["objects"].is_array()) throw ToolInputError("plan.objects must be an array.");
            for (const Json& item : plan["objects"]) { if (failed) break; run("finalalert.place_object", item); }
        }
        if (plan.contains("ini"))
        {
            if (!plan["ini"].is_array()) throw ToolInputError("plan.ini must be an array.");
            for (const Json& item : plan["ini"]) { if (failed) break; run("finalalert.set_ini_value", item); }
        }
        if (plan.contains("save") && !failed)
            run("finalalert.save_map", plan["save"]);
    }

    if (failed)
    {
        if (hasCheckpoint)
        {
            Map->LoadMap(utf16ToUtf8(checkpoint.wstring()));
            Map->CheckMapPackData();
            RefreshEditor(dialog, RefreshKind::Full);
        }
        std::error_code ignored;
        if (hasCheckpoint) std::filesystem::remove(checkpoint, ignored);
        return ToolFailure("plan_failed", "Map plan failed and the previous loaded map was restored.", {
            {"steps", std::move(steps)}, {"cause", failure.value("structuredContent", failure)},
            {"rolledBack", hasCheckpoint}, {"partialApplied", !hasCheckpoint}
        });
    }

    std::error_code ignored;
    if (hasCheckpoint) std::filesystem::remove(checkpoint, ignored);
    RefreshEditor(dialog, RefreshKind::Full);
    return ToolSuccess("Map plan applied as one serialized transaction.", {
        {"steps", std::move(steps)}, {"map", CurrentMapInfo()}
    });
}

Json HandleUndo(CFinalSunDlg& dialog)
{
    RequireMap();
    Map->Undo();
    RefreshEditor(dialog, RefreshKind::Cells);
    return ToolSuccess("Terrain/height/overlay undo requested.", CurrentMapInfo());
}

Json HandleRedo(CFinalSunDlg& dialog)
{
    RequireMap();
    Map->Redo();
    RefreshEditor(dialog, RefreshKind::Cells);
    return ToolSuccess("Terrain/height/overlay redo requested.", CurrentMapInfo());
}
}

CMcpMapTools::Json CMcpMapTools::ListTools()
{
    const Json coordinate = {
        {"type", "object"},
        {"properties", { {"x", IntegerSchema("Internal isometric X coordinate.", 0)}, {"y", IntegerSchema("Internal isometric Y coordinate.", 0)} }},
        {"required", {"x", "y"}}, {"additionalProperties", false}
    };
    const Json regionProperties = {
        {"shape", StringSchema("Region geometry.", {"rectangle", "circle"})},
        {"x", IntegerSchema("Rectangle origin or circle center X.", 0)},
        {"y", IntegerSchema("Rectangle origin or circle center Y.", 0)},
        {"width", IntegerSchema("Rectangle width in internal cells.", 1)},
        {"height", IntegerSchema("Rectangle height in internal cells.", 1)},
        {"radius", IntegerSchema("Circle radius in internal cells.", 0)}
    };
    Json tools = Json::array();
    tools.push_back(MakeTool("finalalert.get_status", "Get editor status",
        "Check whether the in-process editor bridge is ready and whether a map is loaded.", {}, {}, true, false, true));
    tools.push_back(MakeTool("finalalert.get_map_info", "Get map information",
        "Return map size, theater, RA2/YR target, multiplayer state, object counts, resources, and current file.", {}, {}, true, false, true));
    tools.push_back(MakeTool("finalalert.get_cells", "Read map cells",
        "Read validated cells from the internal isometric grid. Use playableOnly unless diagnosing borders.", {
            {"x", IntegerSchema("Region origin X.", 0)}, {"y", IntegerSchema("Region origin Y.", 0)},
            {"width", IntegerSchema("Region width.", 1)}, {"height", IntegerSchema("Region height.", 1)},
            {"playableOnly", BooleanSchema("Return only the playable diamond.")}, {"limit", IntegerSchema("Maximum returned cells.", 1, 20000)}
        }, {}, true, false, true));
    tools.push_back(MakeTool("finalalert.list_catalog", "List placeable catalog",
        "List valid type IDs or houses before placing map objects; entries identify Yuri-only content.", {
            {"kind", StringSchema("Catalog category.", {"structure", "unit", "infantry", "aircraft", "terrain", "house", "tile", "overlay"})},
            {"offset", IntegerSchema("Pagination offset.", 0)}, {"limit", IntegerSchema("Page size.", 1, 2000)}
        }, {"kind"}, true, false, true));
    tools.push_back(MakeTool("finalalert.list_objects", "List map objects",
        "List placed structures, units, infantry, aircraft, terrain objects, and waypoints.", {
            {"kind", StringSchema("Object category or all.", {"all", "structure", "unit", "infantry", "aircraft", "terrain", "waypoint"})},
            {"limit", IntegerSchema("Maximum returned objects.", 1, 10000)}
        }, {}, true, false, true));
    tools.push_back(MakeTool("finalalert.get_preview", "Render map preview",
        "Render the current FinalAlert minimap as a BMP image content block and optional base64 field.", {
            {"includeBase64", BooleanSchema("Include image bytes; false returns dimensions only.")}
        }, {}, true, false, true));
    tools.push_back(MakeTool("finalalert.validate_map", "Validate map",
        "Check multiplayer spawns, RA2/YR compatibility, map heights, and basic resource totals.", {}, {}, true, false, true));
    tools.push_back(MakeTool("finalalert.get_ini_section", "Read map INI section",
        "Read one map INI section for advanced mission inspection. Reads are marshaled through the editor UI thread.", {
            {"section", StringSchema("INI section name.")}, {"offset", IntegerSchema("Pagination offset.", 0)},
            {"limit", IntegerSchema("Maximum key/value pairs.", 1, 5000)}
        }, {"section"}, true, false, true));

    const Json createProperties = {
        {"width", IntegerSchema("Logical map width.", 16, 400)}, {"height", IntegerSchema("Logical map height.", 16, 400)},
        {"theater", StringSchema("Biome/theater.", {"TEMPERATE", "SNOW", "URBAN", "NEWURBAN", "LUNAR", "DESERT"})},
        {"targetGame", StringSchema("Compatibility target.", {"ra2", "yr"})},
        {"startingHeight", IntegerSchema("Initial ground height.", 0, 14)}, {"name", StringSchema("Map display name.")},
        {"author", StringSchema("Map author.")}, {"description", StringSchema("Map description.")},
        {"multiplayer", BooleanSchema("Create multiplayer metadata.")}, {"gameMode", StringSchema("Game mode label.")},
        {"minPlayers", IntegerSchema("Minimum players.", 1, 8)}, {"maxPlayers", IntegerSchema("Maximum players.", 1, 8)},
        {"playerHouse", StringSchema("Single-player human house.")}
    };
    tools.push_back(MakeTool("finalalert.create_map", "Create map",
        "Create a new RA2 or Yuri's Revenge map in the selected theater. This replaces the currently loaded map.",
        createProperties, {"width", "height"}, false, true, true));

    Json importProperties = createProperties;
    importProperties["imagePath"] = StringSchema("Absolute path to a BMP, PNG, or JPEG semantic image.");
    importProperties["imageBase64"] = StringSchema("Raw base64 or data URL for a BMP, PNG, or JPEG image.");
    importProperties["heightMode"] = StringSchema("Height source: none, luminance, or red channel.", {"none", "luminance", "red"});
    importProperties["maxHeight"] = IntegerSchema("Maximum height added by the image.", 0, 14);
    importProperties["markers"] = BooleanSchema("Recognize red spawns, orange ore, magenta gems, and cyan locations.");
    importProperties["markerTolerance"] = IntegerSchema("Per-channel marker color tolerance.", 0, 64);
    importProperties["autoShore"] = BooleanSchema("Generate shore transitions around water.");
    importProperties["autoSmooth"] = BooleanSchema("Run theater LAT smoothing after import.");
    tools.push_back(MakeTool("finalalert.import_semantic_image", "Import semantic image",
        "Create a map from an AI-generated BMP/PNG/JPEG: blue water, yellow sand, gray pavement, green terrain; red/orange/magenta/cyan are optional gameplay markers.",
        std::move(importProperties), {"width", "height"}, false, true, true));
    tools.push_back(MakeTool("finalalert.open_map", "Open map",
        "Open an existing absolute .map, .mpr, or .yrm without file dialogs. Replaces the current map.", {
            {"path", StringSchema("Absolute map path.")}
        }, {"path"}, false, true, true, true));
    tools.push_back(MakeTool("finalalert.save_map", "Save map",
        "Atomically save the current map without FinalAlert's modal save/options dialogs.", {
            {"path", StringSchema("Absolute .map, .mpr, or .yrm destination.")},
            {"includePreview", BooleanSchema("Embed the FinalAlert preview pack.")}, {"compressed", BooleanSchema("Use compressed pack data.")},
            {"createDirectories", BooleanSchema("Create missing parent directories.")}
        }, {"path"}, false, true, true, true));
    tools.push_back(MakeTool("finalalert.set_map_metadata", "Set map metadata",
        "Set map name, target game, player range, game mode, and single/multiplayer metadata. Does not convert theater.", {
            {"name", StringSchema("Map display name.")}, {"author", StringSchema("Map author.")},
            {"description", StringSchema("Map description.")}, {"targetGame", StringSchema("Compatibility target.", {"ra2", "yr", "auto"})},
            {"multiplayer", BooleanSchema("Whether the map is multiplayer-only.")}, {"gameMode", StringSchema("Game mode label.")},
            {"minPlayers", IntegerSchema("Minimum players.", 1, 8)}, {"maxPlayers", IntegerSchema("Maximum players.", 1, 8)},
            {"playerHouse", StringSchema("Single-player human house.")}
        }, {}, false, true, true));

    const Json cellItem = {
        {"type", "object"},
        {"properties", {
            {"x", IntegerSchema("Internal X.", 0)}, {"y", IntegerSchema("Internal Y.", 0)},
            {"height", IntegerSchema("Absolute height.", 0, 14)},
            {"terrain", StringSchema("Semantic terrain.", {"clear", "green", "rough", "sand", "pave", "water"})},
            {"tileIndex", IntegerSchema("Tile variant within a semantic tileset.", 0, 255)},
            {"tileId", IntegerSchema("Exact global tile ID.", 0)}, {"subTile", IntegerSchema("Exact subtile index.", 0, 255)},
            {"overlay", IntegerSchema("Overlay byte; 255 clears.", 0, 255)}, {"overlayData", IntegerSchema("Overlay data byte for non-resources.", 0, 255)}
        }},
        {"required", {"x", "y"}}, {"additionalProperties", false}
    };
    tools.push_back(MakeTool("finalalert.set_cells", "Set cells",
        "Apply a validated batch of exact height, semantic/exact tile, and overlay changes with one editor refresh.", {
            {"cells", { {"type", "array"}, {"items", cellItem}, {"minItems", 1}, {"maxItems", 20000} }},
            {"createSlopes", BooleanSchema("Generate slopes at changed cells.")}, {"smooth", BooleanSchema("Apply LAT smoothing at changed cells.")}
        }, {"cells"}, false, true, true));

    Json paintProperties = regionProperties;
    paintProperties["terrain"] = StringSchema("Semantic terrain.", {"clear", "green", "rough", "sand", "pave", "water"});
    paintProperties["tileIndex"] = IntegerSchema("Semantic tile variant.", 0, 255);
    paintProperties["tileId"] = IntegerSchema("Exact global tile ID.", 0);
    paintProperties["subTile"] = IntegerSchema("Exact subtile index.", 0, 255);
    paintProperties["height"] = IntegerSchema("Absolute height.", 0, 14);
    paintProperties["deltaHeight"] = IntegerSchema("Relative height adjustment.", -14, 14);
    paintProperties["createSlopes"] = BooleanSchema("Generate slopes in the region.");
    paintProperties["smooth"] = BooleanSchema("Apply LAT smoothing in the region.");
    tools.push_back(MakeTool("finalalert.paint_region", "Paint map region",
        "Paint a rectangular/circular terrain or height region on the playable diamond.", std::move(paintProperties), {"x", "y"}, false, true, true));

    Json resourceProperties = regionProperties;
    resourceProperties["resource"] = StringSchema("Resource operation.", {"ore", "gems", "clear"});
    resourceProperties["density"] = NumberSchema("Share of eligible cells to change.", 0.0, 1.0);
    resourceProperties["targetCells"] = IntegerSchema("Exact candidate cell count target.", 0);
    resourceProperties["targetCredits"] = IntegerSchema("Continue until at least this many added credits.", 0);
    resourceProperties["seed"] = IntegerSchema("Deterministic placement seed.", 0);
    resourceProperties["respectTerrain"] = BooleanSchema("Require AllowTiberium ground for placement.");
    tools.push_back(MakeTool("finalalert.place_resources", "Place resources",
        "Place deterministic ore/gems or clear resources in a region by density, cell count, or credit target.",
        std::move(resourceProperties), {"resource", "x", "y"}, false, true, true));
    tools.push_back(MakeTool("finalalert.place_waypoint", "Place waypoint/location",
        "Create or replace a numbered/named waypoint at a playable cell.", {
            {"id", StringSchema("Waypoint or location identifier.")}, {"x", IntegerSchema("Internal X.", 0)}, {"y", IntegerSchema("Internal Y.", 0)}
        }, {"id", "x", "y"}, false, false, true));
    tools.push_back(MakeTool("finalalert.set_player_spawns", "Set multiplayer spawns",
        "Replace continuous player waypoint IDs 0..7 and enable multiplayer metadata.", {
            {"spawns", { {"type", "array"}, {"items", coordinate}, {"minItems", 2}, {"maxItems", 8} }},
            {"minPlayers", IntegerSchema("Minimum players.", 1, 8)}, {"maxPlayers", IntegerSchema("Maximum players.", 2, 8)}
        }, {"spawns"}, false, true, true));
    tools.push_back(MakeTool("finalalert.place_object", "Place map object",
        "Place a validated structure, vehicle, infantry, aircraft, or terrain object from the loaded rules catalog.", {
            {"kind", StringSchema("Object kind.", {"structure", "unit", "infantry", "aircraft", "terrain"})},
            {"type", StringSchema("Rules type ID from list_catalog.")}, {"house", StringSchema("Owner house for non-terrain objects.")},
            {"x", IntegerSchema("Internal X.", 0)}, {"y", IntegerSchema("Internal Y.", 0)},
            {"id", StringSchema("Optional suggested INI object ID.")}, {"allowOverlap", BooleanSchema("Bypass blocking-object collision checks.")}
        }, {"kind", "type", "x", "y"}, false, false, false));
    tools.push_back(MakeTool("finalalert.delete_at", "Delete at cell",
        "Delete selected/all object categories or overlay from a playable cell.", {
            {"kind", StringSchema("Category to remove.", {"all", "structure", "unit", "infantry", "aircraft", "terrain", "waypoint", "overlay"})},
            {"x", IntegerSchema("Internal X.", 0)}, {"y", IntegerSchema("Internal Y.", 0)}
        }, {"x", "y"}, false, true, true));
    tools.push_back(MakeTool("finalalert.set_ini_value", "Set advanced map INI value",
        "Set/delete an advanced mission value (lighting, triggers, teams, scripts, tags). Derived map/object sections are protected.", {
            {"section", StringSchema("INI section name.")}, {"key", StringSchema("INI key.")},
            {"value", StringSchema("Value to write when delete is false.")}, {"delete", BooleanSchema("Delete this key.")}
        }, {"section", "key"}, false, true, true));
    tools.push_back(MakeTool("finalalert.apply_map_plan", "Apply complete map plan",
        "Apply create/metadata/cells/regions/resources/waypoints/spawns/objects/INI/save steps serially with checkpoint rollback for an existing map.", {
            {"plan", { {"type", "object"}, {"properties", {
                {"create", { {"type", "object"} }}, {"metadata", { {"type", "object"} }},
                {"cells", { {"type", "array"} }}, {"regions", { {"type", "array"} }},
                {"resources", { {"type", "array"} }}, {"waypoints", { {"type", "array"} }},
                {"spawns", { {"type", "array"} }}, {"objects", { {"type", "array"} }},
                {"ini", { {"type", "array"} }}, {"save", { {"type", "object"} }}
            }}, {"additionalProperties", false} }}
        }, {"plan"}, false, true, true));
    tools.push_back(MakeTool("finalalert.undo_terrain", "Undo terrain change",
        "Undo the last terrain/height/overlay snapshot. FinalAlert snapshots do not cover objects or INI.", {}, {}, false, true, false));
    tools.push_back(MakeTool("finalalert.redo_terrain", "Redo terrain change",
        "Redo the next terrain/height/overlay snapshot. FinalAlert snapshots do not cover objects or INI.", {}, {}, false, true, false));
    return tools;
}

bool CMcpMapTools::HasTool(const std::string& name)
{
    const auto& names = ToolNames();
    return std::find(names.begin(), names.end(), name) != names.end();
}

bool CMcpMapTools::IsMutation(const std::string& name)
{
    return !ReadOnlyTools().contains(name);
}

CMcpMapTools::Json CMcpMapTools::Execute(const std::string& name, const Json& arguments, CFinalSunDlg& dialog)
{
    if (!HasTool(name))
        return ToolFailure("unknown_tool", "Unknown FinalAlert MCP tool: " + name);
    if (!arguments.is_object())
        return ToolFailure("invalid_arguments", "Tool arguments must be a JSON object.");
    try
    {
        if (name == "finalalert.get_status") return HandleGetStatus();
        if (name == "finalalert.get_map_info") return HandleGetMapInfo();
        if (name == "finalalert.get_cells") return HandleGetCells(arguments);
        if (name == "finalalert.list_catalog") return HandleListCatalog(arguments);
        if (name == "finalalert.list_objects") return HandleListObjects(arguments);
        if (name == "finalalert.get_preview") return HandleGetPreview(arguments);
        if (name == "finalalert.validate_map") return HandleValidateMap();
        if (name == "finalalert.get_ini_section") return HandleGetIniSection(arguments);
        if (name == "finalalert.create_map") return HandleCreateMap(arguments, dialog);
        if (name == "finalalert.import_semantic_image") return HandleImportSemanticImage(arguments, dialog);
        if (name == "finalalert.open_map") return HandleOpenMap(arguments, dialog);
        if (name == "finalalert.save_map") return HandleSaveMap(arguments);
        if (name == "finalalert.set_map_metadata") return HandleSetMetadata(arguments, dialog);
        if (name == "finalalert.set_cells") return HandleSetCells(arguments, dialog);
        if (name == "finalalert.paint_region") return HandlePaintRegion(arguments, dialog);
        if (name == "finalalert.place_resources") return HandlePlaceResources(arguments, dialog);
        if (name == "finalalert.place_waypoint") return HandlePlaceWaypoint(arguments, dialog);
        if (name == "finalalert.set_player_spawns") return HandleSetPlayerSpawns(arguments, dialog);
        if (name == "finalalert.place_object") return HandlePlaceObject(arguments, dialog);
        if (name == "finalalert.delete_at") return HandleDeleteAt(arguments, dialog);
        if (name == "finalalert.set_ini_value") return HandleSetIniValue(arguments, dialog);
        if (name == "finalalert.apply_map_plan") return HandleApplyMapPlan(arguments, dialog);
        if (name == "finalalert.undo_terrain") return HandleUndo(dialog);
        if (name == "finalalert.redo_terrain") return HandleRedo(dialog);
        return ToolFailure("unknown_tool", "The requested tool is not implemented.");
    }
    catch (const ToolInputError& error)
    {
        return ToolFailure("invalid_arguments", error.what());
    }
    catch (const std::exception& error)
    {
        return ToolFailure("tool_exception", error.what());
    }
    catch (CException* error)
    {
        char message[1024] = {};
        error->GetErrorMessage(message, _countof(message));
        error->Delete();
        return ToolFailure("mfc_exception", message[0] ? message : "FinalAlert raised an MFC exception.");
    }
    catch (...)
    {
        return ToolFailure("tool_exception", "FinalAlert raised an unknown exception while executing the tool.");
    }
}
