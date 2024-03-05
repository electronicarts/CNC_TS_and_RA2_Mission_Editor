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

// TaskForce.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "TaskForce.h"
#include "FinalSunDlg.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CTaskForce 

IMPLEMENT_DYNCREATE(CTaskForce, CDialog)

CTaskForce::CTaskForce() : CDialog(CTaskForce::IDD)
{
	//{{AFX_DATA_INIT(CTaskForce)
	m_Group = _T("");
	m_Name = _T("");
	m_NumberOfUnits = 0;
	//}}AFX_DATA_INIT
}

CTaskForce::~CTaskForce()
{
}

void CTaskForce::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTaskForce)
	DDX_Control(pDX, IDC_UNITTYPE, m_UnitType);
	DDX_Control(pDX, IDC_UNITS, m_Units);
	DDX_Control(pDX, IDC_TASKFORCES, m_TaskForces);
	DDX_Text(pDX, IDC_GROUP, m_Group);
	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Text(pDX, IDC_NUMBERUNITS, m_NumberOfUnits);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTaskForce, CDialog)
	//{{AFX_MSG_MAP(CTaskForce)
	ON_CBN_EDITCHANGE(IDC_TASKFORCES, OnEditchangeTaskforces)
	ON_CBN_SELCHANGE(IDC_TASKFORCES, OnSelchangeTaskforces)
	ON_LBN_SELCHANGE(IDC_UNITS, OnSelchangeUnits)
	ON_BN_CLICKED(IDC_DELETEUNIT, OnDeleteunit)
	ON_EN_CHANGE(IDC_NUMBERUNITS, OnChangeNumberunits)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	ON_CBN_EDITCHANGE(IDC_UNITTYPE, OnEditchangeUnittype)
	ON_CBN_SELCHANGE(IDC_UNITTYPE, OnSelchangeUnittype)
	ON_BN_CLICKED(IDC_ADDUNIT, OnAddunit)
	ON_BN_CLICKED(IDC_DELETETASKFORCE, OnDeletetaskforce)
	ON_BN_CLICKED(IDC_ADDTASKFORCE, OnAddtaskforce)
	ON_EN_CHANGE(IDC_GROUP, OnChangeGroup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTaskForce 

void CTaskForce::UpdateDialog()
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_TaskForces.GetCurSel();
	int sel2=m_Units.GetCurSel();

	while(m_TaskForces.DeleteString(0)!=CB_ERR);
	while(m_Units.DeleteString(0)!=LB_ERR);
	while(m_UnitType.DeleteString(0)!=CB_ERR);

	// MW 07/24/01: Clear
	m_Group="";
	m_Name="";
	m_NumberOfUnits=0;
	UpdateData(FALSE);

	int i;
	CIniFileSection& sec=ini.sections["TaskForces"];
	for(i=0;i<sec.values.size();i++)
	{
		CString s;
		s=*sec.GetValue(i);
		s+=" (";
		s+=ini.sections[*sec.GetValue(i)].values["Name"];
		s+=")";
		m_TaskForces.AddString(s);
	}

	CString ss="InfantryTypes";
	for(i=0;i<rules.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*rules.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		/*if(ini.sections.find((char*)(LPCTSTR)type)!=ini.sections.end() && ini.sections[(char*)(LPCTSTR)type].values.find("Name")!=ini.sections[(char*)(LPCTSTR)type].values.end())
			s+=ini.sections[(char*)(LPCTSTR)type].values["Name"];
		else
			s+=rules.sections[(char*)(LPCTSTR)type].values["Name"];
		*/
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}
	for(i=0;i<ini.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*ini.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		/*if(ini.sections.find((char*)(LPCTSTR)type)!=ini.sections.end() && ini.sections[(char*)(LPCTSTR)type].values.find("Name")!=ini.sections[(char*)(LPCTSTR)type].values.end())
			s+=ini.sections[(char*)(LPCTSTR)type].values["Name"];
		else
			s+=rules.sections[(char*)(LPCTSTR)type].values["Name"];
*/
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}

	ss="VehicleTypes";
	for(i=0;i<rules.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*rules.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		/*if(ini.sections.find((char*)(LPCTSTR)type)!=ini.sections.end() && ini.sections[(char*)(LPCTSTR)type].values.find("Name")!=ini.sections[(char*)(LPCTSTR)type].values.end())
			s+=ini.sections[(char*)(LPCTSTR)type].values["Name"];
		else
			s+=rules.sections[(char*)(LPCTSTR)type].values["Name"];
*/
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}
	for(i=0;i<ini.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*ini.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		/*if(ini.sections.find((char*)(LPCTSTR)type)!=ini.sections.end() && ini.sections[(char*)(LPCTSTR)type].values.find("Name")!=ini.sections[(char*)(LPCTSTR)type].values.end())
			s+=ini.sections[(char*)(LPCTSTR)type].values["Name"];
		else
			s+=rules.sections[(char*)(LPCTSTR)type].values["Name"];
*/
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}

	ss="AircraftTypes";
	for(i=0;i<rules.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*rules.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		/*if(ini.sections.find((char*)(LPCTSTR)type)!=ini.sections.end() && ini.sections[(char*)(LPCTSTR)type].values.find("Name")!=ini.sections[(char*)(LPCTSTR)type].values.end())
			s+=ini.sections[(char*)(LPCTSTR)type].values["Name"];
		else
			s+=rules.sections[(char*)(LPCTSTR)type].values["Name"];
*/
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}
	for(i=0;i<ini.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*ini.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		/*if(ini.sections.find((char*)(LPCTSTR)type)!=ini.sections.end() && ini.sections[(char*)(LPCTSTR)type].values.find("Name")!=ini.sections[(char*)(LPCTSTR)type].values.end())
			s+=ini.sections[(char*)(LPCTSTR)type].values["Name"];
		else
			s+=rules.sections[(char*)(LPCTSTR)type].values["Name"];
*/
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}


	if(sel<0)
	{
		if(m_TaskForces.SetCurSel(0)!=CB_ERR)
			OnSelchangeTaskforces();
	}
	else
	{
		if(m_TaskForces.SetCurSel(sel)!=CB_ERR)
			OnSelchangeTaskforces();
	}

	

	

	if(!(sel2<0))
	{
		if(m_Units.SetCurSel(sel2)!=LB_ERR)
		OnSelchangeUnits();
	}

			
}

