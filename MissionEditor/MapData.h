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

// Map->h: Interface for the class CMap.
//
// CMap holds all information for the current map
//
//////////////////////////////////////////////////////////////////////

#include "structs.h"
#include <vector>
#include "variables.h"
#include "macros.h"
#include "ovrlinline.h"
#include "Tube.h"


extern TILEDATA** tiledata;
extern DWORD* tiledata_count;
extern ofstream errstream;
extern map<int, int> tilesets_start;
extern CIniFile* tiles;
extern CFinalSunApp theApp;
extern int shoreset;

extern CIniFile rules;

extern TILEDATA* un_tiledata;
extern CIniFile g_data;

extern int ramp2set;
extern int pave2set;
extern int ramp2set_start;
extern int pave2set_start;
extern int rampset_start;
extern int rampset;



#if !defined(AFX_MAP_H__9278CAC0_D4E7_11D3_B63B_444553540001__INCLUDED_)
#define AFX_MAP_H__9278CAC0_D4E7_11D3_B63B_444553540001__INCLUDED_

#include "IniFile.h"	// Hinzugefügt von der Klassenansicht
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <array>

#define MAPDATA_UPDATE_FROM_INI 0
#define MAPDATA_UPDATE_TO_INI 1
#define MAPDATA_UPDATE_TO_INI_ALL 2
#define MAPDATA_UPDATE_TO_INI_ALL_PREVIEW 4
#define MAPDATA_UPDATE_TO_INI_ALL_COMPRESSED 8

struct NODEDATA
{
	NODEDATA();
	int type;
	int index;
	CString house;
};

// mapfielddata is the data of every field in an extracted isomappack!
struct MAPFIELDDATA
{
	unsigned short wX;
	unsigned short wY;
	WORD wGround;
	BYTE bData[3];
	BYTE bHeight;
	BYTE bData2[1];
};
#define MAPFIELDDATA_SIZE 11

/*
struct TILEDATA{};

contains the information needed for one field of the map.
*/
struct FIELDDATA
{
	FIELDDATA();
	short unit; // unit number
	short infantry[SUBPOS_COUNT]; // infantry number
	short aircraft; // aircraft number
	short structure; // structure number 
	short structuretype; // structure type id
	short terrain; // terrain number
	int terraintype; // terrain type id
#ifdef SMUDGE_SUPP
	short smudge;
	int smudgetype;
#endif
	short waypoint; // waypoint number

	NODEDATA node; // node info
	BYTE overlay; // overlay number
	BYTE overlaydata; // overlay data info
	WORD wGround; // ground type (tile)
	WORD bMapData; // add. data
	BYTE bSubTile;
	BYTE bHeight; // height of tile
	BYTE bMapData2; // add. data2
	short celltag; // celltag uses	

	//std::uint16_t wTubeId; // tube ID
	//char cTubePart; // 0 is start, 1 is exit, and 2-101 are tube parts
	unsigned bReserved : 1; // for program usage
	unsigned bHide : 1;
	unsigned bRedrawTerrain : 1; // force redraw
	unsigned bCliffHack : 1;
	unsigned bRNDImage : 4; // for using a,b,c of tmp tiles
};

struct SNAPSHOTDATA
{
	SNAPSHOTDATA();
	int left;
	int top;
	int bottom;
	int right;

	BOOL* bRedrawTerrain;
	BYTE* overlay;
	BYTE* overlaydata;
	WORD* wGround;
	WORD* bMapData;
	BYTE* bSubTile;
	BYTE* bHeight;
	BYTE* bMapData2;
	BYTE* bRNDData;
	//CIniFile mapfile;
};



class CMapData
{
public:

	BOOL m_noAutoObjectUpdate;

	void SetFielddataAt(DWORD dwPos, FIELDDATA* lpFd)
	{
		if (dwPos >= fielddata_size) return;

		RemoveOvrlMoney(fielddata[dwPos].overlay, fielddata[dwPos].overlaydata);
		fielddata[dwPos] = (*lpFd);
		AddOvrlMoney(fielddata[dwPos].overlay, fielddata[dwPos].overlaydata);
	};

	DWORD GetMapPos(int mapX, int mapY) const
	{
		return mapX + mapY * m_IsoSize;
	}
	DWORD GetMapPos(const MapCoords& coords) const
	{
		return coords.x + coords.y * m_IsoSize;
	}

	bool hasLat(WORD wTileSet) const;

