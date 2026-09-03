# FinalSun / FinalAlert (YR) 2 Mission Editor

This repository contains the source code for the FinalSun/FinalAlert (YR) Mission Editor.

## Fork additions

- An in-process MCP server bound to loopback by default exposes 24 tools for inspecting,
  creating, editing, validating, previewing and saving RA2/YR maps. Coordinated
  edits and semantic-image imports use operation IDs for retry protection.
- **Options > MCP settings** configures startup, port, endpoint, authentication,
  allowlists and request-size limits. Settings are saved atomically and take
  effect after restarting the editor.
- Russian, Ukrainian and Spanish translations cover menus, dialogs and editor
  messages. Game-string decoding repairs legacy Cyrillic CSF text and supports
  an explicit encoding override.
- Regression checks cover translation completeness, CSF decoding and MCP
  configuration persistence. `MissionEditor/data/shared/Test-FinalAlertMcp.ps1`
  checks the MCP protocol against a running editor.

Build this fork with `FinalAlertRelease YR|Win32` and the v143 x86 toolchain.
The output under `dist/FinalAlert2YR/` includes the executable and runtime data;
keep the full package together. MCP connection details and localization notes
are included in the packaged `README-MCP-RU.md` and `README-LOCALIZATION-RU.md`.

### Language and game text settings

Choose **Options > Settings > Language / Sprache** to select English, German,
Russian, Ukrainian or Spanish. Keep the `Translations/` directory beside the
executable. Interface translations do not translate the installed game's unit
and building names: those come from `RA2.CSF` or `RA2MD.CSF`.

Some older Russian releases store Windows-1251 bytes in Unicode character
positions, producing text such as `Èíòðî` instead of `Интро`. The editor detects
this format and repairs names in memory without modifying game archives or map
files. If a small or mixed mod string table is misdetected, set the following
option in `%LOCALAPPDATA%\FinalAlert 2\FinalAlert.ini` and restart:

```ini
[FinalAlert]
GameStringEncoding=auto
```

Supported values are `auto` (detect legacy Russian text), `unicode` (preserve
the original Unicode) and `windows-1251` (force legacy Cyrillic decoding).

## Using MCP

MCP (Model Context Protocol) lets an AI assistant or another compatible client
operate this editor through structured tools. The server runs inside the
editor process and stops when the editor closes. It requires installed RA2/YR
game data; the MCP integration is not part of the legacy FA2sp injection DLL.

### Start the editor and connect

1. Build **FinalAlertRelease YR|Win32** with the **v143** x86 toolchain and the
   required ATL/MFC components. The complete runtime is written to
   `dist/FinalAlert2YR/`. Keep its executable, dependency DLLs, INI files and
   `Translations/` directory together. A machine running a copied build also
   needs the x86 Visual C++ runtime, including MFC.
2. Launch `FinalAlert2YR.exe` from that directory. Complete the initial
   game-path setup and close modal dialogs before using MCP tools.
3. Open **Options > MCP settings**. Enable the server, keep the listen address
   at `127.0.0.1`, and check the displayed URL. Press **OK** to save, then
   restart the editor. The dialog also shows whether the current server is
   running; saving new settings does not restart the current listener.
4. Add a server to your local MCP client using these connection details:

   | Setting | Default |
   | --- | --- |
   | Transport | Streamable HTTP |
   | MCP URL | `http://127.0.0.1:7462/mcp` |
   | Health URL | `http://127.0.0.1:7462/health` |
   | Authentication | None when `AuthToken` is empty; otherwise Bearer token |

5. Let the client initialize the MCP connection and discover `tools/list`.
   Start with `finalalert.get_status`. Read the published `inputSchema` before
   making tool calls; it is the authoritative argument contract.

