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

// TeamTypes.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "TeamTypes.h"
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
// Eigenschaftenseite CTeamTypes 

IMPLEMENT_DYNCREATE(CTeamTypes, CDialog)

CTeamTypes::CTeamTypes() : CDialog(CTeamTypes::IDD)
{
	//{{AFX_DATA_INIT(CTeamTypes)
	m_Aggressive = FALSE;
	m_Annoyance = FALSE;
	m_AreTeamMembersRecruitable = FALSE;
	m_Autocreate = FALSE;
	m_AvoidThreats = FALSE;
	m_Droppod = FALSE;
	m_Full = FALSE;
	m_Group = _T("");
	m_GuardSlower = FALSE;
	m_House = _T("");
	m_IonImmune = FALSE;
	m_IsBaseDefense = FALSE;
	m_Loadable = FALSE;
	m_LooseRecruit = FALSE;
	m_Max = _T("");
	m_Name = _T("");
	m_OnlyTargetHouseEnemy = FALSE;
	m_OnTransOnly = FALSE;
	m_Prebuild = FALSE;
	m_Priority = _T("");
	m_Recruiter = FALSE;
	m_Reinforce = FALSE;
	m_Script = _T("");
	m_Suicide = FALSE;
	m_TaskForce = _T("");
	m_TechLevel = _T("");
	m_TransportReturnsOnUnload = FALSE;
	m_Waypoint = _T("");
	m_Whiner = FALSE;
	m_VeteranLevel = _T("");
	m_Tag = _T("");
	m_TransportWaypoint = _T("");
	m_MindControlDecision = _T("");
	//}}AFX_DATA_INIT
}

CTeamTypes::~CTeamTypes()
{
}

