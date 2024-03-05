/*
    FinalSun/FinalAlert 2 Mission Editor

    Copyright (C) 1999-2024 Electronic Arts, Inc.
    Authored by Matthias Wagner

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*************************************
VARIABLES.CPP

INCLUDES ALL FINAL SUN CONSTANTS AND
VARIABLES USED
*************************************/

#include "stdafx.h"
#include "mapdata.h"
#include "structs.h"
#include "FinalSun.h"
#include <Shlobj.h>
#include "MapTool.h"
#include "inlines.h"
#include <string>
#include <codecvt>

/**INI Files**/
CIniFile rules;
CIniFile art;
CIniFile ai;
CIniFile sound;
CIniFile tutorial;
CIniFile eva; 
CIniFile theme;
CIniFile g_data;
CIniFile language;
CIniFile tiles_t; // temperat.ini shouldn´t be used except in CMapData::UpdateIniFile() and CLoading
CIniFile tiles_s; // snow.ini shouldn´t be used except in CMapData::UpdateIniFile() and CLoading
CIniFile tiles_u; // urban.ini ...
CIniFile tiles_un; // new urbannmd.ini
CIniFile tiles_l; // lunarmd.ini
CIniFile tiles_d; // desertmd.ini
CIniFile* tiles=NULL; // this should be used by every class/function except CMapData::UpdateIniFile();
/*************/

/* the mapdata! */
CMapData* Map;

/* actiondata used for iso view */
ACTIONDATA AD;

/* A map with all the pictures in the pics directory, and some special pics */
map<CString, PICDATA> pics;
TILEDATA* t_tiledata=NULL;
DWORD t_tiledata_count=0;
TILEDATA* s_tiledata=NULL;
DWORD s_tiledata_count=0;
TILEDATA* u_tiledata=NULL;
DWORD u_tiledata_count=0;

// MW new tilesets
TILEDATA* un_tiledata=NULL;
DWORD un_tiledata_count=0;
TILEDATA* l_tiledata=NULL;
DWORD l_tiledata_count=0;
TILEDATA* d_tiledata=NULL;
DWORD d_tiledata_count=0;

TILEDATA** tiledata=NULL;

DWORD* tiledata_count=NULL;

map<int, int> tilesets_start;

// Palettes:
RGBTRIPLE palIso[256];
RGBTRIPLE palUnit[256];
RGBTRIPLE palStd[256];
RGBTRIPLE palTheater[256];
RGBTRIPLE palLib[256];

int iPalIso[256];
int iPalUnit[256];
int iPalStd[256];
int iPalTheater[256];
int iPalLib[256];

int bpp;

map<CString, int> color_conv;
map<COLORREF, int> colorref_conv;

map<int, HOUSEINFO> houses;
map<int, SIDEINFO> sides;

map<CString, BOOL> missingimages;

vector<CString> rndterrainsrc;

/* Overlay tile data */
#ifndef RA2_MODE
int overlay_number[]={0x0,0x2, 0x1a, 0x7e, 0xa7, 0x27};
CString overlay_name[]={"Sandbags","GDI Wall", "Nod Wall", "Veins", "Veinhole monster", "Tracks"};
BOOL overlay_visible[]={TRUE,TRUE,TRUE,FALSE,FALSE, TRUE};
BOOL overlay_trail[]={TRUE,TRUE,TRUE,FALSE,FALSE, TRUE};
BOOL overlay_trdebug[]={FALSE,FALSE,FALSE,FALSE,FALSE, FALSE};
BOOL yr_only[]={FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
int overlay_count=6;
const std::string editor_name = "FinalSun";
#else
int overlay_number[]={0x0,0x2, 0x1a, 0xcb, 0xf1, 0xcc,0xf3,0xf0, 0x27};
BOOL overlay_trdebug[]={FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE};
CString overlay_name[]={"Sandbags","Allied Wall", "Soviet Wall", "Black fence", "Prison camp fence", "White fence", "Yuri Wall", "Kremlin Wall", "Tracks"};
BOOL overlay_visible[]={TRUE,TRUE,TRUE,TRUE,TRUE,TRUE, TRUE, TRUE, TRUE};
BOOL overlay_trail[]={TRUE,TRUE,TRUE,TRUE,TRUE,TRUE, TRUE, TRUE, TRUE};
BOOL yr_only[]={FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE};
int overlay_count=9;
const std::string editor_name = "FinalAlert 2";
#endif

static const std::string GetAppDataPath()
{
    _setmbcp(CP_UTF8);
    setlocale(LC_ALL, "C");
    if (!setlocale(LC_CTYPE, ".65001"))
        setlocale(LC_CTYPE, "");
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CComPtr<IKnownFolderManager> manager;
    CComPtr<IKnownFolder> local_app_data;
    CComHeapPtr<WCHAR> local_app_data_folder;
    HRESULT hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&manager));
    if (SUCCEEDED(hr)) {
        //std::shared_ptr<LPWSTR> x()
        
        if (SUCCEEDED(manager->GetFolder(FOLDERID_LocalAppData, &local_app_data))) {
            LPWSTR b;
            local_app_data->GetPath(KF_FLAG_CREATE, &b);
            local_app_data_folder.Attach(b);
            int a = 0;
            std::string AppFolder = utf16ToUtf8(std::wstring(local_app_data_folder));
            //return CString(CW2A(CStringW(local_app_data_folder), CP_ACP)) + "\\" + editor_name + "\\";
            return AppFolder + "\\" + editor_name + "\\";
        }
    }

    // fallback: use app directory
    wchar_t app_path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, app_path, MAX_PATH);
    std::wstring appPath = app_path;
    std::size_t end = appPath.rfind(L'\\');
    if (end != std::wstring::npos)
        appPath.resize(end + 1);
    return utf16ToUtf8(appPath);
}

/* Application specific global variables */
char AppPath[MAX_PATH + 1] = { 0 };
const std::string u8AppDataPath = GetAppDataPath();
const std::wstring u16AppDataPath = utf8ToUtf16(u8AppDataPath);
char TSPath[MAX_PATH + 1] = { 0 };
char currentMapFile[MAX_PATH + 1] = { 0 };
BOOL bOptionsStartup=FALSE;
bool bAllowAccessBehindCliffs=false;


// infos for buildings and trees (should be extended to infantry, units, and aircraft)
// they are initialized in CIsoView, should be changed to CMapData
BUILDING_INFO buildinginfo[0x0F00];
TREE_INFO treeinfo[0x0F00];
#ifdef SMUDGE_SUPP
SMUDGE_INFO smudgeinfo[0x0F00];
#endif

/* error output */
ofstream errstream;

/* the finalsun app object */
CFinalSunApp theApp;

CString currentOwner="Neutral";

map<CString, XCString> CCStrings;
map<CString, XCString> AllStrings;

// tilesets
int cliffset;
int cliffset_start;
int rampset;
int rampsmoothset;
int slopesetpiecesset;
int tunnelset;
int tracktunnelset;
int dirttunnelset;
int dirttracktunnelset;
int waterset;
int shoreset;
int ramp2set_start;
int pave2set_start;
int rampset_start;
int ramp2set;
int pave2set;
int cliff2set;
int cliff2set_start;
int cliffwater2set;

// debug information
int last_succeeded_operation=0;

#ifdef RA2_MODE
int editor_mode=1;
#ifdef YR_MODE
int yuri_mode=1;
#else
int yuri_mode=0;
#endif
#else
int editor_mode=0;
int yuri_mode=0;
#endif