	void HideField(DWORD dwPos, BOOL bHide);
	void SetReserved(DWORD dwPos, BYTE val);
	DWORD GetTileID(DWORD dwTileSet, int iTile);
	int GetNecessarySlope(DWORD dwPos);
	void CreateSlopesAt(DWORD dwPos)
	{
		//OutputDebugString("CreateSlopes()\n");

		FIELDDATA m = *GetFielddataAt(dwPos);
		if (m.wGround == 0xFFFF) m.wGround = 0;

		TILEDATA& d = (*tiledata)[m.wGround];

		int ns = -1;
		int i, e, p = 0;
		int h[3][3];
		for (i = 0;i < 3; i++)
		{
			for (e = 0;e < 3;e++)
			{
				int pos = dwPos + (i - 1) + (e - 1) * m_IsoSize;
				if (pos < 0 || pos >= m_IsoSize * m_IsoSize)
				{
					h[i][e] = 0;
				}
				else
				{
					FIELDDATA m2 = *GetFielddataAt(pos);

					h[i][e] = m.bHeight - m2.bHeight;
				}


			}
		}

		// check if the current tile must be heightened anyway
		if (!theApp.m_Options.bDisableSlopeCorrection && d.bMorphable)
		{
			if ((h[0][1] < 0 && h[2][1] < 0) || (h[1][0] < 0 && h[1][2] < 0)

				|| (h[1][0] < 0 && h[0][2] < 0 && h[0][1] >= 0)
				|| (h[1][0] < 0 && h[2][2] < 0 && h[2][1] >= 0)

				|| (h[0][1] < 0 && h[2][0] < 0 && h[1][0] >= 0)
				|| (h[0][1] < 0 && h[2][2] < 0 && h[1][2] >= 0)

				|| (h[1][2] < 0 && h[0][0] < 0 && h[0][1] >= 0)
				|| (h[1][2] < 0 && h[2][0] < 0 && h[2][1] >= 0)

				|| (h[2][1] < 0 && h[0][0] < 0 && h[1][0] >= 0)
				|| (h[2][1] < 0 && h[0][2] < 0 && h[1][2] >= 0)

				|| (h[1][0] < 0 && h[0][1] < 0 && h[0][0] >= 0)
				|| (h[0][1] < 0 && h[1][2] < 0 && h[0][2] >= 0)
				|| (h[1][2] < 0 && h[2][1] < 0 && h[2][2] >= 0)
				|| (h[2][1] < 0 && h[1][0] < 0 && h[2][0] >= 0)

				)
			{
				SetHeightAt(dwPos, m.bHeight + 1);
				for (i = -1;i < 2;i++)
					for (e = -1;e < 2;e++)
						CreateSlopesAt(dwPos + i + e * m_IsoSize);

				return;
			}
		}



		BOOL checkOtherSlopes = FALSE;


		if (h[0][0] == -1 && h[2][2] == -1 && h[2][0] >= 0 && h[0][2] >= 0 && h[1][0] >= 0 && h[1][2] >= 0 && h[0][1] >= 0 && h[2][1] >= 0) ns = SLOPE_UP_LEFTTOP_AND_RIGHTBOTTOM;
		if (h[0][2] == -1 && h[2][0] == -1 && h[0][0] >= 0 && h[2][2] >= 0 && h[0][1] >= 0 && h[1][0] >= 0 && h[1][2] >= 0 && h[2][1] >= 0) ns = SLOPE_UP_LEFTBOTTOM_AND_RIGHTTOP;




		if (ns == -1)
			if (h[1][0] == -1 && h[0][1] != -1 && h[1][2] != -1 && h[2][1] != -1)
			{
				ns = SLOPE_UP_LEFT;
			}
			else if (h[0][1] == -1 && h[1][0] != -1 && h[2][1] != -1 && h[1][2] != -1)
			{
				ns = SLOPE_UP_TOP;
			}
			else if (h[1][2] == -1 && h[0][1] != -1 && h[1][0] != -1 && h[2][1] != -1)
			{
				ns = SLOPE_UP_RIGHT;
			}
			else if (h[2][1] == -1 && h[0][1] != -1 && h[1][0] != -1 && h[1][2] != -1)
			{
				ns = SLOPE_UP_BOTTOM;
			}

		if (ns == -1)
		{
			if (h[0][0] == -2) ns = SLOPE_DOWN_BOTTOM;
			if (h[2][0] == -2) ns = SLOPE_DOWN_RIGHT;
			if (h[0][2] == -2) ns = SLOPE_DOWN_LEFT;
			if (h[2][2] == -2) ns = SLOPE_DOWN_TOP;
		}

		if (ns == -1 && h[0][0] == -1)
		{
			if (h[1][0] == -1 && h[0][1] == -1) ns = SLOPE_DOWN_RIGHTBOTTOM;
			else if (h[1][0] == 0 && h[0][1] == 0) ns = SLOPE_UP_LEFTTOP;
			//else if(h[2][2]==1) ns=SLOPE_DOWN_BOTTOM;
		}

		if (ns == -1 && h[2][0] == -1)
		{
			if (h[1][0] == -1 && h[2][1] == -1) ns = SLOPE_DOWN_RIGHTTOP;
			else if (h[1][0] == 0 && h[2][1] == 0) ns = SLOPE_UP_LEFTBOTTOM;
			//else if(h[0][2]==1) ns=SLOPE_DOWN_RIGHT;
		}
		if (ns == -1 && h[0][2] == -1)
		{
			if (h[1][2] == -1 && h[0][1] == -1) ns = SLOPE_DOWN_LEFTBOTTOM;
			else if (h[1][2] == 0 && h[0][1] == 0) ns = SLOPE_UP_RIGHTTOP;
			//else if(h[2][0]==1) ns=SLOPE_DOWN_LEFT;
		}
		if (ns == -1 && h[2][2] == -1)
		{
			if (h[1][2] == -1 && h[2][1] == -1) ns = SLOPE_DOWN_LEFTTOP;
			else if (h[1][2] == 0 && h[2][1] == 0) ns = SLOPE_UP_RIGHTBOTTOM;
			//else if(h[0][0]==1) ns=SLOPE_DOWN_TOP;
		}

		if (ns == -1 && h[1][0] == -1 && h[2][1] == -1) ns = SLOPE_DOWN_RIGHTTOP;
		if (ns == -1 && h[1][2] == -1 && h[2][1] == -1) ns = SLOPE_DOWN_LEFTTOP;
		if (ns == -1 && h[1][0] == -1 && h[0][1] == -1) ns = SLOPE_DOWN_RIGHTBOTTOM;
		if (ns == -1 && h[1][2] == -1 && h[0][1] == -1) ns = SLOPE_DOWN_LEFTBOTTOM;


		int rampbase = rampset_start;//atoi((*tiles).sections["General"].values["RampBase"]);
		int rampsmooth = atoi((*tiles).sections["General"].AccessValueByName("RampSmooth"));

		if (ns == -1 && (d.wTileSet == rampset || d.wTileSet == rampsmooth) && d.bMorphable)
		{
			SetTileAt(dwPos, 0, 0);
		}
		if (tiledata == &un_tiledata)
		{
			int r = ramp2set;
			int m = pave2set;

			if (ns == -1 && (d.wTileSet == r || d.wTileSet == m) && d.bMorphable)
			{
				SetTileAt(dwPos, pave2set_start/*GetTileID(m,0)*/, 0);
			}
		}
		if (d.bMorphable && ns != -1)
		{
			if (tiledata == &un_tiledata) // NEW URBAN FIX FOR URBAN PAVEMENT
			{

				//[NewUrbanInfo]
				//Morphable2=114
				//Ramps2=117
				int r = ramp2set;
				int m = pave2set;
				if (d.wTileSet == r || d.wTileSet == m)
					rampbase = ramp2set_start;


			}

			SetTileAt(dwPos, rampbase + ns - 1, 0);

		}


	}
	void CreateMap(DWORD dwWidth, DWORD dwHeight, LPCTSTR lpTerrainType, DWORD dwGroundHeight);
	BOOL SetTileAt(DWORD dwPos, DWORD dwID, DWORD dwTile)
	{
		if (dwPos > fielddata_size) return FALSE;

		int replacement = 0; // MW fix: ignore for bridges
		if ((*tiledata)[dwID].bReplacementCount && atoi((*tiles).sections["General"].AccessValueByName("BridgeSet")) != (*tiledata)[dwID].wTileSet)
		{
			replacement = rand() * (1 + (*tiledata)[dwID].bReplacementCount) / RAND_MAX;
		}

		fielddata[dwPos].wGround = dwID;
		fielddata[dwPos].bSubTile = dwTile;
		fielddata[dwPos].bRNDImage = replacement;

		int e;
		fielddata[dwPos].bRedrawTerrain = FALSE;
		int xx, yy;
		for (xx = -2;xx < 0;xx++)
		{
			for (yy = -2;yy < 0;yy++)
			{
				int npos = dwPos + xx + yy * m_IsoSize;
				if (npos > 0 && fielddata[dwPos].bHeight - fielddata[npos].bHeight >= 4)
				{
					fielddata[dwPos].bRedrawTerrain = TRUE;
					break;
				}
			}
			if (fielddata[dwPos].bRedrawTerrain) break;
		}

		Mini_UpdatePos(dwPos % m_IsoSize, dwPos / m_IsoSize, IsMultiplayer());

		return TRUE;
	}

