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

// ScriptTypes.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "ScriptTypes.h"
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


#define TMISSION_COUNT 59
#define TPROPERTY_COUNT 4
#define UNLOAD_COUNT 4


// The various missions that a team can have.
enum TeamMissionType {
	TMISSION_ATTACK,
	TMISSION_ATT_WAYPT,
	TMISSION_BERZERK,
	TMISSION_MOVE,
	TMISSION_MOVECELL,
	TMISSION_GUARD,
	TMISSION_LOOP,
	TMISSION_WIN,
	TMISSION_UNLOAD,
	TMISSION_DEPLOY,
	TMISSION_HOUND_DOG,
	TMISSION_DO,
	TMISSION_SET_GLOBAL,
	TMISSION_IDLE_ANIM,
	TMISSION_LOAD,
	TMISSION_SPY,
	TMISSION_PATROL,
	TMISSION_SCRIPT,
	TMISSION_TEAMCHANGE,
	TMISSION_PANIC,
	TMISSION_CHANGE_HOUSE,
	TMISSION_SCATTER,
	TMISSION_GOTO_SHROUD,
	TMISSION_LOSE,
	TMISSION_PLAY_SPEECH,
	TMISSION_PLAY_SOUND,
	TMISSION_PLAY_MOVIE,
	TMISSION_PLAY_MUSIC,
	TMISSION_REDUCE_TIBERIUM,
	TMISSION_BEGIN_PRODUCTION,
	TMISSION_FIRE_SALE,
	TMISSION_SELF_DESTRUCT,
	TMISSION_ION_STORM_START,
	TMISSION_ION_STORM_END,
	TMISSION_CENTER_VIEWPOINT,
	TMISSION_RESHROUD,
	TMISSION_REVEAL,
	TMISSION_DESTROY_MEMBERS,
	TMISSION_CLEAR_GLOBAL,
	TMISSION_SET_LOCAL,
	TMISSION_CLEAR_LOCAL,
	TMISSION_UNPANIC,
	TMISSION_FORCE_FACING,
	TMISSION_FULLY_LOADED,
	TMISSION_UNLOAD_TRUCK,
	TMISSION_LOAD_TRUCK,
	TMISSION_ATTACK_BUILDING_WITH_PROPERTY,
	TMISSION_MOVETO_BUILDING_WITH_PROPERTY,
	TMISSION_SCOUT,
	TMISSION_SUCCESS,
	TMISSION_FLASH,
	TMISSION_PLAY_ANIM,
	TMISSION_TALK_BUBBLE,
	TMISSION_GATHER_FAR,
	TMISSION_GATHER_NEAR,
	TMISSION_ACTIVATE_CURTAIN,
	TMISSION_CHRONO_ATTACK_BUILDING_WITH_PROPERTY,
	TMISSION_CHRONO_ATTACK,
	TMISSION_MOVETO_OWN_BUILDING_WITH_PROPERTY,
};

char const * TMissions[TMISSION_COUNT] = {
	"Attack...",
	"Attack Waypoint...",
	"Go Berzerk",
	"Move to waypoint...",
	"Move to Cell...",
	"Guard area (timer ticks)...",
	"Jump to line #...",
	"Player wins",
	"Unload...",
	"Deploy",
	"Follow friendlies",
	"Do this...",
	"Set global...",
	"Idle Anim...",
	"Load onto Transport",
	"Spy on bldg @ waypt...",
	"Patrol to waypoint...",
	"Change script...",
	"Change team...",
	"Panic",
	"Change house...",
	"Scatter",
	"Goto nearby shroud",
	"Player loses",
	"Play speech...",
	"Play sound...",
	"Play movie...",
	"Play music...",
	"Reduce tiberium",
	"Begin production",
	"Fire sale",
	"Self destruct",
	"Ion storm start in...",
	"Ion storn end",
	"Center view on team (speed)...",
	"Reshroud map",
	"Reveal map",
	"Delete team members",
	"Clear global...",
	"Set local...",
	"Clear local...",
	"Unpanic",
	"Force facing...",
	"Wait till fully loaded",
	"Truck unload",
	"Truck load",
	"Attack enemy building",
	"Moveto enemy building",
	"Scout",
	"Success",
	"Flash",
	"Play Anim",
	"Talk Bubble",
	"Gather at Enemy",
	"Gather at Base",
	"Iron Curtain Me",
	"Chrono Prep for ABwP",
	"Chrono Prep for AQ",
	"Move to own building",
};

