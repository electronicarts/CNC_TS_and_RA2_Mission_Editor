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

#if !defined(AFX_TREES_H__8ACDE600_72AB_11D3_99E1_95E76FDD1C05__INCLUDED_)
#define AFX_TREES_H__8ACDE600_72AB_11D3_99E1_95E76FDD1C05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Trees.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTrees 

class CTrees : public CPropertyPage
{
	DECLARE_DYNCREATE(CTrees)

// Konstruktion
public:
	void UpdateDialog();
	CTrees();
	~CTrees();

// Dialogfelddaten
	//{{AFX_DATA(CTrees)
	enum { IDD = IDD_TREES };
	CEdit	m_Pos;
	CComboBox	m_Type;
	CListBox	m_TreeList;
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTrees)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CTrees)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTreelist();
	afx_msg void OnEditchangeType();
	afx_msg void OnKillfocusType();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDelete();
	afx_msg void OnNew();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TREES_H__8ACDE600_72AB_11D3_99E1_95E76FDD1C05__INCLUDED_
