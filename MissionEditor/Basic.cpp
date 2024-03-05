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

// Basic.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "Basic.h"
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
// Eigenschaftenseite CBasic 

IMPLEMENT_DYNCREATE(CBasic, CPropertyPage)

CBasic::CBasic() : CDialog(CBasic::IDD)
{
	//{{AFX_DATA_INIT(CBasic)
	//}}AFX_DATA_INIT
}

CBasic::~CBasic()
{
}

void CBasic::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBasic)
	DDX_Control(pDX, IDC_REQUIREDADDON, m_RequiredAddOn);
	DDX_Control(pDX, IDC_VEINGROWTHENABLED, m_VeinGrowthEnabled);
	DDX_Control(pDX, IDC_TRUCKCRATE, m_TruckCrate);
	DDX_Control(pDX, IDC_TRAINCRATE, m_TrainCrate);
	DDX_Control(pDX, IDC_TIBERIUMGROWTHENABLED, m_TiberiumGrowthEnabled);
	DDX_Control(pDX, IDC_TIBERIUMDEATHTOVISCEROID, m_TiberiumDeathToVisceroid);
	DDX_Control(pDX, IDC_SKIPSCORE, m_SkipScore);
	DDX_Control(pDX, IDC_SKIPMAPSELECT, m_SkipMapSelect);
	DDX_Control(pDX, IDC_ONETIMEONLY, m_OneTimeOnly);
	DDX_Control(pDX, IDC_OFFICIAL, m_Official);
	DDX_Control(pDX, IDC_NEXTSCENARIO, m_NextScenario);
	DDX_Control(pDX, IDC_PERCENT, m_Percent);
	DDX_Control(pDX, IDC_NEWINIFORMAT, m_NewINIFormat);
	DDX_Control(pDX, IDC_NAME, m_Name);
	DDX_Control(pDX, IDC_MULTIPLAYERONLY, m_MultiplayerOnly);
	DDX_Control(pDX, IDC_INITTIME, m_InitTime);
	DDX_Control(pDX, IDC_IGNOREGLOBALAITRIGGERS, m_IgnoreGlobalAITriggers);
	DDX_Control(pDX, IDC_ICEGROWTHENABLED, m_IceGrowthEnabled);
	DDX_Control(pDX, IDC_FREERADAR, m_FreeRadar);
	DDX_Control(pDX, IDC_ENDOFGAME, m_EndOfGame);
	DDX_Control(pDX, IDC_CARRYOVERCAP, m_CarryOverCap);
	DDX_Control(pDX, IDC_ALTNEXTSCENARIO, m_AltNextScenario);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBasic, CDialog)
	//{{AFX_MSG_MAP(CBasic)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	ON_CBN_EDITCHANGE(IDC_NEXTSCENARIO, OnEditchangeNextscenario)
	ON_CBN_EDITCHANGE(IDC_ALTNEXTSCENARIO, OnEditchangeAltnextscenario)
	ON_EN_CHANGE(IDC_NEWINIFORMAT, OnChangeNewiniformat)
	ON_EN_CHANGE(IDC_CARRYOVERCAP, OnChangeCarryovercap)
	ON_CBN_EDITCHANGE(IDC_ENDOFGAME, OnEditchangeEndofgame)
	ON_CBN_EDITCHANGE(IDC_SKIPSCORE, OnEditchangeSkipscore)
	ON_CBN_EDITCHANGE(IDC_ONETIMEONLY, OnEditchangeOnetimeonly)
	ON_CBN_EDITCHANGE(IDC_SKIPMAPSELECT, OnEditchangeSkipmapselect)
	ON_CBN_EDITCHANGE(IDC_OFFICIAL, OnEditchangeOfficial)
	ON_CBN_EDITCHANGE(IDC_IGNOREGLOBALAITRIGGERS, OnEditchangeIgnoreglobalaitriggers)
	ON_CBN_EDITCHANGE(IDC_TRUCKCRATE, OnEditchangeTruckcrate)
	ON_CBN_EDITCHANGE(IDC_TRAINCRATE, OnEditchangeTraincrate)
	ON_EN_CHANGE(IDC_PERCENT, OnChangePercent)
	ON_EN_CHANGE(IDC_MULTIPLAYERONLY, OnChangeMultiplayeronly)
	ON_CBN_EDITCHANGE(IDC_TIBERIUMGROWTHENABLED, OnEditchangeTiberiumgrowthenabled)
	ON_CBN_EDITCHANGE(IDC_VEINGROWTHENABLED, OnEditchangeVeingrowthenabled)
	ON_CBN_EDITCHANGE(IDC_ICEGROWTHENABLED, OnEditchangeIcegrowthenabled)
	ON_CBN_EDITCHANGE(IDC_TIBERIUMDEATHTOVISCEROID, OnEditchangeTiberiumdeathtovisceroid)
	ON_CBN_EDITCHANGE(IDC_FREERADAR, OnEditchangeFreeradar)
	ON_EN_CHANGE(IDC_INITTIME, OnChangeInittime)
	ON_CBN_EDITCHANGE(IDC_REQUIREDADDON, OnEditchangeRequiredaddon)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CBasic 

