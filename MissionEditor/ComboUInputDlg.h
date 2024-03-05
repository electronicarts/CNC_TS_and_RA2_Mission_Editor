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

#if !defined(AFX_COMBOUINPUTDLG_H__2EEED280_9A8C_11D5_89B3_00E07D97C331__INCLUDED_)
#define AFX_COMBOUINPUTDLG_H__2EEED280_9A8C_11D5_89B3_00E07D97C331__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ComboUInputDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CComboUInputDlg 

#include <vector>

using namespace std;

#define COMBOUINPUT_HOUSES 0
#define COMBOUINPUT_COUNTRIES 1
#define COMBOUINPUT_TRIGGERS 2
#define COMBOUINPUT_TAGS 3
#define COMBOUINPUT_HOUSES_N 4
#define COMBOUINPUT_COUNTRIES_N 5
#define COMBOUINPUT_MANUAL 6

class CComboUInputDlg : public CDialog
{
// Konstruktion
public:
	int m_type;
	vector<CString> m_ManualStrings;
	BOOL bTruncateStrings;
	CComboUInputDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CComboUInputDlg)
	enum { IDD = IDD_COMBO_UINPUT };
	CString	m_Caption;
	CString	m_Combo;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CComboUInputDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CComboUInputDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_COMBOUINPUTDLG_H__2EEED280_9A8C_11D5_89B3_00E07D97C331__INCLUDED_
