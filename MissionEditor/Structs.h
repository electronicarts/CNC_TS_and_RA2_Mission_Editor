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

/**************************************
	Global structs here
**************************************/

#ifndef GLOBALSTRUCTS_H_INCLUDED
#define GLOBALSTRUCTS_H_INCLUDED

#include "defines.h"
#include <vector>
#include <memory>
#include <cstdint>
#include "MissionEditorPackLib.h"
#include "Vec2.h"

class MapTool;


enum class TheaterChar : char
{
	T = 'T',
	A = 'A',
	U = 'U',
	N = 'N',
	D = 'D',
	L = 'L',
	Generic = 'G',
	None = 0
};

inline TheaterChar toTheaterChar(char c)
{
	switch (c)
	{
	case 'T':
		return TheaterChar::T;
	case 'A':
		return TheaterChar::A;
	case 'U':
		return TheaterChar::U;
	case 'N':
		return TheaterChar::N;
	case 'D':
		return TheaterChar::D;
	case 'L':
		return TheaterChar::L;
	case 'G':
		return TheaterChar::Generic;
	default:
		return TheaterChar::None;
	}	
}

struct CLIPBOARD_MAPCOPY_ENTRY
{
	BOOL bRedrawTerrain;
	BYTE overlay;
	BYTE overlaydata;
	WORD wGround;
	WORD bMapData;	
	BYTE bSubTile;
	BYTE bHeight;
	BYTE bMapData2;

	// for RA2<->TS conversion:
	WORD wTileSet;
	BYTE bTile;
};

struct CLIPBOARD_DATA
{
	DWORD dwType;
	DWORD dwVersion;
	DWORD dwReserved;
	BYTE bGame; // 0 TS, 1 RA2
	UINT iWidth;
	UINT iHeight;
};

struct VBORDER
{
	short left;
	short right;
};

/*
picdata

Holds information for every picture:
a directdraw surface, and additional data
*/
struct PICDATA{
	PICDATA() = default;
	PICDATA(const PICDATA& c) = default;
	PICDATA& operator=(const PICDATA& c) = default;
	~PICDATA() = default;

#ifndef NOSURFACES_OBJECTS
	LPDIRECTDRAWSURFACE4 pic;
#else
	void* pic = nullptr; // BYTE* to image, exception: if bType==PICDATA_TYPE_BMP then this is a LPDIRECTDRAWSURFACE!
	VBORDER* vborder = nullptr;
	int* pal = nullptr;
	std::shared_ptr<std::vector<BYTE>> lighting;
	std::shared_ptr<std::vector<BYTE>> rawPic;
	std::shared_ptr<std::vector<VBORDER>> _vBorder;
#endif
	
	FSunPackLib::VoxelNormalClass normalClass = FSunPackLib::VoxelNormalClass::Unknown;
	short x = 0; // for SHPs (starting point of main graphic inside surface)
	short y = 0; // for SHPs (starting point of main graphic inside surface)
	WORD wWidth = 0; // for non-shps: size of whole surface, else size of main graphic
	WORD wHeight = 0; // for non-shps: size of whole surface, else size of main graphic
	WORD wMaxWidth = 0; // for SHPs (size of whole surface)
	WORD wMaxHeight = 0; // for SHPs (size of whole surface)
	BYTE bType = 0; // is loaded from voxel, shp, bmp, etc... (for drawing logic)
	TheaterChar bTerrain = TheaterChar::None;
	BOOL bTried = 0; // already tried to load this? - superseded by global array missingimages

	inline ProjectedVec drawOffset() const
	{
		return ProjectedVec(x, y);
	}

#ifdef NOSURFACES_OBJECTS
	void createVBorder();
#endif

};

