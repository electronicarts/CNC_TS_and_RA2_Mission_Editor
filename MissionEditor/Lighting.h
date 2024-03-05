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

#if !defined(AFX_LIGHTNING_H__9DC7B326_6DF2_11D3_99E1_D14A1D4DCF07__INCLUDED_)
#define AFX_LIGHTNING_H__9DC7B326_6DF2_11D3_99E1_D14A1D4DCF07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Lightning.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CLighting 



class CLighting : public CDialog
{
	DECLARE_DYNCREATE(CLighting)

// Konstruktion
public:
	void UpdateDialog();
	CLighting();
	~CLighting();

// Dialogfelddaten
	//{{AFX_DATA(CLighting)
	enum { IDD = IDD_LIGHTING };
	CFloatEdit	m_Red2;
	CFloatEdit	m_Red;
	CFloatEdit	m_Level2;
	CFloatEdit	m_Level;
	CFloatEdit	m_Green2;
	CFloatEdit	m_Green;
	CFloatEdit	m_Blue2;
	CFloatEdit	m_Blue;
	CFloatEdit	m_Ambient2;
	CFloatEdit	m_Ambient;
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CLighting)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CLighting)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeAmbient();
	afx_msg void OnChangeLevel();
	afx_msg void OnKillfocusAmbient();
	afx_msg void OnChangeRed();
	afx_msg void OnChangeGreen();
	afx_msg void OnChangeBlue();
	afx_msg void OnChangeAmbient2();
	afx_msg void OnChangeLevel2();
	afx_msg void OnChangeRed2();
	afx_msg void OnChangeGreen2();
	afx_msg void OnChangeBlue2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_LIGHTNING_H__9DC7B326_6DF2_11D3_99E1_D14A1D4DCF07__INCLUDED_
