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

#if !defined(AFX_BASIC_H__C350C7AC_63C3_11D3_99E0_C30F10710B17__INCLUDED_)
#define AFX_BASIC_H__C350C7AC_63C3_11D3_99E0_C30F10710B17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Basic.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CBasic 

class CBasic : public CDialog
{
	DECLARE_DYNCREATE(CBasic)

// Konstruktion
public:
	void UpdateStrings();
	void UpdateData();
	void UpdateDialog();
	CBasic();
	~CBasic();

// Dialogfelddaten
	//{{AFX_DATA(CBasic)
	enum { IDD = IDD_BASIC };
	CMyComboBox	m_RequiredAddOn;
	CMyComboBox	m_VeinGrowthEnabled;
	CMyComboBox	m_TruckCrate;
	CMyComboBox	m_TrainCrate;
	CMyComboBox	m_TiberiumGrowthEnabled;
	CMyComboBox	m_TiberiumDeathToVisceroid;
	CMyComboBox	m_SkipScore;
	CMyComboBox	m_SkipMapSelect;
	CMyComboBox	m_OneTimeOnly;
	CMyComboBox	m_Official;
	CMyComboBox	m_NextScenario;
	CEdit	m_Percent;
	CEdit	m_NewINIFormat;
	CEdit	m_Name;
	CEdit	m_MultiplayerOnly;
	CEdit	m_InitTime;
	CMyComboBox	m_IgnoreGlobalAITriggers;
	CMyComboBox	m_IceGrowthEnabled;
	CMyComboBox	m_FreeRadar;
	CMyComboBox	m_EndOfGame;
	CEdit	m_CarryOverCap;
	CMyComboBox	m_AltNextScenario;
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CBasic)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CBasic)
	afx_msg void OnChangeName();
	afx_msg void OnEditchangeNextscenario();
	afx_msg void OnEditchangeAltnextscenario();
	afx_msg void OnChangeNewiniformat();
	afx_msg void OnChangeCarryovercap();
	afx_msg void OnEditchangeEndofgame();
	afx_msg void OnEditchangeSkipscore();
	afx_msg void OnEditchangeOnetimeonly();
	afx_msg void OnEditchangeSkipmapselect();
	afx_msg void OnEditchangeOfficial();
	afx_msg void OnEditchangeIgnoreglobalaitriggers();
	afx_msg void OnEditchangeTruckcrate();
	afx_msg void OnEditchangeTraincrate();
	afx_msg void OnChangePercent();
	afx_msg void OnChangeMultiplayeronly();
	afx_msg void OnEditchangeTiberiumgrowthenabled();
	afx_msg void OnEditchangeVeingrowthenabled();
	afx_msg void OnEditchangeIcegrowthenabled();
	afx_msg void OnEditchangeTiberiumdeathtovisceroid();
	afx_msg void OnEditchangeFreeradar();
	afx_msg void OnChangeInittime();
	afx_msg void OnEditchangeRequiredaddon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_BASIC_H__C350C7AC_63C3_11D3_99E0_C30F10710B17__INCLUDED_
