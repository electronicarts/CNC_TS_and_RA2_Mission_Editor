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

// Triggers.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "Triggers.h"
#include "FinalSunDlg.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"

CString GetWaypoint(int n);
int GetWaypoint(const char* c);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CTriggers 

IMPLEMENT_DYNCREATE(CTriggers, CDialog)


CTriggers::CTriggers() : CDialog(CTriggers::IDD)
{
	//{{AFX_DATA_INIT(CTriggers)
	m_F1 = _T("");
	m_F2 = _T("");
	m_F3 = _T("");
	m_F4 = _T("");
	m_F5 = _T("");
	m_LA1 = _T("");
	m_LA2 = _T("");
	m_LA3 = _T("");
	m_LA4 = _T("");
	m_LA5 = _T("");
	m_LA6 = _T("");
	m_LE1 = _T("");
	m_LE2 = _T("");
	m_Name = _T("");
	m_LAW = _T("");
	//}}AFX_DATA_INIT
}

CTriggers::~CTriggers()
{
}

void CTriggers::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTriggers)
	DDX_Control(pDX, IDC_EVENTPARAM2, m_E2);
	DDX_Control(pDX, IDC_EVENTPARAM1, m_E1);
	DDX_Control(pDX, IDC_ACTIONPARAM6, m_A6);
	DDX_Control(pDX, IDC_ACTIONPARAM5, m_A5);
	DDX_Control(pDX, IDC_ACTIONPARAM4, m_A4);
	DDX_Control(pDX, IDC_ACTIONPARAM3, m_A3);
	DDX_Control(pDX, IDC_ACTIONPARAM2, m_A2);
	DDX_Control(pDX, IDC_ACTIONPARAM1, m_A1);
	DDX_Control(pDX, IDC_HOUSE, m_House);
	DDX_Control(pDX, IDC_TRIGGER2, m_Trigger2);
	DDX_Control(pDX, IDC_TRIGGER, m_Trigger);
	DDX_Control(pDX, IDC_EVENTTYPE, m_EventType);
	DDX_Control(pDX, IDC_EVENT, m_Event);
	DDX_Control(pDX, IDC_ACTIONWAYPOINT, m_ActionWaypoint);
	DDX_Control(pDX, IDC_ACTIONTYPE, m_ActionType);
	DDX_Control(pDX, IDC_ACTION, m_Action);
	DDX_Text(pDX, IDC_FLAG1, m_F1);
	DDX_Text(pDX, IDC_FLAG2, m_F2);
	DDX_Text(pDX, IDC_FLAG3, m_F3);
	DDX_Text(pDX, IDC_FLAG4, m_F4);
	DDX_Text(pDX, IDC_FLAG5, m_F5);
	DDX_Text(pDX, IDC_LABEL_A1, m_LA1);
	DDX_Text(pDX, IDC_LABEL_A2, m_LA2);
	DDX_Text(pDX, IDC_LABEL_A3, m_LA3);
	DDX_Text(pDX, IDC_LABEL_A4, m_LA4);
	DDX_Text(pDX, IDC_LABEL_A5, m_LA5);
	DDX_Text(pDX, IDC_LABEL_A6, m_LA6);
	DDX_Text(pDX, IDC_LABEL_E1, m_LE1);
	DDX_Text(pDX, IDC_LABEL_E2, m_LE2);
	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Text(pDX, IDC_WAYPOINT, m_LAW);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTriggers, CDialog)
	//{{AFX_MSG_MAP(CTriggers)
	ON_CBN_SELCHANGE(IDC_TRIGGER, OnSelchangeTrigger)
	ON_CBN_SELCHANGE(IDC_EVENT, OnSelchangeEvent)
	ON_CBN_SELCHANGE(IDC_ACTION, OnSelchangeAction)
	ON_CBN_EDITCHANGE(IDC_HOUSE, OnEditchangeHouse)
	ON_CBN_SELCHANGE(IDC_HOUSE, OnSelchangeHouse)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	ON_EN_CHANGE(IDC_FLAG1, OnChangeFlag1)
	ON_EN_CHANGE(IDC_FLAG2, OnChangeFlag2)
	ON_EN_CHANGE(IDC_FLAG3, OnChangeFlag3)
	ON_EN_CHANGE(IDC_FLAG4, OnChangeFlag4)
	ON_EN_CHANGE(IDC_FLAG5, OnChangeFlag5)
	ON_CBN_EDITCHANGE(IDC_TRIGGER2, OnEditchangeTrigger2)
	ON_CBN_SELCHANGE(IDC_TRIGGER2, OnSelchangeTrigger2)
	ON_CBN_EDITCHANGE(IDC_EVENTTYPE, OnEditchangeEventtype)
	ON_CBN_SELCHANGE(IDC_EVENTTYPE, OnSelchangeEventtype)
	ON_CBN_EDITCHANGE(IDC_EVENTPARAM1, OnEditchangeEventparam1)
	ON_CBN_SELCHANGE(IDC_EVENTPARAM1, OnSelchangeEventparam1)
	ON_CBN_EDITCHANGE(IDC_EVENTPARAM2, OnEditchangeEventparam2)
	ON_CBN_SELCHANGE(IDC_EVENTPARAM2, OnSelchangeEventparam2)
	ON_CBN_EDITCHANGE(IDC_ACTIONTYPE, OnEditchangeActiontype)
	ON_CBN_SELCHANGE(IDC_ACTIONTYPE, OnSelchangeActiontype)
	ON_CBN_EDITCHANGE(IDC_ACTIONWAYPOINT, OnEditchangeActionwaypoint)
	ON_CBN_SELCHANGE(IDC_ACTIONWAYPOINT, OnSelchangeActionwaypoint)
	ON_CBN_EDITCHANGE(IDC_ACTIONPARAM1, OnEditchangeActionparam1)
	ON_CBN_SELCHANGE(IDC_ACTIONPARAM1, OnSelchangeActionparam1)
	ON_CBN_EDITCHANGE(IDC_ACTIONPARAM2, OnEditchangeActionparam2)
	ON_CBN_SELCHANGE(IDC_ACTIONPARAM2, OnSelchangeActionparam2)
	ON_CBN_EDITCHANGE(IDC_ACTIONPARAM3, OnEditchangeActionparam3)
	ON_CBN_SELCHANGE(IDC_ACTIONPARAM3, OnSelchangeActionparam3)
	ON_CBN_EDITCHANGE(IDC_ACTIONPARAM4, OnEditchangeActionparam4)
	ON_CBN_SELCHANGE(IDC_ACTIONPARAM4, OnSelchangeActionparam4)
	ON_CBN_EDITCHANGE(IDC_ACTIONPARAM5, OnEditchangeActionparam5)
	ON_CBN_SELCHANGE(IDC_ACTIONPARAM5, OnSelchangeActionparam5)
	ON_CBN_EDITCHANGE(IDC_ACTIONPARAM6, OnEditchangeActionparam6)
	ON_CBN_SELCHANGE(IDC_ACTIONPARAM6, OnSelchangeActionparam6)
	ON_BN_CLICKED(IDC_ADDEVENT, OnAddevent)
	ON_BN_CLICKED(IDC_DELETEEVENT, OnDeleteevent)
	ON_BN_CLICKED(IDC_ADDACTION, OnAddaction)
	ON_BN_CLICKED(IDC_DELETEACTION, OnDeleteaction)
	ON_BN_CLICKED(IDC_DELETETRIGGER, OnDeletetrigger)
	ON_BN_CLICKED(IDC_ADDTRIGGER, OnAddtrigger)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTriggers 


