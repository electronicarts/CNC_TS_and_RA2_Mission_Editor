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

#if !defined(AFX_MAPD_H__B5D522E9_69CE_11D3_99E1_C138647F2A00__INCLUDED_)
#define AFX_MAPD_H__B5D522E9_69CE_11D3_99E1_C138647F2A00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapD.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CMapD 

class CMapD : public CDialog
{
	DECLARE_DYNCREATE(CMapD)

// Konstruktion
public:
	void UpdateStrings();
	void UpdateData();
	void UpdateDialog();
	CMapD();
	~CMapD();

// Dialogfelddaten
	//{{AFX_DATA(CMapD)
	enum { IDD = IDD_MAP };
	CEdit	m_LocalSize;
	CComboBox	m_Theater;
	CString	m_Width;
	CString	m_Height;
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CMapD)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CMapD)
	afx_msg void OnChangeUsesize();
	afx_msg void OnEditchangeTheater();
	afx_msg void OnChangelocal();
	afx_msg void OnChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_MAPD_H__B5D522E9_69CE_11D3_99E1_C138647F2A00__INCLUDED_