void CTeamTypes::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTeamTypes)
	DDX_Control(pDX, IDC_MCD_L, m_MCD_L);
	DDX_Control(pDX, IDC_TEAMTYPES, m_TeamTypes);
	DDX_Check(pDX, IDC_AGGRESSIVE, m_Aggressive);
	DDX_Check(pDX, IDC_ANNOYANCE, m_Annoyance);
	DDX_Check(pDX, IDC_ARETEAMMEMBERSRECRUITABLE, m_AreTeamMembersRecruitable);
	DDX_Check(pDX, IDC_AUTOCREATE, m_Autocreate);
	DDX_Check(pDX, IDC_AVOIDTHREATS, m_AvoidThreats);
	DDX_Check(pDX, IDC_DROPPOD, m_Droppod);
	DDX_Check(pDX, IDC_FULL, m_Full);
	DDX_CBString(pDX, IDC_GROUP, m_Group);
	DDX_Check(pDX, IDC_GUARDSLOWER, m_GuardSlower);
	DDX_CBString(pDX, IDC_HOUSE, m_House);
	DDX_Check(pDX, IDC_IONIMMUNE, m_IonImmune);
	DDX_Check(pDX, IDC_ISBASEDEFENSE, m_IsBaseDefense);
	DDX_Check(pDX, IDC_LOADABLE, m_Loadable);
	DDX_Check(pDX, IDC_LOOSERECRUIT, m_LooseRecruit);
	DDX_Text(pDX, IDC_MAX, m_Max);
	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Check(pDX, IDC_ONLYTARGETHOUSEENEMY, m_OnlyTargetHouseEnemy);
	DDX_Check(pDX, IDC_ONTRANSONLY, m_OnTransOnly);
	DDX_Check(pDX, IDC_PREBUILD, m_Prebuild);
	DDX_Text(pDX, IDC_PRIORITY, m_Priority);
	DDX_Check(pDX, IDC_RECRUITER, m_Recruiter);
	DDX_Check(pDX, IDC_REINFORCE, m_Reinforce);
	DDX_CBString(pDX, IDC_SCRIPT, m_Script);
	DDX_Check(pDX, IDC_SUICIDE, m_Suicide);
	DDX_CBString(pDX, IDC_TASKFORCE, m_TaskForce);
	DDX_CBString(pDX, IDC_TECHLEVEL, m_TechLevel);
	DDX_Check(pDX, IDC_TRANSPORTRETURNSONUNLOAD, m_TransportReturnsOnUnload);
	DDX_CBString(pDX, IDC_WAYPOINT, m_Waypoint);
	DDX_Check(pDX, IDC_WHINER, m_Whiner);
	DDX_CBString(pDX, IDC_VETERANLEVEL, m_VeteranLevel);
	DDX_CBString(pDX, IDC_TAG, m_Tag);
	DDX_CBString(pDX, IDC_TRANSPORTWAYPOINT, m_TransportWaypoint);
	DDX_CBString(pDX, IDC_MINDCONTROLDECISION, m_MindControlDecision);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTeamTypes, CDialog)
	//{{AFX_MSG_MAP(CTeamTypes)
	ON_CBN_SELCHANGE(IDC_TEAMTYPES, OnSelchangeTeamtypes)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	ON_BN_CLICKED(IDC_DELETETEAMTYPE, OnDeleteteamtype)
	ON_CBN_EDITCHANGE(IDC_VETERANLEVEL, OnEditchangeVeteranlevel)
	ON_CBN_EDITCHANGE(IDC_HOUSE, OnEditchangeHouse)
	ON_EN_CHANGE(IDC_PRIORITY, OnChangePriority)
	ON_EN_CHANGE(IDC_MAX, OnChangeMax)
	ON_CBN_EDITCHANGE(IDC_TECHLEVEL, OnEditchangeTechlevel)
	ON_CBN_EDITCHANGE(IDC_GROUP, OnEditchangeGroup)
	ON_CBN_EDITCHANGE(IDC_WAYPOINT, OnEditchangeWaypoint)
	ON_CBN_EDITCHANGE(IDC_SCRIPT, OnEditchangeScript)
	ON_CBN_EDITCHANGE(IDC_TASKFORCE, OnEditchangeTaskforce)
	ON_CBN_KILLFOCUS(IDC_VETERANLEVEL, OnKillfocusVeteranlevel)
	ON_CBN_KILLFOCUS(IDC_HOUSE, OnKillfocusHouse)
	ON_CBN_KILLFOCUS(IDC_TECHLEVEL, OnKillfocusTechlevel)
	ON_CBN_KILLFOCUS(IDC_GROUP, OnKillfocusGroup)
	ON_CBN_KILLFOCUS(IDC_WAYPOINT, OnKillfocusWaypoint)
	ON_CBN_KILLFOCUS(IDC_SCRIPT, OnKillfocusScript)
	ON_CBN_KILLFOCUS(IDC_TASKFORCE, OnKillfocusTaskforce)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_LOADABLE, OnLoadable)
	ON_BN_CLICKED(IDC_FULL, OnFull)
	ON_BN_CLICKED(IDC_ANNOYANCE, OnAnnoyance)
	ON_BN_CLICKED(IDC_GUARDSLOWER, OnGuardslower)
	ON_BN_CLICKED(IDC_RECRUITER, OnRecruiter)
	ON_BN_CLICKED(IDC_DROPPOD, OnDroppod)
	ON_BN_CLICKED(IDC_WHINER, OnWhiner)
	ON_BN_CLICKED(IDC_LOOSERECRUIT, OnLooserecruit)
	ON_BN_CLICKED(IDC_AGGRESSIVE, OnAggressive)
	ON_BN_CLICKED(IDC_SUICIDE, OnSuicide)
	ON_BN_CLICKED(IDC_AUTOCREATE, OnAutocreate)
	ON_BN_CLICKED(IDC_PREBUILD, OnPrebuild)
	ON_BN_CLICKED(IDC_ONTRANSONLY, OnOntransonly)
	ON_BN_CLICKED(IDC_REINFORCE, OnReinforce)
	ON_BN_CLICKED(IDC_AVOIDTHREATS, OnAvoidthreats)
	ON_BN_CLICKED(IDC_IONIMMUNE, OnIonimmune)
	ON_BN_CLICKED(IDC_TRANSPORTRETURNSONUNLOAD, OnTransportreturnsonunload)
	ON_BN_CLICKED(IDC_ARETEAMMEMBERSRECRUITABLE, OnAreteammembersrecruitable)
	ON_BN_CLICKED(IDC_ISBASEDEFENSE, OnIsbasedefense)
	ON_BN_CLICKED(IDC_ONLYTARGETHOUSEENEMY, OnOnlytargethouseenemy)
	ON_BN_CLICKED(IDC_NEWTEAMTYPE, OnNewteamtype)
	ON_CBN_EDITCHANGE(IDC_TAG, OnEditchangeTag)
	ON_CBN_KILLFOCUS(IDC_TAG, OnKillfocusTag)
	ON_CBN_EDITCHANGE(IDC_TRANSPORTWAYPOINT, OnEditchangeTransportwaypoint)
	ON_CBN_KILLFOCUS(IDC_TRANSPORTWAYPOINT, OnKillfocusTransportwaypoint)
	ON_CBN_EDITCHANGE(IDC_MINDCONTROLDECISION, OnEditchangeMindcontroldecision)
	ON_CBN_KILLFOCUS(IDC_MINDCONTROLDECISION, OnKillfocusMindcontroldecision)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL stob(const char* s)
{
	if(_stricmp(s,"no")==NULL)
	{
		return FALSE;
	}
	return TRUE;
}