void CTriggers::UpdateDialog()
{
	CIniFile& ini=Map->GetIniFile();
	

	int sel=m_Trigger.GetCurSel();
	int selat=m_Action.GetCurSel();
	int selev=m_Event.GetCurSel();

	while(m_Trigger.DeleteString(0)!=CB_ERR);
	while(m_Trigger2.DeleteString(0)!=CB_ERR);
	while(m_Action.DeleteString(0)!=CB_ERR);
	while(m_Event.DeleteString(0)!=CB_ERR);
	while(m_ActionWaypoint.DeleteString(0)!=CB_ERR);
	while(m_House.DeleteString(0)!=CB_ERR);
	while(m_ActionType.DeleteString(0)!=CB_ERR);
	while(m_EventType.DeleteString(0)!=CB_ERR);
	
	m_LE1="Parameter 1";
	m_LE2="Parameter 2";
	m_LA1="Param1";
	m_LA2="Param2";
	m_LA3="Param3";
	m_LA4="Param4";
	m_LA5="Param5";
	m_LA6="Param6";

	int i;

	m_Trigger2.AddString("<none>");
	for(i=0;i<ini.sections["Triggers"].values.size();i++)
	{
		CString type;
		CString s;
		type=*ini.sections["Triggers"].GetValueName(i);
		
		s=type;
		s+=" (";
		s+=GetParam(ini.sections["Triggers"].values[type], 2);
		s+=")";

		m_Trigger.AddString(s);
		m_Trigger2.AddString(s);

	}
	
	for(i=0;i<g_data.sections["Events"].values.size();i++)
	{
		CString eventid=*g_data.sections["Events"].GetValueName(i);
		CString eventdata=*g_data.sections["Events"].GetValue(i);
		CString text=eventid+" "+GetParam(eventdata,0);
		m_EventType.AddString(text);
	}

	for(i=0;i<g_data.sections["Actions"].values.size();i++)
	{
		CString actionid=*g_data.sections["Actions"].GetValueName(i);
		CString actiondata=*g_data.sections["Actions"].GetValue(i);
		CString text=actionid+" "+GetParam(actiondata,0);
		m_ActionType.AddString(text);
	}
	
	
	ListHouses(m_House, FALSE);

	CComboBox* wayp;
	wayp=(CComboBox*)GetDlgItem(IDC_ACTIONWAYPOINT);
	while(wayp->DeleteString(0)!=CB_ERR);
	if(ini.sections.find("Waypoints")!=ini.sections.end())
	{
		for(i=0;i<ini.sections["Waypoints"].values.size();i++)
		{
			wayp->AddString(*ini.sections["Waypoints"].GetValueName(i));		
		}
	}


	if(sel==-1 || m_Trigger.SetCurSel(sel)==FALSE)
	{
		m_Trigger.SetCurSel(0);
	}
	if(selat==-1 || m_Trigger.SetCurSel(selat)==FALSE)
	{
		m_Action.SetCurSel(0);
	}
	if(selev==-1 || m_Trigger.SetCurSel(selev)==FALSE)
	{
		m_Event.SetCurSel(0);
	}

	OnSelchangeTrigger();
	
}

