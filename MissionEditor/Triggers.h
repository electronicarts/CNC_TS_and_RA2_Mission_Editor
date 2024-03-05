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

#if !defined(AFX_TRIGGERS_H__1E811180_939D_11D3_B63B_F94AB129C441__INCLUDED_)
#define AFX_TRIGGERS_H__1E811180_939D_11D3_B63B_F94AB129C441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Triggers.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTriggers 

class CTriggers : public CDialog
{
	DECLARE_DYNCREATE(CTriggers)

// Konstruktion
public:
	void UpdateDialog();
	CTriggers();
	~CTriggers();

// Dialogfelddaten
	//{{AFX_DATA(CTriggers)
	enum { IDD = IDD_TRIGGERS };
	CComboBox	m_E2;
	CComboBox	m_E1;
	CComboBox	m_A6;
	CComboBox	m_A5;
	CComboBox	m_A4;
	CComboBox	m_A3;
	CComboBox	m_A2;
	CComboBox	m_A1;
	CComboBox	m_House;
	CComboBox	m_Trigger2;
	CComboBox	m_Trigger;
	CComboBox	m_EventType;
	CComboBox	m_Event;
	CComboBox	m_ActionWaypoint;
	CComboBox	m_ActionType;
	CComboBox	m_Action;
	CString	m_F1;
	CString	m_F2;
	CString	m_F3;
	CString	m_F4;
	CString	m_F5;
	CString	m_LA1;
	CString	m_LA2;
	CString	m_LA3;
	CString	m_LA4;
	CString	m_LA5;
	CString	m_LA6;
	CString	m_LE1;
	CString	m_LE2;
	CString	m_Name;
	CString	m_LAW;
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTriggers)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CTriggers)
	afx_msg void OnSelchangeTrigger();
	afx_msg void OnSelchangeEvent();
	afx_msg void OnSelchangeAction();
	afx_msg void OnEditchangeHouse();
	afx_msg void OnSelchangeHouse();
	afx_msg void OnChangeName();
	afx_msg void OnChangeFlag1();
	afx_msg void OnChangeFlag2();
	afx_msg void OnChangeFlag3();
	afx_msg void OnChangeFlag4();
	afx_msg void OnChangeFlag5();
	afx_msg void OnEditchangeTrigger2();
	afx_msg void OnSelchangeTrigger2();
	afx_msg void OnEditchangeEventtype();
	afx_msg void OnSelchangeEventtype();
	afx_msg void OnEditchangeEventparam1();
	afx_msg void OnSelchangeEventparam1();
	afx_msg void OnEditchangeEventparam2();
	afx_msg void OnSelchangeEventparam2();
	afx_msg void OnEditchangeActiontype();
	afx_msg void OnSelchangeActiontype();
	afx_msg void OnEditchangeActionwaypoint();
	afx_msg void OnSelchangeActionwaypoint();
	afx_msg void OnEditchangeActionparam1();
	afx_msg void OnSelchangeActionparam1();
	afx_msg void OnEditchangeActionparam2();
	afx_msg void OnSelchangeActionparam2();
	afx_msg void OnEditchangeActionparam3();
	afx_msg void OnSelchangeActionparam3();
	afx_msg void OnEditchangeActionparam4();
	afx_msg void OnSelchangeActionparam4();
	afx_msg void OnEditchangeActionparam5();
	afx_msg void OnSelchangeActionparam5();
	afx_msg void OnEditchangeActionparam6();
	afx_msg void OnSelchangeActionparam6();
	afx_msg void OnAddevent();
	afx_msg void OnDeleteevent();
	afx_msg void OnAddaction();
	afx_msg void OnDeleteaction();
	afx_msg void OnDeletetrigger();
	afx_msg void OnAddtrigger();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TRIGGERS_H__1E811180_939D_11D3_B63B_F94AB129C441__INCLUDED_