	void SetHeightAt(DWORD dwPos, BYTE bHeight)
	{
		int height = (char)bHeight;
		if (height > MAXHEIGHT) height = MAXHEIGHT; // too high else
		if (height < 0) height = 0;
		if (dwPos < fielddata_size) fielddata[dwPos].bHeight = height;
	}


	int GetBuildingID(LPCSTR lpBuildingName);
	void ImportRUL(LPCTSTR lpFilename);
	void ExportRulesChanges(const char* filename);
	void DeleteRulesSections();
	DWORD GetWaypointCount() const;
	DWORD GetCelltagCount() const;
	WCHAR* GetUnitName(LPCTSTR lpID) const;
	DWORD GetTerrainCount() const;
	DWORD GetAircraftCount() const;
	DWORD GetStructureCount() const;
	DWORD GetUnitCount() const;
	DWORD GetInfantryCount() const;
	void GetStdUnitData(DWORD dwIndex, STDOBJECTDATA* lpStdUnit) const;
	void GetStdAircraftData(DWORD dwIndex, STDOBJECTDATA* lpStdAircraft) const;
	void GetWaypointData(DWORD dwIndex, CString* lpID, DWORD* lpdwPos) const;
	BOOL IsGroundObjectAt(DWORD dwPos) const;
	BOOL AddTerrain(LPCTSTR lpType, DWORD dwPos, int suggestedIndex = -1);
	void GetTerrainData(DWORD dwIndex, CString* lpType) const;
	void GetTerrainData(DWORD dwIndex, TERRAIN* lpTerrain) const;
	BOOL AddUnit(UNIT* lpUnit, LPCTSTR lpType = NULL, LPCTSTR lpHouse = NULL, DWORD dwPos = 0, CString suggestedID = "");
	BOOL AddAircraft(AIRCRAFT* lpAircraft, LPCTSTR lpType = NULL, LPCTSTR lpHouse = NULL, DWORD dwPos = 0, CString suggestedID = "");
	void GetCelltagData(DWORD dwIndex, CString* lpTag, DWORD* lpdwPos) const;
	BOOL AddCelltag(LPCTSTR lpTag, DWORD dwPos);
	void GetAircraftData(DWORD dwIndex, AIRCRAFT* lpAircraft) const;
	void GetUnitData(DWORD dwIndex, UNIT* lpUnit) const;
	void GetInfantryData(DWORD dwIndex, INFANTRY* lpInfantry) const;
	void GetStdInfantryData(DWORD dwIndex, STDOBJECTDATA* lpStdInfantry) const;
	INT GetUnitTypeID(LPCTSTR lpType);
	void InitializeUnitTypes();
	BOOL AddStructure(STRUCTURE* lpStructure, LPCTSTR lpType = NULL, LPCTSTR lpHouse = NULL, DWORD dwPos = 0, CString suggestedID = "");
	BOOL AddInfantry(INFANTRY* lpInfantry, LPCTSTR lpType = NULL, LPCTSTR lpHouse = NULL, DWORD dwPos = 0, int suggestedIndex = -1);
	BOOL AddNode(NODE* lpNode, WORD dwPos);
	void GetStdStructureData(DWORD dwIndex, STDOBJECTDATA* lpStdStructure) const;
	void GetStructureData(DWORD dwIndex, STRUCTURE* lpStructure) const;
	BOOL AddWaypoint(CString lpID, DWORD dwPos);

	void DeleteNode(LPCTSTR lpHouse, DWORD dwIndex);
	void DeleteTerrain(DWORD dwIndex);
	void DeleteAircraft(DWORD dwIndex);
	void DeleteStructure(DWORD dwIndex);
	void DeleteUnit(DWORD dwIndex);
	void DeleteCelltag(DWORD dwIndex);
	void DeleteWaypoint(DWORD dwIndex);
	void DeleteInfantry(DWORD dwIndex);