void CTriggers::OnSelchangeTrigger() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;

	while(m_Action.DeleteString(0)!=CB_ERR);
	while(m_Event.DeleteString(0)!=CB_ERR);

	CString TriggerData, EventData, ActionData;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);
		
	TriggerData=ini.sections["Triggers"].values[CurrentTrigger];
	EventData=ini.sections["Events"].values[CurrentTrigger];
	ActionData=ini.sections["Actions"].values[CurrentTrigger];

	m_Name=GetParam(TriggerData, 2);
	m_House.SetWindowText(TranslateHouse(GetParam(TriggerData,0), TRUE));
	

	CString trig2=GetParam(TriggerData,1);
	if(ini.sections["Triggers"].values.find(trig2)!=ini.sections["Triggers"].values.end())
	{
		trig2+=" (";
		trig2+=GetParam( ini.sections["Triggers"].values[GetParam(TriggerData,1)],2);
		trig2+=")";
	}
	m_Trigger2.SetWindowText(trig2);

	m_F1=GetParam(TriggerData,3);
	m_F2=GetParam(TriggerData,4);
	m_F3=GetParam(TriggerData,5);
	m_F4=GetParam(TriggerData,6);
	m_F5=GetParam(TriggerData,7);


	// okay, now list all events and actions
	// -------------------------------------
	int i;

	// events
	for(i=0;i<atoi(GetParam(EventData, 0));i++)
	{
		char c[10];
		itoa(i,c,10);
		m_Event.AddString(c);
	}
	if(m_Event.SetCurSel(0)!=CB_ERR)
		OnSelchangeEvent();



	// actions
	for(i=0;i<atoi(GetParam(ActionData, 0));i++)
	{
		char c[10];
		itoa(i,c,10);
		m_Action.AddString(c);
	}
	if(m_Action.SetCurSel(0)!=CB_ERR)
		OnSelchangeAction();

	UpdateData(FALSE);
}

void CTriggers::OnSelchangeEvent() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int selev=m_Event.GetCurSel();
	if(selev<0) return;

	
	int i;
	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);


	CString EventData;
	EventData=ini.sections["Events"].values[(LPCTSTR)CurrentTrigger];
	
	int startpos=1+selev*3;
	CString EventType=GetParam(EventData,startpos);
	m_EventType.SetWindowText(EventType);
	for(i=0;i<m_EventType.GetCount();i++)
	{
		CString tmp;
		m_EventType.GetLBText(i,tmp);
		TruncSpace(tmp);
		if(tmp==EventType)
			m_EventType.SetCurSel(i);
	}
	m_E1.SetWindowText(GetParam(EventData,startpos+1));

	m_E2.SetWindowText(GetParam(EventData,startpos+2));

	OnEditchangeEventtype();
}