When a token is configured, send `Authorization: Bearer <your-token>` on every
request, including `/health`. Keep secrets in the client's credential storage
or environment, not in committed configuration. Keep `BindAddress=127.0.0.1`;
a remote client needs an authenticated tunnel or controlled proxy to reach a
local editor. Opening `/mcp` in a browser is not an MCP connection.

### Configuration

Settings are stored in `[Server]` in `FinalAlertMCP.ini` beside the executable.
You can edit the file or use the settings dialog. Restart the editor after
changes, and update your client when the port, endpoint or token changes.

| Key | Default | Meaning |
| --- | --- | --- |
| `Enabled` | `1` | Start the MCP server with the editor; `0` disables it. |
| `BindAddress` | `127.0.0.1` | Keep the server local to this computer. |
| `Port` | `7462` | TCP port, from 1 to 65535. |
| `Endpoint` | `/mcp` | MCP request path; `/health` is reserved. |
| `AuthToken` | Empty | Optional Bearer token, also required for health checks. |
| `AllowedHosts` | `localhost,127.0.0.1,[::1]` | Accepted HTTP Host values. |
| `AllowedOrigins` | `https://chatgpt.com,https://chat.openai.com` | Accepted external browser origins; local origins on the configured port are also accepted. |
| `MaxRequestBytes` | `16777216` | Request limit, 16 MiB by default; the dialog accepts 1024 to 67108864 bytes. |

Avoid wildcard allowlists. If saving fails, ensure that the editor's directory
is writable. A failed settings-file write leaves the previous file intact.

### Check the connection

With the editor running, execute these commands from its runtime directory.
This example uses the default port, `/mcp` path and empty authentication token:

```powershell
Invoke-RestMethod http://127.0.0.1:7462/health
powershell -ExecutionPolicy Bypass -File .\Test-FinalAlertMcp.ps1
```

For an authenticated server, populate `FINALALERT_MCP_TOKEN` in the calling
process environment, then use:

```powershell
$headers = @{ Authorization = "Bearer $env:FINALALERT_MCP_TOKEN" }
Invoke-RestMethod http://127.0.0.1:7462/health -Headers $headers
powershell -ExecutionPolicy Bypass -File .\Test-FinalAlertMcp.ps1
```

The smoke script checks health, tool discovery and status using the server's
session-based and stateless protocol paths. It does not change the loaded map.
Use `-BaseUrl http://127.0.0.1:7463` for a different port. The current script
still asserts that the health response advertises `/mcp`; its `-Endpoint`
parameter alone is not sufficient to test a custom endpoint path.

### Example tool calls

The following JSON objects are the `params` for MCP `tools/call`. Supply them
through your connected MCP client; they are not standalone HTTP requests.

Inspect the editor first:

```json
{"name":"finalalert.get_status","arguments":{}}
```

With a map loaded, query the active catalog instead of guessing unit IDs:

```json
{"name":"finalalert.list_catalog","arguments":{"kind":"unit","offset":0,"limit":50}}
```

To create a blank RA2 map, first save any work you want to keep: creation
replaces the currently loaded map. Generate a fresh UUID for each new mutation
(PowerShell: `[guid]::NewGuid().ToString()`). The IDs below illustrate the format;
replace them before using these examples for new operations.

```json
{
  "name": "finalalert.create_map",
  "arguments": {
    "operationId": "52383cb7-b7a0-41f1-91e3-8e22b6418141",
    "width": 80,
    "height": 80,
    "theater": "TEMPERATE",
    "targetGame": "ra2",
    "startingHeight": 0,
    "name": "MCP Example"
  }
}
```

Creation alone does not produce a playable multiplayer map. Add and validate
starts, resources and metadata before distributing it. To save a working copy,
choose an existing writable directory and an absolute path to a new file:

```json
{
  "name": "finalalert.save_map",
  "arguments": {
    "operationId": "f7e4b8ce-ff83-46ab-97ef-a6b32707d9bc",
    "path": "C:\\Maps\\mcp_example.mpr",
    "includePreview": true
  }
}
```

