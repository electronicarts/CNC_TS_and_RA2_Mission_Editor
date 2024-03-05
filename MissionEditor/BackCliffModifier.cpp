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

// BackCliffModifier.cpp: Implementierung der Klasse CBackCliffModifier.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stdafx.h"
#include "BackCliffModifier.h"
#include <vector>
#include "variables.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CBackCliffModifier::CBackCliffModifier()
{

}

CBackCliffModifier::~CBackCliffModifier()
{
	
}



void CBackCliffModifier::ModifyCurrentPos(DWORD *dwPos, BOOL bBeforePlacing, BOOL bSmall)
{
	if(!bBeforePlacing)
	{
		if(m_direction==cd_verticdiag_top) (*dwPos)+=2+0*Map->GetIsoSize();	
		if(m_direction==cd_verticdiag_bottom) (*dwPos)+=-2+0*Map->GetIsoSize();	
	}
}

void CBackCliffModifier::ModifyStartPos(DWORD *dwPos, BOOL bSmall)
{
	if(m_direction==cd_verticdiag_top) (*dwPos)+=1+1*Map->GetIsoSize();	
	if(m_direction==cd_verticdiag_bottom) (*dwPos)+=-1+0*Map->GetIsoSize();	

	if(m_direction==cd_horiz_right) (*dwPos)+=0+0*Map->GetIsoSize();
	if(m_direction==cd_horiz_left) 
	{
		
		int ground=Map->GetFielddataAt(*dwPos)->wGround;
		(*dwPos)+=0+Map->GetIsoSize();
		if(ground==0xFFFF) ground=0;
		if((*tiledata)[ground].wTileSet==cliffset)
		{
			(*dwPos)-=Map->GetIsoSize();
			//ModifyStartPos(dwPos, bSmall);
		}
	}

	if(m_direction==cd_vertic_top) 
	{
		int ground=Map->GetFielddataAt(*dwPos)->wGround;
			
		(*dwPos)+=1;
		if(ground==0xFFFF) ground=0;
		if((*tiledata)[ground].wTileSet==cliffset)
		{
			(*dwPos)-=1;
			//ModifyStartPos(dwPos, bSmall);
		}
	}

}

CString CBackCliffModifier::GetDataSection()
{
	if(m_bAlternative) return ("CliffBackDataAlt");
	return("CliffBackData");
}