void CTriggers::OnSelchangeAction() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int selac=m_Action.GetCurSel();
	if(selac<0) return;	

	int i;
	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString ActionData;
	ActionData=ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger];

	int startpos=1+selac*8;
	CString ActionType=GetParam(ActionData,startpos);
	m_ActionType.SetWindowText(ActionType);
	for(i=0;i<m_ActionType.GetCount();i++)
	{
		CString tmp;
		m_ActionType.GetLBText(i,tmp);
		TruncSpace(tmp);
		if(tmp==ActionType)
			m_ActionType.SetCurSel(i);
	}
	m_A1.SetWindowText(GetParam(ActionData,startpos+1));
	m_A2.SetWindowText(GetParam(ActionData,startpos+2));
	m_A3.SetWindowText(GetParam(ActionData,startpos+3));
	m_A4.SetWindowText(GetParam(ActionData,startpos+4));
	m_A5.SetWindowText(GetParam(ActionData,startpos+5));
	m_A6.SetWindowText(GetParam(ActionData,startpos+6));
	
	OnEditchangeActiontype();

	char wayp[50];
	itoa(GetWaypoint(GetParam(ActionData,startpos+7)), wayp, 10);
	m_ActionWaypoint.SetWindowText(wayp);
}

void CTriggers::OnEditchangeHouse() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString house;
	m_House.GetWindowText(house);

	house=TranslateHouse(house);
	
	ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger], 0, (LPCTSTR)house);
}

void CTriggers::OnSelchangeHouse() 
{
	CIniFile& ini=Map->GetIniFile();

	int csel=m_House.GetCurSel();
	CString house;
	m_House.GetLBText(csel,house);
	m_House.SetWindowText(house);

	OnEditchangeHouse();
}

void CTriggers::OnChangeName() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData();
	
	CEdit& name=*(CEdit*)GetDlgItem(IDC_NAME);

	int esel=name.GetSel();
	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

		
	ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger], 2, (LPCTSTR)m_Name);

	UpdateDialog();
	
	m_Trigger.SetCurSel(sel);
	OnSelchangeTrigger();
	name.SetSel(esel);
}

void CTriggers::OnChangeFlag1() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

		
	ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger], 3, (LPCTSTR)m_F1);

}

void CTriggers::OnChangeFlag2() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

		
	ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger], 4, (LPCTSTR)m_F2);
	
}

void CTriggers::OnChangeFlag3() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

		
	ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger], 5, (LPCTSTR)m_F3);

	
}

void CTriggers::OnChangeFlag4() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

		
	ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger], 6, (LPCTSTR)m_F4);

}

void CTriggers::OnChangeFlag5() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

		
	ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger], 7, (LPCTSTR)m_F5);

}

void CTriggers::OnEditchangeTrigger2() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString trg;
	m_Trigger2.GetWindowText(trg);
	TruncSpace(trg);
	
	ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Triggers"].values[(LPCTSTR)CurrentTrigger], 1, (LPCTSTR)trg);	
}

void CTriggers::OnSelchangeTrigger2() 
{
	CIniFile& ini=Map->GetIniFile();

	int csel=m_Trigger2.GetCurSel();
	CString trg;
	m_Trigger2.GetLBText(csel,trg);
	m_Trigger2.SetWindowText(trg);
	OnEditchangeTrigger2();
}

void CTriggers::OnEditchangeEventtype() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Event.GetCurSel();
	if(sel2<0) return;

	CString e1,e2;
	m_E1.GetWindowText(e1);
	m_E2.GetWindowText(e2);
	while(m_E2.DeleteString(0)!=CB_ERR);
	while(m_E1.DeleteString(0)!=CB_ERR);
	m_E1.SetWindowText(e1);
	m_E2.SetWindowText(e2);

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString eventtype;
	m_EventType.GetWindowText(eventtype);
	TruncSpace(eventtype);

	int pos=1+3*sel2;
	
	ini.sections["Events"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Events"].values[(LPCTSTR)CurrentTrigger], pos, (LPCTSTR)eventtype);

	if(g_data.sections["Events"].FindName(eventtype)<0) return;

	
	CString ptype[2];
	ptype[0]=GetParam(g_data.sections["Events"].values[eventtype],1);
	ptype[1]=GetParam(g_data.sections["Events"].values[eventtype],2);

	int pListType[2];
	pListType[0]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[0]], 1));
	pListType[1]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[1]], 1));

	int i;
	for(i=0;i<2;i++)
	{
		CString* label;
		if(i==0) label=&m_LE1;
		if(i==1) label=&m_LE2;
		CComboBox* cb;
		if(i==0) cb=&m_E1;
		if(i==1) cb=&m_E2;

		CString old_sel;
		cb->GetWindowText(old_sel);
		TruncSpace(old_sel);

		if(atoi(ptype[i])<0)
		{
			char c[50];
			itoa(abs(atoi(ptype[i])), c, 10);
			cb->SetWindowText(c);
			*label="Static";
			continue;
		}

		*label=GetParam(g_data.sections["ParamTypes"].values[ptype[i]], 0);

		

		HandleParamList(*cb, pListType[i]);
		
		cb->SetWindowText(old_sel);

		int e;
		for(e=0;e<cb->GetCount();e++)
		{
			CString tmp;
			cb->GetLBText(e,tmp);
			TruncSpace(tmp);
			if(tmp==old_sel)
				cb->SetCurSel(e);
		}
		
	}


	

	UpdateData(FALSE);

}