/*
TILEPIC / TILEDATA

Holds information for every tile/subtile in TS.
*/
struct SUBTILE{
#ifndef NOSURFACES
	LPDIRECTDRAWSURFACE4 pic;
#else
#ifdef NOSURFACES_EXTRACT
	BOOL bNotExtracted; // pic is not extracted, even if NOSURFACES_EXTRACT is specifed. Probably there is inner transparency!
#endif
	BYTE* pic; // optionally use direct blitting? This is either a palette index array or non-palette data (if NOSURFACES_EXTRACT is specified)
	VBORDER * vborder;
#endif
	
	WORD wWidth; // width of tile
	WORD wHeight; // height "
	short sX; // x starting pos
	short sY; // y starting pos
	BYTE bZHeight; // z difference from tile ground surface (1unit == f_y/2pixel == 12pixel)
	BYTE bDirection; // for ramps for example
	BYTE bTerrainType; // water, etc...
	RGBTRIPLE rgbLeft;
	RGBTRIPLE rgbRight;
	BYTE bHackedTerrainType; // same like bTerrainType, except some shore pieces that were hacked in fsdata.ini

	std::shared_ptr<PICDATA> anim;

	inline ProjectedVec drawOffset() const
	{
		return ProjectedVec(sX, sY);
	}
};
struct TILEDATA{
	TILEDATA() = default;
	~TILEDATA();
	WORD wTileSet = 0;
	SUBTILE* tiles = nullptr; // the different tiles
	WORD wTileCount = 0; // number of tiles	
	WORD cx = 0;
	WORD cy = 0;
	BOOL bAllowTiberium = 0;
	BOOL bAllowToPlace = TRUE;
	BOOL bMorphable = FALSE; // morphable using lower/raise ground function?
	BOOL bHide = FALSE; // for user interface only
	BOOL bMarbleMadness = FALSE;
	WORD wMarbleGround = 0xFFFF;

public: 
	RECT rect = { 0 };

	TILEDATA* lpReplacements = nullptr;
	BYTE bReplacementCount = 0;
};

/*
actiondata

CViewObjects sets this data struct,
and CIsoView uses it to interact with the user´s CViewObjects and terrain editing choice
*/
struct ACTIONDATA{
	DWORD mode;
	DWORD type;
	DWORD data;
	DWORD data2;
	DWORD data3;
	int z_data;
	CString data_s;
	std::unique_ptr<MapTool> tool;

	~ACTIONDATA();

	void reset();
};

struct SIDEINFO{
	CString name;
	int orig_n;
};	

struct HOUSEINFO{
	CString name;
	SIDEINFO* side; // side is used for rules.ini owner= settings
	RGBTRIPLE color;
	BOOL bPlayable;
};

/**********************
  Unittype infos
**********************/

/* 
building infos 
*/
struct BUILDING_INFO
{
	PICDATA pic[8];
	int pic_count;
	BYTE w;
	BYTE h;
	BOOL bTemp;
	BOOL bSnow;
	BOOL bUrban;
};

struct TREE_INFO
{
	PICDATA pic;
	BYTE w;
	BYTE h;
};

#ifdef SMUDGE_SUPP
struct SMUDGE_INFO
{
	PICDATA pic;
	//BYTE w;
	//BYTE h;
};
#endif

/*
** Condition enumeration
*/
enum ConditionEnum {
	COND_LT = 0,	// less than
	COND_LE,			// less than or equal to
	COND_EQ,			// equal to
	COND_GE,			// greater than or equal to
	COND_GT,			// greater than
	COND_NE,			// not equal to
	COND_COUNT
};

struct AITrigInfo
{
	union{
	struct{
		int Number;
		ConditionEnum Condition;
	};
	char Padding[32];
	};
};


