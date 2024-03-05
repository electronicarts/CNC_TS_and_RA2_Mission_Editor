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

// Houses.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "Houses.h"
#include "resource.h"
#include "finalsundlg.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "newra2housedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CHouses 

IMPLEMENT_DYNCREATE(CHouses, CDialog)

CHouses::CHouses() : CDialog(CHouses::IDD)
{
	//{{AFX_DATA_INIT(CHouses)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

CHouses::~CHouses()
{
}

void CHouses::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHouses)
	DDX_Control(pDX, IDC_HUMANPLAYER, m_HumanPlayer);
	DDX_Control(pDX, IDC_TECHLEVEL, m_TechLevel);
	DDX_Control(pDX, IDC_SIDE, m_Side);
	DDX_Control(pDX, IDC_PLAYERCONTROL, m_PlayerControl);
	DDX_Control(pDX, IDC_PERCENTBUILT, m_PercentBuilt);
	DDX_Control(pDX, IDC_NODECOUNT, m_Nodecount);
	DDX_Control(pDX, IDC_IQ, m_IQ);
	DDX_Control(pDX, IDC_EDGE, m_Edge);
	DDX_Control(pDX, IDC_CREDITS, m_Credits);
	DDX_Control(pDX, IDC_COLOR, m_Color);
	DDX_Control(pDX, IDC_ALLIES, m_Allies);
	DDX_Control(pDX, IDC_ACTSLIKE, m_ActsLike);
	DDX_Control(pDX, IDC_HOUSES, m_houses);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHouses, CDialog)
	//{{AFX_MSG_MAP(CHouses)
	ON_CBN_SELCHANGE(IDC_HOUSES, OnSelchangeHouses)
	ON_BN_CLICKED(IDC_PREPAREHOUSES, OnPreparehouses)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_ADDHOUSE, OnAddhouse)
	ON_BN_CLICKED(IDC_DELETEHOUSE, OnDeletehouse)
	ON_CBN_KILLFOCUS(IDC_IQ, OnKillfocusIq)
	ON_CBN_KILLFOCUS(IDC_EDGE, OnKillfocusEdge)
	ON_CBN_KILLFOCUS(IDC_SIDE, OnKillfocusSide)
	ON_CBN_KILLFOCUS(IDC_COLOR, OnKillfocusColor)
	ON_EN_KILLFOCUS(IDC_ALLIES, OnKillfocusAllies)
	ON_EN_KILLFOCUS(IDC_CREDITS, OnKillfocusCredits)
	ON_CBN_EDITCHANGE(IDC_ACTSLIKE, OnEditchangeActslike)
	ON_CBN_KILLFOCUS(IDC_NODECOUNT, OnKillfocusNodecount)
	ON_CBN_KILLFOCUS(IDC_TECHLEVEL, OnKillfocusTechlevel)
	ON_CBN_KILLFOCUS(IDC_PERCENTBUILT, OnKillfocusPercentbuilt)
	ON_CBN_KILLFOCUS(IDC_PLAYERCONTROL, OnKillfocusPlayercontrol)
	ON_CBN_SELCHANGE(IDC_HUMANPLAYER, OnSelchangeHumanplayer)
	ON_CBN_SELCHANGE(IDC_ACTSLIKE, OnSelchangeActslike)
	ON_EN_SETFOCUS(IDC_ALLIES, OnSetfocusAllies)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CHouses 

