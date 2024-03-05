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

// Unit.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "Unit.h"
#include "resource.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CUnit 


CUnit::CUnit(CWnd* pParent /*=NULL*/)
	: CDialog(CUnit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUnit)
	m_direction = _T("");
	m_flag1 = _T("");
	m_flag2 = _T("");
	m_house = _T("");
	m_flag3 = _T("");
	m_flag4 = _T("");
	m_flag5 = _T("");
	m_flag6 = _T("");
	m_action = _T("");
	m_tag = _T("");
	//}}AFX_DATA_INIT
}


void CUnit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnit)
	DDX_Control(pDX, IDC_STRENGTH, m_strength_ctrl);
	DDX_CBString(pDX, IDC_DIRECTION, m_direction);
	DDX_Text(pDX, IDC_P1, m_flag1);
	DDX_Text(pDX, IDC_P2, m_flag2);
	DDX_CBString(pDX, IDC_HOUSE, m_house);
	DDX_Text(pDX, IDC_P3, m_flag3);
	DDX_Text(pDX, IDC_P4, m_flag4);
	DDX_Text(pDX, IDC_P5, m_flag5);
	DDX_Text(pDX, IDC_P6, m_flag6);
	DDX_CBString(pDX, IDC_STATE, m_action);
	DDX_CBString(pDX, IDC_TAG, m_tag);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUnit, CDialog)
	//{{AFX_MSG_MAP(CUnit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CUnit 

void CUnit::Init(CString house, CString strength, CString direction, CString action, CString tag, CString flag1, CString flag2, CString flag3, CString flag4, CString flag5, CString flag6)
{
	CIniFile& ini=Map->GetIniFile();

	if(house=="") 
	{
		m_house=*rules.sections["Houses"].GetValue(0);
		if(ini.sections.find("Houses")!=ini.sections.end())
			if(ini.sections["Houses"].values.size()>0)
				m_house=TranslateHouse(*ini.sections["Houses"].GetValue(0), TRUE);
	}
	else
		m_house=TranslateHouse(house, TRUE);

	m_flag1=flag1;
	m_flag2=flag2;
	m_flag3=flag3;
	m_flag4=flag4;
	m_flag5=flag5;
	m_flag6=flag6;
	m_action=action;
	m_strength=strength;
	m_tag=tag;
	m_direction=direction;
}

BOOL CUnit::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// init the common (!) dialog things
	int i;
	CComboBox* house, *tag;
	house=(CComboBox*)GetDlgItem(IDC_HOUSE);
	tag=(CComboBox*)GetDlgItem(IDC_TAG);

	ListHouses(*house, FALSE);
	ListTags(*tag,TRUE);

	
	
	UpdateData(FALSE);
	m_strength_ctrl.SetRange(0,256);
	m_strength_ctrl.SetPos(atoi(m_strength));
	
	UpdateStrings();

	return TRUE;  
}

void CUnit::OnOK() 
{
	CDialog::OnOK();
	m_strength=GetText(&m_strength_ctrl);
	UpdateData();
	TruncSpace(m_tag);	
	m_house=TranslateHouse(m_house);
	

	
}

void CUnit::UpdateStrings()
{


	SetWindowText(GetLanguageStringACP("UnitCap"));
	SetDlgItemText(IDC_DESC, GetLanguageStringACP("UnitDesc"));
	SetDlgItemText(IDC_LHOUSE, GetLanguageStringACP("UnitHouse"));
	SetDlgItemText(IDC_LSTRENGTH, GetLanguageStringACP("UnitStrength"));
	SetDlgItemText(IDC_LSTATE, GetLanguageStringACP("UnitState"));
	SetDlgItemText(IDC_LDIRECTION, GetLanguageStringACP("UnitDirection"));
	SetDlgItemText(IDC_LTAG, GetLanguageStringACP("UnitTag"));
	SetDlgItemText(IDC_LP1, GetLanguageStringACP("UnitP1"));
	SetDlgItemText(IDC_LP2, GetLanguageStringACP("UnitP2"));
	SetDlgItemText(IDC_LP3, GetLanguageStringACP("UnitP3"));
	SetDlgItemText(IDC_LP4, GetLanguageStringACP("UnitP4"));
	SetDlgItemText(IDC_LP5, GetLanguageStringACP("UnitP5"));
	SetDlgItemText(IDC_LP6, GetLanguageStringACP("UnitP6"));

	SetDlgItemText(IDOK, GetLanguageStringACP("OK"));
	SetDlgItemText(IDCANCEL, GetLanguageStringACP("Cancel"));
}