/*
Struct for string items
*/
struct XCString
{
	XCString()
	{
		//wString=new(WCHAR[2]);
		//memset(wString, 0, 4);
		wString=NULL;
		bUsedDefault=FALSE;
		len=0;
	}
	~XCString()
	{
		if(wString) delete[] wString;
		len=0;
		wString=NULL;
	}
	void SetString(const CHAR* cString)
	{
		len=strlen(cString);

		this->cString=cString;
		
		if(wString) delete[] wString;
		
		bUsedDefault=FALSE;
		wString=new(WCHAR[len+1]);
		memset(wString, 0, (len+1)*2);
		
		//MultiByteToWideChar(CP_ACP, WC_COMPOSITECHECK, cString, len, wString, len+1);
		mbstowcs(wString, cString, len);		

	}
	void SetString(const WCHAR* wString, int len)
	{
		this->len=len;

		if(this->wString) delete[] this->wString;
		
		bUsedDefault=FALSE;

		this->wString=new(WCHAR[len+1]);	
		memset(this->wString, 0, (len+1)*2);
		memcpy(this->wString, wString, len*2);

		auto bufferSize = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, this->wString, len + 1, nullptr, 0, NULL, &bUsedDefault);
		if (bufferSize == 0)
		{
			cString = "";
			return; // failed
		}

		std::vector<BYTE> bByte(bufferSize + 4, 0);
		if (WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, this->wString, len + 1, (LPSTR)bByte.data(), bufferSize, NULL, &bUsedDefault) == 0)
		{
			cString = "";
			return; // failed
		}
		cString = bByte.data();

	}
	CString cString;
	WCHAR* wString;
	BOOL bUsedDefault;
	int len;
};


/**********************
	Map stuff structs
***********************/

/*
AITRIGGERTYPE{};

Data for ai trigger types
*/
struct AITRIGGERTYPE{
	CString ID; // ai trigger id
	CString name; // param 1
	CString teamtype1; // param 2
	CString owner; // param 3
	CString techlevel; // flag 1/ param 4
	CString type; // param 5
	CString unittype; // param 6
	CString data; // param 7
	CString float1; // param 8 
	CString float2; // param 9
	CString float3; // param 10
	CString skirmish; // param 11
	CString flag4; // param 12 
	CString multihouse; // param 13
	CString basedefense; // param 14
	CString teamtype2; // param 15
	CString easy; // param 16
	CString medium; // param 17
	CString hard; // param 18
};

/*
STDOBJECTDATA

Data for Trees, Units, Infantry, Aircraft & Structures
This should be used for painting procedure... 
except if you want to draw the upgrades on buildings
*/
struct STDOBJECTDATA
{
	CString house; // 1
	CString type; // 2
	CString strength; // 3
	CString y; // 4
	CString x; // 5
};

/*
STRUCTURE

Data for a structure
*/
struct STRUCTURE{
	CString house; // 1
	CString type; // 2
	CString strength; // 3
	CString y; // 4
	CString x; // 5
	CString direction; // 6
	CString tag; // 7
	CString flag1; // 8
	CString flag2; // 9
	CString energy; // 10
	CString upgradecount; // 11
	CString spotlight; // 12
	CString upgrade1; // 13
	CString upgrade2; // 14
	CString upgrade3; // 15
	CString flag3; // 16
	CString flag4; // 17
	unsigned deleted:1;
};

// structure data especially for painting
struct STRUCTUREPAINT{
	COLORREF col;
	CString type;
	short x;
	short y;
	short direction;
	BYTE strength;
	BYTE upradecount;
	CString upgrade1;
	CString upgrade2;
	CString upgrade3;
};


/*
struct INFANTRY{};

Data for an infantry unit
*/
struct INFANTRY{
	CString house; // 1
	CString type; // 2
	CString strength; // 3
	CString y; // 4
	CString x; // 5
	CString pos; // 6
	CString action; // 7
	CString direction; // 8
	CString tag; // 9
	CString flag1; // 10
	CString flag2; // 11
	CString flag3; // 12
	CString flag4; // 13
	CString flag5; // 14
	unsigned deleted:1;
};

struct UNIT{
	CString house; // 1
	CString type; // 2
	CString strength; // 3
	CString y; // 4
	CString x; // 5
	CString direction; // 6
	CString action; // 7
	CString tag; // 8
	CString flag1; // 9
	CString flag2; // 10
	CString flag3; // 11
	CString flag4; // 12
	CString flag5; // 13
	CString flag6; // 14
	unsigned deleted:1;
};