void CTriggers::OnSelchangeEventtype() 
{
	int csel=m_EventType.GetCurSel();
	CString trg;
	m_EventType.GetLBText(csel,trg);
	m_EventType.SetWindowText(trg);
	OnEditchangeEventtype();
	
	
}

void CTriggers::OnEditchangeEventparam1() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Event.GetCurSel();
	if(sel2<0) return;
	

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString param1;
	m_E1.GetWindowText(param1);
	TruncSpace(param1);

	int pos=1+3*sel2+1;
	
	ini.sections["Events"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Events"].values[(LPCTSTR)CurrentTrigger], pos, (LPCTSTR)param1);
	
}

void CTriggers::OnSelchangeEventparam1() 
{
	int csel=m_E1.GetCurSel();
	CString trg;
	m_E1.GetLBText(csel,trg);
	m_E1.SetWindowText(trg);
	OnEditchangeEventparam1();
}

void CTriggers::OnEditchangeEventparam2() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Event.GetCurSel();
	if(sel2<0) return;
	

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString param2;
	m_E2.GetWindowText(param2);
	TruncSpace(param2);

	int pos=1+3*sel2+2;
	
	ini.sections["Events"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Events"].values[(LPCTSTR)CurrentTrigger], pos, (LPCTSTR)param2);
		
}

void CTriggers::OnSelchangeEventparam2() 
{
	int csel=m_E2.GetCurSel();
	CString trg;
	m_E2.GetLBText(csel,trg);
	m_E2.SetWindowText(trg);
	OnEditchangeEventparam2();
}

void CTriggers::OnEditchangeActiontype() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Action.GetCurSel();
	if(sel2<0) return;
	
	CString a;
	m_A1.GetWindowText(a);
	while(m_A1.DeleteString(0)!=CB_ERR);
	m_A1.SetWindowText(a);
	m_A2.GetWindowText(a);
	while(m_A2.DeleteString(0)!=CB_ERR);
	m_A2.SetWindowText(a);
	m_A3.GetWindowText(a);
	while(m_A3.DeleteString(0)!=CB_ERR);
	m_A3.SetWindowText(a);
	m_A4.GetWindowText(a);
	while(m_A4.DeleteString(0)!=CB_ERR);
	m_A4.SetWindowText(a);
	m_A5.GetWindowText(a);
	while(m_A5.DeleteString(0)!=CB_ERR);
	m_A5.SetWindowText(a);
	m_A6.GetWindowText(a);
	while(m_A6.DeleteString(0)!=CB_ERR);
	m_A6.SetWindowText(a);


	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString actiontype;
	m_ActionType.GetWindowText(actiontype);
	TruncSpace(actiontype);

	int pos=1+8*sel2;
	
	ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger], pos, (LPCTSTR)actiontype);

	if(g_data.sections["Actions"].FindName(actiontype)<0) return;

	
	CString ptype[6];
	ptype[0]=GetParam(g_data.sections["Actions"].values[actiontype],1);
	ptype[1]=GetParam(g_data.sections["Actions"].values[actiontype],2);
	ptype[2]=GetParam(g_data.sections["Actions"].values[actiontype],3);
	ptype[3]=GetParam(g_data.sections["Actions"].values[actiontype],4);
	ptype[4]=GetParam(g_data.sections["Actions"].values[actiontype],5);
	ptype[5]=GetParam(g_data.sections["Actions"].values[actiontype],6);

	if(GetParam(g_data.sections["Actions"].values[actiontype],7)=="0")
		m_LAW="Unused";
	else
		m_LAW="Waypoint:";

	int pListType[6];
	pListType[0]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[0]], 1));
	pListType[1]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[1]], 1));

	

	int i;
	for(i=0;i<6;i++)
	{
		CString* label;
		if(i==0) label=&m_LA1;
		if(i==1) label=&m_LA2;
		if(i==2) label=&m_LA3;
		if(i==3) label=&m_LA4;
		if(i==4) label=&m_LA5;
		if(i==5) label=&m_LA6;
		CComboBox* cb;
		if(i==0) cb=&m_A1;
		if(i==1) cb=&m_A2;
		if(i==2) cb=&m_A3;
		if(i==3) cb=&m_A4;
		if(i==4) cb=&m_A5;
		if(i==5) cb=&m_A6;

		CString old_sel;
		cb->GetWindowText(old_sel);
		TruncSpace(old_sel);

		if(atoi(ptype[i])<0)
		{
			char c[50];
			itoa(abs(atoi(ptype[i])), c, 10);
			cb->SetWindowText(c);
			*label="Static";
			continue;
		}

		*label = GetParam(g_data.sections["ParamTypes"].values[ptype[i]], 0);

		

		HandleParamList(*cb, pListType[i]);
		
		cb->SetWindowText(old_sel);

		int e;
		for(e=0;e<cb->GetCount();e++)
		{
			CString tmp;
			cb->GetLBText(e,tmp);
			TruncSpace(tmp);
			if(tmp==old_sel)
				cb->SetCurSel(e);
		}
		
	}
