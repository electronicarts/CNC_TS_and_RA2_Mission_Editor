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

#if !defined(AFX_UNIT_H__44A11CC3_84B6_11D3_B63B_F881F458F743__INCLUDED_)
#define AFX_UNIT_H__44A11CC3_84B6_11D3_B63B_F881F458F743__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Unit.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CUnit 

class CUnit : public CDialog
{
// Konstruktion
public:
	void UpdateStrings();
	CString m_strength;
	void Init(CString house="", CString strength="256", CString direction="64", CString action="Guard", CString tag="None", CString flag1="0", CString flag2="0", CString flag3="0", CString flag4="0", CString flag5="0", CString flag6="0");
	CUnit(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CUnit)
	enum { IDD = IDD_UNIT };
	CSliderCtrl	m_strength_ctrl;
	CString	m_direction;
	CString	m_flag1;
	CString	m_flag2;
	CString	m_house;
	CString	m_flag3;
	CString	m_flag4;
	CString	m_flag5;
	CString	m_flag6;
	CString	m_action;
	CString	m_tag;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CUnit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CUnit)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_UNIT_H__44A11CC3_84B6_11D3_B63B_F881F458F743__INCLUDED_