	INT GetCelltagAt(DWORD dwPos) const
	{
		return fielddata[dwPos].celltag;
	}
	INT GetCelltagAt(MapCoords pos) const
	{
		return GetCelltagAt(GetMapPos(pos));
	}
	INT GetWaypointAt(DWORD dwPos) const
	{
		return fielddata[dwPos].waypoint;
	}
	INT GetWaypointAt(MapCoords pos) const
	{
		return GetWaypointAt(GetMapPos(pos));
	}
	INT GetTerrainAt(DWORD dwPos) const
	{
		return fielddata[dwPos].terrain;
	}
	INT GetTerrainAt(MapCoords pos) const
	{
		return GetTerrainAt(GetMapPos(pos));
	}
	INT GetAirAt(DWORD dwPos) const
	{
		return fielddata[dwPos].aircraft;
	}
	INT GetAirAt(MapCoords pos) const
	{
		return GetAirAt(GetMapPos(pos));
	}
	INT GetStructureAt(DWORD dwPos) const
	{
		if (fielddata[dwPos].structure > -1) return fielddata[dwPos].structure;	return -1;
	}
	INT GetStructureAt(MapCoords pos) const
	{
		return GetStructureAt(GetMapPos(pos));
	}
	INT GetUnitAt(DWORD dwPos) const
	{
		return fielddata[dwPos].unit;
	}
	INT GetUnitAt(MapCoords pos) const
	{
		return GetUnitAt(GetMapPos(pos));
	}
	INT GetInfantryAt(DWORD dwPos, DWORD dwSubPos = 0xFFFFFFFF) const
	{
		if (dwSubPos == 0xFFFFFFFF)
		{
			int i;
			for (i = 0;i < SUBPOS_COUNT;i++)
				if (fielddata[dwPos].infantry[i] != -1)
					return fielddata[dwPos].infantry[i];
			return -1;
		}
		return fielddata[dwPos].infantry[dwSubPos];
	}
	INT GetInfantryAt(MapCoords pos, DWORD dwSubPos = 0xFFFFFFFF)
	{
		return GetInfantryAt(GetMapPos(pos), dwSubPos);
	}
	INT GetNodeAt(DWORD dwPos, CString& lpHouse) const;
	INT GetNodeAt(MapCoords pos, CString& lpHouse) const
	{
		return GetNodeAt(GetMapPos(pos), lpHouse);
	}
	INT GetHeightAt(DWORD dwPos) const
	{
		return fielddata[dwPos].bHeight;
	}
	INT GetHeightAt(const MapCoords& coords) const
	{
		return GetHeightAt(GetMapPos(coords));
	}

	const FIELDDATA* GetFielddataAt(DWORD dwPos) const
	{
		if (dwPos >= fielddata_size)
		{
			outside_f.bReserved = 1;
			return &outside_f;
		}

		return &fielddata[dwPos];
	};

	FIELDDATA* GetFielddataAt(DWORD dwPos)
	{
		if (dwPos >= fielddata_size)
		{
			outside_f.bReserved = 1;
			return &outside_f;
		}

		return &fielddata[dwPos];
	};

	const FIELDDATA* GetFielddataAt(const MapCoords& pos) const
	{
		auto dwPos = GetMapPos(pos);
		if (dwPos >= fielddata_size)
		{
			outside_f.bReserved = 1;
			return &outside_f;
		}

		return &fielddata[dwPos];
	};

	FIELDDATA* GetFielddataAt(const MapCoords& pos)
	{
		auto dwPos = GetMapPos(pos);
		if (dwPos >= fielddata_size)
		{
			outside_f.bReserved = 1;
			return &outside_f;
		}

		return &fielddata[dwPos];
	};

	BYTE GetOverlayDataAt(DWORD dwPos);
	void SetOverlayDataAt(DWORD dwPos, BYTE bValue);
	BYTE GetOverlayAt(DWORD dwPos);
	void SetOverlayAt(DWORD dwPos, BYTE bValue);
	void ClearOverlay();
	void ClearOverlayData();

	const std::vector<std::unique_ptr<CTube>>& GetTubes() const
	{
		return m_tubes;
	}

	DWORD GetIsoSize() const
	{
		return m_IsoSize;
	}
	void LoadMap(const std::string& file);
	void UpdateIniFile(DWORD dwFlags = MAPDATA_UPDATE_TO_INI);
	CIniFile& GetIniFile();
	CString GetAITriggerTypeID(DWORD dwAITriggerType);
	DWORD GetAITriggerTypeIndex(LPCTSTR lpID);
	WORD GetHouseIndex(LPCTSTR lpHouse);
	void GetAITriggerType(DWORD dwAITriggerType, AITRIGGERTYPE* pAITrg);
	DWORD GetAITriggerTypeCount();
	CString GetHouseID(WORD wHouse, BOOL bCountry = FALSE);
	WORD GetHousesCount(BOOL bCountries = FALSE);
	WORD GetHeight() const
	{
		return m_maprect.bottom;
	};
	WORD GetWidth() const
	{
		return m_maprect.right;
	};
	BOOL IsRulesSection(LPCTSTR lpSection);

	CMapData();
	virtual ~CMapData();
	void Pack(BOOL bCreatePreview = FALSE, BOOL bCompression = FALSE);
	void Unpack();
	void UpdateTreeInfo(LPCSTR lpTreeType = NULL);
	void UpdateBuildingInfo(LPCSTR lpUnitType = NULL);
	void CalcMapRect();

	// MW change: UpdateStructures() public, so that houses dialog can access it
	void UpdateStructures(BOOL bSave = FALSE);

	MapCoords ToMapCoords(ProjectedCoords xy) const;
	MapCoords ToMapCoords3d(ProjectedCoords xy, int mapZ) const;
	MapCoords ToMapCoords3d(ProjectedCoords xy, bool bAllowAccessBehindCliffs=false, bool ignoreHideFlagsAndOutside=false) const;
	ProjectedCoords ProjectCoords(MapCoords xy) const;
	ProjectedCoords ProjectCoords3d(MapCoords xy) const;
	ProjectedCoords ProjectCoords3d(MapCoords xy, int z) const;
	bool isInside(MapCoords xy) const;