char const * TMissionsHelp[TMISSION_COUNT] = {
	"Attack some general target",
	"Attack anything nearby the specified waypoint",
	"Cyborg members of the team will go berzerk.",
	"Orders the team to move to a waypoint on the map",
	"Orders the team to move to a specific cell on the map",
	"Guard an area for a specified amount of time",
	"Move to a new line number in the script.  Used for loops.",
	"Duh",
	"Unloads all loaded units.  The command parameter specifies which units should stay a part of the team, and which should be severed from the team.",
	"Causes all deployable units in the team to deploy",
	"Causes the team to follow the nearest friendly unit",
	"Give all team members the specified mission",
	"Sets a global variable",
	"Causes team members to enter their idle animation",
	"Causes all units to load into transports, if able",
	"**OBSOLETE**",
	"Move to a waypoint while scanning for enemies",
	"Causes the team to start using a new script",
	"Causes the team to switch team types",
	"Causes all units in the team to panic",
	"All units in the team switch houses",
	"Tells all units to scatter",
	"Causes units to flee to a shrouded cell",
	"Causes the player to lose",
	"Plays the specified voice file",
	"Plays the specified sound file",
	"Plays the specified movie file",
	"Plays the specified theme",
	"Reduces the amount of tiberium around team members",
	"Signals the owning house to begin production",
	"Causes an AI house to sell all of its buildings and do a Braveheart",
	"Causes all team members to self destruct",
	"Causes an ion storm to begin at the specified time",
	"Causes an ion storm to end",
	"Center view on team (speed)...",
	"Reshrouds the map",
	"Reveals the map",
	"Delete all members from the team",
	"Clears the specified global variable",
	"Sets the specified local variable",
	"Clears the specified local variable",
	"Causes all team members to stop panicking",
	"Forces team members to face a certain direction",
	"Waits until all transports are full",
	"Causes all trucks to unload their crates (ie, change imagery)",
	"Causes all trucks to load crates (ie, change imagery)",
	"Attack a specific type of building with the specified property",
	"Move to a specific type of building with the specified property",
	"The team will scout the bases of the players that have not been scouted",
	"Record a team as having successfully accomplished its mission.  Used for AI trigger weighting.  Put this at the end of every AITrigger script.",
	"Flashes a team for a period of team.",
	"Plays an anim over every unit in the team.",
	"Displays talk bubble over first unit in the team.",
	"Uses AISafeDistance to find a spot close to enemy's base to gather close.",
	"Gathers outside own base perimeter.",
	"Calls (and waits if nearly ready) for House to deliver Iron Curtain to Team.",
	"Teleports team to Building With Property, but needs similar attack order as next mission.",
	"Teleports team to Attack Quarry, but needs similar attack order as next mission.",
	"A BwP move that will only search through buildings owned by this house.",
};


char const *TargetProperties[TPROPERTY_COUNT] = {
	"Least Threat",
	"Greatest Threat",
	"Nearest",
	"Farthest",
};

char const *UnloadTypeNames[UNLOAD_COUNT] = {
	"Keep Transports, Keep Units",
	"Keep Transports, Lose Units",
	"Lose Transports, Keep Units",
	"Lose Transports, Lose Units",
};


/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CScriptTypes 

IMPLEMENT_DYNCREATE(CScriptTypes, CDialog)

CScriptTypes::CScriptTypes() : CDialog(CScriptTypes::IDD)
{
	//{{AFX_DATA_INIT(CScriptTypes)
	m_Name = _T("");
	//}}AFX_DATA_INIT
}

CScriptTypes::~CScriptTypes()
{
}

void CScriptTypes::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScriptTypes)
	DDX_Control(pDX, IDC_DESCRIPTION, m_DescriptionEx);
	DDX_Control(pDX, IDC_PDESC, m_Desc);
	DDX_Control(pDX, IDC_TYPE, m_Type);
	DDX_Control(pDX, IDC_SCRIPTTYPE, m_ScriptType);
	DDX_Control(pDX, IDC_PARAM, m_Param);
	DDX_Control(pDX, IDC_ACTION, m_Action);
	DDX_Text(pDX, IDC_NAME, m_Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScriptTypes, CDialog)
	//{{AFX_MSG_MAP(CScriptTypes)
	ON_CBN_EDITCHANGE(IDC_SCRIPTTYPE, OnEditchangeScripttype)
	ON_CBN_SELCHANGE(IDC_SCRIPTTYPE, OnSelchangeScripttype)
	ON_LBN_SELCHANGE(IDC_ACTION, OnSelchangeAction)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	ON_CBN_EDITCHANGE(IDC_TYPE, OnEditchangeType)
	ON_CBN_SELCHANGE(IDC_TYPE, OnSelchangeType)
	ON_CBN_EDITCHANGE(IDC_PARAM, OnEditchangeParam)
	ON_CBN_SELCHANGE(IDC_PARAM, OnSelchangeParam)
	ON_BN_CLICKED(IDC_ADDACTION, OnAddaction)
	ON_BN_CLICKED(IDC_DELETEACTION, OnDeleteaction)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CScriptTypes 

