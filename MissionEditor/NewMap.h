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

#if !defined(AFX_NEWMAP_H__868C3FC0_AF27_11D3_B63B_BE68077E9F41__INCLUDED_)
#define AFX_NEWMAP_H__868C3FC0_AF27_11D3_B63B_BE68077E9F41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewMap.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CNewMap 

class CNewMap : public CDialog
{
// Konstruktion
public:
	CString m_MapToImport;
	void UpdateStrings();
	CNewMap(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CNewMap)
	enum { IDD = IDD_NEWMAP };
	CButton	m_OK;
	BOOL	m_ImportOverlay;
	BOOL	m_ImportTrees;
	BOOL	m_ImportUnits;
	BOOL	m_Multiplayer;
	CString	m_House;
	BOOL	m_PrepareHouses;
	BOOL	m_AutoProduction;
	UINT	m_Height;
	UINT	m_Width;
	int		m_Import;
	int		m_Theater;
	int		m_GroundHeight;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CNewMap)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CNewMap)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	virtual void OnOK();
	afx_msg void OnMultiplayer();
	afx_msg void OnEditchangeImportfile();
	afx_msg void OnImport();
	afx_msg void OnNew();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CMyComboBox	m_ImportFile;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_NEWMAP_H__868C3FC0_AF27_11D3_B63B_BE68077E9F41__INCLUDED_
