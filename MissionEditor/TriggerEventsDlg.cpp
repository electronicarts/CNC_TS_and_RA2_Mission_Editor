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

// TriggerEventsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "TriggerEventsDlg.h"
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
// Dialogfeld CTriggerEventsDlg 


CTriggerEventsDlg::CTriggerEventsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTriggerEventsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTriggerEventsDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CTriggerEventsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTriggerEventsDlg)
	DDX_Control(pDX, IDC_PARAMVALUE, m_ParamValue);
	DDX_Control(pDX, IDC_PARAMETER, m_Parameter);
	DDX_Control(pDX, IDC_EVENTTYPE, m_EventType);
	DDX_Control(pDX, IDC_EVENTDESCRIPTION, m_EventDescription);
	DDX_Control(pDX, IDC_EVENT, m_Event);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTriggerEventsDlg, CDialog)
	//{{AFX_MSG_MAP(CTriggerEventsDlg)
	ON_BN_CLICKED(IDC_NEWEVENT, OnNewevent)
	ON_BN_CLICKED(IDC_DELETEEVENT, OnDeleteevent)
	ON_CBN_SELCHANGE(IDC_EVENT, OnSelchangeEvent)
	ON_CBN_EDITCHANGE(IDC_EVENTTYPE, OnEditchangeEventtype)
	ON_LBN_SELCHANGE(IDC_PARAMETER, OnSelchangeParameter)
	ON_CBN_EDITCHANGE(IDC_PARAMVALUE, OnEditchangeParamvalue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTriggerEventsDlg 

// MW 07/23/01: Added this because startpos=1+curev*3 isn´t anymore valid for calculating the next event
int GetEventParamStart(CString& EventData, int param)
{
	int count=atoi(GetParam(EventData, 0));
	if(param>=count) return -1;

	int pos=1;
	int i;
	for(i=0;i<param;i++)
	{
		pos+=1; // jump to first eventtype param
		int needs=atoi(GetParam(EventData, pos));
		
		pos+=2; // jump to next usual eventtype
		if(needs==2) // if needs of last eventtype is 2, we need to add 1
		{
			pos+=1;
		}
	}

	return pos;
}

void CTriggerEventsDlg::OnNewevent() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_currentTrigger.GetLength()==0) return;


	CIniFileSection& sec=ini.sections["Events"];

	int cval=atoi(GetParam(sec.values[(LPCTSTR)m_currentTrigger],0));
	cval++;
	char c[50];
	itoa(cval,c,10);
	
	sec.values[(LPCTSTR)m_currentTrigger]=SetParam(sec.values[(LPCTSTR)m_currentTrigger],0,c);
	sec.values[(LPCTSTR)m_currentTrigger]+=",0,0,0";


	UpdateDialog();

	m_Event.SetCurSel(cval-1);
	OnSelchangeEvent();	
}

int FindTokenX(CString data, char token, int n)
{
	if(n<=0) return -1;

	int found=0;
	int pos=0;
	while(found<n)
	{
		if(!pos)
		pos=data.Find(token, 0);
		else
		pos=data.Find(token, pos+1);
		if(pos<0)
		{
			return -1;
		}

		found++;
	}

	return pos;
}

