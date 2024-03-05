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

// ToolSettingsBar.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "ToolSettingsBar.h"
#include "FinalSunDlg.h"
#include "mapdata.h"
#include "variables.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CToolSettingsBar 


CToolSettingsBar::CToolSettingsBar(CWnd* pParent /*=NULL*/)
	: CDialogBar()
{
	//{{AFX_DATA_INIT(CToolSettingsBar)
	m_BrushSize = 0;
	//}}AFX_DATA_INIT
}


void CToolSettingsBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CToolSettingsBar)
	DDX_CBIndex(pDX, IDC_BRUSHSIZE, m_BrushSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CToolSettingsBar, CDialogBar)
	//{{AFX_MSG_MAP(CToolSettingsBar)
	ON_CBN_SELCHANGE(IDC_BRUSHSIZE, OnSelchangeBrushsize)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CToolSettingsBar 



void CToolSettingsBar::OnSelchangeBrushsize() 
{

	UpdateData(TRUE);
	
	int bx,by;

	switch(m_BrushSize)
	{
	case 0:
		bx=1;
		by=1;
		break;
	case 1:
		bx=2;
		by=2;
		break;
	case 2:
		bx=3;
		by=3;
		break;
	case 3:
		bx=4;
		by=4;
		break;
	case 4:
		bx=5;
		by=5;
		break;
	case 5:
		bx=10;
		by=10;
		break;
	case 6:
		bx=1;
		by=2;
		break;
	case 7:
		bx=2;
		by=1;
		break;
	case 8:
		bx=1;
		by=3;
		break;
	case 9:
		bx=3;
		by=1;
		break;
	}

	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x=bx;	
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y=by;	
}

BOOL CToolSettingsBar::OnInitDialog() 
{
	m_BrushSize=0;
	UpdateData(FALSE);

	((CComboBox*)GetDlgItem(IDC_BRUSHSIZE))->SetCurSel(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CToolSettingsBar::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialogBar::OnShowWindow(bShow, nStatus);

	UpdateData(FALSE);	
}
