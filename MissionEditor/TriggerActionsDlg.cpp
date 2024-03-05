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

// TriggerActionsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "TriggerActionsDlg.h"
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


BOOL IsWaypointFormat(CString s)
{
	if(s.GetLength()==0) return TRUE;
	
	if(s[0]>='A' && s[0]<='Z')
		return TRUE;

	return FALSE;

}

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTriggerActionsDlg 


CTriggerActionsDlg::CTriggerActionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTriggerActionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTriggerActionsDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CTriggerActionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTriggerActionsDlg)
	DDX_Control(pDX, IDC_PARAMVALUE, m_ParamValue);
	DDX_Control(pDX, IDC_PARAMETER, m_Parameter);
	DDX_Control(pDX, IDC_ACTIONTYPE, m_ActionType);
	DDX_Control(pDX, IDC_ACTIONDESCRIPTION, m_ActionDescription);
	DDX_Control(pDX, IDC_ACTION, m_Action);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTriggerActionsDlg, CDialog)
	//{{AFX_MSG_MAP(CTriggerActionsDlg)
	ON_CBN_SELCHANGE(IDC_ACTION, OnSelchangeAction)
	ON_CBN_EDITCHANGE(IDC_ACTIONTYPE, OnEditchangeActiontype)
	ON_LBN_SELCHANGE(IDC_PARAMETER, OnSelchangeParameter)
	ON_CBN_EDITCHANGE(IDC_PARAMVALUE, OnEditchangeParamvalue)
	ON_BN_CLICKED(IDC_NEWACTION, OnNewaction)
	ON_BN_CLICKED(IDC_DELETEACTION, OnDeleteaction)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTriggerActionsDlg 

void CTriggerActionsDlg::OnSelchangeAction() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_currentTrigger.GetLength()==0) return;
	int selev=m_Action.GetCurSel();
	if(selev<0) return;
	int curev=m_Action.GetItemData(selev);

	
	int i;

	CString ActionData;
	ActionData=ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger];
	
	int startpos=1+curev*8;
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

	OnEditchangeActiontype();
	
}

void CTriggerActionsDlg::OnEditchangeActiontype() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_currentTrigger.GetLength()==0) return;
	int selev=m_Action.GetCurSel();
	if(selev<0) return;
	int curev=m_Action.GetItemData(selev);

	CString e1,e2;
	while(m_Parameter.DeleteString(0)!=CB_ERR);


	CString eventtype,eventdata;
	m_ActionType.GetWindowText(eventtype);
	TruncSpace(eventtype);

	if(eventtype.GetLength()==0)
	{
		eventtype="0";
		m_ActionType.SetWindowText(eventtype);
	}


	CString acsec="Actions";
#ifdef RA2_MODE
	acsec="ActionsRA2";
#endif
	

	int pos=1+8*curev;
	
	ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger], pos, (LPCTSTR)eventtype);

	if(g_data.sections[acsec].FindName(eventtype)<0) return;

	eventdata=g_data.sections[acsec].values[eventtype];

#ifdef RA2_MODE
	if(g_data.sections["ActionsRA2"].FindName(eventtype)>=0)
		eventdata=g_data.sections["ActionsRA2"].values[eventtype];
#endif

	CString desc=GetParam(eventdata,10);
	desc.Replace("%1",",");
	m_ActionDescription.SetWindowText(desc); 
	
	CString ptype[6];
	ptype[0]=GetParam(eventdata,1);
	ptype[1]=GetParam(eventdata,2);
	ptype[2]=GetParam(eventdata,3);
	ptype[3]=GetParam(eventdata,4);
	ptype[4]=GetParam(eventdata,5);
	ptype[5]=GetParam(eventdata,6);

	int pListType[6];
	memset(pListType, 0, 6*sizeof(int));

	if(atoi(ptype[0])>=0) pListType[0]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[0]], 1));
	if(atoi(ptype[1])>=0) pListType[1]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[1]], 1));
	if(atoi(ptype[2])>=0) pListType[2]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[2]], 1));
	if(atoi(ptype[3])>=0) pListType[3]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[3]], 1));
	if(atoi(ptype[4])>=0) pListType[4]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[4]], 1));
	if(atoi(ptype[5])>=0) pListType[5]=atoi(GetParam(g_data.sections["ParamTypes"].values[ptype[5]], 1));

	int i;
	for(i=0;i<6;i++)
	{

		if(atoi(ptype[i])>0)
		{
			CString paramname=GetParam(g_data.sections["ParamTypes"].values[ptype[i]], 0);
			
			m_Parameter.SetItemData(m_Parameter.AddString(paramname), i);
		}
		else if(atoi(ptype[i])<0)
		{
			char c[50];
			itoa(-atoi(ptype[i]),c,10);
			ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger], pos+i+1,c );			
		}
		else
		{
			ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger], pos+i+1, "0");
		}
		
	}

	// MW fix for waypoint/number issue
	CString code;
	BOOL bNoWP=FALSE;
	code=GetParam(ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger], pos+1);
	if(g_data.sections["DontSaveAsWP"].FindValue(code)>=0) bNoWP=TRUE;

	// conversion below:
	if(IsWaypointFormat(GetParam(ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger], pos+1+6)) && bNoWP)
	{
		int number=GetWaypoint(GetParam(ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger], pos+1+6));
		char c[50];
		itoa(number, c, 10);
		ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger],  pos+1+6, c);
	}
	else if(!IsWaypointFormat(GetParam(ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger], pos+1+6)) && !bNoWP)
	{
		int wp=atoi(GetParam(ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger], pos+1+6));
		CString s=GetWaypoint(wp);
		ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger],  pos+1+6, s);
		
	}


	if(atoi(GetParam(eventdata, 7))==1)
	{
		if(bNoWP)
			m_Parameter.SetItemData(m_Parameter.AddString(TranslateStringACP("Number")), -1);
		else
			m_Parameter.SetItemData(m_Parameter.AddString(TranslateStringACP("Waypoint")), -1);
	}

	m_ParamValue.SetWindowText("");
	if(m_Parameter.GetCount()>0) {
		m_Parameter.SetCurSel(0);
		OnSelchangeParameter();
	}
}

