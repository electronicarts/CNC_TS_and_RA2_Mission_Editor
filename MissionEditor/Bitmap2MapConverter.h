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

// Bitmap2MapConverter.h: Schnittstelle für die Klasse CBitmap2MapConverter.
//
//////////////////////////////////////////////////////////////////////

#include "mapdata.h"

#if !defined(AFX_BITMAP2MAPCONVERTER_H__42E61B61_E18A_11D4_9C88_816E6DEF4C47__INCLUDED_)
#define AFX_BITMAP2MAPCONVERTER_H__42E61B61_E18A_11D4_9C88_816E6DEF4C47__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBitmap2MapConverter  
{
public:
	BOOL Convert(HBITMAP hBitmap, CMapData& mapdata);
	CBitmap2MapConverter();
	virtual ~CBitmap2MapConverter();

};

#endif // !defined(AFX_BITMAP2MAPCONVERTER_H__42E61B61_E18A_11D4_9C88_816E6DEF4C47__INCLUDED_)
