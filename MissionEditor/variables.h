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

/*******************************************
	Declare the global variables here
*******************************************/

#ifndef VARIABLES_H_INCLUDED
#define VARIABLES_H_INCLUDED

#include "structs.h"
#include "FinalSun.h"
#include "MapData.h"

// the map
extern CMapData* Map;

// the ini files:
extern CIniFile rules;
extern CIniFile art;
extern CIniFile ai;
extern CIniFile sound;
extern CIniFile tutorial;
extern CIniFile eva;
extern CIniFile theme;
extern CIniFile g_data;
extern CIniFile language;
extern CIniFile tiles_t;
extern CIniFile tiles_s;
extern CIniFile tiles_u;
extern CIniFile tiles_un; // new urbannmd.ini
extern CIniFile tiles_l; // new lunarmd.ini
extern CIniFile tiles_d; // new desertmd.ini
extern CIniFile* tiles;

// first time options are shown (at startup)
extern BOOL bOptionsStartup;

// the current file beeing edited.
extern char currentMapFile[MAX_PATH + 1];

// all the pictures shown in the mapview
extern map<CString, PICDATA> pics;
extern TILEDATA* t_tiledata;
extern TILEDATA* s_tiledata;
extern TILEDATA* u_tiledata;
extern TILEDATA** tiledata;
extern DWORD s_tiledata_count;
extern DWORD t_tiledata_count;
extern DWORD u_tiledata_count;
extern DWORD* tiledata_count;

// MW new tilesets
extern TILEDATA* un_tiledata;
extern DWORD un_tiledata_count;
extern TILEDATA* l_tiledata;
extern DWORD l_tiledata_count;
extern TILEDATA* d_tiledata;
extern DWORD d_tiledata_count;

extern map<int, int> tilesets_start;

extern RGBTRIPLE palIso[256];
extern RGBTRIPLE palUnit[256];
extern RGBTRIPLE palStd[256];
extern RGBTRIPLE palTheater[256];
extern RGBTRIPLE palLib[256];

extern int iPalIso[256];
extern int iPalUnit[256];
extern int iPalStd[256];
extern int iPalTheater[256];
extern int iPalLib[256];
extern int bpp;

extern map<CString, int> color_conv;
extern map<COLORREF, int> colorref_conv;

extern map<int, HOUSEINFO> houses;
extern map<int, SIDEINFO> sides;

extern map<CString, BOOL> missingimages;

extern vector<CString> rndterrainsrc;

// infos for buildings and trees (should be extended to infantry, units, and aircraft)
// they are initialized in CIsoView, should be changed to CMapData
extern BUILDING_INFO buildinginfo[0x0F00];
extern TREE_INFO treeinfo[0x0F00];
#ifdef SMUDGE_SUPP
extern SMUDGE_INFO smudgeinfo[0x0F00];
#endif

// the app object:
extern CFinalSunApp theApp;

/* error output */
extern ofstream errstream;

// application path
extern char AppPath[MAX_PATH + 1];
extern const std::string u8AppDataPath;
extern const std::wstring u16AppDataPath;
extern char TSPath[MAX_PATH + 1];

extern bool bAllowAccessBehindCliffs;

// overlay types (the ones with additional information like name etc.)
extern int overlay_count; // number of overlay ids that have additional information
extern int overlay_number[]; // what overlay id?
extern CString overlay_name[]; // what name?
extern BOOL overlay_trail[]; // is it handled as trail?
extern BOOL overlay_trdebug[];
extern BOOL yr_only[];


extern CString currentOwner;
extern map<CString, XCString> CCStrings;

// tileset ids
extern int cliffset;
extern int cliffset_start;
extern int rampset;
extern int rampsmoothset;
extern int slopesetpiecesset;
extern int waterset;
extern int shoreset;
extern int ramp2set;
extern int pave2set;
extern int ramp2set_start;
extern int pave2set_start;
extern int rampset_start;
extern int cliff2set;
extern int cliff2set_start;
extern int cliffwater2set;


// ra2 or TS mode?
extern int editor_mode;
extern int yuri_mode;

// debug
extern int last_succeeded_operation;

#endif