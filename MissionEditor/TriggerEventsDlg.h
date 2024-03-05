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

#if !defined(AFX_TRIGGEREVENTSDLG_H__13CDEBA1_961C_11D4_9C87_BD9803B9B54A__INCLUDED_)
#define AFX_TRIGGEREVENTSDLG_H__13CDEBA1_961C_11D4_9C87_BD9803B9B54A__INCLUDED_

#include "resource.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TriggerEventsDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTriggerEventsDlg 

class CTriggerEventsDlg : public CDialog
{
// Konstruktion
public:
	void Clear();
	CString m_currentTrigger;
	void UpdateDialog();
	CTriggerEventsDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CTriggerEventsDlg)
	enum { IDD = IDD_TRIGGEREVENTS };
	CMyComboBox	m_ParamValue;
	CListBox	m_Parameter;
	CMyComboBox	m_EventType;
	CEdit	m_EventDescription;
	CComboBox	m_Event;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTriggerEventsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CTriggerEventsDlg)
	afx_msg void OnNewevent();
	afx_msg void OnDeleteevent();
	afx_msg void OnSelchangeEvent();
	afx_msg void OnEditchangeEventtype();
	afx_msg void OnSelchangeParameter();
	afx_msg void OnEditchangeParamvalue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TRIGGEREVENTSDLG_H__13CDEBA1_961C_11D4_9C87_BD9803B9B54A__INCLUDED_