void CTriggerActionsDlg::OnSelchangeParameter() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_currentTrigger.GetLength()==0) return;
	int selev=m_Action.GetCurSel();
	if(selev<0) return;
	int curev=m_Action.GetItemData(selev);

	int curselparam=m_Parameter.GetCurSel();
	if(curselparam<0) 
	{
		m_ParamValue.SetWindowText("");
		return;
	}

	

	int curparam=m_Parameter.GetItemData(curselparam);
	
	
	
	CString ActionData;
	ActionData=ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger];

	
	int startpos=1+curev*8;

	CString code;
	BOOL bNoWP=FALSE;
	code=GetParam(ActionData, startpos+1);
	if(g_data.sections["DontSaveAsWP"].FindValue(code)>=0) bNoWP=TRUE;

	if(curparam>=0 && curparam<6)
	{
		CString ParamType=GetParam(g_data.sections["Actions"].values[GetParam(ActionData,startpos)],1+curparam);
#ifdef RA2_MODE
		if(g_data.sections["ActionsRA2"].FindName(GetParam(ActionData, startpos))>=0)
		{
			ParamType=GetParam(g_data.sections["ActionsRA2"].values[GetParam(ActionData,startpos)],1+curparam);
		}
#endif
		if(atoi(ParamType)<0)
		{
		}
		else
		{
			CString ListType=GetParam(g_data.sections["ParamTypes"].values[ParamType],1);
			HandleParamList(m_ParamValue, atoi(ListType));
			m_ParamValue.SetWindowText(GetParam(ActionData,startpos+1+curparam));
			
			int i;
			BOOL bFound=FALSE;
			for(i=0;i<m_ParamValue.GetCount();i++)
			{
				CString tmp;
				m_ParamValue.GetLBText(i,tmp);
				TruncSpace(tmp);
				if(tmp==GetParam(ActionData,startpos+1+curparam))
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

					CString p=GetParam(ActionData,startpos+1+curparam);
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
	else if(curparam==-1)
	{
		char wayp[50];
		if(!bNoWP)
		{
			ListWaypoints(m_ParamValue);
			int iWayp=GetWaypoint(GetParam(ActionData,startpos+1+6));
			
			itoa(iWayp, wayp, 10);
		}
		else
		{
			strcpy(wayp, GetParam(ActionData, startpos+1+6));
			HandleParamList(m_ParamValue, 0);
		}
			

		m_ParamValue.SetWindowText(wayp);
	}
}

void CTriggerActionsDlg::OnEditchangeParamvalue() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_currentTrigger.GetLength()==0) return;
	int selev=m_Action.GetCurSel();
	if(selev<0) return;
	int curev=m_Action.GetItemData(selev);

	int curselparam=m_Parameter.GetCurSel();
	if(curselparam<0) 
	{
		m_ParamValue.SetWindowText("");
		return;
	}

	int curparam=m_Parameter.GetItemData(curselparam);
	
	CString ActionData;
	ActionData=ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger];
	
	int startpos=1+curev*8;


	CString code;
	BOOL bNoWP=FALSE;
	code=GetParam(ActionData, startpos+1);
	if(g_data.sections["DontSaveAsWP"].FindValue(code)>=0) bNoWP=TRUE;

	CString newVal;
	m_ParamValue.GetWindowText(newVal);
	TruncSpace(newVal);
	newVal.TrimLeft();

	if(newVal.Find(",",0)>=0) newVal.SetAt(newVal.Find(",",0), 0);

	if(curparam>=0)
	{
		ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger]=SetParam(ActionData, startpos+1+curparam, newVal);
	}
	else if(curparam==-1) // waypoint FIX MW: OR NUMBER!!!
	{
		int pos=1+8*curev+7;
		

		CString waypoint=newVal;
		
		if(!bNoWP) waypoint=GetWaypoint(atoi(newVal));
		
		ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger]=SetParam(ini.sections["Actions"].values[(LPCTSTR)m_currentTrigger], pos, (LPCTSTR)waypoint);
	}

}

