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

// NewMap.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "NewMap.h"
#include "MapOpenDialog.h"
#include "resource.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"

extern CFinalSunApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CNewMap 


CNewMap::CNewMap(CWnd* pParent /*=NULL*/)
	: CDialog(CNewMap::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewMap)
	m_ImportOverlay = FALSE;
	m_ImportTrees = FALSE;
	m_ImportUnits = FALSE;
	m_Multiplayer = FALSE;
	m_House = _T("");
	m_PrepareHouses = FALSE;
	m_AutoProduction = FALSE;
	m_Height = 0;
	m_Width = 0;
	m_Import = -1;
	m_Theater = 0;
	m_GroundHeight = 2;
	//}}AFX_DATA_INIT
}


void CNewMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewMap)
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Check(pDX, IDC_IMPORTOVERLAY, m_ImportOverlay);
	DDX_Check(pDX, IDC_IMPORTTREES, m_ImportTrees);
	DDX_Check(pDX, IDC_IMPORTUNITS, m_ImportUnits);
	DDX_Check(pDX, IDC_MULTIPLAYER, m_Multiplayer);
	DDX_CBString(pDX, IDC_HOUSE, m_House);
	DDX_Check(pDX, IDC_PREPAREHOUSES, m_PrepareHouses);
	DDX_Check(pDX, IDC_AUTOPROD, m_AutoProduction);
	DDX_Text(pDX, IDC_HEIGHT, m_Height);
	DDV_MinMaxUInt(pDX, m_Height, 8, 255);
	DDX_Text(pDX, IDC_WIDTH, m_Width);
	DDV_MinMaxUInt(pDX, m_Width, 8, 255);
	DDX_Radio(pDX, IDC_IMPORT, m_Import);
	DDX_CBIndex(pDX, IDC_THEATER, m_Theater);
	DDX_CBIndex(pDX, IDC_GROUNDHEIGHT, m_GroundHeight);
	DDX_Control(pDX, IDC_IMPORTFILE, m_ImportFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewMap, CDialog)
	//{{AFX_MSG_MAP(CNewMap)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_MULTIPLAYER, OnMultiplayer)
	ON_CBN_EDITCHANGE(IDC_IMPORTFILE, OnEditchangeImportfile)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CNewMap 

BOOL CNewMap::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// preset some stuff
	m_ImportOverlay=TRUE;
	m_ImportUnits=TRUE;
	m_OK.EnableWindow(FALSE);
	m_ImportTrees=TRUE;
	m_PrepareHouses=TRUE;
	m_AutoProduction=TRUE;
	m_Theater=0;
	m_Import=1;
	m_Width=64;
	m_Height=64;
	

	CComboBox& house=*((CComboBox*)(GetDlgItem(IDC_HOUSE)));
	int i;
	for(i=0;i<rules.sections[HOUSES].values.size();i++)
	{
		house.AddString(*rules.sections[HOUSES].GetValue(i));
	}

	m_House=rules.sections[HOUSES].values["0"];

	CComboBox& theater=*((CComboBox*)GetDlgItem(IDC_THEATER));
	theater.AddString(THEATER0);
	theater.AddString(THEATER1);
#ifdef RA2_MODE
	theater.AddString(THEATER2);
#endif

	m_Theater=0;

	UpdateData(FALSE);
	
	UpdateStrings();


	



	// set cursor to wait
	SetCursor(LoadCursor(NULL,IDC_WAIT));

	CString maps=CString(u8AppDataPath.c_str())+"\\stdmaps\\*.mpr";
	CFileFind ff;
	
	if(ff.FindFile(maps))
	{
		BOOL bFileAvailable=TRUE;
		while(bFileAvailable) {
			bFileAvailable=ff.FindNextFile();

			CString file=ff.GetFileName();
			m_ImportFile.AddString(file);
		}

		m_ImportFile.SetCurSel(0);
		OnEditchangeImportfile();
	}

	m_OK.EnableWindow(TRUE);

	// set cursor to ready
	SetCursor(m_hArrowCursor);
	
	return TRUE;  
}