### Map editing workflow

1. Call `finalalert.get_status` and `finalalert.get_map_info`, or open/create a
   map. Always specify `targetGame` as `ra2` or `yr` when creating/importing.
   RA2 supports `TEMPERATE`, `SNOW` and `URBAN`, not the YR-only `NEWURBAN`,
   `LUNAR` or `DESERT` theaters.
2. Read `finalalert.list_catalog` for tiles, overlays, houses and object types.
   Coordinates are isometric map cells, not preview pixels. Inspect cells
   before changing them. Prefer `finalalert.apply_map_plan` for coordinated
   edits and use `set_cells` or `paint_region` for precise terrain changes.
3. Place resources, objects and continuous player waypoints `0..N-1` after
   terrain is stable. Check `Basic`, `Map`, `Header` and `Waypoints`, including
   valid `HomeCell`/`AltHomeCell` references and matching starting-point metadata.
   Verify clear, buildable starts, intended land connectivity and resource
   distribution. For standard RA2 multiplayer compatibility use the full
   `GameMode` list: `standard, meatgrind, navalwar, nukewar, airwar, megawealth, duel, cooperative, teamgame`.
4. Call `finalalert.validate_map` and `finalalert.get_preview`. Inspect the image,
   check all content against the target-game catalogs and save with
   `includePreview: true`. Use a lowercase filename without spaces; RA2
   multiplayer files use `.mpr`. Keep backups outside game-scanned map
   extensions and use separate temporary files for tests.
5. Reopen the exact saved file, rerun validation and render its preview. Audit
   the saved packed sections for valid tile records, contiguous chunks and
   expected decoded sizes: 262144 bytes each for `OverlayPack` and
   `OverlayDataPack`, and `previewWidth * previewHeight * 3` for `PreviewPack`.
   Perform a real game load test before claiming runtime compatibility. A
   zero-issue result from `validate_map` does not prove reachability, balance,
   correct saved packs or successful loading in the game.

Every mutating tool requires a non-empty unique `operationId`. After a timeout,
inspect the editor state and retry only the identical request with the original
ID. Retry caching is not a durable transaction log across editor restarts or
client sessions. Terrain undo/redo covers tiles, heights and overlays, not
objects, waypoints or arbitrary INI edits.

### Available tools

All names below have the `finalalert.` prefix. Use `tools/list` for current
argument schemas, limits and annotations.

| Group | Tools |
| --- | --- |
| Read-only inspection (8) | `get_status`, `get_map_info`, `get_cells`, `list_catalog`, `list_objects`, `get_preview`, `validate_map`, `get_ini_section` |
| Map lifecycle | `create_map`, `import_semantic_image`, `open_map`, `save_map`, `set_map_metadata` |
| Terrain and resources | `set_cells`, `paint_region`, `place_resources` |
| Objects and starts | `place_waypoint`, `set_player_spawns`, `place_object`, `delete_at` |
| Advanced edits and terrain history | `set_ini_value`, `apply_map_plan`, `undo_terrain`, `redo_terrain` |

All 16 tools outside the read-only group require an `operationId`, including
opening and saving a map.

### Importing a semantic image

`finalalert.import_semantic_image` accepts exactly one source: an absolute
`imagePath` or `imageBase64` (raw base64 or a data URL), for BMP, PNG or JPEG.
The image is a terrain mask, not a texture or a screenshot to reproduce.
For predictable classification, use flat PNG/BMP colors without labels,
shadows, antialiasing or compression artifacts:

| Meaning | Color |
| --- | --- |
| Clear ground | `#000000` |
| Grass | `#00C800` |
| Rough ground | `#004000` |
| Sand | `#E6C850` |
| Paving | `#808080` |
| Water | `#0000FF` |
| Player start | `#FF0000` |
| Ore | `#FF8000` |
| Gems | `#FF00FF` |
| Location waypoint | `#00FFFF` |

