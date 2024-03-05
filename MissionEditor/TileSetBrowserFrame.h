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

#if !defined(AFX_TILESETBROWSERFRAME_H__1FD3DBE1_7D2F_11D4_9C87_444553540000__INCLUDED_)
#define AFX_TILESETBROWSERFRAME_H__1FD3DBE1_7D2F_11D4_9C87_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TileSetBrowserFrame.h : Header-Datei
//

#include "TerrainDlg.h"
#include "TileSetBrowserView.h"	// Hinzugefügt von der Klassenansicht

/////////////////////////////////////////////////////////////////////////////
// Rahmen CTileSetBrowserFrame 

class CTileSetBrowserFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CTileSetBrowserFrame)
public:
	CTileSetBrowserFrame();           // Dynamische Erstellung verwendet geschützten Konstruktor

// Attribute
public:

// Operationen
public:
	CTerrainDlg m_bar;

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTileSetBrowserFrame)
	public:
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	protected:
	virtual void PostNcDestroy();
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementierung
public:
	CTileSetBrowserView m_view;
	virtual ~CTileSetBrowserFrame();

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CTileSetBrowserFrame)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TILESETBROWSERFRAME_H__1FD3DBE1_7D2F_11D4_9C87_444553540000__INCLUDED_