CString btos(BOOL b)
{
	CString s="no";
	if(b==TRUE) s="yes";
	return s;
}

int letter2number(char let){
	int reply=let-'A';
	/*if(let=='A')reply=0;
	if(let=='B')reply=1;
	if(let=='C')reply=2;
	if(let=='D')reply=3;
	if(let=='E')reply=4;
	if(let=='F')reply=5;
	if(let=='G')reply=6;
	if(let=='H')reply=7;
	if(let=='I')reply=8;
	if(let=='J')reply=9;
	if(let=='K')reply=10;
	if(let=='L')reply=11;
	if(let=='M')reply=12;
	if(let=='N')reply=13;
	if(let=='O')reply=14;
	if(let=='P')reply=15;
	if(let=='Q')reply=16;
	if(let=='R')reply=17;
	if(let=='S')reply=18;
	if(let=='T')reply=19;
	if(let=='U')reply=20;
	if(let=='V')reply=21;
	if(let=='W')reply=22;
	if(let=='X')reply=23;
	if(let=='Y')reply=24;
	if(let=='Z')reply=25;*/
	return reply;

}

char number2letter(int let){
	int reply=let+'A';
	/*if(let==0)reply='A';
	if(let==1)reply='B';
	if(let==2)reply='C';
	if(let==3)reply='D';
	if(let==4)reply='E';
	if(let==5)reply='F';
	if(let==6)reply='G';
	if(let==7)reply='H';
	if(let==8)reply='I';
	if(let==9)reply='J';
	if(let==10)reply='K';
	if(let==11)reply='L';
	if(let==12)reply='M';
	if(let==13)reply='N';
	if(let==14)reply='O';
	if(let==15)reply='P';
	if(let==16)reply='Q';
	if(let==17)reply='R';
	if(let==18)reply='S';
	if(let==19)reply='T';
	if(let==20)reply='U';
	if(let==21)reply='V';
	if(let==22)reply='W';
	if(let==23)reply='X';
	if(let==24)reply='Y';
	if(let==25)reply='Z';*/

	return reply;

}

int GetWaypoint(const char* c)
{
	if(strlen(c)==0) return -1;
    int i;
    int res=0;
    for(i=0;i<strlen(c);i++)
    {
        int addval=letter2number(c[i]);
        res+=addval+(res+1)*(i*25)+i;
    }
	return res;
}

