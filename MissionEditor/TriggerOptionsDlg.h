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

#if !defined(AFX_TRIGGEROPTIONSDLG_H__FA26A541_949D_11D4_9C87_CBD54CC4BF4A__INCLUDED_)
#define AFX_TRIGGEROPTIONSDLG_H__FA26A541_949D_11D4_9C87_CBD54CC4BF4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TriggerOptionsDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTriggerOptionsDlg 

class CTriggerOptionsDlg : public CDialog
{
// Konstruktion
public:
	void Clear();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void UpdateDialog();
	CString m_currentTrigger;
	CTriggerOptionsDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CTriggerOptionsDlg)
	enum { IDD = IDD_TRIGGEROPTIONS };
	CButton	m_Medium;
	CButton	m_Hard;
	CButton	m_Easy;
	CButton	m_Disabled;
	CMyComboBox	m_TriggerType;
	CEdit	m_Name;
	CMyComboBox	m_House;
	CMyComboBox	m_AttachedTrigger;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTriggerOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	CToolTipCtrl m_tooltip;
	virtual BOOL OnInitDialog();

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CTriggerOptionsDlg)
	afx_msg void OnChangeName();
	afx_msg void OnEditchangeHouse();
	afx_msg void OnEditchangeAttachedtrigger();
	afx_msg void OnKillfocusName();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnEditchangeTriggertype();
	afx_msg void OnDisabled();
	afx_msg void OnEasy();
	afx_msg void OnMedium();
	afx_msg void OnHard();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TRIGGEROPTIONSDLG_H__FA26A541_949D_11D4_9C87_CBD54CC4BF4A__INCLUDED_