void CTaskForce::OnEditchangeTaskforces() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
}

void CTaskForce::OnSelchangeTaskforces() 
{
	CIniFile& ini=Map->GetIniFile();

	CString tf;
	tf=GetText(&m_TaskForces);

	TruncSpace(tf);
	CIniFileSection & sec=ini.sections[(char*)(LPCTSTR)tf];
	m_Name=sec.values["Name"];
	m_Group=sec.values["Group"];
	
	int i;
	while(m_Units.DeleteString(0)!=LB_ERR);
	for(i=0;i<sec.values.size()-2;i++)
	{
		char p[50];
		itoa(i, p, 10);
		CString data=sec.values[p];
		CString type=GetParam(data, 1);
		CString s=GetParam(data, 0);
		s+=" ";
		/*if(ini.sections.find((char*)(LPCTSTR)type)!=ini.sections.end() && ini.sections[(char*)(LPCTSTR)type].values.find("Name")!=ini.sections[(char*)(LPCTSTR)type].values.end())
			s+=ini.sections[(char*)(LPCTSTR)type].values["Name"];
		else
			s+=rules.sections[(char*)(LPCTSTR)type].values["Name"];*/
		s+=Map->GetUnitName(type);
		//s+=")";
		
		m_Units.SetItemData(m_Units.AddString(s), i);
		

	}
	UpdateData(FALSE);

	if(m_Units.SetCurSel(0)!=LB_ERR) OnSelchangeUnits();

	
}

void CTaskForce::OnSelchangeUnits() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Units.GetCurSel();
	int u=m_Units.GetItemData(sel);
	CString tf;
	tf=GetText(&m_TaskForces);
	TruncSpace(tf);
	CIniFileSection & sec=ini.sections[(char*)(LPCTSTR)tf];
	
	char k[50];
	itoa(u, k, 10);
	CString data=sec.values[k];
	CString c=GetParam(data, 0);
	
	CString s;
	CString type=GetParam(data,1);
	/*if(ini.sections.find((char*)(LPCTSTR)type)!=ini.sections.end() && ini.sections[(char*)(LPCTSTR)type].values.find("Name")!=ini.sections[(char*)(LPCTSTR)type].values.end())
		s=ini.sections[(char*)(LPCTSTR)type].values["Name"];
	else
		s=rules.sections[(char*)(LPCTSTR)type].values["Name"];*/
	s=Map->GetUnitName(type);

	m_UnitType.SetWindowText(((CString)(LPCTSTR)type + (CString)" (" + s + (CString)")"));
	m_NumberOfUnits=atoi(c);

	UpdateData(FALSE);
}

void CTaskForce::OnDeleteunit() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Units.GetCurSel();
	if(sel<0) return;
	int u=m_Units.GetItemData(sel);
	CString tf;
	tf=GetText(&m_TaskForces);

	TruncSpace(tf);
	CIniFileSection & sec=ini.sections[(char*)(LPCTSTR)tf];
	
	char k[50];
	itoa(u, k, 10);
	if(sec.values.size()<4) {
		sec.values.erase(k);
		m_Units.DeleteString(sel);
		m_UnitType.SetWindowText("");
		m_NumberOfUnits=atoi("0");
		UpdateDialog();
		
		return;
	}

	int lastpos=sec.values.size()-3;
	char l[50];
	itoa(lastpos, l, 10);
	sec.values[k]=sec.values[l];
	sec.values.erase(l);
	m_Units.DeleteString(sel);
	

	UpdateDialog();
	return;
}

void CTaskForce::OnChangeNumberunits() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData();

	int sel=m_Units.GetCurSel();
	if(sel<0) return;
	int u=m_Units.GetItemData(sel);
	CString tf;
	tf=GetText(&m_TaskForces);

	TruncSpace(tf);
	CIniFileSection & sec=ini.sections[(char*)(LPCTSTR)tf];
	
	char k[50], n[50];;
	itoa(u, k, 10);
	itoa(m_NumberOfUnits, n, 10);
	CString data=sec.values[k];
	CString c=GetParam(data, 1);
	sec.values[k]=n+(CString)","+c;
	UpdateDialog();
}