CString GetWaypoint(int n)
{
	if(n==-1) return (CString)("");
	int i,e;
	for(i=-1;i<26;i++)
	{
		for(e=0;e<26;e++)
		{
			char c[50];
			CString p;
			if(i==-1)
			{
				c[0]=number2letter(e);
				c[1]=0;
				if(GetWaypoint(c)==n) return c;
			}
			else
			{
				c[0]=number2letter(i);
				c[1]=number2letter(e);
				c[2]=0;
				if(GetWaypoint(c)==n) return c;
			}

		}
	}
	return (CString)("");
}
	

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTeamTypes 
void CTeamTypes::UpdateDialog()
{
	if(!yuri_mode)
	{
		GetDlgItem(IDC_MINDCONTROLDECISION)->ShowWindow(SW_HIDE);
		m_MCD_L.ShowWindow(SW_HIDE);
	}

	CIniFile& ini=Map->GetIniFile();

	CComboBox& taskforces=*(CComboBox*)GetDlgItem(IDC_TASKFORCE);
	CComboBox& scripts=*(CComboBox*)GetDlgItem(IDC_SCRIPT);
	CComboBox& houses=*(CComboBox*)GetDlgItem(IDC_HOUSE);

	int sel=m_TeamTypes.GetCurSel();

	while(m_TeamTypes.DeleteString(0)!=CB_ERR);
	while(taskforces.DeleteString(0)!=CB_ERR);
	while(scripts.DeleteString(0)!=CB_ERR);

	// MW 07/24/01: Clear:
	m_Aggressive=0;	
	m_Annoyance=0;
	m_AreTeamMembersRecruitable=0;	
	m_Autocreate=0;	
	m_AvoidThreats=0;	
	m_Droppod=0;	
	m_Full=0;	
	m_Group="";	
	m_GuardSlower=0;
	m_House="";	
	m_IonImmune=0;	
	m_IsBaseDefense=0;	
	m_Loadable=0;	
	m_LooseRecruit=0;	
	m_Max="";	
	m_Name="";	
	m_OnlyTargetHouseEnemy=0;	
	m_OnTransOnly=0;
	m_Prebuild=0;	
	m_Priority="";	
	m_Recruiter=0;	
	m_Reinforce=0;	
	m_Script="";
	m_Suicide=0;	
	m_Tag="";
	m_TaskForce="";
	m_TechLevel="";
	m_TransportReturnsOnUnload=0;
	m_TransportWaypoint="";
	m_VeteranLevel="";
	m_MindControlDecision="";

	UpdateData(FALSE);
	
	int i;
	for(i=0;i<ini.sections["TeamTypes"].values.size();i++)
	{
		CString tt;
		tt=*ini.sections["TeamTypes"].GetValue(i);

		CString str;
		str=tt;
		str+=" (";
		str+=ini.sections[tt].values["Name"];
		str+=")";

		m_TeamTypes.AddString(str);
	}

	for(i=0;i<ini.sections["TaskForces"].values.size();i++)
	{
		
		CString tt;
		tt=*ini.sections["TaskForces"].GetValue(i);

		CString str;
		str=tt;
		str+=" (";
		if(ini.sections.find(tt)!=ini.sections.end())
		{
			str+=ini.sections[tt].values["Name"];
		}
		str+=")";

		taskforces.AddString(str);
	}

	for(i=0;i<ini.sections["ScriptTypes"].values.size();i++)
	{
		
		CString tt;
		tt=*ini.sections["ScriptTypes"].GetValue(i);

		CString str;
		str=tt;
		str+=" (";
		if(ini.sections.find((char*)(LPCTSTR)tt)!=ini.sections.end())
		{
			str+=ini.sections[(char*)(LPCTSTR)tt].values["Name"];
		}
		str+=")";

		scripts.AddString(str);
	}


	// now list all tags, but not "None", because we want the correct language,
	// and ListTags uses (for compatibility) always the english one
	ListTags(*(CComboBox*)GetDlgItem(IDC_TAG), FALSE);
	(*(CComboBox*)GetDlgItem(IDC_TAG)).InsertString(0, GetLanguageStringACP("None"));

	
	CComboBox* house;
	house=(CComboBox*)GetDlgItem(IDC_HOUSE);

	/*while(house->DeleteString(0)!=CB_ERR);
	// houses:  rules.ini + map definitions!
	if(ini.sections.find("Houses")!=ini.sections.end())
	{
		if(ini.sections["Houses"].values.size()==0) goto wasnohouse;
		// we use the map definitions!
		for(i=0;i<ini.sections["Houses"].values.size();i++)
		{
			house->AddString(*ini.sections["Houses"].GetValue(i));		
		}
	}
	else
	{
		wasnohouse:
		for(i=0;i<rules.sections["Houses"].values.size();i++)
		{
			house->AddString(*rules.sections["Houses"].GetValue(i));		
		}				
	}*/
	ListHouses(*house, FALSE, TRUE, TRUE);

	CComboBox* wayp;
	wayp=(CComboBox*)GetDlgItem(IDC_WAYPOINT);

	while(wayp->DeleteString(0)!=CB_ERR);
	// houses:  rules.ini + map definitions!
	if(ini.sections.find("Waypoints")!=ini.sections.end())
	{
		for(i=0;i<ini.sections["Waypoints"].values.size();i++)
		{
			wayp->AddString(*ini.sections["Waypoints"].GetValueName(i));		
		}
	}

	wayp=(CComboBox*)GetDlgItem(IDC_TRANSPORTWAYPOINT);

	while(wayp->DeleteString(0)!=CB_ERR);
	// houses:  rules.ini + map definitions!
	wayp->SetItemData(wayp->InsertString(0,TranslateStringACP("None")),0);
	
	if(ini.sections.find("Waypoints")!=ini.sections.end())
	{
		for(i=0;i<ini.sections["Waypoints"].values.size();i++)
		{
			wayp->SetItemData(wayp->AddString(*ini.sections["Waypoints"].GetValueName(i)),1);		
		}
	}
	
#ifdef TS_MODE
	wayp->ShowWindow(SW_HIDE);
#endif

	m_TeamTypes.SetCurSel(0);
	if(sel>=0) m_TeamTypes.SetCurSel(sel);
	OnSelchangeTeamtypes();
	
	
}

