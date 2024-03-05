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

// NewRA2HouseDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "NewRA2HouseDlg.h"
#include "variables.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CNewRA2HouseDlg 


CNewRA2HouseDlg::CNewRA2HouseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewRA2HouseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewRA2HouseDlg)
	m_Country = _T("");
	//}}AFX_DATA_INIT
}


void CNewRA2HouseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewRA2HouseDlg)
	DDX_CBString(pDX, IDC_COUNTRY, m_Country);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewRA2HouseDlg, CDialog)
	//{{AFX_MSG_MAP(CNewRA2HouseDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CNewRA2HouseDlg 

#include "functions.h"

BOOL CNewRA2HouseDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CComboBox* country=(CComboBox*)GetDlgItem(IDC_COUNTRY);
	
	int i;
	for(i=0;i<rules.sections[HOUSES].values.size();i++)
	{
		country->AddString(TranslateHouse(*rules.sections[HOUSES].GetValue(i), TRUE));
	}

	country->SetCurSel(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}