void CTriggerActionsDlg::OnNewaction() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_currentTrigger.GetLength()==0) return;


	CIniFileSection& sec=ini.sections["Actions"];

	int cval=atoi(GetParam(sec.values[(LPCTSTR)m_currentTrigger],0));
	cval++;
	char c[50];
	itoa(cval,c,10);
	
	sec.values[(LPCTSTR)m_currentTrigger]=SetParam(sec.values[(LPCTSTR)m_currentTrigger],0,c);
	sec.values[(LPCTSTR)m_currentTrigger]+=",0,0,0,0,0,0,0,A";


	UpdateDialog();

	m_Action.SetCurSel(cval-1);
	OnSelchangeAction();	
}

void CTriggerActionsDlg::OnDeleteaction() 
{
	CIniFile& ini=Map->GetIniFile();
	if(m_currentTrigger.GetLength()==0) return;

	int sel2=m_Action.GetCurSel();
	if(sel2<0) return;
	int curev=m_Action.GetItemData(sel2);
	if(MessageBox("Do you really want to delete this action?","Delete action", MB_YESNO)==IDNO) return;


	CIniFileSection& sec=ini.sections["Actions"];

	CString data;
	data=sec.values[(LPCTSTR)m_currentTrigger];

	int v=atoi(GetParam(data,0));
	char c[50];
	v--;
	itoa(v,c,10);
	data=SetParam(data,0, c);

	int pos=1+curev*8;
	int posc=1+v*8;
	int i;
	for(i=0;i<8;i++)
		data=SetParam(data,pos+i, GetParam(data,posc+i));

	// MW April 17th, 2002:
	// fixed: (char*)(LPCTSTR)data should not be modified directly,
	// Instead, moving to a buffer
	char* str_act=new(char[data.GetLength()+1]);
	strcpy(str_act, (LPCSTR) data);
	char* cupos=str_act;//(char*)(LPCTSTR)data;
	for(i=0;i<posc;i++)
	{
		cupos=strchr(cupos+1, ',');
		if(i==posc-1)
		{
			cupos[0]=0;
			break;
		}
	}
	//MessageBox(str_act);

	sec.values[(LPCTSTR)m_currentTrigger]=str_act;
	delete[] str_act;
	UpdateDialog();
		
}

void CTriggerActionsDlg::UpdateDialog()
{
	// MW 07/20/01
	Clear();

	if(m_currentTrigger.GetLength()==0) 
	{
		while(m_Action.DeleteString(0)!=CB_ERR);
		return;
	}

	CIniFile& ini=Map->GetIniFile();

	// 9.3.2001: Only support specified section
#ifndef RA2_MODE
	CString sec="Actions";
#else
	CString sec="ActionsRA2";
#endif

	while(m_ActionType.DeleteString(0)!=CB_ERR);
	int i;
	for(i=0;i<g_data.sections[sec].values.size();i++)
	{
		CString eventid=*g_data.sections[sec].GetValueName(i); //GetParam(*g_data.sections["Actions"].GetValue(i),13);
		CString eventdata=*g_data.sections[sec].GetValue(i);
/*#ifdef RA2_MODE
		if(g_data.sections["ActionsRA2"].FindName(eventid)>=0)
			eventdata=g_data.sections["ActionsRA2"].values[eventid];
#endif*/
		CString text=eventid+" "+GetParam(eventdata,0);
		text.Replace("%1",",");

#ifdef RA2_MODE
		if(GetParam(eventdata,12)=="1" && (yuri_mode || !isTrue(GetParam(eventdata, 14))))
		{		
#else
		if(GetParam(eventdata,11)=="1")
		{
#endif
			m_ActionType.AddString(text);
		}
	}

	int cur_sel=m_Action.GetCurSel();
	while(m_Action.DeleteString(0)!=CB_ERR);
	
	CString Data=ini.sections["Actions"].values[m_currentTrigger];
	int count=atoi(GetParam(Data,0));

	for(i=0;i<count;i++)
	{
		char c[50];
		itoa(i,c,10);

		CString s=TranslateStringACP("Action");
		s+=" ";
		s+=c;

		m_Action.SetItemData(m_Action.AddString(s), i);
	}

	if(cur_sel<0) cur_sel=0;
	if(cur_sel>=count) cur_sel=count-1;

	m_Action.SetCurSel(cur_sel);


	OnSelchangeAction();
}

// MW 07/20/01
void CTriggerActionsDlg::Clear()
{
	m_ActionType.SetWindowText("");	
	while(m_Parameter.DeleteString(0)!=LB_ERR);
	m_ParamValue.SetWindowText("");
	m_ActionDescription.SetWindowText("");
}