void CTeamTypes::OnSelchangeTeamtypes() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_TeamTypes.GetCurSel()<0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);

	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	m_Aggressive=stob(sec.values["Aggressive"]);	
	m_Annoyance=stob(sec.values["Annoyance"]);
	m_AreTeamMembersRecruitable=stob(sec.values["AreTeamMembersRecruitable"]);	
	m_Autocreate=stob(sec.values["Autocreate"]);	
	m_AvoidThreats=stob(sec.values["AvoidThreats"]);	
	m_Droppod=stob(sec.values["Droppod"]);	
	m_Full=stob(sec.values["Full"]);	
	m_Group=sec.values["Group"];	
	m_GuardSlower=stob(sec.values["GuardSlower"]);
	m_House=TranslateHouse(sec.values["House"], TRUE);	
	m_IonImmune=stob(sec.values["IonImmune"]);	
	m_IsBaseDefense=stob(sec.values["IsBaseDefense"]);	
	m_Loadable=stob(sec.values["Loadable"]);	
	m_LooseRecruit=stob(sec.values["LooseRecruit"]);	
	m_Max=sec.values["Max"];	
	m_Name=sec.values["Name"];	
	m_OnlyTargetHouseEnemy=stob(sec.values["OnlyTargetHouseEnemy"]);	
	m_OnTransOnly=stob(sec.values["OnTransOnly"]);
	m_Prebuild=stob(sec.values["Prebuild"]);	
	m_Priority=sec.values["Priority"];	
	m_Recruiter=stob(sec.values["Recruiter"]);	
	m_Reinforce=stob(sec.values["Reinforce"]);	
	m_Script=(sec.values["Script"]);
	if(ini.sections.find(sec.values["Script"])!=ini.sections.end())
		m_Script+=(" ("+ini.sections[sec.values["Script"]].values["Name"]+")");	
	m_Suicide=stob(sec.values["Suicide"]);	
	if(sec.values.find("Tag")!=sec.values.end())
	{
		m_Tag=sec.values["Tag"];
		if(ini.sections["Tags"].values.find((LPCTSTR)m_Tag)!=ini.sections["Tags"].values.end())
		{
			CString tag=m_Tag;
			m_Tag+=" ";
			m_Tag+=GetParam(ini.sections["Tags"].values[(LPCTSTR)tag], 1);
		}
	}
	else
	{
		m_Tag=GetLanguageStringACP("None");
	}
	m_TaskForce=(sec.values["TaskForce"]);
	if(ini.sections.find(sec.values["TaskForce"])!=ini.sections.end())
		m_TaskForce+=(" ("+ini.sections[sec.values["TaskForce"]].values["Name"]+")");	
	m_TechLevel=sec.values["TechLevel"];
	m_TransportReturnsOnUnload=stob(sec.values["TransportsReturnOnUnload"]);
	m_VeteranLevel=sec.values["VeteranLevel"];
	
	if(yuri_mode)
	m_MindControlDecision=sec.values["MindControlDecision"];
	

	int w=GetWaypoint(sec.values["Waypoint"]);
	char c[50];
	itoa(w,c,10);
	if(w!=-1)
	m_Waypoint=c;
	else
	m_Waypoint="";

