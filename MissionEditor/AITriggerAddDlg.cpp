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

// AITriggerAddDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "AITriggerAddDlg.h"
#include "Variables.h"
#include "inlines.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CAITriggerAddDlg 


CAITriggerAddDlg::CAITriggerAddDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAITriggerAddDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAITriggerAddDlg)
	m_AITrigger = _T("");
	//}}AFX_DATA_INIT
}


void CAITriggerAddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAITriggerAddDlg)
	DDX_LBString(pDX, IDC_AITRIGGERS, m_AITrigger);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAITriggerAddDlg, CDialog)
	//{{AFX_MSG_MAP(CAITriggerAddDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CAITriggerAddDlg 

BOOL CAITriggerAddDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CListBox* lb=(CListBox*)GetDlgItem(IDC_AITRIGGERS);

	int i;
	for(i=0;i<ai.sections["AITriggerTypes"].values.size();i++)
	{
		lb->AddString(*ai.sections["AITriggerTypes"].GetValueName(i)+ (CString)" " +GetParam(*ai.sections["AITriggerTypes"].GetValue(i), 0));
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}