	__forceinline CPoint GetMiniMapPos(MapCoords mapCoords)
	{
		int x, y;
		GetMiniMapPos(mapCoords.x, mapCoords.y, x, y);
		return CPoint(static_cast<LONG>(x), static_cast<LONG>(y));
	}

private:
	void UpdateTubes(BOOL bSave);
	MAPFIELDDATA* GetMappackPointer(DWORD dwPos);

	void UpdateMapFieldData(BOOL bSave = FALSE);

	DWORD m_IsoSize;
	mutable FIELDDATA outside_f;
	BOOL isInitialized;
	void UpdateCelltags(BOOL bSave = FALSE);
	void UpdateOverlay(BOOL bSave = FALSE);
	void UpdateNodes(BOOL bSave = FALSE);
	void UpdateWaypoints(BOOL bSave = FALSE);
	void UpdateUnits(BOOL bSave = FALSE);
	void UpdateTerrain(BOOL bSave = FALSE, int num = -1);
	void UpdateInfantry(BOOL bSave = FALSE);
	void UpdateAircraft(BOOL bSave = FALSE);





	map<CString, int> buildingid;
	map<CString, int> terrainid;
#ifdef SMUDGE_SUPP
	map<CString, int> smudgeid;
#endif
	BYTE m_Overlay[262144]; // overlay byte values (extracted)
	BYTE m_OverlayData[262144]; // overlay data byte values (extracted)
	BYTE* m_mfd;	// map field data buffer
	DWORD dwIsoMapSize;
	CIniFile m_mapfile;
	RECT m_maprect;
	RECT m_vismaprect;
	FIELDDATA* fielddata;
	int fielddata_size;
	SNAPSHOTDATA* m_snapshots;
	DWORD dwSnapShotCount;
	int m_cursnapshot;
	int m_money;

	vector<STRUCTUREPAINT> m_structurepaint;


protected:

	void InitMinimap();
	vector<std::unique_ptr<CTube>> m_tubes;

	// vectors for terrain, infantry, structures and units, as those need to be displayed very fast.
	// we don´t need them for aircraft right now, as there won´t be many aircrafts on the map anyway.
#ifdef SMUDGE_SUPP
	vector<SMUDGE> m_smudges;
#endif
	vector<TERRAIN> m_terrain;
	vector<INFANTRY> m_infantry;
	vector<UNIT> m_units;
	vector<STRUCTURE> m_structures;

	// we use a dib to draw the minimap
	std::vector<BYTE> m_mini_colors;
	BITMAPINFO m_mini_biinfo;
	int m_mini_pitch;

	enum OverlayCreditsType
	{
		OverlayCredits_Riparius = 0,
		OverlayCredits_Cruentus = 1,
		OverlayCredits_Vinifera = 2,
		OverlayCredits_Aboreus = 3,
		OverlayCredits_NumOf
	};

	std::array<int, OverlayCredits_NumOf> m_overlayCredits;


	__forceinline void RemoveOvrlMoney(unsigned char ovrl, unsigned char ovrld)
	{
		if (ovrl >= RIPARIUS_BEGIN && ovrl <= RIPARIUS_END)
		{
			m_money -= (ovrld + 1) * m_overlayCredits[OverlayCredits_Riparius];
		}

		if (ovrl >= CRUENTUS_BEGIN && ovrl <= CRUENTUS_END)
		{
			m_money -= (ovrld + 1) * m_overlayCredits[OverlayCredits_Cruentus];
		}

		if (ovrl >= VINIFERA_BEGIN && ovrl <= VINIFERA_END)
		{
			m_money -= (ovrld + 1) * m_overlayCredits[OverlayCredits_Vinifera];
		}

		if (ovrl >= ABOREUS_BEGIN && ovrl <= ABOREUS_END)
		{
			m_money -= (ovrld + 1) * m_overlayCredits[OverlayCredits_Aboreus];
		}
	}

	__forceinline void AddOvrlMoney(unsigned char ovrl, unsigned char ovrld)
	{
		if (ovrl >= RIPARIUS_BEGIN && ovrl <= RIPARIUS_END)
		{
			m_money += (ovrld + 1) * m_overlayCredits[OverlayCredits_Riparius];
		}

		if (ovrl >= CRUENTUS_BEGIN && ovrl <= CRUENTUS_END)
		{
			m_money += (ovrld + 1) * m_overlayCredits[OverlayCredits_Cruentus];
		}

		if (ovrl >= VINIFERA_BEGIN && ovrl <= VINIFERA_END)
		{
			m_money += (ovrld + 1) * m_overlayCredits[OverlayCredits_Vinifera];
		}

		if (ovrl >= ABOREUS_BEGIN && ovrl <= ABOREUS_END)
		{
			m_money += (ovrld + 1) * m_overlayCredits[OverlayCredits_Aboreus];
		}
	}

