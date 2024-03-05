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

// Tags.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "Tags.h"
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
// Eigenschaftenseite CTags 

IMPLEMENT_DYNCREATE(CTags, CDialog)

CTags::CTags() : CDialog(CTags::IDD)
{
	//{{AFX_DATA_INIT(CTags)
	m_Name = _T("");
	//}}AFX_DATA_INIT
}

CTags::~CTags()
{
}

void CTags::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTags)
	DDX_Control(pDX, IDC_REPEAT, m_Repeat);
	DDX_Control(pDX, IDC_TAG, m_Tag);
	DDX_Control(pDX, IDC_TRIGGER, m_Trigger);
	DDX_Text(pDX, IDC_NAME, m_Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTags, CDialog)
	//{{AFX_MSG_MAP(CTags)
	ON_CBN_SELCHANGE(IDC_TAG, OnSelchangeTag)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	ON_CBN_EDITCHANGE(IDC_REPEAT, OnEditchangeRepeat)
	ON_CBN_SELCHANGE(IDC_REPEAT, OnSelchangeRepeat)
	ON_CBN_EDITCHANGE(IDC_TRIGGER, OnEditchangeTrigger)
	ON_CBN_SELCHANGE(IDC_TRIGGER, OnSelchangeTrigger)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTags 

void CTags::UpdateDialog()
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Tag.GetCurSel();
	
	while(m_Tag.DeleteString(0)!=CB_ERR);
	while(m_Trigger.DeleteString(0)!=CB_ERR);

	int i;
	for(i=0;i<ini.sections["Tags"].values.size();i++)
	{
		CString type=*ini.sections["Tags"].GetValueName(i);
		CString s;
		s=type;
		s+=" (";
		s+=GetParam(*ini.sections["Tags"].GetValue(i), 1);
		s+=")";
		m_Tag.AddString(s);
	}

	for(i=0;i<ini.sections["Triggers"].values.size();i++)
	{
		CString type=*ini.sections["Triggers"].GetValueName(i);		
		CString s;
		s=type;
		s+=" (";
		s+=GetParam(ini.sections["Triggers"].values[type], 2);
		s+=")";
		m_Trigger.AddString(s);
	}

	if(m_Tag.SetCurSel(0)!=CB_ERR)
		OnSelchangeTag();

	if(sel!=-1)
	{
		if(m_Tag.SetCurSel(sel)!=CB_ERR)
		{
			OnSelchangeTag();
		}
	}

}

void CTags::OnSelchangeTag() 
{
	CIniFile& ini=Map->GetIniFile();

	int index=m_Tag.GetCurSel();
	if(index<0) return;
	CString type;
	m_Tag.GetLBText(index, type);
	if(type.Find(" ")>=0) type.SetAt(type.Find(" "),0);

	CString data=ini.sections["Tags"].values[(LPCTSTR)type];
	m_Name=GetParam(data,1);
	CString trigger=GetParam(data,2);
	CString typ=trigger;

	trigger+=" (";
	if(ini.sections["Triggers"].values.find(typ)!=ini.sections["Triggers"].values.end())
		trigger+=GetParam(ini.sections["Triggers"].values[typ],2);
	trigger+=")";

	m_Trigger.SetWindowText(trigger);
	
		
	m_Repeat.SetWindowText(GetParam(data,0));

	

	UpdateData(FALSE);
}

void CTags::OnChangeName() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData();

	CEdit&  name=*(CEdit*)GetDlgItem(IDC_NAME);
	int sel2=name.GetSel();

	int index=m_Tag.GetCurSel();
	if(index<0) return;
	CString type;
	m_Tag.GetLBText(index, type);
	if(type.Find(" ")>=0) type.SetAt(type.Find(" "),0);

	CString data=ini.sections["Tags"].values[(LPCTSTR)type];
	
	CString tag, repeat;
	tag=GetParam(data,2);
	repeat=GetParam(data,0);
	data=repeat+","+(LPCTSTR)m_Name+","+tag;
	ini.sections["Tags"].values[(LPCTSTR)type]=data;

	UpdateDialog();
	name.SetSel(sel2);
}

