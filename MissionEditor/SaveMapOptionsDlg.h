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

#if !defined(AFX_SAVEMAPOPTIONSDLG_H__DD7874E1_CED2_11D4_9C87_444553540000__INCLUDED_)
#define AFX_SAVEMAPOPTIONSDLG_H__DD7874E1_CED2_11D4_9C87_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveMapOptionsDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSaveMapOptionsDlg 

class CSaveMapOptionsDlg : public CDialog
{
// Konstruktion
public:
	CSaveMapOptionsDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CSaveMapOptionsDlg)
	enum { IDD = IDD_SAVEOPTIONS };
	int		m_Compress;
	int		m_PreviewMode;
	CString	m_MapName;
	BOOL	m_AirWar;
	BOOL	m_Cooperative;
	BOOL	m_Duel;
	BOOL	m_Meatgrind;
	BOOL	m_Megawealth;
	BOOL	m_Navalwar;
	BOOL	m_Nukewar;
	BOOL	m_Standard;
	BOOL	m_TeamGame;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSaveMapOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSaveMapOptionsDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_SAVEMAPOPTIONSDLG_H__DD7874E1_CED2_11D4_9C87_444553540000__INCLUDED_