void CHouses::UpdateDialog()
{
	while(this->m_houses.DeleteString(0)!=CB_ERR);
	while(this->m_HumanPlayer.DeleteString(0)!=CB_ERR);
	while(this->m_Color.DeleteString(0)!=CB_ERR);
	while(this->m_ActsLike.DeleteString(0)!=CB_ERR);

	ListHouses(m_Side, FALSE, TRUE);

	int i;
	for(i=0;i<rules.sections[HOUSES].values.size();i++)
	{
#ifdef RA2_MODE
		CString j=*rules.sections[HOUSES].GetValue(i);
		j.MakeLower();
		if(j=="nod" || j=="gdi") continue;
#endif
				
		char house_id[5];
		CString houseCString;
		itoa(i,house_id,10);
		houseCString=house_id;
		houseCString+=" ";
		houseCString+=TranslateHouse(*rules.sections[HOUSES].GetValue(i), TRUE);
		m_ActsLike.AddString(houseCString);
	}

	CIniFile& ini=Map->GetIniFile();

	if(ini.sections.find(MAPHOUSES)==ini.sections.end() && ini.sections.size()>0)
	{
		// MessageBox("No houses do exist, if you want to use houses, you should use ""Prepare houses"" before doing anything else.");
	}
	else
	{
		m_HumanPlayer.AddString("None");
		m_HumanPlayer.SetCurSel(0);
		for(i=0;i<ini.sections[MAPHOUSES].values.size();i++)
		{
#ifdef RA2_MODE
			CString j=*ini.sections[MAPHOUSES].GetValue(i);
			j.MakeLower();
			if(j=="nod" || j=="gdi") continue;
#endif	
			m_houses.AddString(TranslateHouse(*ini.sections[MAPHOUSES].GetValue(i), TRUE));
			m_HumanPlayer.AddString(TranslateHouse(*ini.sections[MAPHOUSES].GetValue(i), TRUE));
			
		}

		
		if(ini.sections["Basic"].values.find("Player")!=ini.sections["Basic"].values.end())
		{
			m_HumanPlayer.SetCurSel(m_HumanPlayer.FindStringExact(0, TranslateHouse(ini.sections["Basic"].values["Player"], TRUE)));
			
		}

		m_houses.SetCurSel(0);
		
		m_ActsLike.SetWindowText("");
		m_Allies.SetWindowText("");
		m_Color.SetWindowText("");
		m_Credits.SetWindowText("");
		m_Edge.SetWindowText("");
		m_IQ.SetWindowText("");
		m_Nodecount.SetWindowText("");
		m_PercentBuilt.SetWindowText("");
		m_PlayerControl.SetWindowText("");
		m_Side.SetWindowText("");
		m_TechLevel.SetWindowText("");

		

		OnSelchangeHouses();
	}

	// houses list done

	
	// ok now color list
	const auto& rulesColors = rules.sections["Colors"];
	for(i=0;i< rulesColors.values.size();i++)
	{
		m_Color.AddString(*rulesColors.GetValueName(i));
	}
	for (i = 0;i < ini.sections["Colors"].values.size();i++)
	{
		auto col = *ini.sections["Colors"].GetValueName(i);
		if(rulesColors.values.find(col) == rulesColors.values.end())
			m_Color.AddString(*ini.sections["Colors"].GetValueName(i));
	}
}

BOOL CHouses::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateStrings();
	UpdateDialog();

#ifdef RA2_MODE
	m_ActsLike.ShowWindow(SW_HIDE);
	//m_Edge.ShowWindow(SW_HIDE);
#endif
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CHouses::OnSelchangeHouses() 
{
	CIniFile& ini=Map->GetIniFile();
	
	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);

	name=TranslateHouse(name);

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	// ListHouses(m_ActsLike, TRUE);

#ifndef RA2_MODE
	m_ActsLike.SetWindowText(s.values["ActsLike"]);
#endif
	m_Allies.SetWindowText(TranslateHouse(s.values["Allies"], TRUE));
	m_Color.SetWindowText(s.values["Color"]);
	m_Credits.SetWindowText(s.values["Credits"]);
	m_Edge.SetWindowText(s.values["Edge"]);
	m_IQ.SetWindowText(s.values["IQ"]);
	m_Nodecount.SetWindowText(s.values["NodeCount"]);
	m_PercentBuilt.SetWindowText(s.values["PercentBuilt"]);
	m_PlayerControl.SetWindowText(s.values["PlayerControl"]);
#ifndef RA2_MODE
	m_Side.SetWindowText(s.values["Side"]);
#else
	m_Side.SetWindowText(TranslateHouse(s.values["Country"], TRUE));
#endif
	m_TechLevel.SetWindowText(s.values["TechLevel"]);

}

void CHouses::OnPreparehouses() 
{
	CIniFile& ini=Map->GetIniFile();

#ifdef RA2_MODE
	if(Map->IsMultiplayer())
	{
		ini.sections["Basic"].values["MultiplayerOnly"]="1";

		int i;
		for (i=0;i<rules.sections[HOUSES].values.size();i++)
		{
			char c[50];
			int k=i;
			itoa(k,c,10);
			CString country=*rules.sections[HOUSES].GetValue(i);

			// we now create a MAPHOUSE with the same name as the current rules house
			ini.sections[MAPHOUSES].values[c]=country;
			
			ini.sections[country].values["IQ"]="0";
			ini.sections[country].values["Edge"]="North";
			ini.sections[country].values["Color"]=rules.sections[country].values["Color"];
			ini.sections[country].values["Allies"]=country;
			ini.sections[country].values["Country"]=country;
			ini.sections[country].values["Credits"]="0";
			ini.sections[country].values["NodeCount"]="0";
			ini.sections[country].values["TechLevel"]="1";
			ini.sections[country].values["PercentBuilt"]="0";
			ini.sections[country].values["PlayerControl"]="no";

		}
		
		UpdateDialog();
		return;
	}
#endif

	// import the rules.ini houses
	if(ini.sections.find(MAPHOUSES)!=ini.sections.end())
	{
		if(ini.sections[MAPHOUSES].values.size()>0)
		{
			MessageBox("There are already houses in your map. You need to delete these first.");
			return;
		}
	}

	int i;
	for(i=0;i<rules.sections[HOUSES].values.size();i++)
	{
		AddHouse(GetHouseSectionName(*rules.sections[HOUSES].GetValue(i)));
	}
}

