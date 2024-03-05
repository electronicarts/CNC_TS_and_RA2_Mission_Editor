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

// SearchWaypointDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "SearchWaypointDlg.h"
#include "variables.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSearchWaypointDlg 


CSearchWaypointDlg::CSearchWaypointDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSearchWaypointDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSearchWaypointDlg)
	m_Waypoints = _T("");
	//}}AFX_DATA_INIT
}


void CSearchWaypointDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSearchWaypointDlg)
	DDX_LBString(pDX, IDC_WAYPOINTS, m_Waypoints);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSearchWaypointDlg, CDialog)
	//{{AFX_MSG_MAP(CSearchWaypointDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSearchWaypointDlg 

BOOL CSearchWaypointDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CListBox& ctrl=*(CListBox*)GetDlgItem(IDC_WAYPOINTS);

	while(ctrl.DeleteString(0)!=LB_ERR);

	int i;
	int count=Map->GetWaypointCount();

	for(i=0;i<count;i++)
	{
		CString id;
		DWORD pos;
		Map->GetWaypointData(i, &id, &pos);

		ctrl.SetItemData(ctrl.InsertString(i, id), i);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CSearchWaypointDlg::OnOK() 
{
	CListBox& ctrl=*(CListBox*)GetDlgItem(IDC_WAYPOINTS);

	int sel=ctrl.GetCurSel();

	if(sel<0) m_WaypointIndex=-1;
	else
	{
		m_WaypointIndex=ctrl.GetItemData(sel);
	}
	
	CDialog::OnOK();
}
