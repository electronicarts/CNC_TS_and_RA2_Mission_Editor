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

#if !defined(AFX_HOUSES_H__80D09600_8932_11D3_B63B_A583BFBD8C41__INCLUDED_)
#define AFX_HOUSES_H__80D09600_8932_11D3_B63B_A583BFBD8C41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Houses.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CHouses 

class CHouses : public CDialog
{
	DECLARE_DYNCREATE(CHouses)

// Konstruktion
public:
	void UpdateStrings();
	void AddHouse(const char* name);
	void UpdateDialog();
	CHouses();
	~CHouses();

// Dialogfelddaten
	//{{AFX_DATA(CHouses)
	enum { IDD = IDD_HOUSES };
	CComboBox	m_HumanPlayer;
	CComboBox	m_TechLevel;
	CComboBox	m_Side;
	CComboBox	m_PlayerControl;
	CComboBox	m_PercentBuilt;
	CComboBox	m_Nodecount;
	CComboBox	m_IQ;
	CComboBox	m_Edge;
	CEdit	m_Credits;
	CComboBox	m_Color;
	CEdit	m_Allies;
	CComboBox	m_ActsLike;
	CComboBox	m_houses;
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CHouses)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CHouses)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeHouses();
	afx_msg void OnPreparehouses();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnAddhouse();
	afx_msg void OnDeletehouse();
	afx_msg void OnKillfocusIq();
	afx_msg void OnKillfocusEdge();
	afx_msg void OnKillfocusSide();
	afx_msg void OnKillfocusColor();
	afx_msg void OnKillfocusAllies();
	afx_msg void OnKillfocusCredits();
	afx_msg void OnEditchangeActslike();
	afx_msg void OnKillfocusNodecount();
	afx_msg void OnKillfocusTechlevel();
	afx_msg void OnKillfocusPercentbuilt();
	afx_msg void OnKillfocusPlayercontrol();
	afx_msg void OnSelchangeHumanplayer();
	afx_msg void OnSelchangeActslike();
	afx_msg void OnSetfocusAllies();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_HOUSES_H__80D09600_8932_11D3_B63B_A583BFBD8C41__INCLUDED_