void CScriptTypes::UpdateDialog()
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_ScriptType.GetCurSel();

	
	while(m_ScriptType.DeleteString(0)!=CB_ERR);


	// MW 07/24/01: clear dialog
	m_DescriptionEx.SetWindowText("");
	m_Name="";
	m_Param.SetWindowText("");
	m_Action.SetWindowText("");
	m_Type.SetCurSel(-1);

	UpdateData(FALSE);
	

	int i;
	for(i=0;i<ini.sections["ScriptTypes"].values.size();i++)
	{
		CString type,s;
		type=*ini.sections["ScriptTypes"].GetValue(i);
		s=type;
		s+=" (";
		s+=ini.sections[(LPCTSTR)type].values["Name"];
		s+=")";
		m_ScriptType.AddString(s);
	}

	m_ScriptType.SetCurSel(0);
	if(sel>=0) m_ScriptType.SetCurSel(sel);
	OnSelchangeScripttype();


}

void CScriptTypes::OnEditchangeScripttype() 
{
	
}

void CScriptTypes::OnSelchangeScripttype() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Action.GetCurSel();
	while(m_Action.DeleteString(0)!=CB_ERR);

	CString Scripttype;
	if(m_ScriptType.GetCurSel()<0) return;
	m_ScriptType.GetLBText(m_ScriptType.GetCurSel(), Scripttype);
	TruncSpace(Scripttype);
	
	m_Name=ini.sections[(LPCTSTR)Scripttype].values["Name"];

	int count=ini.sections[(LPCTSTR)Scripttype].values.size()-1;
	int i;
	for(i=0;i<count;i++)
	{
		char c[50];
		itoa(i,c,10);
		m_Action.AddString(c);
	}


	m_Action.SetCurSel(0);
	if(sel>=0) m_Action.SetCurSel(sel);
	OnSelchangeAction();
	
	UpdateData(FALSE);
}

void CScriptTypes::OnSelchangeAction() 
{
	CIniFile& ini=Map->GetIniFile();

	CString Scripttype;
	char action[50];
	if(m_ScriptType.GetCurSel()<0) return;
	if(m_Action.GetCurSel()<0) return;
	m_ScriptType.GetLBText(m_ScriptType.GetCurSel(), Scripttype);
	TruncSpace(Scripttype);

	itoa(m_Action.GetCurSel(), action, 10);
	//m_Type.SetWindowText(GetParam(ini.sections[(LPCTSTR)Scripttype].values[action],0));
	m_Type.SetCurSel(atoi(GetParam(ini.sections[(LPCTSTR)Scripttype].values[action], 0)));

	OnSelchangeType();

	m_Param.SetWindowText(GetParam(ini.sections[(LPCTSTR)Scripttype].values[action],1));

	
}

void CScriptTypes::OnChangeName() 
{
	CIniFile& ini=Map->GetIniFile();

	UpdateData();

	CEdit* n=(CEdit*)GetDlgItem(IDC_NAME);

	DWORD pos=n->GetSel();
	CString Scripttype;
	if(m_ScriptType.GetCurSel()<0) return;
	m_ScriptType.GetLBText(m_ScriptType.GetCurSel(), Scripttype);
	TruncSpace(Scripttype);
	


	ini.sections[(LPCTSTR)Scripttype].values["Name"]=m_Name;

	UpdateDialog();
	n->SetSel(pos);
}

