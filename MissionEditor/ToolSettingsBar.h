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

#if !defined(AFX_TOOLSETTINGSBAR_H__582DFEA1_7F44_11D4_9C87_F809D2CDBE4A__INCLUDED_)
#define AFX_TOOLSETTINGSBAR_H__582DFEA1_7F44_11D4_9C87_F809D2CDBE4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolSettingsBar.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CToolSettingsBar 

class CToolSettingsBar : public CDialogBar
{
// Konstruktion
public:
	CToolSettingsBar(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CToolSettingsBar)
	enum { IDD = IDD_TOOLSETTINGS };
	int		m_BrushSize;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CToolSettingsBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CToolSettingsBar)
	afx_msg void OnSelchangeBrushsize();
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TOOLSETTINGSBAR_H__582DFEA1_7F44_11D4_9C87_F809D2CDBE4A__INCLUDED_