/*

	switch(atoi(actiontype))
	{
	case 0:
	case 23:
	case 24:
	case 46:
	case 47:
		m_LA1="Unused:";
		m_LA2="Unused:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		break;
	case 1:
	case 2:
	case 3:
	case 6:
	case 36:
		case 74:
		m_LA1="Unused:";
		m_LA2="House:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		ListHouses(m_A2);
		break;

	case 4:
		m_LA1="Unknown =1:";
		m_LA2="TeamType:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		m_A1.SetWindowText("1");
		OnEditchangeActionparam1();
		ListTeamTypes(m_A2);
		break;
	case 5:
		m_LA1="Unused?:";
		m_LA2="TeamType:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		ListTeamTypes(m_A2);
		break;

	case 7:
		m_LA1="Unknown =1:";
		m_LA2="TeamType:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		m_A1.SetWindowText("1");
		OnEditchangeActionparam1();
		ListTeamTypes(m_A2);
		break;
	case 8:
	case 17:
	case 18:
		m_LA1="Unused:";
		m_LA2="Waypoint:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		ListWaypoints(m_A2);
		break;

	case 27:
		m_LA1="Unused:";
		m_LA2="Duration:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		break;
	case 43:
		m_LA1="Unused:";
		m_LA2="Unknown:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		m_A2.SetWindowText("7");
		OnEditchangeActionparam2();
		break;
	case 44:
		m_LA1="Unused:";
		m_LA2="Duration:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		break;
	case 48:
		m_LA1="Unused:";
		m_LA2="Speed:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		break;
	case 55:
		m_LA1="Unused:";
		m_LA2="Unknown:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		m_A2.SetWindowText("1");
		OnEditchangeActionparam2();
		break;
	case 56:
		m_LA1="Unused:";
		m_LA2="Global:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		break;
	case 58:
		m_LA1="Unused:";
		m_LA2="Size:";
		m_LA3="Unused:";
		m_LA4="Unused:";
		m_LA5="Unused:";
		m_LA6="Unused:";
		break;
	default:
		m_LA1="Param1:";
		m_LA2="Param2:";
		m_LA3="Param3:";
		m_LA4="Param4:";
		m_LA5="Param5:";
		m_LA6="Param6:";
	}
*/

	UpdateData(FALSE);

}

void CTriggers::OnSelchangeActiontype() 
{
	int csel=m_ActionType.GetCurSel();
	CString trg;
	m_ActionType.GetLBText(csel,trg);
	m_ActionType.SetWindowText(trg);
	OnEditchangeActiontype();	
}

void CTriggers::OnEditchangeActionwaypoint() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Action.GetCurSel();
	if(sel2<0) return;
	

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString waypoint;
	m_ActionWaypoint.GetWindowText(waypoint);
	TruncSpace(waypoint);

	waypoint=GetWaypoint(atoi(waypoint));

	int pos=1+8*sel2+7;
	
	ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger], pos, (LPCTSTR)waypoint);
	
}

