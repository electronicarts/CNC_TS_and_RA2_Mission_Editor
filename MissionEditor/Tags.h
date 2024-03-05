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

#if !defined(AFX_TAGS_H__103BF600_9378_11D3_B63B_8867017BB441__INCLUDED_)
#define AFX_TAGS_H__103BF600_9378_11D3_B63B_8867017BB441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Tags.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTags 

class CTags : public CDialog
{
	DECLARE_DYNCREATE(CTags)

// Konstruktion
public:
	void UpdateDialog();
	CTags();
	~CTags();

// Dialogfelddaten
	//{{AFX_DATA(CTags)
	enum { IDD = IDD_TAGS };
	CComboBox	m_Repeat;
	CComboBox	m_Tag;
	CComboBox	m_Trigger;
	CString	m_Name;
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTags)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CTags)
	afx_msg void OnSelchangeTag();
	afx_msg void OnChangeName();
	afx_msg void OnEditchangeRepeat();
	afx_msg void OnSelchangeRepeat();
	afx_msg void OnEditchangeTrigger();
	afx_msg void OnSelchangeTrigger();
	afx_msg void OnDelete();
	afx_msg void OnAdd();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TAGS_H__103BF600_9378_11D3_B63B_8867017BB441__INCLUDED_
