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

#if !defined(AFX_ALL1_H__B5D522E4_69CE_11D3_99E1_C138647F2A00__INCLUDED_)
#define AFX_ALL1_H__B5D522E4_69CE_11D3_99E1_C138647F2A00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// All1.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CAll 

class CAll : public CDialog
{
// Konstruktion
public:
	void UpdateDialog();
	CAll(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CAll)
	enum { IDD = IDD_ALL };
	CEdit	m_Value;
	CListBox	m_Keys;
	CButton	m_IniSection;
	CButton	m_DeleteSection;
	CButton	m_DeleteKey;
	CButton	m_AddSection;
	CButton	m_AddKey;
	CComboBox	m_Sections;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CAll)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CAll)
	afx_msg void OnSelchangeSections();
	afx_msg void OnChangeValue();
	afx_msg void OnSelchangeKeys();
	afx_msg void OnUpdateValue();
	afx_msg void OnAddsection();
	afx_msg void OnDeletesection();
	afx_msg void OnDeletekey();
	afx_msg void OnAddkey();
	afx_msg void OnInisection();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_ALL1_H__B5D522E4_69CE_11D3_99E1_C138647F2A00__INCLUDED_