void CScriptTypes::OnEditchangeType() 
{
	CIniFile& ini=Map->GetIniFile();

	while(m_Param.DeleteString(0)!=CB_ERR);
	CString Scripttype;
	char action[50];
	if(m_Action.GetCurSel()<0) return;
	if(m_ScriptType.GetCurSel()<0) return;
	m_ScriptType.GetLBText(m_ScriptType.GetCurSel(), Scripttype);
	TruncSpace(Scripttype);

	//CString type;
	//m_Type.GetWindowText(type);
	//TruncSpace(type);
	//MessageBox("beep");
	int type=m_Type.GetCurSel();
	
	int i;
	char tmp[50];

	switch(type)
	{
	case 0:
		ListTargets(m_Param);
		m_Desc.SetWindowText("Target:");
		break;
	case 39:
	case 40:
		ListGlobals(m_Param);
		break;

	case 11:
		ListBehaviours(m_Param);
		break;
	case 1:
	case 3:
	case 16:
		ListWaypoints(m_Param);
		m_Desc.SetWindowText("Waypoint:");
		break;
	case 4:
		m_Desc.SetWindowText("Cell:");
		break;
	case 5:
		m_Desc.SetWindowText("Time units to guard:");
		break;
	case 6:
		m_Desc.SetWindowText("Script action #:");
		while(m_Param.DeleteString(0)!=CB_ERR);
		for(i=1;i<=ini.sections[(LPCTSTR)Scripttype].values.size()-1;i++)
			m_Param.AddString(itoa(i,tmp,10));			
		break;
	case 8:
		m_Desc.SetWindowText("Split groups:");
		while(m_Param.DeleteString(0)!=CB_ERR);
		int i;
		for(i=0;i<UNLOAD_COUNT;i++)
		{
			CString p;
			char c[50];
			itoa(i,c,10);
			p=c;
			p+=" - ";
			p+=UnloadTypeNames[i];

			m_Param.AddString(p);
		}
		break;
	case 9:
	case 14:
	case 37:
		m_Desc.SetWindowText("Use 0:");
		break;
	case 12:
		m_Desc.SetWindowText("Global:");
		break;
	case 20:
		ListHouses(m_Param, TRUE);
		m_Desc.SetWindowText("House:");
		break;
	case 46:
	case 47:
		{
			m_Desc.SetWindowText("Type to move/attack:");

			for(i=0;i<rules.sections["BuildingTypes"].values.size();i++)
			{
				char c[50];
				itoa(i,c,10);
				CString s=c;
				
				s+=" ";
				//s+=rules.sections[*rules.sections["BuildingTypes"].GetValue(i)].values["Name"];
				s+=Map->GetUnitName(*rules.sections["BuildingTypes"].GetValue(i));
				m_Param.AddString(s);
			}
					
			break;
		}

	default:
		m_Desc.SetWindowText("Parameter of action:");
	}
	
	itoa(m_Action.GetCurSel(), action, 10);

	char types[50];
	itoa(type, types, 10);
	ini.sections[(LPCTSTR)Scripttype].values[action]=SetParam(ini.sections[(LPCTSTR)Scripttype].values[action], 0, (LPCTSTR)types);
	
	

}

void CScriptTypes::OnSelchangeType() 
{
	CString str;
	if(m_Type.GetCurSel()>-1)
	{
		//m_Type.GetLBText(m_Type.GetCurSel(), str);
		//m_Type.SetWindowText(str);

		m_DescriptionEx.SetWindowText(TMissionsHelp[m_Type.GetCurSel()]);
	}

	

	OnEditchangeType();
}

void CScriptTypes::OnEditchangeParam() 
{
	CIniFile& ini=Map->GetIniFile();

	CString Scripttype;
	char action[50];
	if(m_Action.GetCurSel()<0) return;
	if(m_ScriptType.GetCurSel()<0) return;
	m_ScriptType.GetLBText(m_ScriptType.GetCurSel(), Scripttype);
	TruncSpace(Scripttype);

	CString param;
	m_Param.GetWindowText(param);
	TruncSpace(param);

	param=TranslateHouse(param);

	itoa(m_Action.GetCurSel(), action, 10);
	ini.sections[(LPCTSTR)Scripttype].values[action]=SetParam(ini.sections[(LPCTSTR)Scripttype].values[action], 1, (LPCTSTR)param);

	
}

void CScriptTypes::OnSelchangeParam() 
{
	m_Param.SetWindowText(GetText(&m_Param));
	OnEditchangeParam();	
}

void CScriptTypes::OnAddaction() 
{
	CIniFile& ini=Map->GetIniFile();

	CString Scripttype;
	if(m_ScriptType.GetCurSel()<0) return;
	m_ScriptType.GetLBText(m_ScriptType.GetCurSel(), Scripttype);
	TruncSpace(Scripttype);


	char action[20];
	int count=ini.sections[(LPCTSTR)Scripttype].values.size()-1;
	itoa(count,action,10);
	ini.sections[(LPCTSTR)Scripttype].values[action]="0,0";

	UpdateDialog();
}

