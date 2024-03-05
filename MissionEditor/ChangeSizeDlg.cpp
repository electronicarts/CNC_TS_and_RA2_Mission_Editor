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

// ChangeSizeDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "ChangeSizeDlg.h"
#include "variables.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CChangeSizeDlg 


CChangeSizeDlg::CChangeSizeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChangeSizeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangeSizeDlg)
	m_Left = 0;
	m_Height = 0;
	m_Top = 0;
	m_Width = 0;
	//}}AFX_DATA_INIT
	m_Width=Map->GetWidth();
	m_Height=Map->GetHeight();
}


void CChangeSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangeSizeDlg)
	DDX_Text(pDX, IDC_LEFT, m_Left);
	DDX_Text(pDX, IDC_HEIGHT, m_Height);
	DDV_MinMaxInt(pDX, m_Height, 0, 400);
	DDX_Text(pDX, IDC_TOP, m_Top);
	DDX_Text(pDX, IDC_WIDTH, m_Width);
	DDV_MinMaxInt(pDX, m_Width, 0, 400);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangeSizeDlg, CDialog)
	//{{AFX_MSG_MAP(CChangeSizeDlg)
	ON_EN_CHANGE(IDC_WIDTH, OnChangeWidth)
	ON_EN_CHANGE(IDC_HEIGHT, OnChangeHeight)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CChangeSizeDlg 

void CChangeSizeDlg::OnChangeWidth() 
{	
	if(UpdateData(TRUE))
	{
		m_Left=(m_Width-Map->GetWidth())/2;
		UpdateData(FALSE);
	}
}

void CChangeSizeDlg::OnChangeHeight() 
{
	if(UpdateData(TRUE))
	{
		m_Top=(m_Height-Map->GetHeight())/2;
		UpdateData(FALSE);
	}		
}
