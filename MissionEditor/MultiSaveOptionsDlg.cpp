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

// MultiSaveOptionsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "MultiSaveOptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CMultiSaveOptionsDlg 


CMultiSaveOptionsDlg::CMultiSaveOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMultiSaveOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMultiSaveOptionsDlg)
	m_mmx = 0;
	//}}AFX_DATA_INIT
}


void CMultiSaveOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiSaveOptionsDlg)
	DDX_Radio(pDX, IDC_MMX, m_mmx);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiSaveOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CMultiSaveOptionsDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CMultiSaveOptionsDlg 