	__forceinline void GetMiniMapPos(int i, int e, int& x, int& y)
	{
		const int pheight = m_mini_biinfo.bmiHeader.biHeight;

		const DWORD dwIsoSize = m_IsoSize;
		y = e / 2 + i / 2;
		x = dwIsoSize - i + e;

		int tx, ty;
		tx = GetWidth();
		ty = GetHeight();

		ty = ty / 2 + tx / 2;
		tx = dwIsoSize - GetWidth() + GetHeight();

		x -= tx;
		y -= ty;

		x += pheight;
		y += pheight / 2;
	}

	
	__forceinline void Mini_UpdatePos(const int i, const int e, bool isMultiplayer)
	{
		const int pwidth = m_mini_biinfo.bmiHeader.biWidth;
		const int pheight = m_mini_biinfo.bmiHeader.biHeight;

		if (m_mini_colors.empty() || !tiledata)
			return;

		
		const DWORD dwIsoSize = m_IsoSize;
		const int pitch = m_mini_pitch;

		int x = 0;
		int y = 0;
		GetMiniMapPos(i, e, x, y);
		y = pheight - y - 1;

		int xiso = i;
		int yiso = e;

		if (xiso >= m_IsoSize)
			xiso = m_IsoSize - 1;
		if (yiso >= m_IsoSize)
			yiso = m_IsoSize - 1;
		if (xiso < 0)
			xiso = 0;
		if (yiso < 0)
			yiso = 0;

		DWORD dwPos = xiso + yiso * dwIsoSize;

		int dwDrawPos = (x * 3 + y * pitch);

		int size = pitch * pheight;

		if (dwDrawPos >= size || x >= pwidth || y >= pheight || x < 0 || y < 0) return;
		if (dwPos >= m_IsoSize * m_IsoSize) return;
		if (dwDrawPos + 3 >= m_mini_colors.size()) return;

		RGBTRIPLE& col = (RGBTRIPLE&)m_mini_colors[dwDrawPos];
		RGBTRIPLE& col_r = (RGBTRIPLE&)m_mini_colors[(dwDrawPos + sizeof(RGBTRIPLE)) < size ? dwDrawPos + sizeof(RGBTRIPLE) : dwDrawPos];


		FIELDDATA td;
		td = *GetFielddataAt(dwPos);



		STDOBJECTDATA sod;
		sod.house = "";
		int ic;
		for (ic = 0;ic < SUBPOS_COUNT;ic++)
		{
			if (td.infantry[ic] >= 0)
			{
				GetStdInfantryData(td.infantry[ic], &sod);
			}
		}
		if (td.structure >= 0)
		{
			GetStdStructureData(td.structure, &sod);
		}
		if (td.aircraft >= 0)
		{
			GetStdAircraftData(td.aircraft, &sod);
		}
		if (td.unit >= 0)
		{
			GetStdUnitData(td.unit, &sod);
		}


		int ground = (td.wGround >= (*tiledata_count)) ? 0 : td.wGround;
		int subt = td.bSubTile;
		int ttype = 0;
		if (subt < (*tiledata)[ground].wTileCount) ttype = (*tiledata)[ground].tiles[subt].bTerrainType;


		// mw added check:
		if (subt >= (*tiledata)[ground].wTileCount) return;

		RGBTRIPLE& l = (*tiledata)[ground].tiles[subt].rgbLeft;
		RGBTRIPLE& r = (*tiledata)[ground].tiles[subt].rgbRight;

		int pos = GetHeightAt(dwPos);

		col.rgbtBlue = l.rgbtBlue;
		col.rgbtGreen = l.rgbtGreen;
		col.rgbtRed = l.rgbtRed;
		col_r.rgbtBlue = r.rgbtBlue;
		col_r.rgbtGreen = r.rgbtGreen;
		col_r.rgbtRed = r.rgbtRed;

		if (isGreenTiberium(td.overlay))
		{
#ifndef RA2_MODE
			col.rgbtBlue = 0;
			col.rgbtGreen = 200;
			col.rgbtRed = 0;
			col_r = col;
#else
			col.rgbtBlue = 0;
			col.rgbtGreen = 250;
			col.rgbtRed = 250;
			col_r = col;
#endif
		}
		else if (td.overlay == OVRL_VEINS)
		{
			col.rgbtBlue = 120;
			col.rgbtGreen = 180;
			col.rgbtRed = 190;
			col_r = col;
		}
		else if (td.overlay == OVRL_VEINHOLE || td.overlay == OVRL_VEINHOLEBORDER)
		{
			col.rgbtBlue = 120;
			col.rgbtGreen = 160;
			col.rgbtRed = 165;
			col_r = col;
		}
		else if (td.overlay != 0xFF)
		{
			col.rgbtBlue = 20;
			col.rgbtGreen = 20;
			col.rgbtRed = 20;
			col_r = col;
		}


		if (sod.house.GetLength() > 0)
		{
			/*
			if(strstr(sod.house, houses[1].name))
			{
				col.rgbtRed=255;
				col.rgbtGreen=0;
				col.rgbtBlue=0;
			}
			else if(strstr(sod.house,houses[0].name))
			{
				col.rgbtRed=180;
				col.rgbtGreen=180;
				col.rgbtBlue=0;
			}
			else
			{
				col.rgbtRed=200;
				col.rgbtGreen=200;
				col.rgbtBlue=200;
			}*/
			COLORREF c = ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->GetColor(sod.house);

			col.rgbtRed = GetRValue(c);
			col.rgbtBlue = GetBValue(c);
			col.rgbtGreen = GetGValue(c);
			col_r = col;
		}

		// MW: ADD: make red start pos dots
		if (isMultiplayer)
		{
			CString id;
			DWORD p;
			int wp = td.waypoint;
			BOOL startpos = FALSE;
			int i, e;
			for (i = -1;i < 2;i++)
			{
				for (e = -1;e < 2;e++)
				{
					if (dwPos + i + e * m_IsoSize < fielddata_size)
					{
						int w = GetWaypointAt(dwPos + i + e * m_IsoSize);
						if (w >= 0)
						{
							GetWaypointData(w, &id, &p);
							if (atoi(id) < 8)
							{
								startpos = TRUE;
								break;
							}
						}


					}
				}
				if (startpos) break;
			}
			if (startpos)
			{
				col.rgbtBlue = 0;
				col.rgbtGreen = 0;
				col.rgbtRed = 255;
				col_r = col;

			}
		}
	}


