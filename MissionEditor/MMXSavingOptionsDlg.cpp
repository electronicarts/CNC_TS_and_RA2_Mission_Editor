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

// MMXSavingOptionsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "MMXSavingOptionsDlg.h"
#include "variables.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CMMXSavingOptionsDlg 


CMMXSavingOptionsDlg::CMMXSavingOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMMXSavingOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMMXSavingOptionsDlg)
	m_Description = _T("");
	m_AirWar = FALSE;
	m_Cooperative = FALSE;
	m_Duel = FALSE;
	m_Maxplayers = 0;
	m_Meatgrind = FALSE;
	m_MegaWealth = FALSE;
	m_MinPlayers = 0;
	m_NavalWar = FALSE;
	m_NukeWar = FALSE;
	m_Standard = TRUE;
	//}}AFX_DATA_INIT

	m_Description=Map->GetIniFile().sections["Basic"].values["Name"];
}


void CMMXSavingOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMMXSavingOptionsDlg)
	DDX_Text(pDX, IDC_DESCRIPTION, m_Description);
	DDX_Check(pDX, IDC_AIRWAR, m_AirWar);
	DDX_Check(pDX, IDC_COOPERATIVE, m_Cooperative);
	DDX_Check(pDX, IDC_DUEL, m_Duel);
	DDX_CBIndex(pDX, IDC_MAXPLAYERS, m_Maxplayers);
	DDX_Check(pDX, IDC_MEATGRIND, m_Meatgrind);
	DDX_Check(pDX, IDC_MEGAWEALTH, m_MegaWealth);
	DDX_CBIndex(pDX, IDC_MINPLAYERS, m_MinPlayers);
	DDX_Check(pDX, IDC_NAVALWAR, m_NavalWar);
	DDX_Check(pDX, IDC_NUKEWAR, m_NukeWar);
	DDX_Check(pDX, IDC_STANDARD, m_Standard);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMMXSavingOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CMMXSavingOptionsDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CMMXSavingOptionsDlg 