void CTaskForce::OnChangeName() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData();

	CEdit& n=*(CEdit*)GetDlgItem(IDC_NAME);
	DWORD pos=n.GetSel();

	if(m_TaskForces.GetCurSel()<0) return;
	CString tf;
	tf=GetText(&m_TaskForces);
	
	TruncSpace(tf);
	CIniFileSection & sec=ini.sections[(char*)(LPCTSTR)tf];
		
	sec.values["Name"]=m_Name;

	UpdateDialog();
	n.SetSel(pos);
}

void CTaskForce::OnEditchangeUnittype() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Units.GetCurSel();
	if(sel<0) return;
	int u=m_Units.GetItemData(sel);
	CString tf;
	tf=GetText(&m_TaskForces);

	TruncSpace(tf);
	CIniFileSection & sec=ini.sections[(char*)(LPCTSTR)tf];
	
	char k[50];
	itoa(u, k, 10);
	
	CString count=GetParam(sec.values[k],0);
	CString type=GetText(&m_UnitType);
	TruncSpace(type);
	
	sec.values[k]=count+","+(char*)(LPCTSTR)type;

	CString ut;
	m_UnitType.GetWindowText(ut);
	UpdateDialog();
	m_UnitType.SetWindowText(ut);

	
}

void CTaskForce::OnSelchangeUnittype() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Units.GetCurSel();
	if(sel<0) return;
	int u=m_Units.GetItemData(sel);
	CString tf;
	tf=GetText(&m_TaskForces);

	TruncSpace(tf);
	CIniFileSection & sec=ini.sections[(char*)(LPCTSTR)tf];
	
	char k[50];
	itoa(u, k, 10);
	
	CString count=GetParam(sec.values[k],0);
	CString type=GetText(&m_UnitType);

	TruncSpace(type);
	
	sec.values[k]=count+","+(char*)(LPCTSTR)type;

	UpdateDialog();	
	//m_UnitType.SetWindowText("H");
}

void CTaskForce::OnAddunit() 
{
	CIniFile& ini=Map->GetIniFile();

	CString tf;
	if(m_TaskForces.GetCurSel()<0) return;
	tf=GetText(&m_TaskForces);

	TruncSpace(tf);
	CIniFileSection & sec=ini.sections[(char*)(LPCTSTR)tf];
	
	char k[50];
	int c=m_Units.GetCount();
	if(c==LB_ERR) c=0;
	itoa(c, k, 10);
	
	sec.values[k]="1"+(CString)","+*rules.sections["InfantryTypes"].GetValue(0);
	
	UpdateDialog();
}

void CTaskForce::OnDeletetaskforce() 
{
	CIniFile& ini=Map->GetIniFile();

	CString tf;
	if(m_TaskForces.GetCurSel()<0) return;
	tf=GetText(&m_TaskForces);

	TruncSpace(tf);
	CIniFileSection & sec=ini.sections[(char*)(LPCTSTR)tf];
	
	int res;
	res=MessageBox("Are you sure to delete the selected task force? If you delete it, make sure to eliminate ANY references to this task force in team-types.","Delete task force",MB_YESNO);
	if(res==IDNO) return;

	int v=ini.sections["TaskForces"].FindValue((char*)(LPCTSTR)tf);
	if(v==-1) return; // SHOULD NEVER BE!!!! AND CANNOT BE!!! BUT MAYBE A BUG...

	ini.sections["TaskForces"].values.erase(*ini.sections["TaskForces"].GetValueName(v));
	ini.sections.erase((char*)(LPCTSTR)tf);
	while(m_Units.DeleteString(0)!=LB_ERR);
	//UpdateDialog();
	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);
}

CString GetFree(const char* section);
void CTaskForce::OnAddtaskforce() 
{
	CIniFile& ini=Map->GetIniFile();

	CString ID=GetFreeID();
	CString tf=GetFree("TaskForces");
	ini.sections["TaskForces"].values[tf]=ID;

	ini.sections[ID].values["Name"]="New task force";
	ini.sections[ID].values["Group"]="-1";

	//UpdateDialog();

	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);

	int i;
	for(i=0;i<m_TaskForces.GetCount();i++)
	{
		CString tf2;
		m_TaskForces.GetLBText(i,tf2);

		TruncSpace(tf2);

		if(strcmp(ID, tf2)==NULL)
		{
			m_TaskForces.SetCurSel(i);
			OnSelchangeTaskforces(); // MW bugfix
			break;
		}
	}
		

}

void CTaskForce::OnChangeGroup() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData();

	CString tf;
	if(m_TaskForces.GetCurSel()<0) return;
	tf=GetText(&m_TaskForces);

	TruncSpace(tf);
	CIniFileSection & sec=ini.sections[(char*)(LPCTSTR)tf];
	
	sec.values["Group"]=m_Group;

	UpdateDialog();

}