void CScriptTypes::OnDeleteaction() 
{
	CIniFile& ini=Map->GetIniFile();

	CString Scripttype;
	if(m_Action.GetCurSel()<0) return;
	if(m_ScriptType.GetCurSel()<0) return;
	m_ScriptType.GetLBText(m_ScriptType.GetCurSel(), Scripttype);
	TruncSpace(Scripttype);

	
	// okay, action is now the deleted one...
	int i;
	for(i=m_Action.GetCurSel(); i<m_Action.GetCount()-1;i++)
	{
		// okay, now move every action one number up.
		char current[50];
		char next[50];
		
		itoa(i, current, 10);
		itoa(i+1, next, 10);

		ini.sections[(LPCTSTR)Scripttype].values[current]=ini.sections[(LPCTSTR)Scripttype].values[next];
	}
	char last[50];
	itoa(m_Action.GetCount()-1, last, 10);
	ini.sections[(LPCTSTR)Scripttype].values.erase(last);

	UpdateDialog();
}

CString GetFree(const char* section);

void CScriptTypes::OnAdd() 
{
	CIniFile& ini=Map->GetIniFile();

	CString ID=GetFreeID();
	
	CString p=GetFree("ScriptTypes");
	ini.sections["ScriptTypes"].values[p]=ID;
	ini.sections[ID].values["Name"]="New script";
	

	
	int i;
	for(i=0;i<m_ScriptType.GetCount();i++)
	{
		CString data;
		m_ScriptType.GetLBText(i, data);
		TruncSpace(data);

		if(data==ID)
		{
			m_ScriptType.SetCurSel(i);
			OnSelchangeScripttype(); // MW bugfix
			break;
		}
	}

	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);
	//UpdateDialog();
}

void CScriptTypes::OnDelete() 
{
	CIniFile& ini=Map->GetIniFile();
	
	CString Scripttype;
	if(m_ScriptType.GetCurSel()<0) return;
	m_ScriptType.GetLBText(m_ScriptType.GetCurSel(), Scripttype);
	TruncSpace(Scripttype);

	int res=MessageBox("Are you sure to delete this ScriptType? Don´t forget to delete any references to this ScriptType","Delete ScriptType", MB_YESNO | MB_ICONQUESTION);
	if(res!=IDYES) return;

	ini.sections.erase((LPCTSTR)Scripttype);
	ini.sections["ScriptTypes"].values.erase(*ini.sections["ScriptTypes"].GetValueName(ini.sections["ScriptTypes"].FindValue((LPCTSTR)Scripttype)));
	//UpdateDialog();
	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);
}



void CScriptTypes::ListBehaviours(CComboBox &cb)
{
	while(cb.DeleteString(0)!=CB_ERR);

	cb.AddString("0 - Sleep");
	cb.AddString("1 - Attack nearest enemy");
	cb.AddString("2 - Move");
	cb.AddString("3 - QMove");
	cb.AddString("4 - Retreat home for R&R");
	cb.AddString("5 - Guard");
	cb.AddString("6 - Sticky (never recruit)");
	cb.AddString("7 - Enter object");
	cb.AddString("8 - Capture object");
	cb.AddString("9 - Move into & get eaten");
	cb.AddString("10 - Harvest");
	cb.AddString("11 - Area Guard");
	cb.AddString("12 - Return (to refinery)");
	cb.AddString("13 - Stop");
	cb.AddString("14 - Ambush (wait until discovered)");
	cb.AddString("15 - Hunt");
	cb.AddString("16 - Unload");
	cb.AddString("17 - Sabotage (move in & destroy)");
	cb.AddString("18 - Construction");
	cb.AddString("19 - Deconstruction");
	cb.AddString("20 - Repair");
	cb.AddString("21 - Rescue");
	cb.AddString("22 - Missile");
	cb.AddString("23 - Harmless");
	cb.AddString("24 - Open");
	cb.AddString("25 - Patrol");
	cb.AddString("26 - Paradrop approach drop zone");
	cb.AddString("27 - Paradrop overlay drop zone");
	cb.AddString("28 - Wait");
	cb.AddString("29 - Attack move");
	if(yuri_mode)
	{
	//	cb.AddString("30 - Spyplane approach");
	//	cb.AddString("31 - Spyplane retreat");
	}
}

BOOL CScriptTypes::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	while(m_Type.DeleteString(0)!=CB_ERR);

	
	int i;
	for(i=0;i<TMISSION_COUNT;i++)
	{
		CString p;
		char c[50];
		itoa(i,c,10);
		
		//p=c;

		
		//p+=" - ";
		p+=TMissions[i];

		if(strlen(TMissions[i])>0)
		{
			m_Type.AddString(p);
		}
	}

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}