//DEL void CBasic::OnOK() 
//DEL {
//DEL 	// TODO: Zusätzliche Prüfung hier einfügen
//DEL 	
//DEL 	CPropertyPage::OnOK();
//DEL }

void CBasic::UpdateDialog()
{
	CIniFile& ini=Map->GetIniFile();

	m_AltNextScenario.SetWindowText(ini.sections["Basic"].values["AltNextScenario"]);
	m_Name.SetWindowText(ini.sections["Basic"].values["Name"]);
	m_CarryOverCap.SetWindowText(ini.sections["Basic"].values["CarryOverCap"]);
	m_EndOfGame.SetWindowText(ini.sections["Basic"].values["EndOfGame"]);
	m_FreeRadar.SetWindowText(ini.sections["Basic"].values["FreeRadar"]);
	m_IceGrowthEnabled.SetWindowText(ini.sections["Basic"].values["IceGrowthEnabled"]);
	m_IgnoreGlobalAITriggers.SetWindowText(ini.sections["Basic"].values["IgnoreGlobalAITriggers"]);
	m_InitTime.SetWindowText(ini.sections["Basic"].values["InitTime"]);
	m_MultiplayerOnly.SetWindowText(ini.sections["Basic"].values["MultiplayerOnly"]);
	m_NewINIFormat.SetWindowText(ini.sections["Basic"].values["NewINIFormat"]);
	m_NextScenario.SetWindowText(ini.sections["Basic"].values["NextScenario"]);
	m_Official.SetWindowText(ini.sections["Basic"].values["Official"]);
	m_OneTimeOnly.SetWindowText(ini.sections["Basic"].values["OneTimeOnly"]);
	m_Percent.SetWindowText(ini.sections["Basic"].values["Percent"]);
	m_SkipMapSelect.SetWindowText(ini.sections["Basic"].values["SkipMapSelect"]);
	m_SkipScore.SetWindowText(ini.sections["Basic"].values["SkipScore"]);
	m_TiberiumDeathToVisceroid.SetWindowText(ini.sections["Basic"].values["TiberiumDeathToVisceroid"]);
	m_TiberiumGrowthEnabled.SetWindowText(ini.sections["Basic"].values["TiberiumGrowthEnabled"]);
	m_TrainCrate.SetWindowText(ini.sections["Basic"].values["TrainCrate"]);
	m_TruckCrate.SetWindowText(ini.sections["Basic"].values["TruckCrate"]);
	m_VeinGrowthEnabled.SetWindowText(ini.sections["Basic"].values["VeinGrowthEnabled"]);

	if(ini.sections["Basic"].values.find("RequiredAddOn")!=ini.sections["Basic"].values.end())
	{
		m_RequiredAddOn.SetWindowText(ini.sections["Basic"].values["RequiredAddOn"]);
	}
	else
		m_RequiredAddOn.SetWindowText("0");

}

void CBasic::OnChangeName() 
{
	CIniFile& ini=Map->GetIniFile();
 	ini.sections["Basic"].values["Name"]=GetText(&m_Name);
}

void CBasic::UpdateData()
{
	// MessageBox("This function ( UpdateData() ) should not be called here... please report to the author.");
	errstream << "CBasic::UpdateData() called - should not be called" << endl;
	errstream.flush();
}


void CBasic::OnEditchangeNextscenario() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["NextScenario"]=GetText(&m_NextScenario);	
}

void CBasic::OnEditchangeAltnextscenario() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["AltNextScenario"]=GetText(&m_AltNextScenario);	
}

void CBasic::OnChangeNewiniformat() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["NewINIFormat"]=GetText(&m_NewINIFormat);
}

void CBasic::OnChangeCarryovercap() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["CarryOverCap"]=GetText(&m_CarryOverCap);
}

void CBasic::OnEditchangeEndofgame() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["EndOfGame"]=GetText(&m_EndOfGame);	
}

void CBasic::OnEditchangeSkipscore() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["SkipScore"]=GetText(&m_SkipScore);	
}

void CBasic::OnEditchangeOnetimeonly() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["OneTimeOnly"]=GetText(&m_OneTimeOnly);	
}

void CBasic::OnEditchangeSkipmapselect() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["SkipMapSelect"]=GetText(&m_SkipMapSelect);	
}

void CBasic::OnEditchangeOfficial() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["Official"]=GetText(&m_Official);	
}

void CBasic::OnEditchangeIgnoreglobalaitriggers() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["IgnoreGlobalAITriggers"]=GetText(&m_IgnoreGlobalAITriggers);	
}

void CBasic::OnEditchangeTruckcrate() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["TruckCrate"]=GetText(&m_TruckCrate);	
}

void CBasic::OnEditchangeTraincrate() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["TrainCrate"]=GetText(&m_TrainCrate);
}