void CTriggers::OnSelchangeActionwaypoint() 
{
	int csel=m_ActionWaypoint.GetCurSel();
	CString trg;
	m_ActionWaypoint.GetLBText(csel,trg);
	m_ActionWaypoint.SetWindowText(trg);
	OnEditchangeActionwaypoint();
}

void CTriggers::OnEditchangeActionparam1() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Action.GetCurSel();
	if(sel2<0) return;
	

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString p1;
	m_A1.GetWindowText(p1);
	TruncSpace(p1);

	int pos=1+8*sel2+1;
	
	ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger], pos, (LPCTSTR)p1);
	
}

void CTriggers::OnSelchangeActionparam1() 
{
	int csel=m_A1.GetCurSel();
	CString trg;
	m_A1.GetLBText(csel,trg);
	m_A1.SetWindowText(trg);
	OnEditchangeActionparam1();
}

void CTriggers::OnEditchangeActionparam2() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Action.GetCurSel();
	if(sel2<0) return;
	

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString p2;
	m_A2.GetWindowText(p2);
	TruncSpace(p2);

	int pos=1+8*sel2+2;
	
	ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger], pos, (LPCTSTR)p2);
	
}

void CTriggers::OnSelchangeActionparam2() 
{
	int csel=m_A2.GetCurSel();
	CString trg;
	m_A2.GetLBText(csel,trg);
	m_A2.SetWindowText(trg);
	OnEditchangeActionparam2();	
}

void CTriggers::OnEditchangeActionparam3() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Action.GetCurSel();
	if(sel2<0) return;
	

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString p3;
	m_A3.GetWindowText(p3);
	TruncSpace(p3);

	int pos=1+8*sel2+3;
	
	ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger], pos, (LPCTSTR)p3);
	
}

void CTriggers::OnSelchangeActionparam3() 
{
	int csel=m_A3.GetCurSel();
	CString trg;
	m_A3.GetLBText(csel,trg);
	m_A3.SetWindowText(trg);
	OnEditchangeActionparam3();
}

void CTriggers::OnEditchangeActionparam4() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Action.GetCurSel();
	if(sel2<0) return;
	

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString p4;
	m_A4.GetWindowText(p4);
	TruncSpace(p4);

	int pos=1+8*sel2+4;
	
	ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger], pos, (LPCTSTR)p4);
	
}

void CTriggers::OnSelchangeActionparam4() 
{
	int csel=m_A4.GetCurSel();
	CString trg;
	m_A4.GetLBText(csel,trg);
	m_A4.SetWindowText(trg);
	OnEditchangeActionparam4();	
}

void CTriggers::OnEditchangeActionparam5() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Action.GetCurSel();
	if(sel2<0) return;
	

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString p5;
	m_A5.GetWindowText(p5);
	TruncSpace(p5);

	int pos=1+8*sel2+5;
	
	ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger], pos, (LPCTSTR)p5);
	
}

void CTriggers::OnSelchangeActionparam5() 
{
	int csel=m_A5.GetCurSel();
	CString trg;
	m_A5.GetLBText(csel,trg);
	m_A5.SetWindowText(trg);
	OnEditchangeActionparam5();
}

void CTriggers::OnEditchangeActionparam6() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Action.GetCurSel();
	if(sel2<0) return;
	

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CString p6;
	m_A6.GetWindowText(p6);
	TruncSpace(p6);

	int pos=1+8*sel2+6;
	
	ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)CurrentTrigger], pos, (LPCTSTR)p6);
	
}

void CTriggers::OnSelchangeActionparam6() 
{
	int csel=m_A6.GetCurSel();
	CString trg;
	m_A6.GetLBText(csel,trg);
	m_A6.SetWindowText(trg);
	OnEditchangeActionparam6();
}

void CTriggers::OnAddevent() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;


	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CIniFileSection& sec=ini.sections["Events"];

	int cval=atoi(GetParam(sec.values[(LPCTSTR)CurrentTrigger],0));
	cval++;
	char c[50];
	itoa(cval,c,10);
	
	sec.values[(LPCTSTR)CurrentTrigger]=SetParam(sec.values[(LPCTSTR)CurrentTrigger],0,c);
	sec.values[(LPCTSTR)CurrentTrigger]+=",13,0,0";


	UpdateDialog();

	m_Trigger.SetCurSel(sel);
	OnSelchangeTrigger();
	m_Event.SetCurSel(cval-1);
	OnSelchangeEvent();

}