struct AIRCRAFT{
	CString house; // 1
	CString type; // 2
	CString strength; // 3
	CString y; // 4
	CString x; // 5
	CString direction; // 6
	CString action; // 7
	CString tag; // 8
	CString flag1; // 9
	CString flag2; // 10
	CString flag3; // 11
	CString flag4; // 12
	unsigned deleted:1;
};

struct TERRAIN{
	CString type;
	int x;
	int y;
	unsigned deleted:1;
};

// MW 08/07/01: Smudge
#ifdef SMUDGE_SUPP
struct SMUDGE{
	CString type;
	int x;
	int y;
	unsigned deleted:1;
};
#endif

/*
NODE

Data for a node
*/
struct NODE{
	CString house;
	CString type;
	CString x;
	CString y;
};


struct RA2STRFILEHEAD
{
	DWORD dwFlag1;
	DWORD dwCount1;
	DWORD dwCount2;
	DWORD dwUnused;
	DWORD dwFlag2;
};

#define RA2STRFILEHEADSIZE 20

struct RA2STRINGENTRY
{
	RA2STRINGENTRY(){
		//memset(this, 0, sizeof(RA2STRINGENTRY));
		id=NULL;
		value=NULL;
		value_asc=0;
		value_asc_size=0;
		dwFlag = 0;
		id_size = 0;
		value_size = 0;
	}
	~RA2STRINGENTRY(){
		if(value) delete[] value;
		if(id) delete[] id;
		if(value_asc) delete[] value_asc;
		value=NULL;
		value_size=0;
		id=NULL;
		id_size=0;
		value_asc=0;
		value_asc_size=0;
	}
	RA2STRINGENTRY(RA2STRINGENTRY& ref)
	{
		value=NULL;
		value_size=0;
		id=NULL;
		id_size = 0;
		value_asc=0;
		value_asc_size=0;
		
		if(ref.value_size && ref.value)
		{
			value_size=ref.value_size;
			value=new(WCHAR[value_size+1]);
			memcpy(value, ref.value, value_size*sizeof(WCHAR));
			value[value_size]=0;
		}
		if(ref.id_size && ref.id)
		{
			id_size=ref.id_size;
			id=new(CHAR[id_size+1]);
			memcpy(id, ref.id, id_size*sizeof(CHAR));
			id[id_size]=0;
		}
		if(ref.value_asc_size && ref.value_asc)
		{
			value_asc_size=ref.value_asc_size;
			value_asc=new(CHAR[value_asc_size+1]);
			memcpy(value_asc, ref.value_asc, value_asc_size*sizeof(CHAR));
			value_asc[value_asc_size]=0;
		}
		dwFlag=ref.dwFlag;
	}
	RA2STRINGENTRY& operator=(RA2STRINGENTRY& ref)
	{
		if(value) delete[] value;
		if(id) delete[] id;
		value=NULL;
		value_size=0;
		id=NULL;
		value_asc=0;
		value_asc_size=0;
		
		if(ref.value_size)
		{
			value_size=ref.value_size;
			value=new(WCHAR[value_size+1]);
			memcpy(value, ref.value, value_size*sizeof(WCHAR));
			value[value_size]=0;
		}
		if(ref.id_size)
		{
			id_size=ref.id_size;
			id=new(CHAR[id_size+1]);
			memcpy(id, ref.id, id_size*sizeof(CHAR));
			id[id_size]=0;
		}
		if(ref.value_asc_size)
		{
			value_asc_size=ref.value_asc_size;
			value_asc=new(CHAR[value_asc_size+1]);
			memcpy(value_asc, ref.value_asc, value_asc_size*sizeof(CHAR));
			value_asc[value_asc_size]=0;
		}
		dwFlag=ref.dwFlag;

		return *this;
	}
		
	DWORD dwFlag;
	CHAR* id;
	DWORD id_size;
	DWORD value_size;
	WCHAR* value;
	CHAR* value_asc;
	DWORD value_asc_size;
};



#endif