void CHouses::AddHouse(const char *name)
{
	CIniFile& ini=Map->GetIniFile();

	if(ini.sections.find(name)!=ini.sections.end())
	{
		MessageBox(((CString)"Sorry this name is not available. " + name + (CString)" is already used in the map file. You need to use another name."));
		return;
	}
	if(ini.sections.find(TranslateHouse(name))!=ini.sections.end())
	{
		MessageBox(((CString)"Sorry this name is not available. " + name + (CString)" is already used in the map file. You need to use another name."));
		return;
	}
#ifdef RA2_MODE
	CNewRA2HouseDlg dlg;
	if(dlg.DoModal()==IDCANCEL) return;
#endif

	int c;
	
	//okay, get a free slot
	int pos=-1;
#ifdef RA2_MODE
	int pos2=-1;
#endif
	for(c=0;c>-1;c++)
	{
		char k[50];
		itoa(c,k,10);
		if(ini.sections[MAPHOUSES].values.find(k)==ini.sections[MAPHOUSES].values.end())
			pos=c;
		if(pos!=-1) break;
	}
#ifdef RA2_MODE
	for(c=0;c>-1;c++)
	{
		char k[50];
		itoa(c,k,10);
		if(ini.sections[HOUSES].values.find(k)==ini.sections[HOUSES].values.end())
			pos2=c;
		if(pos2!=-1) break;
	}
#endif
	
	char k[50];
	itoa(pos,k,10);
	
	ini.sections[MAPHOUSES].values[k]=TranslateHouse(name);

	CString country;
	country=name; 
	country.Replace(" House", "");
	country.Replace("House","");
	if(country.Find(" ")>=0) country.Replace(" ", "_"); //=country.Left(country.Find(" "));

#ifdef RA2_MODE
	itoa(pos2, k, 10);
	ini.sections[HOUSES].values[k]=country;
#endif
	
	ini.sections[TranslateHouse(name)].values["IQ"]="0";
	ini.sections[TranslateHouse(name)].values["Edge"]="West";
	ini.sections[TranslateHouse(name)].values["Allies"]=TranslateHouse(name);

	CString side=name;
#ifdef RA2_MODE
	side=rules.sections[TranslateHouse(dlg.m_Country)].values["Side"];
#endif

	if(strstr(name, "Nod")!=NULL)
	{
#ifndef RA2_MODE
		ini.sections[TranslateHouse(name)].values["Side"]="Nod";
#endif
		ini.sections[TranslateHouse(name)].values["Color"]="DarkRed";
		if(name!="Nod") ini.sections[name].values["Allies"]+=",Nod";
	}
	else
	{
#ifndef RA2_MODE
		ini.sections[TranslateHouse(name)].values["Side"]="GDI";
#endif
		ini.sections[TranslateHouse(name)].values["Color"]="Gold";
		if(name!="GDI") ini.sections[TranslateHouse(name)].values["Allies"]+=",GDI";
	}
	ini.sections[TranslateHouse(name)].values["Credits"]="0";
#ifndef RA2_MODE
	ini.sections[TranslateHouse(name)].values["ActsLike"]="0";
#else
	ini.sections[TranslateHouse(name)].values["Country"]=TranslateHouse(country);
#endif
	ini.sections[TranslateHouse(name)].values["NodeCount"]="0";
	ini.sections[TranslateHouse(name)].values["TechLevel"]="10";
	ini.sections[TranslateHouse(name)].values["PercentBuilt"]="100";
	ini.sections[TranslateHouse(name)].values["PlayerControl"]="no";

#ifdef RA2_MODE
	dlg.m_Country=TranslateHouse(dlg.m_Country); // just to make sure...
	country=TranslateHouse(country);
    ini.sections[country].values["ParentCountry"]=dlg.m_Country;
	ini.sections[country].values["Name"]=country;
	ini.sections[country].values["Suffix"]=rules.sections[dlg.m_Country].values["Suffix"];
	ini.sections[country].values["Prefix"]=rules.sections[dlg.m_Country].values["Prefix"];
	ini.sections[country].values["Color"]=rules.sections[dlg.m_Country].values["Color"];
	ini.sections[country].values["Side"]=rules.sections[dlg.m_Country].values["Side"];
	ini.sections[country].values["SmartAI"]=rules.sections[dlg.m_Country].values["SmartAI"];
	ini.sections[country].values["CostUnitsMult"]="1";
#endif

	int cusel=m_houses.GetCurSel();
	UpdateDialog();
	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs();
	if(cusel!=-1)m_houses.SetCurSel(cusel);
}

