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

// NewMapTypeDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "NewMapTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CNewMapTypeDlg 


CNewMapTypeDlg::CNewMapTypeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewMapTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewMapTypeDlg)
	m_Singleplayer = -1;
	//}}AFX_DATA_INIT
}


void CNewMapTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewMapTypeDlg)
	DDX_Radio(pDX, IDC_SINGLE, m_Singleplayer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewMapTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CNewMapTypeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CNewMapTypeDlg 

BOOL CNewMapTypeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_Singleplayer=1;
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}
