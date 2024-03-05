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

// SpecialFlags.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "SpecialFlags.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSpecialFlags 


CSpecialFlags::CSpecialFlags(CWnd* pParent /*=NULL*/)
	: CDialog(CSpecialFlags::IDD, 0)
{
	//{{AFX_DATA_INIT(CSpecialFlags)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CSpecialFlags::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpecialFlags)
	DDX_Control(pDX, IDC_VISCEROIDS, m_Visceroids);
	DDX_Control(pDX, IDC_TIBERIUMSPREADS, m_TiberiumSpreads);
	DDX_Control(pDX, IDC_TIBERIUMGROWS, m_TiberiumGrows);
	DDX_Control(pDX, IDC_TIBERIUMEXPLOSIVE, m_TiberiumExplosive);
	DDX_Control(pDX, IDC_METEORITES, m_Meteorites);
	DDX_Control(pDX, IDC_MCVDEPLOY, m_MCVDeploy);
	DDX_Control(pDX, IDC_IONSTORMS, m_IonStorms);
	DDX_Control(pDX, IDC_INITIALVETERAN, m_InitialVeteran);
	DDX_Control(pDX, IDC_INERT, m_Inert);
	DDX_Control(pDX, IDC_HARVESTERIMMUNE, m_HarvesterImmune);
	DDX_Control(pDX, IDC_FOGOFWAR, m_FogOfWar);
	DDX_Control(pDX, IDC_FIXEDALLIANCE, m_FixedAlliance);
	DDX_Control(pDX, IDC_DESTROYABLEBRIDGES, m_DestroyableBridges);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpecialFlags, CDialog)
	//{{AFX_MSG_MAP(CSpecialFlags)
	ON_CBN_EDITCHANGE(IDC_TIBERIUMGROWS, OnEditchangeTiberiumgrows)
	ON_CBN_EDITCHANGE(IDC_TIBERIUMSPREADS, OnEditchangeTiberiumspreads)
	ON_CBN_EDITCHANGE(IDC_TIBERIUMEXPLOSIVE, OnEditchangeTiberiumexplosive)
	ON_CBN_EDITCHANGE(IDC_DESTROYABLEBRIDGES, OnEditchangeDestroyablebridges)
	ON_CBN_EDITCHANGE(IDC_MCVDEPLOY, OnEditchangeMcvdeploy)
	ON_CBN_EDITCHANGE(IDC_INITIALVETERAN, OnEditchangeInitialveteran)
	ON_CBN_EDITCHANGE(IDC_FIXEDALLIANCE, OnEditchangeFixedalliance)
	ON_CBN_EDITCHANGE(IDC_HARVESTERIMMUNE, OnEditchangeHarvesterimmune)
	ON_CBN_EDITCHANGE(IDC_FOGOFWAR, OnEditchangeFogofwar)
	ON_CBN_EDITCHANGE(IDC_INERT, OnEditchangeInert)
	ON_CBN_EDITCHANGE(IDC_IONSTORMS, OnEditchangeIonstorms)
	ON_CBN_EDITCHANGE(IDC_METEORITES, OnEditchangeMeteorites)
	ON_CBN_EDITCHANGE(IDC_VISCEROIDS, OnEditchangeVisceroids)

	ON_CBN_SELCHANGE(IDC_TIBERIUMGROWS, OnEditchangeTiberiumgrows)
	ON_CBN_SELCHANGE(IDC_TIBERIUMSPREADS, OnEditchangeTiberiumspreads)
	ON_CBN_SELCHANGE(IDC_TIBERIUMEXPLOSIVE, OnEditchangeTiberiumexplosive)
	ON_CBN_SELCHANGE(IDC_DESTROYABLEBRIDGES, OnEditchangeDestroyablebridges)
	ON_CBN_SELCHANGE(IDC_MCVDEPLOY, OnEditchangeMcvdeploy)
	ON_CBN_SELCHANGE(IDC_INITIALVETERAN, OnEditchangeInitialveteran)
	ON_CBN_SELCHANGE(IDC_FIXEDALLIANCE, OnEditchangeFixedalliance)
	ON_CBN_SELCHANGE(IDC_HARVESTERIMMUNE, OnEditchangeHarvesterimmune)
	ON_CBN_SELCHANGE(IDC_FOGOFWAR, OnEditchangeFogofwar)
	ON_CBN_SELCHANGE(IDC_INERT, OnEditchangeInert)
	ON_CBN_SELCHANGE(IDC_IONSTORMS, OnEditchangeIonstorms)
	ON_CBN_SELCHANGE(IDC_METEORITES, OnEditchangeMeteorites)
	ON_CBN_SELCHANGE(IDC_VISCEROIDS, OnEditchangeVisceroids)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CSpecialFlags::UpdateDialog()
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	m_DestroyableBridges.SetWindowText(sec->values["DestroyableBridges"]);
	m_FixedAlliance.SetWindowText(sec->values["FixedAlliance"]);
	m_FogOfWar.SetWindowText(sec->values["FogOfWar"]);
	m_HarvesterImmune.SetWindowText(sec->values["HarvesterImmune"]);
	m_Inert.SetWindowText(sec->values["Inert"]);
	m_InitialVeteran.SetWindowText(sec->values["InitialVeteran"]);
	m_IonStorms.SetWindowText(sec->values["IonStorms"]);
	m_MCVDeploy.SetWindowText(sec->values["MCVDeploy"]);
	m_Meteorites.SetWindowText(sec->values["Meteorites"]);
	m_TiberiumExplosive.SetWindowText(sec->values["TiberiumExplosive"]);
	m_TiberiumGrows.SetWindowText(sec->values["TiberiumGrows"]);
	m_TiberiumSpreads.SetWindowText(sec->values["TiberiumSpreads"]);
	m_Visceroids.SetWindowText(sec->values["Visceroids"]);