void CHouses::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	CIniFile& ini=Map->GetIniFile();
	
	if(bShow)
	{
		if(ini.sections.find(MAPHOUSES)==ini.sections.end() && ini.sections.size()>0)
		{
#ifndef RA2_MODE
			 MessageBox("No houses do exist, if you want to use houses, you should use ""Prepare houses"" before doing anything else. Note that in a multiplayer map independent computer players cannot be created by using the names GDI and Nod for the house. Just use something like GDI_AI.");
#else
			 MessageBox("No houses do exist, if you want to use houses, you should use ""Prepare houses"" before doing anything else.");

#endif
		}
	}
		else
		{
			// call all KillFocus !
			OnKillfocusIq();
			OnEditchangeActslike();
			OnKillfocusAllies();
			OnKillfocusColor();
			OnKillfocusCredits();
			OnKillfocusEdge();
			OnKillfocusNodecount();
			OnKillfocusPercentbuilt();
			OnKillfocusPlayercontrol();
			OnKillfocusSide();
			OnKillfocusTechlevel();
		}
	
}

void CHouses::OnAddhouse() 
{
	CString name=InputBox(GetLanguageStringACP("AddHouse"),GetLanguageStringACP("AddHouseCap"));
	if(name.GetLength()==0) return;
	
	name=GetHouseSectionName(name);
	//name=TranslateHouse(name);
	AddHouse(name);
}

void CHouses::OnDeletehouse() 
{
	CIniFile& ini=Map->GetIniFile();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	CString uiname;
	m_houses.GetLBText(cusel, name);

	uiname=name;
	name=TranslateHouse(name);

	CString str=GetLanguageStringACP("DeleteHouse");
	str=TranslateStringVariables(1, str, uiname);
	if(MessageBox(str,GetLanguageStringACP("DeleteHouseCap"),MB_YESNO)==IDNO) return;

	ini.sections.erase((LPCTSTR)name);

	int i;
	for(i=0;i<ini.sections[MAPHOUSES].values.size();i++)
	{
		if(*ini.sections[MAPHOUSES].GetValue(i)==name)
		{
			ini.sections[MAPHOUSES].values.erase(*ini.sections[MAPHOUSES].GetValueName(i));
		}
	}

	if(ini.sections[MAPHOUSES].values.size()==0)
		ini.sections.erase(MAPHOUSES);

	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs();
	UpdateDialog();
}

void CHouses::OnKillfocusIq() 
{
	CIniFile& ini=Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_IQ.GetWindowText(t);
	s.values["IQ"]=t;
}

void CHouses::OnKillfocusEdge() 
{
	CIniFile& ini=Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_Edge.GetWindowText(t);
	s.values["Edge"]=t;	
}

void CHouses::OnKillfocusSide() 
{
	CIniFile& ini=Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_Side.GetWindowText(t);
	t=TranslateHouse(t);
#ifndef RA2_MODE
	s.values["Side"]=t;	
#else
	s.values["Country"]=t;
#endif
}

void CHouses::OnKillfocusColor() 
{
	CIniFile& ini=Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_Color.GetWindowText(t);
	s.values["Color"]=t;
	
	//Map->UpdateIniFile(MAPDATA_UPDATE_FROM_INI);
	// MW fix: Only update structures
	// this recalculates the colors
	Map->UpdateStructures(FALSE);
	// and minimap
	Map->RedrawMinimap();
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->RedrawWindow();
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_minimap.RedrawWindow();
}

void CHouses::OnKillfocusAllies() 
{
	CIniFile& ini=Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_Allies.GetWindowText(t);
	t=TranslateHouse(t);
	s.values["Allies"]=t;	
}

