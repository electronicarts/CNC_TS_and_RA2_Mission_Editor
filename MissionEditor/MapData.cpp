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

// Map->cpp: Implementierung der Klasse CMap.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapData.h"
#include "DynamicGraphDlg.h"
#include "MissionEditorPackLib.h"
#include "inlines.h"
#include "variables.h"
#include "maploadingdlg.h"
#include "progressdlg.h"
#include "Structs.h"
#include "Tube.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

typedef map<CString, CString, SortDummy> OURMAP;



void DoEvents()
{
	/*MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}*/
}


void GetNodeName(CString& name, int n);


CString GetFree(const char* section)
{
	CIniFile& ini = Map->GetIniFile();

	int i = 0;
	char l[50];
	itoa(i, l, 10);

	while (ini.sections[section].values.find(l) != ini.sections[section].values.end())
	{
		i++;
		itoa(i, l, 10);
	}

	return l;
}


/*
This function calculates a number for the specified building type.
Because this function is slow, you should only use it to fill the
buildingid map
*/
inline int GetBuildingNumber(LPCTSTR name)
{
	CIniFile& ini = Map->GetIniFile();

	int v = rules.sections["BuildingTypes"].FindValue(name);

	if (v > -1)
	{

		return v;
	}

	v = ini.sections["BuildingTypes"].FindValue(name);
	if (v > -1)
	{
		return v + 0x0C00;
	}
	return -1;
}

inline int GetTerrainNumber(LPCTSTR name)
{
	CIniFile& ini = Map->GetIniFile();

	int v = rules.sections["TerrainTypes"].FindValue(name);

	if (v > -1)
	{

		return v;
	}

	v = ini.sections["TerrainTypes"].FindValue(name);
	if (v > -1)
	{
		return v + 0x0C00;
	}
	return -1;
}

#ifdef SMUDGE_SUPP
inline int GetSmudgeNumber(LPCTSTR name)
{
	CIniFile& ini = Map->GetIniFile();

	int v = rules.sections["SmudgeTypes"].FindValue(name);

	if (v > -1)
	{

		return v;
	}

	v = ini.sections["SmudgeTypes"].FindValue(name);
	if (v > -1)
	{
		return v + 0x0C00;
	}
	return -1;
}
#endif

SNAPSHOTDATA::SNAPSHOTDATA()
{
	memset(this, 0, sizeof(SNAPSHOTDATA));
}


NODEDATA::NODEDATA()
{
	type = -1;
	house = "";
	index = -1;
}

FIELDDATA::FIELDDATA()
{
#ifdef SMUDGE_SUPP
	smudge = -1;
	smudgetype = -1;
#endif
	unit = -1;
	int i;
	for (i = 0;i < SUBPOS_COUNT;i++)
		infantry[i] = -1;
	aircraft = -1;
	structure = -1;
	structuretype = -1;
	terrain = -1;
	waypoint = -1;
	overlay = 0xFF;
	overlaydata = 0x0;
	bMapData = 0x0;
	bSubTile = 0;
	bMapData2 = 0x0;
	wGround = 0xFFFF;
	bHeight = 0;
	celltag = -1;
	bReserved = 0;
	bCliffHack = 0;
	bRedrawTerrain = 0;
	terraintype = -1;
	bHide = FALSE;
	//sTube = 0xFFFF;
	//cTubePart = -1;

	if (tiledata_count && (*tiledata_count)) // only if initialized
	{
		// algorithm taken from UpdateMapFieldData
		int replacement = 0;
		int ground = wGround;
		if (ground == 0xFFFF) ground = 0;
		ASSERT(ground == 0);

		// we assume ground 0 will never be a bridge set, so don't do this check here in contrast to UpdateMapFieldData
		if ((*tiledata)[ground].bReplacementCount) //&& atoi((*tiles).sections["General"].values["BridgeSet"]) != (*tiledata)[ground].wTileSet)
		{
			replacement = rand() * (1 + (*tiledata)[ground].bReplacementCount) / RAND_MAX;
		}

		bRNDImage = replacement;
	}

};

CMapData::CMapData()
{
	m_noAutoObjectUpdate = FALSE;
	m_money = 0;
	m_cursnapshot = -1;
	fielddata = NULL;
	fielddata_size = 0;
	m_IsoSize = 0;
	isInitialized = FALSE;
	tiledata = NULL;
	m_mfd = NULL;
	dwIsoMapSize = 0;
	m_snapshots = NULL;
	dwSnapShotCount = 0;

	memset(&m_mini_biinfo, 0, sizeof(BITMAPINFO));
	m_mini_biinfo.bmiHeader.biBitCount = 24;
	m_mini_biinfo.bmiHeader.biWidth = 0;
	m_mini_biinfo.bmiHeader.biHeight = 0;
	m_mini_biinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_mini_biinfo.bmiHeader.biClrUsed = 0;
	m_mini_biinfo.bmiHeader.biPlanes = 1;
	m_mini_biinfo.bmiHeader.biCompression = BI_RGB;
	m_mini_biinfo.bmiHeader.biClrImportant = 0;
}

CMapData::~CMapData()
{
	errstream << "CMapData::~CMapData()\n";
	errstream.flush();

	// MW : Do not delete tiledata here!
	//if(*tiledata!=NULL) delete[] *tiledata;
	tiledata = NULL;
	if (m_mfd != NULL) delete[] m_mfd;
	m_mfd = NULL;
	if (fielddata != NULL) delete[] fielddata;
	fielddata = NULL;
	fielddata_size = 0;

	int i;

	for (i = 0;i < dwSnapShotCount;i++)
	{
		delete[] m_snapshots[i].bHeight;
		delete[] m_snapshots[i].bMapData;
		delete[] m_snapshots[i].bSubTile;
		delete[] m_snapshots[i].bMapData2;
		delete[] m_snapshots[i].wGround;
		delete[] m_snapshots[i].bRedrawTerrain;
		delete[] m_snapshots[i].overlay;
		delete[] m_snapshots[i].overlaydata;
		// m_snapshots[i].mapfile.Clear();
	}
	if (m_snapshots) delete[] m_snapshots;
	m_snapshots = NULL;
	dwSnapShotCount = 0;
}

void CMapData::CalcMapRect()
{
	CIniFileSection& sec = m_mapfile.sections["Map"];
	char msize[50];
	strcpy_s(msize, sec.values["Size"]);


	int cupos = 0;
	static const int custr_size = 20;
	char custr[custr_size];
	char* cucomma;

	cucomma = strchr(&msize[cupos], ',');
	if (cucomma == NULL) return;
	memcpy_s(custr, custr_size, &msize[cupos], (cucomma - msize) - cupos + 1);
	custr[(cucomma - msize) - cupos] = 0;
	cupos = cucomma - msize + 1;

	m_maprect.left = atoi(custr);


	cucomma = strchr(&msize[cupos], ',');
	if (cucomma == NULL) return;
	memcpy_s(custr, custr_size, &msize[cupos], (cucomma - msize) - cupos + 1);
	custr[((cucomma - msize)) - cupos] = 0;
	cupos = cucomma - msize + 1;

	m_maprect.top = atoi(custr);


	cucomma = strchr(&msize[cupos], ',');
	if (cucomma == NULL) return;
	memcpy_s(custr, custr_size, &msize[cupos], (cucomma - msize) - cupos + 1);
	custr[((cucomma - msize)) - cupos] = 0;
	cupos = cucomma - msize + 1;

	m_maprect.right = atoi(custr);

	cucomma = strchr(&msize[cupos], ','); // we check again... could be there is a new ini format
	if (cucomma == NULL) cucomma = (char*)((int)msize + strlen(msize));
	memcpy_s(custr, custr_size, &msize[cupos], (cucomma - msize) - cupos + 1);
	custr[((cucomma - msize)) - cupos] = 0;
	cupos = cucomma - msize + 1;

	m_maprect.bottom = atoi(custr);

	m_IsoSize = m_maprect.right + m_maprect.bottom;

	// local size

	strcpy_s(msize, sec.values["LocalSize"]);
	cupos = 0;


	cucomma = strchr(&msize[cupos], ',');
	if (cucomma == NULL) return;
	memcpy_s(custr, custr_size, &msize[cupos], (cucomma - msize) - cupos + 1);
	custr[(cucomma - msize) - cupos] = 0;
	cupos = cucomma - msize + 1;

	m_vismaprect.left = atoi(custr);


	cucomma = strchr(&msize[cupos], ',');
	if (cucomma == NULL) return;
	memcpy_s(custr, custr_size, &msize[cupos], (cucomma - msize) - cupos + 1);
	custr[((cucomma - msize)) - cupos] = 0;
	cupos = cucomma - msize + 1;

	m_vismaprect.top = atoi(custr);


	cucomma = strchr(&msize[cupos], ',');
	if (cucomma == NULL) return;
	memcpy_s(custr, custr_size, &msize[cupos], (cucomma - msize) - cupos + 1);
	custr[((cucomma - msize)) - cupos] = 0;
	cupos = cucomma - msize + 1;

	m_vismaprect.right = atoi(custr);


	cucomma = strchr(&msize[cupos], ','); // we check again... could be there is a new ini format
	if (cucomma == NULL) cucomma = (char*)((int)msize + strlen(msize));
	memcpy_s(custr, custr_size, &msize[cupos], (cucomma - msize) - cupos + 1);
	custr[((cucomma - msize)) - cupos] = 0;
	cupos = cucomma - msize + 1;

	m_vismaprect.bottom = atoi(custr);


}





WORD CMapData::GetHousesCount(BOOL bCountries)
{
#ifndef RA2_MODE
	bCountries = FALSE;
#endif

	CString sSection = MAPHOUSES;
	if (bCountries) sSection = HOUSES;

	if (m_mapfile.sections.find(sSection) != m_mapfile.sections.end())
		if (m_mapfile.sections[sSection].values.size() > 0) return m_mapfile.sections[sSection].values.size();

	return(rules.sections[HOUSES].values.size());
}

CString CMapData::GetHouseID(WORD wHouse, BOOL bCountry)
{
	if (wHouse >= GetHousesCount())
		return CString("");

#ifndef RA2_MODE
	bCountry = FALSE;
#endif

	CString sSection = MAPHOUSES;
	if (bCountry) sSection = HOUSES;


	if (m_mapfile.sections.find(sSection) != m_mapfile.sections.end())
		if (m_mapfile.sections[sSection].values.size() > 0) return *m_mapfile.sections[sSection].GetValue(wHouse);

	return(*rules.sections[HOUSES].GetValue(wHouse));
}

DWORD CMapData::GetAITriggerTypeCount()
{
	if (m_mapfile.sections.find("AITriggerTypes") != m_mapfile.sections.end())
		return(m_mapfile.sections["AITriggerTypes"].values.size());

	return 0;
}

void CMapData::GetAITriggerType(DWORD dwAITriggerType, AITRIGGERTYPE* pAITrg)
{
	CString data;

	if (dwAITriggerType >= GetAITriggerTypeCount())
		return;

	data = *m_mapfile.sections["AITriggerTypes"].GetValue(dwAITriggerType);

	pAITrg->ID = *m_mapfile.sections["AITriggerTypes"].GetValueName(dwAITriggerType);
	pAITrg->name = GetParam(data, 0);
	pAITrg->teamtype1 = GetParam(data, 1);
	pAITrg->owner = GetParam(data, 2);
	pAITrg->techlevel = GetParam(data, 3);
	pAITrg->type = GetParam(data, 4);
	pAITrg->unittype = GetParam(data, 5);
	pAITrg->data = GetParam(data, 6);
	pAITrg->float1 = GetParam(data, 7);
	pAITrg->float2 = GetParam(data, 8);
	pAITrg->float3 = GetParam(data, 9);
	pAITrg->skirmish = GetParam(data, 10);
	pAITrg->flag4 = GetParam(data, 11);
	pAITrg->multihouse = GetParam(data, 12);
	pAITrg->basedefense = GetParam(data, 13);
	pAITrg->teamtype2 = GetParam(data, 14);
	pAITrg->easy = GetParam(data, 15);
	pAITrg->medium = GetParam(data, 16);
	pAITrg->hard = GetParam(data, 17);


}

WORD CMapData::GetHouseIndex(LPCTSTR lpHouse)
{
	if (m_mapfile.sections.find(HOUSES) != m_mapfile.sections.end())
		if (m_mapfile.sections[HOUSES].values.size() > 0)
			return m_mapfile.sections[HOUSES].FindValue(lpHouse);


	return rules.sections[HOUSES].FindValue(lpHouse);
}

DWORD CMapData::GetAITriggerTypeIndex(LPCTSTR lpID)
{
	if (GetAITriggerTypeCount() == 0) return 0;

	return m_mapfile.sections["AITriggerTypes"].FindName(lpID);
}

CString CMapData::GetAITriggerTypeID(DWORD dwAITriggerType)
{
	if (dwAITriggerType >= GetAITriggerTypeCount()) return CString("");

	return *m_mapfile.sections["AITriggerTypes"].GetValueName(dwAITriggerType);
}

CIniFile& CMapData::GetIniFile()
{
	UpdateIniFile();
	return m_mapfile;
}

/*
UpdateIniFile();

This will update the mapfile member if bSave==TRUE,
else the other data will be updated according to the info in mapfile.

This sections will NOT be updated during loading from ini,
because of perfomance issues. They will not be updated not
during saving to ini if MAPDATA_UPDATE_TO_INI_ALL is not specified:
	-	IsoMapPack5
	-	OverlayPack
	-	OverlayDataPack
If you want to load these sections, the caller needs to call Unpack();
in order to do this.
*/
void CMapData::UpdateIniFile(DWORD dwFlags)
{
	BOOL bSave = TRUE;
	if (dwFlags == MAPDATA_UPDATE_FROM_INI)
		bSave = FALSE;

	if (dwFlags == MAPDATA_UPDATE_FROM_INI)
	{
		CalcMapRect();
		InitMinimap();

		slopesetpiecesset = atoi((*tiles).sections["General"].values["SlopeSetPieces"]);
		rampset = atoi((*tiles).sections["General"].values["RampBase"]);
		rampsmoothset = atoi((*tiles).sections["General"].values["RampSmooth"]);
		cliffset = atoi((*tiles).sections["General"].values["CliffSet"]);
		cliffset_start = GetTileID(cliffset, 0);
		waterset = atoi((*tiles).sections["General"].values["WaterSet"]);
		shoreset = atoi((*tiles).sections["General"].values["ShorePieces"]);
		rampset_start = GetTileID(atoi((*tiles).sections["General"].values["RampBase"]), 0);
		ramp2set = atoi(g_data.sections["NewUrbanInfo"].values["Ramps2"]);
		ramp2set_start = GetTileID(ramp2set, 0);
		pave2set = atoi(g_data.sections["NewUrbanInfo"].values["Morphable2"]);
		pave2set_start = GetTileID(pave2set, 0);
		cliff2set = atoi(g_data.sections["NewUrbanInfo"].values["Cliffs2"]);
		cliff2set_start = GetTileID(cliff2set, 0);
		cliffwater2set = atoi(g_data.sections["NewUrbanInfo"].values["CliffsWater2"]);

		InitializeUnitTypes();
		UpdateBuildingInfo();
		UpdateTreeInfo();
#ifdef SMUDGE_SUPP
		UpdateSmudgeInfo();
#endif
		UpdateMapFieldData(bSave);


	}
	else if (dwFlags & MAPDATA_UPDATE_TO_INI_ALL)
	{
		UpdateAircraft(bSave);
		UpdateCelltags(bSave);
		UpdateInfantry(bSave);
		UpdateNodes(bSave);
		UpdateOverlay(bSave);
		UpdateStructures(bSave);
		UpdateTerrain(bSave);
#ifdef SMUDGE_SUPP
		UpdateSmudges(bSave);
#endif
		UpdateUnits(bSave);
		UpdateWaypoints(bSave);
		UpdateTubes(bSave);
		errstream << "UpdateMapFieldData() called" << endl;
		errstream.flush();
		UpdateMapFieldData(bSave);
		errstream << "About to call Pack()" << endl;
		errstream.flush();
		Pack(dwFlags & MAPDATA_UPDATE_TO_INI_ALL_PREVIEW, dwFlags & MAPDATA_UPDATE_TO_INI_ALL_COMPRESSED);
		return;
	}

	if (!bSave) UpdateAircraft(bSave);
	UpdateCelltags(bSave);
	if (!bSave) UpdateInfantry(bSave);
	UpdateNodes(bSave);
	if (!bSave) UpdateOverlay(bSave);
	if (!bSave) UpdateStructures(bSave);
	if (!bSave) UpdateTerrain(bSave);
#ifdef SMUDGE_SUPP
	if (!bSave) UpdateSmudges(bSave);
#endif
	if (!bSave) UpdateUnits(bSave);
	UpdateWaypoints(bSave);
	UpdateTubes(bSave);

}

void CMapData::LoadMap(const std::string& file)
{
	errstream << "LoadMap() frees memory\n";
	errstream.flush();

	if (fielddata != NULL) delete[] fielddata;
	int i;
	for (i = 0;i < dwSnapShotCount;i++)
	{
		delete[] m_snapshots[i].bHeight;
		delete[] m_snapshots[i].bMapData;
		delete[] m_snapshots[i].bSubTile;
		delete[] m_snapshots[i].bMapData2;
		delete[] m_snapshots[i].wGround;
		delete[] m_snapshots[i].bRedrawTerrain;
		delete[] m_snapshots[i].overlay;
		delete[] m_snapshots[i].overlaydata;
		// m_snapshots[i].mapfile.Clear();
	}
	if (m_snapshots != NULL) delete[] m_snapshots;



	fielddata = NULL;
	fielddata_size = 0;
	m_snapshots = NULL;
	dwSnapShotCount = 0;
	m_cursnapshot = -1;

	m_tubes.clear();
	m_tubes.reserve(32);

	m_infantry.clear();
	m_terrain.clear();
	m_units.clear();
	m_structures.clear();
#ifdef SMUDGE_SUPP
	m_smudges.clear();
#endif

	errstream << "LoadMap() loads map file\n";
	errstream.flush();

	m_mapfile.Clear();
	m_mapfile.LoadFile(file, TRUE);

	// any .mpr is a multi map. Previous FinalAlert/FinalSun versions did not set this value correctly->
	char lowc[MAX_PATH] = { 0 };
	strcpy_s(lowc, file.c_str());
	_strlwr(lowc);
	if (strstr(lowc, ".mpr"))
	{
		m_mapfile.sections["Basic"].values["MultiplayerOnly"] = "1";
		if (m_mapfile.sections["Basic"].FindName("Player") >= 0) m_mapfile.sections["Basic"].values.erase("Player");
	}



	errstream << "LoadMap() repairs Taskforces (if needed)\n";
	errstream.flush();

	// repair taskforces (bug in earlier 0.95 versions)
	for (i = 0;i < m_mapfile.sections["TaskForces"].values.size();i++)
	{
		vector<CString> toDelete;
		toDelete.reserve(5);

		CIniFileSection& sec = m_mapfile.sections[*m_mapfile.sections["TaskForces"].GetValue(i)];
		int e;
		for (e = 0;e < sec.values.size();e++)
		{
			if (sec.GetValue(e)->GetLength() == 0)
			{
				toDelete.push_back(*sec.GetValueName(e));
			}


		}
		for (e = 0;e < toDelete.size();e++)
		{
			sec.values.erase(toDelete[e]);
		}
	}

	errstream << "LoadMap() loads graphics\n";
	errstream.flush();

	CDynamicGraphDlg dlg(theApp.m_pMainWnd);
	dlg.ShowWindow(SW_SHOW);
	dlg.UpdateWindow();

	theApp.m_loading->Unload();
	theApp.m_loading->InitMixFiles();

	map<CString, PICDATA>::iterator it = pics.begin();
	for (int e = 0;e < pics.size();e++)
	{
		try
		{
#ifdef NOSURFACES_OBJECTS			
			if (it->second.bType == PICDATA_TYPE_BMP)
			{
				if (it->second.pic != NULL)
				{
					((LPDIRECTDRAWSURFACE4)it->second.pic)->Release();
				}
			}
			else
			{
				if (it->second.pic != NULL)
				{
					delete[] it->second.pic;
				}
				if (it->second.vborder) delete[] it->second.vborder;
			}
#else
			if (it->second.pic != NULL) it->second.pic->Release();
#endif

			it->second.pic = NULL;
		}
		catch (...)
		{
			CString err;
			err = "Access violation while trying to release surface ";
			char c[6];
			itoa(e, c, 10);
			err += c;

			err += "\n";
			OutputDebugString(err);
			continue;
		}

		it++;
	}

	pics.clear();
	missingimages.clear();

	theApp.m_loading->InitPics();

	UpdateBuildingInfo();
	UpdateTreeInfo();
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->UpdateOverlayPictures();


	if (m_mapfile.sections["Map"].values["Theater"] == THEATER0)
	{
		tiledata = &s_tiledata;
		tiledata_count = &s_tiledata_count;
		tiles = &tiles_s;
		theApp.m_loading->FreeTileSet();
		tiledata = &u_tiledata;
		tiledata_count = &u_tiledata_count;
		tiles = &tiles_u;
		theApp.m_loading->FreeTileSet();

		// MW new tilesets
		tiledata = &un_tiledata;
		tiledata_count = &un_tiledata_count;
		tiles = &tiles_un;
		theApp.m_loading->FreeTileSet();
		tiledata = &d_tiledata;
		tiledata_count = &d_tiledata_count;
		tiles = &tiles_d;
		theApp.m_loading->FreeTileSet();
		tiledata = &l_tiledata;
		tiledata_count = &l_tiledata_count;
		tiles = &tiles_l;
		theApp.m_loading->FreeTileSet();

		tiledata = &t_tiledata;
		tiledata_count = &t_tiledata_count;
		tiles = &tiles_t;
		theApp.m_loading->FreeTileSet();
		theApp.m_loading->InitTMPs(&dlg.m_Progress);
		theApp.m_loading->cur_theat = 'T';

	}
	else if (m_mapfile.sections["Map"].values["Theater"] == THEATER1)
	{
		tiledata = &t_tiledata;
		tiledata_count = &t_tiledata_count;
		tiles = &tiles_t;
		theApp.m_loading->FreeTileSet();
		tiledata = &u_tiledata;
		tiledata_count = &u_tiledata_count;
		tiles = &tiles_u;
		theApp.m_loading->FreeTileSet();

		// MW new tilesets
		tiledata = &un_tiledata;
		tiledata_count = &un_tiledata_count;
		tiles = &tiles_un;
		theApp.m_loading->FreeTileSet();
		tiledata = &d_tiledata;
		tiledata_count = &d_tiledata_count;
		tiles = &tiles_d;
		theApp.m_loading->FreeTileSet();
		tiledata = &l_tiledata;
		tiledata_count = &l_tiledata_count;
		tiles = &tiles_l;
		theApp.m_loading->FreeTileSet();

		tiledata = &s_tiledata;
		tiledata_count = &s_tiledata_count;
		tiles = &tiles_s;
		theApp.m_loading->FreeTileSet();
		theApp.m_loading->InitTMPs(&dlg.m_Progress);
		theApp.m_loading->cur_theat = 'A';
	}
	else if (m_mapfile.sections["Map"].values["Theater"] == THEATER2)
	{
		tiledata = &t_tiledata;
		tiledata_count = &t_tiledata_count;
		tiles = &tiles_t;
		theApp.m_loading->FreeTileSet();
		tiledata = &s_tiledata;
		tiledata_count = &s_tiledata_count;
		tiles = &tiles_s;
		theApp.m_loading->FreeTileSet();

		// MW new tilesets
		tiledata = &un_tiledata;
		tiledata_count = &un_tiledata_count;
		tiles = &tiles_un;
		theApp.m_loading->FreeTileSet();
		tiledata = &d_tiledata;
		tiledata_count = &d_tiledata_count;
		tiles = &tiles_d;
		theApp.m_loading->FreeTileSet();
		tiledata = &l_tiledata;
		tiledata_count = &l_tiledata_count;
		tiles = &tiles_l;
		theApp.m_loading->FreeTileSet();

		tiledata = &u_tiledata;
		tiledata_count = &u_tiledata_count;
		tiles = &tiles_u;
		theApp.m_loading->FreeTileSet();
		theApp.m_loading->InitTMPs(&dlg.m_Progress);
		theApp.m_loading->cur_theat = 'U';
	}
	else if (yuri_mode && m_mapfile.sections["Map"].values["Theater"] == THEATER3)
	{
		tiledata = &t_tiledata;
		tiledata_count = &t_tiledata_count;
		tiles = &tiles_t;
		theApp.m_loading->FreeTileSet();
		tiledata = &s_tiledata;
		tiledata_count = &s_tiledata_count;
		tiles = &tiles_s;
		theApp.m_loading->FreeTileSet();

		// MW new tilesets

		tiledata = &d_tiledata;
		tiledata_count = &d_tiledata_count;
		tiles = &tiles_d;
		theApp.m_loading->FreeTileSet();
		tiledata = &l_tiledata;
		tiledata_count = &l_tiledata_count;
		tiles = &tiles_l;
		theApp.m_loading->FreeTileSet();

		tiledata = &u_tiledata;
		tiledata_count = &u_tiledata_count;
		tiles = &tiles_u;
		theApp.m_loading->FreeTileSet();

		tiledata = &un_tiledata;
		tiledata_count = &un_tiledata_count;
		tiles = &tiles_un;
		theApp.m_loading->FreeTileSet();

		theApp.m_loading->InitTMPs(&dlg.m_Progress);
		theApp.m_loading->cur_theat = 'N';
	}
	else if (yuri_mode && m_mapfile.sections["Map"].values["Theater"] == THEATER4)
	{
		tiledata = &t_tiledata;
		tiledata_count = &t_tiledata_count;
		tiles = &tiles_t;
		theApp.m_loading->FreeTileSet();
		tiledata = &s_tiledata;
		tiledata_count = &s_tiledata_count;
		tiles = &tiles_s;
		theApp.m_loading->FreeTileSet();

		// MW new tilesets
		tiledata = &un_tiledata;
		tiledata_count = &un_tiledata_count;
		tiles = &tiles_un;
		theApp.m_loading->FreeTileSet();
		tiledata = &d_tiledata;
		tiledata_count = &d_tiledata_count;
		tiles = &tiles_d;
		theApp.m_loading->FreeTileSet();


		tiledata = &u_tiledata;
		tiledata_count = &u_tiledata_count;
		tiles = &tiles_u;
		theApp.m_loading->FreeTileSet();

		tiledata = &l_tiledata;
		tiledata_count = &l_tiledata_count;
		tiles = &tiles_l;
		theApp.m_loading->FreeTileSet();

		theApp.m_loading->InitTMPs(&dlg.m_Progress);
		theApp.m_loading->cur_theat = 'L';
	}
	else if (m_mapfile.sections["Map"].values["Theater"] == THEATER5)
	{
		tiledata = &t_tiledata;
		tiledata_count = &t_tiledata_count;
		tiles = &tiles_t;
		theApp.m_loading->FreeTileSet();
		tiledata = &s_tiledata;
		tiledata_count = &s_tiledata_count;
		tiles = &tiles_s;
		theApp.m_loading->FreeTileSet();

		// MW new tilesets
		tiledata = &un_tiledata;
		tiledata_count = &un_tiledata_count;
		tiles = &tiles_un;
		theApp.m_loading->FreeTileSet();

		tiledata = &l_tiledata;
		tiledata_count = &l_tiledata_count;
		tiles = &tiles_l;
		theApp.m_loading->FreeTileSet();


		tiledata = &u_tiledata;
		tiledata_count = &u_tiledata_count;
		tiles = &tiles_u;
		theApp.m_loading->FreeTileSet();

		tiledata = &d_tiledata;
		tiledata_count = &d_tiledata_count;
		tiles = &tiles_d;
		theApp.m_loading->FreeTileSet();

		theApp.m_loading->InitTMPs(&dlg.m_Progress);
		theApp.m_loading->cur_theat = 'D';
	}
	else
	{
		theApp.m_loading->FreeAll();
		CString s = "Fatal error! %9 doesn´t support the theater of this map!";
		s = TranslateStringACP(s);
		MessageBox(0, s, "Error", 0);
		exit(0);
	}
	dlg.DestroyWindow();



	CalcMapRect();

	isInitialized = TRUE;

	errstream << "LoadMap() allocates fielddata\n";
	errstream.flush();

	fielddata = new(FIELDDATA[(GetIsoSize() + 1) * (GetIsoSize() + 1)]);
	fielddata_size = (GetIsoSize() + 1) * (GetIsoSize() + 1);

	errstream << "LoadMap() unpacks data\n";
	errstream.flush();



	Unpack();

	errstream << "LoadMap() updates from ini\n";
	errstream.flush();


	UpdateIniFile(MAPDATA_UPDATE_FROM_INI);

}