#ifdef RA2_MODE
	if(isTrue(sec.values["UseTransportOrigin"]))
	{
		int w=GetWaypoint(sec.values["TransportWaypoint"]);
		char c[50];
		itoa(w,c,10);
		if(w!=-1)
		m_TransportWaypoint=c;
		else
		m_TransportWaypoint="";

	}
	else
		m_TransportWaypoint=TranslateStringACP("None");
#endif

	m_Whiner=stob(sec.values["Whiner"]);

	UpdateData(FALSE);
}

void CTeamTypes::OnChangeName() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CEdit& n=*(CEdit*)GetDlgItem(IDC_NAME);
	DWORD pos=n.GetSel();

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Name"]=m_Name;

	UpdateDialog();

	n.SetSel(pos);
}

void CTeamTypes::OnDeleteteamtype() 
{
	CIniFile& ini=Map->GetIniFile();

	if(m_TeamTypes.GetCurSel()!=-1)
	{
		int res=MessageBox("Are you sure that you want to delete the selected team-type? If you delete it, don´t forget to delete any reference to the team-type.","Delete team-type",MB_YESNO);
		if(res==IDNO) return;

		CString str;
		str=GetText(&m_TeamTypes);
		TruncSpace(str);

		int i;
		CIniFile& ini=Map->GetIniFile();
		for(i=0;i<ini.sections["TeamTypes"].values.size();i++)
		{
			if(strcmp(str, *ini.sections["TeamTypes"].GetValue(i))==NULL)
			{
				ini.sections["TeamTypes"].values.erase(*ini.sections["TeamTypes"].GetValueName(i));
			}
		}
		ini.sections.erase((char*)(LPCTSTR)str);
	}
	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);
	//UpdateDialog();
}

void CTeamTypes::OnEditchangeVeteranlevel() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["VeteranLevel"]=m_VeteranLevel;	
}

void CTeamTypes::OnEditchangeHouse() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["House"]=TranslateHouse(m_House);	
}

void CTeamTypes::OnChangePriority() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Priority"]=m_Priority;
}

void CTeamTypes::OnChangeMax() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Max"]=m_Max;
}

void CTeamTypes::OnEditchangeTechlevel() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["TechLevel"]=m_TechLevel;	
}

void CTeamTypes::OnEditchangeGroup() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	//MessageBox(str);

	sec.values["Group"]=m_Group;	
}

void CTeamTypes::OnEditchangeWaypoint() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	if(strlen(m_Waypoint)==0) sec.values["Waypoint"]="";
	else
		sec.values["Waypoint"]=GetWaypoint(atoi(m_Waypoint));	
}

void CTeamTypes::OnEditchangeScript() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	CString tmp=m_Script;
	TruncSpace(tmp);
	sec.values["Script"]=tmp;	
		
}

