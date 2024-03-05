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

#if !defined(TIPDLG_H_INCLUDED_)
#define TIPDLG_H_INCLUDED_

// CG: Diese Datei wurde von der Komponente "Tipps und Tricks" hinzugefügt

/////////////////////////////////////////////////////////////////////////////
// CTipDlg Dialogfeld

class CTipDlg : public CDialog
{
// Konstruktion
public:
	CTipDlg(CWnd* pParent = NULL);	 // Standard-Konstruktor

// Dialogfelddaten
	//{{AFX_DATA(CTipDlg)
	enum { IDD = IDD_TIP };
	BOOL	m_bStartup;
	CString	m_strTip;
	//}}AFX_DATA

	FILE* m_pStream;

// Überschreibungen
	// Vom Klassen-Assistenten generierte Überschreibungen virtueller Funktionen
	//{{AFX_VIRTUAL(CTipDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CTipDlg();

protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CTipDlg)
	afx_msg void OnNextTip();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void GetNextTipString(CString& strNext);
};

#endif // !defined(TIPDLG_H_INCLUDED_)