void CBasic::OnChangePercent() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["Percent"]=GetText(&m_Percent);
}

void CBasic::OnChangeMultiplayeronly() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["MultiplayerOnly"]=GetText(&m_MultiplayerOnly);	
}

void CBasic::OnEditchangeTiberiumgrowthenabled() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["TiberiumGrowthEnabled"]=GetText(&m_TiberiumGrowthEnabled);
}

void CBasic::OnEditchangeVeingrowthenabled() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["VeinGrowthEnabled"]=GetText(&m_VeinGrowthEnabled);	
}

void CBasic::OnEditchangeIcegrowthenabled() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["IceGrowthEnabled"]=GetText(&m_IceGrowthEnabled);	
}

void CBasic::OnEditchangeTiberiumdeathtovisceroid() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["TiberiumDeathToVisceroid"]=GetText(&m_TiberiumDeathToVisceroid);	
}

void CBasic::OnEditchangeFreeradar() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["FreeRadar"]=GetText(&m_FreeRadar);
}

void CBasic::OnChangeInittime() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["InitTime"]=GetText(&m_InitTime);
}

void CBasic::UpdateStrings()
{
	SetDlgItemText(IDC_LDESC, GetLanguageStringACP("BasicDesc"));
	SetDlgItemText(IDC_LNAME, GetLanguageStringACP("BasicName"));
	SetDlgItemText(IDC_LNEXTSCENARIO, GetLanguageStringACP("BasicNextScenario"));
	SetDlgItemText(IDC_LALTNEXTSCENARIO, GetLanguageStringACP("BasicAltNextScenario"));
	SetDlgItemText(IDC_LNEWINIFORMAT, GetLanguageStringACP("BasicNewIniFormat"));
	SetDlgItemText(IDC_LCARRYOVERCAP, GetLanguageStringACP("BasicCarryOverCap"));
	SetDlgItemText(IDC_LENDOFGAME, GetLanguageStringACP("BasicEndOfGame"));
	SetDlgItemText(IDC_LSKIPSCORE, GetLanguageStringACP("BasicSkipScore"));
	SetDlgItemText(IDC_LONETIMEONLY, GetLanguageStringACP("BasicOneTimeOnly"));
	SetDlgItemText(IDC_LSKIPMAPSELECT, GetLanguageStringACP("BasicSkipMapSelect"));
	SetDlgItemText(IDC_LOFFICIAL, GetLanguageStringACP("BasicOfficial"));
	SetDlgItemText(IDC_LIGNOREGLOBALAITRIGGERS, GetLanguageStringACP("BasicIgnoreGlobalAITriggers"));
	SetDlgItemText(IDC_LTRUCKCRATE, GetLanguageStringACP("BasicTruckCrate"));
	SetDlgItemText(IDC_LTRAINCRATE, GetLanguageStringACP("BasicTrainCrate"));
	SetDlgItemText(IDC_LPERCENT, GetLanguageStringACP("BasicPercent"));
	SetDlgItemText(IDC_LMULTIPLAYERONLY, GetLanguageStringACP("BasicMultiplayerOnly"));
	SetDlgItemText(IDC_LTIBERIUMGROWTHENABLED, GetLanguageStringACP("BasicTiberiumGrowthEnabled"));
	SetDlgItemText(IDC_LVEINGROWTHENABLED, GetLanguageStringACP("BasicVeinGrowthEnabled"));
	SetDlgItemText(IDC_LICEGROWTHENABLED, GetLanguageStringACP("BasicIceGrowthEnabled"));
	SetDlgItemText(IDC_LTIBERIUMDEATHTOVISCEROID, GetLanguageStringACP("BasicTiberiumDeathToVisceroid"));
	SetDlgItemText(IDC_LFREERADAR, GetLanguageStringACP("BasicFreeRadar"));
	SetDlgItemText(IDC_LINITIME, GetLanguageStringACP("BasicInitTime"));
	SetDlgItemText(IDC_LADDONNEEDED, GetLanguageStringACP("BasicAddOnNeeded"));



#ifdef RA2_MODE
	GetDlgItem(IDC_LVEINGROWTHENABLED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_VEINGROWTHENABLED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LTRAINCRATE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_TRAINCRATE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_ICEGROWTHENABLED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LICEGROWTHENABLED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_TIBERIUMDEATHTOVISCEROID)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LTIBERIUMDEATHTOVISCEROID)->ShowWindow(SW_HIDE);


#endif

	SetWindowText(TranslateStringACP("Basic"));
}

void CBasic::OnEditchangeRequiredaddon() 
{
	CIniFile& ini=Map->GetIniFile();
	ini.sections["Basic"].values["RequiredAddOn"]=GetText(&m_RequiredAddOn);	
	if(ini.sections["Basic"].values["RequiredAddOn"]=="0") ini.sections["Basic"].values.erase("RequiredAddOn");

}

void CBasic::PostNcDestroy() 
{
	// do not call PostNcDestroy as this is a member of FinalSunDlg	
	//CPropertyPage::PostNcDestroy();
}
