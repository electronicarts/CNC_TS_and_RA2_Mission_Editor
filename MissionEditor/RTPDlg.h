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

#if !defined(AFX_RTPDLG_H__8E65AA00_2C54_11D5_89B2_444553540000__INCLUDED_)
#define AFX_RTPDLG_H__8E65AA00_2C54_11D5_89B2_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RTPDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CRTPDlg 

class CRTPDlg : public CDialog
{
// Konstruktion
public:
	CRTPDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CRTPDlg)
	enum { IDD = IDD_TERRAINPLACING };
	CButton	m_Preview;
	CListBox	m_Used;
	CListBox	m_Available;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CRTPDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	CString m_LastSelected;

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CRTPDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	virtual void OnOK();
	afx_msg void OnSelchangeAvail();
	afx_msg void OnDblclkAvail();
	afx_msg void OnSelchangeUsed();
	afx_msg void OnDblclkUsed();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_RTPDLG_H__8E65AA00_2C54_11D5_89B2_444553540000__INCLUDED_