void CTriggers::OnDeleteevent() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Event.GetCurSel();
	if(sel2<0) return;
	if(MessageBox("Do you really want to delete this event?","Delete event", MB_YESNO)==IDNO) return;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CIniFileSection& sec=ini.sections["Events"];

	CString data;
	data=sec.values[(LPCTSTR)CurrentTrigger];

	int v=atoi(GetParam(data,0));
	char c[50];
	v--;
	itoa(v,c,10);
	data=SetParam(data,0, c);

	int pos=1+sel2*3;
	int posc=1+v*3;
	int i;
	for(i=0;i<3;i++)
		data=SetParam(data,pos+i, GetParam(data,posc+i));

	char* cupos=(char*)(LPCTSTR)data;
	for(i=0;i<posc;i++)
	{
		cupos=strchr(cupos+1, ',');
		if(i==posc-1)
		{
			cupos[0]=0;
			break;
		}
	}

	sec.values[(LPCTSTR)CurrentTrigger]=data;
	UpdateDialog();
	m_Trigger.SetCurSel(sel);
	OnSelchangeTrigger();
}

void CTriggers::OnAddaction() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;


	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CIniFileSection& sec=ini.sections["Actions"];

	int cval=atoi(GetParam(sec.values[(LPCTSTR)CurrentTrigger],0));
	cval++;
	char c[50];
	itoa(cval,c,10);
	
	sec.values[(LPCTSTR)CurrentTrigger]=SetParam(sec.values[(LPCTSTR)CurrentTrigger],0,c);
	sec.values[(LPCTSTR)CurrentTrigger]+=",0,0,0,0,0,0,0,A";


	UpdateDialog();

	m_Trigger.SetCurSel(sel);
	OnSelchangeTrigger();
	m_Action.SetCurSel(cval-1);
	OnSelchangeAction();	
}

void CTriggers::OnDeleteaction() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int sel2=m_Action.GetCurSel();
	if(sel2<0) return;
	if(MessageBox("Do you really want to delete this action?","Delete action", MB_YESNO)==IDNO) return;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	CIniFileSection& sec=ini.sections["Actions"];

	CString data;
	data=sec.values[(LPCTSTR)CurrentTrigger];

	int v=atoi(GetParam(data,0));
	char c[50];
	v--;
	itoa(v,c,10);
	data=SetParam(data,0, c);

	int pos=1+sel2*8;
	int posc=1+v*8;
	int i;
	for(i=0;i<3;i++)
		data=SetParam(data,pos+i, GetParam(data,posc+i));

	char* cupos=(char*)(LPCTSTR)data;
	for(i=0;i<posc;i++)
	{
		cupos=strchr(cupos+1, ',');
		if(i==posc-1)
		{
			cupos[0]=0;
			break;
		}
	}

	sec.values[(LPCTSTR)CurrentTrigger]=data;
	UpdateDialog();	

	m_Trigger.SetCurSel(sel);
	OnSelchangeTrigger();
}


void CTriggers::OnDeletetrigger() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;

	if(MessageBox("Do you really want to delete this trigger? Don´t forget to delete the attached tag (important!)","Delete trigger", MB_YESNO)==IDNO) return;

	CString CurrentTrigger;
	m_Trigger.GetLBText(sel, CurrentTrigger);
	TruncSpace(CurrentTrigger);

	ini.sections["Triggers"].values.erase((LPCTSTR)CurrentTrigger);
	ini.sections["Events"].values.erase((LPCTSTR)CurrentTrigger);
	ini.sections["Actions"].values.erase((LPCTSTR)CurrentTrigger);

	//UpdateDialog();
	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);
}

void CTriggers::OnAddtrigger() 
{
	CIniFile& ini=Map->GetIniFile();

	CString ID_T=GetFreeID();
	ini.sections["Triggers"].values[ID_T]="GDI,<none>,New trigger,0,1,1,1,0";
	ini.sections["Events"].values[ID_T]="0";
	ini.sections["Actions"].values[ID_T]="0";

	if(MessageBox("Trigger created. If you want to create a simple tag now, press Yes. The tag will be called ""New tag"", you should name it like the trigger (after you have set up the trigger).","Trigger created",MB_YESNO))
	{
		CString ID_TAG=GetFreeID();
		ini.sections["Tags"].values[ID_TAG]="0,New tag,";
		ini.sections["Tags"].values[ID_TAG]+=ID_T;
	}

	//UpdateDialog();
	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);
		
}


