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

#include "stdafx.h"
#include "finalsun.h"
#include "ComboUInputDlg.h"
#include "functions.h"
#include "inlines.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CComboUInputDlg 


CComboUInputDlg::CComboUInputDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CComboUInputDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CComboUInputDlg)
	m_Caption = _T("");
	m_Combo = _T("");
	//}}AFX_DATA_INIT

	m_type=0;
	bTruncateStrings=FALSE;
}


void CComboUInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CComboUInputDlg)
	DDX_Text(pDX, IDC_CAPTION, m_Caption);
	DDX_CBString(pDX, IDC_COMBO1, m_Combo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CComboUInputDlg, CDialog)
	//{{AFX_MSG_MAP(CComboUInputDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CComboUInputDlg 

BOOL CComboUInputDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CComboBox* box=(CComboBox*)GetDlgItem(IDC_COMBO1);

	switch(m_type)
	{
		case COMBOUINPUT_HOUSES:
		{
			ListHouses(*box, FALSE, FALSE);
			break;
		}
		case COMBOUINPUT_COUNTRIES:
			ListHouses(*box, FALSE, TRUE);
			break;
		case COMBOUINPUT_TRIGGERS:
			ListTriggers(*box);
			break;
		case COMBOUINPUT_TAGS:
			ListTags(*box, FALSE);
			break;
		case COMBOUINPUT_HOUSES_N:
		{
			ListHouses(*box, TRUE, FALSE);
			break;
		}
		case COMBOUINPUT_COUNTRIES_N:
			ListHouses(*box, TRUE, TRUE);
			break;
		case COMBOUINPUT_MANUAL:
			int i;
			for(i=0;i<m_ManualStrings.size();i++)
				box->AddString(m_ManualStrings[i]);
	}

	box->SetCurSel(0);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CComboUInputDlg::OnOK() 
{
	UpdateData(TRUE);
	
	if(m_type==COMBOUINPUT_TRIGGERS || m_type==COMBOUINPUT_TAGS || m_type==COMBOUINPUT_HOUSES_N || m_type==COMBOUINPUT_COUNTRIES_N) TruncSpace(m_Combo);
	else if(m_type==COMBOUINPUT_MANUAL)
	{
		if(bTruncateStrings) TruncSpace(m_Combo);
	}
	else
	{
		m_Combo=TranslateHouse(m_Combo, FALSE);
	}

	EndDialog(0);	

	//CDialog::OnOK();
}

void CComboUInputDlg::OnCancel() 
{
	// TODO: Zusätzlichen Bereinigungscode hier einfügen
	
	//CDialog::OnCancel();
}