void CHouses::OnKillfocusCredits() 
{
	CIniFile& ini=Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_Credits.GetWindowText(t);
	s.values["Credits"]=t;	
}

void CHouses::OnEditchangeActslike() 
{
	CIniFile& ini=Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_ActsLike.GetWindowText(t);
	TruncSpace(t);
	t=TranslateHouse(t);
	s.values["ActsLike"]=t;	
}

void CHouses::OnKillfocusNodecount() 
{
	CIniFile& ini=Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_Nodecount.GetWindowText(t);
	s.values["NodeCount"]=t;	
}

void CHouses::OnKillfocusTechlevel() 
{
	CIniFile& ini=Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_TechLevel.GetWindowText(t);
	s.values["TechLevel"]=t;	
}

void CHouses::OnKillfocusPercentbuilt() 
{
	CIniFile& ini=Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_PercentBuilt.GetWindowText(t);
	s.values["PercentBuilt"]=t;	
}

void CHouses::OnKillfocusPlayercontrol() 
{
	CIniFile& ini=Map->GetIniFile();

	SetMainStatusBarReady();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_PlayerControl.GetWindowText(t);
	s.values["PlayerControl"]=t;	
}

void CHouses::OnSelchangeHumanplayer() 
{
	CIniFile& ini=Map->GetIniFile();

	CString pl;
	m_HumanPlayer.GetLBText(m_HumanPlayer.GetCurSel(),pl);
	pl=TranslateHouse(pl);

	if(pl.GetLength()==0 || pl=="None")
	{
		ini.sections["Basic"].values.erase("Player");
	}
	else
	{
		ini.sections["Basic"].values["Player"]=(LPCTSTR)pl;
	}
}

void CHouses::OnSelchangeActslike() 
{
	CIniFile& ini=Map->GetIniFile();

	int cusel;
	cusel=m_houses.GetCurSel();
	if(cusel==-1) return;

	CString name;
	m_houses.GetLBText(cusel, name);
	name=TranslateHouse(name);

	

	CIniFileSection& s=ini.sections[(LPCTSTR)name];

	CString t;
	m_ActsLike.GetLBText(m_ActsLike.GetCurSel(),t);
	TruncSpace(t);
	t=TranslateHouse(t);
	s.values["ActsLike"]=t;		
}

void CHouses::UpdateStrings()
{
	SetDlgItemText(IDC_DESC, GetLanguageStringACP("HousesDesc"));
	SetDlgItemText(IDC_LPLAYER, GetLanguageStringACP("HousesPlayerHouse"));
	SetDlgItemText(IDC_LHOUSE, GetLanguageStringACP("HousesHouse"));
	SetDlgItemText(IDC_LIQ, GetLanguageStringACP("HousesIQ"));
	SetDlgItemText(IDC_LEDGE, GetLanguageStringACP("HousesEdge"));
	SetDlgItemText(IDC_LSIDE, GetLanguageStringACP("HousesSide"));
	SetDlgItemText(IDC_LCOLOR, GetLanguageStringACP("HousesColor"));
	SetDlgItemText(IDC_LALLIES, GetLanguageStringACP("HousesAllies"));
	SetDlgItemText(IDC_LCREDITS, GetLanguageStringACP("HousesCredits"));
	SetDlgItemText(IDC_LACTSLIKE, GetLanguageStringACP("HousesActsLike"));
	SetDlgItemText(IDC_LNODECOUNT, GetLanguageStringACP("HousesNodeCount"));
	SetDlgItemText(IDC_LTECHLEVEL, GetLanguageStringACP("HousesTechlevel"));
	SetDlgItemText(IDC_LBUILDACTIVITY, GetLanguageStringACP("HousesBuildActivity"));
	SetDlgItemText(IDC_LPLAYERCONTROL, GetLanguageStringACP("HousesPlayerControl"));

	SetDlgItemText(IDC_PREPAREHOUSES, GetLanguageStringACP("HousesPrepareHouses"));
	SetDlgItemText(IDC_ADDHOUSE, GetLanguageStringACP("HousesAddHouse"));
	SetDlgItemText(IDC_DELETEHOUSE, GetLanguageStringACP("HousesDeleteHouse"));

	SetWindowText(TranslateStringACP(HOUSES));
}

void CHouses::OnSetfocusAllies() 
{
	SetMainStatusBar(GetLanguageStringACP("HousesAlliesHelp"));	
}

void CHouses::PostNcDestroy() 
{
	CDialog::PostNcDestroy();
}