Set `heightMode: "none"` when colors should only control terrain and markers;
adjust elevation afterwards with terrain tools. Marker and smoothing options
are described by `tools/list`. Inspect the imported cells and waypoints before
placing objects or saving a deliverable map.

### Troubleshooting

| Symptom | What to check |
| --- | --- |
| Connection refused or health timeout | Start the editor, enable MCP, check for a port conflict and restart after settings changes. |
| HTTP 401 | Send the configured Bearer token, including on `/health`. |
| HTTP 403 | Check the Host/Origin allowlists and the actual client headers. |
| HTTP 405 on `GET /mcp` | Use a Streamable HTTP MCP client; a browser GET does not invoke a tool. |
| `editor_busy` | Close modal editor dialogs, then retry the identical mutation with its original operation ID. |
| Unknown object or tile | Refresh `list_catalog` for the currently loaded game, theater and mod. |
| No map loaded | Open or create a map before invoking map-dependent tools. |
| Settings cannot be saved | Check write access to the runtime directory. |

## Upstream project

The official version for both applications has been increased to v2.0 in order to illustrate they now run properly on modern operating systems. We also used this opporunity to update the application and fix some known issues.

# Fixes and Changes
- Fixed a few code issues to allow the application to run on modern operating systems.
- Updated the application icons with new 256x256 graphics.
- You can now zoom in & out with the middle mouse button or wheel
- You don't need administrator access anymore if the program is installed in the program folder, as all user data and settings are now stored in your AppData folder
- Tunnel tube editing completely reimplemented
  + Tunnel tilesets available
  + You can edit existing tubes by using the tool on the end of an existing tube
  + You can now create curved tubes
  + You can now create unidirectional tubes
- LAT support for Crystal and Swamp terrain in Tiberian Sun temperate maps
- House colors are now read from map or rules.ini
- Units and buildings are now shaded in the house colors correctly
- Voxel units now have shading applied
- Minimap can now be resized
- Undo steps increased to 64
- Map rendering performance improved
- Several fixes for crashes
- Fixed addon turrets display
- SHP turrets and Voxel turrets + barrels have their positioning fixed.
- Maps up to 400x112 (or 112x400) are now allowed.
- Fixed minimap for non-quadratic maps.

# Install build requirements
- Microsoft Visual Studio 2022
  1. Install Visual Studio 2022 if you haven't done so already.
  2. (Re-)Start the Visual Studio Installer
  3. Apply any updates to Visual Studio 2022
  4. Click on Modify.
  5. Under Workloads ensure "Desktop Development with C++" is enabled. Ensure the details are visible on the right panel below "Desktop Development with C++" and enable the following checkboxes:
     - MSVC v143 - VS 2022 C++-x64/x86 build tools
     - C++-ATL for v143 build tools x86 & x64
     - vcpkg package manager: if this item is missing, please check that you have updated Visual Studio 2022!
     - newest Windows 10 SDK
     - newest Windows 11 SDK
     - C++-MFC for v143 build tools x86 & x64
     - Do not untick any other checkboxes!
  6. Apply the changes.
- Git™ for Windows - Ensure you do not run any other git implementation when using vcpkg.

# Building the source code
The source code consists of 2 projects:
- MissionEditor: The main mission editor application code.
- MissionEditorPackLib: Wraps XCC objects with C functions and includes some loading and packing code.

## Compilation
1. Open `MissionEditor.sln` in Visual Studio with the required v143 components.
2. Select **FinalAlertRelease YR** and **Win32**.
3. Build the solution. The executable and runtime data are written to
   `dist/FinalAlert2YR/`.
4. Run the executable from that directory or use Visual Studio debugging.

## Updating third-party libraries
In order to update zlib, bzip, boost and lzo to newest version, start a "Developer Command Prompt for VS 2022", move to the "3rdParty\xcc" folder with vcpkg and Git for Windows in PATH:
   
       git --version
       vcpkg x-update-baseline