void CTriggerEventsDlg::OnDeleteevent() 
{
	char d[50];
	//itoa(FindTokenX("0,1,2,3,4", ',', 3),d,10);
	//MessageBox(d);

	CIniFile& ini=Map->GetIniFile();
	if(m_currentTrigger.GetLength()==0) return;

	int sel2=m_Event.GetCurSel();
	if(sel2<0) return;
	int curev=m_Event.GetItemData(sel2);
	if(MessageBox("Do you really want to delete this event?","Delete event", MB_YESNO)==IDNO) return;


	CIniFileSection& sec=ini.sections["Events"];

	CString data;
	data=sec.values[(LPCTSTR)m_currentTrigger];

	CString orig_data=data;

	int v=atoi(GetParam(data,0));
	char c[50];
	v--;
	itoa(v,c,10);
	data=SetParam(data,0, c);

	int pos=GetEventParamStart(orig_data, curev);//1+curev*3;
	//int posc=GetEventParamStart(orig_data, v);//1+v*3;
	
	// MW 07/23/01:
	// NEW DELETE EVENT CODE...

	// Now check for those 2 param + code events...
	BOOL bEvent1Needs4=FALSE;

	
	if(GetParam(data, pos+1)=="2") bEvent1Needs4=TRUE;
				
	int count=3;
	if(bEvent1Needs4) count=4;

	int del_start=FindTokenX(data, ',', pos);
	int del_end=FindTokenX(data, ',', pos+count);
	if(del_end<0)
	{
		// beyond end, so delete all...
		del_end=data.GetLength();

	}
	data.Delete(del_start, del_end-del_start);

	/*
	old delete event code, that used replacement. Not possible anymore because of 4 values/event sometimes now
	
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
	}*/

	sec.values[(LPCTSTR)m_currentTrigger]=data;
	UpdateDialog();
		
}

void CTriggerEventsDlg::OnSelchangeEvent() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_currentTrigger.GetLength()==0) return;
	int selev=m_Event.GetCurSel();
	if(selev<0) return;
	int curev=m_Event.GetItemData(selev);

	
	int i;

	CString EventData;
	EventData=ini.sections["Events"].values[(LPCTSTR)m_currentTrigger];
	
	int startpos=GetEventParamStart(EventData, curev); //1+curev*3;
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

	OnEditchangeEventtype();
}

void CTriggerEventsDlg::OnEditchangeEventtype() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_currentTrigger.GetLength()==0) return;
	int selev=m_Event.GetCurSel();
	if(selev<0) return;
	int curev=m_Event.GetItemData(selev);

	CString e1,e2;
	while(m_Parameter.DeleteString(0)!=CB_ERR);


	CString eventtype,eventdata;
	m_EventType.GetWindowText(eventtype);
	TruncSpace(eventtype);

	if(eventtype.GetLength()==0)
	{
		eventtype="0";
		m_EventType.SetWindowText(eventtype);
	}

	int pos=GetEventParamStart(ini.sections["Events"].values[(LPCTSTR)m_currentTrigger], curev); //1+3*curev;
	
	BOOL bAlready4=FALSE;
	if(atoi(GetParam(ini.sections["Events"].values[(LPCTSTR)m_currentTrigger], pos+1))==2)
		bAlready4=TRUE;
	
	ini.sections["Events"].values[(LPCTSTR)m_currentTrigger]=SetParam(ini.sections["Events"].values[(LPCTSTR)m_currentTrigger], pos, (LPCTSTR)eventtype);


	CString evsec="Events";
#ifdef RA2_MODE
	evsec="EventsRA2";
#endif


	if(g_data.sections[evsec].FindName(eventtype)<0) return;
	eventdata=g_data.sections[evsec].values[eventtype];

#ifdef RA2_MODE
	if(g_data.sections["EventsRA2"].FindName(eventtype)>=0)
		eventdata=g_data.sections["EventsRA2"].values[eventtype];
