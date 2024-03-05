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

// MapD.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "MapD.h"
#include "resource.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "changesizedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CMapD 

IMPLEMENT_DYNCREATE(CMapD, CDialog)

CMapD::CMapD() : CDialog(CMapD::IDD)
{
	//{{AFX_DATA_INIT(CMapD)
	m_Width = _T("");
	m_Height = _T("");
	//}}AFX_DATA_INIT
}

CMapD::~CMapD()
{
}

void CMapD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapD)
	DDX_Control(pDX, IDC_USESIZE, m_LocalSize);
	DDX_Control(pDX, IDC_THEATER, m_Theater);
	DDX_Text(pDX, IDC__SIZEX, m_Width);
	DDX_Text(pDX, IDC__SIZEY, m_Height);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapD, CDialog)
	//{{AFX_MSG_MAP(CMapD)
	ON_EN_CHANGE(IDC_USESIZE, OnChangeUsesize)
	ON_CBN_EDITCHANGE(IDC_THEATER, OnEditchangeTheater)
	ON_BN_CLICKED(IDC_CHANGELOCAL, OnChangelocal)
	ON_CBN_SELCHANGE(IDC_THEATER, OnEditchangeTheater)
	ON_BN_CLICKED(IDC_CHANGE, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CMapD 

void CMapD::UpdateDialog()
{
	CIniFile& ini=Map->GetIniFile();

	m_LocalSize.SetWindowText( ini.sections["Map"].values["LocalSize"] );
	//m_Size.SetWindowText( ini.sections["Map"].values["Size"] );
	m_Theater.SetWindowText( ini.sections["Map"].values["Theater"] );

	char c[50];
	itoa(Map->GetWidth(), c, 10);
	m_Width=c;
	itoa(Map->GetHeight(), c, 10);
	m_Height=c;

	CDialog::UpdateData(FALSE);
}

void CMapD::UpdateData()
{
	//MessageBox("This function ( UpdateData() ) should not be called, please contact the author.");
}

void CMapD::OnChangeUsesize() 
{
	
}

void CMapD::OnEditchangeTheater() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Map"].values["Theater"]=GetText(&m_Theater);	
}

void CMapD::UpdateStrings()
{

	GetDlgItem(IDC_DESC)->SetWindowText(GetLanguageStringACP("MapDesc"));
	GetDlgItem(IDC_SIZEFRAME)->SetWindowText(GetLanguageStringACP("MapSizeFrame"));
	GetDlgItem(IDC_LSIZE)->SetWindowText(GetLanguageStringACP("MapSize"));
	GetDlgItem(IDC_USEABLEFRAME)->SetWindowText(GetLanguageStringACP("MapVisibleSizeFrame"));
	GetDlgItem(IDC_LUSEABLE)->SetWindowText(GetLanguageStringACP("MapVisibleSize"));
	GetDlgItem(IDC_LTHEATER)->SetWindowText(GetLanguageStringACP("MapTheater"));
}

void CMapD::OnChangelocal() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Map"].values["LocalSize"]=GetText(&m_LocalSize);
	
	Map->CalcMapRect();
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CMapD::OnChange() 
{
	/*
	CDialog::UpdateData(TRUE);

	int width, height;
	width=atoi(m_Width);
	height=atoi(m_Height);

	Map->ResizeMap(width, height);
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	*/

	CChangeSizeDlg dlg;
	if(dlg.DoModal()==IDCANCEL) return;

	if(dlg.m_Width<16 || dlg.m_Width>400 || dlg.m_Height<16 || dlg.m_Height>400 || (dlg.m_Width + dlg.m_Height) > 512)
	{
		MessageBox("Width and Height must both be between 16 and 400 and both added must be less than 512.", "Error");
		return;
	}

	Map->ResizeMap(dlg.m_Left, dlg.m_Top, dlg.m_Width, dlg.m_Height);


	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_minimap.UpdateView();

	char c[50];
	itoa(dlg.m_Width, c, 10);
	
	m_Width=c;
	itoa(dlg.m_Height, c, 10);
	m_Height=c;
	CDialog::UpdateData(FALSE);
}
