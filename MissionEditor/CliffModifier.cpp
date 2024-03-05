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

// CliffModifier.cpp: Implementierung der Klasse CCliffModifier.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stdafx.h"
#include "CliffModifier.h"
#include "variables.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CCliffModifier::CCliffModifier()
{

}

CCliffModifier::~CCliffModifier()
{

}

BOOL CCliffModifier::PlaceCliff(DWORD dwXFrom, DWORD dwYFrom, DWORD dwXDest, DWORD dwYDest, BOOL bAlternative)
{
	m_bAlternative=bAlternative;

	int x_diff, y_diff;

	x_diff=dwXDest-dwXFrom;
	y_diff=dwYDest-dwYFrom;

	if(x_diff>0 && y_diff>0 && ((float)y_diff)/((float)x_diff) > 1.33f)
	{
		x_diff=0;
		dwXDest=dwXFrom;		
	}
	if(x_diff<0 && y_diff>0 && ((float)y_diff)/(-(float)x_diff) > 1.33f)
	{
		x_diff=0;
		dwXDest=dwXFrom;		
	}
	if(x_diff>0 && y_diff<0 && ((float)-y_diff)/((float)x_diff) > 1.33f)
	{
		x_diff=0;
		dwXDest=dwXFrom;		
	}
	if(x_diff<0 && y_diff<0 && (-(float)y_diff)/(-(float)x_diff) > 1.33f)
	{
		x_diff=0;
		dwXDest=dwXFrom;		
	}
	if(y_diff>0 && x_diff>0 && ((float)x_diff)/((float)y_diff) > 2)
	{
		y_diff=0;
		dwYDest=dwYFrom;		
	}
	if(y_diff<0 && x_diff>0 && ((float)x_diff)/(-(float)y_diff) > 2)
	{
		y_diff=0;
		dwYDest=dwYFrom;		
	}
	if(y_diff>0 && x_diff<0 && ((float)-x_diff)/((float)y_diff) > 2)
	{
		y_diff=0;
		dwYDest=dwYFrom;		
	}
	if(y_diff<0 && x_diff<0 && (-(float)x_diff)/(-(float)y_diff) > 2)
	{
		y_diff=0;
		dwYDest=dwYFrom;		
	}

	m_dwFrom=dwXFrom + dwYFrom*Map->GetIsoSize();
	m_dwTo=dwXDest + dwYDest*Map->GetIsoSize();
	
	if(!x_diff && !y_diff) return FALSE;
	
	if(x_diff && y_diff)
	{
		if((x_diff>0 && y_diff>0))
		{
			m_direction=cd_verticdiag_bottom;
		}
		else if((x_diff<0 && y_diff<0))
		{
			m_direction=cd_verticdiag_top;
		}
		else if((x_diff<0 && y_diff>0) )
		{
			m_direction=cd_horizdiag_right;
		}
		else if(x_diff>0 && y_diff<0)
		{
			m_direction=cd_horizdiag_left;
		}

	}
	else if(y_diff<0)
	{
		m_direction=cd_horiz_left;
	}
	else if(y_diff>0)
	{
		m_direction=cd_horiz_right;
	}
	else if(x_diff<0)
	{
		m_direction=cd_vertic_top;
	}
	else if(x_diff>0)
	{
		m_direction=cd_vertic_bottom;
	}

	switch(m_direction)
	{
	case cd_horiz_left:
		m_addx=0;
		m_addy=-1;
		break;
	case cd_horiz_right:
		m_addx=0;
		m_addy=1;
		break;
	case cd_horizdiag_left:
		m_addx=1;
		m_addy=-1;
		break;
	case cd_horizdiag_right:
		m_addx=-1;
		m_addy=1;
		break;
	case cd_vertic_top:
		m_addx=-1;
		m_addy=0;
		break;
	case cd_vertic_bottom:
		m_addx=1;
		m_addy=0;
		break;
	case cd_verticdiag_top:
		m_addx=-1;
		m_addy=-1;
		break;
	case cd_verticdiag_bottom:
		m_addx=1;
		m_addy=1;
		break;
	}

	int i,e;
	DWORD dwCurPos=m_dwFrom;
	int startheight=Map->GetHeightAt(dwCurPos);

	

	DWORD dwLastTile=-1;

	FIELDDATA* fd=Map->GetFielddataAt(dwCurPos);
	int ground=fd->wGround;
	int subtile=fd->bSubTile;
	if(ground==0xFFFF) ground=0;
	startheight-=(*tiledata)[ground].tiles[subtile].bZHeight;
	
	ModifyStartPos(&dwCurPos, FALSE);

	BOOL bSmall=FALSE;
	DWORD dwFirstStartPos=dwCurPos;	
	DWORD dwTile=GetTileToPlace(dwCurPos, &bSmall);

	
	
	BOOL bFirstPos=TRUE;

	while(dwCurPos!=m_dwTo)
	{
		ModifyCurrentPos(&dwCurPos, TRUE, bSmall);

		if(!bFirstPos) 
		{
			dwTile=GetTileToPlace(dwCurPos, &bSmall);
			dwLastTile=dwTile;
		}
		else
			bFirstPos=FALSE;
		if(dwTile==-1) break;
		
		TILEDATA* t=&(*tiledata)[dwTile];
		
		if(m_addx<0) dwCurPos+=t->cx*m_addx;
		if(m_addy<0) dwCurPos+=t->cy*m_addy*Map->GetIsoSize();
	

		int o=0;
		DWORD dwTmpTile=dwTile;
		while(dwFirstStartPos!=dwCurPos && dwTile==dwLastTile)
		{
			o++;
			if(o==10) break;
			dwTmpTile=GetTileToPlace(dwCurPos, &bSmall);

			if((*tiledata)[dwTmpTile].cx==(*tiledata)[dwTile].cx && (*tiledata)[dwTile].cy==(*tiledata)[dwTmpTile].cy)
				dwTile=dwTmpTile;
		}
		
		

		t=&(*tiledata)[dwTile];
		
		
		
		
		
		

		int p=0;
		for(i=0;i<t->cx;i++)
		{
			for(e=0;e<t->cy;e++)
			{
				
				if(t->tiles[p].pic!=NULL)
				{
					
					
					Map->SetHeightAt(dwCurPos+i+e*Map->GetIsoSize(), startheight+t->tiles[p].bZHeight);
					Map->SetTileAt(dwCurPos+i+e*Map->GetIsoSize(), dwTile, p);
					
				}
				p++;
			}
		}		

		if(m_addx>0) dwCurPos+=t->cx*m_addx;
		if(m_addy>0) dwCurPos+=t->cy*m_addy*Map->GetIsoSize();
		
		ModifyCurrentPos(&dwCurPos, FALSE, bSmall);

		dwLastTile=dwTile;
	}

	return TRUE;	
}

