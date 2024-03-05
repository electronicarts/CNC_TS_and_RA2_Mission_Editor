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

// Aircraft.cpp: implementation file
//

#include "stdafx.h"
#include "FinalSun.h"
#include "Aircraft.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// dialog field CAircraft 


CAircraft::CAircraft(CWnd* pParent /*=NULL*/)
	: CDialog(CAircraft::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAircraft)
	m_direction = _T("");
	m_house = _T("");
	m_flag1 = _T("");
	m_flag2 = _T("");
	m_flag3 = _T("");
	m_flag4 = _T("");
	m_action = _T("");
	m_tag = _T("");
	//}}AFX_DATA_INIT
}


void CAircraft::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAircraft)
	DDX_Control(pDX, IDC_STRENGTH, m_strength_ctrl);
	DDX_CBString(pDX, IDC_DIRECTION, m_direction);
	DDX_CBString(pDX, IDC_HOUSE, m_house);
	DDX_Text(pDX, IDC_P1, m_flag1);
	DDX_Text(pDX, IDC_P2, m_flag2);
	DDX_Text(pDX, IDC_P3, m_flag3);
	DDX_Text(pDX, IDC_P4, m_flag4);
	DDX_CBString(pDX, IDC_STATE, m_action);
	DDX_CBString(pDX, IDC_TAG, m_tag);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAircraft, CDialog)
	//{{AFX_MSG_MAP(CAircraft)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// handlers for messages CAircraft 

void CAircraft::Init(CString house, CString strength, CString direction, CString action, CString tag, CString flag1, CString flag2, CString flag3, CString flag4)
{
	if(house.GetLength()==0) 
	{
		m_house=TranslateHouse(Map->GetHouseID(0), TRUE);
	}
	else
		m_house=TranslateHouse(house, TRUE);

	

	m_flag1=flag1;
	m_flag2=flag2;
	m_flag3=flag3;
	m_flag4=flag4;

	m_action=action;
	m_strength=strength;
	
	m_tag=tag;
	m_direction=direction;
	
}

BOOL CAircraft::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// init the common (!) dialog things
	int i;
	CComboBox* house, *tag;
	
	house=(CComboBox*)GetDlgItem(IDC_HOUSE);
	
	tag=(CComboBox*)GetDlgItem(IDC_TAG);
	
	ListHouses(*house, FALSE);
	ListTags(*tag, TRUE);

	ListYesNo(*(CComboBox*)GetDlgItem(IDC_P3));
	
	UpdateData(FALSE);
	m_strength_ctrl.SetRange(0,256);
	m_strength_ctrl.SetPos(atoi(m_strength));
	
	UpdateStrings();

	return TRUE;  
}

void CAircraft::OnOK() 
{
	CDialog::OnOK();
	m_strength=GetText(&m_strength_ctrl);

	

	UpdateData();
	m_house=TranslateHouse(m_house, FALSE);
	TruncSpace(m_tag);
	TruncSpace(m_flag1);
	TruncSpace(m_flag2);
	TruncSpace(m_flag3);
	TruncSpace(m_flag4);

}

void CAircraft::UpdateStrings()
{
	SetWindowText(GetLanguageStringACP("AirCap"));
	GetDlgItem(IDC_LHOUSE)->SetWindowText(GetLanguageStringACP("AirHouse"));
	GetDlgItem(IDC_LDESC)->SetWindowText(GetLanguageStringACP("AirDesc"));
	GetDlgItem(IDC_LSTRENGTH)->SetWindowText(GetLanguageStringACP("AirStrength"));
	GetDlgItem(IDC_LSTATE)->SetWindowText(GetLanguageStringACP("AirState"));
	GetDlgItem(IDC_LDIRECTION)->SetWindowText(GetLanguageStringACP("AirDirection"));
	GetDlgItem(IDC_LTAG)->SetWindowText(GetLanguageStringACP("AirTag"));
	GetDlgItem(IDC_LP1)->SetWindowText(GetLanguageStringACP("AirP1"));
	GetDlgItem(IDC_LP2)->SetWindowText(GetLanguageStringACP("AirP2"));
	GetDlgItem(IDC_LP3)->SetWindowText(GetLanguageStringACP("AirP3"));
	GetDlgItem(IDC_LP4)->SetWindowText(GetLanguageStringACP("AirP4"));

	SetDlgItemText(IDOK, GetLanguageStringACP("OK"));
	SetDlgItemText(IDCANCEL, GetLanguageStringACP("Cancel"));
}
