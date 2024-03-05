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

#if !defined(AFX_INFANTRIE_H__F5A248C4_84A5_11D3_B63B_F881F458F743__INCLUDED_)
#define AFX_INFANTRIE_H__F5A248C4_84A5_11D3_B63B_F881F458F743__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Infantrie.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CInfantrie 

class CInfantrie : public CDialog
{
// Konstruktion
public:
	void UpdateStrings();
	CString m_strength;
	void Init(CString house="", CString strength="256", CString action="Guard", CString direction="64", CString tag="None", CString p1="0", CString p2="-1", CString p3="0", CString p4="0", CString p5="0");
	CInfantrie(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CInfantrie)
	enum { IDD = IDD_INFANTRY };
	CSliderCtrl	m_strength_ctrl;
	CString	m_direction;
	CString	m_house;
	CString	m_flag1;
	CString	m_flag2;
	CString	m_flag3;
	CString	m_flag4;
	CString	m_action;
	CString	m_tag;
	CString	m_flag5;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CInfantrie)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CInfantrie)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_INFANTRIE_H__F5A248C4_84A5_11D3_B63B_F881F458F743__INCLUDED_
