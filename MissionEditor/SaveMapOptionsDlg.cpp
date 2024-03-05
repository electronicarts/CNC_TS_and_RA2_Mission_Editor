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

// SaveMapOptionsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "SaveMapOptionsDlg.h"
#include "variables.h"
#include "inifile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSaveMapOptionsDlg 


CSaveMapOptionsDlg::CSaveMapOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSaveMapOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaveMapOptionsDlg)
	m_Compress = 1;
	m_PreviewMode = 0;
	m_MapName = _T("");
	m_AirWar = FALSE;
	m_Cooperative = FALSE;
	m_Duel = FALSE;
	m_Meatgrind = FALSE;
	m_Megawealth = FALSE;
	m_Navalwar = FALSE;
	m_Nukewar = FALSE;
	m_Standard = FALSE;
	m_TeamGame = FALSE;
	//}}AFX_DATA_INIT
	
	CIniFile& ini=Map->GetIniFile();
	if(!Map->IsMultiplayer())
		m_PreviewMode=1;

	m_MapName=ini.sections["Basic"].values["Name"];
}


void CSaveMapOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveMapOptionsDlg)
	DDX_Radio(pDX, IDC_COMPRESS, m_Compress);
	DDX_Radio(pDX, IDC_PREVIEWMODE, m_PreviewMode);
	DDX_Text(pDX, IDC_MAPNAME, m_MapName);
	#ifdef RA2_MODE
	DDX_Check(pDX, IDC_AIRWAR, m_AirWar);
	DDX_Check(pDX, IDC_COOPERATIVE, m_Cooperative);
	DDX_Check(pDX, IDC_DUEL, m_Duel);
	DDX_Check(pDX, IDC_MEATGRIND, m_Meatgrind);
	DDX_Check(pDX, IDC_MEGAWEALTH, m_Megawealth);
	DDX_Check(pDX, IDC_NAVALWAR, m_Navalwar);
	DDX_Check(pDX, IDC_NUKEWAR, m_Nukewar);
	DDX_Check(pDX, IDC_STANDARD, m_Standard);
	DDX_Check(pDX, IDC_TEAMGAME, m_TeamGame);
	#endif
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveMapOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CSaveMapOptionsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSaveMapOptionsDlg 

BOOL CSaveMapOptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CIniFile& ini=Map->GetIniFile();
	if(!Map->IsMultiplayer())
	{
		GetDlgItem(IDC_PREVIEWMODE)->EnableWindow(FALSE);
		GetDlgItem(IDC_NOPREVIEW)->EnableWindow(FALSE);
		GetDlgItem(IDC_EXISTINGPREVIEW)->EnableWindow(FALSE);

#ifndef TS_MODE
		GetDlgItem(IDC_AIRWAR)->EnableWindow(FALSE);
		GetDlgItem(IDC_COOPERATIVE)->EnableWindow(FALSE);
		GetDlgItem(IDC_DUEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_MEATGRIND)->EnableWindow(FALSE);
		GetDlgItem(IDC_MEGAWEALTH)->EnableWindow(FALSE);
		GetDlgItem(IDC_NAVALWAR)->EnableWindow(FALSE);
		GetDlgItem(IDC_NUKEWAR)->EnableWindow(FALSE);
		GetDlgItem(IDC_STANDARD)->EnableWindow(FALSE);
		GetDlgItem(IDC_TEAMGAME)->EnableWindow(FALSE);
#endif	
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}
