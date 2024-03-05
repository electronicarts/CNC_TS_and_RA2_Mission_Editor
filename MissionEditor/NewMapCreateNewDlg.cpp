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

// NewMapCreateNewDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "NewMapCreateNewDlg.h"
#include "variables.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CNewMapCreateNewDlg 


CNewMapCreateNewDlg::CNewMapCreateNewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewMapCreateNewDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewMapCreateNewDlg)
	m_Height = 0;
	m_Width = 0;
	m_StartingHeight = -1;
	m_Theater = -1;
	//}}AFX_DATA_INIT
}


void CNewMapCreateNewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewMapCreateNewDlg)
	DDX_Text(pDX, IDC_HEIGHT, m_Height);
	DDX_Text(pDX, IDC_WIDTH, m_Width);
	DDX_CBIndex(pDX, IDC_STARTINGHEIGHT, m_StartingHeight);
	DDX_CBIndex(pDX, IDC_THEATER, m_Theater);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewMapCreateNewDlg, CDialog)
	//{{AFX_MSG_MAP(CNewMapCreateNewDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CNewMapCreateNewDlg 

BOOL CNewMapCreateNewDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CComboBox& theater=*((CComboBox*)GetDlgItem(IDC_THEATER));
	theater.AddString(THEATER0);
	theater.AddString(THEATER1);
#ifdef RA2_MODE
	theater.AddString(THEATER2);
	if(yuri_mode) // MW YR support
	{
		theater.AddString(THEATER3);
		theater.AddString(THEATER4);
		theater.AddString(THEATER5);
	}

#endif

	m_Theater=0;
	m_Width=50;
	m_Height=50;
	m_StartingHeight=0;

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CNewMapCreateNewDlg::OnOK() 
{
	
	UpdateData(TRUE);

	if(m_Width>400 || m_Height>400 || m_Width<16 || m_Height<16 || (m_Width + m_Height) > 512)
	{
		MessageBox("Width and Height must both be between 16 and 400 and both added must be less than 512.","Error");
		return;
	}

	if(m_Width+m_Height>256)
	{
#ifdef RA2_MODE
		int res=MessageBox("Width + height is bigger than 256, this may cause problems in RA2. Continue?","Warning",MB_YESNO);
#else
		int res=MessageBox("Width + height is bigger than 256, this may cause problems in TS. Continue?","Warning",MB_YESNO);
#endif
		if(res==IDNO) return;
	}

	CDialog::OnOK();
}