void CTeamTypes::OnEditchangeTaskforce() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	CString tmp=m_TaskForce;
	TruncSpace(tmp);
	sec.values["TaskForce"]=tmp;	
}



void CTeamTypes::OnKillfocusVeteranlevel() 
{
	OnEditchangeVeteranlevel();	
}

void CTeamTypes::OnKillfocusHouse() 
{
	OnEditchangeHouse();	
}

void CTeamTypes::OnKillfocusTechlevel() 
{
	OnEditchangeTechlevel();	
}

void CTeamTypes::OnKillfocusGroup() 
{
	OnEditchangeGroup();	
}

void CTeamTypes::OnKillfocusWaypoint() 
{
	OnEditchangeWaypoint();	
}

void CTeamTypes::OnKillfocusScript() 
{
	OnEditchangeScript();	
}

void CTeamTypes::OnKillfocusTaskforce() 
{
	OnEditchangeTaskforce();	
}

void CTeamTypes::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	if(!bShow)
	{
		OnKillfocusGroup();
		OnKillfocusHouse();
		OnKillfocusScript();
		OnKillfocusTaskforce();
		OnKillfocusTechlevel();
		OnKillfocusVeteranlevel();
		OnKillfocusWaypoint();
		OnKillfocusTag();
#ifdef RA2_MODE
		OnKillfocusTransportwaypoint();
		if(yuri_mode) OnKillfocusMindcontroldecision();
#endif
	}
}



void CTeamTypes::OnLoadable() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Loadable"]=btos(m_Loadable);	
}

void CTeamTypes::OnFull() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Full"]=btos(m_Full);	
}

void CTeamTypes::OnAnnoyance() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Annoyance"]=btos(m_Annoyance);	
}

void CTeamTypes::OnGuardslower() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["GuardSlower"]=btos(m_GuardSlower);	
}

void CTeamTypes::OnRecruiter() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Recruiter"]=btos(m_Recruiter);	
}

void CTeamTypes::OnDroppod() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Droppod"]=btos(m_Droppod);	
}

void CTeamTypes::OnWhiner() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Whiner"]=btos(m_Whiner);	
}

void CTeamTypes::OnLooserecruit() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["LooseRecruit"]=btos(m_LooseRecruit);	
}

void CTeamTypes::OnAggressive() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Aggressive"]=btos(m_Aggressive);	
}

void CTeamTypes::OnSuicide() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Suicide"]=btos(m_Suicide);	
}

void CTeamTypes::OnAutocreate() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Autocreate"]=btos(m_Autocreate);	
}

void CTeamTypes::OnPrebuild() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Prebuild"]=btos(m_Prebuild);	
}

void CTeamTypes::OnOntransonly() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["OnTransOnly"]=btos(m_OnTransOnly);	
}

void CTeamTypes::OnReinforce() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["Reinforce"]=btos(m_Reinforce);	
}

void CTeamTypes::OnAvoidthreats() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["AvoidThreats"]=btos(m_AvoidThreats);	
}

void CTeamTypes::OnIonimmune() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["IonImmune"]=btos(m_IonImmune);	
}

void CTeamTypes::OnTransportreturnsonunload() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["TransportsReturnOnUnload"]=btos(m_TransportReturnsOnUnload);	
}

void CTeamTypes::OnAreteammembersrecruitable() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["AreTeamMembersRecruitable"]=btos(m_AreTeamMembersRecruitable);	
}

void CTeamTypes::OnIsbasedefense() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["IsBaseDefense"]=btos(m_IsBaseDefense);	
}

void CTeamTypes::OnOnlytargethouseenemy() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	sec.values["OnlyTargetHouseEnemy"]=btos(m_OnlyTargetHouseEnemy);	
}

CString GetFree(const char* section);

