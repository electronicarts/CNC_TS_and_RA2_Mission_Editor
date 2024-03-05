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

// NewMapSpDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "NewMapSpDlg.h"
#include "variables.h"
#include "functions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CNewMapSpDlg 


CNewMapSpDlg::CNewMapSpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewMapSpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewMapSpDlg)
	m_AutoProd = FALSE;
	m_House = -1;
	m_PrepareHouses = FALSE;
	//}}AFX_DATA_INIT
}


void CNewMapSpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewMapSpDlg)
	DDX_Check(pDX, IDC_AUTOPROD, m_AutoProd);
	DDX_CBIndex(pDX, IDC_HOUSE, m_House);
	DDX_Check(pDX, IDC_PREPAREHOUSES, m_PrepareHouses);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewMapSpDlg, CDialog)
	//{{AFX_MSG_MAP(CNewMapSpDlg)
	ON_BN_CLICKED(IDC_PREPAREHOUSES, OnPreparehouses)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CNewMapSpDlg 

void CNewMapSpDlg::OnPreparehouses() 
{
	UpdateData(TRUE);
	BOOL bEnable=m_PrepareHouses;

	GetDlgItem(IDC_AUTOPROD)->EnableWindow(bEnable);
	GetDlgItem(IDC_HOUSE)->EnableWindow(bEnable);
	
}

BOOL CNewMapSpDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CComboBox& house=*((CComboBox*)(GetDlgItem(IDC_HOUSE)));
	int i;
	for(i=0;i<rules.sections[HOUSES].values.size();i++)
	{
		house.AddString(TranslateHouse(*rules.sections[HOUSES].GetValue(i), TRUE));
	}

	m_PrepareHouses=TRUE;
	m_AutoProd=TRUE;
	m_House=0;
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CNewMapSpDlg::OnOK() 
{	
	CDialog::OnOK();
	//m_House=TranslateHouse(m_House);
}
