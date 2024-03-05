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

#if !defined(AFX_TRIGGERACTIONSDLG_H__234BEB21_96D7_11D4_9C87_E6A6DF12E04A__INCLUDED_)
#define AFX_TRIGGERACTIONSDLG_H__234BEB21_96D7_11D4_9C87_E6A6DF12E04A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TriggerActionsDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTriggerActionsDlg 

class CTriggerActionsDlg : public CDialog
{
// Konstruktion
public:
	void Clear();
	void UpdateDialog();
	CString m_currentTrigger;
	CTriggerActionsDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CTriggerActionsDlg)
	enum { IDD = IDD_TRIGGERACTIONS };
	CMyComboBox	m_ParamValue;
	CListBox	m_Parameter;
	CMyComboBox	m_ActionType;
	CEdit	m_ActionDescription;
	CComboBox	m_Action;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTriggerActionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CTriggerActionsDlg)
	afx_msg void OnSelchangeAction();
	afx_msg void OnEditchangeActiontype();
	afx_msg void OnSelchangeParameter();
	afx_msg void OnEditchangeParamvalue();
	afx_msg void OnNewaction();
	afx_msg void OnDeleteaction();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TRIGGERACTIONSDLG_H__234BEB21_96D7_11D4_9C87_E6A6DF12E04A__INCLUDED_
