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

#if !defined(AFX_SCRIPTTYPES_H__981EF700_951E_11D3_B63B_BCCF9C98B341__INCLUDED_)
#define AFX_SCRIPTTYPES_H__981EF700_951E_11D3_B63B_BCCF9C98B341__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptTypes.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CScriptTypes 

class CScriptTypes : public CDialog
{
	DECLARE_DYNCREATE(CScriptTypes)

// Konstruktion
public:
	void UpdateDialog();
	CScriptTypes();
	~CScriptTypes();

// Dialogfelddaten
	//{{AFX_DATA(CScriptTypes)
	enum { IDD = IDD_SCRIPTTYPES };
	CEdit	m_DescriptionEx;
	CStatic	m_Desc;
	CComboBox	m_Type;
	CComboBox	m_ScriptType;
	CComboBox	m_Param;
	CListBox	m_Action;
	CString	m_Name;
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CScriptTypes)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	void ListBehaviours(CComboBox& cb);
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CScriptTypes)
	afx_msg void OnEditchangeScripttype();
	afx_msg void OnSelchangeScripttype();
	afx_msg void OnSelchangeAction();
	afx_msg void OnChangeName();
	afx_msg void OnEditchangeType();
	afx_msg void OnSelchangeType();
	afx_msg void OnEditchangeParam();
	afx_msg void OnSelchangeParam();
	afx_msg void OnAddaction();
	afx_msg void OnDeleteaction();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_SCRIPTTYPES_H__981EF700_951E_11D3_B63B_BCCF9C98B341__INCLUDED_