#endif

	CString desc=GetParam(eventdata,5);
	desc.Replace("%1",",");
	m_EventDescription.SetWindowText(desc); 
	
	CString ptype[2];
	ptype[0]=GetParam(eventdata,1);
	ptype[1]=GetParam(eventdata,2);


	int pListType[2];
	pListType[0]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[0]], 1));
	pListType[1]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[1]], 1));

	int code=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[0]], 2)); // usually 0

	// **************************************
	// MW ADD SUPPORT FOR 2 PARAMS+CODE
	if(code==2)
	{	
		// add code + event 0 ONLY IF THEY DO NOT ALREADY EXIST
		if(!bAlready4) ini.sections["Events"].values[(LPCTSTR)m_currentTrigger]=SetParam(ini.sections["Events"].values[(LPCTSTR)m_currentTrigger], pos+1, "2,0");		

	}
	else
	{
		// remove code + event 0
		char c[50];
		itoa(code, c, 10);
		ini.sections["Events"].values[(LPCTSTR)m_currentTrigger]=SetParam(ini.sections["Events"].values[(LPCTSTR)m_currentTrigger], pos+1, c);

		if(bAlready4) 
		{
						
			CString& data=ini.sections["Events"].values[(LPCTSTR)m_currentTrigger];

			int del_start=FindTokenX(data, ',', pos+2);
			int del_end=FindTokenX(data, ',', pos+3);
			if(del_end<0)
			{
				// beyond end, so delete all...
				del_end=data.GetLength();

			}
			data.Delete(del_start, del_end-del_start);
		}

	}
	// **************************************

	int i;
	for(i=0;i<2;i++)
	{
		int add=0;
		if(code==2) add=1;
		
			if(atoi(ptype[i])!=0 && atoi(ptype[i])>0 && atoi(ptype[i])!=47)
			{
				CString paramname=GetParam(g_data.sections["ParamTypes"].values[ptype[i]], 0);				
				
				m_Parameter.SetItemData(m_Parameter.AddString(paramname), i+add);
			}
		
		
	}

	m_ParamValue.SetWindowText("");
	if(m_Parameter.GetCount()>0) {
		m_Parameter.SetCurSel(0);
		OnSelchangeParameter();
	}
	
	

	RepairTrigger(ini.sections["Triggers"].values[m_currentTrigger]);
	
}



void CTriggerEventsDlg::OnSelchangeParameter() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_currentTrigger.GetLength()==0) return;
	int selev=m_Event.GetCurSel();
	if(selev<0) return;
	int curev=m_Event.GetItemData(selev);

	int curselparam=m_Parameter.GetCurSel();
	if(curselparam<0) 
	{
		m_ParamValue.SetWindowText("");
		return;
	}

	

	int curparam=m_Parameter.GetItemData(curselparam);
	
		
	CString EventData;
	EventData=ini.sections["Events"].values[(LPCTSTR)m_currentTrigger];
	
	int startpos=GetEventParamStart(EventData, curev);//1+curev*3;

	
	// MW FIX FOR CODE!=0
	int original_cuparam=curparam;
#ifdef RA2_MODE
	CString Param1=GetParam(g_data.sections["EventsRA2"].values[GetParam(EventData,startpos)],1);
	CString Code=GetParam(g_data.sections["ParamTypes"].values[Param1],2);
	//MessageBox(Param1, Code);
	if(atoi(Code)!=0) curparam--;
#endif
	// END FIx

	CString ParamType=GetParam(g_data.sections["Events"].values[GetParam(EventData,startpos)],1+curparam);
#ifdef RA2_MODE
	if(g_data.sections["EventsRA2"].FindName(GetParam(EventData, startpos))>=0) ParamType=GetParam(g_data.sections["EventsRA2"].values[GetParam(EventData,startpos)],1+curparam);
#endif

	if(atoi(ParamType)<0)
	{
	}
	else
	{
		CString ListType=GetParam(g_data.sections["ParamTypes"].values[ParamType],1);
		
		HandleParamList(m_ParamValue, atoi(ListType));
		m_ParamValue.SetWindowText(GetParam(EventData,startpos+1+original_cuparam));
		
		int i;
		BOOL bFound=FALSE;
		for(i=0;i<m_ParamValue.GetCount();i++)
		{
			CString tmp;
			m_ParamValue.GetLBText(i,tmp);
			TruncSpace(tmp);
			if(tmp==GetParam(EventData,startpos+1+original_cuparam))
			{
				m_ParamValue.SetCurSel(i);
				bFound=TRUE;
				break;
			}
		}
		
		/*if(!bFound)
		{
			// not found, change it...
		
			if(m_ParamValue.GetCount()) {
				m_ParamValue.SetCurSel(0);
				OnEditchangeParamvalue();
			}
			else
			{
				// not numeric value?

				CString p=GetParam(EventData,startpos+1+original_cuparam);
				char c[50];
				itoa(atoi(p), c, 10);
				if(c!=p) {
					m_ParamValue.SetWindowText("0");
					OnEditchangeParamvalue();
				}				
			}
		}*/
	}
}