void CMapData::Unpack()
{
	if (!isInitialized) return;

	CMapLoadingDlg d;
	d.ShowWindow(SW_SHOW);
	d.UpdateWindow();

	CString ovrl;
	int i;


	ovrl = "";

	for (i = 0;i < m_mapfile.sections["OverlayPack"].values.size();i++)
	{
		ovrl += *m_mapfile.sections["OverlayPack"].GetValue(i);
	}

	//BYTE values[262144];
	const size_t VALUESIZE = 262144;
	std::vector<BYTE> values(VALUESIZE, 0xFF);
	int hexlen;


	if (ovrl.GetLength() > 0)
	{
		std::vector<BYTE> hex;
		hexlen = FSunPackLib::DecodeBase64(ovrl, hex);
		FSunPackLib::DecodeF80(hex.data(), hexlen, values, VALUESIZE);
		values.resize(VALUESIZE, 0xFF);  // fill rest
	}

	memcpy(m_Overlay, values.data(), std::min(VALUESIZE, values.size()));

	ovrl = "";

	for (i = 0;i < m_mapfile.sections["OverlayDataPack"].values.size();i++)
	{
		ovrl += *m_mapfile.sections["OverlayDataPack"].GetValue(i);
	}

	values.assign(VALUESIZE, 0);
	if (ovrl.GetLength() > 0)
	{
		std::vector<BYTE> hex;

		hexlen = FSunPackLib::DecodeBase64(ovrl, hex);
		FSunPackLib::DecodeF80(hex.data(), hexlen, values, VALUESIZE);
		values.resize(VALUESIZE, 0xFF);  // fill rest
	}

	memcpy(m_OverlayData, values.data(), std::min(VALUESIZE, values.size()));


	CString IsoMapPck;
	int len_needed = 0;

	CIniFileSection& sec = m_mapfile.sections["IsoMapPack5"];
	int lines = sec.values.size();

	/*char c[50];
	itoa(m_mapfile.sections["IsoMapPack5"].values.size(), c, 10);
	MessageBox(0,c,"",0);*/

	for (i = 0;i < lines;i++)
	{
		len_needed += sec.GetValue(i)->GetLength();
	}

	LPSTR lpMapPack = new(char[len_needed + 1]);
	memset(lpMapPack, 0, len_needed + 1);

	int cur_pos = 0;
	for (i = 0;i < lines;i++)
	{
		memcpy(lpMapPack + cur_pos, (LPCSTR)*sec.GetValue(i), sec.GetValue(i)->GetLength());
		cur_pos += sec.GetValue(i)->GetLength();
		DoEvents();

		//IsoMapPck+=*sec.GetValue(i);
	}

	IsoMapPck = lpMapPack;

	delete[] lpMapPack;


	if (m_mfd != NULL) delete[] m_mfd;
	m_mfd = NULL;
	dwIsoMapSize = 0;

	if (IsoMapPck.GetLength() > 0)
	{
		std::vector<BYTE> hexC;

		//DoEvents();

		hexlen = FSunPackLib::DecodeBase64(IsoMapPck, hexC);

		// first let´s find out the size of the mappack data
		const auto hex = hexC.data();
		int SP = 0;
		int MapSizeBytes = 0;
		int sec = 0;
		while (SP < hexlen)
		{
			WORD wSrcSize;
			WORD wDestSize;
			memcpy(&wSrcSize, hex + SP, 2);
			SP += 2;
			memcpy(&wDestSize, hex + SP, 2);
			SP += 2;

			MapSizeBytes += wDestSize;
			SP += wSrcSize;

			sec++;
		}

		m_mfd = new(BYTE[MapSizeBytes]);
		dwIsoMapSize = MapSizeBytes / MAPFIELDDATA_SIZE;

		FSunPackLib::DecodeIsoMapPack5(hex, hexlen, (BYTE*)m_mfd, d.m_Progress.m_hWnd, TRUE);

		int k;
		/*fstream f;
		f.open("C:\\isomappack5.txt",ios_base::in | ios_base::out | ios_base::trunc);
		for(k=0;k<150;k++)
		{
			f << "Byte " << k << ":	" << (int)m_mfd[k] << endl;
		}
		f.flush();*/
	}

	d.DestroyWindow();

}