void CCliffModifier::ModifyStartPos(DWORD *dwStartPos, BOOL bSmall)
{

}

void CCliffModifier::ModifyCurrentPos(DWORD *dwPos, BOOL bBeforePlacing, BOOL bSmall)
{
	
}

DWORD CCliffModifier::GetTileToPlace(DWORD dwPos, BOOL* bSmall)
{
	vector<DWORD> careables;
	vector<DWORD> useables;
	map<DWORD, BOOL> notusedascliff;

	CString type;
	int count=0;

		
	switch(m_direction)
	{
	case cd_horiz_left:
	case cd_horiz_right:
		type="horiz_";
		break;
	case cd_horizdiag_left:
	case cd_horizdiag_right:
		type="horiz_diag_";
		break;
	case cd_vertic_top:
	case cd_vertic_bottom:
		type="vertic_";
		break;
	case cd_verticdiag_top:
	case cd_verticdiag_bottom:
		type="vertic_diag_";
		break;
	}

		
	CString sec=GetDataSection();
	CIniFile& ini=Map->GetIniFile();
	if(g_data.sections.find(sec+ini.sections["Map"].values["Theater"])!=g_data.sections.end())
		sec=sec+ini.sections["Map"].values["Theater"];

	count=atoi(g_data.sections[sec].values[type+"c"]);

	
	
	int i;
	DWORD dwStartSet=0;
	/*for(i=0;i<(*tiledata_count);i++)
	{
		if( (!m_bAlternative && (*tiledata)[i].wTileSet==cliffset) || (m_bAlternative && (*tiledata)[i].wTileSet==cliff2set))
		{
			dwStartSet=i;
			break;
		}
	}*/
	// a bit faster:
	if(m_bAlternative) dwStartSet=cliff2set_start;
	else dwStartSet=cliffset_start;
	
	for(i=0;i<count;i++)
	{
		char c[50];
		itoa(i, c, 10);
		CString cur=type;
		cur+=c;
		notusedascliff[dwStartSet+atoi(g_data.sections[sec].values[cur])]=TRUE;
	}

	CString corner_searched="";
	DWORD dwCurPos=dwPos;
	int isosize=Map->GetIsoSize();
	ModifyStartPos(&dwCurPos, FALSE);
	int e;
	for(i=-1;i<2;i++)
	{
		for(e=-1;e<2;e++)
		{
			FIELDDATA* fd=Map->GetFielddataAt(dwCurPos+i+e*isosize);
			int ground=fd->wGround;
			if(ground==0xFFFF) ground=0;

			if( (*tiledata)[ground].wTileSet==cliffset && notusedascliff.find(ground)==notusedascliff.end())
			{
				
				
				if(i==0 && e==-1) {corner_searched="cornerleft_";break;}
				if(i==0 && e==1) {corner_searched="cornerright_";break;}
				if(i==-1 && e==0) {corner_searched="cornertop_";break;}
				if(i==1 && e==0) {corner_searched="cornerbottom_";break;}
				/*if(e==-1) {corner_searched="cornerleft_";break;}
				if(e==1) {corner_searched="cornerright_";break;}
				if(i==-1) {corner_searched="cornertop_";break;}
				if(i==1) {corner_searched="cornerbottom_";break;}*/
			}
			
		}
		if(corner_searched.GetLength()>0) break;
	}

	BOOL bCornerFound=FALSE;
	if(g_data.sections[sec].FindName(type+corner_searched+"c")>=0)
	{
		int icount=atoi(g_data.sections[sec].values[type+corner_searched+"c"]);
		if(icount)
		{
			bCornerFound=TRUE;
			count=icount;
		}
	}
	
	if(!bCornerFound) corner_searched="";

	if(count==0) return -1;


	DWORD dwX=dwPos%Map->GetIsoSize();
	DWORD dwY=dwPos/Map->GetIsoSize();
	DWORD dwDX=m_dwTo%Map->GetIsoSize();
	DWORD dwDY=m_dwTo/Map->GetIsoSize();

	
	for(i=0;i<count;i++)
	{
		char c[50];
		itoa(i,c,10);
		careables.push_back(dwStartSet+atoi(g_data.sections[sec].values[type+corner_searched+c]));				
	}

	for(i=0;i<careables.size();i++)
	{
		TILEDATA& t=(*tiledata)[careables[i]];	

		if(m_addx>0 && dwX+m_addx*t.cx>dwDX) continue;
		if(m_addy>0 && dwY+m_addy*t.cy>dwDY) continue;
		if(m_addx<0 && dwX+m_addx*t.cx<dwDX) continue;
		if(m_addy<0 && dwY+m_addy*t.cy<dwDY) continue;

		useables.push_back(careables[i]);
	}

	if(useables.size()<1) return -1;

	
	*bSmall=FALSE;

	int k;
	k=rand()%(useables.size());

	TILEDATA& t1=(*tiledata)[useables[k]];
	for(i=0;i<careables.size();i++)
	{
		TILEDATA& t=(*tiledata)[careables[i]];
		if(t.cx>t1.cx || t.cy>t1.cy) {*bSmall=TRUE; break;}
	}
		
	// check for water
	BOOL bWater=FALSE;
	for(i=0;i<t1.cx;i++)
	{
		for(e=0;e<t1.cy;e++)
		{

			FIELDDATA* fd=Map->GetFielddataAt(dwPos+i+e*Map->GetIsoSize());
			int ground=fd->wGround;
			if(ground==0xFFFF) ground=0;
			if((i==t1.cx-1 || e==t1.cy-1) && (*tiledata)[ground].tiles[fd->bSubTile].bHackedTerrainType==TERRAINTYPE_WATER)
			{
				bWater=TRUE;
				break;
			}
		}
		if(bWater) break;
	}
	

	if(bWater && useables[k]-dwStartSet<22)
	{
		CString tset;
		char c[50];
		int watercliffs=atoi(tiles->sections["General"].values["WaterCliffs"]);
		if(m_bAlternative) watercliffs=cliffwater2set;
		itoa(watercliffs, c, 10);
		int e;
		for(e=0;e<4-strlen(c);e++)
			tset+="0";
		tset+=c;
		CString sec="TileSet";
		sec+=tset;

		if(tiles->sections.find(sec)==tiles->sections.end()) return useables[k];


		if(atoi(tiles->sections[sec].values["TilesInSet"])>useables[k]-dwStartSet)
		{
			DWORD dwStartWaterSet=0;
			for(i=0;i<(*tiledata_count);i++)
			{
				if((*tiledata)[i].wTileSet==watercliffs)
				{
					dwStartWaterSet=i;
					return dwStartWaterSet+(useables[k]-dwStartSet);
				}
			}
			
			
		}
	}

	return useables[k];
}