	// helper function. Is val==iSet1 or val=iSet2?	
	__forceinline BOOL st(int val, int iSet)
	{

		if (val == iSet) return TRUE;
		return FALSE;
	}


public:
	void RedrawMinimap();
	BOOL GetInfantryINIData(int index, CString* lpINI);


#ifdef SMUDGE_SUPP
	void UpdateSmudges(BOOL bSave = FALSE, int num = -1);
	void DeleteSmudge(DWORD dwIndex);
	BOOL AddSmudge(SMUDGE* lpSmudge);
	void GetSmudgeData(DWORD dwIndex, SMUDGE* lpData) const;
	void UpdateSmudgeInfo(LPCSTR lpSmudgeType = NULL);
#endif

	BOOL IsYRMap();
	BOOL IsMapSection(LPCSTR lpSectionName);
	void ResizeMap(int iLeft, int iTop, DWORD dwNewWidth, DWORD dwNewHeight);
	void SmoothTiberium(DWORD dwPos);
	int GetPowerOfHouse(LPCTSTR lpHouse);
	int GetMoneyOnMap() const;
	int CalcMoneyOnMap();
	void GetMinimap(BYTE** lpData, BITMAPINFO* lpBI, int* pitch);
	void GetStructurePaint(int index, STRUCTUREPAINT* lpStructurePaint) const;
	void Paste(int x, int y, int z_mod);
	void Copy(int left = 0, int top = 0, int right = 0, int bottom = 0);
	CString GetTheater();
	BOOL IsMultiplayer();
	void CreateShore(int left, int top, int right, int bottom, BOOL bRemoveUseless = TRUE);
	void Redo();
	void SmoothAllAt(DWORD dwPos);

	__forceinline void SmoothAt(DWORD dwPos, int iSmoothSet, int iLatSet, int iTargetSet, BOOL bIgnoreShore = TRUE)
	{
		int i;
		int its, iss, ils;
		int iLatGround = tilesets_start[iLatSet];
		int iSmoothGround = tilesets_start[iSmoothSet];
		its = (*tiledata)[tilesets_start[iTargetSet]].tiles[0].bTerrainType;
		iss = (*tiledata)[iSmoothGround].tiles[0].bTerrainType;
		ils = (*tiledata)[iLatGround].tiles[0].bTerrainType;



		FIELDDATA m = *GetFielddataAt(dwPos);
		if (m.wGround == 0xFFFF) m.wGround = 0;

		// do we have that certain LAT tile here?
		if ((*tiledata)[m.wGround].wTileSet != iSmoothSet && (*tiledata)[m.wGround].wTileSet != iLatSet) return;

		//if(m.wGround==iLatGround) m.wGround=iSmoothGround;

		if (its == iss && (*tiledata)[m.wGround].wTileSet == iSmoothSet) m.wGround = iLatGround;

		if (its == iss) ils += 1;

		BOOL bOnlyLat = TRUE;
		//BOOL bNoLat=TRUE;
		int ns = -1;
		int e, p = 0;
		int ts[3][3];  // terrain info

		int set = (*tiledata)[m.wGround].wTileSet;

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

					int cur_set = (*tiledata)[m2.wGround].wTileSet;

					//if(cur_set==iSmoothSet) bNoLat=FALSE;
					//

					if (its == iss && cur_set == iSmoothSet)
					{
						m2.wGround = iLatGround; cur_set = iLatSet;
					}

					if (cur_set == iSmoothSet || cur_set == iTargetSet) bOnlyLat = FALSE;

					/*if(cur_set != iSmoothSet && cur_set!= iLatSet && cur_set!=iTargetSet)
					{
						ts[i][e]=(*tiledata)[m2.wGround].tiles[m2.bSubTile].bTerrainType;
						if((*tiledata)[m2.wGround].wTileSet!=shoreset)
						{
							ts[i][e]=0;//ts[i][e]+1; // make sure you don´t smooth at it except it´s shore
						}
						//if(bIgnoreShore && (*tiledata)[m2.wGround].wTileSet==shoreset)
						//	ts[i][e]=0;//ts[i][e]+1;
					}
					else*/ if (its == iss && cur_set != set)
					{
						if (cur_set == shoreset && !bIgnoreShore)
							ts[i][e] = its;
						else if (cur_set != iSmoothSet && cur_set != iTargetSet && cur_set != iLatSet)
							ts[i][e] = 0;
						else
							ts[i][e] = its;
					}
					else if (its == iss && cur_set == set)
	ts[i][e] = ils;
					else
					{
						ts[i][e] = (*tiledata)[m2.wGround].tiles[m2.bSubTile].bTerrainType;

						if (cur_set != shoreset && cur_set != iLatSet && cur_set != iSmoothSet)
						{
							ts[i][e] = 0;//ts[i][e]+1; // make sure you don´t smooth at it except it´s shore
						}

					}
				}
			}
		}

		//if(bOnlyLat) return;


		int needed = -1;

		// 1/1 is smoothed tile

		if (ts[1][1] == ils)
		{
			// single lat
			if (ts[0][1] != ils && ts[1][0] != ils
				&& ts[1][2] != ils && ts[2][1] != ils)
				needed = 16;
			else if (ts[0][1] == ils && ts[1][0] == ils
				&& ts[1][2] == ils && ts[2][1] == ils)
				needed = 0;
			else if (ts[0][1] == ils && ts[2][1] == ils &&
				ts[1][0] != ils && ts[1][2] != ils)
				needed = 11;
			else if (ts[1][0] == ils && ts[1][2] == ils &&
				ts[0][1] != ils && ts[2][1] != ils)
				needed = 6;
			else if (ts[1][0] != ils && ts[0][1] == ils &&
				ts[2][1] == ils)
				needed = 9;
			else if (ts[2][1] != ils && ts[1][0] == ils &&
				ts[1][2] == ils)
				needed = 5;
			else if (ts[1][2] != ils && ts[0][1] == ils &&
				ts[2][1] == ils)
				needed = 3;
			else if (ts[0][1] != ils && ts[1][0] == ils &&
				ts[1][2] == ils)
				needed = 2;
			else if (ts[0][1] == ils && ts[1][0] != ils &&
				ts[1][2] != ils && ts[2][1] != ils)
				needed = 15;
			else if (ts[1][2] == ils && ts[1][0] != ils &&
				ts[0][1] != ils && ts[2][1] != ils)
				needed = 14;
			else if (ts[2][1] == ils && ts[1][0] != ils &&
				ts[0][1] != ils && ts[1][2] != ils)
				needed = 12;
			else if (ts[1][0] == ils && ts[0][1] != ils &&
				ts[1][2] != ils && ts[2][1] != ils)
				needed = 8;
			else if (ts[1][0] != ils && ts[2][1] != ils)
				needed = 13;
			else if (ts[1][0] != ils && ts[0][1] != ils)
				needed = 10;
			else if (ts[2][1] != ils && ts[1][2] != ils)
				needed = 7;
			else if (ts[0][1] != ils && ts[1][2] != ils)
				needed = 4;


		}
		else if (ts[1][1] == its)
		{
			// replace target set instead of smooth set
			//if(st(ts[0][0], && ts[0][1]
		}


		needed -= 1;
		if (needed >= 0)
		{
			/*for(i=0;i<*tiledata_count;i++)
			{
				if((*tiledata)[i].wTileSet==iLatSet)
				{
					break;
				}
			}*/
			i = tilesets_start[iLatSet];

			// i is first lat tile
			int e;
			for (e = 0;e < needed;e++)
			{
				i += (*tiledata)[i].wTileCount;
			}

			SetTileAt(dwPos, i, 0);
		}
		else if (needed == -1)
		{
			/*for(i=0;i<*tiledata_count;i++)
			{
				if((*tiledata)[i].wTileSet==iSmoothSet)
				{
					break;
				}
			}*/
			i = tilesets_start[iSmoothSet];

			// i is first lat tile		
			SetTileAt(dwPos, i, 0);
		}
	}

	BOOL GetLocalSize(RECT* rect) const;
	void Undo();
	void TakeSnapshot(BOOL bEraseFollowing = TRUE, int left = 0, int top = 0, int right = 0, int bottom = 0);
	BOOL CheckMapPackData();
	int GetInfantryCountAt(DWORD dwPos);
	void DeleteTube(std::uint16_t wID);
	// void SetTube(WORD wID, CTube *lpTI);
	void SetTube(CTube* lpTI);
	CTube* GetTube(std::uint16_t wID);
};