void CTriggerEventsDlg::OnEditchangeParamvalue() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_currentTrigger.GetLength()==0) return;
	int selev=m_Event.GetCurSel();
	if(selev<0) return;
	int curev=m_Event.GetItemData(selev);

	int curselparam=m_Parameter.GetCurSel();
	if(curselparam<0) 
	{
		m_ParamValue.SetWindowText("");
		return;
	}

	int curparam=m_Parameter.GetItemData(curselparam);
	
	CString EventData;
	EventData=ini.sections["Events"].values[(LPCTSTR)m_currentTrigger];
	
	int startpos=GetEventParamStart(EventData, curev);// 1+curev*3;


	CString newVal;
	m_ParamValue.GetWindowText(newVal);
	TruncSpace(newVal);
	newVal.TrimLeft();

	if(newVal.Find(",",0)>=0) newVal.SetAt(newVal.Find(",",0), 0);

	ini.sections["Events"].values[(LPCTSTR)m_currentTrigger]=SetParam(EventData, startpos+1+curparam, newVal);
}

void CTriggerEventsDlg::UpdateDialog()
{
	// MW 07/20/01
	Clear();

	if(m_currentTrigger.GetLength()==0) 
	{
		while(m_Event.DeleteString(0)!=CB_ERR);
		return;
	}
	CIniFile& ini=Map->GetIniFile();

	while(m_EventType.DeleteString(0)!=CB_ERR);
	int i;

	//9.3.2001: Only use specified section, do not merge
#ifndef RA2_MODE 
	CString sec="Events";
#else
	CString sec="EventsRA2";
#endif

	for(i=0;i<g_data.sections[sec].values.size();i++)
	{
		CString eventid = *g_data.sections[sec].GetValueName(i); //GetParam(*g_data.sections["Events"].GetValue(i),8);
		CString eventdata=*g_data.sections[sec].GetValue(i);
/*#ifdef RA2_MODE
		if(g_data.sections["EventsRA2"].FindName(eventid)>=0)
			eventdata=g_data.sections["EventsRA2"].values[eventid];
#endif*/

		CString text=eventid+" "+GetParam(eventdata,0);
		text.Replace("%1",",");
#ifdef RA2_MODE
		// MW 07/18/01
		// for yuri mode, only check if it´s for RA2, else support it only if YR isnt´needed...
		if(GetParam(eventdata, 7)=="1" && ( yuri_mode || !atoi(GetParam(eventdata,9)) ) )
		{


#else
		if(GetParam(eventdata, 6)=="1")
		{
#endif
			m_EventType.AddString(text);
		}
	}



	int cur_sel=m_Event.GetCurSel();
	while(m_Event.DeleteString(0)!=CB_ERR);
	
	CString Data=ini.sections["Events"].values[m_currentTrigger];
	int count=atoi(GetParam(Data,0));

	for(i=0;i<count;i++)
	{
		char c[50];
		itoa(i,c,10);

		CString s=TranslateStringACP("Event");
		s+=" ";
		s+=c;

		m_Event.SetItemData(m_Event.AddString(s), i);
	}

	if(cur_sel<0) cur_sel=0;
	if(cur_sel>=count) cur_sel=count-1;

	m_Event.SetCurSel(cur_sel);


	OnSelchangeEvent();
}

// MW 07/20/01
void CTriggerEventsDlg::Clear()
{
	m_EventType.SetWindowText("");	
	while(m_Parameter.DeleteString(0)!=LB_ERR);
	m_ParamValue.SetWindowText("");
	m_EventDescription.SetWindowText("");
}