void CNewMap::OnBrowse() 
{
	UpdateData();

	CMapOpenDialog dlg(TRUE, NULL, NULL,  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, "TS maps|*.mpr;*.map|TS multi maps|*.mpr|TS single maps|*.map|");

	char cuPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, cuPath);
	dlg.m_ofn.lpstrInitialDir=cuPath;

	if(theApp.m_Options.TSExe.GetLength()) dlg.m_ofn.lpstrInitialDir=(char*)(LPCTSTR)theApp.m_Options.TSExe;


	if(dlg.DoModal()==IDCANCEL) return;	
	
	m_ImportFile.SetWindowText(dlg.GetPathName());

	UpdateData(FALSE);
	OnEditchangeImportfile();
}



void CNewMap::OnOK() 
{
	UpdateData();	

	CDialog::OnOK();
}

void CNewMap::OnMultiplayer() 
{
	UpdateData();

	CButton& ph=*((CButton*)(GetDlgItem(IDC_PREPAREHOUSES)));
	CButton& autoprod=*((CButton*)(GetDlgItem(IDC_AUTOPROD)));
	CComboBox& house=*((CComboBox*)(GetDlgItem(IDC_HOUSE)));
	
	if(m_Multiplayer)
	{
		ph.EnableWindow(FALSE);
		autoprod.EnableWindow(FALSE);
		m_AutoProduction=FALSE;
		house.EnableWindow(FALSE);
		m_PrepareHouses=FALSE;
	}
	else
	{
		autoprod.EnableWindow(TRUE);
		m_AutoProduction=TRUE;
		ph.EnableWindow(TRUE);
		house.EnableWindow(TRUE);
		m_PrepareHouses=TRUE;
	}

	UpdateData(FALSE);
}

void CNewMap::UpdateStrings()
{
	SetDlgItemText(IDC_LDESC, GetLanguageStringACP("NewMapDesc"));
	SetDlgItemText(IDC_BROWSE, GetLanguageStringACP("NewMapBrowse"));
	SetDlgItemText(IDC_MULTIPLAYER, GetLanguageStringACP("NewMapMultiplayer"));
	SetDlgItemText(IDC_PREPAREHOUSES, GetLanguageStringACP("NewMapPrepareStandardHouses"));
	SetDlgItemText(IDC_AUTOPROD, GetLanguageStringACP("NewMapSetAutoProduction"));
	SetDlgItemText(IDC_LPLAYERHOUSE, GetLanguageStringACP("NewMapPlayerHouse"));
	SetDlgItemText(IDC_LIMPORTOPTIONS, GetLanguageStringACP("NewMapImportOptions"));
	SetDlgItemText(IDC_IMPORTTREES, GetLanguageStringACP("NewMapImportTrees"));
	SetDlgItemText(IDC_IMPORTOVERLAY, GetLanguageStringACP("NewMapImportOverlay"));
	SetDlgItemText(IDC_IMPORTUNITS, GetLanguageStringACP("NewMapImportUnits"));
	SetDlgItemText(IDOK, GetLanguageStringACP("OK"));
	SetDlgItemText(IDCANCEL, GetLanguageStringACP("Cancel"));

	SetWindowText(GetLanguageStringACP("NewMapCap"));
}

void CNewMap::OnEditchangeImportfile() 
{
	UpdateData();

	CIniFile cmap;
	CString file;
	m_ImportFile.GetWindowText(file);

	m_MapToImport=file;

	if(file.Find(":")<0)
	{
		m_MapToImport=CString(u8AppDataPath.c_str())+"\\stdmaps\\";
		m_MapToImport+=file;
	}	

	cmap.InsertFile(m_MapToImport,"Map");
	
	if(cmap.sections.find("Map")==cmap.sections.end())
	{
		m_OK.EnableWindow(FALSE);
		return;
	}

	m_OK.EnableWindow(TRUE);	
}


void CNewMap::OnImport() 
{
	m_OK.EnableWindow(FALSE);
	OnEditchangeImportfile();	
}

void CNewMap::OnNew() 
{
	m_OK.EnableWindow(TRUE);	
}