void CTeamTypes::OnNewteamtype() 
{
	CIniFile& ini=Map->GetIniFile();

	CString id=GetFreeID()	;
	CString p;
	p=GetFree("TeamTypes");

	
	ini.sections["TeamTypes"].values[p]=id;
	CIniFileSection& s=ini.sections[id];
	s.values["Name"]="New teamtype";
	s.values["VeteranLevel"]="1";
	s.values["Loadable"]="no";
	s.values["Full"]="yes";
	s.values["Annoyance"]="no";
	s.values["GuardSlower"]="no";
	s.values["Recruiter"]="no";
	s.values["Autocreate"]="yes";
	s.values["Prebuild"]="no";
	s.values["Reinforce"]="no";
	s.values["Droppod"]="no";
	s.values["Whiner"]="no";
	s.values["LooseRecruit"]="no";
	s.values["Aggressive"]="no";
	s.values["Suicide"]="no";
	s.values["Priority"]="5";
	s.values["Max"]="5";
	s.values["TechLevel"]="0";
	s.values["Group"]="-1";
	s.values["OnTransOnly"]="no";
	s.values["AvoidThreats"]="no";
	s.values["IonImmune"]="no";
	s.values["TransportsReturnOnUnload"]="no";
	s.values["AreTeamMembersRecruitable"]="no";
	s.values["IsBaseDefense"]="no";
	s.values["OnlyTargetHouseEnemy"]="no";

#ifdef RA2_MODE
	s.values["UseTransportOrigin"]="no";
	if(yuri_mode) s.values["MindControlDecision"]="0";
	
#endif
		
	
	//UpdateDialog();
	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);

	int i;
	for(i=0;i<m_TeamTypes.GetCount();i++)
	{
		CString k;
		m_TeamTypes.GetLBText(i, k);
		TruncSpace(k);
		if(strcmp(k, id)==NULL)
		{
			m_TeamTypes.SetCurSel(i);
			OnSelchangeTeamtypes();
		}
	}

	CComboBox& houses=*(CComboBox*)GetDlgItem(IDC_HOUSE);
	houses.SetCurSel(0);
	OnEditchangeHouse();
	CComboBox& waypoints=*(CComboBox*)GetDlgItem(IDC_WAYPOINT);
	waypoints.SetCurSel(0);
	CComboBox& script=*(CComboBox*)GetDlgItem(IDC_SCRIPT);
	script.SetCurSel(0);
	CComboBox& taskforce=*(CComboBox*)GetDlgItem(IDC_TASKFORCE);
	taskforce.SetCurSel(0);
	OnKillfocusHouse();
	OnKillfocusWaypoint();
	OnKillfocusScript();
	OnKillfocusTaskforce();
}

void CTeamTypes::OnEditchangeTag() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);

	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	TruncSpace(m_Tag);

	sec.values["Tag"]=m_Tag;			
	if(m_Tag==GetLanguageStringACP("None") || m_Tag.GetLength()==0)
	{
		sec.values.erase("Tag");
	}
}

void CTeamTypes::OnKillfocusTag() 
{
	OnEditchangeTag();	
}

void CTeamTypes::OnEditchangeTransportwaypoint() 
{
#ifndef RA2_MODE
	return;
#endif

	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	if(strlen(m_TransportWaypoint)==0 || isSame(m_TransportWaypoint, TranslateStringACP("None")))
	{
		sec.values.erase("TransportWaypoint");
		sec.values["UseTransportOrigin"]="no";
	}
	else
	{
		sec.values["TransportWaypoint"]=GetWaypoint(atoi(m_TransportWaypoint));	
		sec.values["UseTransportOrigin"]="yes";
	}
	
}

void CTeamTypes::OnKillfocusTransportwaypoint() 
{
	OnEditchangeTransportwaypoint();	
}

void CTeamTypes::OnEditchangeMindcontroldecision() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData(TRUE);
	if(m_TeamTypes.GetCount()==0) return;

	CString str;
	str=GetText(&m_TeamTypes);
	TruncSpace(str);
	CIniFileSection& sec=ini.sections[(char*)(LPCTSTR)str];

	CString tmp=m_MindControlDecision;
	TruncSpace(tmp);
	sec.values["MindControlDecision"]=tmp;	
}

void CTeamTypes::OnKillfocusMindcontroldecision() 
{
	OnEditchangeMindcontroldecision();	
}
