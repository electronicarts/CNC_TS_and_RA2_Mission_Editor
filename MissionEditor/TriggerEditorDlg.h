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

#if !defined(AFX_TRIGGEREDITORDLG_H__E9CA56E1_93E6_11D4_9C87_F3E3AC59A04A__INCLUDED_)
#define AFX_TRIGGEREDITORDLG_H__E9CA56E1_93E6_11D4_9C87_F3E3AC59A04A__INCLUDED_

#include "TriggerOptionsDlg.h"	// Hinzugefügt von der Klassenansicht
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TriggerEditorDlg.h : Header-Datei
//

#include "MyComboBox.h"
#include "TriggerEventsDlg.h"	// Hinzugefügt von der Klassenansicht
#include "TriggerActionsDlg.h"	// Hinzugefügt von der Klassenansicht

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTriggerEditorDlg 

class CTriggerEditorDlg : public CDialog
{
// Konstruktion
public:
	void Clear();
	CTriggerActionsDlg m_TriggerActions;
	CTriggerEventsDlg m_TriggerEvents;
	CTriggerOptionsDlg m_TriggerOptions;
	void UpdateDialog();
	CTriggerEditorDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CTriggerEditorDlg)
	enum { IDD = IDD_TRIGGEREDITOR };
	CMyComboBox	m_Trigger;
	CTabCtrl	m_tab;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTriggerEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CTriggerEditorDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnNewtrigger();
	afx_msg void OnDeletetrigger();
	afx_msg void OnSelchangeTrigger();
	afx_msg void OnEditchangeTrigger();
	afx_msg void OnSelchangeTriggertab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPlaceonmap();
	afx_msg void OnClone();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TRIGGEREDITORDLG_H__E9CA56E1_93E6_11D4_9C87_F3E3AC59A04A__INCLUDED_
