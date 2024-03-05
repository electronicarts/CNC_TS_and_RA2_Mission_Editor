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

// CliffModifier.h: Schnittstelle für die Klasse CCliffModifier.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIFFMODIFIER_H__3853D323_CD37_11D4_9C87_F2DC6A2E6849__INCLUDED_)
#define AFX_CLIFFMODIFIER_H__3853D323_CD37_11D4_9C87_F2DC6A2E6849__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TerrainModifier.h"



class CCliffModifier : public CTerrainModifier  
{
	
public:
	virtual BOOL PlaceCliff(DWORD dwXFrom, DWORD dwYFrom, DWORD dwXDest, DWORD dwYDest, BOOL bAlternative=FALSE);
	CCliffModifier();
	virtual ~CCliffModifier();

protected:
	virtual CString GetDataSection()=0;
	virtual void ModifyCurrentPos(DWORD* dwPos, BOOL bBeforePlacing, BOOL bSmall);
	virtual void ModifyStartPos(DWORD* dwStartPos, BOOL bSmall);
	
	enum e_cliffdirections {cd_horiz_left, cd_horiz_right, cd_horizdiag_left,
		cd_horizdiag_right, cd_vertic_top, cd_vertic_bottom, cd_verticdiag_top, cd_verticdiag_bottom };

	DWORD m_dwTo;
	DWORD m_dwFrom;
	e_cliffdirections m_direction;

	int m_addx;
	int m_addy;
	BOOL m_bAlternative;
	virtual DWORD GetTileToPlace(DWORD dwPos, BOOL* bSmall) ;
};

#endif // !defined(AFX_CLIFFMODIFIER_H__3853D323_CD37_11D4_9C87_F2DC6A2E6849__INCLUDED_)