void CTags::OnEditchangeRepeat() 
{
	CIniFile& ini=Map->GetIniFile();

	int index=m_Tag.GetCurSel();
	if(index<0) return;

	CString str;
	m_Repeat.GetWindowText(str);

	CString type;
	m_Tag.GetLBText(index, type);
	if(type.Find(" ")>=0) type.SetAt(type.Find(" "),0);

	CString data=ini.sections["Tags"].values[(LPCTSTR)type];
	
	CString trigger, name;
	trigger=GetParam(data,2);
	name=GetParam(data,1);
	data=(CString)(LPCTSTR)str+","+name+","+trigger;
	ini.sections["Tags"].values[(LPCTSTR)type]=data;
	

	UpdateDialog();

}

void CTags::OnSelchangeRepeat() 
{
	CIniFile& ini=Map->GetIniFile();

	int index=m_Tag.GetCurSel();
	if(index<0) return;

	int v=m_Repeat.GetCurSel();
	CString str;

	
	m_Repeat.GetLBText(v,str);


	CString type;
	m_Tag.GetLBText(index, type);
	TruncSpace(type);

	TruncSpace(str);

	CString data=ini.sections["Tags"].values[(LPCTSTR)type];
	
	CString trigger, name;
	trigger=GetParam(data,2);
	name=GetParam(data,1);
	data=(CString)(LPCTSTR)str+","+name+","+trigger;
	ini.sections["Tags"].values[(LPCTSTR)type]=data;
	

	UpdateDialog();
}

void CTags::OnEditchangeTrigger() 
{
	CIniFile& ini=Map->GetIniFile();

	int index=m_Tag.GetCurSel();
	if(index<0) return;

	CString str;
	m_Trigger.GetWindowText(str);
	if(str.Find(" ")>=0) str.SetAt(str.Find(" "),0);

	CString type;
	m_Tag.GetLBText(index, type);
	if(type.Find(" ")>=0) type.SetAt(type.Find(" "),0);

	CString data=ini.sections["Tags"].values[(LPCTSTR)type];
	
	CString repeat, name;
	repeat=GetParam(data,0);
	name=GetParam(data,1);
	data=repeat+","+name+","+(LPCTSTR)str;
	ini.sections["Tags"].values[(LPCTSTR)type]=data;
	

	//UpdateDialog();
		
}

void CTags::OnSelchangeTrigger() 
{
	CIniFile& ini=Map->GetIniFile();

	int index=m_Tag.GetCurSel();
	if(index<0) return;

	int v=m_Trigger.GetCurSel();
	CString str;

	
	m_Trigger.GetLBText(v,str);	
	
	if(str.Find(" ")>=0) str.SetAt(str.Find(" "),0);

	CString type;
	m_Tag.GetLBText(index, type);
	if(type.Find(" ")>=0) type.SetAt(type.Find(" "),0);

	CString data=ini.sections["Tags"].values[(LPCTSTR)type];
	
	CString repeat, name;
	repeat=GetParam(data,0);
	name=GetParam(data,1);
	data=repeat+","+name+","+(LPCTSTR)str;
	ini.sections["Tags"].values[(LPCTSTR)type]=data;
	

	//UpdateDialog();
}

void CTags::OnDelete() 
{
	CIniFile& ini=Map->GetIniFile();

	int index=m_Tag.GetCurSel();
	if(index<0) return;

	CString type;
	m_Tag.GetLBText(index, type);
	if(type.Find(" ")>=0) type.SetAt(type.Find(" "),0);

	int res=MessageBox("Are you sure to delete the selected tag? This may cause the attached trigger to don´t work anymore, if no other tag has the trigger attached.","Delete tag", MB_YESNO);
	if(res==IDNO) return;
	
	ini.sections["Tags"].values.erase((LPCTSTR)type);
	UpdateDialog();
}

void CTags::OnAdd() 
{
	CIniFile& ini=Map->GetIniFile();

	CString ID=GetFreeID();	

	if(ini.sections["Triggers"].values.size()<1)
	{
		MessageBox("Before creating tags, you need at least one trigger.","Error");
		return;
	};

	CString data;
	data="0,New Tag,";
	data+=*ini.sections["Triggers"].GetValueName(0);
	ini.sections["Tags"].values[ID]=data;

	
	UpdateDialog();

	int i;
	for(i=0;i<m_Tag.GetCount();i++)
	{
		CString j;
		m_Tag.GetLBText(i,j);
		if(j.Find(" ")>=0) j.SetAt(j.Find(" "),0);

		if(j==ID)
		{
			m_Tag.SetCurSel(i);
			break;
		}
	}
}