#ifdef RA2_MODE
	SetDlgItemText(IDC_LTIBERIUMGROWS,"Ore grows:");
	SetDlgItemText(IDC_LTIBERIUMSPREADS,"Ore spreads:");
	GetDlgItem(IDC_LTIBERIUMEXPLOSIVE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_TIBERIUMEXPLOSIVE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_HARVESTERIMMUNE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LHARVESTERIMMUNE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_METEORITES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LMETEORITES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_VISCEROIDS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LVISCEROIDS)->ShowWindow(SW_HIDE);

	SetDlgItemText(IDC_LFOGOFWAR, "Shroud:");
	SetDlgItemText(IDC_LIONSTORMS, "Weather Storms:");
#endif

};
/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSpecialFlags 

void CSpecialFlags::OnEditchangeTiberiumgrows() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_TiberiumGrows);
	sec->values["TiberiumGrows"]=(LPCTSTR)str;
}

void CSpecialFlags::OnEditchangeTiberiumspreads() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_TiberiumSpreads);
	sec->values["TiberiumSpreads"]=(LPCTSTR)str;	
}

void CSpecialFlags::OnEditchangeTiberiumexplosive() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_TiberiumExplosive);
	sec->values["TiberiumExplosive"]=(LPCTSTR)str;
}

void CSpecialFlags::OnEditchangeDestroyablebridges() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_DestroyableBridges);
	sec->values["DestroyableBridges"]=(LPCTSTR)str;	
}

void CSpecialFlags::OnEditchangeMcvdeploy() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_MCVDeploy);
	sec->values["MCVDeploy"]=(LPCTSTR)str;	
}

void CSpecialFlags::OnEditchangeInitialveteran() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_InitialVeteran);
	sec->values["InitialVeteran"]=(LPCTSTR)str;
}

void CSpecialFlags::OnEditchangeFixedalliance() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_FixedAlliance);
	sec->values["FixedAlliance"]=(LPCTSTR)str;
}

void CSpecialFlags::OnEditchangeHarvesterimmune() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_HarvesterImmune);
	sec->values["HarvesterImmune"]=(LPCTSTR)str;	
}

void CSpecialFlags::OnEditchangeFogofwar() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_FogOfWar);
	sec->values["FogOfWar"]=(LPCTSTR)str;	
}

void CSpecialFlags::OnEditchangeInert() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_Inert);
	sec->values["Inert"]=(LPCTSTR)str;	
}

void CSpecialFlags::OnEditchangeIonstorms() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_IonStorms);
	sec->values["IonStorms"]=(LPCTSTR)str;	
}

void CSpecialFlags::OnEditchangeMeteorites() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_Meteorites);
	sec->values["Meteorites"]=(LPCTSTR)str;	
}

void CSpecialFlags::OnEditchangeVisceroids() 
{
	CIniFile& ini=Map->GetIniFile();

	CIniFileSection *sec;
	sec=&ini.sections["SpecialFlags"];
	CString str;


	str=GetText(&m_Visceroids);
	sec->values["Visceroids"]=(LPCTSTR)str;	
}
