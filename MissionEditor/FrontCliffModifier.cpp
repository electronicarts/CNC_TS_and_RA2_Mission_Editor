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

// FrontCliffModifier.cpp: Implementierung der Klasse CFrontCliffModifier.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stdafx.h"
#include "FrontCliffModifier.h"
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

CFrontCliffModifier::CFrontCliffModifier()
{

}

CFrontCliffModifier::~CFrontCliffModifier()
{

}





void CFrontCliffModifier::ModifyStartPos(DWORD *dwStartPos, BOOL bSmall)
{
	
	if(m_direction==cd_horiz_right) (*dwStartPos)-=1+0*Map->GetIsoSize();
	if(m_direction==cd_horiz_left) (*dwStartPos)+=-(1)+Map->GetIsoSize();
	if(m_direction==cd_vertic_top) (*dwStartPos)-=-1+0*Map->GetIsoSize();
	if(m_direction==cd_vertic_bottom) (*dwStartPos)-=0*Map->GetIsoSize();

	if(m_direction==cd_verticdiag_top) (*dwStartPos)+=1+Map->GetIsoSize();
	if(m_direction==cd_verticdiag_bottom) (*dwStartPos)+=0-Map->GetIsoSize();
	
	
	if(m_direction==cd_horizdiag_right) (*dwStartPos)+=1+0*Map->GetIsoSize();
	if(m_direction==cd_horizdiag_left) (*dwStartPos)+=0+1*Map->GetIsoSize();
}

void CFrontCliffModifier::ModifyCurrentPos(DWORD *dwPos, BOOL bBeforePlacing, BOOL bSmall)
{
	if(!bBeforePlacing)
	{
		if(!bSmall)
		{
			if(m_direction==cd_verticdiag_top) (*dwPos)+=0+2*Map->GetIsoSize();	
			if(m_direction==cd_verticdiag_bottom) (*dwPos)+=0-2*Map->GetIsoSize();	
		}
		else
		{
			if(m_direction==cd_verticdiag_top) (*dwPos)+=0+2*Map->GetIsoSize();	
			if(m_direction==cd_verticdiag_bottom) (*dwPos)+=0-2*Map->GetIsoSize();	
		}
	}
	else
	{
		if(bSmall)
		{
			if(m_direction==cd_verticdiag_top) (*dwPos)+=0-1*Map->GetIsoSize();	
			if(m_direction==cd_verticdiag_bottom) (*dwPos)+=0+1*Map->GetIsoSize();	


		}

	}
}

CString CFrontCliffModifier::GetDataSection()
{
	if(m_bAlternative) return ("CliffFrontDataAlt");
	return("CliffFrontData");
}
