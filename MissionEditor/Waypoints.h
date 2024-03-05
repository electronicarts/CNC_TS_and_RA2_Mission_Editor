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

#if !defined(AFX_WAYPOINTS_H__B0ED0400_7374_11D3_99E1_BB3FC259EC06__INCLUDED_)
#define AFX_WAYPOINTS_H__B0ED0400_7374_11D3_99E1_BB3FC259EC06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Waypoints.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CWaypoints 

class CWaypoints : public CPropertyPage
{
	DECLARE_DYNCREATE(CWaypoints)

// Konstruktion
public:
	void UpdateDialog();
	CWaypoints();
	~CWaypoints();

// Dialogfelddaten
	//{{AFX_DATA(CWaypoints)
	enum { IDD = IDD_WAYPOINTS };
	CStatic	m_dbg;
	CListBox	m_Waypoints;
	CEdit	m_Pos;
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CWaypoints)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CWaypoints)
	afx_msg void OnSelchangeWaypoints();
	afx_msg void OnKillfocusPos();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDelete();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_WAYPOINTS_H__B0ED0400_7374_11D3_99E1_BB3FC259EC06__INCLUDED_