inline bool CMapData::isInside(MapCoords xy) const
{
	return xy.x >= 0 && xy.y >= 0 && xy.x < m_IsoSize && xy.y < m_IsoSize;
}

inline MapCoords CMapData::ToMapCoords(ProjectedCoords xy) const
{
	return ToMapCoords3d(xy, 0);
}

inline MapCoords CMapData::ToMapCoords3d(ProjectedCoords xy, int mapZ) const
{
	float cx = xy.x, cy = xy.y + mapZ * f_y / 2;
	return MapCoords(
		cy / (float)f_y - cx / (float)f_x + (float)(m_IsoSize - 2) / 2 + (float)0.5,
		cy / (float)f_y + cx / (float)f_x - (float)(m_IsoSize - 2) / 2.0f - (float)0.5
	);
}

inline MapCoords CMapData::ToMapCoords3d(const ProjectedCoords xy, bool bAllowAccessBehindCliffs, bool ignoreHideFlagsAndOutside) const
{
	auto xy2d = ToMapCoords(xy);
	static const auto fxy = ProjectedVec(f_x, f_y).convertT<float>();
	auto ret = ignoreHideFlagsAndOutside ? xy2d : MapCoords(-1, -1);

	for (int i = 15; i >= 0; i--)
	{
		for (int e = 0; e < 3; e++)
		{
			const MapVec off(i, i);  // this is a vertical line starting from the bottom
			const MapVec off2(e == 1 ? -1 : 0, e == 2 ? -1 : 0);  // increase x or y or neither in map coordinates by one so that we have a broader area to check
			const MapCoords cur = xy2d + off + off2;

			if (isInside(cur))
			{
				const auto& mfd = *GetFielddataAt(GetMapPos(cur));
				const int ground = mfd.wGround == 0xFFFF ? 0 : mfd.wGround;
				if (ignoreHideFlagsAndOutside || (!mfd.bHide && !(tiledata && (*tiledata)[ground].bHide)))
				{
					const auto curProj = ProjectCoords3d(cur);

					// now projCoords hold the logical pixel coordinates for the current field... 
					// we now need to check if cx and cy are in this field
					//if(*x >= m && *x<= m+f_x && *y>=n && *y<=n+f_y)
					{
						auto df1 = (xy - curProj).convertT<float>();
						auto dfScaled = df1 / fxy;
						auto d = Vec2<CSProjected, float>(dfScaled.y - dfScaled.x + 0.5f, dfScaled.y + dfScaled.x - 0.5f);
						if (d.x >= 0.0f && d.y >= 0.0f && d.x <= 1.0f && d.y <= 1.0f)// || (!bAllowAccessBehindCliffs && xy.y >= curProj.y)))
							//if (d == MapCoords(0, 0))
						{
							if (bAllowAccessBehindCliffs)
								ret = cur;
							else
								return cur;
						}
					}
				}
			}
		}
	}
	return ret;
}

inline ProjectedCoords CMapData::ProjectCoords(MapCoords xy) const
{
	return ProjectCoords3d(xy, 0);
}

inline ProjectedCoords CMapData::ProjectCoords3d(MapCoords xy, int z) const
{
	return ProjectedCoords(
		(m_IsoSize - 2 - xy.x + xy.y) * f_x / 2,
		(xy.y + xy.x - z) * f_y / 2
	);
}

inline ProjectedCoords CMapData::ProjectCoords3d(MapCoords xy) const
{
	return ProjectCoords3d(xy, isInside(xy) ? GetHeightAt(xy) : 0);
}

#endif // !defined(AFX_MAP_H__9278CAC0_D4E7_11D3_B63B_444553540001__INCLUDED_)