void CMapData::Pack(BOOL bCreatePreview, BOOL bCompression)
{
	if (!isInitialized) return;

	errstream << "Erasing sections" << endl;
	errstream.flush();

	int i;
	BYTE* base64 = NULL; // must be freed!

	m_mapfile.sections.erase("OverlayPack");
	m_mapfile.sections.erase("OverlayDataPack");
	m_mapfile.sections.erase("IsoMapPack5"); // only activate when packing isomappack is supported

	DWORD pos;

	errstream << "Creating Digest" << endl;
	errstream.flush();

	if (m_mapfile.sections["Digest"].values.size() == 0)
	{
		srand(GetTickCount());
		unsigned short vals[10];
		for (i = 0;i < 10;i++)
			vals[i] = rand() * 65536 / RAND_MAX;

		base64 = FSunPackLib::EncodeBase64((BYTE*)vals, 20);

		i = 0;
		pos = 0;
		while (TRUE)
		{
			i++;
			char cLine[50];
			itoa(i, cLine, 10);
			char str[200];
			memset(str, 0, 200);
			WORD cpysize = 70;
			if (pos + cpysize > strlen((char*)base64)) cpysize = strlen((char*)base64) - pos;
			memcpy(str, &base64[pos], cpysize);
			if (strlen(str) > 0)
				m_mapfile.sections["Digest"].values[cLine] = str;
			if (cpysize < 70) break;
			pos += 70;
		}

		delete[] base64;
		base64 = NULL;
	}


	BYTE* values = new(BYTE[262144]);
	BYTE* hexpacked = NULL; // must be freed!


	errstream << "Values allocated. Pointer: " << (int)values << endl;
	errstream.flush();


	errstream << "Packing overlay" << endl;
	errstream.flush();

	for (i = 0;i < 262144;i++)
	{
		values[i] = m_Overlay[i];
	}

	int hexpackedLen = FSunPackLib::EncodeF80(values, 262144, 32, &hexpacked);
	base64 = FSunPackLib::EncodeBase64(hexpacked, hexpackedLen);


	errstream << "Saving overlay" << endl;

	i = 0;
	pos = 0;
	while (TRUE)
	{
		i++;
		char cLine[50];
		itoa(i, cLine, 10);
		char str[200];
		memset(str, 0, 200);
		WORD cpysize = 70;
		if (pos + cpysize > strlen((char*)base64)) cpysize = strlen((char*)base64) - pos;
		memcpy(str, &base64[pos], cpysize);
		if (strlen(str) > 0)
			m_mapfile.sections["OverlayPack"].values[cLine] = str;
		if (cpysize < 70) break;
		pos += 70;
	}



#ifndef _DEBUG__
	delete[] hexpacked;
	delete[] base64;
#endif

	errstream << "Pack overlaydata" << endl;
	errstream.flush();

	for (i = 0;i < 262144;i++)
	{
		values[i] = m_OverlayData[i];
	}

	hexpacked = NULL;

	errstream << "Format80" << endl;
	errstream.flush();

	hexpackedLen = FSunPackLib::EncodeF80(values, 262144, 32, &hexpacked);


	errstream << "Base64" << endl;
	errstream.flush();

	base64 = FSunPackLib::EncodeBase64(hexpacked, hexpackedLen);


	errstream << "Overlaydata done" << endl;
	errstream.flush();

	i = 0;
	pos = 0;
	while (TRUE)
	{
		i++;
		char cLine[50];
		itoa(i, cLine, 10);
		char str[200];
		memset(str, 0, 200);
		WORD cpysize = 70;
		if (pos + cpysize > strlen((char*)base64)) cpysize = strlen((char*)base64) - pos;
		memcpy(str, &base64[pos], cpysize);
		if (strlen(str) > 0)
			m_mapfile.sections["OverlayDataPack"].values[cLine] = str;
		if (cpysize < 70) break;
		pos += 70;
	}


#ifndef _DEBUG__
	delete[] hexpacked;
	delete[] base64;
#endif

	hexpacked = NULL;

	errstream << "Pack isomappack" << endl;
	errstream.flush();


	hexpackedLen = FSunPackLib::EncodeIsoMapPack5(m_mfd, dwIsoMapSize * MAPFIELDDATA_SIZE, &hexpacked);


	errstream << "done" << endl;
	errstream.flush();

	errstream << "hexdata size: " << hexpackedLen;
	errstream << endl << "Now converting to base64";
	errstream.flush();
	base64 = FSunPackLib::EncodeBase64(hexpacked, hexpackedLen);
	errstream << "done" << endl;
	errstream.flush();

	i = 0;
	pos = 0;
	int base64len = strlen((char*)base64);
	while (TRUE)
	{
		i++;
		char cLine[50];
		itoa(i, cLine, 10);
		char str[200];
		memset(str, 0, 200);
		int cpysize = 70;
		if (pos + cpysize > base64len) cpysize = base64len - pos;
		memcpy(str, &base64[pos], cpysize);
		if (cpysize)
			m_mapfile.sections["IsoMapPack5"].values[cLine] = str;

		if (cpysize < 70) break;
		pos += 70;
	}

	errstream << "finished copying into inifile" << endl;
	errstream.flush();


#ifndef _DEBUG__
	delete[] hexpacked;
	delete[] base64;
#endif


	// create minimap
	if (bCreatePreview)
	{
		BITMAPINFO biinfo;
		BYTE* lpDibData;
		int pitch;
		((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_minimap.DrawMinimap(&lpDibData, biinfo, pitch);

		m_mapfile.sections.erase("PreviewPack");
		m_mapfile.sections["Preview"].values["Size"] = m_mapfile.sections["Map"].values["Size"];
		char c[50];
		itoa(biinfo.bmiHeader.biWidth, c, 10);
		m_mapfile.sections["Preview"].values["Size"] = SetParam(m_mapfile.sections["Preview"].values["Size"], 2, c);
		itoa(biinfo.bmiHeader.biHeight, c, 10);
		m_mapfile.sections["Preview"].values["Size"] = SetParam(m_mapfile.sections["Preview"].values["Size"], 3, c);

		BYTE* lpRAW = new(BYTE[biinfo.bmiHeader.biWidth * biinfo.bmiHeader.biHeight * 3]);

		int mapwidth = GetWidth();
		int mapheight = GetHeight();
		int e;
		for (i = 0;i < biinfo.bmiHeader.biWidth;i++)
		{
			for (e = 0;e < biinfo.bmiHeader.biHeight;e++)
			{
				int dest = i * 3 + e * biinfo.bmiHeader.biWidth * 3;
				int src = i * 3 + (biinfo.bmiHeader.biHeight - e - 1) * pitch;
				memcpy(&lpRAW[dest + 2], &lpDibData[src + 0], 1);
				memcpy(&lpRAW[dest + 1], &lpDibData[src + 1], 1);
				memcpy(&lpRAW[dest + 0], &lpDibData[src + 2], 1);
			}
		}


		hexpacked = NULL;
		hexpackedLen = FSunPackLib::EncodeIsoMapPack5(lpRAW, biinfo.bmiHeader.biWidth * biinfo.bmiHeader.biHeight * 3, &hexpacked);

		base64 = FSunPackLib::EncodeBase64(hexpacked, hexpackedLen);

		// uses IsoMapPack5 encoding routine


		errstream << "Saving minimap" << endl;

		i = 0;
		pos = 0;
		while (TRUE)
		{
			i++;
			char cLine[50];
			itoa(i, cLine, 10);
			char str[200];
			memset(str, 0, 200);
			WORD cpysize = 70;
			if (pos + cpysize > strlen((char*)base64)) cpysize = strlen((char*)base64) - pos;
			memcpy(str, &base64[pos], cpysize);
			if (strlen(str) > 0)
				m_mapfile.sections["PreviewPack"].values[cLine] = str;
			if (cpysize < 70) break;
			pos += 70;
		}



		//#ifndef _DEBUG__
		delete[] base64;
		delete[] hexpacked;
		//#endif


		delete[] lpRAW;
		//delete[] lpDibData;
	}

	delete[] values;
}

void CMapData::ClearOverlayData()
{
	memset(m_OverlayData, 0x0, 262144);
	// Pack();
}

void CMapData::ClearOverlay()
{
	memset(m_Overlay, 0xFF, 262144);
	// Pack();
}

void CMapData::SetOverlayAt(DWORD dwPos, BYTE bValue)
{
	int y = dwPos / m_IsoSize;
	int x = dwPos % m_IsoSize;

	if (y + x * 512 > 262144 || dwPos > m_IsoSize * m_IsoSize) return;

	BYTE& ovrl = m_Overlay[y + x * 512];
	BYTE& ovrld = m_OverlayData[y + x * 512];


	RemoveOvrlMoney(ovrl, ovrld);


	m_Overlay[y + x * 512] = bValue;
	m_OverlayData[y + x * 512] = 0;
	fielddata[dwPos].overlay = bValue;
	fielddata[dwPos].overlaydata = 0;

	BYTE& ovrl2 = m_Overlay[y + x * 512];
	BYTE& ovrld2 = m_OverlayData[y + x * 512];
	AddOvrlMoney(ovrl2, ovrld2);

	int i, e;
	for (i = -1;i < 2;i++)
		for (e = -1;e < 2;e++)
			if (i + x > 0 && i + x < m_IsoSize && y + e >= 0 && y + e < m_IsoSize)
				SmoothTiberium(dwPos + i + e * m_IsoSize);







	Mini_UpdatePos(x, y, IsMultiplayer());

}

BYTE CMapData::GetOverlayAt(DWORD dwPos)
{
	if (dwPos > fielddata_size) return 0;
	return fielddata[dwPos].overlay;
}

void CMapData::SetOverlayDataAt(DWORD dwPos, BYTE bValue)
{

	int y = dwPos / m_IsoSize;
	int x = dwPos % m_IsoSize;

	if (y + x * 512 > 262144 || dwPos > m_IsoSize * m_IsoSize) return;

	BYTE& ovrl = m_Overlay[y + x * 512];
	BYTE& ovrld = m_OverlayData[y + x * 512];

	if (ovrl >= RIPARIUS_BEGIN && ovrl <= RIPARIUS_END)
	{
		//m_money-=(ovrld+1)*(atoi(rules.sections["Riparius"].values["Value"]));
		return;
	}

	if (ovrl >= CRUENTUS_BEGIN && ovrl <= CRUENTUS_END)
	{
		//m_money-=(ovrld+1)*(atoi(rules.sections["Cruentus"].values["Value"]));
		return;
	}

	if (ovrl >= VINIFERA_BEGIN && ovrl <= VINIFERA_END)
	{
		//m_money-=(ovrld+1)*(atoi(rules.sections["Vinifera"].values["Value"]));
		return;
	}

	if (ovrl >= ABOREUS_BEGIN && ovrl <= ABOREUS_END)
	{
		//m_money-=(ovrld+1)*(atoi(rules.sections["Aboreus"].values["Value"]));	
		return;
	}

	m_OverlayData[y + x * 512] = bValue;
	fielddata[dwPos].overlaydata = bValue;

	BYTE& ovrl2 = m_Overlay[y + x * 512];
	BYTE& ovrld2 = m_OverlayData[y + x * 512];


	/*if(ovrl2>=RIPARIUS_BEGIN && ovrl2<=RIPARIUS_END)
	{
		m_money+=(ovrld2+1)*(atoi(rules.sections["Riparius"].values["Value"]));
	}

	if(ovrl2>=CRUENTUS_BEGIN && ovrl2<=CRUENTUS_END)
	{
		m_money+=(ovrld2+1)*(atoi(rules.sections["Cruentus"].values["Value"]));
	}

	if(ovrl2>=VINIFERA_BEGIN && ovrl2<=VINIFERA_END)
	{
		m_money+=(ovrld2+1)*(atoi(rules.sections["Vinifera"].values["Value"]));
	}

	if(ovrl2>=ABOREUS_BEGIN && ovrl2<=ABOREUS_END)
	{
		m_money+=(ovrld2+1)*(atoi(rules.sections["Aboreus"].values["Value"]));
	}*/


}

BYTE CMapData::GetOverlayDataAt(DWORD dwPos)
{
	return fielddata[dwPos].overlaydata;
}

/*
UpdateInfantry();

Updates the tiledata if bSave==FALSE (standard) according to the mapfile.
If bSave==TRUE, nothing is done at the moment (would be for updating the mapfile, but not needed yet,
	because the mapfile is modified directly)
*/
void CMapData::UpdateInfantry(BOOL bSave)
{
	vector<INFANTRY>& iv = m_infantry;

	if (bSave == FALSE)
	{
		iv.clear();
		iv.reserve(100);

		int i;
		for (i = 0;i < GetIsoSize() * GetIsoSize();i++)
		{
			int e;
			for (e = 0;e < SUBPOS_COUNT;e++)
				fielddata[i].infantry[e] = -1;
		}


		if (m_mapfile.sections.find("Infantry") != m_mapfile.sections.end())
		{
			CIniFileSection& sec = m_mapfile.sections["Infantry"];

			for (i = 0;i < sec.values.size();i++)
			{
				CString data = *sec.GetValue(i);

				int x = atoi(GetParam(data, 4));
				int y = atoi(GetParam(data, 3));
				int sp = atoi(GetParam(data, 5));
				int pos = x + y * GetIsoSize();

				INFANTRY id;
				id.deleted = 0;
				id.house = GetParam(data, 0);
				id.type = GetParam(data, 1);
				id.strength = GetParam(data, 2);
				id.y = GetParam(data, 3);
				id.x = GetParam(data, 4);
				id.pos = GetParam(data, 5);
				id.action = GetParam(data, 6);
				id.direction = GetParam(data, 7);
				id.tag = GetParam(data, 8);
				id.flag1 = GetParam(data, 9);
				id.flag2 = GetParam(data, 10);
				id.flag3 = GetParam(data, 11);
				id.flag4 = GetParam(data, 12);
				id.flag5 = GetParam(data, 13);

				iv.push_back(id);

				int spp = sp - 1;
				if (spp < 0) spp = 0;

				if (spp < SUBPOS_COUNT)
					if (pos < fielddata_size)
						fielddata[pos].infantry[spp] = iv.size() - 1;

				Mini_UpdatePos(x, y, IsMultiplayer());



				/*else
				{
					int e;
					for(e=0;e<SUBPOS_COUNT;e++)
					{
						if(fielddata[pos].infantry[e]<0)
						{
							fielddata[pos].infantry[e]=i;
							break;
						}
					}
				}*/
			}
		}
	}
	else
	{
		m_mapfile.sections.erase("Infantry");
		int i;

		int count = 0;
		for (i = 0;i < iv.size();i++)
		{
			INFANTRY& id = iv[i];
			if (!id.deleted)
			{
				INFANTRY& infantry = id;

				CString value;
				value = infantry.house + "," + infantry.type + "," + infantry.strength + "," + infantry.y +
					"," + infantry.x + "," + infantry.pos + "," + infantry.action + "," + infantry.direction + "," +
					infantry.tag + "," + infantry.flag1 + "," + infantry.flag2 + "," + infantry.flag3 + "," +
					infantry.flag4 + "," + infantry.flag5;

				char c[50];
				itoa(count, c, 10);

				m_mapfile.sections["Infantry"].values[c] = value;

				count++;
			}
		}

	}
}

void CMapData::UpdateAircraft(BOOL bSave)
{
	if (bSave == FALSE)
	{
		int i;
		for (i = 0;i < GetIsoSize() * GetIsoSize();i++)
		{
			fielddata[i].aircraft = -1;
		}


		if (m_mapfile.sections.find("Aircraft") != m_mapfile.sections.end())
		{
			CIniFileSection& sec = m_mapfile.sections["Aircraft"];

			for (i = 0;i < sec.values.size();i++)
			{
				int x = atoi(GetParam(*sec.GetValue(i), 4));
				int y = atoi(GetParam(*sec.GetValue(i), 3));
				int pos = x + y * GetIsoSize();
				if (pos < fielddata_size) fielddata[pos].aircraft = i;

				Mini_UpdatePos(x, y, IsMultiplayer());
			}
		}
	}
}

void CMapData::UpdateStructures(BOOL bSave)
{
	if (bSave == FALSE)
	{
		int i;
		for (i = 0;i < GetIsoSize() * GetIsoSize();i++)
		{
			fielddata[i].structure = -1;
			fielddata[i].structuretype = -1;
		}

		m_structurepaint.clear();

		if (m_mapfile.sections.find("Structures") != m_mapfile.sections.end())
		{
			CIniFileSection& sec = m_mapfile.sections["Structures"];

			for (i = 0;i < sec.values.size();i++)
			{
				STRUCTUREPAINT sp;
				sp.col = ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->GetColor(GetParam(*sec.GetValue(i), 0));
				sp.strength = atoi(GetParam(*sec.GetValue(i), 2));
				sp.upgrade1 = GetParam(*sec.GetValue(i), 12);
				sp.upgrade2 = GetParam(*sec.GetValue(i), 13);
				sp.upgrade3 = GetParam(*sec.GetValue(i), 14);
				sp.upradecount = atoi(GetParam(*sec.GetValue(i), 10));
				sp.x = atoi(GetParam(*sec.GetValue(i), 4));
				sp.y = atoi(GetParam(*sec.GetValue(i), 3));
				sp.direction = atoi(GetParam(*sec.GetValue(i), 5));
				sp.type = GetParam(*sec.GetValue(i), 1);

				TruncSpace(sp.upgrade1);
				TruncSpace(sp.upgrade2);
				TruncSpace(sp.upgrade3);

				m_structurepaint.push_back(sp);

				int x = atoi(GetParam(*sec.GetValue(i), 4));
				int y = atoi(GetParam(*sec.GetValue(i), 3));
				int d, e;
				int bid = buildingid[GetParam(*sec.GetValue(i), 1)];
				for (d = 0;d < buildinginfo[bid].h;d++)
				{
					for (e = 0;e < buildinginfo[bid].w;e++)
					{
						int pos = (x + d) + (y + e) * GetIsoSize();
						if (pos < fielddata_size)
						{
							fielddata[pos].structure = i;
							fielddata[pos].structuretype = bid;
						}

						Mini_UpdatePos(x + d, y + e, IsMultiplayer());
					}
				}


			}
		}
	}
}

void CMapData::UpdateTerrain(BOOL bSave, int num)
{
	vector<TERRAIN>& t = m_terrain;

	if (bSave == FALSE)
	{
		if (m_mapfile.sections.find("Terrain") == m_mapfile.sections.end() || m_mapfile.sections["Terrain"].values.size() <= 0)
			return;

		if (num < 0)
		{
			t.clear();
			t.reserve(100);

			int i;
			for (i = 0;i < GetIsoSize() * GetIsoSize();i++)
			{
				fielddata[i].terrain = -1;
			}



			CIniFileSection& sec = m_mapfile.sections["Terrain"];

			for (i = 0;i < sec.values.size();i++)
			{
				int x, y;
				PosToXY(*sec.GetValueName(i), &x, &y);

				// check for valid coordinates ; MW May 17th 2001
				ASSERT(x >= 0 && x < GetIsoSize());
				ASSERT(y >= 0 && y < GetIsoSize());
				if (x < 0 || x >= GetIsoSize() || y < 0 || y >= GetIsoSize())
				{
					// invalid coordinates - ignore in release
				}
				else
				{
					TERRAIN td;
					td.deleted = 0;
					td.type = *sec.GetValue(i);
					td.x = x;
					td.y = y;

					t.push_back(td);

					int pos = x + y * GetIsoSize();
					fielddata[pos].terrain = i;
					fielddata[pos].terraintype = terrainid[td.type];
				}
			}

			m_mapfile.sections.erase("Terrain");

		}

	}
	else
	{

		//if(num<0)
		{
			//if(m_mapfile.sections.find("Terrain")!=m_mapfile.sections.end()) MessageBox(0,"Reupdate!","",0);
			m_mapfile.sections.erase("Terrain");
			int i;

			for (i = 0;i < t.size();i++)
			{
				TERRAIN& td = t[i];
				if (!td.deleted)
				{
					char j1[15];
					char k1[15];

					int x, y;
					x = td.x;
					y = td.y;

					itoa(y, j1, 10);
					if (strlen(j1) < 3)
					{
						strcpy_safe(j1 + 1, j1);
						j1[0] = '0';
					}
					if (strlen(j1) < 3)
					{
						strcpy_safe(j1 + 1, j1);
						j1[0] = '0';
					}
					itoa(x, k1, 10);
					strcat(k1, j1);


					m_mapfile.sections["Terrain"].values[k1] = td.type;
				}
			}
		}

	}
}

void CMapData::UpdateUnits(BOOL bSave)
{
	if (bSave == FALSE)
	{
		int i;
		for (i = 0;i < GetIsoSize() * GetIsoSize();i++)
		{
			fielddata[i].unit = -1;
		}


		if (m_mapfile.sections.find("Units") != m_mapfile.sections.end())
		{
			CIniFileSection& sec = m_mapfile.sections["Units"];

			for (i = 0;i < sec.values.size();i++)
			{
				int x = atoi(GetParam(*sec.GetValue(i), 4));
				int y = atoi(GetParam(*sec.GetValue(i), 3));
				int pos = x + y * GetIsoSize();
				if (pos < fielddata_size) fielddata[pos].unit = i;

				Mini_UpdatePos(x, y, IsMultiplayer());
			}
		}
	}

}

void CMapData::UpdateWaypoints(BOOL bSave)
{
	if (bSave == FALSE)
	{
		int i;
		for (i = 0;i < GetIsoSize() * GetIsoSize();i++)
		{
			fielddata[i].waypoint = -1;
		}


		if (m_mapfile.sections.find("Waypoints") != m_mapfile.sections.end())
		{
			CIniFileSection& sec = m_mapfile.sections["Waypoints"];

			for (i = 0;i < sec.values.size();i++)
			{
				int x, y;
				PosToXY(*sec.GetValue(i), &x, &y);

				int pos = x + y * GetIsoSize();
				if (pos < 0 || pos >= fielddata_size) continue;
				fielddata[pos].waypoint = i;

				int k, l;
				for (k = -1;k < 2;k++)
					for (l = -1;l < 2;l++)
						Mini_UpdatePos(x + k, y + l, IsMultiplayer());
			}
		}
	}

}

void CMapData::UpdateNodes(BOOL bSave)
{
	if (bSave == FALSE)
	{
		int i;

		for (i = 0;i < GetIsoSize() * GetIsoSize();i++)
		{
			fielddata[i].node.index = -1;
			fielddata[i].node.type = -1;
			fielddata[i].node.house = "";
		}

		if (m_mapfile.sections.find(MAPHOUSES) != m_mapfile.sections.end())
		{

			for (i = 0;i < m_mapfile.sections[MAPHOUSES].values.size();i++)
			{
				CIniFileSection& sec = m_mapfile.sections[*m_mapfile.sections[MAPHOUSES].GetValue(i)];
				int c = atoi(sec.values["NodeCount"]);

				int e;
				for (e = 0;e < c;e++)
				{
					CString p;
					GetNodeName(p, e);

					CString type, sx, sy;
					type = GetParam(sec.values[p], 0);
					sy = GetParam(sec.values[p], 1);
					sx = GetParam(sec.values[p], 2);

					int x = atoi(sx);
					int y = atoi(sy);
					int bid = buildingid[type];
					int d, f;
					for (d = 0;d < buildinginfo[bid].h;d++)
					{
						for (f = 0;f < buildinginfo[bid].w;f++)
						{
							int pos = x + d + (y + f) * GetIsoSize();
							fielddata[pos].node.type = buildingid[type];
							fielddata[pos].node.house = *m_mapfile.sections[MAPHOUSES].GetValue(i);
							fielddata[pos].node.index = e;
						}
					}
				}
			}
		}
	}

}

void CMapData::UpdateOverlay(BOOL bSave)
{
	if (bSave == FALSE)
	{
		int u, v;
		const bool mp = IsMultiplayer();
		for (u = 0;u < GetIsoSize();u++)
		{
			for (v = 0;v < GetIsoSize();v++)
			{
				fielddata[u + v * GetIsoSize()].overlay = m_Overlay[v + u * 512];
				fielddata[u + v * GetIsoSize()].overlaydata = m_OverlayData[v + u * 512];

				Mini_UpdatePos(u, v, mp);

			}
		}
	}
	else
	{
		int u, v;
		for (u = 0;u < GetIsoSize();u++)
		{
			for (v = 0;v < GetIsoSize();v++)
			{
				m_Overlay[v + u * 512] = fielddata[u + v * GetIsoSize()].overlay;
				m_OverlayData[v + u * 512] = fielddata[u + v * GetIsoSize()].overlaydata;
			}
		}
	}

	m_money = CalcMoneyOnMap();

}


void CMapData::UpdateCelltags(BOOL bSave)
{
	if (bSave == FALSE)
	{
		int i;
		for (i = 0;i < GetIsoSize() * GetIsoSize();i++)
		{
			fielddata[i].celltag = -1;
		}


		if (m_mapfile.sections.find("CellTags") != m_mapfile.sections.end())
		{
			CIniFileSection& sec = m_mapfile.sections["CellTags"];

			for (i = 0;i < sec.values.size();i++)
			{
				int x, y;
				PosToXY(*sec.GetValueName(i), &x, &y);

				int pos = x + y * GetIsoSize();
				if (pos < fielddata_size) fielddata[pos].celltag = i;
			}
		}
	}
}


INT CMapData::GetNodeAt(DWORD dwPos, CString& lpHouse) const
{
	lpHouse = fielddata[dwPos].node.house;
	return fielddata[dwPos].node.index;
}



void CMapData::DeleteInfantry(DWORD dwIndex)
{
	//if(dwIndex>=m_infantry.size()) MessageBox(0,"CMapData::DeleteInfantry(): Out of range error", "Error", 0);

	if (dwIndex >= m_infantry.size()) return;

	// BUG TRACING HERE, FOR THE COPY INSTEAD MOVE INFANTRY BUG!
	// SOLUTION WAS IN ADDINFANTRY();
	if (m_infantry[dwIndex].deleted)
	{
		//MessageBox(0,"CMapData::DeleteInfantry() called for infantry that already got deleted!", "Error",0);
		errstream << "CMapData::DeleteInfantry() called for infantry that already got deleted! Index: " << dwIndex << endl;
		errstream.flush();
	}

	if (m_infantry[dwIndex].deleted) return;



	m_infantry[dwIndex].deleted = 1;

	int x = atoi(m_infantry[dwIndex].x);
	int y = atoi(m_infantry[dwIndex].y);
	int pos = atoi(m_infantry[dwIndex].pos);

	if (pos > 0) pos--;

	if (x + y * m_IsoSize < fielddata_size) fielddata[x + y * m_IsoSize].infantry[pos] = -1;

	Mini_UpdatePos(x, y, IsMultiplayer());

	/*if(dwIndex>=m_mapfile.sections["Infantry"].values.size()) return;

	CIniFileSection& sec=m_mapfile.sections["Infantry"];
	int x=atoi(GetParam(*sec.GetValue(dwIndex),4));
	int y=atoi(GetParam(*sec.GetValue(dwIndex),3));


	m_mapfile.sections["Infantry"].values.erase(*m_mapfile.sections["Infantry"].GetValueName(dwIndex));
	UpdateInfantry(FALSE);*/


}

void CMapData::DeleteWaypoint(DWORD dwIndex)
{
	if (dwIndex >= GetWaypointCount()) return;

	CString id;
	DWORD pos;
	Map->GetWaypointData(dwIndex, &id, &pos);

	int x = pos % m_IsoSize;
	int y = pos / m_IsoSize;



	m_mapfile.sections["Waypoints"].values.erase(*m_mapfile.sections["Waypoints"].GetValueName(dwIndex));
	if (!m_noAutoObjectUpdate) UpdateWaypoints(FALSE);

	int k, l;
	for (k = -1;k < 2;k++)
		for (l = -1;l < 2;l++)
			Mini_UpdatePos(x + k, y + l, IsMultiplayer());
}

void CMapData::DeleteCelltag(DWORD dwIndex)
{
	if (dwIndex >= GetCelltagCount()) return;

	m_mapfile.sections["CellTags"].values.erase(*m_mapfile.sections["CellTags"].GetValueName(dwIndex));
	if (!m_noAutoObjectUpdate) UpdateCelltags(FALSE);
}

void CMapData::DeleteUnit(DWORD dwIndex)
{
	if (dwIndex >= GetUnitCount()) return;

	CIniFileSection& sec = m_mapfile.sections["Units"];
	int x = atoi(GetParam(*sec.GetValue(dwIndex), 4));
	int y = atoi(GetParam(*sec.GetValue(dwIndex), 3));


	m_mapfile.sections["Units"].values.erase(*m_mapfile.sections["Units"].GetValueName(dwIndex));
	if (!m_noAutoObjectUpdate) UpdateUnits(FALSE);

	Mini_UpdatePos(x, y, IsMultiplayer());
}

void CMapData::DeleteStructure(DWORD dwIndex)
{
	if (dwIndex >= GetStructureCount()) return;

	CIniFileSection& sec = m_mapfile.sections["Structures"];
	int x = atoi(GetParam(*sec.GetValue(dwIndex), 4));
	int y = atoi(GetParam(*sec.GetValue(dwIndex), 3));
	CString type = GetParam(*sec.GetValue(dwIndex), 1);

	m_mapfile.sections["Structures"].values.erase(*m_mapfile.sections["Structures"].GetValueName(dwIndex));
	if (!m_noAutoObjectUpdate) UpdateStructures(FALSE);

	int d, e;
	int bid = buildingid[type];
	for (d = 0;d < buildinginfo[bid].h;d++)
	{
		for (e = 0;e < buildinginfo[bid].w;e++)
		{
			int pos = (x + d) + (y + e) * GetIsoSize();

			Mini_UpdatePos(x + d, y + e, IsMultiplayer());
		}
	}
}

void CMapData::DeleteAircraft(DWORD dwIndex)
{
	if (dwIndex >= GetAircraftCount()) return;

	CIniFileSection& sec = m_mapfile.sections["Aircraft"];
	int x = atoi(GetParam(*sec.GetValue(dwIndex), 4));
	int y = atoi(GetParam(*sec.GetValue(dwIndex), 3));


	m_mapfile.sections["Aircraft"].values.erase(*m_mapfile.sections["Aircraft"].GetValueName(dwIndex));
	if (!m_noAutoObjectUpdate) UpdateAircraft(FALSE);

	Mini_UpdatePos(x, y, IsMultiplayer());
}

void CMapData::DeleteTerrain(DWORD dwIndex)
{
	/* March 23th 2001: MW: Rewrote code, much faster */
	if (dwIndex >= GetTerrainCount()) return;
	if (m_terrain[dwIndex].deleted) return;

	//UpdateTerrain(FALSE);
	// replacement of slow UpdateTerrain():

	//CString valuename=*m_mapfile.sections["Terrain"].GetValueName(dwIndex);

	int x, y;
	//PosToXY(valuename, &x, &y);
	//m_mapfile.sections["Terrain"].values.erase(valuename);	
	x = m_terrain[dwIndex].x;
	y = m_terrain[dwIndex].y;
	m_terrain[dwIndex].deleted = 1;

	int pos = x + y * GetIsoSize();
	if (x + y * m_IsoSize < fielddata_size)
	{
		fielddata[pos].terrain = -1;
		fielddata[pos].terraintype = -1;
	}

}

void CMapData::DeleteNode(LPCTSTR lpHouse, DWORD dwIndex)
{
	CString p; // p is last node
	GetNodeName(p, atoi(m_mapfile.sections[lpHouse].values["NodeCount"]) - 1);

	char c[50];
	itoa(atoi(m_mapfile.sections[lpHouse].values["NodeCount"]) - 1, c, 10);

	int i;
	for (i = dwIndex;i < atoi(m_mapfile.sections[lpHouse].values["NodeCount"]) - 1;i++)
	{
		CString d1, d2;
		GetNodeName(d1, i);
		GetNodeName(d2, i + 1);
		m_mapfile.sections[lpHouse].values[d1] = m_mapfile.sections[lpHouse].values[d2];
	}

	m_mapfile.sections[lpHouse].values.erase(p);
	m_mapfile.sections[lpHouse].values["NodeCount"] = c;

	UpdateNodes(FALSE);
}

BOOL CMapData::AddWaypoint(CString lpID, DWORD dwPos)
{
	// create waypoint, auto number
	CString id = lpID;


	if (lpID.GetLength() == 0) id = GetFree("Waypoints");


	char j[15];
	char k[15];
	memset(j, 0, 15);
	memset(k, 0, 15);
	itoa(dwPos / GetIsoSize(), j, 10);
	if (strlen(j) < 3)
	{
		strcpy_safe(j + 1, j);
		j[0] = '0';
	}
	if (strlen(j) < 3)
	{
		strcpy_safe(j + 1, j);
		j[0] = '0';
	}
	itoa(dwPos % GetIsoSize(), k, 10);
	strcat(k, j);

	//MessageBox(0,k,"",0);


	m_mapfile.sections["Waypoints"].values[id] = k;
	if (!m_noAutoObjectUpdate) UpdateWaypoints(FALSE);

	return TRUE;
}



void CMapData::GetStructureData(DWORD dwIndex, STRUCTURE* lpStructure) const
{	
	const auto section = m_mapfile.GetSection("Structures");
	if (!section || dwIndex >= section->values.size())
		return;

	CString data = *section->GetValue(dwIndex);

	lpStructure->house = GetParam(data, 0);
	lpStructure->type = GetParam(data, 1);
	lpStructure->strength = GetParam(data, 2);
	lpStructure->y = GetParam(data, 3);
	lpStructure->x = GetParam(data, 4);
	lpStructure->direction = GetParam(data, 5);
	lpStructure->tag = GetParam(data, 6);
	lpStructure->flag1 = GetParam(data, 7);
	lpStructure->flag2 = GetParam(data, 8);
	lpStructure->energy = GetParam(data, 9);
	lpStructure->upgradecount = GetParam(data, 10);
	lpStructure->spotlight = GetParam(data, 11);
	lpStructure->upgrade1 = GetParam(data, 12);
	lpStructure->upgrade2 = GetParam(data, 13);
	lpStructure->upgrade3 = GetParam(data, 14);
	lpStructure->flag3 = GetParam(data, 15);
	lpStructure->flag4 = GetParam(data, 16);

}

void CMapData::GetStdStructureData(DWORD dwIndex, STDOBJECTDATA* lpStdStructure) const
{
	const auto section = m_mapfile.GetSection("Structures");
	if (!section || dwIndex >= section->values.size())
		return;

	CString data = *section->GetValue(dwIndex);

	lpStdStructure->house = GetParam(data, 0);
	lpStdStructure->type = GetParam(data, 1);
	lpStdStructure->strength = GetParam(data, 2);
	lpStdStructure->y = GetParam(data, 3);
	lpStdStructure->x = GetParam(data, 4);
}

BOOL CMapData::AddNode(NODE* lpNode, WORD dwPos)
{
	NODE node;
	if (lpNode != NULL)
	{
		node = *lpNode;
	}
	else
	{
		node.x.Format("%d", dwPos % Map->GetIsoSize());
		node.y.Format("%d", dwPos / Map->GetIsoSize());
		node.house = GetHouseID(0);
		node.type = *rules.sections["BuildingTypes"].GetValue(0);
	}

	if (m_mapfile.sections.find(HOUSES) == m_mapfile.sections.end() || m_mapfile.sections[HOUSES].values.size() == 0)
	{
		return FALSE;
	}


	int c = atoi(m_mapfile.sections[(LPCTSTR)node.house].values["NodeCount"]);

	c++;
	char sc[50];
	itoa(c, sc, 10);
	m_mapfile.sections[(LPCTSTR)node.house].values["NodeCount"] = sc;

	c--;
	CString p;
	GetNodeName(p, c);


	m_mapfile.sections[(LPCTSTR)node.house].values[p] = node.type;
	m_mapfile.sections[(LPCTSTR)node.house].values[p] += ",";
	m_mapfile.sections[(LPCTSTR)node.house].values[p] += node.y;
	m_mapfile.sections[(LPCTSTR)node.house].values[p] += ",";
	m_mapfile.sections[(LPCTSTR)node.house].values[p] += node.x;

	UpdateNodes(FALSE);

	return TRUE;
}

BOOL CMapData::AddInfantry(INFANTRY* lpInfantry, LPCTSTR lpType, LPCTSTR lpHouse, DWORD dwPos, int suggestedIndex)
{

	INFANTRY infantry;
	if (lpInfantry != NULL)
	{
		infantry = *lpInfantry;
		dwPos = atoi(infantry.x) + atoi(infantry.y) * Map->GetIsoSize();

		// MW Bugfix: not checking if infantry.pos does already exist caused crashes with user scripts!
		if (GetInfantryAt(dwPos, atoi(infantry.pos)) >= 0)
			infantry.pos = "-1";
	}
	else
	{
		char cx[10], cy[10];
		itoa(dwPos % Map->GetIsoSize(), cx, 10);
		itoa(dwPos / Map->GetIsoSize(), cy, 10);



		infantry.action = "Guard";
		infantry.tag = "None";
		infantry.direction = "64";
		infantry.flag1 = "0";
		infantry.flag2 = "-1";
		infantry.flag3 = "0";
		infantry.flag4 = "1";
		infantry.flag5 = "0";
		infantry.strength = "256";
		infantry.house = lpHouse;
		infantry.pos = "-1";
		infantry.type = lpType;
		infantry.x = cx;
		infantry.y = cy;


	}

	if (infantry.pos == "-1")
	{
		int subpos = -1;
		int i;

		if (GetInfantryCountAt(dwPos) == 0)
			subpos = 0;
		else
		{
			int oldInf = GetInfantryAt(dwPos, 0);
			if (oldInf > -1)
			{
				INFANTRY inf;
				GetInfantryData(oldInf, &inf);

				if (inf.pos == "0")
					for (i = 1;i < SUBPOS_COUNT;i++)
					{
						if (GetInfantryAt(dwPos, i) == -1)
						{
							//subpos=i+1;

							char c[50];
							itoa(i, c, 10);
							inf.pos = c;
							DeleteInfantry(oldInf);
							AddInfantry(&inf);
							break;
						}

					}
			}

			// if(GetInfantryAt(dwPos, 0)==oldInf) return FALSE;

			for (i = 0;i < SUBPOS_COUNT;i++)
			{
				if (GetInfantryAt(dwPos, i) == -1)
				{
					subpos = i + 1;
					break;
				}
			}
		}

		if (subpos < 0) return FALSE;
		char c[50];
		itoa(subpos, c, 10);

		infantry.pos = c;
	}



	/*
	CString id=GetFree("Infantry");

	CString value;
	value=infantry.house+","+infantry.type+","+infantry.strength+","+infantry.y+
		","+infantry.x+","+infantry.pos+","+infantry.action+","+infantry.direction+","+
		infantry.tag+","+infantry.flag1+","+infantry.flag2+","+infantry.flag3+","+
		infantry.flag4+","+infantry.flag5;

	m_mapfile.sections["Infantry"].values[id]=(LPCTSTR)value;

	UpdateInfantry(FALSE);
	*/

	infantry.deleted = 0;

	/* This code just pushed it back to the end, caused some trouble (and used memory)
	m_infantry.push_back(infantry);

	int sp=atoi(infantry.pos);
	if(sp>0) sp--;

	int pos=dwPos;
	fielddata[pos].infantry[sp]=m_infantry.size()-1;
	*/

	// below code should be much more compatible to the very old code (the direct ini one)

	int sp = atoi(infantry.pos);
	if (sp > 0) sp--;

	int i;
	BOOL bFound = FALSE;
	if (suggestedIndex >= 0 && suggestedIndex < m_infantry.size())
	{
		if (m_infantry[suggestedIndex].deleted)
		{
			m_infantry[suggestedIndex] = infantry;
			if (dwPos < fielddata_size) fielddata[dwPos].infantry[sp] = suggestedIndex;
			bFound = TRUE;

		}
	}

	if (!bFound)
		for (i = 0;i < m_infantry.size();i++)
		{
			if (m_infantry[i].deleted) // yep, found one, replace it
			{
				m_infantry[i] = infantry;
				if (dwPos < fielddata_size) fielddata[dwPos].infantry[sp] = i;
				bFound = TRUE;
				break;
			}
		}

	if (!bFound)
	{
		m_infantry.push_back(infantry);
		if (dwPos < fielddata_size) fielddata[dwPos].infantry[sp] = m_infantry.size() - 1;
	}


	return TRUE;
}

BOOL CMapData::AddStructure(STRUCTURE* lpStructure, LPCTSTR lpType, LPCTSTR lpHouse, DWORD dwPos, CString suggestedID)
{
	STRUCTURE structure;
	if (lpStructure != NULL)
	{
		structure = *lpStructure;
	}
	else
	{
		char cx[10], cy[10];
		itoa(dwPos % Map->GetIsoSize(), cx, 10);
		itoa(dwPos / Map->GetIsoSize(), cy, 10);

		structure.tag = "None";
		structure.direction = "64";
		structure.flag1 = "1";
		structure.flag2 = "0";
		structure.spotlight = "0";
		structure.flag3 = "0";
		structure.flag4 = "0";
		structure.energy = "1";
		structure.upgrade1 = "None";
		structure.upgrade2 = "None";
		structure.upgrade3 = "None";
		structure.upgradecount = "0";
		structure.strength = "256";
		structure.house = lpHouse;
		structure.type = lpType;
		structure.x = cx;
		structure.y = cy;


	}


	CString id = GetFree("Structures");

	if (suggestedID.GetLength() > 0)
	{
		if (m_mapfile.sections["Structures"].values.find(suggestedID) == m_mapfile.sections["Structures"].values.end())
			id = suggestedID;
	}

	CString value;
	value = structure.house + "," + structure.type + "," + structure.strength + "," + structure.y +
		"," + structure.x + "," + structure.direction + "," + structure.tag + "," + structure.flag1 + "," +
		structure.flag2 + "," + structure.energy + "," + structure.upgradecount + "," + structure.spotlight + ","
		+ structure.upgrade1 + "," + structure.upgrade2 + "," + structure.upgrade3 + "," + structure.flag3 + "," + structure.flag4;

	m_mapfile.sections["Structures"].values[id] = (LPCTSTR)value;

	if (!m_noAutoObjectUpdate) UpdateStructures(FALSE);

	return TRUE;
}



void CMapData::InitializeUnitTypes()
{
	buildingid.clear();
	terrainid.clear();

	int i;
	m_overlayCredits[OverlayCredits_Riparius] = atoi(m_mapfile.GetValueByName("Riparius", "Value", rules.sections["Riparius"].AccessValueByName("Value")));
	m_overlayCredits[OverlayCredits_Cruentus] = atoi(m_mapfile.GetValueByName("Cruentus", "Value", rules.sections["Cruentus"].AccessValueByName("Value"))); 
	m_overlayCredits[OverlayCredits_Vinifera] = atoi(m_mapfile.GetValueByName("Vinifera", "Value", rules.sections["Vinifera"].AccessValueByName("Value")));
	m_overlayCredits[OverlayCredits_Aboreus] = atoi(m_mapfile.GetValueByName("Aboreus", "Value", rules.sections["Aboreus"].AccessValueByName("Value")));
	for (i = 0;i < rules.sections["BuildingTypes"].values.size();i++)
	{
		CString type = *rules.sections["BuildingTypes"].GetValue(i);

		int n = GetBuildingNumber(type);
		buildingid[type] = n;
	}

	for (i = 0;i < m_mapfile.sections["BuildingTypes"].values.size();i++)
	{
		CString type = *m_mapfile.sections["BuildingTypes"].GetValue(i);

		int n = GetBuildingNumber(type);
		buildingid[type] = n;
	}

	for (i = 0;i < rules.sections["TerrainTypes"].values.size();i++)
	{
		CString type = *rules.sections["TerrainTypes"].GetValue(i);

		int n = GetTerrainNumber(type);
		terrainid[type] = n;
	}

	for (i = 0;i < m_mapfile.sections["TerrainTypes"].values.size();i++)
	{
		CString type = *m_mapfile.sections["TerrainTypes"].GetValue(i);

		int n = GetTerrainNumber(type);
		terrainid[type] = n;
	}

#ifdef SMUDGE_SUPP

	for (i = 0;i < rules.sections["SmudgeTypes"].values.size();i++)
	{
		CString type = *rules.sections["SmudgeTypes"].GetValue(i);

		int n = GetSmudgeNumber(type);
		smudgeid[type] = n;
	}

	for (i = 0;i < m_mapfile.sections["SmudgeTypes"].values.size();i++)
	{
		CString type = *m_mapfile.sections["SmudgeTypes"].GetValue(i);

		int n = GetSmudgeNumber(type);
		smudgeid[type] = n;
	}

#endif


}

INT CMapData::GetUnitTypeID(LPCTSTR lpType)
{
	// we only support building types, terrain types and smudge types at the moment
	if (buildingid.find(lpType) != buildingid.end()) return buildingid[lpType];
	if (terrainid.find(lpType) != terrainid.end()) return terrainid[lpType];
#ifdef SMUDGE_SUPP
	if (smudgeid.find(lpType) != smudgeid.end()) return smudgeid[lpType];
#endif
	return 0;
}

void CMapData::GetStdInfantryData(DWORD dwIndex, STDOBJECTDATA* lpStdInfantry) const
{
	/*CString data=*m_mapfile.sections["Infantry"].GetValue(dwIndex);

	lpStdInfantry->house=GetParam(data, 0);
	lpStdInfantry->type=GetParam(data, 1);
	lpStdInfantry->strength=atoi(GetParam(data, 2));
	lpStdInfantry->y=GetParam(data, 3);
	lpStdInfantry->x=atoi(GetParam(data, 4));*/

	lpStdInfantry->house = m_infantry[dwIndex].house;
	lpStdInfantry->type = m_infantry[dwIndex].type;
	lpStdInfantry->strength = m_infantry[dwIndex].strength;
	lpStdInfantry->y = m_infantry[dwIndex].y;
	lpStdInfantry->x = m_infantry[dwIndex].x;

}

void CMapData::GetInfantryData(DWORD dwIndex, INFANTRY* lpInfantry) const
{
	ASSERT(dwIndex < m_infantry.size());

	if (dwIndex >= m_infantry.size()) return;

	/*lpInfantry->house=m_infantry.house;
	lpInfantry->type=m_infantry.;
	lpInfantry->strength=m_infantry.;
	lpInfantry->y=m_infantry.;
	lpInfantry->x=m_infantry.;
	lpInfantry->pos=m_infantry.;
	lpInfantry->action=m_infantry.;
	lpInfantry->direction=m_infantry.;
	lpInfantry->tag=m_infantry.;
	lpInfantry->flag1=m_infantry.;
	lpInfantry->flag2=m_infantry.;
	lpInfantry->flag3=m_infantry.;
	lpInfantry->flag4=m_infantry.;
	lpInfantry->flag5=m_infantry.;*/
	*lpInfantry = m_infantry[dwIndex];

	//memcpy(lpInfantry, &m_infantry[dwIndex], sizeof(INFANTRY));

	//ASSERT(dwIndex>=0 && dwIndex<m_mapfile.sections["Infantry"].values.size());



	/*if(dwIndex>=m_mapfile.sections["Infantry"].values.size()) return;

	CString data=*m_mapfile.sections["Infantry"].GetValue(dwIndex);

	lpInfantry->house=GetParam(data, 0);
	lpInfantry->type=GetParam(data, 1);
	lpInfantry->strength=GetParam(data, 2);
	lpInfantry->y=GetParam(data, 3);
	lpInfantry->x=GetParam(data, 4);
	lpInfantry->pos=GetParam(data, 5);
	lpInfantry->action=GetParam(data, 6);
	lpInfantry->direction=GetParam(data, 7);
	lpInfantry->tag=GetParam(data, 8);
	lpInfantry->flag1=GetParam(data, 9);
	lpInfantry->flag2=GetParam(data, 10);
	lpInfantry->flag3=GetParam(data, 11);
	lpInfantry->flag4=GetParam(data, 12);
	lpInfantry->flag5=GetParam(data, 13);*/

}

void CMapData::GetUnitData(DWORD dwIndex, UNIT* lpUnit) const
{
	const auto section = m_mapfile.GetSection("Units");
	if (!section || dwIndex >= section->values.size())
		return;

	CString data = *section->GetValue(dwIndex);

	lpUnit->house = GetParam(data, 0);
	lpUnit->type = GetParam(data, 1);
	lpUnit->strength = GetParam(data, 2);
	lpUnit->y = GetParam(data, 3);
	lpUnit->x = GetParam(data, 4);
	lpUnit->direction = GetParam(data, 5);
	lpUnit->action = GetParam(data, 6);
	lpUnit->tag = GetParam(data, 7);
	lpUnit->flag1 = GetParam(data, 8);
	lpUnit->flag2 = GetParam(data, 9);
	lpUnit->flag3 = GetParam(data, 10);
	lpUnit->flag4 = GetParam(data, 11);
	lpUnit->flag5 = GetParam(data, 12);
	lpUnit->flag6 = GetParam(data, 13);
}

void CMapData::GetAircraftData(DWORD dwIndex, AIRCRAFT* lpAircraft) const
{
	const auto section = m_mapfile.GetSection("Aircraft");
	if (!section || dwIndex >= section->values.size())
		return;

	CString data = *section->GetValue(dwIndex);

	lpAircraft->house = GetParam(data, 0);
	lpAircraft->type = GetParam(data, 1);
	lpAircraft->strength = GetParam(data, 2);
	lpAircraft->y = GetParam(data, 3);
	lpAircraft->x = GetParam(data, 4);
	lpAircraft->direction = GetParam(data, 5);
	lpAircraft->action = GetParam(data, 6);
	lpAircraft->tag = GetParam(data, 7);
	lpAircraft->flag1 = GetParam(data, 8);
	lpAircraft->flag2 = GetParam(data, 9);
	lpAircraft->flag3 = GetParam(data, 10);
	lpAircraft->flag4 = GetParam(data, 11);
}

BOOL CMapData::AddCelltag(LPCTSTR lpTag, DWORD dwPos)
{
	char j[15];
	char k[15];
	memset(j, 0, 15);
	memset(k, 0, 15);
	itoa(dwPos / GetIsoSize(), j, 10);
	if (strlen(j) < 3)
	{
		strcpy_safe(j + 1, j);
		j[0] = '0';
	}
	if (strlen(j) < 3)
	{
		strcpy_safe(j + 1, j);
		j[0] = '0';
	}
	itoa(dwPos % GetIsoSize(), k, 10);
	strcat(k, j);

	m_mapfile.sections["CellTags"].values[k] = lpTag;

	if (!m_noAutoObjectUpdate) UpdateCelltags(FALSE);
	return TRUE;
}

void CMapData::GetCelltagData(DWORD dwIndex, CString* lpTag, DWORD* lpdwPos) const
{
	const auto section = m_mapfile.GetSection("CellTags");
	if (!section || dwIndex >= section->values.size())
		return;

	int x, y;
	CString pos;

	pos = *section->GetValueName(dwIndex);
	PosToXY(pos, &x, &y);

	const auto tag = section->GetValueByName(pos);
	*lpTag = tag;
	*lpdwPos = x + y * GetIsoSize();
}

BOOL CMapData::AddAircraft(AIRCRAFT* lpAircraft, LPCTSTR lpType, LPCTSTR lpHouse, DWORD dwPos, CString suggestedID)
{
	AIRCRAFT aircraft;

	if (lpAircraft != NULL)
	{
		aircraft = *lpAircraft;
	}
	else
	{
		char sx[15], sy[15];
		itoa(dwPos % GetIsoSize(), sx, 10);
		itoa(dwPos / GetIsoSize(), sy, 10);

		aircraft.type = lpType;
		aircraft.house = lpHouse;
		aircraft.action = "Guard";
		aircraft.tag = "None";
		aircraft.direction = "64";
		aircraft.strength = "256";
		aircraft.x = sx;
		aircraft.y = sy;
		aircraft.flag1 = "0";
		aircraft.flag2 = "0";
		aircraft.flag3 = "1";
		aircraft.flag4 = "0";
	}

	CString id = GetFree("Aircraft");

	if (suggestedID.GetLength() > 0)
	{
		if (m_mapfile.sections["Aircraft"].values.find(suggestedID) == m_mapfile.sections["Aircraft"].values.end())
			id = suggestedID;
	}

	CString value;
	value = aircraft.house + "," + aircraft.type + "," + aircraft.strength + "," + aircraft.y + "," +
		aircraft.x + "," + aircraft.direction + "," + aircraft.action + "," + aircraft.tag + ","
		+ aircraft.flag1 + "," + aircraft.flag2 + "," + aircraft.flag3 + "," + aircraft.flag4;

	m_mapfile.sections["Aircraft"].values[id] = value;

	if (!m_noAutoObjectUpdate) UpdateAircraft(FALSE);

	return TRUE;
}

BOOL CMapData::AddUnit(UNIT* lpUnit, LPCTSTR lpType, LPCTSTR lpHouse, DWORD dwPos, CString suggestedID)
{
	UNIT unit;

	if (lpUnit != NULL)
	{
		unit = *lpUnit;
	}
	else
	{
		char sx[15], sy[15];
		itoa(dwPos % GetIsoSize(), sx, 10);
		itoa(dwPos / GetIsoSize(), sy, 10);

		unit.type = lpType;
		unit.house = lpHouse;
		unit.action = "Guard";
		unit.tag = "None";
		unit.direction = "64";
		unit.strength = "256";
		unit.x = sx;
		unit.y = sy;
		unit.flag1 = "0";
		unit.flag2 = "-1";
		unit.flag3 = "0";
		unit.flag4 = "-1";
		unit.flag5 = "1";
		unit.flag6 = "0";

	}

	CString id = GetFree("Units");

	if (suggestedID.GetLength() > 0)
	{
		if (m_mapfile.sections["Units"].values.find(suggestedID) == m_mapfile.sections["Units"].values.end())
			id = suggestedID;
	}

	CString value;
	value = unit.house + "," + unit.type + "," + unit.strength + "," + unit.y + "," +
		unit.x + "," + unit.direction + "," + unit.action + "," + unit.tag + ","
		+ unit.flag1 + "," + unit.flag2 + "," + unit.flag3 + "," + unit.flag4 + "," + unit.flag5 + "," + unit.flag6;

	m_mapfile.sections["Units"].values[id] = value;

	if (!m_noAutoObjectUpdate) UpdateUnits(FALSE);
	return TRUE;
}

void CMapData::GetTerrainData(DWORD dwIndex, CString* lpType) const
{
	ASSERT(m_terrain.size() > dwIndex);

	/*	if(m_mapfile.sections["Terrain"].values.size()<=dwIndex)
	{
		errstream << "GetTerrainData() fails..." << endl;
		errstream.flush();
		*lpType="";
		return;
	}*/
	*lpType = m_terrain[dwIndex].type;//*m_mapfile.sections["Terrain"].GetValue(dwIndex);
}

// MW New for script interpreter:
void CMapData::GetTerrainData(DWORD dwIndex, TERRAIN* lpType) const
{
	ASSERT(m_terrain.size() > dwIndex);

	*lpType = m_terrain[dwIndex];
}

BOOL CMapData::AddTerrain(LPCTSTR lpType, DWORD dwPos, int suggestedIndex)
{
	/*char j1[15];
	char k1[15];*/

	int x, y;
	x = dwPos % GetIsoSize();
	y = dwPos / GetIsoSize();

	/*itoa(y, j1, 10);
	if(strlen(j1)<3)
	{
		strcpy_safe(j1+1, j1);
		j1[0]='0';
	}
	if(strlen(j1)<3)
	{
		strcpy_safe(j1+1, j1);
		j1[0]='0';
	}
	itoa(x, k1, 10);
	strcat(k1, j1);*/



	/*
	March 23th, 2001: Replaced slow UpdateTerrain() call with direct modification:
	*/

	TERRAIN td;
	td.deleted = 0;
	td.type = lpType;
	td.x = x;
	td.y = y;

	if (terrainid.find(td.type) == terrainid.end()) return FALSE;

	BOOL bFound = FALSE;

	if (suggestedIndex >= 0 && suggestedIndex < m_terrain.size())
	{
		if (m_terrain[suggestedIndex].deleted)
		{
			m_terrain[suggestedIndex] = td;
			if (dwPos < fielddata_size)
			{
				fielddata[dwPos].terrain = suggestedIndex;
				fielddata[dwPos].terraintype = terrainid[lpType];
			}
			bFound = TRUE;

		}
	}

	int i;
	if (!bFound)
		for (i = 0;i < m_terrain.size();i++)
		{
			if (m_terrain[i].deleted) // yep, found one, replace it
			{
				m_terrain[i] = td;
				if (dwPos < fielddata_size)
				{
					fielddata[dwPos].terrain = i;
					fielddata[dwPos].terraintype = terrainid[lpType];
				}
				bFound = TRUE;
				break;
			}
		}

	if (!bFound)
	{
		m_terrain.push_back(td);
		int pos = x + y * GetIsoSize();
		if (pos < fielddata_size)
		{
			fielddata[pos].terrain = m_terrain.size() - 1;
			fielddata[pos].terraintype = terrainid[lpType];
		}
	}

	//UpdateTerrain();

	return TRUE;

}

BOOL CMapData::IsGroundObjectAt(DWORD dwPos) const
{
	int m_id = 0;

	m_id = GetInfantryAt(dwPos);
	if (m_id < 0)
		m_id = GetUnitAt(dwPos);
	if (m_id < 0)
		m_id = GetAirAt(dwPos);
	if (m_id < 0)
		m_id = GetStructureAt(dwPos);
	if (m_id < 0)
		m_id = GetTerrainAt(dwPos);

	if (m_id < 0) return FALSE;

	return TRUE;

}

void CMapData::GetWaypointData(DWORD dwIndex, CString* lpID, DWORD* lpdwPos) const
{
	if (lpID) *lpID = "";
	if (lpdwPos) *lpdwPos = 0;

	const auto section = m_mapfile.GetSection("Waypoints");
	if (!section || dwIndex >= section->values.size())
		return;

	CString data = *section->GetValue(dwIndex);

	int x, y;
	PosToXY(data, &x, &y);

	if (lpID) *lpID = *section->GetValueName(dwIndex);
	if (lpdwPos) *lpdwPos = x + y * GetIsoSize();

}

void CMapData::GetStdAircraftData(DWORD dwIndex, STDOBJECTDATA* lpStdAircraft) const
{
	const auto section = m_mapfile.GetSection("Aircraft");
	if (!section || dwIndex >= section->values.size())
		return;

	CString data = *section->GetValue(dwIndex);

	lpStdAircraft->house = GetParam(data, 0);
	lpStdAircraft->type = GetParam(data, 1);
	lpStdAircraft->strength.Format("%d", atoi(GetParam(data, 2)));
	lpStdAircraft->y = GetParam(data, 3);
	lpStdAircraft->x.Format("%d", atoi(GetParam(data, 4)));
}

void CMapData::GetStdUnitData(DWORD dwIndex, STDOBJECTDATA* lpStdUnit) const
{
	const auto section = m_mapfile.GetSection("Units");
	if (!section || dwIndex >= section->values.size())
		return;

	CString data = *section->GetValue(dwIndex);

	lpStdUnit->house = GetParam(data, 0);
	lpStdUnit->type = GetParam(data, 1);
	lpStdUnit->strength.Format("%d", atoi(GetParam(data, 2)));
	lpStdUnit->y = GetParam(data, 3);
	lpStdUnit->x.Format("%d", atoi(GetParam(data, 4)));
}

DWORD CMapData::GetInfantryCount() const
{
	return m_infantry.size();//m_mapfile.sections["Infantry"].values.size();
}

DWORD CMapData::GetUnitCount() const
{
	const auto section = m_mapfile.GetSection("Units");
	return section ? section->values.size() : 0;
}

DWORD CMapData::GetStructureCount() const
{
	const auto section = m_mapfile.GetSection("Structures");
	return section ? section->values.size() : 0;
}

DWORD CMapData::GetAircraftCount() const
{
	const auto section = m_mapfile.GetSection("Aircraft");
	return section ? section->values.size() : 0;
}

DWORD CMapData::GetTerrainCount() const
{
	return m_terrain.size();//m_mapfile.sections["Terrain"].values.size();
}

/*
CString CMapData::GetUnitName(LPCTSTR lpID);

Returns the name of the unit identified with the ID lpID.
Note: takes care of the current mapfile settings
*/
WCHAR unknown[] = L"MISSING";
WCHAR* CMapData::GetUnitName(LPCTSTR lpID) const
{
	WCHAR* res = NULL;

	if (g_data.sections["Rename"].FindName(lpID) >= 0)
	{
		CCStrings[lpID].SetString((LPSTR)(LPCSTR)GetLanguageStringACP(g_data.sections["Rename"].values[(LPCSTR)lpID]));
		res = CCStrings[lpID].wString;
		return res;
	}

	if (CCStrings.find(lpID) != CCStrings.end() && CCStrings[lpID].len > 0) res = CCStrings[lpID].wString;

	if (!res && m_mapfile.sections.find(lpID) != m_mapfile.sections.end())
	{
		const auto section = m_mapfile.GetSection(lpID);
		if (section && section->values.find("Name") != section->values.end())
		{
			CCStrings[lpID].SetString(section->values.at("Name"));

			res = CCStrings[lpID].wString;
		}
	}

	if (!res && rules.sections.find(lpID) != rules.sections.end())
	{
		const auto section = rules.GetSection(lpID);
		if (section && section->values.find("Name") != section->values.end())
		{
			CCStrings[lpID].SetString(section->values.at("Name"));
			res = CCStrings[lpID].wString;
		}
	}

	if (!res)
	{
		CCStrings[lpID].SetString(L"MISSING", 7);
		res = CCStrings[lpID].wString;
	}


	return res;

	//return CString("");
}

DWORD CMapData::GetCelltagCount() const
{
	const auto section = m_mapfile.GetSection("CellTags");
	return section ? section->values.size() : 0;
}

DWORD CMapData::GetWaypointCount() const
{
	const auto section = m_mapfile.GetSection("Waypoints");
	return section ? section->values.size() : 0;
}

void CMapData::DeleteRulesSections()
{
	int i;

	// delete any rules sections except the types lists (we need those to get the data of new units in the map)...
	for (i = 0; i < m_mapfile.sections.size(); i++)
	{
		CString name = *m_mapfile.GetSectionName(i);

		if (IsRulesSection(name) && name.Find("Types") < 0)
		{
			m_mapfile.sections.erase(name);
		}
	}

	// now delete these types lists...
	for (i = 0; i < m_mapfile.sections.size(); i++)
	{
		CString name = *m_mapfile.GetSectionName(i);

		if (IsRulesSection(name))
		{
			m_mapfile.sections.erase(name);
		}
	}
}

BOOL CMapData::IsRulesSection(LPCTSTR lpSection)
{
	int i;
	for (i = 0;i < GetHousesCount();i++)
		if (GetHouseID(i) == lpSection) return FALSE;

	if (strcmp(lpSection, HOUSES) == NULL) return FALSE;
	if (strcmp(lpSection, "VariableNames") == NULL) return FALSE;

	if (rules.sections.find(lpSection) != rules.sections.end()) return TRUE;


	if (m_mapfile.sections.find("InfantryTypes") != m_mapfile.sections.end())
		if (m_mapfile.sections["InfantryTypes"].FindValue(lpSection) >= 0)
			return TRUE;

	if (m_mapfile.sections.find("VehicleTypes") != m_mapfile.sections.end())
		if (m_mapfile.sections["VehicleTypes"].FindValue(lpSection) >= 0)
			return TRUE;

	if (m_mapfile.sections.find("AircraftTypes") != m_mapfile.sections.end())
		if (m_mapfile.sections["AircraftTypes"].FindValue(lpSection) >= 0)
			return TRUE;

	if (m_mapfile.sections.find("BuildingTypes") != m_mapfile.sections.end())
		if (m_mapfile.sections["BuildingTypes"].FindValue(lpSection) >= 0)
			return TRUE;

	if (m_mapfile.sections.find("TerrainTypes") != m_mapfile.sections.end())
		if (m_mapfile.sections["TerrainTypes"].FindValue(lpSection) >= 0)
			return TRUE;

	if ((CString)"IsoMapPack5" != lpSection && (CString)"OverlayPack" != lpSection && (CString)"OverlayDataPack" != lpSection && m_mapfile.sections.find(lpSection) != m_mapfile.sections.end())
	{
		CIniFileSection& sec = m_mapfile.sections[lpSection];
		if (sec.FindName("ROF") > -1 && sec.FindName("Range") > -1 &&
			sec.FindName("Damage") > -1 && sec.FindName("Warhead") > -1)
			return TRUE; // a weapon

		if (sec.FindName("Spread") > -1 && sec.FindName("Range") > -1 &&
			sec.FindName("Damage") > -1 && sec.FindName("Warhead") > -1)
			return TRUE; // a warhead

		// check for projectile/warhead
		for (i = 0;i < m_mapfile.sections.size();i++)
		{
			CString name = *m_mapfile.GetSectionName(i);
			if ((CString)"IsoMapPack5" != name && (CString)"OverlayPack" != name && (CString)"OverlayDataPack" != name && (m_mapfile.GetSection(i)->FindName("Projectile") > -1 || m_mapfile.GetSection(i)->FindName("Warhead") > -1))
			{
				// MW Bugfix: Check if is found in Projectile first...
				// This may have caused several crashes while saving
				if (m_mapfile.GetSection(i)->FindName("Projectile") >= 0)
					if (*m_mapfile.GetSection(i)->GetValue(m_mapfile.GetSection(i)->FindName("Projectile")) == lpSection)
						return TRUE;
			}
		}
	}



	return FALSE;

}

void CMapData::ExportRulesChanges(const char* filename)
{
	CIniFile rul;

	int i;
	for (i = 0;i < m_mapfile.sections.size();i++)
	{

		CString name = *m_mapfile.GetSectionName(i);

		if (IsRulesSection(name))
		{

			rul.sections[name] = *m_mapfile.GetSection(i);

		}
	}

	rul.SaveFile(std::string(filename));
}

void CMapData::ImportRUL(LPCTSTR lpFilename)
{
	m_mapfile.InsertFile(std::string(lpFilename), NULL);
	m_mapfile.sections.erase("Editor");
	UpdateBuildingInfo();
	UpdateTreeInfo();
}

void CMapData::UpdateMapFieldData(BOOL bSave)
{
	if (bSave == FALSE)
	{
		int i;
		int e;
		/*for(i=0;i<GetIsoSize()*GetIsoSize();i++)
		{
			int dwX=i%m_IsoSize;
			int dwY=i/m_IsoSize;
			int mapwidth=Map->GetWidth();
				int mapheight=Map->GetHeight();
				int k=1;

			if( dwX<1|| dwY<1 || dwX+dwY<mapwidth+k || dwX+dwY>mapwidth+mapheight*2 || (dwY+k>mapwidth && dwX-k<dwY-mapwidth) || (dwX+k>mapwidth && dwY+mapwidth-k<dwX))
			{

			}
			else
			{
				BOOL bFound=FALSE;
				for(e=0;e<dwIsoMapSize;e++)
				{
					MAPFIELDDATA* mfd=(MAPFIELDDATA*)&m_mfd[e*MAPFIELDDATA_SIZE];
					if(mfd->wY==dwX && mfd->wX==dwY)
					{
						bFound=TRUE;
						break;
					}
				}

				if(!bFound)
				{
					errstream << dwX << " " << dwY << " not found";
					errstream.flush();
				}
			}
		}*/

		/*	int count=0;
			int max_count=0;
			BYTE b;
			b=m_mfd[0];
			for (i=0;i<dwIsoMapSize*MAPFIELDDATA_SIZE;i++)
			{
				if(m_mfd[i]!=b) { b=m_mfd[i]; count=0; }
				if(m_mfd[i]==b) count++;
				if(max_count<count) max_count=count;
				if(count>10) MessageBox(0,"uh","",0);
			}

			char c[50];
			itoa(max_count,c,10);
			MessageBox(0,c,"",0);*/

		const bool mp = IsMultiplayer();
		for (i = 0;i < dwIsoMapSize;i++)
		{
			MAPFIELDDATA* mfd = (MAPFIELDDATA*)&m_mfd[i * MAPFIELDDATA_SIZE];
			int pos = mfd->wY + mfd->wX * GetIsoSize();
			if (dwIsoMapSize - i < 50) errstream << mfd->wY << " " << mfd->wX << endl;
			if (pos < (GetIsoSize() + 1) * (GetIsoSize() + 1))
			{
				fielddata[pos].wGround = mfd->wGround;
				fielddata[pos].bHeight = mfd->bHeight;
				memcpy(&fielddata[pos].bMapData, mfd->bData, 3);
				memcpy(&fielddata[pos].bMapData2, mfd->bData2, 1);

				int replacement = 0;
				int ground = mfd->wGround;
				if (ground == 0xFFFF) ground = 0;

				if ((*tiledata)[ground].bReplacementCount && atoi((*tiles).sections["General"].values["BridgeSet"]) != (*tiledata)[ground].wTileSet)
				{
					replacement = rand() * (1 + (*tiledata)[ground].bReplacementCount) / RAND_MAX;
				}

				fielddata[pos].bRNDImage = replacement;

				Mini_UpdatePos(i % GetIsoSize(), i / GetIsoSize(), mp);

				/*int dwX=mfd->wY;
				int dwY=mfd->wX;
				int mapwidth=Map->GetWidth();
				int mapheight=Map->GetHeight();
				const int k=1;
				if( dwX<1|| dwY<1 || dwX+dwY<mapwidth+k || dwX+dwY>mapwidth+mapheight*2 || (dwY+k>mapwidth && dwX-k<dwY-mapwidth) || (dwX+k>mapwidth && dwY+mapwidth-k<dwX))
				{
					errstream << "Outside " << dwX << " " << dwY << endl;
					errstream.flush();
				}*/

			}
			else // if(mfd->wY==0xFFFE && mfd->wX==0xFFFE)
			{
				char c[2];
				c[0] = mfd->bHeight;
				c[1] = 0;
				OutputDebugString(c);
			}

			/*if(mfd->bData[0] || mfd->bData[1] || mfd->bData2[0])
			{
				char c[50];
				itoa(mfd->wY, c, 10);
				OutputDebugString("Data at ");
				OutputDebugString(c);
				OutputDebugString(" ");
				itoa(mfd->wX,c,10);
				OutputDebugString(c);
				OutputDebugString(": ");
				itoa(mfd->bData[0], c, 10);
				OutputDebugString(c);
				OutputDebugString(" ");
				itoa(mfd->bData[1], c, 10);
				OutputDebugString(c);
				OutputDebugString(" ");
				itoa(mfd->bData[2], c, 10);
				OutputDebugString(c);
				OutputDebugString(" ");
				itoa(mfd->bData2[0], c, 10);
				OutputDebugString(c);
				OutputDebugString("\n");
			}*/

		}

		for (i = 0;i < m_IsoSize;i++)
		{
			for (e = 0;e < m_IsoSize;e++)
			{
				int pos = i + e * m_IsoSize;
				int xx, yy;
				fielddata[pos].bRedrawTerrain = FALSE;
				for (xx = -2;xx < 0;xx++)
				{
					for (yy = -2;yy < 0;yy++)
					{
						int npos = pos + xx + yy * m_IsoSize;
						if (npos > 0 && fielddata[pos].bHeight - fielddata[npos].bHeight >= 4)
						{
							fielddata[pos].bRedrawTerrain = TRUE;
							break;
						}
					}
					if (fielddata[pos].bRedrawTerrain) break;
				}
			}
		}
	}
	else
	{
		int x, y, n = 0;

		// this code here must be improved to produce smaller maps. Just ignore the data outside the visible rect!

		if (m_mfd) delete[] m_mfd;
		dwIsoMapSize = m_IsoSize * m_IsoSize + 15;
		m_mfd = new(BYTE[(dwIsoMapSize + m_IsoSize * m_IsoSize) * MAPFIELDDATA_SIZE]);
		memset(m_mfd, 0, dwIsoMapSize * MAPFIELDDATA_SIZE);



		int p = 0;
		const int width = m_maprect.right;
		const int height = m_maprect.bottom;

		errstream << "Memory allocated for mappack, saving fielddata into mapfields" << endl;
		errstream.flush();

		int i;
		int mapwidth = m_maprect.right;
		int mapheight = m_maprect.bottom;

		//#ifdef UNUSED
		int dwX, dwY;
		for (dwX = 0;dwX <= m_IsoSize;dwX++)
		{
			for (dwY = 0;dwY <= m_IsoSize;dwY++)
			{
				/*for(i=0;i<fielddata_size;i++)
				{*/
				//int dwX=i%m_IsoSize;
				//int dwY=i/m_IsoSize;
				i = dwX + dwY * m_IsoSize;

				//if( dwX<2|| dwY<2 || dwX+dwY<mapwidth+2 || dwX+dwY+2>mapwidth+mapheight*2 || (dwY+2>mapwidth && dwX-2<dwY-mapwidth) || (dwX+2>mapwidth && dwY+mapwidth-2<dwX)) continue;
				if (dwX < 1 || dwY < 1 || dwX + dwY<mapwidth + 1 || dwX + dwY>mapwidth + mapheight * 2 || (dwY + 1 > mapwidth && dwX - 1 < dwY - mapwidth) || (dwX + 1 > mapwidth && dwY + mapwidth - 1 < dwX)) continue;


				MAPFIELDDATA* mfd = (MAPFIELDDATA*)&m_mfd[p * MAPFIELDDATA_SIZE];
				mfd->wGround = fielddata[i].wGround;
				mfd->wX = dwY;
				mfd->wY = dwX;
				mfd->bHeight = fielddata[i].bHeight;
				memcpy(&mfd->bData, &fielddata[i].bMapData, 3); // includes fielddata[i].bSubTile!
				memcpy(&mfd->bData2, &fielddata[i].bMapData2, 1);

				p++;
			}

		}
		//#endif

		/*


				if(width>height)
				{
					int add_bottom=1;
					int add_top=-1;
					int n_bottom, n_top;
					n_bottom=n_top=width;
					for(x=0;x<width+height+1;x++)
					{

						for(y=n_top;y<=n_bottom;y++)
						{
							if(y<width+height+1 && y>=0)
							{
								if( x<1|| y<1 || x+y<mapwidth+1 || x+y>mapwidth+mapheight*2 || (y+1>mapwidth && x-1<y-mapwidth) || (x+1>mapwidth && y+mapwidth-1<x)) continue;


								int pos=y+x*GetIsoSize();

								MAPFIELDDATA* mfd=(MAPFIELDDATA*)&m_mfd[p*MAPFIELDDATA_SIZE];
								mfd->wGround=fielddata[pos].wGround;
								mfd->wX=x;
								mfd->wY=y;
								mfd->bHeight=fielddata[pos].bHeight;
								memcpy(&mfd->bData, &fielddata[pos].bMapData, 3);
								memcpy(&mfd->bData2, &fielddata[pos].bMapData2, 1);

								p++;
							}
						}

						if(x==height) add_bottom=-1;
						if(x==width) add_top=1;

						n_top+=add_top;
						n_bottom+=add_bottom;
					}
				}
				else
				{
					int add_right=1;
					int add_left=-1;
					int n_right, n_left;
					n_right=n_left=height;
					for(y=width+height;y>=0;y--)
					{

						for(x=n_left;x<=n_right;x++)
						{
							if(x<width+height && x>=0)
							{
								if( x<1|| y<1 || x+y<mapwidth+1 || x+y>mapwidth+mapheight*2 || (y+1>mapwidth && x-1<y-mapwidth) || (x+1>mapwidth && y+mapwidth-1<x)) continue;

								int pos=y+x*GetIsoSize();

								//if(pos>0 && pos<=
								MAPFIELDDATA* mfd=(MAPFIELDDATA*)&m_mfd[p*MAPFIELDDATA_SIZE];
								mfd->wGround=fielddata[pos].wGround;
								mfd->wX=x;
								mfd->wY=y;
								mfd->bHeight=fielddata[pos].bHeight;
								memcpy(&mfd->bData, &fielddata[pos].bMapData, 3);
								memcpy(&mfd->bData2, &fielddata[pos].bMapData2, 1);

								p++;
							}
						}

						if(y==height) add_right=-1;
						if(y==width) add_left=1;

						n_left+=add_left;
						n_right+=add_right;
					}
				}*/

				/*
				else
				{
					for(y=height;y>=0;y--)
					{
						for(x=0;x<=n;x++)
						{
							int pos=y+x*GetIsoSize();

							MAPFIELDDATA* mfd=(MAPFIELDDATA*)&m_mfd[p*MAPFIELDDATA_SIZE];
							mfd->wGround=fielddata[pos].wGround;
							mfd->wX=x;
							mfd->wY=y;
							mfd->bHeight=fielddata[pos].bHeight;
							memcpy(&mfd->bData, &fielddata[pos].bMapData, 3);
							memcpy(&mfd->bData2, &fielddata[pos].bMapData2, 1);
							char c[50];
							itoa(x,c,10);
							OutputDebugString(c);
							OutputDebugString(" ");
							itoa(y,c,10);
							OutputDebugString(c);
							OutputDebugString(" ");
							itoa(p,c,10);
							OutputDebugString(c);
							OutputDebugString(" ");
							OutputDebugString("\n");
							p++;
						}
						if(height-y<width)
						n++;
						else if(y<=width)
						n--;


					}
				}*/

		dwIsoMapSize = p;//+14; // dwIsoMapSize is smaller than whole iso map!



		int startpos = (p)*MAPFIELDDATA_SIZE;
		/*for(i=0;i<14;i++)
		{
			MAPFIELDDATA* mfd=(MAPFIELDDATA*)&m_mfd[startpos+i*MAPFIELDDATA_SIZE];
			mfd->wX=0xFFFE;
			mfd->wY=0xFFFE;
			int c;
			switch(i)
			{
			case 0:
				c='F';
				break;
			case 1:
				c='I';
				break;
			case 2:
				c='N';
				break;
			case 3:
				c='A';
				break;
			case 4:
				c='L';
				break;
			case 5:
				c='S';
				break;
			case 6:
				c='U';
				break;
			case 7:
				c='N';
				break;
			case 8:
				c='/';
				break;
			case 9:
				c='A';
				break;
			case 10:
				c='L';
				break;
			case 11:
				c='E';
				break;
			case 12:
				c='R';
				break;
			case 13:
				c='T';
				break;
			}
			mfd->bHeight=c;

		}*/
		/*for(i=0;i<m_IsoSize;i++)
		{
			for(e=0;e<m_IsoSize;e++)
			{
				int pos=e+i*GetIsoSize();

				MAPFIELDDATA* mfd=(MAPFIELDDATA*)&m_mfd[n*MAPFIELDDATA_SIZE];
				mfd->wGround=fielddata[pos].wGround;
				mfd->wX=i;
				mfd->wY=e;
				mfd->bHeight=fielddata[pos].bHeight;
				memcpy(&mfd->bData, &fielddata[pos].bMapData, 3);
				memcpy(&mfd->bData2, &fielddata[pos].bMapData2, 1);
				n++;
			}
		}*/


	}
}

void CMapData::UpdateBuildingInfo(LPCSTR lpUnitType)
{
	CIniFile& ini = GetIniFile();

	if (!lpUnitType)
	{
		memset(buildinginfo, 0, 0x0F00 * sizeof(BUILDING_INFO));

		int i;
		for (i = 0;i < rules.sections["BuildingTypes"].values.size();i++)
		{


			CString type = *rules.sections["BuildingTypes"].GetValue(i);
			CString artname = type;


			if (rules.sections[type].values.find("Image") != rules.sections[type].values.end())
			{
				artname = rules.sections[type].values["Image"];
			}
			if (ini.sections.find(type) != ini.sections.end())
			{
				if (ini.sections[type].values.find("Image") != ini.sections[type].values.end())
				{
					artname = ini.sections[type].values["Image"];
				}
			}

			int w, h;
			char d[6];
			memcpy(d, art.sections[artname].values["Foundation"], 1);
			d[1] = 0;
			w = atoi(d);
			if (w == 0) w = 1;
			memcpy(d, (LPCTSTR)art.sections[artname].values["Foundation"] + 2, 1);
			d[1] = 0;
			h = atoi(d);
			if (h == 0) h = 1;

			int n = Map->GetUnitTypeID(type);

			if (n >= 0 && n < 0x0F00)
			{
				buildinginfo[n].w = w;
				buildinginfo[n].h = h;

				buildinginfo[n].bSnow = FALSE;
				buildinginfo[n].bTemp = FALSE;
				buildinginfo[n].bUrban = FALSE;


				CString lpPicFile = GetUnitPictureFilename(type, 0);

				if (pics.find(lpPicFile) != pics.end())
				{
					if (pics[lpPicFile].bTerrain == TheaterChar::None)
					{
						buildinginfo[n].bSnow = TRUE;
						buildinginfo[n].bTemp = TRUE;
						buildinginfo[n].bUrban = TRUE;
					}
					else if (pics[lpPicFile].bTerrain == TheaterChar::T) buildinginfo[n].bTemp = TRUE;
					else if (pics[lpPicFile].bTerrain == TheaterChar::A) buildinginfo[n].bSnow = TRUE;
					else if (pics[lpPicFile].bTerrain == TheaterChar::U) buildinginfo[n].bUrban = TRUE;
				}
				else
				{
					buildinginfo[n].bSnow = TRUE;
					buildinginfo[n].bTemp = TRUE;
					buildinginfo[n].bUrban = TRUE;
				}

				buildinginfo[n].pic_count = 8;
				int k;
				for (k = 0;k < 8;k++)
				{
					lpPicFile = GetUnitPictureFilename(type, k);

					if (pics.find(lpPicFile) != pics.end())
					{
						buildinginfo[n].pic[k] = pics[lpPicFile];
					}
					else
					{
						buildinginfo[n].pic[k].pic = NULL;
					}

				}


			}
			else
			{
				errstream << "Building not found " << endl;
				errstream.flush();
			}

		}

		for (i = 0;i < ini.sections["BuildingTypes"].values.size();i++)
		{


			CString type = *ini.sections["BuildingTypes"].GetValue(i);
			CString artname = type;


			if (ini.sections.find(type) != ini.sections.end())
			{
				if (ini.sections[type].values.find("Image") != ini.sections[type].values.end())
				{
					artname = ini.sections[type].values["Image"];
				}
			}

			int w, h;
			char d[6];
			memcpy(d, art.sections[artname].values["Foundation"], 1);
			d[1] = 0;
			w = atoi(d);
			if (w == 0) w = 1;
			memcpy(d, (LPCTSTR)art.sections[artname].values["Foundation"] + 2, 1);
			d[1] = 0;
			h = atoi(d);
			if (h == 0) h = 1;

			int n = Map->GetUnitTypeID(type);

			if (n >= 0 && n < 0x0F00)
			{
				buildinginfo[n].w = w;
				buildinginfo[n].h = h;
				buildinginfo[n].bSnow = TRUE;
				buildinginfo[n].bTemp = TRUE;
				buildinginfo[n].bUrban = TRUE;

				CString lpPicFile = GetUnitPictureFilename(type, 0);

				int k;
				for (k = 0;k < 8;k++)
				{
					lpPicFile = GetUnitPictureFilename(type, k);

					if (pics.find(lpPicFile) != pics.end())
					{
						buildinginfo[n].pic[k] = pics[lpPicFile];
					}
					else
					{
						buildinginfo[n].pic[k].pic = NULL;
					}
				}

			}

		}
	}
	else
	{
		// only for specific building -> faster
		CString type = lpUnitType;
		CString artname = type;


		if (rules.sections[type].values.find("Image") != rules.sections[type].values.end())
		{
			artname = rules.sections[type].values["Image"];
		}
		if (ini.sections.find(type) != ini.sections.end())
		{
			if (ini.sections[type].values.find("Image") != ini.sections[type].values.end())
			{
				artname = ini.sections[type].values["Image"];
			}
		}

		int w, h;
		char d[6];
		memcpy(d, art.sections[artname].values["Foundation"], 1);
		d[1] = 0;
		w = atoi(d);
		if (w == 0) w = 1;
		memcpy(d, (LPCTSTR)art.sections[artname].values["Foundation"] + 2, 1);
		d[1] = 0;
		h = atoi(d);
		if (h == 0) h = 1;

		int n = Map->GetUnitTypeID(type);

		if (n >= 0 && n < 0x0F00)
		{
			buildinginfo[n].w = w;
			buildinginfo[n].h = h;
			CString lpPicFile = GetUnitPictureFilename(type, 0);
			buildinginfo[n].pic_count = 8;

			int k;
			for (k = 0;k < 8;k++)
			{
				lpPicFile = GetUnitPictureFilename(type, k);

				if (pics.find(lpPicFile) != pics.end())
				{
					buildinginfo[n].pic[k] = pics[lpPicFile];
				}
				else
				{
					buildinginfo[n].pic[k].pic = NULL;
				}
			}

		}
	}

}

void CMapData::UpdateTreeInfo(LPCSTR lpTreeType)
{
	CIniFile& ini = GetIniFile();

	if (!lpTreeType)
	{
		memset(treeinfo, 0, 0x0F00 * sizeof(TREE_INFO));

		int i;
		for (i = 0;i < rules.sections["TerrainTypes"].values.size();i++)
		{


			CString type = *rules.sections["TerrainTypes"].GetValue(i);
			CString artname = type;


			if (rules.sections[type].values.find("Image") != rules.sections[type].values.end())
			{
				artname = rules.sections[type].values["Image"];
			}
			if (ini.sections.find(type) != ini.sections.end())
			{
				if (ini.sections[type].values.find("Image") != ini.sections[type].values.end())
				{
					artname = ini.sections[type].values["Image"];
				}
			}

			int w, h;
			char d[6];
			memcpy(d, art.sections[artname].values["Foundation"], 1);
			d[1] = 0;
			w = atoi(d);
			if (w == 0) w = 1;
			memcpy(d, (LPCTSTR)art.sections[artname].values["Foundation"] + 2, 1);
			d[1] = 0;
			h = atoi(d);
			if (h == 0) h = 1;

			int n = GetUnitTypeID(type);

			if (n >= 0 && n < 0x0F00)
			{
				treeinfo[n].w = w;
				treeinfo[n].h = h;

				CString lpPicFile = GetUnitPictureFilename(type, 0);

				if (pics.find(lpPicFile) != pics.end())
				{

					treeinfo[n].pic = pics[lpPicFile];
				}
				else
					treeinfo[n].pic.pic = NULL;
			}

		}

		for (i = 0;i < ini.sections["TerrainTypes"].values.size();i++)
		{


			CString type = *ini.sections["TerrainTypes"].GetValue(i);
			CString artname = type;


			if (ini.sections.find(type) != ini.sections.end())
			{
				if (ini.sections[type].values.find("Image") != ini.sections[type].values.end())
				{
					artname = ini.sections[type].values["Image"];
				}
			}

			int w, h;
			char d[6];
			memcpy(d, art.sections[artname].values["Foundation"], 1);
			d[1] = 0;
			w = atoi(d);
			if (w == 0) w = 1;
			memcpy(d, (LPCTSTR)art.sections[artname].values["Foundation"] + 2, 1);
			d[1] = 0;
			h = atoi(d);
			if (h == 0) h = 1;

			int n = Map->GetUnitTypeID(type);

			if (n >= 0 && n < 0x0F00)
			{
				treeinfo[n].w = w;
				treeinfo[n].h = h;

				CString lpPicFile = GetUnitPictureFilename(type, 0);

				if (pics.find(lpPicFile) != pics.end())
				{
					treeinfo[n].pic = pics[lpPicFile];
				}
				else
					treeinfo[n].pic.pic = NULL;
			}

		}
	}
	else
	{
		CString type = lpTreeType;
		CString artname = type;


		if (rules.sections[type].values.find("Image") != rules.sections[type].values.end())
		{
			artname = rules.sections[type].values["Image"];
		}
		if (ini.sections.find(type) != ini.sections.end())
		{
			if (ini.sections[type].values.find("Image") != ini.sections[type].values.end())
			{
				artname = ini.sections[type].values["Image"];
			}
		}

		int w, h;
		char d[6];
		memcpy(d, art.sections[artname].values["Foundation"], 1);
		d[1] = 0;
		w = atoi(d);
		if (w == 0) w = 1;
		memcpy(d, (LPCTSTR)art.sections[artname].values["Foundation"] + 2, 1);
		d[1] = 0;
		h = atoi(d);
		if (h == 0) h = 1;
		int n = Map->GetUnitTypeID(type);

		if (n >= 0 && n < 0x0F00)
		{
			treeinfo[n].w = w;
			treeinfo[n].h = h;

			CString lpPicFile = GetUnitPictureFilename(type, 0);
			if (pics.find(lpPicFile) != pics.end())
			{

				treeinfo[n].pic = pics[lpPicFile];
			}
			else
				treeinfo[n].pic.pic = NULL;
		}

	}

}

int CMapData::GetBuildingID(LPCSTR lpBuildingName)
{
	if (buildingid.find(lpBuildingName) == buildingid.end()) return -1;
	return buildingid[lpBuildingName];
}



MAPFIELDDATA* CMapData::GetMappackPointer(DWORD dwPos)
{
	int x = dwPos % GetIsoSize(); // note that x=y and y=x in the mappack (according to our struct)!
	int y = dwPos / GetIsoSize();

	int i = 0;
	for (i = 0;i < dwIsoMapSize;i++)
	{
		MAPFIELDDATA* cur = (MAPFIELDDATA*)&m_mfd[i * MAPFIELDDATA_SIZE];
		if (cur->wX == y && cur->wY == x)
			return cur;
	}

	return NULL;
}


void CMapData::CreateMap(DWORD dwWidth, DWORD dwHeight, LPCTSTR lpTerrainType, DWORD dwGroundHeight)
{
	if (fielddata != NULL) delete[] fielddata;
	int i;
	for (i = 0;i < dwSnapShotCount;i++)
	{
		delete[] m_snapshots[i].bHeight;
		delete[] m_snapshots[i].bMapData;
		delete[] m_snapshots[i].bSubTile;
		delete[] m_snapshots[i].bMapData2;
		delete[] m_snapshots[i].wGround;
		delete[] m_snapshots[i].bRedrawTerrain;
		delete[] m_snapshots[i].overlay;
		delete[] m_snapshots[i].overlaydata;
		// m_snapshots[i].mapfile.Clear();
	}
	if (m_snapshots != NULL) delete[] m_snapshots;





	fielddata = NULL;
	fielddata_size = 0;
	m_snapshots = NULL;
	dwSnapShotCount = 0;
	m_cursnapshot = -1;


	m_tubes.clear();
	m_tubes.reserve(32);

	m_infantry.clear();
	m_terrain.clear();
	m_units.clear();
	m_structures.clear();
#ifdef SMUDGE_SUPP
	m_smudges.clear();
#endif


	m_mapfile.Clear();

	CString stdMap;
	stdMap = AppPath;
#ifdef TS_MODE
	stdMap += "\\StdMapTS.ini";
#endif
#ifdef RA2_MODE
	stdMap += "\\StdMapRA2.ini";
#endif

	m_mapfile.InsertFile(stdMap, NULL);

	char c[50];
	CString mapsize;
	itoa(dwWidth, c, 10);
	mapsize = "0,0,";
	mapsize += c;
	itoa(dwHeight, c, 10);
	mapsize += ",";
	mapsize += c;

	m_mapfile.sections["Map"].values["Size"] = mapsize;

	itoa(dwWidth - 4, c, 10);
	mapsize = "2,4,";
	mapsize += c;
	itoa(dwHeight - 6, c, 10);
	mapsize += ",";
	mapsize += c;

	m_mapfile.sections["Map"].values["Theater"] = lpTerrainType;
	m_mapfile.sections["Map"].values["LocalSize"] = mapsize;

	map<CString, PICDATA>::iterator it = pics.begin();
	for (int e = 0;e < pics.size();e++)
	{
		try
		{
#ifdef NOSURFACES_OBJECTS			
			if (it->second.bType == PICDATA_TYPE_BMP)
			{
				if (it->second.pic != NULL)
				{
					((LPDIRECTDRAWSURFACE4)it->second.pic)->Release();
				}
			}
			else
			{
				if (it->second.pic != NULL)
				{
					delete[] it->second.pic;
				}
				if (it->second.vborder) delete[] it->second.vborder;
			}
#else
			if (it->second.pic != NULL) it->second.pic->Release();
#endif

			it->second.pic = NULL;
		}
		catch (...)
		{
			CString err;
			err = "Access violation while trying to release surface ";
			char c[6];
			itoa(e, c, 10);
			err += c;

			err += "\n";
			OutputDebugString(err);
			continue;
		}

		it++;
	}

	std::unique_ptr<CDynamicGraphDlg> dlg;
	if (theApp.m_pMainWnd)
	{
		dlg.reset(new CDynamicGraphDlg(theApp.m_pMainWnd));
		dlg->ShowWindow(SW_SHOW);
		dlg->UpdateWindow();
	}

	pics.clear();
	missingimages.clear();

	UpdateBuildingInfo();
	UpdateTreeInfo();

	if (theApp.m_loading)
	{
		theApp.m_loading->Unload();
		theApp.m_loading->InitMixFiles();

		if (theApp.m_pMainWnd)
			((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->UpdateOverlayPictures();

		theApp.m_loading->InitPics();

		if (m_mapfile.sections["Map"].values["Theater"] == THEATER0)
		{
			tiledata = &s_tiledata;
			tiledata_count = &s_tiledata_count;
			tiles = &tiles_s;
			theApp.m_loading->FreeTileSet();
			tiledata = &u_tiledata;
			tiledata_count = &u_tiledata_count;
			tiles = &tiles_u;
			theApp.m_loading->FreeTileSet();

			// MW new tilesets
			tiledata = &un_tiledata;
			tiledata_count = &un_tiledata_count;
			tiles = &tiles_un;
			theApp.m_loading->FreeTileSet();
			tiledata = &d_tiledata;
			tiledata_count = &d_tiledata_count;
			tiles = &tiles_d;
			theApp.m_loading->FreeTileSet();
			tiledata = &l_tiledata;
			tiledata_count = &l_tiledata_count;
			tiles = &tiles_l;
			theApp.m_loading->FreeTileSet();

			tiledata = &t_tiledata;
			tiledata_count = &t_tiledata_count;
			tiles = &tiles_t;
			theApp.m_loading->FreeTileSet();
			if (dlg)
				theApp.m_loading->InitTMPs(&dlg->m_Progress);
			theApp.m_loading->cur_theat = 'T';

		}
		else if (m_mapfile.sections["Map"].values["Theater"] == THEATER1)
		{
			tiledata = &t_tiledata;
			tiledata_count = &t_tiledata_count;
			tiles = &tiles_t;
			theApp.m_loading->FreeTileSet();
			tiledata = &u_tiledata;
			tiledata_count = &u_tiledata_count;
			tiles = &tiles_u;
			theApp.m_loading->FreeTileSet();

			// MW new tilesets
			tiledata = &un_tiledata;
			tiledata_count = &un_tiledata_count;
			tiles = &tiles_un;
			theApp.m_loading->FreeTileSet();
			tiledata = &d_tiledata;
			tiledata_count = &d_tiledata_count;
			tiles = &tiles_d;
			theApp.m_loading->FreeTileSet();
			tiledata = &l_tiledata;
			tiledata_count = &l_tiledata_count;
			tiles = &tiles_l;
			theApp.m_loading->FreeTileSet();

			tiledata = &s_tiledata;
			tiledata_count = &s_tiledata_count;
			tiles = &tiles_s;
			theApp.m_loading->FreeTileSet();
			if (dlg)
				theApp.m_loading->InitTMPs(&dlg->m_Progress);
			theApp.m_loading->cur_theat = 'A';
		}
		else if (m_mapfile.sections["Map"].values["Theater"] == THEATER2)
		{
			tiledata = &t_tiledata;
			tiledata_count = &t_tiledata_count;
			tiles = &tiles_t;
			theApp.m_loading->FreeTileSet();
			tiledata = &s_tiledata;
			tiledata_count = &s_tiledata_count;
			tiles = &tiles_s;
			theApp.m_loading->FreeTileSet();

			// MW new tilesets
			tiledata = &un_tiledata;
			tiledata_count = &un_tiledata_count;
			tiles = &tiles_un;
			theApp.m_loading->FreeTileSet();
			tiledata = &d_tiledata;
			tiledata_count = &d_tiledata_count;
			tiles = &tiles_d;
			theApp.m_loading->FreeTileSet();
			tiledata = &l_tiledata;
			tiledata_count = &l_tiledata_count;
			tiles = &tiles_l;
			theApp.m_loading->FreeTileSet();

			tiledata = &u_tiledata;
			tiledata_count = &u_tiledata_count;
			tiles = &tiles_u;
			theApp.m_loading->FreeTileSet();
			if (dlg)
				theApp.m_loading->InitTMPs(&dlg->m_Progress);
			theApp.m_loading->cur_theat = 'U';
		}
		else if (yuri_mode && m_mapfile.sections["Map"].values["Theater"] == THEATER3)
		{
			tiledata = &t_tiledata;
			tiledata_count = &t_tiledata_count;
			tiles = &tiles_t;
			theApp.m_loading->FreeTileSet();
			tiledata = &s_tiledata;
			tiledata_count = &s_tiledata_count;
			tiles = &tiles_s;
			theApp.m_loading->FreeTileSet();

			// MW new tilesets

			tiledata = &d_tiledata;
			tiledata_count = &d_tiledata_count;
			tiles = &tiles_d;
			theApp.m_loading->FreeTileSet();
			tiledata = &l_tiledata;
			tiledata_count = &l_tiledata_count;
			tiles = &tiles_l;
			theApp.m_loading->FreeTileSet();

			tiledata = &u_tiledata;
			tiledata_count = &u_tiledata_count;
			tiles = &tiles_u;
			theApp.m_loading->FreeTileSet();

			tiledata = &un_tiledata;
			tiledata_count = &un_tiledata_count;
			tiles = &tiles_un;
			theApp.m_loading->FreeTileSet();

			if (dlg)
				theApp.m_loading->InitTMPs(&dlg->m_Progress);
			theApp.m_loading->cur_theat = 'N';
		}
		else if (yuri_mode && m_mapfile.sections["Map"].values["Theater"] == THEATER4)
		{
			tiledata = &t_tiledata;
			tiledata_count = &t_tiledata_count;
			tiles = &tiles_t;
			theApp.m_loading->FreeTileSet();
			tiledata = &s_tiledata;
			tiledata_count = &s_tiledata_count;
			tiles = &tiles_s;
			theApp.m_loading->FreeTileSet();

			// MW new tilesets
			tiledata = &un_tiledata;
			tiledata_count = &un_tiledata_count;
			tiles = &tiles_un;
			theApp.m_loading->FreeTileSet();
			tiledata = &d_tiledata;
			tiledata_count = &d_tiledata_count;
			tiles = &tiles_d;
			theApp.m_loading->FreeTileSet();


			tiledata = &u_tiledata;
			tiledata_count = &u_tiledata_count;
			tiles = &tiles_u;
			theApp.m_loading->FreeTileSet();

			tiledata = &l_tiledata;
			tiledata_count = &l_tiledata_count;
			tiles = &tiles_l;
			theApp.m_loading->FreeTileSet();

			if (dlg)
			{
				theApp.m_loading->InitTMPs(&dlg->m_Progress);
			}
			theApp.m_loading->cur_theat = 'L';
		}
		else if (m_mapfile.sections["Map"].values["Theater"] == THEATER5)
		{
			tiledata = &t_tiledata;
			tiledata_count = &t_tiledata_count;
			tiles = &tiles_t;
			theApp.m_loading->FreeTileSet();
			tiledata = &s_tiledata;
			tiledata_count = &s_tiledata_count;
			tiles = &tiles_s;
			theApp.m_loading->FreeTileSet();

			// MW new tilesets
			tiledata = &un_tiledata;
			tiledata_count = &un_tiledata_count;
			tiles = &tiles_un;
			theApp.m_loading->FreeTileSet();

			tiledata = &l_tiledata;
			tiledata_count = &l_tiledata_count;
			tiles = &tiles_l;
			theApp.m_loading->FreeTileSet();


			tiledata = &u_tiledata;
			tiledata_count = &u_tiledata_count;
			tiles = &tiles_u;
			theApp.m_loading->FreeTileSet();

			tiledata = &d_tiledata;
			tiledata_count = &d_tiledata_count;
			tiles = &tiles_d;
			theApp.m_loading->FreeTileSet();

			if (dlg)
				theApp.m_loading->InitTMPs(&dlg->m_Progress);
			theApp.m_loading->cur_theat = 'D';
		}
		else
		{
			theApp.m_loading->FreeAll();
			CString s = "Fatal error! %9 doesn´t support the theater of this map!";
			s = TranslateStringACP(s);
			MessageBox(0, s, "Error", 0);
			exit(0);
		}
	}
	else
	{
		// e.g. unittests
		tiles = &tiles_t;
	}


	errstream << "CreateMap() clears data\n";
	errstream.flush();

	CalcMapRect();
	ClearOverlay();
	ClearOverlayData();

	isInitialized = TRUE;
	if (fielddata != NULL) delete[] fielddata;

	errstream << "CreateMap() allocates memory\n";
	errstream.flush();
	fielddata = new(FIELDDATA[(GetIsoSize() + 1) * (GetIsoSize() + 1)]); // +1 because of some unpacking problems
	fielddata_size = (GetIsoSize() + 1) * (GetIsoSize() + 1);
	dwIsoMapSize = 0;

	errstream << "CreateMap() frees m_mfd\n";
	errstream.flush();
	if (m_mfd != NULL) delete[] m_mfd;
	m_mfd = NULL;

	errstream << "CreateMap() loads from ini\n";
	errstream.flush();
	UpdateIniFile(MAPDATA_UPDATE_FROM_INI);

	AddWaypoint("98", (dwWidth + dwHeight) / 2 + (dwWidth + dwHeight) * (dwWidth + dwHeight) / 2);
	AddWaypoint("99", (dwWidth + dwHeight) / 2 + (dwWidth + dwHeight) * (dwWidth + dwHeight) / 2 - 1);

	errstream << "CreateMap() sets terrain height\n";
	errstream.flush();


	for (i = 0;i < fielddata_size;i++)
	{
		fielddata[i].bHeight = dwGroundHeight;
	}

	errstream << "CreateMap() finished\n";
	errstream.flush();

}

/*void CMapData::CreateSlopesAt(DWORD dwPos)
{
	//OutputDebugString("CreateSlopes()\n");

	FIELDDATA m=*GetFielddataAt(dwPos);
	if(m.wGround==0xFFFF) m.wGround=0;

	TILEDATA& d=(*tiledata)[m.wGround];

	int ns=-1;
	int i,e,p=0;
	int h[3][3];
	for(i=0;i<3; i++)
	{
		for(e=0;e<3;e++)
		{
			int pos=dwPos+(i-1)+(e-1)*m_IsoSize;
			if(pos<0 || pos>=m_IsoSize*m_IsoSize)
			{
				h[i][e]=0;
			}
			else
			{
				FIELDDATA m2=*GetFielddataAt(pos);

				h[i][e]=m.bHeight-m2.bHeight;
			}


		}
	}

	// hmm... check if the current tile must be heightened anyway
	if(!theApp.m_Options.bDisableSlopeCorrection && d.bMorphable)
	{
		if((h[0][1]<0 && h[2][1]<0) || (h[1][0]<0 && h[1][2]<0)

			|| (h[1][0]<0 && h[0][2]<0 && h[0][1]>=0)
			|| (h[1][0]<0 && h[2][2]<0 && h[2][1]>=0)

			|| (h[0][1]<0 && h[2][0]<0 && h[1][0]>=0)
			|| (h[0][1]<0 && h[2][2]<0 && h[1][2]>=0)

			|| (h[1][2]<0 && h[0][0]<0 && h[0][1]>=0)
			|| (h[1][2]<0 && h[2][0]<0 && h[2][1]>=0)

			|| (h[2][1]<0 && h[0][0]<0 && h[1][0]>=0)
			|| (h[2][1]<0 && h[0][2]<0 && h[1][2]>=0)

			|| (h[1][0]<0 && h[0][1]<0 && h[0][0]>=0)
			|| (h[0][1]<0 && h[1][2]<0 && h[0][2]>=0)
			|| (h[1][2]<0 && h[2][1]<0 && h[2][2]>=0)
			|| (h[2][1]<0 && h[1][0]<0 && h[2][0]>=0)

			)
		{
			SetHeightAt(dwPos, m.bHeight+1);
			for(i=-1;i<2;i++)
				for(e=-1;e<2;e++)
					CreateSlopesAt(dwPos+i+e*m_IsoSize);

			return;
		}
	}

	//OutputDebugString("Running check\n");

	BOOL checkOtherSlopes=FALSE;
	/*if(h[0][0] && h[0][1] && h[0][2] && h[1][0] && h[1][2] && h[2][0] && h[2][1] && h[2][2])
	{
		ns=-1;
		checkOtherSlopes=TRUE;
	}*//*

	if(h[0][0]==-1 && h[2][2]==-1 && h[2][0]>=0 && h[0][2]>=0 && h[1][0]>=0 && h[1][2]>=0 && h[0][1]>=0 && h[2][1]>=0) ns=SLOPE_UP_LEFTTOP_AND_RIGHTBOTTOM;
	if(h[0][2]==-1 && h[2][0]==-1 && h[0][0]>=0 && h[2][2]>=0 && h[0][1]>=0 && h[1][0]>=0 && h[1][2]>=0 && h[2][1]>=0) ns=SLOPE_UP_LEFTBOTTOM_AND_RIGHTTOP;

	// that would just be another solution:
	// if(h[0][0]==-1 && h[2][2]==-1 && h[2][0]>=0 && h[0][2]>=0 && h[1][0]>=0 && h[1][2]>=0 && h[0][1]>=0 && h[2][1]>=0) ns=SLOPE_UP_LEFTTOP_AND_RIGHTBOTTOM2;
	// if(h[0][2]==-1 && h[2][0]==-1 && h[0][0]>=0 && h[2][2]>=0 && h[0][1]>=0 && h[1][0]>=0 && h[1][2]>=0 && h[2][1]>=0) ns=SLOPE_UP_LEFTBOTTOM_AND_RIGHTTOP2;



	if(ns==-1)
	if(h[1][0]==-1 && h[0][1]!=-1 && h[1][2]!=-1 && h[2][1]!=-1)
	{
		ns=SLOPE_UP_LEFT;
	}
	else if(h[0][1]==-1 && h[1][0]!=-1 && h[2][1]!=-1 && h[1][2]!=-1)
	{
		ns=SLOPE_UP_TOP;
	}
	else if(h[1][2]==-1 && h[0][1]!=-1 && h[1][0]!=-1 && h[2][1]!=-1)
	{
		ns=SLOPE_UP_RIGHT;
	}
	else if(h[2][1]==-1 && h[0][1]!=-1 && h[1][0]!=-1 && h[1][2]!=-1)
	{
		ns=SLOPE_UP_BOTTOM;
	}

	if(ns==-1)
	{
		if(h[0][0]==-2) ns=SLOPE_DOWN_BOTTOM;
		if(h[2][0]==-2) ns=SLOPE_DOWN_RIGHT;
		if(h[0][2]==-2) ns=SLOPE_DOWN_LEFT;
		if(h[2][2]==-2) ns=SLOPE_DOWN_TOP;
	}

	if(ns==-1 && h[0][0]==-1)
	{
		if(h[1][0]==-1 && h[0][1]==-1 ) ns=SLOPE_DOWN_RIGHTBOTTOM;
		else if(h[1][0]==0 && h[0][1]==0) ns=SLOPE_UP_LEFTTOP;
		//else if(h[2][2]==1) ns=SLOPE_DOWN_BOTTOM;
	}

	if( ns==-1 && h[2][0]==-1)
	{
		if(h[1][0]==-1 && h[2][1]==-1 ) ns=SLOPE_DOWN_RIGHTTOP;
		else if(h[1][0]==0 && h[2][1]==0) ns=SLOPE_UP_LEFTBOTTOM;
		//else if(h[0][2]==1) ns=SLOPE_DOWN_RIGHT;
	}
	if( ns==-1 && h[0][2]==-1)
	{
		if(h[1][2]==-1 && h[0][1]==-1 ) ns=SLOPE_DOWN_LEFTBOTTOM;
		else if(h[1][2]==0 && h[0][1]==0) ns=SLOPE_UP_RIGHTTOP;
		//else if(h[2][0]==1) ns=SLOPE_DOWN_LEFT;
	}
	if( ns==-1 && h[2][2]==-1)
	{
		if(h[1][2]==-1 && h[2][1]==-1 ) ns=SLOPE_DOWN_LEFTTOP;
		else if(h[1][2]==0 && h[2][1]==0) ns=SLOPE_UP_RIGHTBOTTOM;
		//else if(h[0][0]==1) ns=SLOPE_DOWN_TOP;
	}

	if(ns==-1 && h[1][0]==-1 && h[2][1]==-1 ) ns=SLOPE_DOWN_RIGHTTOP;
	if(ns==-1 && h[1][2]==-1 && h[2][1]==-1 ) ns=SLOPE_DOWN_LEFTTOP;
	if(ns==-1 && h[1][0]==-1 && h[0][1]==-1 ) ns=SLOPE_DOWN_RIGHTBOTTOM;
	if(ns==-1 && h[1][2]==-1 && h[0][1]==-1 ) ns=SLOPE_DOWN_LEFTBOTTOM;


	//OutputDebugString("Applying changes()\n");



	int rampbase=atoi((*tiles).sections["General"].values["RampBase"]);
	int rampsmooth=atoi((*tiles).sections["General"].values["RampSmooth"]);

	//if((rampbase==d.wTileSet || rampsmooth==d.wTileSet) && m.bSubTile==ns)
	//	return;

	if(ns==-1 && (d.wTileSet==rampbase || d.wTileSet==rampsmooth) && d.bMorphable)
	{
		SetTileAt(dwPos, 0, 0);
	}
	if(d.bMorphable && ns!=-1)
	{
		SetTileAt(dwPos, GetTileID(rampbase, ns-1), 0);

	}



	/*if(ns!=-1 || checkOtherSlopes)
	{
		for(i=-1;i<2;i++)
		{
			for(e=-1;e<2;e++)
			{
				int pos=dwPos+(i)+(e)*m_IsoSize;
				if(pos>0 && pos<m_IsoSize*m_IsoSize)
				{
					CreateSlopesAt(pos);
				}
			}
		}
	}*//*

}*/

int CMapData::GetNecessarySlope(DWORD dwPos)
{


	return 0;
}

DWORD CMapData::GetTileID(DWORD dwTileSet, int iTile)
{
	int i, e;
	DWORD tilecount = 0;
	for (i = 0;i < 10000;i++)
	{
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for (e = 0;e < 4 - strlen(c);e++)
			tset += "0";
		tset += c;
		CString sec = "TileSet";
		sec += tset;

		if (tiles->sections.find(sec) == tiles->sections.end()) return 0xFFFFFFFF;


		for (e = 0;e < atoi(tiles->sections[sec].values["TilesInSet"]);e++)
		{
			if (i == dwTileSet && e == iTile) return tilecount;
			tilecount++;

		}


	}

	return tilecount;

}

void CMapData::SetReserved(DWORD dwPos, BYTE val)
{
	fielddata[dwPos].bReserved = val;
}



void CMapData::HideField(DWORD dwPos, BOOL bHide)
{
	fielddata[dwPos].bHide = bHide;
}



void CMapData::UpdateTubes(BOOL bSave)
{
	if (!bSave)
	{
		m_tubes.clear();
		if (m_mapfile.sections.find("Tubes") == m_mapfile.sections.end())
			return;

		auto& tubeSection = m_mapfile.sections["Tubes"];

		std::uint16_t secSize = tubeSection.values.size();
		m_tubes.reserve(secSize + 10);

		for (auto& [sTubeId, value] : tubeSection)
		{
			const auto tubeId = std::atoi(sTubeId);
			if (tubeId < 0 || tubeId >= std::numeric_limits<std::uint16_t>::max())
				continue;
			m_tubes.push_back(std::make_unique<CTube>(static_cast<std::uint16_t>(tubeId), value.GetString()));
		}
	}
	else
	{

	}

}



void CMapData::SetTube(CTube* lpTI)
{
	CString sTubeId;
	if (lpTI->hasId())
		sTubeId = std::to_string(lpTI->getId()).c_str();
	else
	{
		for (std::uint16_t i = 0;i < 10000;i++)
		{
			sTubeId = std::to_string(i).c_str();
			if (m_mapfile.sections["Tubes"].values.find(sTubeId) == m_mapfile.sections["Tubes"].values.end())
			{
				lpTI->setId(i);
				break;
			}
		}
	}

	if (!lpTI->hasId())
		return;

	m_mapfile.sections["Tubes"].values[sTubeId] = lpTI->toString().c_str();
	UpdateTubes(FALSE);
}

CTube* CMapData::GetTube(std::uint16_t wID)
{
	auto it = std::find_if(m_tubes.begin(), m_tubes.end(), [wID](const auto& el) {return el->getId() == wID;});
	if (it == m_tubes.end())
		return nullptr;

	return it->get();
}


void CMapData::DeleteTube(std::uint16_t wID)
{
	if (m_mapfile.sections.find("Tubes") == m_mapfile.sections.end())
		return;

	auto& tubeSection = m_mapfile.sections["Tubes"];
	const CString sTubeId = std::to_string(wID).c_str();

	std::erase_if(tubeSection.values, [&sTubeId](const auto& el) {return el.first == sTubeId;});

	//CString id1 = *m_mapfile.sections["Tubes"].GetValueName(wID);

	/*CTube i1 = m_tubes[wID];

	vector<CString> toErase;
	toErase.reserve(10);

	// we delete every tube with same starting coordinates or when starting coordinate of i1 is ending of curi
	int i;
	for (i = 0;i < m_tubes.size();i++)
	{
		CTube& curi = m_tubes[i];
		if ((curi.getEndX() == i1.getStartX() && curi.getEndY() == i1.getStartY()) || (curi.getStartX() == i1.getStartX() && curi.getStartY() == i1.getStartY()))
		{
			toErase.push_back(*m_mapfile.sections["Tubes"].GetValueName(i));
		}
	}

	for (i = 0;i < toErase.size();i++)
	{
		m_mapfile.sections["Tubes"].values.erase(toErase[i]);
	}*/

	UpdateTubes(FALSE);
}

int CMapData::GetInfantryCountAt(DWORD dwPos)
{
	int i;
	int sc = 0;
	for (i = 0;i < SUBPOS_COUNT;i++)
	{
		if (fielddata[dwPos].infantry[i] > -1) sc++;
	}

	return sc;
}

/*
Checks if the map data is valid
May fail if the user changed the tileset manually,
or if loading maps made with modified tilesets
*/
BOOL CMapData::CheckMapPackData()
{
	int i;
	for (i = 0;i < fielddata_size;i++)
	{
		int gr = fielddata[i].wGround;
		if (gr != 0xFFFF && gr >= (*tiledata_count))
			return FALSE;
		if (gr == 0xFFFF) gr = 0;
		if ((*tiledata)[gr].wTileCount <= fielddata[i].bSubTile) return FALSE;
	}

	return TRUE;
}

/*
Takes a snapshot of the map at a certain location.
Be aware that this won´t make a copy of any units etc.

This is used for undo and similar things (like displaying and immediatly removing tiles when moving
the mouse on the map before placing a tile).
This method is very fast, as long as you don´t copy the whole map all the time.
*/
void CMapData::TakeSnapshot(BOOL bEraseFollowing, int left, int top, int right, int bottom)
{
	DWORD dwOldSnapShotCount = dwSnapShotCount;

	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right > m_IsoSize) right = m_IsoSize;
	if (bottom > m_IsoSize) bottom = m_IsoSize;

	if (right == 0) right = m_IsoSize;
	if (bottom == 0) bottom = m_IsoSize;

	int e;
	if (bEraseFollowing)
	{
		for (e = dwSnapShotCount - 1;e > m_cursnapshot;e--)
		{
			delete[] m_snapshots[e].bHeight;
			delete[] m_snapshots[e].bMapData;
			delete[] m_snapshots[e].bSubTile;
			delete[] m_snapshots[e].bMapData2;
			delete[] m_snapshots[e].overlay;
			delete[] m_snapshots[e].overlaydata;
			delete[] m_snapshots[e].wGround;
			delete[] m_snapshots[e].bRedrawTerrain;
			delete[] m_snapshots[e].bRNDData;
			// m_snapshots[0].mapfile.Clear();
		}
		dwSnapShotCount = m_cursnapshot + 1;
	}


	dwSnapShotCount += 1;
	m_cursnapshot++;

	if (dwSnapShotCount > 64)
	{
		dwSnapShotCount = 64;
		m_cursnapshot = 63;
		int i;
		delete[] m_snapshots[0].bHeight;
		delete[] m_snapshots[0].bMapData;
		delete[] m_snapshots[0].bSubTile;
		delete[] m_snapshots[0].bMapData2;
		delete[] m_snapshots[0].overlay;
		delete[] m_snapshots[0].overlaydata;
		delete[] m_snapshots[0].wGround;
		delete[] m_snapshots[0].bRedrawTerrain;
		delete[] m_snapshots[0].bRNDData;
		// m_snapshots[0].mapfile.Clear();
		for (i = 1;i < dwSnapShotCount;i++)
		{
			m_snapshots[i - 1] = m_snapshots[i];
		}

	}
	else
	{
		SNAPSHOTDATA* b = new(SNAPSHOTDATA[dwSnapShotCount]);

		if (m_snapshots)
		{
			memcpy(b, m_snapshots, sizeof(SNAPSHOTDATA) * (dwSnapShotCount - 1));
			delete[] m_snapshots;
		}

		m_snapshots = b;
	}


	m_cursnapshot = dwSnapShotCount - 1;


	SNAPSHOTDATA ss = m_snapshots[dwSnapShotCount - 1];
	// ss.mapfile=m_mapfile;
	int width, height;
	width = right - left;
	height = bottom - top;

	int size = width * height;
	ss.left = left;
	ss.top = top;
	ss.right = right;
	ss.bottom = bottom;
	ss.bHeight = new(BYTE[size]);
	ss.bMapData = new(WORD[size]);
	ss.bSubTile = new(BYTE[size]);
	ss.bMapData2 = new(BYTE[size]);
	ss.wGround = new(WORD[size]);
	ss.overlay = new(BYTE[size]);
	ss.overlaydata = new(BYTE[size]);
	ss.bRedrawTerrain = new(BOOL[size]);
	ss.bRNDData = new(BYTE[size]);
	int i;
	for (i = 0;i < width;i++)
	{
		for (e = 0;e < height;e++)
		{
			int pos_w, pos_r;
			pos_w = i + e * width;
			pos_r = left + i + (top + e) * m_IsoSize;
			ss.bHeight[pos_w] = fielddata[pos_r].bHeight;
			ss.bMapData[pos_w] = fielddata[pos_r].bMapData;
			ss.bSubTile[pos_w] = fielddata[pos_r].bSubTile;
			ss.bMapData2[pos_w] = fielddata[pos_r].bMapData2;
			ss.wGround[pos_w] = fielddata[pos_r].wGround;
			ss.overlay[pos_w] = fielddata[pos_r].overlay;
			ss.overlaydata[pos_w] = fielddata[pos_r].overlaydata;
			ss.bRedrawTerrain[pos_w] = fielddata[pos_r].bRedrawTerrain;
			ss.bRNDData[pos_w] = fielddata[pos_r].bRNDImage;
		}
	}

	m_snapshots[dwSnapShotCount - 1] = ss;

}

/*
Just uses the last SnapShot to reverse changes on the map.
Very fast
*/
void CMapData::Undo()
{
	if (dwSnapShotCount == 0) return;
	if (m_cursnapshot < 0) return;


	//dwSnapShotCount--;
	m_cursnapshot -= 1;

	int left, top, width, height;
	left = m_snapshots[m_cursnapshot + 1].left;
	top = m_snapshots[m_cursnapshot + 1].top;
	width = m_snapshots[m_cursnapshot + 1].right - left;
	height = m_snapshots[m_cursnapshot + 1].bottom - top;

	const bool mp = IsMultiplayer();
	int i, e;
	for (i = 0;i < width;i++)
	{
		for (e = 0;e < height;e++)
		{
			int pos_w, pos_r;
			pos_r = i + e * width;
			pos_w = left + i + (top + e) * m_IsoSize;
			fielddata[pos_w].bHeight = m_snapshots[m_cursnapshot + 1].bHeight[pos_r];
			fielddata[pos_w].bMapData = m_snapshots[m_cursnapshot + 1].bMapData[pos_r];
			fielddata[pos_w].bSubTile = m_snapshots[m_cursnapshot + 1].bSubTile[pos_r];
			fielddata[pos_w].bMapData2 = m_snapshots[m_cursnapshot + 1].bMapData2[pos_r];
			fielddata[pos_w].wGround = m_snapshots[m_cursnapshot + 1].wGround[pos_r];

			RemoveOvrlMoney(fielddata[pos_w].overlay, fielddata[pos_w].overlaydata);
			fielddata[pos_w].overlay = m_snapshots[m_cursnapshot + 1].overlay[pos_r];
			fielddata[pos_w].overlaydata = m_snapshots[m_cursnapshot + 1].overlaydata[pos_r];
			AddOvrlMoney(fielddata[pos_w].overlay, fielddata[pos_w].overlaydata);



			fielddata[pos_w].bRedrawTerrain = m_snapshots[m_cursnapshot + 1].bRedrawTerrain[pos_r];
			fielddata[pos_w].bRNDImage = m_snapshots[m_cursnapshot + 1].bRNDData[pos_r];


			Mini_UpdatePos(left + i, top + e, mp);
		}
	}

	// no need for SmoothTiberium: handled externally, because we undo more than just the very needed area
	// when changing overlay!



}

BOOL CMapData::GetLocalSize(RECT* rect) const
{
	if (!isInitialized) return FALSE;

	*rect = m_vismaprect;

	return TRUE;
}



/*
Opposite of Undo(). If possible, redoes the changes.
Very fast.
*/
void CMapData::Redo()
{
	if (dwSnapShotCount <= m_cursnapshot + 1 || !dwSnapShotCount) return;

	m_cursnapshot += 1; // dwSnapShotCount-1;

	if (m_cursnapshot + 1 >= dwSnapShotCount) m_cursnapshot = dwSnapShotCount - 2;

	int left, top, width, height;
	left = m_snapshots[m_cursnapshot + 1].left;
	top = m_snapshots[m_cursnapshot + 1].top;
	width = m_snapshots[m_cursnapshot + 1].right - left;
	height = m_snapshots[m_cursnapshot + 1].bottom - top;

	int i, e;
	const bool mp = IsMultiplayer();
	for (i = 0;i < width;i++)
	{
		for (e = 0;e < height;e++)
		{
			int pos_w, pos_r;
			pos_r = i + e * width;
			pos_w = left + i + (top + e) * m_IsoSize;
			fielddata[pos_w].bHeight = m_snapshots[m_cursnapshot + 1].bHeight[pos_r];
			fielddata[pos_w].bMapData = m_snapshots[m_cursnapshot + 1].bMapData[pos_r];
			fielddata[pos_w].bSubTile = m_snapshots[m_cursnapshot + 1].bSubTile[pos_r];
			fielddata[pos_w].bMapData2 = m_snapshots[m_cursnapshot + 1].bMapData2[pos_r];
			fielddata[pos_w].wGround = m_snapshots[m_cursnapshot + 1].wGround[pos_r];
			RemoveOvrlMoney(fielddata[pos_w].overlay, fielddata[pos_w].overlaydata);
			fielddata[pos_w].overlay = m_snapshots[m_cursnapshot + 1].overlay[pos_r];
			fielddata[pos_w].overlaydata = m_snapshots[m_cursnapshot + 1].overlaydata[pos_r];
			AddOvrlMoney(fielddata[pos_w].overlay, fielddata[pos_w].overlaydata);
			fielddata[pos_w].bRedrawTerrain = m_snapshots[m_cursnapshot + 1].bRedrawTerrain[pos_r];
			fielddata[pos_w].bRNDImage = m_snapshots[m_cursnapshot + 1].bRNDData[pos_r];

			Mini_UpdatePos(left + i, top + e, mp);
		}
	}

	/*
	int i;
	for(i=0;i<fielddata_size;i++)
	{
		fielddata[i].bHeight=m_snapshots[m_cursnapshot+1].bHeight[i];
		fielddata[i].bMapData=m_snapshots[m_cursnapshot+1].bMapData[i];
		fielddata[i].bSubTile=m_snapshots[m_cursnapshot+1].bSubTile[i];
		fielddata[i].bMapData2=m_snapshots[m_cursnapshot+1].bMapData2[i];
		fielddata[i].wGround=m_snapshots[m_cursnapshot+1].wGround[i];
		fielddata[i].overlay=m_snapshots[m_cursnapshot+1].overlay[i];
		fielddata[i].overlaydata=m_snapshots[m_cursnapshot+1].overlaydata[i];
		fielddata[i].bRedrawTerrain=m_snapshots[m_cursnapshot+1].bRedrawTerrain[i];
	}
	*/

}

static const int tile_to_lat_count = 7;
static const CString tile_to_lat[tile_to_lat_count][3] = {
	{"SandTile", "ClearToSandLat", "ClearTile"},
	{"GreenTile", "ClearToGreenLat", "ClearTile"},
	{"RoughTile", "ClearToRoughLat", "ClearTile"},
	{"PaveTile", "ClearToPaveLat", "ClearTile"},
	{"BlueMoldTile", "ClearToBlueMoldLat", ""}, //"ClearTile"}, // TODO: fix
	{"CrystalTile", "ClearToCrystalLat", "ClearTile"},
	{"SwampTile", "WaterToSwampLat", "WaterSet"}
};

bool CMapData::hasLat(WORD wGround) const
{
	if (wGround >= *tiledata_count)
		return false;
	const auto set = (*tiledata)[wGround].wTileSet;

	const auto& sec = tiles->sections["General"];
	const CString empty;

	for (int i = 0; i < tile_to_lat_count; ++i)
	{
		const int tile = atoi(sec.GetValueByName(tile_to_lat[i][0], empty));
		const int lat = atoi(sec.GetValueByName(tile_to_lat[i][1], empty));
		const int target_tile = atoi(sec.GetValueByName(tile_to_lat[i][2], empty));
		if (lat &&
			(set == tile ||
				set == lat ||
				set == target_tile))
			return true;
	}

	return false;
}

void CMapData::SmoothAllAt(DWORD dwPos)
{
	if (theApp.m_Options.bDisableAutoLat) return;

	if (dwPos > fielddata_size) return;

	int set = 0, ground = fielddata[dwPos].wGround;

	if (ground == 0xFFFF) ground = 0;
	set = (*tiledata)[ground].wTileSet;

	const auto& sec = tiles->sections["General"];
	const CString empty;

	for (int i = 0; i < tile_to_lat_count; ++i)
	{
		const int tile = atoi(sec.GetValueByName(tile_to_lat[i][0], empty));
		const int lat = atoi(sec.GetValueByName(tile_to_lat[i][1], empty));
		const int target_tile = atoi(sec.GetValueByName(tile_to_lat[i][2], empty));
		if (strlen(tile_to_lat[i][2]) &&
			lat &&
			(set == tile ||
				set == lat))
			SmoothAt(dwPos, tile, lat, target_tile);
	}
}

void CMapData::CreateShore(int left, int top, int right, int bottom, BOOL bRemoveUseless)
{


	int i;
	int isosize = Map->GetIsoSize();
	int mapsize = isosize * isosize;
	int mapwidth = Map->GetWidth();
	int mapheight = Map->GetHeight();
	// int shoreset=atoi((*tiles).sections["General"].values["ShorePieces"]);

	short* tsets = new(short[mapsize]);
	BYTE* terrain = new(BYTE[mapsize]);
	int* tile = new(int[mapsize]);
	BOOL* hasChanged = new(BOOL[isosize * isosize]);
	BOOL* noChange = new(BOOL[isosize * isosize]);
	//BOOL* replaced=new(BOOL[isosize*isosize]); // replaced by water<->ground

	memset(hasChanged, 0, sizeof(BOOL) * isosize * isosize);
	memset(noChange, 0, sizeof(BOOL) * isosize * isosize);
	//memset(replaced, 0, sizeof(BOOL)*isosize*isosize);

	int watercliffset = atoi((*tiles).sections["General"].values["WaterCliffs"]);
	int xx, yy;

	for (i = 0;i < *tiledata_count;i++)
	{
		if ((*tiledata)[i].wTileSet == waterset && (*tiledata)[i].cx == 1 && (*tiledata)[i].cy == 1) break;
	}

	int smallwater = i;




	last_succeeded_operation = 7002;


	map<int, int> softsets;
	CString sec = "SoftTileSets";

	for (i = 0;i < g_data.sections[sec].values.size();i++)
	{
		CString tset = *g_data.sections[sec].GetValueName(i);
		TruncSpace(tset);
		int p = (*tiles).sections["General"].FindName(tset);
		if (p < 0) continue;

		int set = atoi(*(*tiles).sections["General"].GetValue(p));
		if (atoi(*g_data.sections[sec].GetValue(i))) softsets[set] = 1;


	}




	last_succeeded_operation = 7001;

	// remove partial shore pieces (wrong ones)
	for (xx = left - 2;xx < right + 2;xx++)
	{
		for (yy = top - 2;yy < bottom + 2;yy++)
		{

			if (xx < 1 || yy < 1 || xx + yy<mapwidth + 1 || xx + yy>mapwidth + mapheight * 2 || (yy + 1 > mapwidth && xx - 1 < yy - mapwidth) || (xx + 1 > mapwidth && yy + mapwidth - 1 < xx)) continue;

			int pos = xx + yy * isosize;
			if (noChange[pos]) continue;

			FIELDDATA* fd = Map->GetFielddataAt(pos);
			int ground = fd->wGround;
			if (ground == 0xFFFF) ground = 0;
			TILEDATA& td = (*tiledata)[ground];
			if (td.wTileSet != shoreset) continue;

			// we have a shore piece here. check if it is set correct
			BOOL bCorrect = TRUE;

			int of = fd->bSubTile;

			int ox = of / td.cy;
			int oy = of % td.cy;


			int xxx, yyy;
			int p = 0;
			for (xxx = xx - ox;xxx < xx + td.cx - ox;xxx++)
			{
				for (yyy = yy - oy;yyy < yy + td.cy - oy;yyy++)
				{
					int pos = xxx + yyy * isosize;
					if (td.tiles[p].pic != NULL)
					{
						FIELDDATA* curf = Map->GetFielddataAt(pos);
						int curg = curf->wGround;
						if (curg == 0xFFFF) curg = 0;

						if (curg != ground || curf->bSubTile != p)
						{
							bCorrect = FALSE;
							break;
						}
					}
					p++;
				}
				if (!bCorrect) break;
			}

			if (!bCorrect)
			{
				int iWaterFound = 0;
				/*for(xxx=xx-1;xxx<=xx+1;xxx++)
				{
					for(yyy=yy-1;yyy<=yy+1;yyy++)
					{
						if(xxx==xx && yyy==yy) continue;

						int pos=xxx+yyy*isosize;
						FIELDDATA* curf=Map->GetFielddataAt(pos);
						int curg=curf->wGround;
						if(curg==0xFFFF) curg=0;

						if((*tiledata)[curg].tiles[curf->bSubTile].bHackedTerrainType==TERRAINTYPE_WATER)
							iWaterFound++;

					}
				}*/

				int pos = xx + yy * isosize;
				FIELDDATA* curf = Map->GetFielddataAt(pos);
				int curg = curf->wGround;
				if (curg == 0xFFFF) curg = 0;

				if ((*tiledata)[curg].tiles[curf->bSubTile].bHackedTerrainType == TERRAINTYPE_WATER) iWaterFound = 8;

				if (iWaterFound > 7)
				{
					for (i = 0;i < *tiledata_count;i++)
					{
						if ((*tiledata)[i].wTileSet == waterset && (*tiledata)[i].cx == 1 && (*tiledata)[i].cy == 1) break;
					}
					SetTileAt(xx + yy * isosize, i, 0);
					noChange[xx + yy * isosize] = FALSE;
					//replaced[xx+yy*isosize]=TRUE;
				}
				else
				{
					SetTileAt(xx + yy * isosize, 0, 0);
					noChange[xx + yy * isosize] = FALSE;
					//replaced[xx+yy*isosize]=TRUE;

				}
			}

		}
	}


	// remove too small water and ground pieces (NEW)
	if (bRemoveUseless)
	{
		for (xx = left;xx < right;xx++)
		{
			for (yy = top;yy < bottom;yy++)
			{

				if (xx < 1 || yy < 1 || xx + yy<mapwidth + 1 || xx + yy>mapwidth + mapheight * 2 || (yy + 1 > mapwidth && xx - 1 < yy - mapwidth) || (xx + 1 > mapwidth && yy + mapwidth - 1 < xx)) continue;

				int dwPos = xx + yy * isosize;

				//if(noChange[dwPos]) continue;

				FIELDDATA* fd = Map->GetFielddataAt(dwPos);
				int ground = fd->wGround;
				if (ground == 0xFFFF) ground = 0;
				TILEDATA& td = (*tiledata)[ground];

				if (softsets.find(td.wTileSet) == softsets.end()) continue;

				if (td.tiles[fd->bSubTile].bHackedTerrainType != TERRAINTYPE_WATER && td.tiles[fd->bSubTile].bHackedTerrainType != TERRAINTYPE_GROUND) continue;

				int ts[3][3];  // terrain info
				int i, e;
				for (i = 0;i < 3; i++)
				{
					for (e = 0;e < 3;e++)
					{
						int pos = dwPos + (i - 1) + (e - 1) * m_IsoSize;
						if (pos < 0 || pos >= fielddata_size)
						{
							ts[i][e] = 0;
						}
						else
						{
							FIELDDATA m2 = *GetFielddataAt(pos);
							if (m2.wGround == 0xFFFF) m2.wGround = 0;

							ts[i][e] = (*tiledata)[m2.wGround].tiles[m2.bSubTile].bHackedTerrainType;
						}
					}
				}


				if ((ts[1][0] != ts[1][1] && ts[1][2] != ts[1][1]) ||
					(ts[0][1] != ts[1][1] && ts[2][1] != ts[1][1]))
				{
					if (ts[1][1] == TERRAINTYPE_WATER)
					{
						SetTileAt(dwPos, 0, 0);
						//replaced[dwPos]=TRUE;
					}
					else if (ts[1][1] == TERRAINTYPE_GROUND)
					{
						if ((ts[1][0] == TERRAINTYPE_WATER && ts[1][2] == TERRAINTYPE_WATER) || (ts[0][1] == TERRAINTYPE_WATER && ts[2][1] == TERRAINTYPE_WATER))
						{
							SetTileAt(dwPos, smallwater, 0);
							//replaced[dwPos]=TRUE;
						}
					}
				}

			}
		}
	}


	last_succeeded_operation = 7003;

	// retrieve non-changeable fields
	for (xx = left;xx < right;xx++)
	{
		for (yy = top;yy < bottom;yy++)
		{
			if (xx < 1 || yy < 1 || xx + yy<mapwidth + 1 || xx + yy>mapwidth + mapheight * 2 || (yy + 1 > mapwidth && xx - 1 < yy - mapwidth) || (xx + 1 > mapwidth && yy + mapwidth - 1 < xx)) continue;

			int pos = xx + yy * isosize;
			FIELDDATA* fd = GetFielddataAt(pos);
			int ground = fd->wGround;
			if (ground == 0xFFFF) ground = 0;

			tsets[pos] = (*tiledata)[ground].wTileSet;
			terrain[pos] = (*tiledata)[ground].tiles[fd->bSubTile].bHackedTerrainType;
			tile[pos] = ground;

			if (xx >= left && xx < right && yy >= top && yy < bottom)
			{

				if (softsets.find((*tiledata)[ground].wTileSet) == softsets.end()/*(*tiledata)[ground].wTileSet==cliffset || (*tiledata)[ground].wTileSet==watercliffset*/)
				{
					noChange[pos] = TRUE; continue;
				}



				TILEDATA& td = (*tiledata)[ground];

				if (td.wTileSet == shoreset)
				{
					int of = fd->bSubTile;

					int ox = of / td.cy;
					int oy = of % td.cy;

					if (xx - ox < left || yy - oy < top || xx - ox + td.cx >= right || yy - oy + td.cy >= bottom)
					{
						/*if(!replaced[pos])*/ noChange[pos] = TRUE;
					}
				}
			}
		}
	}




	/*CProgressCtrl pc;
	RECT r,rw;
	GetWindowRect(&r);
	rw.left=r.left+(r.right-r.left)/2-80;
	rw.top=r.top+(r.bottom-r.top)/2-15;
	rw.right=rw.left+160;
	rw.bottom=rw.top+30;
	pc.Create(WS_POPUPWINDOW | PBS_SMOOTH, rw, m_view.m_isoview, 0);*/

	int tStart, tEnd;
	tStart = -1;
	tEnd = 0;
	for (i = 0;i < *tiledata_count;i++)
	{
		if ((*tiledata)[i].wTileSet == shoreset)
		{
			if (tStart < 0) tStart = i;
			if (i > tEnd) tEnd = i;
		}
	}

	/*pc.SetRange(0, (tEnd-tStart)*2);
		pc.ShowWindow(SW_SHOW);
		pc.RedrawWindow();*/


	last_succeeded_operation = 7004;

	for (i = tStart;i <= tEnd;i++)
	{
		/*pc.SetPos(i-tStart);
		pc.UpdateWindow();*/
		TILEDATA& td = (*tiledata)[i];

		if (td.wTileSet == shoreset)
		{
			int pos = i - tStart;
			if (pos != 4 && pos != 5 && pos != 12 && pos != 13 && pos != 20 && pos != 21 && pos != 28 && pos != 29
				&& pos != 6 && pos != 7 && pos != 14 && pos != 15 && pos != 22 && pos != 23 && pos != 30 && pos != 31 && (pos < 32 || pos>39))
				continue;

			int x, y;
			int water_count = 0;
			int p = 0;
			for (x = 0;x < td.cx;x++)
			{
				for (y = 0;y < td.cy;y++)
				{
					if (td.tiles[p].bHackedTerrainType == TERRAINTYPE_WATER)
						water_count++;
					p++;
				}
			}

			// tsets now has the tileset of every single field in range (x+16, y+16)
			// terrain has the terrain type of every single field
			int max_x = td.cx < 16 ? td.cx : 16;
			int max_y = td.cy < 16 ? td.cy : 16;

			for (x = left;x < right;x++)
			{
				for (y = top;y < bottom;y++)
				{
					last_succeeded_operation = 7010;

					int xx, yy;

					//if(!replaced[x+y*isosize] && (x<left || y<top || x>=right || y>=bottom)) continue;
					if (x < 1 || y < 1 || x + y<mapwidth + 1 || x + y>mapwidth + mapheight * 2 || (y + 1 > mapwidth && x - 1 < y - mapwidth) || (x + 1 > mapwidth && y + mapwidth - 1 < x)) continue;


					/*BOOL wat_ex=FALSE;
					for(xx=x;xx<x+max_x;xx++)
					{
						for(yy=y;yy<y+max_y;yy++)
						{
							FIELDDATA* fd=Map->GetFielddataAt(xx+yy*isosize);
							int ground=fd->wGround;
							if(ground==0xFFFF) ground=0;
							int tile_t=(*tiledata)[ground].tiles[fd->bSubTile].bHackedTerrainType;
							if(tile_t==TERRAINTYPE_WATER || tile_t==0xa)
								wat_ex=TRUE;
							if(wat_ex) break;
						}
						if(wat_ex) break;
					}
					if(!wat_ex) continue;*/

					BOOL bFits = TRUE;
					int p = 0;
					for (xx = x;xx < x + max_x;xx++)
					{
						for (yy = y;yy < y + max_y;yy++)
						{
							if (xx >= isosize || yy >= isosize) continue;


							int tpos = i - tStart;
							int xadd = 0, yadd = 0;

							/*
							if(tpos>=0 && tpos<=7) xadd=1;
							if(tpos>=6 && tpos<=15) yadd=1;
							if(tpos>=14 && tpos<=23) xadd=-1;
							if(tpos>=22 && tpos<=31) yadd=-1;
							if(tpos>=30 && tpos<=31) xadd=1;

							if(tpos>=32 && tpos<=33)
							{
								xadd=1; yadd=1;

							}

							if(tpos>=34 && tpos<=35)
							{
								xadd=-1; yadd=1;

							}

							if(tpos>=36 && tpos<=37)
							{
								xadd=-1; yadd=-1;

							}

							if(tpos>=38 && tpos<=39)
							{
								xadd=1; yadd=-1;

							}*/

							/*if(tpos>=32 && tpos<=33)
							{
								xadd=1; yadd=1;
								if(
								terrain[xx+1+yy*isosize]==TERRAINTYPE_WATER || terrain[xx+(yy+1)*isosize]==TERRAINTYPE_WATER
								)
								{bFits=FALSE; break;}
							}

							if(tpos>=34 && tpos<=35)
							{
								xadd=-1; yadd=1;
								if(
								terrain[xx-1+yy*isosize]==TERRAINTYPE_WATER || terrain[xx+(yy+1)*isosize]==TERRAINTYPE_WATER
								)
								{bFits=FALSE; break;}
							}

							if(tpos>=36 && tpos<=37)
							{
								xadd=-1; yadd=-1;
								if(
								terrain[xx-1+yy*isosize]==TERRAINTYPE_WATER || terrain[xx+(yy-1)*isosize]==TERRAINTYPE_WATER
								)
								{bFits=FALSE; break;}
							}

							if(tpos>=38 && tpos<=39)
							{
								xadd=1; yadd=-1;
								if(
								terrain[xx+1+yy*isosize]==TERRAINTYPE_WATER || terrain[xx+(yy-1)*isosize]==TERRAINTYPE_WATER
								)
								{bFits=FALSE; break;}
							}*/

							last_succeeded_operation = 7011;

							if (xadd && yadd)
							{
								if (tsets[xx + xadd + yy * isosize] == waterset || tsets[xx + (yy + yadd) * isosize] == waterset)
								{
									bFits = FALSE;
									break;
								}
							}


							int pos_water = xx + xadd + (yy + yadd) * isosize;
							int pos_data = xx + yy * isosize;



							BYTE& tile_t = td.tiles[p].bHackedTerrainType;

							if (tsets[pos_data] == shoreset)
							{
								if (hasChanged[pos_data]) // only cancel if this routine set the shore
								{
									// curves are preferred
									if ((max_x != 2 || max_y != 2 || water_count != 3))
									{
										if (!((max_x == 3 && max_y == 2) || (max_x == 2 && max_y == 3)))
										{
											bFits = FALSE;
											break;
										}

									}

								}
							}

							last_succeeded_operation = 7012;

							// one step curves
							if (noChange[pos_data])
							{
								bFits = FALSE;
								break;
							}


							// 2 big shore pieces need special treatment
							if (tile[pos_data] <= tEnd && tile[pos_data] >= tEnd - 2)
								bFits = FALSE;

							if (tile_t == TERRAINTYPE_WATER)
							{
								if (terrain[pos_water] != TERRAINTYPE_WATER)
								{
									bFits = FALSE;
								}
							}
							else
							{
								if (terrain[pos_water] != TERRAINTYPE_GROUND)
									//if(tsets[pos_water]==waterset)
								{
									bFits = FALSE;
								}
							}


							if (!bFits) break;
							p++;
						}
						if (!bFits) break;
					}

					last_succeeded_operation = 7012;

					if (bFits) // ok, place shore (later we need to do random choose of the different tiles here
					{
						// find similar shore piece (randomness)
						int count = 0;
						int pieces[16];
						int k;
						TILEDATA& t_orig = (*tiledata)[i];
						for (k = 0;k < *tiledata_count;k++)
						{
							TILEDATA& t = (*tiledata)[k];

							if (t.bMarbleMadness) continue;

							if (t.cx != t_orig.cx || t.cy != t_orig.cy) continue;

							if (k != 4 && k != 5 && k != 12 && k != 13 && k != 20 && k != 21 && k != 28 && k != 29
								&& (k < 32 || k>39))
							{
							}
							else continue;

							int xx, yy;
							BOOL bSame = TRUE;
							int p = 0;
							for (xx = 0;xx < t.cx;xx++)
							{
								for (yy = 0;yy < t.cy;yy++)
								{
									if (t.tiles[p].bHackedTerrainType != t_orig.tiles[p].bHackedTerrainType)
										bSame = FALSE;
									p++;
									if (!bSame) break;
								}
								if (!bSame) break;
							}

							if (bSame && count < 16)
							{
								pieces[count] = k;
								count++;
							}
						}

						last_succeeded_operation = 7013;

						k = ((float)rand() * count) / (float)RAND_MAX;
						if (k >= count) k = count - 1;
						k = pieces[k];

						TILEDATA& t = (*tiledata)[k];
						int p = 0;
						int xx, yy;
						int startheight = GetHeightAt(x + y * isosize);
						for (xx = 0;xx < t.cx;xx++)
						{
							for (yy = 0;yy < t.cy;yy++)
							{
								if (x + xx >= isosize || y + yy >= isosize) continue;

								int pos = x + xx + (y + yy) * isosize;
								last_succeeded_operation = 7014;

								if (t.tiles[p].pic != NULL)
								{
									SetHeightAt(pos, startheight + t.tiles[p].bZHeight);
									SetTileAt(pos, k, p);
									last_succeeded_operation = 7015;
									tile[pos] = i;
									tsets[pos] = (*tiledata)[k].wTileSet;
									terrain[pos] = (*tiledata)[k].tiles[p].bHackedTerrainType;
									hasChanged[pos] = TRUE;
									if ((t.cx == 3 && t.cy == 2) || (t.cx == 2 && t.cy == 3)) noChange[pos] = TRUE;
								}
								p++;
							}
						}


					}

				}
			}
		}
	}

	last_succeeded_operation = 7005;


	for (i = tStart;i <= tEnd;i++)
	{
		/*pc.SetPos(i-tStart+(tEnd-tStart));
		pc.UpdateWindow();*/
		TILEDATA& td = (*tiledata)[i];

		if ((*tiledata)[i].wTileSet == shoreset)
		{
			int pos = i - tStart;
			if (pos != 4 && pos != 5 && pos != 12 && pos != 13 && pos != 20 && pos != 21 && pos != 28 && pos != 29
				&& pos != 6 && pos != 7 && pos != 14 && pos != 15 && pos != 22 && pos != 23 && pos != 30 && pos != 31 && (pos < 32 || pos>39))

			{
			}
			else continue;

			int x, y;
			int water_count = 0;
			int p = 0;
			for (x = 0;x < td.cx;x++)
			{
				for (y = 0;y < td.cy;y++)
				{
					if (td.tiles[p].bHackedTerrainType == TERRAINTYPE_WATER)
						water_count++;
					p++;
				}
			}


			// tsets now has the tileset of every single field in range (x+16, y+16)
			// terrain has the terrain type of every single field
			int max_x = td.cx < 16 ? td.cx : 16;
			int max_y = td.cy < 16 ? td.cy : 16;

			for (x = left;x < right;x++)
			{
				for (y = top;y < bottom;y++)
				{
					int xx, yy;

					//if(!replaced[x+y*isosize] && (x<left || y<top || x>=right || y>=bottom)) continue;
					if (x < 1 || y < 1 || x + y<mapwidth + 1 || x + y>mapwidth + mapheight * 2 || (y + 1 > mapwidth && x - 1 < y - mapwidth) || (x + 1 > mapwidth && y + mapwidth - 1 < x)) continue;





					/*BOOL wat_ex=FALSE;
					for(xx=x;xx<x+max_x;xx++)
					{
						for(yy=y;yy<y+max_y;yy++)
						{
							FIELDDATA* fd=Map->GetFielddataAt(xx+yy*isosize);
							int ground=fd->wGround;
							if(ground==0xFFFF) ground=0;
							int tile_t=(*tiledata)[ground].tiles[fd->bSubTile].bHackedTerrainType;
							if(tile_t==TERRAINTYPE_WATER || tile_t==0xa)
								wat_ex=TRUE;
							if(wat_ex) break;
						}
						if(wat_ex) break;
					}
					if(!wat_ex) continue;*/


					BOOL bFits = TRUE;
					int p = 0;
					for (xx = x;xx < x + max_x;xx++)
					{
						for (yy = y;yy < y + max_y;yy++)
						{
							if (xx >= isosize || yy >= isosize) continue;

							int tpos = i - tStart;
							int xadd = 0, yadd = 0;

							/*if(tpos>=0 && tpos<=7) xadd=1;
							if(tpos>=6 && tpos<=15) yadd=1;
							if(tpos>=14 && tpos<=23) xadd=-1;
							if(tpos>=22 && tpos<=31) yadd=-1;
							if(tpos>=30 && tpos<=31) xadd=1;

							if(tpos>=32 && tpos<=33)
							{
								xadd=1; yadd=1;

							}

							if(tpos>=34 && tpos<=35)
							{
								xadd=-1; yadd=1;

							}

							if(tpos>=36 && tpos<=37)
							{
								xadd=-1; yadd=-1;

							}

							if(tpos>=38 && tpos<=39)
							{
								xadd=1; yadd=-1;

							}


							if(xadd && yadd)
							{
								if(tsets[xx+xadd+yy*isosize]==waterset || tsets[xx+(yy+yadd)*isosize]==waterset)
								{
									bFits=FALSE;
									break;
								}
							}*/




							int pos_water = xx + xadd + (yy + yadd) * isosize;
							int pos_data = xx + yy * isosize;

							BYTE& tile_t = td.tiles[p].bHackedTerrainType;

							if (tsets[pos_data] == shoreset)
							{
								if (hasChanged[pos_data]) // only cancel if this routine set the shore
								{
									// curves are preferred
									if ((max_x != 2 || max_y != 2 || water_count != 3))
									{
										if (!((max_x == 3 && max_y == 2) || (max_x == 2 && max_y == 3)))
										{
											bFits = FALSE;
											break;
										}
									}

								}
							}

							// one step curves
							if (noChange[pos_data])
							{
								bFits = FALSE;
								break;
							}


							if (tile[pos_data] <= tEnd && tile[pos_data] >= tEnd - 2)
								bFits = FALSE;

							if (tile_t == TERRAINTYPE_WATER)
							{
								if (terrain[pos_water] != TERRAINTYPE_WATER)
								{
									bFits = FALSE;
								}
							}
							else
							{
								if (terrain[pos_water] != TERRAINTYPE_GROUND)
									//if(tsets[pos_water]==waterset)
								{
									bFits = FALSE;
								}
							}


							if (!bFits) break;
							p++;
						}
						if (!bFits) break;
					}

					last_succeeded_operation = 7031;
					if (bFits) // ok, place shore 
					{
						// find similar shore piece (randomness)
						int count = 0;
						int pieces[16];
						int k;
						TILEDATA& t_orig = (*tiledata)[i];
						for (k = 0;k < *tiledata_count;k++)
						{
							TILEDATA& t = (*tiledata)[k];
							if (t.cx != t_orig.cx || t.cy != t_orig.cy) continue;

							if (t.bMarbleMadness) continue;

							if (k != 4 && k != 5 && k != 12 && k != 13 && k != 20 && k != 21 && k != 28 && k != 29
								&& (k < 32 || k>39))
							{
							}
							else continue;

							int xx, yy;
							BOOL bSame = TRUE;
							int p = 0;
							for (xx = 0;xx < t.cx;xx++)
							{
								for (yy = 0;yy < t.cy;yy++)
								{
									if (t.tiles[p].bHackedTerrainType != t_orig.tiles[p].bHackedTerrainType)
										bSame = FALSE;
									p++;
									if (!bSame) break;
								}
								if (!bSame) break;
							}

							if (bSame && count < 16)
							{
								pieces[count] = k;
								count++;
							}
						}

						last_succeeded_operation = 7032;

						k = ((float)rand() * count) / (float)RAND_MAX;
						if (k >= count) k = count - 1;
						k = pieces[k];

						TILEDATA& t = (*tiledata)[k];
						int p = 0;
						int xx, yy;
						int startheight = GetHeightAt(x + y * isosize);
						for (xx = 0;xx < t.cx;xx++)
						{
							for (yy = 0;yy < t.cy;yy++)
							{
								if (x + xx >= isosize || y + yy >= isosize) continue;

								int pos = x + xx + (y + yy) * isosize;

								if (t.tiles[p].pic != NULL)
								{
									SetHeightAt(pos, startheight + t.tiles[p].bZHeight);
									SetTileAt(pos, k, p);
									tile[pos] = i;
									tsets[pos] = (*tiledata)[k].wTileSet;
									terrain[pos] = (*tiledata)[k].tiles[p].bHackedTerrainType;
									hasChanged[pos] = TRUE;
									if ((t.cx == 3 && t.cy == 2) || (t.cx == 2 && t.cy == 3)) noChange[pos] = TRUE;
								}
								p++;
							}
						}
					}

				}
			}
		}
	}

	last_succeeded_operation = 7006;

	memset(hasChanged, 0, sizeof(BOOL) * isosize * isosize);

	// now make LAT (RA2 only)

#ifdef RA2_MODE
	int x, y;
	for (x = left;x < right;x++)
	{
		for (y = top;y < bottom;y++)
		{
			int xx, yy;
			if (x < 1 || y < 1 || x + y<mapwidth + 1 || x + y>mapwidth + mapheight * 2 || (y + 1 > mapwidth && x - 1 < y - mapwidth) || (x + 1 > mapwidth && y + mapwidth - 1 < x)) continue;

			int pos = x + y * isosize;

			if (noChange[pos]) continue;
			if (terrain[pos] == TERRAINTYPE_GROUND && tsets[pos] != shoreset && tsets[pos] != cliffset && tsets[pos] != watercliffset)
			{
				int i, e;
				BOOL bShoreFound = FALSE;
				for (i = x - 1;i <= x + 1;i++)
				{
					for (e = y - 1;e <= y + 1;e++)
					{
						if (tsets[i + e * isosize] == shoreset) bShoreFound = TRUE;
						if (bShoreFound) break;
					}
					if (bShoreFound) break;
				}

				if (bShoreFound)
				{
					int sandtile = atoi(tiles->sections["General"].values["GreenTile"]);
					int sandlat = atoi(tiles->sections["General"].values["ClearToGreenLat"]);

					int i;
					for (i = 0;i < *tiledata_count;i++)
						if ((*tiledata)[i].wTileSet == sandtile) break;
					Map->SetTileAt(pos, i, 0);
					hasChanged[pos] = TRUE;

				}

			}
		}
	}

	for (x = left - 1;x < right + 1;x++)
	{
		for (y = top - 1;y < bottom + 1;y++)
		{
			int xx, yy;
			if (x < 1 || y < 1 || x + y<mapwidth + 1 || x + y>mapwidth + mapheight * 2 || (y + 1 > mapwidth && x - 1 < y - mapwidth) || (x + 1 > mapwidth && y + mapwidth - 1 < x)) continue;

			int pos = x + y * isosize;
			if (noChange[pos]) continue;

			if (terrain[pos] == TERRAINTYPE_GROUND && tsets[pos] != shoreset && tsets[pos] != cliffset && tsets[pos] != watercliffset)
			{
				int i, e;
				BOOL bShoreFound = FALSE;
				BOOL bSomethingChanged = FALSE;
				for (i = x - 1;i <= x + 1;i++)
				{
					for (e = y - 1;e <= y + 1;e++)
					{
						if (tsets[i + e * isosize] == shoreset) bShoreFound = TRUE;
						if (hasChanged[i + e * isosize]) bSomethingChanged = TRUE;
						if (bShoreFound && hasChanged[i + e * isosize]) break;
					}
					if (bShoreFound && hasChanged[i + e * isosize]) break;
				}



				if (bShoreFound && hasChanged)
				{
					int sandtile = atoi(tiles->sections["General"].values["GreenTile"]);
					int sandlat = atoi(tiles->sections["General"].values["ClearToGreenLat"]);


					SmoothAt(pos, sandtile, sandlat, atoi(tiles->sections["General"].values["ClearTile"]));
				}

			}
		}
	}
#endif



	//delete[] replaced;
	delete[] hasChanged;
	delete[] noChange;
	delete[] tsets;
	delete[] terrain;
	delete[] tile;

	//pc.DestroyWindow();



}

BOOL CMapData::IsMultiplayer()
{
	if (m_mapfile.sections["Basic"].FindName("Player") >= 0) return FALSE;
	if (isTrue(m_mapfile.sections["Basic"].values["MultiplayerOnly"])) return TRUE;
	if (m_mapfile.sections.find(MAPHOUSES) == m_mapfile.sections.end()) return TRUE;

	return FALSE;
}

CString CMapData::GetTheater()
{
	return m_mapfile.sections["Map"].values["Theater"];
}

void CMapData::Copy(int left, int top, int right, int bottom)
{
	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right > m_IsoSize) right = m_IsoSize;
	if (bottom > m_IsoSize) bottom = m_IsoSize;

	if (right == 0) right = m_IsoSize;
	if (bottom == 0) bottom = m_IsoSize;

	if (left >= right) return;
	if (top >= bottom) return;


	CLIPBOARD_DATA cd;
	cd.dwType = 1;
	cd.iWidth = right - left;
	cd.iHeight = bottom - top;
	cd.dwVersion = 0;
	cd.dwReserved = 0;
	if (editor_mode == ra2_mode) cd.bGame = 1; else cd.bGame = 0;

	HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(CLIPBOARD_DATA) + cd.iWidth * cd.iHeight * sizeof(CLIPBOARD_MAPCOPY_ENTRY));

	last_succeeded_operation = 80200;

	void* lpVoid = GlobalLock(hGlob);

	if (!lpVoid)
	{
		MessageBox(0, "Failed to lock memory", "Error", 0);
		return;
	}

	memcpy(lpVoid, &cd, sizeof(CLIPBOARD_DATA));

	CLIPBOARD_MAPCOPY_ENTRY* data = (CLIPBOARD_MAPCOPY_ENTRY*)(((BYTE*)lpVoid) + sizeof(CLIPBOARD_DATA));

	int i;
	int e;

	int lowestheight = 255;
	for (i = 0;i < cd.iWidth;i++)
	{
		for (e = 0;e < cd.iHeight;e++)
		{
			if (i + left < 0 || e + top < 0 || i + left >= m_IsoSize || e + top >= m_IsoSize) continue;

			int pos_r;
			pos_r = left + i + (top + e) * m_IsoSize;

			int ground = fielddata[pos_r].wGround;
			if (ground == 0xFFFF) ground = 0;

			if (fielddata[pos_r].bHeight - (*tiledata)[ground].tiles[fielddata[pos_r].bSubTile].bZHeight < lowestheight) lowestheight = fielddata[pos_r].bHeight - (*tiledata)[ground].tiles[fielddata[pos_r].bSubTile].bZHeight;
		}
	}

	last_succeeded_operation = 80201;

	for (i = 0;i < cd.iWidth;i++)
	{
		for (e = 0;e < cd.iHeight;e++)
		{
			if (i + left < 0 || e + top < 0 || i + left >= m_IsoSize || e + top >= m_IsoSize) continue;

			int pos_w, pos_r;
			pos_w = i + e * cd.iWidth;
			pos_r = left + i + (top + e) * m_IsoSize;

			data[pos_w].bHeight = fielddata[pos_r].bHeight - lowestheight;
			data[pos_w].bMapData = fielddata[pos_r].bMapData;
			data[pos_w].bSubTile = fielddata[pos_r].bSubTile;
			data[pos_w].bMapData2 = fielddata[pos_r].bMapData2;
			data[pos_w].wGround = fielddata[pos_r].wGround;
			data[pos_w].overlay = fielddata[pos_r].overlay;
			data[pos_w].overlaydata = fielddata[pos_r].overlaydata;
			data[pos_w].bRedrawTerrain = fielddata[pos_r].bRedrawTerrain;

			int ground = fielddata[pos_r].wGround;
			if (ground == 0xFFFF) ground = 0;
			int tset = (*tiledata)[ground].wTileSet;

			int k;
			for (k = 0;k < *tiledata_count;k++)
			{
				if ((*tiledata)[k].wTileSet == tset) break;
			}

			data[pos_w].wTileSet = tset;
			data[pos_w].bTile = ground - k;
		}
	}

	last_succeeded_operation = 80202;

	while (GlobalUnlock(hGlob));

	OpenClipboard(theApp.m_pMainWnd->m_hWnd);
	EmptyClipboard();

	if (!SetClipboardData(theApp.m_cf, hGlob))
	{
		MessageBox(0, "Failed to set clipboard data", "Error", 0);

	}

	CloseClipboard();

}

void CMapData::Paste(int x, int y, int z_mod)
{
	OpenClipboard(theApp.m_pMainWnd->m_hWnd);
	HANDLE handle = GetClipboardData(theApp.m_cf);

	void* lpVoid = GlobalLock(handle);

	if (!lpVoid)
	{
		CloseClipboard();
		return;
	}

	last_succeeded_operation = 3001;

	CLIPBOARD_DATA cd;
	memcpy(&cd, lpVoid, sizeof(CLIPBOARD_DATA));


	x -= cd.iWidth / 2;
	y -= cd.iHeight / 2;

	CLIPBOARD_MAPCOPY_ENTRY* data = (CLIPBOARD_MAPCOPY_ENTRY*)(((BYTE*)lpVoid) + sizeof(CLIPBOARD_DATA));

	last_succeeded_operation = 3002;

	int lowestheight = 255;
	int mapwidth, mapheight;
	mapwidth = GetWidth();
	mapheight = GetHeight();

	int i;
	int e;
	for (i = 0;i < cd.iWidth;i++)
	{
		for (e = 0;e < cd.iHeight;e++)
		{
			if (x + i < 0 || y + e < 0 || x + i >= m_IsoSize || y + e >= m_IsoSize) continue;

			FIELDDATA* fd = Map->GetFielddataAt(i + x + (y + e) * m_IsoSize);
			int ground = fd->wGround;
			if (ground = 0xFFFF) ground = 0;


			int height = fd->bHeight;//-(*tiledata)[ground].tiles[fd->bSubTile].bZHeight;

			if (height < lowestheight) lowestheight = height;


		}
	}

	int ground = GetFielddataAt(x + y * m_IsoSize)->wGround;
	if (ground == 0xFFFF) ground = 0;
	int startheight = lowestheight + z_mod;//-(*tiledata)[ground].tiles[GetFielddataAt(x+y*m_IsoSize)->bSubTile].bZHeight;

	//char c[50];
	//itoa(startheight, c, 10);
	//MessageBox(0,c,"",0);

	last_succeeded_operation = 3003;

	const bool mp = IsMultiplayer();
	for (i = 0;i < cd.iWidth;i++)
	{
		for (e = 0;e < cd.iHeight;e++)
		{
			int pos_w, pos_r;
			pos_r = i + e * cd.iWidth;
			pos_w = x + i + (y + e) * m_IsoSize;

			if (x + i < 0 || y + e < 0 || x + i >= m_IsoSize || y + e >= m_IsoSize) continue;

			RemoveOvrlMoney(fielddata[pos_w].overlay, fielddata[pos_w].overlaydata);

			fielddata[pos_w].overlay = data[pos_r].overlay;
			fielddata[pos_w].overlaydata = data[pos_r].overlaydata;

			AddOvrlMoney(fielddata[pos_w].overlay, fielddata[pos_w].overlaydata);

			SetHeightAt(pos_w, startheight + data[pos_r].bHeight);

			int tset = data[pos_r].wTileSet;
			int tile = data[pos_r].bTile;
			int k;
			int found = -1;
			for (k = 0;k < *tiledata_count;k++)
			{
				if ((*tiledata)[k].wTileSet == tset)
				{
					found = k;
					break;
				}
			}

			if (found < 0) continue;
			if ((*tiledata)[found + tile].wTileSet != tset)
			{
				continue;
			}
			if ((*tiledata)[found + tile].wTileCount <= data[pos_r].bSubTile) continue;


			fielddata[pos_w].bSubTile = data[pos_r].bSubTile;
			fielddata[pos_w].bRedrawTerrain = data[pos_r].bRedrawTerrain;
			fielddata[pos_w].wGround = found + tile;



			fielddata[pos_w].bMapData = data[pos_r].bMapData;
			fielddata[pos_w].bMapData2 = data[pos_r].bMapData2;

			Mini_UpdatePos(x + i, y + e, mp);
		}
	}

	last_succeeded_operation = 3005;
	GlobalUnlock(handle);

	CloseClipboard();
}

void CMapData::GetStructurePaint(int index, STRUCTUREPAINT* lpStructurePaint) const
{
	if (index < 0 || index >= m_structurepaint.size()) return;

	*lpStructurePaint = m_structurepaint[index];
}

void CMapData::InitMinimap()
{

	int pwidth = GetWidth() * 2;
	int pheight = GetHeight();

	memset(&m_mini_biinfo, 0, sizeof(BITMAPINFO));
	m_mini_biinfo.bmiHeader.biBitCount = 24;
	m_mini_biinfo.bmiHeader.biWidth = pwidth;
	m_mini_biinfo.bmiHeader.biHeight = pheight;
	m_mini_biinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_mini_biinfo.bmiHeader.biClrUsed = 0;
	m_mini_biinfo.bmiHeader.biPlanes = 1;
	m_mini_biinfo.bmiHeader.biCompression = BI_RGB;
	m_mini_biinfo.bmiHeader.biClrImportant = 0;

	int pitch = pwidth * 3;
	if (pitch == 0) return;

	if (pitch % sizeof(DWORD))
	{
		pitch += sizeof(DWORD) - (pwidth * 3) % sizeof(DWORD);
	}

	m_mini_pitch = pitch;

	//m_mini_colors=new(BYTE[pitch*pheight]);
	m_mini_colors.resize(pitch * pheight);


	memset(m_mini_colors.data(), 255, pitch * (pheight));


	/*DWORD dwIsoSize=GetIsoSize();

	int mapwidth=GetWidth();
	int mapheight=GetHeight();
	int i,e;

	int size=pitch*pheight;

	for(i=0;i<dwIsoSize;i++)
	{
		for(e=0;e<dwIsoSize;e++)
		{
			Mini_UpdatePos(i,e);
		}
	}*/

}

void CMapData::GetMinimap(BYTE** lpData, BITMAPINFO* lpBI, int* pitch)
{
	*lpData = m_mini_colors.data();
	*pitch = m_mini_pitch;
	memcpy(lpBI, &m_mini_biinfo, sizeof(BITMAPINFO));
}

int CMapData::CalcMoneyOnMap()
{
	int i;
	int money = 0;
	for (i = 0;i < fielddata_size;i++)
	{
		FIELDDATA& fd = fielddata[i];

		BYTE& ovrl = fd.overlay;
		BYTE& ovrld = fd.overlaydata;


		if (ovrl >= RIPARIUS_BEGIN && ovrl <= RIPARIUS_END)
		{
			money += (ovrld + 1) * (atoi(rules.sections["Riparius"].values["Value"]));
		}

		if (ovrl >= CRUENTUS_BEGIN && ovrl <= CRUENTUS_END)
		{
			money += (ovrld + 1) * (atoi(rules.sections["Cruentus"].values["Value"]));
		}

		if (ovrl >= VINIFERA_BEGIN && ovrl <= VINIFERA_END)
		{
			money += (ovrld + 1) * (atoi(rules.sections["Vinifera"].values["Value"]));
		}

		if (ovrl >= ABOREUS_BEGIN && ovrl <= ABOREUS_END)
		{
			money += (ovrld + 1) * (atoi(rules.sections["Aboreus"].values["Value"]));
		}
	}

	return money;
}

int CMapData::GetMoneyOnMap() const
{
	return m_money;
}

int CMapData::GetPowerOfHouse(LPCTSTR lpHouse)
{
	// stub
	return 0;
}

void CMapData::SmoothTiberium(DWORD dwPos)
{
	static int _adj[9] = { 0,1,3,4,6,7,8,10,11 };

	BYTE& ovrl = fielddata[dwPos].overlay;

	if (!(ovrl >= RIPARIUS_BEGIN && ovrl <= RIPARIUS_END) &&
		!(ovrl >= CRUENTUS_BEGIN && ovrl <= CRUENTUS_END) &&
		!(ovrl >= VINIFERA_BEGIN && ovrl <= VINIFERA_END) &&
		!(ovrl >= ABOREUS_BEGIN && ovrl <= ABOREUS_END))
		return;

	RemoveOvrlMoney(ovrl, fielddata[dwPos].overlaydata);

	int i, e;
	int x, y;
	x = dwPos % m_IsoSize;
	y = dwPos / m_IsoSize;
	int count = 0;

	for (i = -1;i < 2;i++)
	{
		for (e = -1;e < 2;e++)
		{
			int xx = x + i;
			int yy = y + e;

			if (xx < 0 || xx >= m_IsoSize || yy < 0 || yy >= m_IsoSize) continue;

			FIELDDATA& fd = fielddata[xx + yy * m_IsoSize];

			BYTE& ovrl = fd.overlay;
			BYTE& ovrld = fd.overlaydata;


			if (ovrl >= RIPARIUS_BEGIN && ovrl <= RIPARIUS_END)
			{
				count++;
			}

			if (ovrl >= CRUENTUS_BEGIN && ovrl <= CRUENTUS_END)
			{
				count++;
			}

			if (ovrl >= VINIFERA_BEGIN && ovrl <= VINIFERA_END)
			{
				count++;
			}

			if (ovrl >= ABOREUS_BEGIN && ovrl <= ABOREUS_END)
			{
				count++;
			}

		}
	}

	ASSERT(count);

	fielddata[dwPos].overlaydata = _adj[count - 1];
	m_OverlayData[y + x * 512] = _adj[count - 1];

	AddOvrlMoney(ovrl, fielddata[dwPos].overlaydata);

}

void CMapData::ResizeMap(int iLeft, int iTop, DWORD dwNewWidth, DWORD dwNewHeight)
{
#ifndef RA2_MODE
	if (MessageBox(0, "Tunnels may be damaged after changing the map size. Continue?", "Warning", MB_YESNO) == IDNO) return;
#endif

	// MW: New object replacing code to fix crashes
	INFANTRY* inf = new(INFANTRY[GetInfantryCount()]);
	int inf_count = GetInfantryCount();
	STRUCTURE* str = new(STRUCTURE[GetStructureCount()]);
	int str_count = GetStructureCount();
	UNIT* unit = new(UNIT[GetUnitCount()]);
	int unit_count = GetUnitCount();
	AIRCRAFT* air = new(AIRCRAFT[GetAircraftCount()]);
	int air_count = GetAircraftCount();
	TERRAIN* terrain = new(TERRAIN[GetTerrainCount()]);
	int terrain_count = GetTerrainCount();
	CString* wp_id = new(CString[GetWaypointCount()]);
	int wp_count = GetWaypointCount();
	DWORD* wp_pos = new(DWORD[GetWaypointCount()]);
	CString* ct_tag = new(CString[GetCelltagCount()]);
	int ct_count = GetCelltagCount();
	DWORD* ct_pos = new(DWORD[GetCelltagCount()]);

	int i;

	// Now copy the objects into above arrays and delete them from map
	for (i = 0;i < inf_count;i++)
	{
		INFANTRY obj;
		GetInfantryData(i, &obj);
		inf[i] = obj;
	}
	for (i = inf_count - 1;i >= 0;i--)
		DeleteInfantry(i);

	for (i = 0;i < air_count;i++)
	{
		AIRCRAFT obj;
		GetAircraftData(i, &obj);

		air[i] = obj;
	}
	for (i = air_count - 1;i >= 0;i--)
		DeleteAircraft(i);

	for (i = 0;i < str_count;i++)
	{
		STRUCTURE obj;
		GetStructureData(i, &obj);

		str[i] = obj;
	}
	for (i = str_count - 1;i >= 0;i--)
		DeleteStructure(i);

	for (i = 0;i < unit_count;i++)
	{
		UNIT obj;
		GetUnitData(i, &obj);

		unit[i] = obj;
	}
	for (i = unit_count - 1;i >= 0;i--)
		DeleteUnit(i);

	for (i = 0;i < terrain_count;i++)
	{
		terrain[i] = m_terrain[i];
	}
	for (i = 0;i < terrain_count;i++)
		DeleteTerrain(i);


	for (i = 0;i < wp_count;i++)
	{
		DWORD pos;
		CString id;

		GetWaypointData(i, &id, &pos);

		wp_id[i] = id;
		wp_pos[i] = pos;
	}

	// for(i=0;i<wp_count;i++) DeleteWaypoint(0);


	for (i = 0;i < ct_count;i++)
	{
		DWORD pos;
		CString tag;

		GetCelltagData(i, &tag, &pos);

		ct_tag[i] = tag;
		ct_pos[i] = pos;
	}

	for (i = 0;i < ct_count;i++) DeleteCelltag(0);


	FIELDDATA* old_fd = fielddata;
	int ow = GetWidth();
	int oh = GetHeight();
	int os = GetIsoSize();
	int old_fds = fielddata_size;

	int left = iLeft;
	int top = iTop;


	// hmm, erase any snapshots... we probably can remove this and do coordinate conversion instead
	// but for now we just delete them...
	for (i = 0;i < dwSnapShotCount;i++)
	{
		delete[] m_snapshots[i].bHeight;
		delete[] m_snapshots[i].bMapData;
		delete[] m_snapshots[i].bSubTile;
		delete[] m_snapshots[i].bMapData2;
		delete[] m_snapshots[i].wGround;
		delete[] m_snapshots[i].bRedrawTerrain;
		delete[] m_snapshots[i].overlay;
		delete[] m_snapshots[i].overlaydata;
		// m_snapshots[i].mapfile.Clear();
	}
	if (m_snapshots != NULL) delete[] m_snapshots;


	fielddata = NULL;
	fielddata_size = 0;
	m_snapshots = NULL;
	dwSnapShotCount = 0;
	m_cursnapshot = -1;


	char c[50];
	CString mapsize;
	itoa(dwNewWidth, c, 10);
	mapsize = "0,0,";
	mapsize += c;
	itoa(dwNewHeight, c, 10);
	mapsize += ",";
	mapsize += c;

	m_mapfile.sections["Map"].values["Size"] = mapsize;

	itoa(dwNewWidth - 4, c, 10);
	mapsize = "2,4,";
	mapsize += c;
	itoa(dwNewHeight - 6, c, 10);
	mapsize += ",";
	mapsize += c;

	m_mapfile.sections["Map"].values["LocalSize"] = mapsize;


	CalcMapRect();
	ClearOverlay();
	ClearOverlayData();



	errstream << "ResizeMap() allocates memory\n";
	errstream.flush();
	fielddata = new(FIELDDATA[(GetIsoSize() + 1) * (GetIsoSize() + 1)]); // +1 because of some unpacking problems
	fielddata_size = (GetIsoSize() + 1) * (GetIsoSize() + 1);
	dwIsoMapSize = 0; // our iso mappack is empty now, as we didn´t load from a file

	errstream << "ResizeMap() frees m_mfd\n";
	errstream.flush();
	if (m_mfd != NULL) delete[] m_mfd;
	m_mfd = NULL;


	// x_move and y_move specify the movement for each field, related to the old position
	int x_move = 0;
	int y_move = 0;

	x_move += (dwNewWidth - ow);


	// x_move and y_move now take care of the map sizing. This means,
	// all new or removed tiles will now be added/deleted to/from the bottom right
	// but we want to consider left and right, as the user selected it.
	// so, do some coordinate conversion:

	x_move += top;
	y_move += top;

	x_move += -left;
	y_move += left;

	//char c[50];
	/*char d[50];
	itoa(y_move, d, 10);
	itoa(x_move, c, 10);
	MessageBox(0, c, d,0);*/


	// copy tiles now
	int e;
	for (i = 0;i < os;i++)
	{
		for (e = 0;e < os;e++)
		{
			int x, y;
			x = i + x_move;
			y = e + y_move;

			if (x < 0 || y < 0 || x >= m_IsoSize || y >= m_IsoSize) continue;

			FIELDDATA& fdd = fielddata[x + y * m_IsoSize];
			FIELDDATA& fdo = old_fd[i + e * os];

			fdd.bCliffHack = fdo.bCliffHack;
			fdd.bHeight = fdo.bHeight;
			fdd.bHide = fdo.bHide;
			fdd.bMapData = fdo.bMapData;
			fdd.bMapData2 = fdo.bMapData2;
			fdd.bRedrawTerrain = fdo.bRedrawTerrain;
			fdd.bRNDImage = fdo.bRNDImage;
			fdd.bSubTile = fdo.bSubTile;
			fdd.overlay = fdo.overlay;
			fdd.overlaydata = fdo.overlaydata;
			fdd.wGround = fdo.wGround;
		}
	}

	// MW 07/22/01: Added Progress dialog - it just was slow, and did not crash...
	int allcount = GetInfantryCount() + GetAircraftCount() + GetUnitCount() + GetStructureCount() + GetTerrainCount() + GetWaypointCount() + GetCelltagCount();
	int curcount = 0;
	CProgressDlg* dlg = new(CProgressDlg)("Updating objects, please wait");
	dlg->SetRange(0, allcount - 1);
	dlg->ShowWindow(SW_SHOW);

	//m_noAutoObjectUpdate=TRUE; // deactivate Update*()... faster

	int count = inf_count; // this temp variable is *needed* (infinite loop)!!!
	for (i = 0;i < count;i++)
	{
		if (inf[i].deleted)
		{
			dlg->SetPosition(i + curcount);
			dlg->UpdateWindow();

			continue; // MW June 12 01
		}

		INFANTRY obj;
		obj = inf[i];

		char c[50];
		obj.x = itoa(atoi(obj.x) + x_move, c, 10);
		obj.y = itoa(atoi(obj.y) + y_move, c, 10);

		int x = atoi(obj.x);
		int y = atoi(obj.y);


		if (x < 0 || y < 0 || x >= m_IsoSize || y >= m_IsoSize) continue;

		AddInfantry(&obj);

		dlg->SetPosition(i + curcount);
		dlg->UpdateWindow();

	}

	curcount += count;

	count = air_count;
	for (i = 0;i < count;i++)
	{
		// if(air[i].deleted) continue;

		AIRCRAFT obj;
		obj = air[i];

		char c[50];
		obj.x = itoa(atoi(obj.x) + x_move, c, 10);
		obj.y = itoa(atoi(obj.y) + y_move, c, 10);

		int x = atoi(obj.x);
		int y = atoi(obj.y);


		if (x < 0 || y < 0 || x >= m_IsoSize || y >= m_IsoSize) continue;

		AddAircraft(&obj);

		dlg->SetPosition(i + curcount);
		dlg->UpdateWindow();
	}

	UpdateAircraft(FALSE);

	curcount += count;

	count = str_count;
	for (i = 0;i < count;i++)
	{
		// if(str[i].deleted) continue;

		STRUCTURE obj;
		obj = str[i];

		char c[50];
		obj.x = itoa(atoi(obj.x) + x_move, c, 10);
		obj.y = itoa(atoi(obj.y) + y_move, c, 10);

		int x = atoi(obj.x);
		int y = atoi(obj.y);


		if (x < 0 || y < 0 || x >= m_IsoSize || y >= m_IsoSize) continue;

		AddStructure(&obj);

		dlg->SetPosition(i + curcount);
		dlg->UpdateWindow();
	}

	UpdateStructures(FALSE);

	curcount += count;

	count = unit_count;
	for (i = 0;i < count;i++)
	{
		// if(units[i].deleted) continue;

		UNIT obj;
		obj = unit[i];

		char c[50];
		obj.x = itoa(atoi(obj.x) + x_move, c, 10);
		obj.y = itoa(atoi(obj.y) + y_move, c, 10);

		int x = atoi(obj.x);
		int y = atoi(obj.y);


		if (x < 0 || y < 0 || x >= m_IsoSize || y >= m_IsoSize) continue;

		AddUnit(&obj);

		dlg->SetPosition(i + curcount);
		dlg->UpdateWindow();
	}

	UpdateUnits(FALSE);

	curcount += count;

	count = terrain_count;
	for (i = 0;i < count;i++)
	{
		if (terrain[i].deleted)
		{
			dlg->SetPosition(i + curcount);
			dlg->UpdateWindow();
			continue; // MW June 12 01
		}

		CString obj;
		int x = terrain[i].x;
		int y = terrain[i].y;
		obj = terrain[i].type;


		char c[50];
		x = x + x_move;
		y = y + y_move;


		if (x < 0 || y < 0 || x >= m_IsoSize || y >= m_IsoSize) continue;

		AddTerrain(obj, x + y * m_IsoSize);

		dlg->SetPosition(i + curcount);
		dlg->UpdateWindow();
	}

	//UpdateTerrain(TRUE);
	//UpdateTerrain(FALSE);

	curcount += count;

	count = wp_count;
	for (i = 0;i < count;i++)
	{
		DWORD pos;
		CString id;

		pos = wp_pos[i];
		id = wp_id[i];

		int x = pos % os + x_move;
		int y = pos / os + y_move;

		if (x < 0 || y < 0 || x >= m_IsoSize || y >= m_IsoSize) continue;

		AddWaypoint(id, x + y * m_IsoSize);

		dlg->SetPosition(i + curcount);
		dlg->UpdateWindow();
	}

	UpdateWaypoints(FALSE);

	curcount += count;


	for (i = 0;i < ct_count;i++)
	{
		DWORD pos = ct_pos[i];
		CString tag = ct_tag[i];

		int x = pos % os + x_move;
		int y = pos / os + y_move;

		if (x < 0 || y < 0 || x >= m_IsoSize || y >= m_IsoSize) continue;

		AddCelltag(tag, x + y * m_IsoSize);

		dlg->SetPosition(i + curcount);
		dlg->UpdateWindow();
	}

	UpdateCelltags(FALSE);

	m_noAutoObjectUpdate = FALSE;

	errstream << "Delete old_fd" << endl;
	errstream.flush();

	dlg->DestroyWindow();
	dlg = new(CProgressDlg)("Updating Minimap, please wait");
	count = 0;
	dlg->SetRange(0, m_IsoSize * m_IsoSize);
	dlg->ShowWindow(SW_SHOW);

	if (old_fd) delete[] old_fd;

	errstream << "Init minimap" << endl;
	errstream.flush();

	InitMinimap();

	errstream << "Fill minimap" << endl;
	errstream.flush();

	const bool mp = IsMultiplayer();
	for (i = 0;i < m_IsoSize;i++)
	{
		for (e = 0;e < m_IsoSize;e++)
		{
			Mini_UpdatePos(i, e, mp);

			count++;

		}
		dlg->SetPosition(count);
		dlg->UpdateWindow();
	}

	errstream << "Finished" << endl;
	errstream.flush();

	if (inf) delete[] inf;
	if (str) delete[] str;
	if (unit) delete[] unit;
	if (air) delete[] air;
	if (terrain) delete[] terrain;
	if (wp_id) delete[] wp_id;
	if (wp_pos) delete[] wp_pos;
	if (ct_tag) delete[] ct_tag;
	if (ct_pos) delete[] ct_pos;

	dlg->DestroyWindow();


}

/*
Returns TRUE for all sections that should not be modified using the INI editor,
because they become modified whenever the map is saved by the editor itself.
*/
BOOL CMapData::IsMapSection(LPCSTR lpSectionName)
{
	CString str;
	str = lpSectionName;

	if (str == "IsoMapPack5" || str == "OverlayPack" || str == "OverlayDataPack" ||
		str == "Preview" || str == "PreviewPack" || str == "Map" ||
		str == "Structures" || str == "Terrain" || str == "Units" || str == "Aircraft" || str == "Infantry"
		|| str == "Variables")
		return TRUE;

	return FALSE;
}

int GetEventParamStart(CString& EventData, int param);

BOOL CMapData::IsYRMap()
{
#ifdef TS_MODE
	return FALSE;
#else

	//if(yuri_mode) // always check for this
	{
		if (Map->GetTheater() == THEATER3 || Map->GetTheater() == THEATER4 || Map->GetTheater() == THEATER5)
			return TRUE;

		int i;
		int max = 0;
		if (tiledata == &u_tiledata)
		{
			max = atoi(g_data.sections["RA2TileMax"].values["Urban"]);
		}
		else if (tiledata == &s_tiledata) max = atoi(g_data.sections["RA2TileMax"].values["Snow"]);
		else if (tiledata == &t_tiledata) max = atoi(g_data.sections["RA2TileMax"].values["Temperat"]);

		int yroverlay = atoi(g_data.sections["YROverlay"].values["Begin"]);

		for (i = 0;i < fielddata_size;i++)
		{
			if (fielddata[i].wGround != 0xFFFF && fielddata[i].wGround >= max)
			{
				return TRUE;
			}
			if (fielddata[i].overlay >= yroverlay && fielddata[i].overlay != 0xFF)
				return TRUE;
		}

		int count;

		count = GetInfantryCount();
		for (i = 0;i < count;i++)
		{
			INFANTRY inf;
			GetInfantryData(i, &inf);

			if (inf.deleted) continue;

			CIniFileSection& sec = g_data.sections["YRInfantry"];

			if (sec.values.find(inf.type) != sec.values.end())
			{
				return TRUE;
			}
		}

		count = GetStructureCount();
		for (i = 0;i < count;i++)
		{
			STRUCTURE str;
			GetStructureData(i, &str);

			if (str.deleted) continue;

			CIniFileSection& sec = g_data.sections["YRBuildings"];

			if (sec.values.find(str.type) != sec.values.end())
			{
				return TRUE;
			}
		}

		count = GetUnitCount();
		for (i = 0;i < count;i++)
		{
			UNIT unit;
			GetUnitData(i, &unit);

			if (unit.deleted) continue;

			CIniFileSection& sec = g_data.sections["YRUnits"];

			if (sec.values.find(unit.type) != sec.values.end())
			{
				return TRUE;
			}
		}

		count = GetAircraftCount();
		for (i = 0;i < count;i++)
		{
			AIRCRAFT air;
			GetAircraftData(i, &air);

			if (air.deleted) continue;

			CIniFileSection& sec = g_data.sections["YRAircraft"];

			if (sec.values.find(air.type) != sec.values.end())
			{
				return TRUE;
			}
		}

		count = GetTerrainCount();
		for (i = 0;i < count;i++)
		{
			TERRAIN& tr = m_terrain[i];


			if (tr.deleted) continue;

			CIniFileSection& sec = g_data.sections["YRTerrain"];

			if (sec.values.find(tr.type) != sec.values.end())
			{
				return TRUE;
			}
		}

		count = m_mapfile.sections["Triggers"].values.size();
		for (i = 0;i < count;i++)
		{
			CString event;
			CString action;
			CString id;

			id = *m_mapfile.sections["Triggers"].GetValueName(i);

			event = m_mapfile.sections["Events"].values[id];
			action = m_mapfile.sections["Actions"].values[id];

			int eventcount, actioncount;
			eventcount = atoi(GetParam(event, 0));
			actioncount = atoi(GetParam(action, 0));

			int e;

			for (e = 0;e < eventcount;e++)
			{
				CString type = GetParam(event, GetEventParamStart(event, e));
				if (g_data.sections["EventsRA2"].values.find(type) != g_data.sections["EventsRA2"].values.end())
				{
					if (isTrue(GetParam(g_data.sections["EventsRA2"].values[type], 9)))
						return TRUE;
				}
			}

			for (e = 0;e < actioncount;e++)
			{
				CString type = GetParam(action, 1 + e * 8);
				if (g_data.sections["ActionsRA2"].values.find(type) != g_data.sections["ActionsRA2"].values.end())
				{
					if (isTrue(GetParam(g_data.sections["ActionsRA2"].values[type], 14)))
						return TRUE;
				}
			}
		}
	}

	return FALSE;
#endif
}

#ifdef SMUDGE_SUPP
BOOL CMapData::AddSmudge(SMUDGE* lpSmudge)
{


	SMUDGE td;
	td = *lpSmudge;
	int pos = td.x + td.y * GetIsoSize();

	if (smudgeid.find(td.type) == smudgeid.end()) return FALSE;

	BOOL bFound = FALSE;

	int i;
	for (i = 0;i < m_smudges.size();i++)
	{
		if (m_smudges[i].deleted) // yep, found one, replace it
		{
			m_smudges[i] = td;
			if (pos < fielddata_size)
			{
				fielddata[pos].smudge = i;
				fielddata[pos].smudgetype = smudgeid[td.type];
			}

			bFound = TRUE;
			break;
		}
	}

	if (!bFound)
	{
		m_smudges.push_back(td);

		if (pos < fielddata_size)
		{
			fielddata[pos].smudge = m_smudges.size() - 1;
			fielddata[pos].smudgetype = smudgeid[td.type];
		}
	}


	return TRUE;
}

void CMapData::DeleteSmudge(DWORD dwIndex)
{
	if (m_smudges[dwIndex].deleted) return;

	int x, y;

	x = m_smudges[dwIndex].x;
	y = m_smudges[dwIndex].y;
	m_smudges[dwIndex].deleted = 1;

	int pos = x + y * GetIsoSize();
	if (x + y * m_IsoSize < fielddata_size)
	{
		fielddata[pos].smudge = -1;
		fielddata[pos].smudgetype = -1;
	}
}

void CMapData::UpdateSmudges(BOOL bSave, int num)
{
	vector<SMUDGE>& t = m_smudges;

	if (bSave == FALSE)
	{
		if (m_mapfile.sections.find("Smudge") == m_mapfile.sections.end() || m_mapfile.sections["Smudge"].values.size() <= 0)
			return;

		if (num < 0)
		{
			t.clear();
			t.reserve(100);

			int i;
			for (i = 0;i < GetIsoSize() * GetIsoSize();i++)
			{
				fielddata[i].smudge = -1;
			}



			CIniFileSection& sec = m_mapfile.sections["Smudge"];

			for (i = 0;i < sec.values.size();i++)
			{
				int x, y;
				x = atoi(GetParam(*sec.GetValue(i), 2));
				y = atoi(GetParam(*sec.GetValue(i), 1));

				// check for valid coordinates ; MW May 17th 2001
				ASSERT(x >= 0 && x < GetIsoSize());
				ASSERT(y >= 0 && y < GetIsoSize());
				if (x < 0 || x >= GetIsoSize() || y < 0 || y >= GetIsoSize())
				{
					// invalid coordinates - ignore in release
				}
				else
				{
					SMUDGE td;
					td.deleted = 0;
					td.type = GetParam(*sec.GetValue(i), 0);
					td.x = x;
					td.y = y;

					t.push_back(td);

					int pos = x + y * GetIsoSize();
					fielddata[pos].smudge = i;
					fielddata[pos].smudgetype = smudgeid[td.type];
				}
			}

			m_mapfile.sections.erase("Smudge");

		}

	}
	else
	{

		//if(num<0)
		{
			//if(m_mapfile.sections.find("Smudge")!=m_mapfile.sections.end()) MessageBox(0,"Reupdate!","",0);
			m_mapfile.sections.erase("Smudge");
			int i;

			for (i = 0;i < t.size();i++)
			{
				SMUDGE& td = t[i];
				if (!td.deleted)
				{
					char c[50];
					CString val = td.type;
					val += ",";
					itoa(td.y, c, 10);
					val += c;
					val += ",";
					itoa(td.x, c, 10);
					val += c;
					val += ",0";

					itoa(i, c, 10);

					m_mapfile.sections["Smudge"].values[c] = val;
				}
			}
		}

	}

}

void CMapData::UpdateSmudgeInfo(LPCSTR lpSmudgeType)
{
	CIniFile& ini = GetIniFile();

	if (!lpSmudgeType)
	{
		memset(smudgeinfo, 0, 0x0F00 * sizeof(SMUDGE_INFO));

		int i;
		for (i = 0;i < rules.sections["SmudgeTypes"].values.size();i++)
		{


			CString type = *rules.sections["SmudgeTypes"].GetValue(i);
			CString artname = type;


			int n = GetUnitTypeID(type);

			if (n >= 0 && n < 0x0F00)
			{

				CString lpPicFile = GetUnitPictureFilename(type, 0);

				if (pics.find(lpPicFile) != pics.end())
				{

					smudgeinfo[n].pic = pics[lpPicFile];
				}
				else
					smudgeinfo[n].pic.pic = NULL;
			}

		}

		for (i = 0;i < ini.sections["SmudgeTypes"].values.size();i++)
		{


			CString type = *ini.sections["SmudgeTypes"].GetValue(i);
			CString artname = type;



			int n = Map->GetUnitTypeID(type);

			if (n >= 0 && n < 0x0F00)
			{
				//smudgeinfo[n].w=w;
				//smudgeinfo[n].h=h;

				CString lpPicFile = GetUnitPictureFilename(type, 0);

				if (pics.find(lpPicFile) != pics.end())
				{
					smudgeinfo[n].pic = pics[lpPicFile];
				}
				else
					smudgeinfo[n].pic.pic = NULL;
			}

		}
	}
	else
	{
		CString type = lpSmudgeType;
		CString artname = type;



		int n = Map->GetUnitTypeID(type);

		if (n >= 0 && n < 0x0F00)
		{

			CString lpPicFile = GetUnitPictureFilename(type, 0);
			if (pics.find(lpPicFile) != pics.end())
			{

				smudgeinfo[n].pic = pics[lpPicFile];
			}
			else
				smudgeinfo[n].pic.pic = NULL;
		}

	}


}

void CMapData::GetSmudgeData(DWORD dwIndex, SMUDGE* lpData) const
{
	ASSERT(m_smudges.size() > dwIndex);


	*lpData = m_smudges[dwIndex];//*m_mapfile.sections["Terrain"].GetValue(dwIndex);
}
#endif




BOOL CMapData::GetInfantryINIData(int index, CString* lpINI)
{
	ASSERT(index < m_infantry.size());

	if (index >= m_infantry.size() || index < 0) return FALSE;

	INFANTRY& infantry = m_infantry[index];
	CString value;
	value = infantry.house + "," + infantry.type + "," + infantry.strength + "," + infantry.y +
		"," + infantry.x + "," + infantry.pos + "," + infantry.action + "," + infantry.direction + "," +
		infantry.tag + "," + infantry.flag1 + "," + infantry.flag2 + "," + infantry.flag3 + "," +
		infantry.flag4 + "," + infantry.flag5;

	*lpINI = value;

	return TRUE;

}

void CMapData::RedrawMinimap()
{
	const bool mp = IsMultiplayer();
	int i, e;
	for (i = 0; i < m_IsoSize; i++)
		for (e = 0; e < m_IsoSize; e++)
			Mini_UpdatePos(i, e, mp);
}
