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

#if !defined(AFX_SPECIALFLAGS_H__500DF000_7058_11D3_99E1_97DB32807805__INCLUDED_)
#define AFX_SPECIALFLAGS_H__500DF000_7058_11D3_99E1_97DB32807805__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpecialFlags.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSpecialFlags 

class CSpecialFlags : public CDialog
{
// Konstruktion
public:
	void UpdateDialog();
	CSpecialFlags(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CSpecialFlags)
	enum { IDD = IDD_SPECIALFLAGS };
	CMyComboBox	m_Visceroids;
	CMyComboBox	m_TiberiumSpreads;
	CMyComboBox	m_TiberiumGrows;
	CMyComboBox	m_TiberiumExplosive;
	CMyComboBox	m_Meteorites;
	CMyComboBox	m_MCVDeploy;
	CMyComboBox	m_IonStorms;
	CMyComboBox	m_InitialVeteran;
	CMyComboBox	m_Inert;
	CMyComboBox	m_HarvesterImmune;
	CMyComboBox	m_FogOfWar;
	CMyComboBox	m_FixedAlliance;
	CMyComboBox	m_DestroyableBridges;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSpecialFlags)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSpecialFlags)
	afx_msg void OnEditchangeTiberiumgrows();
	afx_msg void OnEditchangeTiberiumspreads();
	afx_msg void OnEditchangeTiberiumexplosive();
	afx_msg void OnEditchangeDestroyablebridges();
	afx_msg void OnEditchangeMcvdeploy();
	afx_msg void OnEditchangeInitialveteran();
	afx_msg void OnEditchangeFixedalliance();
	afx_msg void OnEditchangeHarvesterimmune();
	afx_msg void OnEditchangeFogofwar();
	afx_msg void OnEditchangeInert();
	afx_msg void OnEditchangeIonstorms();
	afx_msg void OnEditchangeMeteorites();
	afx_msg void OnEditchangeVisceroids();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_SPECIALFLAGS_H__500DF000_7058_11D3_99E1_97DB32807805__INCLUDED_
