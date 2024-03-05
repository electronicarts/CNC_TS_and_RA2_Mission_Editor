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

// AiTriggerTypesEnable.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "AiTriggerTypesEnable.h"
#include "mapdata.h"
#include "variables.h"
#include "inlines.h"
#include "aitriggeradddlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CAiTriggerTypesEnable 

IMPLEMENT_DYNCREATE(CAiTriggerTypesEnable, CDialog)

CAiTriggerTypesEnable::CAiTriggerTypesEnable() : CDialog(CAiTriggerTypesEnable::IDD)
{
	//{{AFX_DATA_INIT(CAiTriggerTypesEnable)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

CAiTriggerTypesEnable::~CAiTriggerTypesEnable()
{
}

void CAiTriggerTypesEnable::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAiTriggerTypesEnable)
	DDX_Control(pDX, IDC_AITRIGGERTYPE, m_AITriggerType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAiTriggerTypesEnable, CDialog)
	//{{AFX_MSG_MAP(CAiTriggerTypesEnable)
	ON_BN_CLICKED(IDC_ENABLEALL, OnEnableall)
	ON_CBN_SELCHANGE(IDC_AITRIGGERTYPE, OnSelchangeAitriggertype)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CAiTriggerTypesEnable 

void CAiTriggerTypesEnable::UpdateDialog()
{
	int sel=m_AITriggerType.GetCurSel();
	if(sel<0) sel=0;

	while(m_AITriggerType.DeleteString(0)!=CB_ERR);

	CIniFile& ini=Map->GetIniFile();

	int i;
	for(i=0;i<ini.sections["AITriggerTypesEnable"].values.size();i++)
	{
		CString aitrigger=*ini.sections["AITriggerTypesEnable"].GetValueName(i);
		CString str=aitrigger;
		str+=" (";

		if(ai.sections["AITriggerTypes"].values.find(aitrigger)!=ai.sections["AITriggerTypes"].values.end())
		{
			// standard ai trigger
			str+=GetParam(ai.sections["AITriggerTypes"].values[aitrigger],0);
			str+=" -> ";
			str+=*ini.sections["AITriggerTypesEnable"].GetValue(i);


		}
		if(ini.sections["AITriggerTypes"].values.find(aitrigger)!=ini.sections["AITriggerTypes"].values.end())
		{
			str+=GetParam(ini.sections["AITriggerTypes"].values[aitrigger],0);
			str+=" -> ";
			str+=*ini.sections["AITriggerTypesEnable"].GetValue(i);
		}

		str+=")";

		m_AITriggerType.AddString(str);
	}

	if(m_AITriggerType.SetCurSel(sel)==CB_ERR)
		m_AITriggerType.SetCurSel(0);

	OnSelchangeAitriggertype();
	
}

void CAiTriggerTypesEnable::OnEnableall() 
{
	// enable all standard ai triggers
	CIniFile& ini=Map->GetIniFile();
	int i;
	for(i=0;i<ai.sections["AITriggerTypes"].values.size();i++)
	{
		ini.sections["AITriggerTypesEnable"].values[*ai.sections["AITriggerTypes"].GetValueName(i)]="yes";		
	}

	UpdateDialog();
}

void CAiTriggerTypesEnable::OnSelchangeAitriggertype() 
{
	int sel=m_AITriggerType.GetCurSel();
	if(sel<0) return;
		
}

void CAiTriggerTypesEnable::OnDelete() 
{
	int sel=m_AITriggerType.GetCurSel();
	if(sel<0) return;
	CString aitrigger;
	m_AITriggerType.GetLBText(sel,aitrigger);
	if(aitrigger.Find(" ")>=0) aitrigger.SetAt(aitrigger.Find(" "), 0);
	
	CIniFile& ini=Map->GetIniFile();

	ini.sections["AITriggerTypesEnable"].values.erase((LPCTSTR)aitrigger);
	UpdateDialog();
}

void CAiTriggerTypesEnable::OnAdd() 
{
	

	//CString p=InputBox("Please enter the ID of the AITriggerType (for a list of all AITriggerType-IDs use the All-Section)","Enable AITriggerType");
	CAITriggerAddDlg dlg;
	if(dlg.DoModal()==IDCANCEL) return;

	CString p=dlg.m_AITrigger;
	TruncSpace(p);
	if(p.GetLength()==0) return;

	CIniFile& ini=Map->GetIniFile();

	ini.sections["AITriggerTypesEnable"].values[p]="yes";
	UpdateDialog();
}