A version of XCC is directly included in "3rdParty\xcc". It is a slightly modified and heavily stripped down version. The changes are available in `3rdParty\xcc\patch.<COMMIT_HASH>.diff`, where `COMMIT_HASH` is the commit hash of the XCC repository (at the time of this writing `3rdParty\xcc\patch.70358b46858973426c1ecf204485cb2a88716217.diff`). If you update XCC to a newer version by replacing the files, please also apply the (eventually updated) patch afterwards.

# Creating a distribution
After modifying the source code you might want to create a distribution.
Please ensure that you have taken care of everything required by the licenses (e.g. notices of source code changes and your copyright statement) and that the program itself does make clear it has been modified by you.

There is a helper script that can create the distributions in the script folder:

    cd scripts
    build_and_distribute.bat

This rebuilds everything and creates several files in the dist folder:

  - FinalSun.zip
    - Contains a FinalSun distribution
  - FinalAlert2.zip
    - Contains a FinalAlert2 distribution
  - FinalAlert2YR.zip
    - Contains a FinalAlert2 Yuri's Revenge distribution
  - MissionEditorSource.zip
    - The source code of this git repository (no uncommitted changes included)
  - MissionEditorExternalSources.zip
    - A best-effort dump of third party source code and binaries used for the build. This possibly contains material not for distribution! You should archive this for reference and/or in the event that original sources become unavailable.	

When distributing, you yourself are responsible for fulfilling all license requirements both of this repository and of all third party libraries being used.
There is no warranty that the helper script automatically fulfills all requirements.

# Directories
MissionEditor\data\shared - Data copied to both FS/FA2 distribution
MissionEditor\data\FinalAlert2 - Additional data copied to FA2 distribution
MissionEditor\data\FinalSun - Additional data copied to FS distribution

MissionEditor\PropertySheets - Some property sheets to ease managing the many project configurations

dist - Will contain the output binaries in subfolders. Required DLLs of third-party libraries and the data directory will be copied there using MissionEditor\PropertySheets\common.props

# Contributions
This is an archive repository and will not be maintained. We suggest you fork this repository to fix any issues or add new features.

# Source
Most of the source code has been written around 1999-2001 and does not take advantage of modern C++ features (e.g. smart pointers or RAII) or hardware rendering. However, we've enabled C++20 support and refactored some critical parts to be up to date.

# Authors
- Electronic Arts Inc.
- Matthias Wagner
  + Author of the original FinalSun and FinalAlert 2 (YR)
  + Bug fixes, updates and refactorings
  + Updating the build system
- Olaf van der Spek
  + For the XCC Library
- Luke "CCHyper" Feenan
  + Additional programming
  + Preparing the source code for open source release
  + New icons and various other graphics

# Special Thanks
We'd like to thank the team at Electronic Arts for approving and making the release of the source code possible.

# Legal
The video games "Command & Conquer: Tiberian Sun", "Command & Conquer: Red Alert 2", and "Command & Conquer: Yuri's Revenge" are copyright of Westwood Studios. All Rights Reserved. Westwood Studios is a trademark or registered trademark of Electronic Arts in the U.S. and/or other countries. All rights reserved.

Microsoft, DirectX, Visual C++, Visual Studio and Windows are trademarks of the Microsoft group of companies.

Git and the Git logo are either registered trademarks or trademarks of Software Freedom Conservancy, Inc., corporate home of the Git Project, in the United States and/or other countries.

# License
Unless otherwise stated, the source code provided in this repository is licenced under the [GNU General Public License version 3](<https://www.gnu.org/licenses/gpl-3.0.html>). Please see the accompanying LICENSE file and the actual files of interest.

Third party libraries in the 3rdParty folder may be licensed differently. Please see their accompanying LICENSE or COPYING file and the actual files of interest.
