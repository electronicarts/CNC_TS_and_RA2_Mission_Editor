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

// Loading.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "Loading.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include "resource.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"
#include "MissionEditorPackLib.h"
#include <chrono>
#include <thread>
#include "VoxelNormals.h"
#include <format>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CLoading 


CLoading::CLoading(CWnd* pParent /*=NULL*/)
	: CDialog(CLoading::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoading)
	//}}AFX_DATA_INIT

	Create(CLoading::IDD, pParent);

	loaded=FALSE;
	cur_theat='T';

	

	m_hCache=NULL;
	m_hConquer=NULL;
	m_hIsoSnow=NULL;
	m_hIsoTemp=NULL;
	m_hLocal=NULL;
	m_hTemperat=NULL;
	m_hTibSun=NULL;
	m_hLangMD=NULL;
	m_pic_count=0;

	s_tiledata=NULL;
	t_tiledata=NULL;
	u_tiledata=NULL;
	s_tiledata_count=0;
	t_tiledata_count=0;
	u_tiledata_count=0;
	tiledata=NULL;
	tiledata_count=0;

	int i=0;
	for(i=0;i<101; i++)
	{
		
		m_hExpand[i].hExpand=NULL;
	}
	for(i=0;i<100; i++)
	{
		
		m_hECache[i]=NULL;
	}

	errstream << "CLoading::CLoading() called" << endl;
	errstream.flush();
}


void CLoading::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoading)
	DDX_Control(pDX, IDC_VERSION, m_Version);
	DDX_Control(pDX, IDC_BUILTBY, m_BuiltBy);
	DDX_Control(pDX, IDC_CAP, m_cap);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoading, CDialog)
	//{{AFX_MSG_MAP(CLoading)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// message handlers CLoading 

BOOL bUseFirestorm=TRUE;

void CLoading::Load()
{
	auto startTime = std::chrono::steady_clock::now();

		
	CString artFile;

	
	

	// show a wait cursor
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	// write a log
	errstream << "CLoading::Load() called" << std::endl << std::endl;
	errstream.flush();


	// initialize the FSunPackLib::XCC
	errstream << "Initializing FSunPackLib::XCC" << std::endl;
	errstream.flush();
	FSunPackLib::XCC_Initialize(TRUE);
	m_cap.SetWindowText(GetLanguageStringACP("LoadExtractStdMixFiles"));

	errstream << "Initializing mix files" << std::endl;
	errstream.flush();
	MEMORYSTATUS ms;
	ms.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	int cs=ms.dwAvailPhys+ms.dwAvailPageFile;
	InitMixFiles();
	errstream << "Loading palettes" << std::endl << std::endl;
	errstream.flush();
	InitPalettes();

	// Load voxel normal tables
	InitVoxelNormalTables();

	
	// create a ini file containing some info XCC Mixer needs
	CreateINI();

	// set progress bar range to 0-2
	m_progress.SetRange(0,2);	
	
	// rules.ini
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadRules"));
	m_progress.SetPos(1);
	UpdateWindow();
	LoadTSIni("Rules.ini", &rules, FALSE);

#ifdef TS_MODE
	if(bUseFirestorm) 
	{
		// very special case, the types list don´t have any order according to the original rules!
		CIniFile fstorm;
		LoadTSIni("firestrm.ini", &fstorm, TRUE);

		int i;
		CString cuSection;
		cuSection="InfantryTypes";
		for(i=0;i<fstorm.sections[cuSection].values.size();i++)
		{
			// we simply add a FS to the index
			char c[50];
			itoa(i, c, 10);
			strcat(c, "FS");
			rules.sections[cuSection].values[c]=*fstorm.sections[cuSection].GetValue(i);
			rules.sections[cuSection].value_orig_pos[c]=rules.sections[cuSection].values.size()-1;
		}
		cuSection="VehicleTypes";
		for(i=0;i<fstorm.sections[cuSection].values.size();i++)
		{
			// we simply add a FS to the index
			char c[50];
			itoa(i, c, 10);
			strcat(c, "FS");
			rules.sections[cuSection].values[c]=*fstorm.sections[cuSection].GetValue(i);
			rules.sections[cuSection].value_orig_pos[c]=rules.sections[cuSection].values.size()-1;
		}
		cuSection="AircraftTypes";
		for(i=0;i<fstorm.sections[cuSection].values.size();i++)
		{
			// we simply add a FS to the index
			char c[50];
			itoa(i, c, 10);
			strcat(c, "FS");
			rules.sections[cuSection].values[c]=*fstorm.sections[cuSection].GetValue(i);
			rules.sections[cuSection].value_orig_pos[c]=rules.sections[cuSection].values.size()-1;
		}
		cuSection="BuildingTypes";
		for(i=0;i<fstorm.sections[cuSection].values.size();i++)
		{
			// we simply add a FS to the index
			char c[50];
			itoa(i, c, 10);
			strcat(c, "FS");
			rules.sections[cuSection].values[c]=*fstorm.sections[cuSection].GetValue(i);
			rules.sections[cuSection].value_orig_pos[c]=rules.sections[cuSection].values.size()-1;
		}
		cuSection="SuperWeaponTypes";
		for(i=0;i<fstorm.sections[cuSection].values.size();i++)
		{
			// we simply add a FS to the index
			char c[50];
			itoa(i, c, 10);
			strcat(c, "FS");
			rules.sections[cuSection].values[c]=*fstorm.sections[cuSection].GetValue(i);
			rules.sections[cuSection].value_orig_pos[c]=rules.sections[cuSection].values.size()-1;
		}

		for(i=0;i<fstorm.sections.size(); i++)
		{
			cuSection=*fstorm.GetSectionName(i);
			if(cuSection!="SuperWeaponTypes" && cuSection!="InfantryTypes" && cuSection!="VehicleTypes" && cuSection!="AircraftTypes" && cuSection!="BuildingTypes")
			{
				int e;
				for(e=0;e<fstorm.sections[cuSection].values.size();e++)
				{
					CString cuValue=*fstorm.sections[cuSection].GetValueName(e);
					rules.sections[cuSection].values[cuValue]=*fstorm.sections[cuSection].GetValue(e);
				}
			}
		}
	}
#else
	if(bUseFirestorm && yuri_mode) // MW actually this is Yuri's Revenge
	{
		CIniFile rulesmd;
		LoadTSIni("rulesmd.ini", &rulesmd, TRUE);
		if(rulesmd.sections.size()>0)
		{
			rules=rulesmd;
		}
	}
#endif

	m_progress.SetPos(2);
	//rules.DeleteLeadingSpaces(TRUE, TRUE);
	//rules.DeleteEndingSpaces(TRUE, TRUE);

	PrepareHouses();
	HackRules();
	
	UpdateWindow();


	// art.ini
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadArt"));
	m_progress.SetPos(1);
	UpdateWindow();
	LoadTSIni("Art.ini", &art, FALSE);
#ifdef TS_MODE
	if(bUseFirestorm) // LoadTSIni("ArtFs.ini", &art, TRUE);
	{
		CIniFile artfs;
		LoadTSIni("artfs.ini", &artfs, TRUE);

		// MW April 8th: bugfix... we erased Movies even if there was no new data!!!
		if(artfs.sections.size())
		{

			int i;
			CString cuSection;
			cuSection="Movies";
			art.sections.erase(cuSection);
			/*for(i=0;i<artfs.sections[cuSection].values.size();i++)
			{
				// we simply add a FS to the index
				char c[50];
				itoa(i, c, 10);
				strcat(c, "FS");
				art.sections[cuSection].values[c]=*artfs.sections[cuSection].GetValue(i);
				art.sections[cuSection].value_orig_pos[c]=art.sections[cuSection].values.size()-1;
			}*/

			for(i=0;i<artfs.sections.size(); i++)
			{
				cuSection=*artfs.GetSectionName(i);
				//if(cuSection!="Movies")
				{
					int e;
					for(e=0;e<artfs.sections[cuSection].values.size();e++)
					{
						CString cuValue=*artfs.sections[cuSection].GetValueName(e);
						art.sections[cuSection].values[cuValue]=*artfs.sections[cuSection].GetValue(e);
						if(cuSection=="Movies") art.sections[cuSection].value_orig_pos[cuValue]=artfs.sections[cuSection].GetValueOrigPos(e);
					}
				}
			}
		}

	}
#else
	if(bUseFirestorm && yuri_mode) // Yuri's Revenge
	{
		CIniFile artmd;
		LoadTSIni("artmd.ini", &artmd, TRUE);
		if(artmd.sections.size()>0)
		{
			art.Clear();
			art=artmd;
		}
	}
#endif

	

	m_progress.SetPos(2);
	//art.DeleteLeadingSpaces(TRUE, TRUE);
	//art.DeleteEndingSpaces(TRUE, TRUE);
	UpdateWindow();

	// tutorial.ini
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadTutorial"));
	m_progress.SetPos(1);
	UpdateWindow();
	LoadTSIni("Tutorial.ini", &tutorial, FALSE);

	// sound.ini
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadSound"));
	m_progress.SetPos(1);
	UpdateWindow();	
	
	
	if(bUseFirestorm)
	{
		int b;
		for(b=99;b>0;b--)
		{
			char p[50];
			itoa(b, p, 10);
			CString name="Sound";
			if(strlen(p)<2) name+="0";
			name+=p;
			name+=".ini";

			LoadTSIni(name, &sound, FALSE);
			if(sound.sections.size()>0) break;
		}

	} else LoadTSIni("Sound01.ini", &sound, FALSE);
	if(sound.sections.size()==0) LoadTSIni("Sound.ini", &sound, FALSE);

	m_progress.SetPos(2);
	UpdateWindow();

	// eva.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadEva"));
	m_progress.SetPos(1);
	UpdateWindow();
	LoadTSIni("eva.ini", &eva, FALSE);
	m_progress.SetPos(2);
	UpdateWindow();

	// theme.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadTheme"));
	m_progress.SetPos(1);
	UpdateWindow();
	LoadTSIni("theme.ini", &theme, FALSE);
	m_progress.SetPos(2);
	UpdateWindow();


	// AI.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadAI"));
	m_progress.SetPos(1);
	UpdateWindow();
	LoadTSIni("Ai.ini", &ai, FALSE);
#ifdef TS_MODE
	if(bUseFirestorm) LoadTSIni("aifs.ini", &ai, TRUE);;
#else
	if(bUseFirestorm && yuri_mode) LoadTSIni("aimd.ini", &ai, TRUE); // YR
#endif
	m_progress.SetPos(2);
	UpdateWindow();


	const BOOL preferLocalTheater = theApp.m_Options.bPreferLocalTheaterFiles ? TRUE : FALSE;

	// Temperat.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadTemperat"));
	m_progress.SetPos(1);
	UpdateWindow();
	LoadTSIni("Temperat.ini", &tiles_t, FALSE, preferLocalTheater);
	if(yuri_mode) LoadTSIni("TemperatMD.ini", &tiles_t, TRUE, preferLocalTheater);
	m_progress.SetPos(2);
	UpdateWindow();

	// Snow.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadSnow"));
	m_progress.SetPos(1);
	UpdateWindow();
	LoadTSIni("Snow.ini", &tiles_s, FALSE, preferLocalTheater);
	if(yuri_mode) LoadTSIni("SnowMD.ini", &tiles_s, TRUE, preferLocalTheater);
	m_progress.SetPos(2);
	UpdateWindow();

	// Urban.INI
	m_cap.SetWindowText(GetLanguageStringACP("LoadLoadUrban"));
	m_progress.SetPos(1);
	UpdateWindow();
	LoadTSIni("Urban.ini", &tiles_u, FALSE, preferLocalTheater);
	if(yuri_mode) LoadTSIni("UrbanMD.ini", &tiles_u, TRUE, preferLocalTheater);
	m_progress.SetPos(2);
	UpdateWindow();

	if(yuri_mode)
	{
		m_cap.SetWindowText(GetLanguageStringACP("LoadLoadUrbanN"));
		m_progress.SetPos(1);
		UpdateWindow();
		LoadTSIni("UrbanNMD.ini", &tiles_un, FALSE, preferLocalTheater);
		m_progress.SetPos(2);
		UpdateWindow();

		// MW FIX: MAKE URBAN RAMPS MORPHABLE:
		if(tiles_un.sections["TileSet0117"].FindName("Morphable")<0)
			tiles_un.sections["TileSet0117"].values["Morphable"]="true";

		m_cap.SetWindowText(GetLanguageStringACP("LoadLoadLunar"));
		m_progress.SetPos(1);
		UpdateWindow();
		LoadTSIni("LunarMD.ini", &tiles_l, FALSE, preferLocalTheater);
		m_progress.SetPos(2);
		UpdateWindow();

		m_cap.SetWindowText(GetLanguageStringACP("LoadLoadDesert"));
		m_progress.SetPos(1);
		UpdateWindow();
		LoadTSIni("DesertMD.ini", &tiles_d, FALSE, preferLocalTheater);
		m_progress.SetPos(2);
		UpdateWindow();
	}

	
	
	// load Command & Conquer Rules.ini section names
	LoadStrings();


	// ok now directdraw
	m_cap.SetWindowText(GetLanguageStringACP("LoadInitDDraw"));
	m_progress.SetRange(0,4);
	
	InitDirectDraw();

	m_progress.SetPos(3);
	UpdateWindow();
	
	/*errstream << "Now calling InitPics()\n";
	errstream.flush();
	m_cap.SetWindowText(GetLanguageStringACP("LoadInitPics"));
	InitPics();
	errstream << "InitPics() finished\n\n\n";
	errstream.flush();*/

	auto delay = std::chrono::duration<double>(theApp.m_Options.fLoadScreenDelayInSeconds);
	if ((std::chrono::steady_clock::now() - startTime) < delay)
	{
		std::this_thread::sleep_until(startTime + delay);
	}

	DestroyWindow();

}



//
// InitPics loads all graphics except terrain graphics!
void CLoading::InitPics(CProgressCtrl* prog)
{
	MEMORYSTATUS ms;
	ms.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	int cs=ms.dwAvailPhys+ms.dwAvailPageFile;

	errstream << "InitPics() called. Available memory: " << cs << endl;
	errstream.flush();

	
	
	
	CalcPicCount();

	if(m_progress.m_hWnd!=NULL) m_progress.SetRange(0,m_pic_count/15+1);

	if(!theApp.m_Options.bDoNotLoadBMPs)
	{
		int k;
		CFileFind ff;
		CString bmps=(CString)AppPath+"\\pics2\\*.bmp";
		if(ff.FindFile(bmps))
		{

			BOOL lastFile=FALSE;
				
			for(k=0;k<m_bmp_count+1;k++)
			{
					
				if(ff.FindNextFile()==0)
				{
					if(lastFile) // we already were at last file, so that´s an error here!
					break;
					lastFile=TRUE;
				}			
				
				try
				{
					pics[(LPCTSTR)ff.GetFileName()].pic = BitmapToSurface(((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd, *BitmapFromFile(ff.GetFilePath())).Detach();

					DDSURFACEDESC2 desc;
					memset(&desc, 0, sizeof(DDSURFACEDESC2));
					desc.dwSize = sizeof(DDSURFACEDESC2);
					desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
					((LPDIRECTDRAWSURFACE4)pics[(LPCTSTR)ff.GetFileName()].pic)->GetSurfaceDesc(&desc);
					pics[(LPCTSTR)ff.GetFileName()].wHeight = desc.dwHeight;
					pics[(LPCTSTR)ff.GetFileName()].wWidth = desc.dwWidth;
					pics[(LPCTSTR)ff.GetFileName()].wMaxWidth = desc.dwWidth;
					pics[(LPCTSTR)ff.GetFileName()].wMaxHeight = desc.dwHeight;
					pics[(LPCTSTR)ff.GetFileName()].bType = PICDATA_TYPE_BMP;

					FSunPackLib::SetColorKey(((LPDIRECTDRAWSURFACE4)(pics[(LPCTSTR)ff.GetFileName()].pic)), -1);
				}
				catch (const BitmapNotFound&)
				{
				}

				if(m_progress.m_hWnd!=NULL && k%15==0)
				{
					m_progress.SetPos(m_progress.GetPos()+1);
					UpdateWindow();
				}
			}
		}
	}

	DDSURFACEDESC2 desc;

	try {
		pics["SCROLLCURSOR"].pic = BitmapToSurface(((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd, *BitmapFromResource(IDB_SCROLLCURSOR)).Detach();
		FSunPackLib::SetColorKey((LPDIRECTDRAWSURFACE4)pics["SCROLLCURSOR"].pic, -1);
		memset(&desc, 0, sizeof(DDSURFACEDESC2));
		desc.dwSize = sizeof(DDSURFACEDESC2);
		desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
		((LPDIRECTDRAWSURFACE4)pics["SCROLLCURSOR"].pic)->GetSurfaceDesc(&desc);
		pics["SCROLLCURSOR"].wHeight = desc.dwHeight;
		pics["SCROLLCURSOR"].wWidth = desc.dwWidth;
		pics["SCROLLCURSOR"].bType = PICDATA_TYPE_BMP;
	}
	catch (const BitmapNotFound&)
	{
	}

	try {
		pics["CELLTAG"].pic = BitmapToSurface(((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd, *BitmapFromResource(IDB_CELLTAG)).Detach();
		FSunPackLib::SetColorKey((LPDIRECTDRAWSURFACE4)pics["CELLTAG"].pic, CLR_INVALID);
		memset(&desc, 0, sizeof(DDSURFACEDESC2));
		desc.dwSize = sizeof(DDSURFACEDESC2);
		desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
		((LPDIRECTDRAWSURFACE4)pics["CELLTAG"].pic)->GetSurfaceDesc(&desc);
		pics["CELLTAG"].wHeight = desc.dwHeight;
		pics["CELLTAG"].wWidth = desc.dwWidth;
#ifdef RA2_MODE
		pics["CELLTAG"].x = -1;
		pics["CELLTAG"].y = 0;
#else
		pics["CELLTAG"].x = -1;
		pics["CELLTAG"].y = -1;
#endif
		pics["CELLTAG"].bType = PICDATA_TYPE_BMP;
	}
	catch (const BitmapNotFound&) 
	{
	}

	try
	{
		pics["FLAG"].pic = BitmapToSurface(((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd, *BitmapFromResource(IDB_FLAG)).Detach();
		FSunPackLib::SetColorKey((LPDIRECTDRAWSURFACE4)pics["FLAG"].pic, -1);
		memset(&desc, 0, sizeof(DDSURFACEDESC2));
		desc.dwSize = sizeof(DDSURFACEDESC2);
		desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
		((LPDIRECTDRAWSURFACE4)pics["FLAG"].pic)->GetSurfaceDesc(&desc);
		pics["FLAG"].wHeight = desc.dwHeight;
		pics["FLAG"].wWidth = desc.dwWidth;
		pics["FLAG"].bType = PICDATA_TYPE_BMP;
	}
	catch (const BitmapNotFound&)
	{
	}



	// MW April 2nd, 2001
	// prepare 1x1 hidden tile replacement
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	ddsd.dwSize=sizeof(DDSURFACEDESC2);
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.dwWidth=f_x;
	ddsd.dwHeight=f_y;

	LPDIRECTDRAWSURFACE4 srf=NULL;
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd->CreateSurface(&ddsd, &srf, 0);

	pics["HTILE"].pic=srf;
	pics["HTILE"].wHeight=ddsd.dwHeight;
	pics["HTILE"].wWidth=ddsd.dwWidth;
	pics["HTILE"].bType=PICDATA_TYPE_BMP;
	
	HDC hDC;
	srf->GetDC(&hDC);

	HPEN p;
	int width=1;
	p=CreatePen(PS_DOT, 0, RGB(0,120,0));

	SelectObject(hDC,p);

	POINT p1, p2, p3, p4;
	p1.x=f_x/2;
	p1.y=0;
	p2.x=f_x/2+f_x/2;
	p2.y=f_y/2;
	p3.x=f_x/2-f_x/2+f_x/2-1;
	p3.y=f_y/2+f_y/2-1; 
	p4.x=f_x/2+f_x/2-1;
	p4.y=f_y/2-1;


	SetBkMode(hDC, TRANSPARENT);
	MoveToEx(hDC, p1.x,p1.y-1,NULL);
	LineTo(hDC, p2.x+1,p2.y);
	LineTo(hDC, p3.x,p3.y+1);
	LineTo(hDC, p4.x-1, p4.y);
	LineTo(hDC, p1.x, p1.y-1);

	srf->ReleaseDC(hDC);

	DeleteObject(p);


	// new: Prepare building terrain information:
	int i;
	for(i=0;i<rules.sections["BuildingTypes"].values.size();i++)
	{
		PrepareUnitGraphic(*rules.sections["BuildingTypes"].GetValue(i));
		
	}
	ms.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	cs=ms.dwAvailPhys+ms.dwAvailPageFile;

	int piccount=pics.size();

	errstream << "InitPics() finished and loaded " << piccount << " pictures. Available memory: " << cs << endl;
	errstream.flush();

	return;
}

BOOL CLoading::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString version;
	version.LoadString(IDS_VERSIONTEXT);
#ifdef TS_MODE
	version.LoadString(IDS_VERSIONTEXTTS);
#endif

	m_Version.SetWindowText(version);
	
	CString builder;
	builder.LoadString(IDS_BUILTBY);
	m_BuiltBy.SetWindowText(builder);

	SetDlgItemText(IDC_LBUILTBY, GetLanguageStringACP("LoadBuiltBy"));
	SetDlgItemText(IDC_LVERSION, GetLanguageStringACP("LoadVersion"));
	SetDlgItemText(IDC_CAP, GetLanguageStringACP("LoadLoading"));
	

	
	UpdateWindow();

	return TRUE;
}

// write a small ini file containing the FinalSun path and version (XCC needs this)
// TODO: this was made for Win9x. It does not work anymore on modern operating systems if you don't run the editor as administrator (which you should not do)
void CLoading::CreateINI()
{
	

	wchar_t iniFile_[MAX_PATH];
	CIniFile path;
	CString version;
	
	GetWindowsDirectoryW(iniFile_, MAX_PATH);
	std::string iniFile = utf16ToUtf8(iniFile_);
#ifdef RA2_MODE
	iniFile += "\\FinalAlert2.ini";
#else
	iniFile += "\\FinalSun.ini";
#endif

#ifdef RA2_MODE
	CString app = "FinalAlert";
#else
	CString app = "FinalSun";
#endif

	version.LoadString(IDS_VERSION);
	path.sections[app].values["Path"]=AppPath;
	path.sections[app].values["Version"]=version;

	path.SaveFile(iniFile);
}










void CLoading::LoadTSIni(LPCTSTR lpFilename, CIniFile *lpIniFile, BOOL bIsExpansion, BOOL bCheckEditorDir)
{
	errstream << "LoadTSIni(" << lpFilename << "," << lpIniFile << "," << bIsExpansion << ") called" << endl;
	errstream.flush();

	if (bCheckEditorDir) {
		// check if file is available in the editors application data folder
		if (DoesFileExist((u8AppDataPath + lpFilename).c_str()))
		{
			errstream << "File found in mission editor AppData directory (" << u8AppDataPath << ")" << endl;
			errstream.flush();
			if (!bIsExpansion)
				lpIniFile->LoadFile(u8AppDataPath + lpFilename, TRUE);
			else
				lpIniFile->InsertFile(u8AppDataPath + lpFilename, NULL, TRUE);
			return;
		}

		// check if file is available in the editors root folder
		if(DoesFileExist(CString(AppPath) + lpFilename))
		{
			errstream << "File found in Mission Editor directory (" << AppPath << ")" << endl;
			errstream.flush();
			if(!bIsExpansion)
				lpIniFile->LoadFile((CString)AppPath+lpFilename, TRUE);
			else
				lpIniFile->InsertFile((CString)AppPath+lpFilename, NULL, TRUE);
			return;
		}
	}

	if(theApp.m_Options.bSearchLikeTS)
	{
		
		
		// check if Rules.ini is available
		if(DoesFileExist((CString)TSPath+lpFilename))
		{
			errstream << "File found in TS directory (" << TSPath << ")" << endl;
			errstream.flush();
			if(!bIsExpansion)
				lpIniFile->LoadFile((CString)TSPath+lpFilename, TRUE);
			else
				lpIniFile->InsertFile((CString)TSPath+lpFilename, NULL, TRUE);
			return;
		}
		

		BOOL bExpandFound=FALSE;

		

		/*int i;
		for(i=1;i<101;i++)
		{
			if(m_hExpand[i].hExpand!=NULL)
			{
				if(FSunPackLib::XCC_ExtractFile(lpFilename, (CString)TSPath+(CString)"FinalSun"+lpFilename, m_hExpand[i].hExpand))
				{

					errstream << lpFilename << " found in expansion #" << i << endl;
					errstream.flush();
						
					//if(!bIsExpansion)
					//	lpIniFile->InsertFile((CString)TSPath+(CString)"FinalSun"+lpFilename, NULL);
					//else
					lpIniFile->InsertFile((CString)TSPath+(CString)"FinalSun"+lpFilename, NULL, TRUE);

					DeleteFile((CString)TSPath+(CString)"FinalSun"+lpFilename);
					
					bExpandFound=TRUE;
				}
				 
			}
		}*/

		HMIXFILE hMix=FindFileInMix(lpFilename);
		if(hMix)
		{
			if(FSunPackLib::XCC_ExtractFile(lpFilename, (CString)TSPath+(CString)"FinalSun"+lpFilename, hMix))
				{

					errstream << lpFilename << " found " << endl;
					errstream.flush();
						
					//if(!bIsExpansion)
					//	lpIniFile->InsertFile((CString)TSPath+(CString)"FinalSun"+lpFilename, NULL);
					//else
					lpIniFile->InsertFile((CString)TSPath+(CString)"FinalSun"+lpFilename, NULL, TRUE);

					DeleteFile((CString)TSPath+(CString)"FinalSun"+lpFilename);
					
					bExpandFound=TRUE;
				}
		}

		if(!bIsExpansion && !bExpandFound)
		{
			// not found, go ahead if this is not a expansion only file... 
			FSunPackLib::XCC_ExtractFile(lpFilename,(CString)TSPath+(CString)"FinalSun"+lpFilename, m_hLocal);			
			
			lpIniFile->LoadFile((CString)TSPath+(CString)"FinalSun"+lpFilename, TRUE);
			
			DeleteFile((CString)TSPath+(CString)"FinalSun"+lpFilename);
		}
		
		
	}
	else if(bIsExpansion==FALSE)
	{
		FSunPackLib::XCC_ExtractFile(lpFilename,(CString)TSPath+(CString)"FinalSun"+lpFilename, m_hLocal);
		
		lpIniFile->LoadFile((CString)TSPath+(CString)"FinalSun"+lpFilename, TRUE);
		DeleteFile((CString)TSPath+(CString)"FinalSun"+lpFilename);
		return;
	}
}


void CLoading::InitSHPs(CProgressCtrl* prog)
{
	MEMORYSTATUS ms;
	ms.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	int cs=ms.dwAvailPhys+ms.dwAvailPageFile;

	errstream << "InitSHPs() called. Available memory: " << cs << endl;
	errstream.flush();

	int i;

	// overlay:
	if(!theApp.m_Options.bDoNotLoadOverlayGraphics)
	for(i=0;i<rules.sections["OverlayTypes"].values.size();i++)
	{
		LoadOverlayGraphic(*rules.sections["OverlayTypes"].GetValue(i), i);
		if(m_progress.m_hWnd!=NULL && i%15==0) {
			m_progress.SetPos(m_progress.GetPos()+1);
			UpdateWindow();
		}
	}	

	if(!theApp.m_Options.bDoNotLoadVehicleGraphics)
	for(i=0;i<rules.sections["VehicleTypes"].values.size();i++)
	{
		LoadUnitGraphic(*rules.sections["VehicleTypes"].GetValue(i));
		if(m_progress.m_hWnd!=NULL && i%15==0) {
			m_progress.SetPos(m_progress.GetPos()+1);
			UpdateWindow();
		}
	}

	if(!theApp.m_Options.bDoNotLoadInfantryGraphics)
	for(i=0;i<rules.sections["InfantryTypes"].values.size();i++)
	{
		LoadUnitGraphic(*rules.sections["InfantryTypes"].GetValue(i));
		if(m_progress.m_hWnd!=NULL && i%15==0){
			m_progress.SetPos(m_progress.GetPos()+1);
			UpdateWindow();
		}
	}

	if(!theApp.m_Options.bDoNotLoadBuildingGraphics)
	for(i=0;i<rules.sections["BuildingTypes"].values.size();i++)
	{
		LoadUnitGraphic(*rules.sections["BuildingTypes"].GetValue(i));
		if(m_progress.m_hWnd!=NULL && i%15==0) {
			m_progress.SetPos(m_progress.GetPos()+1);
			UpdateWindow();
		}
	}

	if(!theApp.m_Options.bDoNotLoadAircraftGraphics)
	for(i=0;i<rules.sections["AircraftTypes"].values.size();i++)
	{
		LoadUnitGraphic(*rules.sections["AircraftTypes"].GetValue(i));
		if(m_progress.m_hWnd!=NULL && i%15==0) {
			m_progress.SetPos(m_progress.GetPos()+1);
			UpdateWindow();
		}
	}

	if(!theApp.m_Options.bDoNotLoadTreeGraphics)
	for(i=0;i<rules.sections["TerrainTypes"].values.size();i++)
	{
		LoadUnitGraphic(*rules.sections["TerrainTypes"].GetValue(i));
		if(m_progress.m_hWnd!=NULL && i%15==0)
		{
			m_progress.SetPos(m_progress.GetPos()+1);
			UpdateWindow();
		}
	}

#ifdef SMUDGE_SUPP
	for(i=0;i<rules.sections["SmudgeTypes"].values.size();i++)
	{
		LoadUnitGraphic(*rules.sections["SmudgeTypes"].GetValue(i));
		/*if(m_progress.m_hWnd!=NULL && i%15==0)
		{
			m_progress.SetPos(m_progress.GetPos()+1);
			UpdateWindow();
		}*/
	}
#endif
	
}

/*
CLoading::LoadUnitGraphic();

Matze:
June 15th:
- Added bib support, works fine.
*/

#ifdef NOSURFACES_OBJECTS // palettized

// blit util
__forceinline void Blit_Pal(BYTE* dst, int x, int y, int dwidth, int dheight, BYTE* src, int swidth, int sheight)
{
		
	if(src==NULL || dst==NULL) return;



	if(x+swidth<0 || y+sheight<0) return;
	if(x>=dwidth || y>=dheight) return;


	RECT blrect;
	RECT srcRect;
	srcRect.left=0;
	srcRect.top=0;
	srcRect.right=swidth;
	srcRect.bottom=sheight;
	blrect.left=x;
	if(blrect.left<0) 
	{
		srcRect.left=1-blrect.left;
		blrect.left=1;
	}
	blrect.top=y;
	if(blrect.top<0) 
	{
		srcRect.top=1-blrect.top;
		blrect.top=1;
	}
	blrect.right=(x+swidth);
	if(x+swidth>dwidth)
	{
		srcRect.right=swidth-((x+swidth)-dwidth);
		blrect.right=dwidth;
	}
	blrect.bottom=(y+sheight);
	if(y+sheight>dheight)
	{
		srcRect.bottom=sheight-((y+sheight)-dheight);
		blrect.bottom=dheight;
	}


	int i,e;
	for(i=srcRect.left;i<srcRect.right;i++)
	{
		for(e=srcRect.top;e<srcRect.bottom;e++)
		{
			BYTE& val=src[i+e*swidth];
			if(val)
			{
				BYTE* dest=dst+(blrect.left+i)+(blrect.top+e)*dwidth;
				*dest=val;
			}
		}
	}
	
}

__forceinline void Blit_PalD(BYTE* dst, RECT blrect, const BYTE* src, RECT srcRect, int swidth, int dwidth, int sheight, int dheight, const BYTE* const srcMask=nullptr)
{
		
	if(src==NULL || dst==NULL) return;

	if(blrect.left+swidth<0 || blrect.top+sheight<0) return;
	if(blrect.left>=dwidth || blrect.top>=dheight) return;
	
	int x=blrect.left;
	int y=blrect.top;

	int i,e;
	for(i=srcRect.left;i<srcRect.right;i++)
	{
		for(e=srcRect.top;e<srcRect.bottom;e++)
		{
			auto pos = i + e * swidth;
			const BYTE& val=src[pos];
			if(blrect.left+i>0 && blrect.top+e>0 && blrect.left+i<dwidth && blrect.top+e<dheight)
			{
				if (srcMask)
				{
					if (srcMask[pos] == 0)
						continue;
				}
				else
				{
					if (val == 0)
						continue;
				}

				BYTE* dest=dst+(blrect.left+i)+(blrect.top+e)*dwidth;
				*dest=val;
			}
		}
	}
	
}

#ifdef TS_MODE
const Vec3f lightDirection = Vec3f(1.0f, 0.0f, -1.0f).normalize();
#else
const Vec3f lightDirection = Vec3f(1.0f, 0.0f, -1.0f).normalize();
#endif


HTSPALETTE CLoading::GetIsoPalette(char theat)
{
	HTSPALETTE isoPalette = m_hPalIsoTemp;
	switch (theat)
	{
	case 'T':
	case 'G':
		isoPalette = m_hPalIsoTemp;
		break;
	case 'A':
		isoPalette = m_hPalIsoSnow;
		break;
	case 'U':
		isoPalette = m_hPalIsoUrb;
		break;
	case 'N':
		isoPalette = m_hPalIsoUbn;
		break;
	case 'D':
		isoPalette = m_hPalIsoDes;
		break;
	case 'L':
		isoPalette = m_hPalIsoLun;
		break;
	}
	return isoPalette;
}

HTSPALETTE CLoading::GetUnitPalette(char theat)
{
	HTSPALETTE isoPalette = m_hPalUnitTemp;
	switch (theat)
	{
	case 'T':
	case 'G':
		isoPalette = m_hPalUnitTemp;
		break;
	case 'A':
		isoPalette = m_hPalUnitSnow;
		break;
	case 'U':
		isoPalette = m_hPalUnitUrb;
		break;
	case 'N':
		isoPalette = m_hPalUnitUbn;
		break;
	case 'D':
		isoPalette = m_hPalUnitDes;
		break;
	case 'L':
		isoPalette = m_hPalUnitLun;
		break;
	}
	return isoPalette;
}

CString theatToSuffix(char theat)
{
	if (theat == 'T') return ".tem";
	else if (theat == 'A') return ".sno";
	else if (theat == 'U') return ".urb";
	else if (theat == 'L') return ".lun";
	else if (theat == 'D') return ".des";
	else if (theat == 'N') return ".ubn";
	return ".tem";
}

char suffixToTheat(const CString& suffix)
{
	if (suffix == ".tem")
		return 'T';
	else if (suffix == ".sno")
		return 'A';
	else if (suffix == ".urb")
		return 'U';
	else if (suffix == ".lun")
		return 'L';
	else if (suffix == ".des")
		return 'D';
	else if (suffix == ".ubn")
		return 'N';
	return 'T';
}

std::optional<FindShpResult> CLoading::FindUnitShp(const CString& image, char preferred_theat, const CIniFileSection& artSection)
{
	if (image.IsEmpty())
		return std::nullopt;

	const char kTheatersToTry[] = { preferred_theat, 'G', 'T', 'A', 'U', 'N', 'D', 'L', 0};
	const CString kSuffixesToTry[] = { theatToSuffix(preferred_theat), ".tem", ".sno", ".urb", ".lun", ".des", ".ubn", ""};
	
	const char first = image.GetAt(0);
	const bool firstCharSupportsTheater = first == 'G' || first == 'N' || first == 'C' || first == 'Y';

	HTSPALETTE forcedPalette = 0;
	const auto& unitPalettePrefixes = g_data.sections["ForceUnitPalettePrefix"];
	if (unitPalettePrefixes.end() != std::find_if(unitPalettePrefixes.begin(), unitPalettePrefixes.end(), [&image](const auto& pair) {return image.Find(pair.second) == 0;}))
	{
		forcedPalette = GetUnitPalette(preferred_theat);
	}

	const auto& isoPalettePrefixes = g_data.sections["ForceIsoPalettePrefix"];
	if (isoPalettePrefixes.end() != std::find_if(isoPalettePrefixes.begin(), isoPalettePrefixes.end(), [&image](const auto& pair) {return image.Find(pair.second) == 0;}))
	{
		forcedPalette = GetIsoPalette(preferred_theat);
	}

	const bool isTheater = isTrue(artSection.GetValueByName("Theater"));
	const bool isNewTheater = isTrue(artSection.GetValueByName("NewTheater"));
	const bool terrainPalette = isTrue(artSection.GetValueByName("TerrainPalette"));

	auto unitOrIsoPalette = terrainPalette ? GetIsoPalette(preferred_theat) : GetUnitPalette(preferred_theat);

	HMIXFILE curMixFile = 0;
	CString curFilename = image + ".shp";
	TheaterChar curMixTheater = TheaterChar::None;
	char curTheater = 'G';
	CString curSuffix;

	// Phase 0: theater with .tem, .sno etc
	if (isTheater)
	{
		for (int t = 0; !(curSuffix = kSuffixesToTry[t]).IsEmpty(); ++t)
		{
			curFilename = image + curSuffix;
			curFilename.MakeLower();
			curMixFile = FindFileInMix(curFilename, &curMixTheater);
			if (curMixFile)
				return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(suffixToTheat(curSuffix)), forcedPalette ? forcedPalette : GetIsoPalette(suffixToTheat(curSuffix)));
		}
	}
	// Phase 1: if NewTheater is enabled and first character indicates support, try with real theater, then in order of kTheatersToTry
	// Otherwise,
	if (isNewTheater)
	{
		if (firstCharSupportsTheater)
		{
			/*curFilename.SetAt(1, preferred_theat);
			curMixFile = FindFileInMix(curFilename, &curMixTheater);
			if (curMixFile)
				return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(preferred_theat), GetUnitPalette(preferred_theat));*/
			for (int t = 0; curTheater=kTheatersToTry[t]; ++t)
			{
				curFilename.SetAt(1, curTheater);
				curMixFile = FindFileInMix(curFilename, &curMixTheater);
				if (curMixFile)
					return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(curTheater), forcedPalette ? forcedPalette : unitOrIsoPalette);
			}
		}
	}
	// Phase 2: try again even if isNewTheater is not true but the first char signals it should support theaters
	if (firstCharSupportsTheater)
	{
		/*curFilename.SetAt(1, preferred_theat);
		curMixFile = FindFileInMix(curFilename, &curMixTheater);
		if (curMixFile)
			return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(preferred_theat), GetUnitPalette(preferred_theat));*/
		for (int t = 0; curTheater = kTheatersToTry[t]; ++t)
		{
			curFilename.SetAt(1, curTheater);
			curMixFile = FindFileInMix(curFilename, &curMixTheater);
			if (curMixFile)
				return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(curTheater), forcedPalette ? forcedPalette : unitOrIsoPalette);
		}
	}
	// Phase 3: try unchanged filename
	curFilename = image + ".shp";
	curMixFile = FindFileInMix(curFilename, &curMixTheater);
	if (curMixFile)
		return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(preferred_theat), forcedPalette ? forcedPalette : unitOrIsoPalette);
	// Phase 4: try lowercase and otherwise unchanged filename
	curFilename = image + ".shp";
	curFilename.MakeLower();
	curMixFile = FindFileInMix(curFilename, &curMixTheater);
	if (curMixFile)
		return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(preferred_theat), forcedPalette ? forcedPalette : unitOrIsoPalette);
	// Phase 5: try with .tem, .sno etc endings with preferred theater
	for (int t = 0; !(curSuffix = kSuffixesToTry[t]).IsEmpty(); ++t)
	{
		curFilename = image + curSuffix;
		curFilename.MakeLower();
		curMixFile = FindFileInMix(curFilename, &curMixTheater);
		if (curMixFile)
			return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(suffixToTheat(curSuffix)), forcedPalette ? forcedPalette : GetIsoPalette(suffixToTheat(curSuffix)));
	}
	// Phase 6: try with theater in 2nd char even if first char does not indicate support
	curFilename = image + ".shp";
	for (int t = 0; curTheater = kTheatersToTry[t]; ++t)
	{
		curFilename.SetAt(1, curTheater);
		curMixFile = FindFileInMix(curFilename, &curMixTheater);
		if (curMixFile)
			return FindShpResult(curMixFile, curMixTheater, curFilename, toTheaterChar(curTheater), forcedPalette ? forcedPalette : GetIsoPalette(curTheater));
	}

	return std::nullopt;
}

int lepton_to_screen_y(int leptons)
{
	return leptons * f_y / 256;
}

BOOL CLoading::LoadUnitGraphic(LPCTSTR lpUnittype)
{
	errstream << "Loading: " << lpUnittype << endl;
	errstream.flush();

	last_succeeded_operation=10;

	CString _rules_image; // the image used
	CString filename; // filename of the image
	char theat=cur_theat; // standard theater char is t (Temperat). a is snow.

	BOOL bAlwaysSetChar=FALSE; // second char is always theater, even if NewTheater not specified!
	WORD wStep=1; // step is 1 for infantry, buildings, etc, and for shp vehicles it specifies the step rate between every direction
	WORD wStartWalkFrame=0; // for examply cyborg reaper has another walk starting frame
	int iTurretOffset=0; // used for centering y pos of turret (if existing) (for vehicles)
	const BOOL bStructure=rules.sections["BuildingTypes"].FindValue(lpUnittype)>=0; // is this a structure?
	const BOOL bVehicle = rules.sections["VehicleTypes"].FindValue(lpUnittype) >= 0; // is this a structure?

	BOOL bPowerUp=rules.sections[lpUnittype].values["PowersUpBuilding"]!="";

		
	CIsoView& v=*((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;

	_rules_image=lpUnittype;
	if(rules.sections[lpUnittype].values.find("Image")!=rules.sections[lpUnittype].values.end())
		_rules_image=rules.sections[lpUnittype].values["Image"];

	CString _art_image = _rules_image;
	if(art.sections[_rules_image].values.find("Image")!=art.sections[_rules_image].values.end())
	{
		if(!isTrue(g_data.sections["IgnoreArtImage"].values[_rules_image]))
			_art_image=art.sections[_rules_image].values["Image"];
	}

	const CString& image = _art_image;
	const auto& rulesSection = rules.sections[lpUnittype];
	const auto& artSection = art.sections[image];

	if(!isTrue(art.sections[image].values["Voxel"])) // is it a shp graphic?
	{
		try
		{

		auto shp = FindUnitShp(image, cur_theat, artSection);
		if (!shp)
		{
			errstream << "Building SHP in theater " << cur_theat << " not found: " << image << endl;
			errstream.flush();
			missingimages[lpUnittype] = TRUE;
			return FALSE;
		}

		filename = shp->filename;
		HTSPALETTE hPalette = shp->palette;
		const auto hShpMix = shp->mixfile;
		theat = static_cast<char>(shp->theat);
		auto limited_to_theater = isTrue(artSection.GetValueByName("TerrainPalette")) ? shp->mixfile_theater : TheaterChar::None;

		
		SHPHEADER head;
		int superanim_1_zadjust=0;
		int superanim_2_zadjust=0;
		int superanim_3_zadjust=0;
		int superanim_4_zadjust=0;

		CString turretanim_name;
		CString turretanim_filename;
		CString barrelanim_filename;
		BYTE* bib=NULL;
		SHPHEADER bib_h;
		BYTE* activeanim=NULL;
		SHPHEADER activeanim_h;
		BYTE* idleanim=NULL;
		SHPHEADER idleanim_h;
		BYTE* activeanim2=NULL;
		SHPHEADER activeanim2_h;
		BYTE* activeanim3=NULL;
		SHPHEADER activeanim3_h;
		BYTE* superanim1=NULL;
		SHPHEADER superanim1_h;
		BYTE* superanim2=NULL;
		SHPHEADER superanim2_h;
		BYTE* superanim3=NULL;
		SHPHEADER superanim3_h;
		BYTE* superanim4=NULL;
		SHPHEADER superanim4_h;
		BYTE* specialanim1=NULL;
		SHPHEADER specialanim1_h;
		BYTE* specialanim2=NULL;
		SHPHEADER specialanim2_h;
		BYTE* specialanim3=NULL;
		SHPHEADER specialanim3_h;
		BYTE* specialanim4=NULL;
		SHPHEADER specialanim4_h;
		BYTE** lpT=NULL;
		SHPHEADER* lpT_h=NULL;
		std::vector<BYTE> turretColors[8];
		std::vector<BYTE> turretLighting[8];
		std::vector<BYTE> vxlBarrelColors[8];
		std::vector<BYTE> vxlBarrelLighting[8];
		SHPHEADER turrets_h[8];
		SHPIMAGEHEADER turretinfo[8];
		SHPHEADER barrels_h[8];
		SHPIMAGEHEADER barrelinfo[8];
	
			
		if(hShpMix>0)
		{		
				
			std::string shp_mixfile;
			if (FSunPackLib::XCC_GetMixName(hShpMix, shp_mixfile))
			{
				errstream << (LPCTSTR)filename << " found ";
				errstream << " in " << shp_mixfile << endl;
				errstream.flush();
			}
			//hShpMix=20;

			    
			if(rules.sections[lpUnittype].values["Bib"]!="no") // seems to be ignored by TS, art.ini overwrites???
			{			
				LoadBuildingSubGraphic("BibShape", artSection, bAlwaysSetChar, theat, hShpMix, bib_h, bib);
			}

			LoadBuildingSubGraphic("ActiveAnim", artSection, bAlwaysSetChar, theat, hShpMix, activeanim_h, activeanim);
			LoadBuildingSubGraphic("IdleAnim", artSection, bAlwaysSetChar, theat, hShpMix, idleanim_h, idleanim);
			LoadBuildingSubGraphic("ActiveAnim2", artSection, bAlwaysSetChar, theat, hShpMix, activeanim2_h, activeanim2);
			LoadBuildingSubGraphic("ActiveAnim3", artSection, bAlwaysSetChar, theat, hShpMix, activeanim3_h, activeanim3);
			if (!isTrue(g_data.sections["IgnoreSuperAnim1"].values[image]))
				LoadBuildingSubGraphic("SuperAnim", artSection, bAlwaysSetChar, theat, hShpMix, superanim1_h, superanim1);
			if (!isTrue(g_data.sections["IgnoreSuperAnim2"].values[image]))
				LoadBuildingSubGraphic("SuperAnimTwo", artSection, bAlwaysSetChar, theat, hShpMix, superanim2_h, superanim2);
			if (!isTrue(g_data.sections["IgnoreSuperAnim3"].values[image]))
				LoadBuildingSubGraphic("SuperAnimThree", artSection, bAlwaysSetChar, theat, hShpMix, superanim3_h, superanim3);
			if (!isTrue(g_data.sections["IgnoreSuperAnim4"].values[image]))
				LoadBuildingSubGraphic("SuperAnimFour", artSection, bAlwaysSetChar, theat, hShpMix, superanim4_h, superanim4);
			LoadBuildingSubGraphic("SpecialAnim", artSection, bAlwaysSetChar, theat, hShpMix, specialanim1_h, specialanim1);
			LoadBuildingSubGraphic("SpecialAnimTwo", artSection, bAlwaysSetChar, theat, hShpMix, specialanim2_h, specialanim2);
			LoadBuildingSubGraphic("SpecialAnimThree", artSection, bAlwaysSetChar, theat, hShpMix, specialanim3_h, specialanim3);
			LoadBuildingSubGraphic("SpecialAnimFour", artSection, bAlwaysSetChar, theat, hShpMix, specialanim4_h, specialanim4);

			BOOL bVoxelTurret = FALSE;
			BOOL bVoxelBarrel = FALSE;

			FSunPackLib::VoxelNormalClass vnc = FSunPackLib::VoxelNormalClass::Unknown;

			if (isTrue(rules.sections[image].values["Turret"]))
			{
				turretanim_name = rules.sections[image].values["TurretAnim"];
				auto vxl_turretanim_filename = turretanim_name.IsEmpty() ? image + "tur.vxl" : turretanim_name + ".vxl";
				auto vxl_barrelanim_filename = image + "barl.vxl";
				if (art.sections[turretanim_name].values.find("Image") != art.sections[turretanim_name].values.end())
					vxl_turretanim_filename = art.sections[turretanim_name].values["Image"] + ".vxl";

				if (bStructure && turretanim_name.GetLength() > 0 && isFalse(rules.sections[image].values["TurretAnimIsVoxel"]))
				{
					turretanim_filename = turretanim_name + ".shp";
					if (art.sections[turretanim_name].values.find("Image") != art.sections[turretanim_name].values.end()) turretanim_filename = art.sections[turretanim_name].values["Image"] + ".shp";

					if (isTrue(artSection.GetValueByName("NewTheater")))
					{
						auto tmp = turretanim_filename;
						tmp.SetAt(1, theat);
						if (FSunPackLib::XCC_DoesFileExist(tmp, hShpMix))
							turretanim_filename = tmp;
					}


					FSunPackLib::SetCurrentSHP(turretanim_filename, hShpMix);
					FSunPackLib::XCC_GetSHPHeader(&head);

					int iStartTurret = 0;
					const WORD wAnimCount = 4; // anims between each "normal" direction, seems to be hardcoded

					int i;

					for (i = 0;i < 8;i++)
					{
						if (iStartTurret + i * wAnimCount < head.c_images)
						{
							FSunPackLib::XCC_GetSHPImageHeader(iStartTurret + i * wAnimCount, &turretinfo[i]);
							FSunPackLib::XCC_GetSHPHeader(&turrets_h[i]);
							FSunPackLib::LoadSHPImage(iStartTurret + i * wAnimCount, turretColors[i]);
							turretLighting[i].clear();
						}

					}
				}
				else if (
					(bStructure && turretanim_name.GetLength() > 0 && isTrue(rules.sections[image].values["TurretAnimIsVoxel"]))
					|| (!bStructure && (FindFileInMix(vxl_turretanim_filename) || FindFileInMix(vxl_barrelanim_filename)))
				)
				{
					turretanim_filename = vxl_turretanim_filename;
					barrelanim_filename = vxl_barrelanim_filename;

					HMIXFILE hVXL = FindFileInMix(vxl_turretanim_filename);
					HMIXFILE hBarl = FindFileInMix(vxl_barrelanim_filename);

					if (artSection.values.find("TurretOffset") != art.sections[image].values.end())
						iTurretOffset = atoi(art.sections[image].values["TurretOffset"]);
					Vec3f turretModelOffset(iTurretOffset / 6.0f, 0.0f, 0.0f);


					if (hVXL)
					{
						bVoxelTurret = TRUE;

						if (
							FSunPackLib::SetCurrentVXL(turretanim_filename, hVXL)
							)
						{
							// we assume the voxel normal class is always the same for the combined voxels
							FSunPackLib::GetVXLSectionInfo(0, vnc);

							int i;

							for (i = 0;i < 8;i++)
							{
								float r_x, r_y, r_z;


								const int dir = bVehicle ? ((i + 1) % 8) : i;
								r_x = 300;
								r_y = 0;
								r_z = 45 * dir + 90;

								// convert
								const double pi = 3.141592654;
								const Vec3f rotation(r_x / 180.0f * pi, r_y / 180.0f * pi, r_z / 180.0f * pi);


								RECT r;
								int center_x, center_y;
								if (!
									FSunPackLib::LoadVXLImage(*m_voxelNormalTables, lightDirection, rotation, turretModelOffset, turretColors[i], turretLighting[i], &center_x, &center_y, 0, 0, 0, 0, 0, &r)
									)
								{

								}
								else
								{
									turrets_h[i].cx = r.right - r.left;
									turrets_h[i].cy = r.bottom - r.top;

									turretinfo[i].x = center_x;
									turretinfo[i].y = center_y;
									turretinfo[i].cx = r.right - r.left;
									turretinfo[i].cy = r.bottom - r.top;

								}

							}
						}
					}
					if (hBarl)
					{
						bVoxelBarrel = TRUE;

						if (
							FSunPackLib::SetCurrentVXL(barrelanim_filename, hBarl)
							)
						{
							// we assume the voxel normal class is always the same for the combined voxels
							FSunPackLib::GetVXLSectionInfo(0, vnc);

							int i;

							for (i = 0;i < 8;i++)
							{
								float r_x, r_y, r_z;


								const int dir = bVehicle ? ((i + 1) % 8) : i;
								r_x = 300;
								r_y = 0;
								r_z = 45 * dir + 90;

								// convert
								const double pi = 3.141592654;
								const Vec3f rotation(r_x / 180.0f * pi, r_y / 180.0f * pi, r_z / 180.0f * pi);


								RECT r;
								int center_x, center_y;
								if (!
									FSunPackLib::LoadVXLImage(*m_voxelNormalTables, lightDirection, rotation, turretModelOffset, vxlBarrelColors[i], vxlBarrelLighting[i], &center_x, &center_y, atoi(rules.sections[image].values["TurretAnimZAdjust"]), 0, 0, 0, 0, &r)
									)
								{

								}
								else
								{
									barrels_h[i].cx = r.right - r.left;
									barrels_h[i].cy = r.bottom - r.top;

									barrelinfo[i].x = center_x;
									barrelinfo[i].y = center_y;
									barrelinfo[i].cx = r.right - r.left;
									barrelinfo[i].cy = r.bottom - r.top;

								}

							}
						}
					}
				}
			}


			if(art.sections[image].values.find("WalkFrames")!=art.sections[image].values.end())
				wStep=atoi(art.sections[image].values["WalkFrames"]);
			if(art.sections[image].values.find("StartWalkFrame")!=art.sections[image].values.end())
				wStartWalkFrame=atoi(art.sections[image].values["StartWalkFrame"]);
			
			if(art.sections[image].values["Palette"]=="lib")
				hPalette=m_hPalLib;

			BOOL bSuccess=FSunPackLib::SetCurrentSHP(filename, hShpMix);
			if(
			!bSuccess
			)
			{
				filename=image+".sno";
				if(cur_theat=='T' || cur_theat=='U' /* || cur_theat=='N' ? */) hPalette=m_hPalIsoTemp;
				HMIXFILE hShpMix=FindFileInMix(filename);
				bSuccess=FSunPackLib::SetCurrentSHP(filename, hShpMix);
				
				if(!bSuccess)
				{
					missingimages[lpUnittype]=TRUE;	
				}
			}
			
			if(bSuccess)
			{
			
			FSunPackLib::XCC_GetSHPHeader(&head);
			int i;
			int maxPics=head.c_images;
			if(maxPics>8) maxPics=8; // we only need 8 pictures for every direction!
			if(bStructure && !bPowerUp && !isTrue(rules.sections[image].values["Turret"])) maxPics=1;
			if(bVoxelTurret) maxPics=8;
			


			if(!bStructure && rules.sections[image].values["Turret"]=="yes")
			{
				int iStartTurret=wStartWalkFrame+8*wStep;
				const WORD wAnimCount=4; // anims between each "normal" direction, seems to be hardcoded
				
				int i;

				for(i=0;i<8;i++)
				{
					if(iStartTurret+i*wAnimCount<head.c_images)
					{
						FSunPackLib::XCC_GetSHPImageHeader(iStartTurret+i*wAnimCount, &turretinfo[i]);
						FSunPackLib::XCC_GetSHPHeader(&turrets_h[i]);
						FSunPackLib::LoadSHPImage(iStartTurret+i*wAnimCount, turretColors[i]);
					}
					
				}
			}

			
			
			// create an array of pointers to directdraw surfaces
			lpT=new(BYTE*[maxPics]);
			memset(lpT, 0, sizeof(BYTE)*maxPics);
			std::vector<std::vector<BYTE>> lighting(maxPics);
			std::vector<SHPIMAGEHEADER> shp_image_headers(maxPics);
			
			if(bVoxelTurret && bStructure)
			{
				for(i=0;i<maxPics; i++)
				{
					FSunPackLib::LoadSHPImage(0, 1, &lpT[i]);
					FSunPackLib::XCC_GetSHPImageHeader(0, &shp_image_headers[i]);
				}
			}
			else if(wStep==1 && (rules.sections[lpUnittype].values["PowersUpBuilding"].GetLength()==0 || !isTrue(rules.sections[lpUnittype].values["Turret"]))) 
			{ // standard case...

				FSunPackLib::LoadSHPImage(wStartWalkFrame, maxPics, lpT);
				for (int i = 0; i < maxPics; ++i)
					FSunPackLib::XCC_GetSHPImageHeader(wStartWalkFrame + i, &shp_image_headers[i]);
			
			}
			else if(rules.sections[lpUnittype].values["PowersUpBuilding"].GetLength()!=0 && isTrue(rules.sections[lpUnittype].values["Turret"]))
			{ // a "real" turret (vulcan cannon, etc...)
				for(i=0;i<maxPics; i++)
				{
					FSunPackLib::LoadSHPImage(i*4, 1, &lpT[i]);
					FSunPackLib::XCC_GetSHPImageHeader(i*4, &shp_image_headers[i]);
				}
			}
			else	 
			{ 
				// walk frames used
				for(i=0;i<maxPics; i++)
				{
					const int dir = bVehicle ? ((i + 1) % 8) : i;
					const int pic_in_file = dir * wStep + wStartWalkFrame;
					FSunPackLib::LoadSHPImage(pic_in_file, 1, &lpT[i]);
					FSunPackLib::XCC_GetSHPImageHeader(pic_in_file, &shp_image_headers[i]);
				}
			}
 
			for(i=0; i<maxPics; i++)
			{
				int pic_in_file=i;
				if(bStructure && bVoxelTurret) pic_in_file=0;
				SHPIMAGEHEADER imghead = shp_image_headers[i];
				//FSunPackLib::XCC_GetSHPImageHeader(pic_in_file, &imghead);

				if(bib!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, bib, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, 0, head.cx, head.cy, bib, bib_h.cx, bib_h.cy);

					imghead.cx=head.cx-imghead.x; // update size of main graphic
					imghead.cy=head.cy-imghead.y;

				}

				if(activeanim!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, activeanim, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, 0, head.cx, head.cy, activeanim, activeanim_h.cx, activeanim_h.cy);

					
				}

				if(idleanim!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, idleanim, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, 0, head.cx, head.cy, idleanim, idleanim_h.cx, idleanim_h.cy);

					
				}

				if(activeanim2!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, activeanim2, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, 0, head.cx, head.cy, activeanim2, activeanim2_h.cx, activeanim2_h.cy);
					
				}

				if(activeanim3!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, activeanim3, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, 0, head.cx, head.cy, activeanim3, activeanim3_h.cx, activeanim3_h.cy);
						
				}
				
				if(superanim1!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, superanim1, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, superanim_1_zadjust, head.cx, head.cy, superanim1, superanim1_h.cx, superanim1_h.cy);

					
				}

				if(superanim2!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, superanim2, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, superanim_2_zadjust, head.cx, head.cy, superanim2, superanim2_h.cx, superanim2_h.cy);

					
				}

				if(superanim3!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, superanim3, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, superanim_3_zadjust, head.cx, head.cy, superanim3, superanim3_h.cx, superanim3_h.cy);

					
				}

				if(superanim4!=NULL && strcmp(lpUnittype, "YAGNTC")!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, superanim4, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, superanim_4_zadjust, head.cx, head.cy, superanim4, superanim4_h.cx, superanim4_h.cy);

					
				}

				if(specialanim1!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, specialanim1, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, 0, head.cx, head.cy, specialanim1, specialanim1_h.cx, specialanim1_h.cy);

					
				}

				if(specialanim2!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, specialanim2, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, 0, head.cx, head.cy, specialanim2, specialanim2_h.cx, specialanim2_h.cy);
					
				}

				if(specialanim3!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, specialanim3, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, 0, head.cx, head.cy, specialanim3, specialanim3_h.cx, specialanim3_h.cy);

					
				}

				if(specialanim4!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					//lpT[i]->Blt(NULL, specialanim4, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);
					Blit_Pal(lpT[i], 0, 0, head.cx, head.cy, specialanim4, specialanim4_h.cx, specialanim4_h.cy);
					
				}
				
				if (!vxlBarrelLighting[i].empty() || !turretLighting[i].empty())
					lighting[i].resize(head.cx * head.cy, 46);  // value needs to lead to 1.0 lighting

				// barrels hidden behind turret:
				if(!vxlBarrelColors[i].empty() && (i == 1 || i == 0 || i == 7))
				{
					DDSURFACEDESC2 ddsd;
					memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
					ddsd.dwSize = sizeof(DDSURFACEDESC2);
					ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
					ddsd.dwWidth = barrels_h[i].cx;
					ddsd.dwHeight = barrels_h[i].cy;

					int XMover, YMover;
					char c[50];
					itoa(i, c, 10);
#ifdef RA2_MODE
					XMover = atoi(g_data.sections["BuildingVoxelBarrelsRA2"].values[(CString)lpUnittype + "X"]);
					YMover = atoi(g_data.sections["BuildingVoxelBarrelsRA2"].values[(CString)lpUnittype + "Y"]);
					XMover += atoi(g_data.sections["BuildingVoxelBarrelsRA2"].values[(CString)lpUnittype + (CString)"X" + c]);
					YMover += atoi(g_data.sections["BuildingVoxelBarrelsRA2"].values[(CString)lpUnittype + (CString)"Y" + c]);
#else
					XMover = atoi(g_data.sections["BuildingVoxelBarrels"].values[(CString)lpUnittype + "X"]);
					YMover = atoi(g_data.sections["BuildingVoxelBarrels"].values[(CString)lpUnittype + "Y"]);
#endif

					RECT srcRect, destRect;

					int mx = head.cx / 2 + atoi(rules.sections[image].values["TurretAnimX"]) - barrelinfo[i].x;
					int my = head.cy / 2 + atoi(rules.sections[image].values["TurretAnimY"]) - barrelinfo[i].y;

					srcRect.top = 0;
					srcRect.left = 0;
					srcRect.right = ddsd.dwWidth;
					srcRect.bottom = ddsd.dwHeight;
					destRect.top = YMover + my;
					destRect.left = XMover + mx;
					destRect.right = destRect.left + ddsd.dwWidth;
					destRect.bottom = destRect.top + ddsd.dwHeight;


					errstream << "vxl barrel: " << i << " size: " << ddsd.dwWidth << " " << ddsd.dwHeight << " at " << destRect.left << " " << destRect.top << endl;
					errstream.flush();
					Blit_PalD(lpT[i], destRect, vxlBarrelColors[i].data(), srcRect, ddsd.dwWidth, head.cx, ddsd.dwHeight, head.cy);
					Blit_PalD(lighting[i].data(), destRect, vxlBarrelLighting[i].data(), srcRect, ddsd.dwWidth, head.cx, ddsd.dwHeight, head.cy, vxlBarrelColors[i].data());
									
				}
				
				
				if(!turretColors[i].empty())
				{
					DDSURFACEDESC2 ddsd;
					memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
					ddsd.dwSize=sizeof(DDSURFACEDESC2);
					ddsd.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
					//turrets[i]->GetSurfaceDesc(&ddsd);
					// replace DX code:
					ddsd.dwWidth=turrets_h[i].cx;
					ddsd.dwHeight=turrets_h[i].cy;

					int XMover, YMover;
					char c[50];
					itoa(i, c, 10);
#ifdef RA2_MODE
					XMover = atoi(g_data.sections["BuildingVoxelTurretsRA2"].values[(CString)lpUnittype + "X"]);
					YMover = atoi(g_data.sections["BuildingVoxelTurretsRA2"].values[(CString)lpUnittype + "Y"]);
					XMover += atoi(g_data.sections["BuildingVoxelTurretsRA2"].values[(CString)lpUnittype + (CString)"X" + c]);
					YMover += atoi(g_data.sections["BuildingVoxelTurretsRA2"].values[(CString)lpUnittype + (CString)"Y" + c]);
#else
					XMover = atoi(g_data.sections["BuildingVoxelTurrets"].values[(CString)lpUnittype + "X"]);
					YMover = atoi(g_data.sections["BuildingVoxelTurrets"].values[(CString)lpUnittype + "Y"]);
#endif

					RECT srcRect, destRect;
					
					if (bVoxelTurret)
					{
						int mx = head.cx / 2 + atoi(rules.sections[image].values["TurretAnimX"]) - turretinfo[i].x;
						int my = head.cy / 2 + atoi(rules.sections[image].values["TurretAnimY"]) - turretinfo[i].y;						

						srcRect.top=0;
						srcRect.left=0;
						srcRect.right=ddsd.dwWidth;
						srcRect.bottom=ddsd.dwHeight;
						destRect.top=YMover+my;
						destRect.left=XMover+mx;
						destRect.right=destRect.left+ddsd.dwWidth;;
						destRect.bottom=destRect.top+ddsd.dwHeight;
				
					}
					else // !bVoxelTurret
					{

						int mx = atoi(rules.sections[image].values["TurretAnimX"]);
						int my = atoi(rules.sections[image].values["TurretAnimY"]);//+atoi(rules.sections[image].values["barrelAnimZAdjust"]);



						srcRect.top = 0;
						srcRect.left = 0;
						srcRect.right = turrets_h[i].cx;
						srcRect.bottom = turrets_h[i].cy;
						destRect.top = YMover + my;
						destRect.left = XMover + mx;
						destRect.right = destRect.left + srcRect.right;
						destRect.bottom = destRect.top + srcRect.bottom;
					}

					errstream << "vxl turret: " << i << " size: " << ddsd.dwWidth << " " << ddsd.dwHeight << " at " << destRect.left << " " << destRect.top  << endl;
					errstream.flush(); 
					Blit_PalD(lpT[i], destRect, turretColors[i].data(), srcRect, ddsd.dwWidth, head.cx, ddsd.dwHeight, head.cy);
					Blit_PalD(lighting[i].data(), destRect, turretLighting[i].data(), srcRect, ddsd.dwWidth, head.cx, ddsd.dwHeight, head.cy, turretColors[i].data());
									
				}

				// barrels in front of turret
				if(!vxlBarrelColors[i].empty() && i!=1 && i!=0 && i!=7)
				{
					DDSURFACEDESC2 ddsd;
					memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
					ddsd.dwSize = sizeof(DDSURFACEDESC2);
					ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
					ddsd.dwWidth = barrels_h[i].cx;
					ddsd.dwHeight = barrels_h[i].cy;

					int XMover, YMover;
					char c[50];
					itoa(i, c, 10);
#ifdef RA2_MODE
					XMover = atoi(g_data.sections["BuildingVoxelBarrelsRA2"].values[(CString)lpUnittype + "X"]);
					YMover = atoi(g_data.sections["BuildingVoxelBarrelsRA2"].values[(CString)lpUnittype + "Y"]);
					XMover += atoi(g_data.sections["BuildingVoxelBarrelsRA2"].values[(CString)lpUnittype + (CString)"X" + c]);
					YMover += atoi(g_data.sections["BuildingVoxelBarrelsRA2"].values[(CString)lpUnittype + (CString)"Y" + c]);
#else
					XMover = atoi(g_data.sections["BuildingVoxelBarrels"].values[(CString)lpUnittype + "X"]);
					YMover = atoi(g_data.sections["BuildingVoxelBarrels"].values[(CString)lpUnittype + "Y"]);
#endif

					RECT srcRect, destRect;

					int mx = head.cx / 2 + atoi(rules.sections[image].values["TurretAnimX"]) - barrelinfo[i].x;
					int my = head.cy / 2 + atoi(rules.sections[image].values["TurretAnimY"]) - barrelinfo[i].y;

					srcRect.top = 0;
					srcRect.left = 0;
					srcRect.right = ddsd.dwWidth;
					srcRect.bottom = ddsd.dwHeight;
					destRect.top = YMover + my;
					destRect.left = XMover + mx;
					destRect.right = destRect.left + ddsd.dwWidth;
					destRect.bottom = destRect.top + ddsd.dwHeight;


					errstream << "vxl barrel: " << i << " size: " << ddsd.dwWidth << " " << ddsd.dwHeight << " at " << destRect.left << " " << destRect.top << endl;
					errstream.flush();
					Blit_PalD(lpT[i], destRect, vxlBarrelColors[i].data(), srcRect, ddsd.dwWidth, head.cx, ddsd.dwHeight, head.cy);
					Blit_PalD(lighting[i].data(), destRect, vxlBarrelLighting[i].data(), srcRect, ddsd.dwWidth, head.cx, ddsd.dwHeight, head.cy, vxlBarrelColors[i].data());

									
				}

				if(!bPowerUp && i!=0 && (imghead.unknown==0 && !isTrue(g_data.sections["Debug"].values["IgnoreSHPImageHeadUnused"])) && bStructure)
				{
					if(lpT[i]) delete[] lpT[i];
					lpT[i]=NULL;
				}
				else
				{
					char ic[50];
					itoa(i, ic, 10);

					PICDATA p;
					p.pic=lpT[i];
					if (std::find_if(lighting[i].begin(), lighting[i].end(), [](const BYTE b) { return b != 255; }) != lighting[i].end())
						p.lighting = std::shared_ptr<std::vector<BYTE>>(new std::vector<BYTE>(std::move(lighting[i])));
					else
						lighting[i].clear();
					p.vborder=new(VBORDER[head.cy]);
					int k;
					for(k=0;k<head.cy;k++)
					{
						int l,r;
						GetDrawBorder(lpT[i], head.cx, k, l, r, 0);
						p.vborder[k].left=l;
						p.vborder[k].right=r;
					}

					if(hPalette==m_hPalIsoTemp || hPalette==m_hPalIsoUrb || hPalette==m_hPalIsoSnow || hPalette==m_hPalIsoUbn || hPalette==m_hPalIsoDes || hPalette==m_hPalIsoLun) p.pal=iPalIso;
					if(hPalette==m_hPalTemp || hPalette==m_hPalUrb || hPalette==m_hPalSnow || hPalette==m_hPalUbn || hPalette==m_hPalLun || hPalette==m_hPalDes) p.pal=iPalTheater;
					if(hPalette==m_hPalUnitTemp || hPalette==m_hPalUnitUrb || hPalette==m_hPalUnitSnow || hPalette==m_hPalUnitDes || hPalette==m_hPalUnitLun || hPalette==m_hPalUnitUbn) p.pal=iPalUnit;
					if(hPalette==m_hPalLib) p.pal=iPalLib;
					
					p.x=imghead.x;
					p.y=imghead.y;
					p.wHeight=imghead.cy;
					p.wWidth=imghead.cx;
					p.wMaxWidth=head.cx;
					p.wMaxHeight=head.cy;
					p.bType=PICDATA_TYPE_SHP;
					p.bTerrain=limited_to_theater;
					
					pics[image+ic]=p;

					//errstream << " --> finished as " << (LPCSTR)(image+ic) << endl;
					//errstream.flush();
				}
				
			
			}
			
			delete[] lpT;

			
			if(bib) delete[] bib;
			if(activeanim) delete[] activeanim;
			if(idleanim) delete[] idleanim;
			if(activeanim2) delete[] activeanim2;
			if(activeanim3) delete[] activeanim3;
			if(superanim1) delete[] superanim1;
			if(superanim2) delete[] superanim2;
			if(superanim3) delete[] superanim3;
			if(superanim4) delete[] superanim4;
			if(specialanim1) delete[] specialanim1;
			if(specialanim2) delete[] specialanim2;
			if(specialanim3) delete[] specialanim3;
			if(specialanim4) delete[] specialanim4;
			
			//for(i=0;i<8;i++)
			//	if(turrets[i]) delete[] turrets[i];
			
			}

			//errstream << " --> Finished" << endl;
			//errstream.flush();
		}
		
		else
		{
			errstream << "File in theater " << cur_theat << " not found: " << (LPCTSTR)filename << endl;
			errstream.flush();

			missingimages[lpUnittype]=TRUE;			
		}

		}
		catch(...)
		{
			errstream << " exception " << endl;
			errstream.flush();
		}
	
		
	}
	else
	{
		filename=image+".vxl";

		HMIXFILE hMix=FindFileInMix(filename);
		if(hMix==FALSE)
		{
			missingimages[lpUnittype]=TRUE;
			return FALSE;
		}

		int XMover, YMover;
#ifdef RA2_MODE
		XMover=atoi(g_data.sections["VehicleVoxelTurretsRA2"].values[(CString)lpUnittype+"X"]);
		YMover=atoi(g_data.sections["VehicleVoxelTurretsRA2"].values[(CString)lpUnittype+"Y"]);
#else
		XMover=atoi(g_data.sections["VehicleVoxelTurrets"].values[(CString)lpUnittype+"X"]);
		YMover=atoi(g_data.sections["VehicleVoxelTurrets"].values[(CString)lpUnittype+"Y"]);
#endif

		if (artSection.values.find("TurretOffset") != art.sections[image].values.end())
			iTurretOffset = atoi(art.sections[image].values["TurretOffset"]);
		
		int i;

		for(i=0;i<8;i++)
		{
			float r_x,r_y,r_z;
			

			r_x = 300;
			r_y=0;
			r_z=45*i+90;
			
			//r_x = 0;
			//r_y = 0;
			//r_z = 0;

			// convert
			const double pi = 3.141592654;
			Vec3f rotation(r_x / 180.0f * pi, r_y / 180.0f * pi, r_z / 180.0f * pi);
			Vec3f turretModelOffset(iTurretOffset / 6.0f, 0.0f, 0.0f);
			

			std::vector<BYTE> colors;
			std::shared_ptr<std::vector<BYTE>> pLighting(new std::vector<BYTE>);
			auto& lighting = *pLighting;
			std::vector<BYTE> turretColors;
			std::vector<BYTE> turretNormals;
			std::vector<BYTE> barrelColors;
			std::vector<BYTE> barrelNormals;
			RECT lprT;
			RECT lprB;
			int turret_x,turret_y,turret_x_zmax,turret_y_zmax,barrel_x,barrel_y;

			if(isTrue(rules.sections[lpUnittype].values["Turret"]))
			{
				if(FSunPackLib::SetCurrentVXL(image+"tur.vxl", hMix))
				{
					FSunPackLib::LoadVXLImage(*m_voxelNormalTables, lightDirection, rotation, turretModelOffset, turretColors, turretNormals, &turret_x,&turret_y, 0, &turret_x_zmax, &turret_y_zmax,-1,-1,&lprT);
				}
				if(FSunPackLib::SetCurrentVXL(image+"barl.vxl", hMix))
				{
					FSunPackLib::LoadVXLImage(*m_voxelNormalTables, lightDirection, rotation, turretModelOffset, barrelColors, barrelNormals, &barrel_x,&barrel_y, 0, NULL, NULL, 0, 0, &lprB);
				}	
			}


			if(!FSunPackLib::SetCurrentVXL(filename, hMix))
			{
				return FALSE;
			}

			

			int xcenter,ycenter,xcenter_zmax,ycenter_zmax;

			RECT r;

			if(!
			FSunPackLib::LoadVXLImage(*m_voxelNormalTables, lightDirection, rotation, Vec3f(), colors, lighting, &xcenter, &ycenter,0, &xcenter_zmax, &ycenter_zmax,-1,-1,&r)
			)
			{
				return FALSE;
			}

			FSunPackLib::VoxelNormalClass vnc = FSunPackLib::VoxelNormalClass::Unknown;
			FSunPackLib::GetVXLSectionInfo(0, vnc);  // we assume the normal class for all voxels sections and turrets or barrels is the same

			DDSURFACEDESC2 ddsd;
			memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
			ddsd.dwSize=sizeof(DDSURFACEDESC2);
			ddsd.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
			ddsd.dwWidth=r.right-r.left;
			ddsd.dwHeight=r.bottom-r.top;

			//lpT->GetSurfaceDesc(&ddsd);

			// turret
			if(turretColors.size())
			{
				DDSURFACEDESC2 ddsdT;
				memset(&ddsdT, 0, sizeof(DDSURFACEDESC2));
				ddsdT.dwSize=sizeof(DDSURFACEDESC2);
				ddsdT.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
				ddsdT.dwWidth=lprT.right-lprT.left;
				ddsdT.dwHeight=lprT.bottom-lprT.top;
				//lpTurret->GetSurfaceDesc(&ddsdT);

				DDBLTFX fx;
				memset(&fx, 0, sizeof(DDBLTFX));
				fx.dwSize=sizeof(DDBLTFX);

				
				RECT srcRect, destRect;
				srcRect.left=0;					
				srcRect.right=ddsdT.dwWidth;
				destRect.left=xcenter - turret_x + XMover;
				destRect.right=destRect.left+ddsdT.dwWidth;
				srcRect.top=0;					
				srcRect.bottom=ddsdT.dwHeight;
				destRect.top=ycenter - turret_y  + YMover;
				destRect.bottom=destRect.top+ddsdT.dwHeight;

				errstream << destRect.left << " " << destRect.top << endl;
				errstream.flush();

				Blit_PalD(colors.data(), destRect, turretColors.data(), srcRect, ddsdT.dwWidth, ddsd.dwWidth, ddsdT.dwHeight, ddsd.dwHeight);
				Blit_PalD(lighting.data(), destRect, turretNormals.data(), srcRect, ddsdT.dwWidth, ddsd.dwWidth, ddsdT.dwHeight, ddsd.dwHeight, turretColors.data());
				//AssertNormals(turretColors, turretNormals);
				
			}

			// barrel
			if(barrelColors.size())
			{
				DDSURFACEDESC2 ddsdB;
				memset(&ddsdB, 0, sizeof(DDSURFACEDESC2));
				ddsdB.dwSize=sizeof(DDSURFACEDESC2);
				ddsdB.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
				ddsdB.dwWidth=lprB.right-lprB.left;
				ddsdB.dwHeight=lprB.bottom-lprB.top;
				//lpBarrel->GetSurfaceDesc(&ddsdB);
				
				DDSURFACEDESC2 ddsdT;
				memset(&ddsdT, 0, sizeof(DDSURFACEDESC2));
				ddsdT.dwSize=sizeof(DDSURFACEDESC2);
				ddsdT.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
				
				if(turretColors.size()) 
				{
					ddsdT.dwWidth=lprT.right-lprT.left;
					ddsdT.dwHeight=lprT.bottom-lprT.top;
					//lpTurret->GetSurfaceDesc(&ddsdT);
				}


				DDBLTFX fx;
				memset(&fx, 0, sizeof(DDBLTFX));
				fx.dwSize=sizeof(DDBLTFX);

				RECT srcRect, destRect;
				srcRect.left=0;					
				srcRect.right=ddsdB.dwWidth;
				destRect.left=xcenter-barrel_x+XMover;					
				destRect.right=destRect.left+ddsdB.dwWidth;
				srcRect.top=0;					
				srcRect.bottom=ddsdB.dwHeight;
				destRect.top=ycenter-barrel_y+YMover;					
				destRect.bottom=destRect.top+ddsdB.dwHeight;

				Blit_PalD(colors.data(), destRect, barrelColors.data(), srcRect, ddsdB.dwWidth, ddsd.dwWidth, ddsdB.dwHeight, ddsd.dwHeight);
				Blit_PalD(lighting.data(), destRect, barrelNormals.data(), srcRect, ddsdB.dwWidth, ddsd.dwWidth, ddsdB.dwHeight, ddsd.dwHeight, barrelColors.data());
				//AssertNormals(vxlBarrelColors, barrelNormals);
				
			}

			// all VXL, so every non-transparent area should have a normal
			//AssertNormals(colors, lighting);

			char ic[50];
			itoa(7-i, ic, 10);

			errstream << ddsd.dwWidth << " " << ddsd.dwHeight << "\n";
			PICDATA p;
			p.pic = new(BYTE[colors.size()]);
			memcpy(p.pic, colors.data(), colors.size());
			p.lighting = pLighting;
			p.normalClass = vnc;
			
			p.vborder=new(VBORDER[ddsd.dwHeight]);
			int k;
			for(k=0;k<ddsd.dwHeight;k++)
			{
				int l,r;
				GetDrawBorder(colors.data(), ddsd.dwWidth, k, l, r, 0);
				p.vborder[k].left=l;
				p.vborder[k].right=r;
			}

			//if(hPalette==m_hPalIsoTemp || hPalette==m_hPalIsoUrb || hPalette==m_hPalIsoSnow) p.pal=iPalIso;
			//if(hPalette==m_hPalTemp || hPalette==m_hPalUrb || hPalette==m_hPalSnow) p.pal=iPalTheater;
			//if(hPalette==m_hPalUnitTemp || hPalette==m_hPalUnitUrb || hPalette==m_hPalUnitSnow) p.pal=iPalUnit;
			//if(hPalette==m_hPalLib) p.pal=iPalLib;
			p.pal=iPalUnit;
			
			p.x=-xcenter;
			p.y=-ycenter;
			p.wHeight=ddsd.dwHeight;
			p.wWidth=ddsd.dwWidth;
			p.wMaxWidth=ddsd.dwWidth;
			p.wMaxHeight=ddsd.dwHeight;
			p.bType=PICDATA_TYPE_VXL;
			p.bTerrain=TheaterChar::None;
					
			pics[image+ic]=p;

			errstream << "vxl saved as " << (LPCSTR)image << (LPCSTR)ic << endl;
			errstream.flush();

			//delete[] lpT;
			
		}
		


	}

	

	return FALSE;	
}
void CLoading::LoadBuildingSubGraphic(const CString& subkey, const CIniFileSection& artSection, BOOL bAlwaysSetChar, char theat, HMIXFILE hShpMix, SHPHEADER& shp_h, BYTE*& shp)
{
	CString subname = artSection.GetValueByName(subkey);
	if (subname.GetLength() > 0)
	{
		auto res = FindUnitShp(subname, theat, artSection);
		/*CString subfilename = subname + ".shp";

		if (isTrue(artSection.GetValueByName("NewTheater")) || bAlwaysSetChar || subfilename.GetAt(0) == 'G' || subfilename.GetAt(0) == 'N' || subfilename.GetAt(0) == 'Y' || subfilename.GetAt(0) == 'C')
		{
			auto subfilename_theat = subfilename;
			subfilename_theat.SetAt(1, theat);
			if (FSunPackLib::XCC_DoesFileExist(subfilename_theat, hShpMix))
				subfilename = subfilename_theat;
		}*/

		if (res && FSunPackLib::XCC_DoesFileExist(res->filename, res->mixfile))
		{
			FSunPackLib::SetCurrentSHP(res->filename, res->mixfile);
			FSunPackLib::XCC_GetSHPHeader(&shp_h);
			FSunPackLib::LoadSHPImage(0, 1, &shp);

		}
	}
}
#else // surfaces
BOOL CLoading::LoadUnitGraphic(LPCTSTR lpUnittype)
{
	last_succeeded_operation=10;

	CString _rules_image; // the image used
	CString filename; // filename of the image
	char theat=cur_theat; // standard theater char is t (Temperat). a is snow.

	BOOL bAlwaysSetChar; // second char is always theater, even if NewTheater not specified!
	WORD wStep=1; // step is 1 for infantry, buildings, etc, and for shp vehicles it specifies the step rate between every direction
	WORD wStartWalkFrame=0; // for examply cyborg reaper has another walk starting frame
	int iTurretOffset=0; // used for centering y pos of turret (if existing)
	BOOL bStructure=rules.sections["BuildingTypes"].FindValue(lpUnittype)>=0; // is this a structure?

	BOOL bPowerUp=rules.sections[lpUnittype].values["PowersUpBuilding"]!="";
		
	HTSPALETTE hPalette;
	if(theat=='T') hPalette=m_hPalIsoTemp;
	if(theat=='A') hPalette=m_hPalIsoSnow;
	if(theat=='U') hPalette=m_hPalIsoUrb;
	
	CIsoView& v=*((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;

	_rules_image = lpUnittype;
	if (rules.sections[lpUnittype].values.find("Image") != rules.sections[lpUnittype].values.end())
		_rules_image = rules.sections[lpUnittype].values["Image"];

	CString _art_image = _rules_image;
	if (art.sections[_rules_image].values.find("Image") != art.sections[_rules_image].values.end())
	{
		if (!isTrue(g_data.sections["IgnoreArtImage"].values[_rules_image]))
			_art_image = art.sections[_rules_image].values["Image"];
	}

	const CString& image = _art_image;
	const auto& rulesSection = rules.sections[lpUnittype];
	const auto& artSection = art.sections[image];

	if(!isTrue(art.sections[image].values["Voxel"])) // is it a shp graphic?
	{
		try
		{

		filename=image+".shp";

				
		BYTE bTerrain=0;

		
		
		BOOL isNewTerrain=FALSE;
		if(isTrue(art.sections[image].values["NewTheater"]))//&& isTrue(artSection.GetValueByName("TerrainPalette")))//(filename.GetAt(0)=='G' || filename.GetAt(0)=='N' || filename.GetAt(0)=='C') && filename.GetAt(1)=='A')
		{
			hPalette=m_hPalUnitTemp;
			if(theat=='A') hPalette=m_hPalUnitSnow;
			if(theat=='U') hPalette=m_hPalUnitUrb;
			filename.SetAt(1, theat);
			isNewTerrain=TRUE;
		}
		
		
		HMIXFILE hShpMix=FindFileInMix(filename, &bTerrain);
		
		BYTE bIgnoreTerrain=TRUE;
		
		if(hShpMix==NULL && isNewTerrain)
		{
			filename.SetAt(1, 'G');
			hShpMix=FindFileInMix(filename, &bTerrain);
			if(hShpMix) theat='G';
			
		}

		
		
		if(hShpMix==NULL && isNewTerrain)
		{
			filename.SetAt(1, 'A');
			hShpMix=FindFileInMix(filename, &bTerrain);
			if(hShpMix) theat='A';
		}
		
		if(hShpMix==NULL && isNewTerrain)
		{
			filename.SetAt(1, 'T');
			hShpMix=FindFileInMix(filename, &bTerrain);
			if(hShpMix){
				theat='T';
				hPalette=m_hIsoTemp;
			}
		}

		
		if(isTrue(artSection.GetValueByName("TerrainPalette")))
		{
			bIgnoreTerrain=FALSE;
			
			if(cur_theat=='T')
			hPalette=m_hPalIsoTemp;
			else if(cur_theat=='A')
				hPalette=m_hPalIsoSnow;
			else if (cur_theat=='U')
				hPalette=m_hPalIsoUrb;
				
			
			
		}


		
		if(hShpMix==0) 
		{
			filename=image;
			filename+=".shp";
			hShpMix=FindFileInMix(filename, &bTerrain);

			
			
			if(hShpMix==NULL)
			{
				filename=image;
				if(theat=='T') filename+=".tem";
				if(theat=='A') filename+=".sno";
				if(theat=='U') filename+=".urb";
				filename.MakeLower();
				hShpMix=FindFileInMix(filename, &bTerrain);
				
				if(hShpMix==NULL)
				{
					filename=image;
					filename+=".tem";
					hShpMix=FindFileInMix(filename, &bTerrain);
					if(hShpMix)
					{
						hPalette=m_hPalIsoTemp;
					}
				}
				
				if(hShpMix!=NULL)
				{
				
					
					
				}
				else
				{
					filename=image+".shp";

					filename.SetAt(1, 'A');

					hShpMix=FindFileInMix(filename);

					if(hShpMix!=NULL)
					{
						bAlwaysSetChar=TRUE;
					}
					else
					{
						filename.SetAt(1, 'A');
						hShpMix=FindFileInMix(filename);

						if(hShpMix!=NULL)
						{
							theat='A';
							bAlwaysSetChar=TRUE;
						}
						else
						{
							filename.SetAt(1, 'U');
							hShpMix=FindFileInMix(filename);
							if(hShpMix) theat='U';
							else
							{
								filename.SetAt(1, 'T');
								hShpMix=FindFileInMix(filename);
								if(hShpMix) theat='T';
							}
						}
					}
				}
			}
			else
			{
				theat='T';
			}

		}
		else
		{

			// now we need to find out the palette
			
				if(isTrue(artSection.GetValueByName("TerrainPalette"))) // it´s a file in isotemp.mix/isosno.mix
				{
										
				}
				else // it´s a file in temperat.mix/snow.mix
				{
					if(cur_theat=='T') hPalette=m_hPalUnitTemp;
					if(cur_theat=='A') hPalette=m_hPalUnitSnow;
					if(cur_theat=='U') hPalette=m_hPalUnitUrb;
				}
			
		}

						

		if(filename=="tibtre01.tem" || filename=="tibtre02.tem" || filename=="tibtre03.tem" || filename=="veinhole.tem")
		{
			hPalette=m_hPalUnitTemp;
		}

		
		SHPHEADER head;
		CString bibname;
		CString bibfilename;
		CString activeanim_name;
		CString activeanim_filename;
		CString idleanim_name;
		CString idleanim_filename;
		CString activeanim2_name;
		CString activeanim2_filename;
		CString activeanim3_name;
		CString activeanim3_filename;
		CString superanim1_name,superanim1_filename;
		CString superanim2_name,superanim2_filename;
		CString superanim3_name,superanim3_filename;
		CString superanim4_name,superanim4_filename;
		CString specialanim1_name,specialanim1_filename;
		CString specialanim2_name,specialanim2_filename;
		CString specialanim3_name,specialanim3_filename;
		CString specialanim4_name,specialanim4_filename;

		CString turretanim_name;
		CString turretanim_filename;
		LPDIRECTDRAWSURFACE4 bib=NULL;
		LPDIRECTDRAWSURFACE4 activeanim=NULL;
		LPDIRECTDRAWSURFACE4 idleanim=NULL;
		LPDIRECTDRAWSURFACE4 activeanim2=NULL;
		LPDIRECTDRAWSURFACE4 activeanim3=NULL;
		LPDIRECTDRAWSURFACE4 superanim1=NULL;
		LPDIRECTDRAWSURFACE4 superanim2=NULL;
		LPDIRECTDRAWSURFACE4 superanim3=NULL;
		LPDIRECTDRAWSURFACE4 superanim4=NULL;
		LPDIRECTDRAWSURFACE4 specialanim1=NULL;
		LPDIRECTDRAWSURFACE4 specialanim2=NULL;
		LPDIRECTDRAWSURFACE4 specialanim3=NULL;
		LPDIRECTDRAWSURFACE4 specialanim4=NULL;
		LPDIRECTDRAWSURFACE4* lpT=NULL;
		LPDIRECTDRAWSURFACE4 turrets[8] = { 0 };
		SHPIMAGEHEADER turretinfo[8];
		
		if(hShpMix>0)
		{		
				

			//errstream << (LPCTSTR)filename << " found " ;
			//errstream.flush();

			    
			if(rules.sections[lpUnittype].values["Bib"]!="no") // seems to be ignored by TS, art.ini overwrites???
			{
				
				bibname=art.sections[image].values["BibShape"];
				if(bibname.GetLength()>0)
				{
					bibfilename=bibname+".shp";

					if(isTrue(art.sections[image].values["NewTheater"]))
						bibfilename.SetAt(1, theat);

					if(bAlwaysSetChar) bibfilename.SetAt(1, theat);

					if(FSunPackLib::XCC_DoesFileExist(bibfilename, hShpMix))
					{
						FSunPackLib::SetCurrentSHP(bibfilename, hShpMix);
						FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &bib);
						
					}
				}
			}

			activeanim_name=art.sections[image].values["ActiveAnim"];
			if(activeanim_name.GetLength()>0)
			{
				activeanim_filename=activeanim_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					activeanim_filename.SetAt(1, theat);

				if(bAlwaysSetChar) activeanim_filename.SetAt(1, theat);

				if(FSunPackLib::XCC_DoesFileExist(activeanim_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(activeanim_filename, hShpMix);
					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &activeanim);
					
					
				}
			}

			idleanim_name=art.sections[image].values["IdleAnim"];
			if(idleanim_name.GetLength()>0)
			{
				idleanim_filename=idleanim_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					idleanim_filename.SetAt(1, theat);

				if(bAlwaysSetChar) idleanim_filename.SetAt(1, theat);

				if(FSunPackLib::XCC_DoesFileExist(idleanim_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(idleanim_filename, hShpMix);
					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &idleanim);
				}
			}


			activeanim2_name=art.sections[image].values["ActiveAnimTwo"];
			if(activeanim2_name.GetLength()>0)
			{
				activeanim2_filename=activeanim2_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					activeanim2_filename.SetAt(1, theat);

				if(bAlwaysSetChar) activeanim2_filename.SetAt(1, theat);
				
				if(FSunPackLib::XCC_DoesFileExist(activeanim2_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(activeanim2_filename, hShpMix);
					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &activeanim2);
					
				}
			}

			activeanim3_name=art.sections[image].values["ActiveAnimThree"];
			if(activeanim3_name.GetLength()>0)
			{
				activeanim3_filename=activeanim3_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					activeanim3_filename.SetAt(1, theat);

				if(bAlwaysSetChar) activeanim3_filename.SetAt(1, theat);
				
				if(FSunPackLib::XCC_DoesFileExist(activeanim3_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(activeanim3_filename, hShpMix);
					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &activeanim3);
					
				}
			}

			superanim1_name=art.sections[image].values["SuperAnim"];
			if(superanim1_name.GetLength()>0)
			{
				superanim1_filename=superanim1_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					superanim1_filename.SetAt(1, theat);

				if(bAlwaysSetChar) superanim1_filename.SetAt(1, theat);
				
				if(FSunPackLib::XCC_DoesFileExist(superanim1_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(superanim1_filename, hShpMix);					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &superanim1);
				}
			}

			superanim2_name=art.sections[image].values["SuperAnimTwo"];
			if(superanim2_name.GetLength()>0)
			{
				superanim2_filename=superanim2_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					superanim2_filename.SetAt(1, theat);

				if(bAlwaysSetChar) superanim2_filename.SetAt(1, theat);
				
				if(FSunPackLib::XCC_DoesFileExist(superanim2_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(superanim2_filename, hShpMix);					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &superanim2);
				}
			}

			superanim3_name=art.sections[image].values["SuperAnimThree"];
			if(superanim3_name.GetLength()>0)
			{
				superanim3_filename=superanim3_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					superanim3_filename.SetAt(1, theat);

				if(bAlwaysSetChar) superanim3_filename.SetAt(1, theat);
				
				if(FSunPackLib::XCC_DoesFileExist(superanim3_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(superanim3_filename, hShpMix);					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &superanim3);
				}
			}

			superanim4_name=art.sections[image].values["SuperAnimFour"];
			if(superanim4_name.GetLength()>0)
			{
				superanim4_filename=superanim4_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					superanim4_filename.SetAt(1, theat);

				if(bAlwaysSetChar) superanim4_filename.SetAt(1, theat);
				
				if(FSunPackLib::XCC_DoesFileExist(superanim4_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(superanim4_filename, hShpMix);					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &superanim4);
				}
			}

			specialanim1_name=art.sections[image].values["SpecialAnim"];
			if(specialanim1_name.GetLength()>0)
			{
				specialanim1_filename=specialanim1_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					specialanim1_filename.SetAt(1, theat);

				if(bAlwaysSetChar) specialanim1_filename.SetAt(1, theat);
				
				if(FSunPackLib::XCC_DoesFileExist(specialanim1_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(specialanim1_filename, hShpMix);					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &specialanim1);
				}
			}

			specialanim2_name=art.sections[image].values["SpecialAnimTwo"];
			if(specialanim2_name.GetLength()>0)
			{
				specialanim2_filename=specialanim2_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					specialanim2_filename.SetAt(1, theat);

				if(bAlwaysSetChar) specialanim2_filename.SetAt(1, theat);
				
				if(FSunPackLib::XCC_DoesFileExist(specialanim2_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(specialanim2_filename, hShpMix);					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &specialanim2);
				}
			}

			specialanim3_name=art.sections[image].values["SpecialAnimThree"];
			if(specialanim3_name.GetLength()>0)
			{
				specialanim3_filename=specialanim3_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					specialanim3_filename.SetAt(1, theat);

				if(bAlwaysSetChar) specialanim3_filename.SetAt(1, theat);
				
				if(FSunPackLib::XCC_DoesFileExist(specialanim3_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(specialanim3_filename, hShpMix);					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &specialanim3);
				}
			}

			specialanim4_name=art.sections[image].values["SpecialAnimFour"];
			if(specialanim4_name.GetLength()>0)
			{
				specialanim4_filename=specialanim4_name+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					specialanim4_filename.SetAt(1, theat);

				if(bAlwaysSetChar) specialanim4_filename.SetAt(1, theat);
				
				if(FSunPackLib::XCC_DoesFileExist(specialanim4_filename, hShpMix))
				{
					FSunPackLib::SetCurrentSHP(specialanim4_filename, hShpMix);					
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &specialanim4);
				}
			}

			BOOL bVoxelTurret=FALSE;

			turretanim_name=rules.sections[image].values["TurretAnim"];
			if(bStructure && rules.sections[image].values["Turret"]=="yes" && turretanim_name.GetLength()>0 && rules.sections[image].values["TurretAnimIsVoxel"]!="true")
			{
				turretanim_filename=turretanim_name+".shp";
				if(art.sections[turretanim_name].values.find("Image")!=art.sections[turretanim_name].values.end()) turretanim_filename=art.sections[turretanim_name].values["Image"]+".shp";

				if(isTrue(art.sections[image].values["NewTheater"]))
					turretanim_filename.SetAt(1, theat);
				
			
				FSunPackLib::SetCurrentSHP(turretanim_filename, hShpMix);
				FSunPackLib::XCC_GetSHPHeader(&head);

				int iStartTurret=0;
				const WORD wAnimCount=4; // anims between each "normal" direction, seems to be hardcoded
				
				int i;

				for(i=0;i<8;i++)
				{
					if(iStartTurret+i*wAnimCount<head.c_images)
					{
						FSunPackLib::XCC_GetSHPImageHeader(iStartTurret+i*wAnimCount, &turretinfo[i]);
						FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, iStartTurret+i*wAnimCount, 1, &turrets[i]);
					}
					
				}
			}
			else if(bStructure && rules.sections[image].values["Turret"]=="yes" && turretanim_name.GetLength()>0 && rules.sections[image].values["TurretAnimIsVoxel"]=="true")
			{
				turretanim_filename=turretanim_name+".vxl";
				if(art.sections[turretanim_name].values.find("Image")!=art.sections[turretanim_name].values.end()) turretanim_filename=art.sections[turretanim_name].values["Image"]+".vxl";

				//if(isTrue(art.sections[image].values["NewTheater"]))
				//	turretanim_filename.SetAt(1, theat);
				
				HMIXFILE hVXL=FindFileInMix(turretanim_filename);
			
				if(hVXL)
				{
					bVoxelTurret=TRUE;

					if(
					FSunPackLib::SetCurrentVXL(turretanim_filename, hVXL)
					)
					{					
						int i;

						for(i=0;i<8;i++)
						{
							float r_x,r_y,r_z;
				

							r_x=300;
							r_y=0;
							r_z=45*i+90;

							// convert
							const double pi = 3.141592654;
							r_x=r_x/180.0f*pi;
							r_y=r_y/180.0f*pi;
							r_z=r_z/180.0f*pi;						

							int center_x, center_y;
							if(!
							FSunPackLib::LoadVXLImageInSurface(*m_voxelNormalTables, lightDirection, v.dd, 0, 1, r_x, r_y, r_z, &turrets[i], hPalette,&center_x, &center_y,atoi(rules.sections[image].values["TurretAnimZAdjust"]))
							)
							{
								
							}
							else
							{
								DDSURFACEDESC2 ddsd;
								memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
								ddsd.dwSize=sizeof(DDSURFACEDESC2);
								ddsd.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
								turrets[i]->GetSurfaceDesc(&ddsd);
								turretinfo[i].x=-center_x;
								turretinfo[i].y=-center_y;
								turretinfo[i].cx=ddsd.dwWidth;
								turretinfo[i].cy=ddsd.dwHeight;
							}
							
						}
					}
				}
			}


			if(art.sections[image].values.find("WalkFrames")!=art.sections[image].values.end())
				wStep=atoi(art.sections[image].values["WalkFrames"]);
			if(art.sections[image].values.find("StartWalkFrame")!=art.sections[image].values.end())
				wStartWalkFrame=atoi(art.sections[image].values["StartWalkFrame"]);
			if(art.sections[image].values.find("TurretOffset")!=art.sections[image].values.end())
				iTurretOffset=atoi(art.sections[image].values["TurretOffset"]);

			
			if(art.sections[image].values["Palette"]=="lib")
				hPalette=m_hPalLib;

			BOOL bSuccess=FSunPackLib::SetCurrentSHP(filename, hShpMix);
			if(
			!bSuccess
			)
			{
				filename=image+=".sno";
				if(cur_theat=='T' || cur_theat=='U') hPalette=m_hPalIsoTemp;
				hShpMix=FindFileInMix(filename, &bTerrain);
				bSuccess=FSunPackLib::SetCurrentSHP(filename, hShpMix);
				
				if(!bSuccess)
				{
					missingimages[lpUnittype]=TRUE;	
				}
			}
			
			if(bSuccess)
			{
			
			FSunPackLib::XCC_GetSHPHeader(&head);
			int i;
			int maxPics=head.c_images;
			if(maxPics>8) maxPics=8; // we only need 8 pictures for every direction!
			if(bStructure && !bPowerUp) maxPics=1;
			if(bVoxelTurret) maxPics=8;


			if(!bStructure && rules.sections[image].values["Turret"]=="yes")
			{
				int iStartTurret=wStartWalkFrame+8*wStep;
				const WORD wAnimCount=4; // anims between each "normal" direction, seems to be hardcoded
				
				int i;

				for(i=0;i<8;i++)
				{
					if(!bStructure && iStartTurret+i*wAnimCount<head.c_images)
					{
						FSunPackLib::XCC_GetSHPImageHeader(iStartTurret+i*wAnimCount, &turretinfo[i]);
						FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, iStartTurret+i*wAnimCount, 1, &turrets[i]);
					}
					
				}
			}

			
			
			// create an array of pointers to directdraw surfaces
			lpT=new(LPDIRECTDRAWSURFACE4[maxPics]);
			memset(lpT, 0, sizeof(LPDIRECTDRAWSURFACE4)*maxPics);
			
			if(bVoxelTurret)
			{
				for(i=0;i<maxPics; i++)
				{
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, 1, &lpT[i]);
				}
			}
			else if(wStep==1 && (rules.sections[lpUnittype].values["PowersUpBuilding"].GetLength()==0 || !isTrue(rules.sections[lpUnittype].values["Turret"]))) 
			{ // standard case...

				FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, wStartWalkFrame, maxPics, lpT);
			
			}
			else if(rules.sections[lpUnittype].values["PowersUpBuilding"].GetLength()!=0 && isTrue(rules.sections[lpUnittype].values["Turret"]))
			{ // a "real" turret (vulcan cannon, etc...)
				for(i=0;i<maxPics; i++)
				{
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, i*4, 1, &lpT[i]);
				}
			}
			else	 
			{ // walk frames used
				for(i=0;i<maxPics; i++)
				{
					int pic_in_file=i;
					FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, i*wStep+wStartWalkFrame, 1, &lpT[i]);
				}
			}

			for(i=0; i<maxPics; i++)
			{
				int pic_in_file=i;
				if(bStructure && bVoxelTurret) pic_in_file=0;
				SHPIMAGEHEADER imghead;
				FSunPackLib::XCC_GetSHPImageHeader(pic_in_file, &imghead);

				if(bib!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, bib, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					imghead.cx=head.cx-imghead.x; // update size of main graphic
					imghead.cy=head.cy-imghead.y;

				}

				if(activeanim!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, activeanim, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}

				if(idleanim!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, idleanim, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}

				if(activeanim2!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, activeanim2, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}

				if(activeanim3!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, activeanim3, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}
				
				if(superanim1!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, superanim1, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}

				if(superanim2!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, superanim2, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}

				if(superanim3!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, superanim3, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}

				if(superanim4!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, superanim4, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}

				if(specialanim1!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, specialanim1, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}

				if(specialanim2!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, specialanim2, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}

				if(specialanim3!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, specialanim3, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}

				if(specialanim4!=NULL)
				{
					DDBLTFX fx;

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					lpT[i]->Blt(NULL, specialanim4, NULL, DDBLT_KEYSRC | DDBLT_WAIT , &fx);

					
				}
				
				
				
				if(turrets[i]!=NULL)
				{
					DDBLTFX fx;
					int iMove=0;

					DDSURFACEDESC2 ddsd;
					memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
					ddsd.dwSize=sizeof(DDSURFACEDESC2);
					ddsd.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
					turrets[i]->GetSurfaceDesc(&ddsd);
					

					memset(&fx, 0, sizeof(DDBLTFX));
					fx.dwSize=sizeof(DDBLTFX);

					RECT srcRect, destRect;
					srcRect.left=0;					
					srcRect.right=ddsd.dwWidth;
					destRect.left=(head.cx-ddsd.dwWidth)/2;					
					destRect.right=head.cx-destRect.left;
					
					if(iMove<0)
					{
						srcRect.top=-iMove;
						srcRect.bottom=ddsd.dwHeight;
						destRect.top=0;
						destRect.bottom=head.cy+iMove-(head.cy-ddsd.dwHeight);
					}
					else
					{
						int mx=imghead.x/2+imghead.cx/2+(-turretinfo[i].x/2-turretinfo[i].cx)+ atoi(rules.sections[image].values["TurretAnimX"]);
						int my=imghead.y/2+imghead.cy/2+(-turretinfo[i].y/2-turretinfo[i].cy) + atoi(rules.sections[image].values["TurretAnimY"]);//+atoi(rules.sections[image].values["TurretAnimZAdjust"]);
						
						if(ddsd.dwWidth!=head.cx || ddsd.dwHeight!=head.cy)
						{
							// voxel turret
							//mx=head.cx/2-ddsd.dwWidth/2;//+atoi(rules.sections[image].values["TurretAnimX"]);
							//my=head.cy/2-ddsd.dwHeight/2+atoi(rules.sections[image].values["TurretAnimY"])+atoi(rules.sections[image].values["TurretAnimZAdjust"])/2;
							mx=imghead.x+imghead.cx/2+turretinfo[i].x+atoi(rules.sections[image].values["TurretAnimX"]);
							my=imghead.y+imghead.cy/2+turretinfo[i].y+atoi(rules.sections[image].values["TurretAnimY"]);//+atoi(rules.sections[image].values["TurretAnimZAdjust"])/2;
							
							errstream << turretinfo[i].x << " y:" << turretinfo[i].y << " mx:" << mx << " my:" << my << endl;
							errstream.flush();
							
							int XMover, YMover;
#ifdef RA2_MODE
							XMover=atoi(g_data.sections["BuildingVoxelTurretsRA2"].values[(CString)lpUnittype+"X"]);
							YMover=atoi(g_data.sections["BuildingVoxelTurretsRA2"].values[(CString)lpUnittype+"Y"]);
#else
							XMover=atoi(g_data.sections["BuildingVoxelTurrets"].values[(CString)lpUnittype+"X"]);
							YMover=atoi(g_data.sections["BuildingVoxelTurrets"].values[(CString)lpUnittype+"Y"]);
#endif

							mx+=XMover;
							my+=YMover;

							srcRect.top=0;
							srcRect.left=0;
							srcRect.right=ddsd.dwWidth;
							srcRect.bottom=ddsd.dwHeight;
							destRect.top=my;
							destRect.left=mx;
							destRect.right=destRect.left+ddsd.dwWidth;
							destRect.bottom=destRect.top+ddsd.dwHeight;
							if(destRect.top<0)
							{
								int old=destRect.top;
								destRect.top=0;
								srcRect.top-=old-destRect.top;

							}
							if(destRect.right>=head.cx)
							{
								int old=destRect.right;
								destRect.right=head.cx;
								srcRect.right-=old-destRect.right;
							}
							if(destRect.bottom>=head.cy)
							{
								int old=destRect.bottom;
								destRect.bottom=head.cy;
								srcRect.bottom-=old-destRect.bottom;
							}
						}
						else
						{

							if(mx<0)mx=0;
							if(my<0)my=0;
							srcRect.top=0;
							srcRect.right=ddsd.dwWidth-mx;
							srcRect.bottom=ddsd.dwHeight-my;
							destRect.top=my;
							destRect.left=mx+(head.cx-ddsd.dwWidth)/2;
							destRect.right=destRect.left+ddsd.dwWidth;;
							destRect.bottom=destRect.top+ddsd.dwHeight;
						}
					}
						
										
					

					if(lpT[i]->Blt(&destRect, turrets[i], &srcRect, DDBLT_KEYSRC | DDBLT_WAIT, &fx)!=DD_OK)
					{
						
						errstream << "vxl turret: " << i << " size: " << ddsd.dwWidth << " " << ddsd.dwHeight << "  failed" << endl;
						errstream.flush(); 
						//exit(-99);
					}

									
				}

				if(!bPowerUp && i!=0 && imghead.unknown==0 && bStructure)
				{
					if(lpT[i]) lpT[i]->Release();
				}
				else
				{
					char ic[50];
					itoa(i, ic, 10);

					PICDATA p;
					p.pic=lpT[i];				
					p.x=imghead.x;
					p.y=imghead.y;
					p.wHeight=imghead.cy;
					p.wWidth=imghead.cx;
					p.wMaxWidth=head.cx;
					p.wMaxHeight=head.cy;
					p.bType=PICDATA_TYPE_SHP;
					p.bTerrain=bTerrain;
					if(bIgnoreTerrain) p.bTerrain=0;

					
					pics[image+ic]=p;

					//errstream << " --> finished as " << (LPCSTR)(image+ic) << endl;
					//errstream.flush();
				}
				
			
			}
			
			delete[] lpT;

			
			if(bib) bib->Release();
			if(activeanim)activeanim->Release();
			if(idleanim)idleanim->Release();
			if(activeanim2)activeanim2->Release();
			if(activeanim3)activeanim3->Release();
			if(superanim1)superanim1->Release();
			if(superanim2)superanim2->Release();
			if(superanim3)superanim3->Release();
			if(superanim4)superanim4->Release();
			if(specialanim1)specialanim1->Release();
			if(specialanim2)specialanim2->Release();
			if(specialanim3)specialanim3->Release();
			if(specialanim4)specialanim4->Release();
			
			for(i=0;i<8;i++)
				if(turrets[i])turrets[i]->Release();
			
			}

			//errstream << " --> Finished" << endl;
			//errstream.flush();
		}
		
		else
		{
			errstream << "File in theater " << cur_theat << " not found: " << (LPCTSTR)filename << endl;
			errstream.flush();

			missingimages[lpUnittype]=TRUE;			
		}

		}
		catch(...)
		{
			errstream << " exception " << endl;
			errstream.flush();
		}
	
		
	}
	else
	{
		filename=image+".vxl"; 

		HMIXFILE hMix=FindFileInMix(filename);
		if(hMix==FALSE)
		{
			missingimages[lpUnittype]=TRUE;
			return FALSE;
		}

		
		int i;

		try
		{
		
		for(i=0;i<8;i++)
		{
			float r_x,r_y,r_z;
			

			r_x=300;
			r_y=0;
			r_z=45*i+90;

			// convert
			const double pi = 3.141592654;
			r_x=r_x/180.0f*pi;
			r_y=r_y/180.0f*pi;
			r_z=r_z/180.0f*pi;

			

			LPDIRECTDRAWSURFACE4 lpT;//=new(LPDIRECTDRAWSURFACE4[1]);
			LPDIRECTDRAWSURFACE4 lpTurret=NULL;
			LPDIRECTDRAWSURFACE4 lpBarrel=NULL;
			int turret_x,turret_y,turret_x_zmax,turret_y_zmax,barrel_x,barrel_y;

			if(isTrue(rules.sections[lpUnittype].values["Turret"]))
			{
				if(FSunPackLib::SetCurrentVXL(image+"tur.vxl", hMix))
				{
					FSunPackLib::LoadVXLImageInSurface(*m_voxelNormalTables, lightDirection, v.dd, 0, 1, r_x, r_y, r_z, &lpTurret, m_hPalUnitTemp,&turret_x,&turret_y,0,&turret_x_zmax, &turret_y_zmax,-1,-1);
				}
				if(FSunPackLib::SetCurrentVXL(image+"barl.vxl", hMix))
				{
					FSunPackLib::LoadVXLImageInSurface(*m_voxelNormalTables, lightDirection, v.dd, 0, 1, r_x, r_y, r_z, &lpBarrel, m_hPalUnitTemp,&barrel_x,&barrel_y,0,NULL,NULL,0,0);
				}	
			}


			if(!FSunPackLib::SetCurrentVXL(filename, hMix))
			{
				return FALSE;
			}

			

			int xcenter,ycenter,xcenter_zmax,ycenter_zmax;

			if(!
			FSunPackLib::LoadVXLImageInSurface(*m_voxelNormalTables, lightDirection, v.dd, 0, 1, r_x, r_y, r_z, &lpT, m_hPalUnitTemp,&xcenter, &ycenter,0,&xcenter_zmax,&ycenter_zmax)
			)
			{
				return FALSE;
			}

			DDSURFACEDESC2 ddsd;
			memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
			ddsd.dwSize=sizeof(DDSURFACEDESC2);
			ddsd.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
			lpT->GetSurfaceDesc(&ddsd);

			// turret
			if(lpTurret)
			{
				DDSURFACEDESC2 ddsdT;
				memset(&ddsdT, 0, sizeof(DDSURFACEDESC2));
				ddsdT.dwSize=sizeof(DDSURFACEDESC2);
				ddsdT.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
				lpTurret->GetSurfaceDesc(&ddsdT);

				DDBLTFX fx;
				memset(&fx, 0, sizeof(DDBLTFX));
				fx.dwSize=sizeof(DDBLTFX);

				RECT srcRect, destRect;
				srcRect.left=0;					
				srcRect.right=ddsdT.dwWidth;
				destRect.left=xcenter_zmax-turret_x;					
				destRect.right=destRect.left+ddsdT.dwWidth;
				srcRect.top=0;					
				srcRect.bottom=ddsdT.dwHeight;
				destRect.top=ycenter_zmax-turret_y;					
				destRect.bottom=destRect.top+ddsdT.dwHeight;

				lpT->Blt(&destRect, lpTurret, &srcRect, DDBLT_KEYSRC | DDBLT_WAIT, &fx);
				
			}

			// barrel
			if(lpBarrel)
			{
				DDSURFACEDESC2 ddsdB;
				memset(&ddsdB, 0, sizeof(DDSURFACEDESC2));
				ddsdB.dwSize=sizeof(DDSURFACEDESC2);
				ddsdB.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
				lpBarrel->GetSurfaceDesc(&ddsdB);
				
				DDSURFACEDESC2 ddsdT;
				memset(&ddsdT, 0, sizeof(DDSURFACEDESC2));
				ddsdT.dwSize=sizeof(DDSURFACEDESC2);
				ddsdT.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
				
				if(lpTurret) lpTurret->GetSurfaceDesc(&ddsdT);


				DDBLTFX fx;
				memset(&fx, 0, sizeof(DDBLTFX));
				fx.dwSize=sizeof(DDBLTFX);

				RECT srcRect, destRect;
				srcRect.left=0;					
				srcRect.right=ddsdB.dwWidth;
				destRect.left=xcenter_zmax-barrel_x+(turret_x_zmax-turret_x);					
				destRect.right=destRect.left+ddsdB.dwWidth;
				srcRect.top=0;					
				srcRect.bottom=ddsdB.dwHeight;
				destRect.top=ycenter_zmax-barrel_y+(turret_y_zmax-turret_y);					
				destRect.bottom=destRect.top+ddsdB.dwHeight;

				lpT->Blt(&destRect, lpBarrel, &srcRect, DDBLT_KEYSRC | DDBLT_WAIT, &fx);
				
			}

			char ic[50];
			itoa(7-i, ic, 10);

			errstream << ddsd.dwWidth << " " << ddsd.dwHeight << "\n";
			PICDATA p;
			p.pic=lpT;				
			p.x=-xcenter;
			p.y=-ycenter;
			p.wHeight=ddsd.dwHeight;
			p.wWidth=ddsd.dwWidth;
			p.wMaxWidth=ddsd.dwWidth;
			p.wMaxHeight=ddsd.dwHeight;
			p.bType=PICDATA_TYPE_VXL;
			p.bTerrain=0;
					
			pics[image+ic]=p;

			errstream << "vxl saved as " << (LPCSTR)image << (LPCSTR)ic << endl;
			errstream.flush();

			if(lpBarrel) lpBarrel->Release();
			if(lpTurret) lpTurret->Release();

			//delete[] lpT;
			
		}
		}
		catch(...)
		{

		}

	}

	

	return FALSE;	
}
#endif


BOOL CLoading::InitMixFiles()
{
	last_succeeded_operation=8;

	MEMORYSTATUS ms;
	ms.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	int cs=ms.dwAvailPhys+ms.dwAvailPageFile;

	errstream << "InitMixFiles() called. Available memory: " << cs << endl;
	errstream.flush();


	// load tibsun.mix and local.mix
	if(DoesFileExist((CString)TSPath+(CString)"\\"+MAINMIX))
	{
		errstream << "Loading " MAINMIX ".mix" ;
		errstream.flush();
		m_hTibSun=FSunPackLib::XCC_OpenMix((CString)TSPath+(CString)"\\"+MAINMIX, NULL);
		if(m_hTibSun!=NULL)
		{
			errstream << " success" << endl;
			errstream.flush();
		}
		else
		{
			ShowWindow(SW_HIDE);
			MessageBox(GetLanguageStringACP("Err_TSNotInstalled"));
			exit(200);
		}

		m_hLanguage=FSunPackLib::XCC_OpenMix((CString)TSPath+(CString)"\\Language.mix", NULL);
		m_hLangMD=FSunPackLib::XCC_OpenMix((CString)TSPath+(CString)"\\Langmd.mix", NULL);
		m_hMarble=FSunPackLib::XCC_OpenMix((CString)TSPath+(CString)"\\marble.mix", NULL);

		//if(!m_hLanguage) MessageBox("No language file found");

		if(!m_hMarble)
		{
			m_hMarble=FSunPackLib::XCC_OpenMix((CString)AppPath+(CString)"\\marble.mix", NULL);
		}
	}
	else
	{
		ShowWindow(SW_HIDE);
		MessageBox(GetLanguageStringACP("Err_TSNotInstalled"));
		exit(199);
	}

	errstream << "Loading local.mix";
	errstream.flush();
	if(DoesFileExist((CString)TSPath+"\\Local.mix")==FALSE || theApp.m_Options.bSearchLikeTS==FALSE)
	{
		m_hLocal=FSunPackLib::XCC_OpenMix("Local.mix", m_hTibSun);
	}
	else
		m_hLocal=FSunPackLib::XCC_OpenMix((CString)TSPath+"\\Local.mix", NULL);
	errstream << " successful" << endl;
	errstream.flush();


    
	errstream << "Loading conquer.mix, temperat.mix, isotemp.mix, isosnow.mix, isourb.mix and cache.mix";
	errstream.flush();
	m_hConquer=FSunPackLib::XCC_OpenMix("conquer.mix", m_hTibSun);
	m_hTemperat=FSunPackLib::XCC_OpenMix("temperat.mix", m_hTibSun);
	m_hUrban=FSunPackLib::XCC_OpenMix("urban.mix", m_hTibSun);
    m_hSnow=FSunPackLib::XCC_OpenMix("snow.mix", m_hTibSun);
	m_hUrbanN=FSunPackLib::XCC_OpenMix("urbann.mix", m_hTibSun);
	m_hLunar=FSunPackLib::XCC_OpenMix("lunar.mix", m_hTibSun);
    m_hDesert=FSunPackLib::XCC_OpenMix("desert.mix", m_hTibSun);
	m_hIsoTemp=FSunPackLib::XCC_OpenMix("isotemp.mix", m_hTibSun);
	m_hIsoSnow=FSunPackLib::XCC_OpenMix("isosnow.mix", m_hTibSun);
	m_hIsoUrb=FSunPackLib::XCC_OpenMix("isourb.mix", m_hTibSun);
	m_hIsoUbn=FSunPackLib::XCC_OpenMix("isoubn.mix", m_hTibSun);
	m_hIsoLun=FSunPackLib::XCC_OpenMix("isolun.mix", m_hTibSun);
	m_hIsoDes=FSunPackLib::XCC_OpenMix("isodes.mix", m_hTibSun);
	m_hIsoGen=FSunPackLib::XCC_OpenMix("isogen.mix", m_hTibSun);
	//m_hBuildings=FSunPackLib::XCC_OpenMix("_ID1085587737", m_hTibSun);	
	m_hBuildings=FSunPackLib::XCC_OpenMix("Generic.mix", m_hTibSun);
	m_hCache=FSunPackLib::XCC_OpenMix("cache.mix", m_hTibSun);
	m_hTem=FSunPackLib::XCC_OpenMix("tem.mix", m_hTibSun);
	m_hSno=FSunPackLib::XCC_OpenMix("sno.mix", m_hTibSun);
	m_hUrb=FSunPackLib::XCC_OpenMix("urb.mix", m_hTibSun);
	m_hUbn=FSunPackLib::XCC_OpenMix("ubn.mix", m_hTibSun);
	m_hLun=FSunPackLib::XCC_OpenMix("lun.mix", m_hTibSun);
	m_hDes=FSunPackLib::XCC_OpenMix("des.mix", m_hTibSun);
	if(!m_hMarble) FSunPackLib::XCC_OpenMix("marble.mix", m_hTibSun);

	if(m_hMarble) theApp.m_Options.bSupportMarbleMadness=TRUE;

	errstream << " successful" << endl;
	errstream.flush();

	int i;

	yuri_mode=FALSE;


	if(!theApp.m_Options.bSearchLikeTS) return TRUE;
	
	if(DoesFileExist((CString)TSPath+"\\ra2md.mix"))
		yuri_mode=TRUE; // MW Apr 17th, make it available right here!

	// load expansion mix files
	for(i=0;i<101;i++)
	{
		CString expand;		
		char n[50];	
		
		// MW April 17th, 2002:
		// read expandxxmd.mix files in yurimode		
			
		itoa(i, n, 10);
		expand=TSPath;
		expand+="\\Expand";		
		if(yuri_mode) expand+="md";
		if(i<10) expand+="0";
		expand+=n;
		expand+=".mix";

		CString nappend=".mix";

		CString append=".mix";
		if(i==100) append="md.mix";

		
		if(yuri_mode) append="md.mix";


		if(i==100) expand=(CString)TSPath+"\\ra2md.mix"; // support the mission disk!!!

		errstream << "Searching " << (LPCTSTR)expand << endl;
		errstream.flush();

		if(DoesFileExist(expand))
		{

			OutputDebugString(expand);
			OutputDebugString(": ");
			

			m_hExpand[i].hExpand=FSunPackLib::XCC_OpenMix(expand, NULL);

#ifdef YR_MODE
			if(i==100 && m_hExpand[i].hExpand) yuri_mode=TRUE;
#endif

			errstream << (LPCTSTR)expand << " found: loading ";
			errstream.flush();

			CString conquer="conquer";
			if(i==100) conquer="conq";
			if(FSunPackLib::XCC_DoesFileExist(conquer+append, m_hExpand[i].hExpand))
			{
				OutputDebugString(conquer+append);
				OutputDebugString(": ");
				m_hExpand[i].hConquer=FSunPackLib::XCC_OpenMix((CString)conquer+append, m_hExpand[i].hExpand);
				errstream << "conquer.mix, ";
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"local"+append, m_hExpand[i].hExpand))
			{
				OutputDebugString((CString)"local"+append);
				OutputDebugString(": ");
				m_hExpand[i].hLocal=FSunPackLib::XCC_OpenMix((CString)"local"+append, m_hExpand[i].hExpand);
				errstream << "local.mix, ";
			}
			//if(FSunPackLib::XCC_DoesFileExist("_ID1085587737", m_hExpand[i].hExpand))
			{
				//m_hExpand[i].hConquer=FSunPackLib::XCC_OpenMix("_ID1085587737", m_hExpand[i].hExpand);
				//errstream << "1085587737, ";
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"temperat"+append, m_hExpand[i].hExpand))
			{
				OutputDebugString((CString)"temperat"+append);
				OutputDebugString(": ");
				m_hExpand[i].hTemperat=FSunPackLib::XCC_OpenMix((CString)"temperat"+append, m_hExpand[i].hExpand);
				errstream << "temperat.mix, ";
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"urban"+append, m_hExpand[i].hExpand))
			{
				OutputDebugString((CString)"urban"+append);
				OutputDebugString(": ");
				m_hExpand[i].hUrban=FSunPackLib::XCC_OpenMix((CString)"urban"+append, m_hExpand[i].hExpand);
				errstream << "urban.mix, ";
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"snow"+append, m_hExpand[i].hExpand))
			{
				OutputDebugString((CString)"snow"+append);
				OutputDebugString(": ");
				
				FSunPackLib::_DEBUG_EnableLogs=true;

				HMIXFILE hM=FSunPackLib::XCC_OpenMix((CString)"snow"+append, m_hExpand[i].hExpand);
				m_hExpand[i].hSnow=hM;
				errstream << "snow.mix, ";
				errstream.flush();

				FSunPackLib::_DEBUG_EnableLogs=false;
			}

			CString generic="generic";
			if(i==100) generic="gener";
			if(FSunPackLib::XCC_DoesFileExist((CString)generic+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hGeneric=FSunPackLib::XCC_OpenMix((CString)generic+append, m_hExpand[i].hExpand);
				errstream << "generic.mix, ";
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"urbann"+nappend, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hUrbanN=FSunPackLib::XCC_OpenMix((CString)"urbann"+nappend, m_hExpand[i].hExpand);
				errstream << "urbann.mix, ";
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"lunar"+nappend, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hLunar=FSunPackLib::XCC_OpenMix((CString)"lunar"+nappend, m_hExpand[i].hExpand);
				errstream << "lunar.mix, ";
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"desert"+nappend, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hDesert=FSunPackLib::XCC_OpenMix((CString)"desert"+nappend, m_hExpand[i].hExpand);
				errstream << "desert.mix, ";
				errstream.flush();
			}
			CString isotemp="isotemp";
			if(i==100) isotemp="isotem";
			if(FSunPackLib::XCC_DoesFileExist(isotemp+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hIsoTemp=FSunPackLib::XCC_OpenMix((CString)isotemp+append, m_hExpand[i].hExpand);
				errstream << "isotemp.mix, ";
				errstream.flush();
			}
			CString isosnow="isosnow";
			if(i==100) isosnow="isosno";
			if(FSunPackLib::XCC_DoesFileExist((CString)isosnow+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hIsoSnow=FSunPackLib::XCC_OpenMix((CString)isosnow+append, m_hExpand[i].hExpand);
				errstream << "isosnow.mix, ";
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"isourb"+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hIsoUrb=FSunPackLib::XCC_OpenMix((CString)"isourb"+append, m_hExpand[i].hExpand);
				errstream << "isourb.mix, ";
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"isoubn"+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hIsoUbnMd=FSunPackLib::XCC_OpenMix((CString)"isoubn"+append, m_hExpand[i].hExpand);
				errstream << "isoubn.mix, ";
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"isolun"+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hIsoLunMd=FSunPackLib::XCC_OpenMix((CString)"isolun"+append, m_hExpand[i].hExpand);
				errstream << "isolun.mix, ";
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"isodes"+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hIsoDesMd=FSunPackLib::XCC_OpenMix((CString)"isodes"+append, m_hExpand[i].hExpand);
				errstream << "isodes.mix, ";
				errstream.flush();
			}
			
			if(FSunPackLib::XCC_DoesFileExist((CString)"isoubn"+nappend, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hIsoUbn=FSunPackLib::XCC_OpenMix((CString)"isoubn"+nappend, m_hExpand[i].hExpand);
				errstream << "isoubn.mix, ";
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"isolun"+nappend, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hIsoLun=FSunPackLib::XCC_OpenMix((CString)"isolun"+nappend, m_hExpand[i].hExpand);
				errstream << "isolun.mix, ";
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"isodes"+nappend, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hIsoDes=FSunPackLib::XCC_OpenMix((CString)"isodes"+nappend, m_hExpand[i].hExpand);
				errstream << "isodes.mix, ";
				errstream.flush();
			}

			if(FSunPackLib::XCC_DoesFileExist((CString)"isogen"+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hIsoGenMd=FSunPackLib::XCC_OpenMix((CString)"isogen"+append, m_hExpand[i].hExpand);
				errstream << "isogen.mix, ";
				errstream.flush();
			}

			if(FSunPackLib::XCC_DoesFileExist((CString)"isogen"+nappend, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hIsoGen=FSunPackLib::XCC_OpenMix((CString)"isogen"+nappend, m_hExpand[i].hExpand);
				errstream << "isogen.mix, ";
				errstream.flush();
			}

			CString cache="ecache01";
			if(i==100) cache="cache";
			if(FSunPackLib::XCC_DoesFileExist((CString)cache+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hECache=FSunPackLib::XCC_OpenMix((CString)cache+append, m_hExpand[i].hExpand);
				errstream << LPCSTR("ecache01"+append+", ");
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"tem"+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hTem=FSunPackLib::XCC_OpenMix((CString)"tem"+append, m_hExpand[i].hExpand);
				errstream << LPCSTR("tem"+append+", ");
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"sno"+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hSno=FSunPackLib::XCC_OpenMix((CString)"sno"+append, m_hExpand[i].hExpand);
				errstream << LPCSTR("sno"+append+", ");
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"urb"+append, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hUrb=FSunPackLib::XCC_OpenMix((CString)"urb"+append, m_hExpand[i].hExpand);
				errstream << LPCSTR("urb"+append+", ");
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"ubn"+nappend, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hUbn=FSunPackLib::XCC_OpenMix((CString)"ubn"+nappend, m_hExpand[i].hExpand);
				errstream << LPCSTR("ubn"+nappend+", ");
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"lun"+nappend, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hLun=FSunPackLib::XCC_OpenMix((CString)"lun"+nappend, m_hExpand[i].hExpand);
				errstream << LPCSTR("lun"+nappend+", ");
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"des"+nappend, m_hExpand[i].hExpand))
			{
				m_hExpand[i].hDes=FSunPackLib::XCC_OpenMix((CString)"des"+nappend, m_hExpand[i].hExpand);
				errstream << LPCSTR("des"+nappend+", ");
				errstream.flush();
			}
			if(FSunPackLib::XCC_DoesFileExist((CString)"marble"+append, m_hExpand[i].hExpand))
			{
				theApp.m_Options.bSupportMarbleMadness=TRUE;

				m_hExpand[i].hMarble=FSunPackLib::XCC_OpenMix((CString)"marble"+append, m_hExpand[i].hExpand);
				errstream << LPCSTR("marble"+append+", ");
				errstream.flush();
			}
			
			errstream << endl;
			errstream.flush();
		}
		else
		{
			m_hExpand[i].hExpand=NULL;
		}
	}

	// load expansion ecache mix files
	for(i=0;i<100;i++)
	{
		CString expand;		
		char n[50];			
			
		itoa(i, n, 10);
		expand=TSPath;
		if(!yuri_mode) expand+="\\ECache";
		else expand+="\\ecachemd";

		if(i<10) expand+="0";
		expand+=n;
		expand+=".mix";

	
		if(DoesFileExist(expand))
		{
			m_hECache[i]=FSunPackLib::XCC_OpenMix(expand, NULL);
			errstream << (LPCTSTR)expand << " found and loaded" << endl;
			errstream.flush();
		}
		else
		{
			m_hECache[i]=NULL;
		}
	}

	ms.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	cs=ms.dwAvailPhys+ms.dwAvailPageFile;

	errstream << "InitMixFiles() finished. Available memory: " << cs << endl;
	errstream.flush();

	return TRUE;
}

CLoading::~CLoading()
{
	Unload();
}

void CLoading::Unload()
{
	FSunPackLib::XCC_CloseMix(m_hCache);
	FSunPackLib::XCC_CloseMix(m_hConquer);
	FSunPackLib::XCC_CloseMix(m_hIsoSnow);
	FSunPackLib::XCC_CloseMix(m_hIsoTemp);
	FSunPackLib::XCC_CloseMix(m_hIsoUrb);
	FSunPackLib::XCC_CloseMix(m_hIsoGen);
	FSunPackLib::XCC_CloseMix(m_hLocal);
	FSunPackLib::XCC_CloseMix(m_hTemperat);
	FSunPackLib::XCC_CloseMix(m_hSnow);
	FSunPackLib::XCC_CloseMix(m_hUrban);
	FSunPackLib::XCC_CloseMix(m_hTibSun);
	FSunPackLib::XCC_CloseMix(m_hSno);
	FSunPackLib::XCC_CloseMix(m_hTem);
	FSunPackLib::XCC_CloseMix(m_hUrb);
	FSunPackLib::XCC_CloseMix(m_hBuildings);


	m_hCache = NULL;
	m_hConquer = NULL;
	m_hIsoSnow = NULL;
	m_hIsoTemp = NULL;
	m_hIsoUrb = NULL;
	m_hLocal = NULL;
	m_hTemperat = NULL;
	m_hSnow = NULL;
	m_hUrban = NULL;
	m_hTibSun = NULL;
	m_hBuildings = NULL;
	m_hIsoGen = NULL;


	int i = 0;
	for (i = 0;i < 100; i++)
	{
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hExpand);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hConquer);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hECache);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hIsoSnow);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hIsoTemp);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hIsoUrb);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hIsoGen);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hTemperat);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hSnow);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hUrban);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hSno);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hTem);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hUrb);
		FSunPackLib::XCC_CloseMix(m_hExpand[i].hBuildings);
		m_hExpand[i].hExpand = NULL;
	}

	for (i = 0;i < 100; i++)
	{
		FSunPackLib::XCC_CloseMix(m_hECache[i]);
	}

	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	int cs = ms.dwAvailPhys + ms.dwAvailPageFile;

	errstream << "CLoading::Unload finished. Available memory: " << cs << endl;
	errstream.flush();
}


HMIXFILE CLoading::FindFileInMix(LPCTSTR lpFilename, TheaterChar* pTheaterChar)
{
	if(pTheaterChar) 
		*pTheaterChar = TheaterChar::None;

	int i;
	// MW: added ecache support
	for(i=100;i>=0; i--)
	{
		HMIXFILE cuExp=m_hECache[i];

		if(cuExp!=NULL)
		{			
			if(FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp))
				return cuExp;
		}
	}

	for(i=100;i>=0; i--)
	{
		EXPANDMIX cuExp=m_hExpand[i];

		if(cuExp.hExpand!=NULL)
		{			
			if(FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hExpand))
				return cuExp.hExpand;
			if(cuExp.hECache!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hECache))
				return cuExp.hECache;
			if(cuExp.hConquer!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hConquer))
				return cuExp.hConquer;
			if(cuExp.hLocal!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hLocal))
				return cuExp.hLocal;
			if(cuExp.hTemperat!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hTemperat))
				return cuExp.hTemperat;
			if(strcmp(lpFilename,"yayard.shp")==NULL)
				FSunPackLib::_DEBUG_EnableLogs=true;
			if(cuExp.hSnow!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hSnow))
					return cuExp.hSnow;
			FSunPackLib::_DEBUG_EnableLogs=false;
			if(cuExp.hGeneric!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hGeneric))
				return cuExp.hGeneric;
			if(cuExp.hUrban!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hUrban))
				return cuExp.hUrban;
			if(cuExp.hUrbanN!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hUrbanN))
				return cuExp.hUrbanN;
			if(cuExp.hLunar!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hLunar))
				return cuExp.hLunar;
			if(cuExp.hDesert!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hDesert))
				return cuExp.hDesert;
			if(cuExp.hBuildings!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hBuildings))
				return cuExp.hBuildings;
			if(cuExp.hIsoGen!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoGen))
				return cuExp.hIsoGen;
			if(cuExp.hIsoGenMd!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoGenMd))
				return cuExp.hIsoGenMd;
			if(cuExp.hIsoTemp!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoTemp))
			{
				if(pTheaterChar) *pTheaterChar=TheaterChar::T;
				return cuExp.hIsoTemp;
			}
			if(cuExp.hIsoSnow!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoSnow))
			{
				if(pTheaterChar) *pTheaterChar = TheaterChar::A;
				return cuExp.hIsoSnow;	
			}
			if(cuExp.hIsoUrb!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoUrb))
			{
				if(pTheaterChar) *pTheaterChar = TheaterChar::U;
				return cuExp.hIsoUrb;	
			}
			if(cuExp.hIsoUbn!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoUbn))
			{
				if(pTheaterChar) *pTheaterChar = TheaterChar::N;
				return cuExp.hIsoUbn;
			}
			if(cuExp.hIsoDes!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoDes))
			{
				if(pTheaterChar) *pTheaterChar = TheaterChar::D;
				return cuExp.hIsoDes;	
			}
			if(cuExp.hIsoLun!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoLun))
			{
				if(pTheaterChar) *pTheaterChar = TheaterChar::L;
				return cuExp.hIsoLun;	
			}
			if(cuExp.hIsoUbnMd!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoUbnMd))
			{
				if(pTheaterChar) *pTheaterChar = TheaterChar::N;
				return cuExp.hIsoUbnMd;
			}
			if(cuExp.hIsoDesMd!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoDesMd))
			{
				if(pTheaterChar) *pTheaterChar = TheaterChar::D;
				return cuExp.hIsoDesMd;	
			}
			if(cuExp.hIsoLunMd!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hIsoLunMd))
			{
				if(pTheaterChar) *pTheaterChar = TheaterChar::L;
				return cuExp.hIsoLunMd;	
			}
			if(cuExp.hSno!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hSno))
				return cuExp.hSno;	
			if(cuExp.hTem!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hTem))
				return cuExp.hTem;	
			if(cuExp.hUrb!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hUrb))
				return cuExp.hUrb;
			if(cuExp.hUbn!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hUbn))
				return cuExp.hUbn;	
			if(cuExp.hLun!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hLun))
				return cuExp.hLun;	
			if(cuExp.hDes!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hDes))
				return cuExp.hDes;
			if(cuExp.hMarble!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, cuExp.hMarble))
				return cuExp.hMarble;	
		}
	}

	if(m_hTibSun!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hTibSun))
		return m_hTibSun;
	if(m_hLanguage!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hLanguage))
		return m_hLanguage;	
	if(m_hLangMD!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hLangMD))
		return m_hLangMD;	
	if(m_hLocal!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hLocal))
		return m_hLocal;
	if(m_hBuildings!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hBuildings))
		return m_hBuildings;
	if(m_hCache!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hCache))
		return m_hCache;
	if(m_hConquer!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hConquer))
		return m_hConquer;
	if(m_hTemperat!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hTemperat))
		return m_hTemperat;
	if(m_hSnow!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hSnow))
		return m_hSnow;
	if(m_hUrban!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hUrban))
		return m_hUrban;
	if(m_hUrbanN!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hUrbanN))
		return m_hUrbanN;
	if(m_hLunar!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hLunar))
		return m_hLunar;
	if(m_hDesert!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hDesert))
		return m_hDesert;
	if(m_hIsoGen!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoGen))
	{
		return m_hIsoGen;
	}
	if(m_hIsoTemp!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoTemp))
	{
		if(pTheaterChar) *pTheaterChar = TheaterChar::T;
		return m_hIsoTemp;
	}
	if(m_hIsoSnow!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoSnow))
	{
		if(pTheaterChar) *pTheaterChar = TheaterChar::A;
		return m_hIsoSnow;	
	}
	if(m_hIsoUrb!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoUrb))
	{
		if(pTheaterChar) *pTheaterChar = TheaterChar::U;
		return m_hIsoUrb;	
	}
	if(m_hIsoUbn!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoUbn))
	{
		if(pTheaterChar) *pTheaterChar = TheaterChar::N;
		return m_hIsoUbn;
	}
	if(m_hIsoLun!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoLun))
	{
		if(pTheaterChar) *pTheaterChar = TheaterChar::L;
		return m_hIsoLun;	
	}
	if(m_hIsoDes!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hIsoDes))
	{
		if(pTheaterChar) *pTheaterChar = TheaterChar::D;
		return m_hIsoDes;	
	}
	if(m_hTem!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hTem))
		return m_hTem;
	if(m_hSno!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hSno))
		return m_hSno;	
	if(m_hUrb!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hUrb))
		return m_hUrb;	
	if(m_hUbn!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hUbn))
		return m_hUbn;
	if(m_hLun!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hLun))
		return m_hLun;	
	if(m_hDes!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hDes))
		return m_hDes;	

	if(m_hMarble!=NULL && FSunPackLib::XCC_DoesFileExist(lpFilename, m_hMarble))
		return m_hMarble;		

	

	return NULL;
}



void CLoading::InitPalettes()
{
	errstream << "InitPalettes() called\n";
	if(!FSunPackLib::XCC_ExtractFile("isotem.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache))
		errstream << "IsoTem.pal failed\n";
	m_hPalIsoTemp=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	if(!FSunPackLib::XCC_ExtractFile("isosno.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache))
		errstream << "IsoSno.pal failed\n";
	m_hPalIsoSnow=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	if(!FSunPackLib::XCC_ExtractFile("isourb.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache))
		errstream << "IsoUrb.pal failed\n";
	m_hPalIsoUrb=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	HMIXFILE m_hCache2=m_hExpand[100].hECache;
	if(!FSunPackLib::XCC_ExtractFile("isolun.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache2))
		errstream << "IsoLun.pal failed\n";
	m_hPalIsoLun=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");
	
	if(!FSunPackLib::XCC_ExtractFile("isodes.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache2))
		errstream << "IsoDes.pal failed\n";
	m_hPalIsoDes=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	if(!FSunPackLib::XCC_ExtractFile("isoubn.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache2))
		errstream << "IsoUbn.pal failed\n";
	m_hPalIsoUbn=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");


	if(!FSunPackLib::XCC_ExtractFile("unittem.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache))
		errstream << "UnitTem.pal failed";
	m_hPalUnitTemp=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	if(!FSunPackLib::XCC_ExtractFile("unitsno.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache))
		errstream << "UnitSno.pal failed\n";
	m_hPalUnitSnow=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	if(!FSunPackLib::XCC_ExtractFile("uniturb.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache))
		errstream << "UnitUrb.pal failed\n";
	m_hPalUnitUrb=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");


	if(!FSunPackLib::XCC_ExtractFile("unitlun.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache2))
		errstream << "UnitLun.pal failed\n";
	m_hPalUnitLun=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	if(!FSunPackLib::XCC_ExtractFile("unitdes.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache2))
		errstream << "UnitDes.pal failed\n";
	m_hPalUnitDes=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	if(!FSunPackLib::XCC_ExtractFile("unitubn.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache2))
		errstream << "UnitUbn.pal failed\n";
	m_hPalUnitUbn=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");


	if(!FSunPackLib::XCC_ExtractFile("snow.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache))
		errstream << "Snow.pal failed\n";
	m_hPalSnow=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	if(!FSunPackLib::XCC_ExtractFile("temperat.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache))
		errstream << "Temperat.pal failed\n";
	m_hPalTemp=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	if(!FSunPackLib::XCC_ExtractFile("urban.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache))
		errstream << "Urban.pal failed\n";
	m_hPalUrb=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");


	if(!FSunPackLib::XCC_ExtractFile("lunar.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache2))
		errstream << "lunar.pal failed\n";
	m_hPalLun=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	if(!FSunPackLib::XCC_ExtractFile("desert.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache2))
		errstream << "desert.pal failed\n";
	m_hPalDes=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");

	if(!FSunPackLib::XCC_ExtractFile("urbann.pal", u8AppDataPath+"\\TmpPalette.pal", m_hCache2))
		errstream << "urbann.pal failed\n";
	m_hPalUbn=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");


	if(!FSunPackLib::XCC_ExtractFile("_ID2124019542", u8AppDataPath+"\\TmpPalette.pal", m_hCache))
		errstream << "lib.pal failed\n";
	m_hPalLib=FSunPackLib::LoadTSPalette(u8AppDataPath+"\\TmpPalette.pal", NULL);
	deleteFile(u8AppDataPath +"\\TmpPalette.pal");


	errstream << "\n";
	errstream.flush();


}

void CLoading::InitTMPs(CProgressCtrl* prog)
{
	FetchPalettes();
	

	MEMORYSTATUS ms;
	ms.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	int cs=ms.dwAvailPhys+ms.dwAvailPageFile;

	errstream << "InitTMPs() called. Available memory: " << cs << endl;
	errstream.flush();

	// we need to have that here, CMapData::UpdateIniFile() is too late for the shore hack
	shoreset=atoi((*tiles).sections["General"].values["ShorePieces"]);
	waterset=atoi((*tiles).sections["General"].values["WaterSet"]);



		


	int i, tcount=0;

	for(i=0;i<10000;i++)
	{
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for(e=0;e<4-strlen(c);e++)
			tset+="0";
		tset+=c;
		CString sec="TileSet";
		sec+=tset;

		if(tiles->sections.find(sec)==tiles->sections.end()) break;
			
		for(e=0;e<atoi(tiles->sections[sec].values["TilesInSet"]);e++)
		{
			tcount++;
		}

			
	}

	if(prog) prog->SetRange(0, tcount);
	if(prog) prog->SetPos(0);

	if(*tiledata!=NULL) delete[] *tiledata;
	*tiledata=new(TILEDATA[tcount]);
	*tiledata_count=tcount;

	DWORD tilecount=0;	
	for(i=0;i<10000;i++)
	{
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for(e=0;e<4-strlen(c);e++)
			tset+="0";
		tset+=c;
		CString sec="TileSet";
		sec+=tset;

		if(tiles->sections.find(sec)==tiles->sections.end()) break;

		BOOL bTib, bMorph, bPlace, bMadness;
		bPlace=TRUE;
		bTib=FALSE;
		bMorph=FALSE;
		bMadness=FALSE;
		tiles->sections[sec].values["AllowTiberium"].MakeLower();
		if(tiles->sections[sec].values["AllowTiberium"]=="true")
			bTib=TRUE;
		tiles->sections[sec].values["Morphable"].MakeLower();
		if(tiles->sections[sec].values["Morphable"]=="true")
			bMorph=TRUE;
		tiles->sections[sec].values["AllowToPlace"].MakeLower();
		if(tiles->sections[sec].values["AllowToPlace"]=="no")
			bPlace=FALSE;
		tiles->sections[sec].values["NonMarbleMadness"].MakeLower();
		if(tiles->sections[sec].values["NonMarbleMadness"].GetLength()>0)
			bMadness=TRUE;
		auto tilesetAnimSection = tiles->GetSection(tiles->sections[sec].GetValueByName("SetName"));

		tilesets_start[i]=tilecount;


		for(e=0;e<atoi(tiles->sections[sec].values["TilesInSet"]);e++)
		{
			std::string sId = std::format("{:02}", e + 1);
			CString filename=tiles->sections[sec].values["FileName"];
			filename+=sId.c_str();

			CString bas_f=filename;
			
			CString suffix;

			if(tiles==&tiles_t) suffix =".tem";
			if(tiles==&tiles_s) suffix =".sno";
			if(tiles==&tiles_u) suffix =".urb";
			if(tiles==&tiles_un) suffix =".ubn";
			if(tiles==&tiles_l) suffix =".lun";
			if(tiles==&tiles_d) suffix =".des";

			filename += suffix;
			HTSPALETTE hPalette=m_hPalIsoTemp;
			if(tiles==&tiles_s) hPalette=m_hPalIsoSnow;
			if(tiles==&tiles_u) hPalette=m_hPalIsoUrb;
			if(tiles==&tiles_t) hPalette=m_hPalIsoTemp;
			if(tiles==&tiles_un) hPalette=m_hPalIsoUbn;
			if(tiles==&tiles_d) hPalette=m_hPalIsoDes;
			if(tiles==&tiles_l) hPalette=m_hPalIsoLun;

			// MW add: use other...
			if(FindFileInMix(filename)==NULL && tiles==&tiles_un) { filename=bas_f+".urb"; hPalette=m_hPalIsoUrb; }
			if(FindFileInMix(filename)==NULL) { filename=bas_f+".tem"; hPalette=m_hPalIsoTemp; }

			(*tiledata)[tilecount].bAllowTiberium=bTib;
			(*tiledata)[tilecount].bAllowToPlace=bPlace;
			(*tiledata)[tilecount].bMorphable=bMorph;
			(*tiledata)[tilecount].bMarbleMadness=bMadness;
			(*tiledata)[tilecount].wTileSet=i;

			int reps;
			for(reps=0;reps<5;reps++)
			{
				CString r_filename=filename;
				
				if(reps>0)
				{
					char c[3];
					c[0]='a'+reps-1;
					c[1]='.';
					c[2]=0;
					
					r_filename.Replace(".", c);

					if(!LoadTile(r_filename, FindFileInMix(filename), hPalette, tilecount, TRUE))
						break;
				}
				else
				{
					LoadTile(filename, FindFileInMix(filename), hPalette, tilecount, FALSE);
				}
			}

			if (tilesetAnimSection)
			{
				auto anim = tilesetAnimSection->GetValueByName(std::format("Tile{}Anim", sId).c_str());
				auto offsetX = std::atoi(tilesetAnimSection->GetValueByName(std::format("Tile{}XOffset", sId).c_str()));
				auto offsetY = std::atoi(tilesetAnimSection->GetValueByName(std::format("Tile{}YOffset", sId).c_str()));
				auto attachesTo = std::atoi(tilesetAnimSection->GetValueByName(std::format("Tile{}AttachesTo", sId).c_str()));
				auto animFileName = anim + suffix;
				HMIXFILE hAnimMix = FindFileInMix(animFileName);
				if (hAnimMix)
				{
					auto& tile = (*tiledata)[tilecount];
					if (tile.wTileCount <= attachesTo)
					{
						ASSERT(tile.wTileCount > attachesTo);
					}
					else
					{
						auto& subtile = tile.tiles[attachesTo];
						SHPHEADER shp_h;
						SHPIMAGEHEADER shpi_h;
						auto animPic = std::make_shared<PICDATA>();
						auto rawPic = std::make_shared<std::vector<BYTE>>();
						animPic->rawPic = rawPic;
						FSunPackLib::SetCurrentSHP(animFileName, hAnimMix);
						FSunPackLib::XCC_GetSHPHeader(&shp_h);
						int imageNum = 1;
						FSunPackLib::XCC_GetSHPImageHeader(1, &shpi_h);
						if (shpi_h.unknown == 0)
						{
							// shadow
							imageNum = 0;
						}
						FSunPackLib::XCC_GetSHPImageHeader(imageNum, &shpi_h);
						if (FSunPackLib::LoadSHPImage(imageNum, *rawPic))
						{
							subtile.anim = animPic;
							animPic->pic = animPic->rawPic->data();
							animPic->bType = PICDATA_TYPE_SHP;
							animPic->pal = iPalIso;
							animPic->wWidth = animPic->wMaxWidth = shp_h.cx;
							animPic->wHeight = animPic->wMaxHeight = shp_h.cy;
							animPic->x = offsetX;
							animPic->y = offsetY;
							animPic->createVBorder();
						}
					}
				}
			}
			
			tilecount++;
			if(prog!=NULL /*&& tilecount%15==0*/)
			{
				prog->SetPos(tilecount);
				prog->UpdateWindow();
			}
		}

		
	}

	tilecount=0;	
	for(i=0;i<10000;i++)
	{
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for(e=0;e<4-strlen(c);e++)
			tset+="0";
		tset+=c;
		CString sec="TileSet";
		sec+=tset;

		if(tiles->sections.find(sec)==tiles->sections.end()) break;

		
		int madnessid=atoi(tiles->sections[sec].values["MarbleMadness"]);
		
		for(e=0;e<atoi(tiles->sections[sec].values["TilesInSet"]);e++)
		{
			if(madnessid)
			{
				(*tiledata)[tilecount].wMarbleGround=tilesets_start[madnessid]+(tilecount-tilesets_start[i]);
			}
			else
				(*tiledata)[tilecount].wMarbleGround=0xFFFF;

			tilecount++;								
			
		}

		
	}
			
}

#ifdef NOSURFACES // first version, using palettized (or extracted) data
BOOL CLoading::LoadTile(LPCSTR lpFilename, HMIXFILE hOwner, HTSPALETTE hPalette, DWORD dwID, BOOL bReplacement)
{
	last_succeeded_operation=12;

	
	int tileCount;
	
	if(FSunPackLib::XCC_DoesFileExist(lpFilename, hOwner))	
	{		
		FSunPackLib::SetCurrentTMP(lpFilename, hOwner);
		{

			//FSunPackLib::SetCurrentTMP((CString)AppPath+"\\TmpTmp.tmp"/* lpFilename*/, NULL/*hOwner*/);
			int tileWidth, tileHeight;
			RECT rect;
			FSunPackLib::XCC_GetTMPInfo(&rect, &tileCount, &tileWidth, &tileHeight);

		
			BYTE** pics=new(BYTE*[tileCount]);

			if(FSunPackLib::LoadTMPImage(0, tileCount, pics)) // be aware this allocates memory!
			//if(FSunPackLib::LoadTMPImageInSurface(v.dd,lpFilename, 0, tileCount, pics, hPalette, hOwner))
			{
				TILEDATA* td;
				if(!bReplacement) td=&(*tiledata)[dwID];
				else
				{

					TILEDATA* lpTmp=NULL;
					if((*tiledata)[dwID].bReplacementCount)
					{
						lpTmp=new(TILEDATA[(*tiledata)[dwID].bReplacementCount]);
						memcpy(lpTmp, (*tiledata)[dwID].lpReplacements, sizeof(TILEDATA)* (*tiledata)[dwID].bReplacementCount);
					}
					
					(*tiledata)[dwID].lpReplacements=new(TILEDATA[(*tiledata)[dwID].bReplacementCount+1]);

					if((*tiledata)[dwID].bReplacementCount)
					{
						memcpy((*tiledata)[dwID].lpReplacements, lpTmp, sizeof(TILEDATA)*(*tiledata)[dwID].bReplacementCount);
						delete[] lpTmp;
					}

					td=&(*tiledata)[dwID].lpReplacements[(*tiledata)[dwID].bReplacementCount];
					(*tiledata)[dwID].bReplacementCount++;
				}

							
				td->tiles=new(SUBTILE[tileCount]);
				td->wTileCount=tileCount;
				td->cx=tileWidth;
				td->cy=tileHeight;
				td->rect=rect;
				
				int i;
				for(i=0;i<tileCount;i++)
				{
					if(pics[i]!=NULL)
					{

						int cx,cy;
						BYTE height,terraintype,direction;
						POINT p;
						FSunPackLib::XCC_GetTMPTileInfo(i, &p, &cx, &cy, &direction, &height, &terraintype, &td->tiles[i].rgbLeft, &td->tiles[i].rgbRight);
						
						td->tiles[i].pic=pics[i];
						td->tiles[i].sX=p.x;
						td->tiles[i].sY=p.y;
						td->tiles[i].wWidth=cx;
						td->tiles[i].wHeight=cy;
						td->tiles[i].bZHeight=height;
						td->tiles[i].bTerrainType=terraintype;
						td->tiles[i].bHackedTerrainType=terraintype;
						td->tiles[i].bDirection=direction;

						td->tiles[i].vborder=new(VBORDER[cy]);

						int k;
						int size=0;
						BOOL TranspInside=FALSE;
						for(k=0;k<cy;k++)
						{
							int l,r;
							BOOL* ti=NULL;
							if(!TranspInside) ti=&TranspInside;
							GetDrawBorder(pics[i], cx, k, l, r, 0, ti); 
							td->tiles[i].vborder[k].left=l;
							td->tiles[i].vborder[k].right=r;

							if(r>=l)
							size+=r-l+1;
						}

#ifdef NOSURFACES_EXTRACT

						if(!TranspInside)
						{
							// extract the palette data!
							td->tiles[i].bNotExtracted=FALSE;
							td->tiles[i].pic=new(BYTE[size*bpp]);
							int l;
							int pos=0;
							for(k=0;k<cy;k++)
							{
								int left, right;
								left=td->tiles[i].vborder[k].left;
								right=td->tiles[i].vborder[k].right;
								for(l=left;l<=right;l++)
								{
									memcpy(&td->tiles[i].pic[pos], &iPalIso[pics[i][l+k*cx]], bpp);
									pos+=bpp;
								}
							}
							delete[] pics[i];
						}
						else
							td->tiles[i].bNotExtracted=TRUE;
#endif


						if(terraintype==0xa) 
						{
#ifdef RA2_MODE
							td->tiles[i].bHackedTerrainType=TERRAINTYPE_WATER;
#else
							td->tiles[i].bHackedTerrainType=TERRAINTYPE_WATER;
#endif
						}
						if(terraintype==TERRAINTYPE_ROUGH) td->tiles[i].bHackedTerrainType=TERRAINTYPE_GROUND;

						//if((*tiledata)[dwID].wTileSet==waterset) (*tiledata)[dwID].tiles[i].bHackedTerrainType=TERRAINTYPE_WATER;

						// shore hack: check fsdata.ini for new shore terrain
						if(td->wTileSet==shoreset)
						{
							int h;
							for(h=0;h<(*tiledata_count);h++)
							{
								if((*tiledata)[h].wTileSet==shoreset) break;
							}

							int pos=dwID-h;
							char c[50];
							itoa(pos,c,10);
							CString hack=c;
							hack+="_";
							itoa(i, c, 10);
							hack+=c;/*
							hack+="_";
							itoa(i/tileWidth, c, 10);
							hack+=c;*/

							CString section="ShoreTerrainTS";
#ifdef RA2_MODE
							section="ShoreTerrainRA2";
#endif
							
							if(g_data.sections[section].FindName(hack)>=0)
							{
								int t=atoi(g_data.sections[section].values[hack]);
								if(t) td->tiles[i].bHackedTerrainType=TERRAINTYPE_WATER;
								else
									td->tiles[i].bHackedTerrainType=0xe;
							}
						}
						if((*tiledata)[dwID].wTileSet==waterset) (*tiledata)[dwID].tiles[i].bHackedTerrainType=TERRAINTYPE_WATER;
					}
					else
					{
						td->tiles[i].pic=NULL;
						td->tiles[i].sX=0;
						td->tiles[i].sY=0;
						td->tiles[i].wWidth=0;
						td->tiles[i].wHeight=0;
						td->tiles[i].bZHeight=0;
						td->tiles[i].bTerrainType=0;
						td->tiles[i].bDirection=0;
						td->tiles[i].vborder=NULL;
						
					}
				}
			}
			
			if(tileCount>0) delete[] pics;
		}
	}	
	else
	{
		errstream << lpFilename << " not found" << endl;
		return FALSE;
	}
	
	
	
	

	return TRUE;
	

}
#else // now standard version, with surfaces
BOOL CLoading::LoadTile(LPCSTR lpFilename, HMIXFILE hOwner, HTSPALETTE hPalette, DWORD dwID, BOOL bReplacement)
{
	last_succeeded_operation=12;

	//errstream << "Loading " << lpFilename << " owned by " << hOwner << ", palette " << hPalette ;
	//errstream << lpFilename << endl;
	//errstream.flush();

	CIsoView& v=*((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;


	//DeleteFile((CString)AppPath+(CString)"\\TmpTmp.tmp");
	//FSunPackLib::XCC_ExtractFile(lpFilename, (CString)AppPath+(CString)"\\TmpTmp.tmp" /*lpFilename*//*, hOwner);

	int tileCount;
	try{
	if(FSunPackLib::XCC_DoesFileExist(lpFilename, hOwner))
	//if(DoesFileExist((CString)AppPath+(CString)"\\TmpTmp.tmp"))
	{
		//if(
		FSunPackLib::SetCurrentTMP(lpFilename, hOwner);
		//)
		{

			//FSunPackLib::SetCurrentTMP((CString)AppPath+"\\TmpTmp.tmp"/* lpFilename*//*, NULL/*hOwner*//*);
			int tileWidth, tileHeight;
			RECT rect;
			FSunPackLib::XCC_GetTMPInfo(&rect, &tileCount, &tileWidth, &tileHeight);

		
			LPDIRECTDRAWSURFACE4* pics=new(LPDIRECTDRAWSURFACE4[tileCount]);
			if(FSunPackLib::LoadTMPImageInSurface(v.dd,0, tileCount, pics, hPalette))
			//if(FSunPackLib::LoadTMPImageInSurface(v.dd,lpFilename, 0, tileCount, pics, hPalette, hOwner))
			{
				TILEDATA* td;
				if(!bReplacement) td=&(*tiledata)[dwID];
				else
				{

					TILEDATA* lpTmp=NULL;
					if((*tiledata)[dwID].bReplacementCount)
					{
						lpTmp=new(TILEDATA[(*tiledata)[dwID].bReplacementCount]);
						memcpy(lpTmp, (*tiledata)[dwID].lpReplacements, sizeof(TILEDATA)* (*tiledata)[dwID].bReplacementCount);
					}
					
					(*tiledata)[dwID].lpReplacements=new(TILEDATA[(*tiledata)[dwID].bReplacementCount+1]);

					if((*tiledata)[dwID].bReplacementCount)
					{
						memcpy((*tiledata)[dwID].lpReplacements, lpTmp, sizeof(TILEDATA)*(*tiledata)[dwID].bReplacementCount);
						delete[] lpTmp;
					}

					td=&(*tiledata)[dwID].lpReplacements[(*tiledata)[dwID].bReplacementCount];
					(*tiledata)[dwID].bReplacementCount++;
				}

							
				td->tiles=new(SUBTILE[tileCount]);
				td->wTileCount=tileCount;
				td->cx=tileWidth;
				td->cy=tileHeight;
				td->rect=rect;
				
				int i;
				for(i=0;i<tileCount;i++)
				{
					if(pics[i]!=NULL)
					{

						int cx,cy;
						BYTE height,terraintype,direction;
						POINT p;
						FSunPackLib::XCC_GetTMPTileInfo(i, &p, &cx, &cy, &direction, &height, &terraintype, &td->tiles[i].rgbLeft, &td->tiles[i].rgbRight);
						td->tiles[i].pic=pics[i];
						td->tiles[i].sX=p.x;
						td->tiles[i].sY=p.y;
						td->tiles[i].wWidth=cx;
						td->tiles[i].wHeight=cy;
						td->tiles[i].bZHeight=height;
						td->tiles[i].bTerrainType=terraintype;
						td->tiles[i].bHackedTerrainType=terraintype;
						td->tiles[i].bDirection=direction;

						if(terraintype==0xa) 
						{
#ifdef RA2_MODE
							td->tiles[i].bHackedTerrainType=TERRAINTYPE_WATER;
#else
							td->tiles[i].bHackedTerrainType=TERRAINTYPE_WATER;
#endif
						}
						if(terraintype==TERRAINTYPE_ROUGH) td->tiles[i].bHackedTerrainType=TERRAINTYPE_GROUND;

						//if((*tiledata)[dwID].wTileSet==waterset) (*tiledata)[dwID].tiles[i].bHackedTerrainType=TERRAINTYPE_WATER;

						// shore hack: check fsdata.ini for new shore terrain
						if(td->wTileSet==shoreset)
						{
							int h;
							for(h=0;h<(*tiledata_count);h++)
							{
								if((*tiledata)[h].wTileSet==shoreset) break;
							}

							int pos=dwID-h;
							char c[50];
							itoa(pos,c,10);
							CString hack=c;
							hack+="_";
							itoa(i, c, 10);
							hack+=c;/*
							hack+="_";
							itoa(i/tileWidth, c, 10);
							hack+=c;*/

							CString section="ShoreTerrainTS";
#ifdef RA2_MODE
							section="ShoreTerrainRA2";
#endif
							
							if(g_data.sections[section].FindName(hack)>=0)
							{
								int t=atoi(g_data.sections[section].values[hack]);
								if(t) td->tiles[i].bHackedTerrainType=TERRAINTYPE_WATER;
								else
									td->tiles[i].bHackedTerrainType=0xe;
							}
						}
						if((*tiledata)[dwID].wTileSet==waterset) (*tiledata)[dwID].tiles[i].bHackedTerrainType=TERRAINTYPE_WATER;
					}
					else
					{
						td->tiles[i].pic=NULL;
						td->tiles[i].sX=0;
						td->tiles[i].sY=0;
						td->tiles[i].wWidth=0;
						td->tiles[i].wHeight=0;
						td->tiles[i].bZHeight=0;
						td->tiles[i].bTerrainType=0;
						td->tiles[i].bDirection=0;
					}
				}
			}
			
			if(tileCount>0) delete[] pics;
		}
	}	
	else
	{
		//errstream << " not found" << endl;
		return FALSE;
	}
	}
	catch(...)
	{

	}
	
	if((*tiledata)[dwID].wTileCount==0 || (*tiledata)[dwID].tiles[0].pic==NULL)
	{
		//errstream << " failed" << endl;
		//errstream.flush();

		return FALSE;
	}
	else
	{
		//errstream << " succeeded" << endl;
		//errstream.flush();
	}

	return TRUE;
	

}
#endif

#ifdef NOSURFACES_OBJECTS // palettized
void CLoading::LoadOverlayGraphic(LPCTSTR lpOvrlName_, int iOvrlNum)
{
	last_succeeded_operation=11;

	CString image; // the image used
	CString filename; // filename of the image
	SHPHEADER head;
	char theat=cur_theat;
	BYTE** lpT=NULL;

	char OvrlID[50];
	itoa(iOvrlNum, OvrlID, 10);

	HTSPALETTE hPalette = m_hPalIsoTemp;
	if(cur_theat=='T')
	{
		hPalette=m_hPalIsoTemp;
	#ifdef RA2_MODE
		hPalette=m_hPalIsoTemp;
	#endif
	}
	if(cur_theat=='A')
	{
		hPalette=m_hPalIsoSnow;
	#ifdef RA2_MODE
		hPalette=m_hPalIsoSnow;
	#endif
	}
	if(cur_theat=='U' && m_hPalIsoUrb)
	{
		hPalette=m_hPalIsoUrb;
	#ifdef RA2_MODE
		hPalette=m_hPalIsoUrb;
	#endif		
	}
	if(cur_theat=='N' && m_hPalIsoUbn)
	{
		hPalette=m_hPalIsoUbn;
	#ifdef RA2_MODE
		hPalette=m_hPalIsoUbn;
	#endif		
	}
	if(cur_theat=='L' && m_hPalIsoLun)
	{
		hPalette=m_hPalIsoLun;
	#ifdef RA2_MODE
		hPalette=m_hPalIsoLun;
	#endif		
	}
	if(cur_theat=='D' && m_hPalIsoDes)
	{
		hPalette=m_hPalIsoDes;
	#ifdef RA2_MODE
		hPalette=m_hPalIsoDes;
	#endif		
	}

	HTSPALETTE forcedPalette = 0;
	const auto& isoPalettePrefixes = g_data.sections["ForceOvrlIsoPalettePrefix"];
	const auto& unitPalettePrefixes = g_data.sections["ForceOvrlUnitPalettePrefix"];
	const CString sOvrlName(lpOvrlName_);
	if (unitPalettePrefixes.end() != std::find_if(unitPalettePrefixes.begin(), unitPalettePrefixes.end(), [&sOvrlName](const auto& pair) {return sOvrlName.Find(pair.second) == 0;}))
	{
		forcedPalette = GetUnitPalette(cur_theat);
	}	
	if (isoPalettePrefixes.end() != std::find_if(isoPalettePrefixes.begin(), isoPalettePrefixes.end(), [&sOvrlName](const auto& pair) {return sOvrlName.Find(pair.second) == 0;}))
	{
		forcedPalette = GetIsoPalette(cur_theat);
	}
	
	HMIXFILE hMix;

	CString lpOvrlName = lpOvrlName_;
	if(lpOvrlName.Find(' ')>=0) lpOvrlName = lpOvrlName.Left(lpOvrlName.Find(' '));
	//if(strchr(lpOvrlName, ' ')!=NULL) strchr(lpOvrlName, ' ')[0]=0;
	//if(lpOvrlName

	CString isveinhole_t=rules.sections[lpOvrlName].values["IsVeinholeMonster"];
	CString istiberium_t=rules.sections[lpOvrlName].values["Tiberium"];
	CString isveins_t=rules.sections[lpOvrlName].values["IsVeins"];
	isveinhole_t.MakeLower();
	istiberium_t.MakeLower();
	isveins_t.MakeLower();
	BOOL isveinhole=FALSE, istiberium=FALSE, isveins=FALSE;
	if(isTrue(isveinhole_t)) isveinhole=TRUE;
	if(isTrue(istiberium_t)) istiberium=TRUE;
	if(isTrue(isveins_t)) isveins=TRUE;


	
 

	image=lpOvrlName;
	if(rules.sections[lpOvrlName].values.find("Image")!=rules.sections[lpOvrlName].values.end())
		image=rules.sections[lpOvrlName].values["Image"];

	TruncSpace(image);

	CString imagerules=image;
	if(art.sections[image].values.find("Image")!=art.sections[image].values.end())
		image=art.sections[image].values["Image"];

	TruncSpace(image);

	if(cur_theat=='T') filename=image+".tem";
	if(cur_theat=='A') filename=image+".sno";
	if(cur_theat=='U') filename=image+".urb";
	if(cur_theat=='N') filename=image+".ubn";
	if(cur_theat=='L') filename=image+".lun";
	if(cur_theat=='D') filename=image+".des";


	hMix=FindFileInMix(filename);

	const auto& artSection = art.sections[image];
	
	if(hMix==NULL)
	{
		filename=image+".shp";
		if(isTrue(artSection.GetValueByName("NewTheater")))
			filename.SetAt(1, theat);

		if(cur_theat=='U' && m_hPalUnitUrb) hPalette=m_hPalUnitUrb;
		if(cur_theat=='T') hPalette=m_hPalUnitTemp;
		if(cur_theat=='A') hPalette=m_hPalUnitSnow;
		if(cur_theat=='N') hPalette=m_hPalUnitUbn;
		if(cur_theat=='L') hPalette=m_hPalUnitLun;
		if(cur_theat=='D') hPalette=m_hPalUnitDes;

		hMix=FindFileInMix(filename);

		//errstream << (LPCSTR)filename << " " << endl;
		//errstream.flush();

		if(hMix==NULL)
		{
			filename.SetAt(1, 'T');
			hMix=FindFileInMix(filename);
		}
		if(hMix==NULL)
		{
			filename.SetAt(1, 'A');
			hMix=FindFileInMix(filename);
		}
		if(hMix==NULL)
		{
			filename.SetAt(1, 'U');
			hMix=FindFileInMix(filename);
		}
		if(hMix==NULL)
		{
			filename.SetAt(1, 'N');
			hMix=FindFileInMix(filename);
		}
		if(hMix==NULL)
		{
			filename.SetAt(1, 'L');
			hMix=FindFileInMix(filename);
		}
		if(hMix==NULL)
		{
			filename.SetAt(1, 'D');
			hMix=FindFileInMix(filename);
		}

		if(cur_theat=='T' || cur_theat=='U')
		{
			hPalette=m_hPalUnitTemp;
		}
		else
			hPalette=m_hPalUnitSnow;

	}
	
	if(hMix==NULL)
	{
		filename=image+".tem";
		hMix=FindFileInMix(filename);
		if(hMix) hPalette=m_hPalIsoTemp;
	}
	if(hMix==NULL)
	{
		filename=image+".sno";
		hMix=FindFileInMix(filename);
		if(hMix) hPalette=m_hPalIsoSnow;
	}
	if(hMix==NULL)
	{
		filename=image+".urb";
		hMix=FindFileInMix(filename);
		if(hMix && m_hPalIsoUrb) hPalette=m_hPalIsoUrb;
	}
	if(hMix==NULL)
	{
		filename=image+".ubn";
		hMix=FindFileInMix(filename);
		if(hMix && m_hPalIsoUbn) hPalette=m_hPalIsoUbn;
	}
	if(hMix==NULL)
	{
		filename=image+".lun";
		hMix=FindFileInMix(filename);
		if(hMix && m_hPalIsoLun) hPalette=m_hPalIsoLun;
	}
	if(hMix==NULL)
	{
		filename=image+".des";
		hMix=FindFileInMix(filename);
		if(hMix && m_hPalIsoDes) hPalette=m_hPalIsoDes;
	}

	if(isveinhole==TRUE || isveins==TRUE || istiberium==TRUE)
	{
		hPalette=m_hPalTemp;
#ifndef RA2_MODE
		hPalette=m_hPalUnitTemp;
#endif
	}

	hPalette = forcedPalette ? forcedPalette : hPalette;


	if(hMix!=NULL)
	{

		errstream << "Overlay: " << (LPCSTR) filename << endl;
		errstream.flush();
		
		FSunPackLib::SetCurrentSHP(filename, hMix);
		{
		
			FSunPackLib::XCC_GetSHPHeader(&head);

			int i;
			int maxPics=head.c_images;
			if(maxPics>max_ovrl_img) maxPics=max_ovrl_img;  

				
			// create an array of pointers to directdraw surfaces
			lpT=new(BYTE*[maxPics]);
			memset(lpT, 0, sizeof(BYTE)*maxPics);

			// if tiberium, change color
			BOOL bIsBlueTib=FALSE;
			BOOL bIsGreenTib=FALSE;
			RGBTRIPLE rgbOld[16], rgbNew;
	#ifndef RA2_MODE
			if(istiberium)
			{
				if(lpOvrlName[4]=='_') // other than green!
					bIsBlueTib=TRUE;
				else
					bIsGreenTib=TRUE;

				

				int i;
				for(i=0;i<16;i++)
				{
					if(bIsGreenTib)
					{
						rgbNew.rgbtBlue=0;
						if(i!=0)
							rgbNew.rgbtGreen=255-i*16+1;
						else
							rgbNew.rgbtGreen=255;
						rgbNew.rgbtRed=0;
					}
					else if(bIsBlueTib)
					{
						if(i!=0)
							rgbNew.rgbtBlue=255-i*16+1;
						else
							rgbNew.rgbtBlue=255;
						rgbNew.rgbtGreen=0;
						rgbNew.rgbtRed=0; // change green/blue (not RGB but BGR)
					}

					int rb=rgbNew.rgbtBlue;
					int rr=rgbNew.rgbtRed;

					FSunPackLib::SetTSPaletteEntry(hPalette, 0x10+i, &rgbNew, &rgbOld[i]);
				}
			}
	#endif

			FSunPackLib::LoadSHPImage(0, maxPics, lpT);

	#ifndef RA2_MODE
			if(istiberium)
				for(i=0;i<16;i++)
					FSunPackLib::SetTSPaletteEntry(hPalette, 0x10+i, &rgbOld[i], NULL);
	#endif

			for(i=0; i<maxPics; i++)
			{
				SHPIMAGEHEADER imghead;
				FSunPackLib::XCC_GetSHPImageHeader(i, &imghead);
				
				// MW: fixed april 20th, 2002
				if(imghead.unknown==0 && !isTrue(g_data.sections["Debug"].values["IgnoreSHPImageHeadUnused"])) // is it a shadow or not used image?
					if(lpT[i]) 
					{
						delete[] lpT[i];
						lpT[i]=NULL;
					}
				
				if(/*imghead.unknown &&*/ lpT[i])
				{
					char ic[50];
					itoa(i, ic, 10);

					PICDATA p;
					p.pic=lpT[i];

					if(hPalette==m_hPalIsoTemp || hPalette==m_hPalIsoUrb || hPalette==m_hPalIsoSnow || hPalette==m_hPalIsoDes || hPalette==m_hPalIsoLun || hPalette==m_hPalIsoUbn) p.pal=iPalIso;
					if(hPalette==m_hPalTemp || hPalette==m_hPalUrb || hPalette==m_hPalSnow || hPalette==m_hPalDes || hPalette==m_hPalLun || hPalette==m_hPalUbn) p.pal=iPalTheater;
					if(hPalette==m_hPalUnitTemp || hPalette==m_hPalUnitUrb || hPalette==m_hPalUnitSnow || hPalette==m_hPalUnitDes || hPalette==m_hPalUnitLun || hPalette==m_hPalUnitUbn) p.pal=iPalUnit;

					p.vborder=new(VBORDER[head.cy]);
					int k;
					for(k=0;k<head.cy;k++)
					{
						int l,r;
						GetDrawBorder(lpT[i], head.cx, k, l, r, 0);
						p.vborder[k].left=l;
						p.vborder[k].right=r;
					}

					p.x=imghead.x;
					p.y=imghead.y;

					p.wHeight=imghead.cy;
					p.wWidth=imghead.cx;
					p.wMaxWidth=head.cx;
					p.wMaxHeight=head.cy;
					p.bType=PICDATA_TYPE_SHP;


					pics[(CString)"OVRL"+ OvrlID + "_" + ic]=p;	
				}
			}

				
			delete[] lpT;
		} 

	}

	int i;
	for(i=0; i<0xFF; i++)
	{
		char ic[50];
		itoa(i, ic, 10);

		pics[(CString)"OVRL"+ OvrlID + "_" + ic].bTried=TRUE;	
	}

	
}

#else // surfaces
void CLoading::LoadOverlayGraphic(LPCTSTR lpOvrlName_, int iOvrlNum)
{
	last_succeeded_operation=11;

	CString image; // the image used
	CString filename; // filename of the image
	SHPHEADER head;
	char theat=cur_theat;
	LPDIRECTDRAWSURFACE4* lpT=NULL;

	char OvrlID[50];
	itoa(iOvrlNum, OvrlID, 10);

	HTSPALETTE hPalette;
	if(cur_theat=='T')
	{
		hPalette=m_hPalIsoTemp;
	#ifdef RA2_MODE
		hPalette=m_hPalIsoTemp;
	#endif
	}
	if(cur_theat=='A')
	{
		hPalette=m_hPalIsoSnow;
	#ifdef RA2_MODE
		hPalette=m_hPalIsoSnow;
	#endif
	}
	if(cur_theat=='U' && m_hPalIsoUrb)
	{
		hPalette=m_hPalIsoUrb;
	#ifdef RA2_MODE
		hPalette=m_hPalIsoUrb;
	#endif		
	}
	
	HMIXFILE hMix;

	CIsoView& v=*((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;

	CString lpOvrlName = lpOvrlName_;
	if (lpOvrlName.Find(' ') >= 0) lpOvrlName = lpOvrlName.Left(lpOvrlName.Find(' '));

	CString isveinhole_t=rules.sections[lpOvrlName].values["IsVeinholeMonster"];
	CString istiberium_t=rules.sections[lpOvrlName].values["Tiberium"];
	CString isveins_t=rules.sections[lpOvrlName].values["IsVeins"];
	isveinhole_t.MakeLower();
	istiberium_t.MakeLower();
	isveins_t.MakeLower();
	BOOL isveinhole=FALSE, istiberium=FALSE, isveins=FALSE;
	if(isTrue(isveinhole_t)) isveinhole=TRUE;
	if(isTrue(istiberium_t)) istiberium=TRUE;
	if(isTrue(isveins_t)) isveins=TRUE;


	


	image=lpOvrlName;
	if(rules.sections[lpOvrlName].values.find("Image")!=rules.sections[lpOvrlName].values.end())
		image=rules.sections[lpOvrlName].values["Image"];

	TruncSpace(image);

	CString imagerules=image;
	if(art.sections[image].values.find("Image")!=art.sections[image].values.end())
		image=art.sections[image].values["Image"];

	TruncSpace(image);

	if(cur_theat=='T') filename=image+".tem";
	if(cur_theat=='A') filename=image+".sno";
	if(cur_theat=='U') filename=image+".urb";


	hMix=FindFileInMix(filename);
	
	
	if(hMix==NULL)
	{
		filename=image+".shp";
		if(isTrue(art.sections[image].values["NewTheater"])) 
			filename.SetAt(1, theat);

		if(cur_theat=='U' && m_hPalUnitUrb) hPalette=m_hPalUnitUrb;
		if(cur_theat=='T') hPalette=m_hPalUnitTemp;
		if(cur_theat=='A') hPalette=m_hPalUnitSnow;

		hMix=FindFileInMix(filename);

		// errstream << (LPCSTR)filename << " " << hMix;

		if(hMix==NULL)
		{
			filename.SetAt(1, 'T');
			hMix=FindFileInMix(filename);
		}
		if(hMix==NULL)
		{
			filename.SetAt(1, 'A');
			hMix=FindFileInMix(filename);
		}
		if(hMix==NULL)
		{
			filename.SetAt(1, 'U');
			hMix=FindFileInMix(filename);
		}

		if(cur_theat=='T' || cur_theat=='U')
		{
			hPalette=m_hPalUnitTemp;
		}
		else
			hPalette=m_hPalUnitSnow;

	}
	
	if(hMix==NULL)
	{
		filename=image+".tem";
		hMix=FindFileInMix(filename);
		if(hMix) hPalette=m_hPalIsoTemp;
	}
	if(hMix==NULL)
	{
		filename=image+".sno";
		hMix=FindFileInMix(filename);
		if(hMix) hPalette=m_hPalIsoSnow;
	}
	if(hMix==NULL)
	{
		filename=image+".urb";
		hMix=FindFileInMix(filename);
		if(hMix && m_hPalIsoUrb) hPalette=m_hPalIsoUrb;
	}

	if(isveinhole==TRUE || isveins==TRUE || istiberium==TRUE)
	{
		hPalette=m_hPalTemp;
#ifndef RA2_MODE
		hPalette=m_hPalUnitTemp;
#endif
	}

	if(hMix!=NULL)
	{
		
		FSunPackLib::SetCurrentSHP(filename, hMix);
		{
		
			FSunPackLib::XCC_GetSHPHeader(&head);

			int i;
			int maxPics=head.c_images;
			if(maxPics>max_ovrl_img) maxPics=max_ovrl_img; // max may_ovrl_img pictures (memory usage!)
			//errstream << ", loading " << maxPics << " of " << head.c_images << " pictures. ";
			//errstream.flush();

				
			// create an array of pointers to directdraw surfaces
			lpT=new(LPDIRECTDRAWSURFACE4[maxPics]);
			memset(lpT, 0, sizeof(LPDIRECTDRAWSURFACE4)*maxPics);

			// if tiberium, change color
			BOOL bIsBlueTib=FALSE;
			BOOL bIsGreenTib=FALSE;
			RGBTRIPLE rgbOld[16], rgbNew;
	#ifndef RA2_MODE
			if(istiberium)
			{
				if(lpOvrlName[4]=='_') // other than green!
					bIsBlueTib=TRUE;
				else
					bIsGreenTib=TRUE;

				

				int i;
				for(i=0;i<16;i++)
				{
					if(bIsGreenTib)
					{
						rgbNew.rgbtBlue=0;
						if(i!=0)
							rgbNew.rgbtGreen=255-i*16+1;
						else
							rgbNew.rgbtGreen=255;
						rgbNew.rgbtRed=0;
					}
					else if(bIsBlueTib)
					{
						if(i!=0)
							rgbNew.rgbtBlue=255-i*16+1;
						else
							rgbNew.rgbtBlue=255;
						rgbNew.rgbtGreen=0;
						rgbNew.rgbtRed=0;
					}

					FSunPackLib::SetTSPaletteEntry(hPalette, 0x10+i, &rgbNew, &rgbOld[i]);
				}
			}
	#endif

			FSunPackLib::LoadSHPImageInSurface(v.dd, hPalette, 0, maxPics, lpT);

	#ifndef RA2_MODE
			if(istiberium)
				for(i=0;i<16;i++)
					FSunPackLib::SetTSPaletteEntry(hPalette, 0x10+i, &rgbOld[i], NULL);
	#endif

			for(i=0; i<maxPics; i++)
			{
				SHPIMAGEHEADER imghead;
				FSunPackLib::XCC_GetSHPImageHeader(i, &imghead);
				
				if(imghead.unknown==0) // is it a shadow or not used image?
					if(lpT[i]) lpT[i]->Release();
				
				if(imghead.unknown && lpT[i])
				{
					char ic[50];
					itoa(i, ic, 10);

					PICDATA p;
					p.pic=lpT[i];				
					p.x=imghead.x;
					p.y=imghead.y;

					p.wHeight=imghead.cy;
					p.wWidth=imghead.cx;
					p.wMaxWidth=head.cx;
					p.wMaxHeight=head.cy;
					p.bType=PICDATA_TYPE_SHP;
					


					pics[(CString)"OVRL"+ OvrlID + "_" + ic]=p;	
				}
			}

				
			delete[] lpT;

			//errstream << " --> Finished" << endl;
			//errstream.flush();
		} 
		/*else
		{
			errstream << " failed";
			errstream.flush();
		}*/
	}

	else
	{
		//errstream << "File not found: " << (LPCTSTR)filename << endl;
		//errstream.flush();	
	}	

	int i;
	for(i=0; i<0xFF; i++)
	{
		char ic[50];
		itoa(i, ic, 10);

		pics[(CString)"OVRL"+ OvrlID + "_" + ic].bTried=TRUE;	
	}

	
}
#endif

void CLoading::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CLoading::InitVoxelNormalTables()
{
	try
	{
		std::ifstream f(std::string(AppPath) + "\\voxel_normal_tables.bin", std::ios::binary);
		m_voxelNormalTables.reset(new VoxelNormalTables(f));
	}
	catch (const std::runtime_error& e)
	{
		errstream << e.what() << std::endl;
		m_voxelNormalTables.reset(new VoxelNormalTables());
	}
}

void CLoading::CalcPicCount()
{
	m_pic_count=0;

	CString bmps=(CString)AppPath+"\\pics\\*.bmp";
	if(!theApp.m_Options.bDoNotLoadBMPs)
	{
		CFileFind ff;
		if(ff.FindFile(bmps))
		{
			// found bmp
			m_pic_count++;
			while(ff.FindNextFile()) m_pic_count++;
		}
	}

	m_bmp_count=m_pic_count;

	if(!theApp.m_Options.bDoNotLoadVehicleGraphics)	m_pic_count+=rules.sections["VehicleTypes"].values.size();
	if(!theApp.m_Options.bDoNotLoadOverlayGraphics) m_pic_count+=rules.sections["OverlayTypes"].values.size();
	if(!theApp.m_Options.bDoNotLoadInfantryGraphics) m_pic_count+=rules.sections["InfantryTypes"].values.size();
	if(!theApp.m_Options.bDoNotLoadBuildingGraphics) m_pic_count+=rules.sections["BuildingTypes"].values.size();
	if(!theApp.m_Options.bDoNotLoadAircraftGraphics) m_pic_count+=rules.sections["AircraftTypes"].values.size();
	if(!theApp.m_Options.bDoNotLoadTreeGraphics) m_pic_count+=rules.sections["TerrainTypes"].values.size();

	int i;
/*
	if(!theApp.m_Options.bDoNotLoadSnowGraphics)
	{
		tiledata=&s_tiledata;
		tiles=&tiles_s;
		tiledata_count=&s_tiledata_count;	
		for(i=0;i<10000;i++)
		{
			CString tset;
			char c[50];
			itoa(i, c, 10);
			int e;
			for(e=0;e<4-strlen(c);e++)
				tset+="0";
			tset+=c;
			CString sec="TileSet";
			sec+=tset;

			if(tiles->sections.find(sec)==tiles->sections.end()) break;
				
			for(e=0;e<atoi(tiles->sections[sec].values["TilesInSet"]);e++)
			{
				m_pic_count++;
			}
				
		}
	}

	if(!theApp.m_Options.bDoNotLoadTemperateGraphics)
	{
		tiledata=&t_tiledata;
		tiles=&tiles_t;
		tiledata_count=&t_tiledata_count;	
		for(i=0;i<10000;i++)
		{
			CString tset;
			char c[50];
			itoa(i, c, 10);
			int e;
			for(e=0;e<4-strlen(c);e++)
				tset+="0";
			tset+=c;
			CString sec="TileSet";
			sec+=tset;

			if(tiles->sections.find(sec)==tiles->sections.end()) break;
				
			for(e=0;e<atoi(tiles->sections[sec].values["TilesInSet"]);e++)
			{
				m_pic_count++;
			}
				
		}
	}*/
}


BOOL CLoading::InitDirectDraw()
{
	last_succeeded_operation=7;

	errstream << "\n\nDirectDrawCreate() will be called now\n";
	errstream.flush();
		
	CIsoView& v=*((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;
	if(DirectDrawCreate(NULL, &v.dd_1, NULL)!=DD_OK)
	{
		errstream << "DirectDrawCreate() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("DirectDraw could not be initialized! Quitting...");
		exit(-1);

		return FALSE;
	} 

	errstream << "DirectDrawCreate() successful\n\n";
	errstream.flush();

	if(m_progress.m_hWnd) m_progress.SetPos(1);
	if(m_hWnd) UpdateWindow();

	errstream << "Now querying the DirectX 7 or 6 interface\n";
	errstream.flush();

	if(v.dd_1->QueryInterface(IID_IDirectDraw7, (void**)&v.dd)!=DD_OK)
	{
		errstream << "QueryInterface() failed -> Using DirectX 6.0\n";
		errstream.flush();
		//ShowWindow(SW_HIDE);
		//MessageBox("You don´t have DirectX 6.0 but an older version. Quitting...");
		//exit(-1);

		//return FALSE;

		if(v.dd_1->QueryInterface(IID_IDirectDraw4, (void**)&v.dd)!=DD_OK)
		{
			MessageBox("You need at least DirectX 6.0 to run this program", "Error");
			exit(-1);
			return FALSE;
		}
	}

	errstream << "QueryInterface() successful\n\nNow setting cooperative level\n";
	errstream.flush();
		
	if(v.dd->SetCooperativeLevel(v.m_hWnd, DDSCL_NORMAL | DDSCL_NOWINDOWCHANGES )!=DD_OK)
	{
		errstream << "SetCooperativeLevel() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("Cooperative Level could not be set! Quitting...");
		v.dd->Release();
		v.dd=NULL;
		exit(-2);

		return FALSE;
	}

	errstream << "SetCooperativeLevel() successful\n\nCreating primary surface\n";
	errstream.flush();


	if(m_progress.m_hWnd) m_progress.SetPos(2);
	if(m_hWnd) UpdateWindow();
		
	DDSURFACEDESC2 ddsd;


	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	ddsd.dwSize=sizeof(DDSURFACEDESC2);
	ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE ;
	ddsd.dwFlags=DDSD_CAPS ;

    

	int res=0;
	int trycount=0;
	do
	{
		res=v.dd->CreateSurface(&ddsd, &v.lpds, NULL);
		errstream << "Return code: " << res << endl;
		errstream.flush();

		//if(res!=DD_OK && (res!=DDERR_PRIMARYSURFACEALREADYEXISTS || trycount>100))
		if(res!=DD_OK && trycount>=300)
		{
			
			errstream << "CreateSurface() failed\n";
			
			errstream.flush();
			ShowWindow(SW_HIDE);
			MessageBox("Primary surface could not be initialized! Quitting...");
			v.dd->Release();
			v.dd=NULL;
			exit(-3);

			return FALSE;
		}
		trycount++;
		if(res!=DD_OK)
		{ 
			Sleep(50);
		}

		
	}while(res!=DD_OK);

#ifdef NOSURFACES
	DDPIXELFORMAT pf;
	memset(&pf, 0, sizeof(DDPIXELFORMAT));
	pf.dwSize=sizeof(DDPIXELFORMAT);

	v.lpds->GetPixelFormat(&pf);

	if(!pf.dwBBitMask || !pf.dwRBitMask || !pf.dwGBitMask)
	{
		ShowWindow(SW_HIDE);
		MessageBox("You must not use a palette color mode like 8 bit in order to run FinalSun/FinalAlert 2. Please check readme.txt","Error",MB_OK);

		v.lpds->Release();
		v.lpds=NULL;
		v.dd->Release();
		v.dd=NULL;
		exit(-3);
		return FALSE;
	}
	bpp=(pf.dwRGBBitCount+7)/8;
#endif


	errstream << "CreateSurface() successful\n\nCreating backbuffer surface\n";
	errstream.flush();

	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	ddsd.dwSize=sizeof(DDSURFACEDESC2);
	ddsd.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
	v.lpds->GetSurfaceDesc(&ddsd);
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;


	ddsd.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;

	
	if(v.dd->CreateSurface(&ddsd, &v.lpdsBack, NULL)!=DD_OK)
	{
		errstream << "CreateSurface() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("Backbuffer surface could not be initialized! Quitting...");
		v.lpds->Release();
		v.lpds=NULL;
		v.dd->Release();
		v.dd=NULL;
		exit(-4);

		return FALSE;
	}
	if (theApp.m_Options.bHighResUI && v.dd->CreateSurface(&ddsd, &v.lpdsBackHighRes, NULL) != DD_OK)
	{
		errstream << "CreateSurface() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("Highres Backbuffer surface could not be initialized! Quitting...");
		v.lpdsBack->Release();
		v.lpdsBack = NULL;
		v.lpds->Release();
		v.lpds = NULL;
		v.dd->Release();
		v.dd = NULL;
		exit(-4);

		return FALSE;
	}
	if(v.dd->CreateSurface(&ddsd, &v.lpdsTemp, NULL)!=DD_OK)
	{
		errstream << "CreateSurface() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("Tempbuffer surface could not be initialized! Quitting...");
		v.lpdsBack->Release();
		v.lpdsBack=NULL;
		if (v.lpdsBackHighRes)
			v.lpdsBackHighRes->Release();
		v.lpdsBackHighRes = nullptr;
		v.lpds->Release();
		v.lpds=NULL;
		v.dd->Release();
		v.dd=NULL;
		exit(-4);

		return FALSE;
	}

	errstream << "CreateSurface() successful\n\nNow creating clipper\n";
	errstream.flush();

	LPDIRECTDRAWCLIPPER ddc;
	if(v.dd->CreateClipper(0, &ddc, NULL)!=DD_OK)
	{
		errstream << "CreateClipper() failed\n";
		errstream.flush();
		ShowWindow(SW_HIDE);
		MessageBox("Clipper could not be created! Quitting...");
		v.lpdsTemp->Release();
		v.lpdsTemp=NULL;
		v.lpdsBack->Release();
		v.lpdsBack=NULL;
		if (v.lpdsBackHighRes)
			v.lpdsBackHighRes->Release();
		v.lpdsBackHighRes = nullptr;
		v.lpds->Release();
		v.lpds=NULL;
		v.dd->Release();
		v.dd=NULL;
		exit(-6);
	}

	errstream << "CreateClipper() successful\n\n";
	errstream.flush();

	v.lpds->SetClipper(ddc);
	
	ddc->SetHWnd(0, v.m_hWnd);

	return TRUE;
}

void CLoading::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CWnd* poswindow=GetDlgItem(IDC_SAVEOFTEN);

	if(!poswindow) return;

	RECT r1,r2;
	poswindow->GetWindowRect(&r1);
	GetWindowRect(&r2);

	dc.SetTextColor(RGB(255, 0, 0));
	dc.SetBkMode(TRANSPARENT);

	RECT dest;
	dest.top=r1.bottom-r2.top-45;
	dest.bottom=r1.bottom-r2.top+10;
	dest.left=10;
	dest.right=r1.right-r1.left+10;

	CFont f;
	f.CreatePointFont(90, "Tahoma");
	dc.SelectObject(&f);

}

void CLoading::FreeTileSet()
{
	// free the current tileset

	int i;
	for(i=0;i<(*tiledata_count);i++)
	{
		int e;
		int z;
 
		// delete replacements. Replacements themself MUST NOT have replacements
		for(z=0;z<(*tiledata)[i].bReplacementCount;z++)
		{
			TILEDATA& rept=(*tiledata)[i].lpReplacements[z];
			
			for(e=0;e<rept.wTileCount;e++)
			{
#ifdef NOSURFACES
				BYTE* curSur=rept.tiles[e].pic;
				if(curSur) delete[] curSur;
				if(rept.tiles[e].vborder) delete[] rept.tiles[e].vborder;
#else
				LPDIRECTDRAWSURFACE4 curSur=rept.tiles[e].pic;
				if(curSur) curSur->Release();
#endif
			}
			
			delete[] rept.tiles;
			rept.tiles=NULL;
			rept.wTileCount=0;
			rept.bReplacementCount=0;

			ASSERT(!rept.lpReplacements); // make sure we don´t have replacements for this replacement
		}

		for(e=0;e<(*tiledata)[i].wTileCount;e++)
		{
#ifdef NOSURFACES
			BYTE* curSur=(*tiledata)[i].tiles[e].pic;
			if(curSur) delete[] curSur;
			if((*tiledata)[i].tiles[e].vborder) delete[] (*tiledata)[i].tiles[e].vborder;
#else
			LPDIRECTDRAWSURFACE4 curSur=(*tiledata)[i].tiles[e].pic;
			if(curSur) curSur->Release();
#endif
		}

		delete[] ((*tiledata)[i].tiles);
		(*tiledata)[i].tiles=NULL;
		(*tiledata)[i].wTileCount=0;
		(*tiledata)[i].bReplacementCount=0;
		if((*tiledata)[i].lpReplacements) delete[] (*tiledata)[i].lpReplacements;
		(*tiledata)[i].lpReplacements=NULL;
	}
	if(*tiledata) delete[] (*tiledata);
	(*tiledata)=NULL;
	(*tiledata_count)=0;
}

void CLoading::FreeAll()
{
	last_succeeded_operation=14;

	// MW fix: We need to set tiledata and tiledata_count to the old pointers again
	int t=0;
	if(tiledata==&t_tiledata) t=0;
	if(tiledata==&s_tiledata) t=1;
	if(tiledata==&u_tiledata) t=2;
	if(tiledata==&un_tiledata) t=3;
	if(tiledata==&l_tiledata) t=4;
	if(tiledata==&d_tiledata) t=4;

	//try{
	tiledata=&t_tiledata;
	tiledata_count=&t_tiledata_count;
	FreeTileSet();
	tiledata=&s_tiledata;
	tiledata_count=&s_tiledata_count;
	FreeTileSet();
	tiledata=&u_tiledata;
	tiledata_count=&u_tiledata_count;
	FreeTileSet();

	// MW New tilesets:
	tiledata=&un_tiledata;
	tiledata_count=&un_tiledata_count;
	FreeTileSet();
	tiledata=&l_tiledata;
	tiledata_count=&l_tiledata_count;
	FreeTileSet();
	tiledata=&d_tiledata;
	tiledata_count=&d_tiledata_count;
	FreeTileSet();
	/* }
	catch(...)
	{
		errstream << "Exception in FreeTileSet()" << endl;
	}*/

	// MW Reset tiledata & tiledata_count
	if(t==0) { tiledata=&t_tiledata; tiledata_count=&t_tiledata_count; }
	if(t==1) { tiledata=&s_tiledata; tiledata_count=&s_tiledata_count; }
	if(t==2) { tiledata=&u_tiledata; tiledata_count=&u_tiledata_count; }
	if(t==3) { tiledata=&un_tiledata; tiledata_count=&un_tiledata_count; }
	if(t==4) { tiledata=&l_tiledata; tiledata_count=&l_tiledata_count; }
	if(t==5) { tiledata=&d_tiledata; tiledata_count=&d_tiledata_count; }

	map<CString, PICDATA>::iterator i=pics.begin();
	for (int e=0;e<pics.size();e++)
	{
		try{
#ifdef NOSURFACES_OBJECTS			
			if(i->second.bType==PICDATA_TYPE_BMP)
			{
				if(i->second.pic!=NULL) 
				{
					((LPDIRECTDRAWSURFACE4)i->second.pic)->Release();
				}
			}
			else
			{
				if(i->second.pic!=NULL) 
				{
					delete[] i->second.pic;
				}
				if(i->second.vborder) delete[] i->second.vborder;
			}
#else
			if(i->second.pic!=NULL) i->second.pic->Release();
#endif

			i->second.pic=NULL;
		}
		catch(...)
		{
			CString err;
			err="Access violation while trying to release surface ";
			char c[6];
			itoa(e,c,10);
			err+=c;
				
			err+="\n";
			OutputDebugString(err);	
			continue;
		}
			
		i++;
	}



	try{
	CFinalSunDlg* dlg=((CFinalSunDlg*)theApp.m_pMainWnd);
	if(dlg->m_view.m_isoview->lpds!=NULL) dlg->m_view.m_isoview->lpds->Release();
	dlg->m_view.m_isoview->lpds=NULL;
	if(dlg->m_view.m_isoview->lpdsBack!=NULL) dlg->m_view.m_isoview->lpdsBack->Release();
	dlg->m_view.m_isoview->lpdsBack=NULL;
	if (dlg->m_view.m_isoview->lpdsBackHighRes != NULL) dlg->m_view.m_isoview->lpdsBackHighRes->Release();
	dlg->m_view.m_isoview->lpdsBackHighRes = NULL;
	if(dlg->m_view.m_isoview->lpdsTemp!=NULL) dlg->m_view.m_isoview->lpdsTemp->Release();
	dlg->m_view.m_isoview->lpdsTemp=NULL;
	if(dlg->m_view.m_isoview->dd!=NULL) dlg->m_view.m_isoview->dd->Release();
	dlg->m_view.m_isoview->dd=NULL;
	if(dlg->m_view.m_isoview->dd_1!=NULL) dlg->m_view.m_isoview->dd_1->Release();
	dlg->m_view.m_isoview->dd_1=NULL;
	}
	catch(...)
	{
		errstream << "Exception while freeing DirectDraw" << endl;	
	}
} 

void CLoading::PostNcDestroy() 
{

//	delete this; // on stack!
//	CDialog::PostNcDestroy();
}

void CLoading::PrepareHouses()
{
	int i;
	int p=0 ;
	for(i=0;i<rules.sections["Sides"].values.size();i++)
	{
		int t=0;
		while(GetParam(*rules.sections["Sides"].GetValue(i), t).GetLength()>0)
		{
			sides[p].name=GetParam(*rules.sections["Sides"].GetValue(i), t);
			sides[p].orig_n=rules.sections["Sides"].GetValueOrigPos(i); // mw fix instead of =i
			t++;
			p++;
		}
	}

	for(i=0;i<rules.sections[HOUSES].values.size();i++)
	{
		if(rules.sections[HOUSES].GetValueOrigPos(i)<0) continue;

		HOUSEINFO& house=houses[rules.sections[HOUSES].GetValueOrigPos(i)];
		house.name=*rules.sections[HOUSES].GetValue(i);
		house.bPlayable=isTrue(rules.sections[house.name].values["Multiplay"]);
		memset(&house.color, 0, sizeof(RGBTRIPLE));
		int e;
		for(e=0;e<sides.size();e++)
		{
			house.side=NULL;
			if(sides[e].name==rules.sections[house.name].values["Side"])
				house.side=&sides[e];
		}
		
	}
}


BYTE* Search(BYTE** lpData, BYTE* lpSearched)
{
	BYTE* lpDat=*lpData;
	
	lpDat=(BYTE*)strstr((LPCSTR)lpDat, (LPCSTR)lpSearched)+strlen((LPCSTR)lpSearched);

	return lpDat;
}

class SortDummy2{
public:
	bool operator() (const CString& x, const CString& y) const
	{
		// the length is more important than spelling (numbers!!!)...
		if(x.GetLength()<y.GetLength()) return true;
		if(x.GetLength()==y.GetLength())
		{	
			CString x2=x;
			CString y2=y;
			x2.MakeLower();
			y2.MakeLower();
			if(x2<y2) return true;
		}

		return false;
	
	}
};

extern map<CString, XCString> AllStrings;
void CLoading::LoadStrings()
{
	last_succeeded_operation=9;

#ifdef RA2_MODE

	// MW April 17th, 2002:
	// ra2md.csf supported!
	std::string file="RA2.CSF";
	if(yuri_mode)
		file="RA2MD.CSF";

	errstream << "LoadStrings() executing" << endl;
	errstream.flush();

	BYTE *lpData=NULL;
	DWORD dwSize;
	if(DoesFileExist((std::string(TSPath) + "\\" + file).c_str()))
	{
		std::ifstream f(std::string(TSPath) + "\\" + file, ios::binary);
		if (f.good())
		{
			f.seekg(0, std::ios::end);
			auto size = f.tellg();
			if (size > 0)
			{
				lpData = new(BYTE[size]);
				dwSize = size;
				f.seekg(0, std::ios::beg);
				f.read(reinterpret_cast<char*>(lpData), dwSize);
			}
		}
	}
	errstream << "LoadStrings() loading from mix" << endl;
	errstream.flush();

	if(!lpData)
	{
		HMIXFILE hMix=FindFileInMix(file.c_str());
		//HMIXFILE hMix=m_hLanguage;
		if(hMix)
		{
			if (FSunPackLib::XCC_ExtractFile(file, u8AppDataPath + "\\RA2Tmp.csf", hMix))
			{
				std::ifstream f(u8AppDataPath + "\\RA2Tmp.csf", ios::binary);
				if (f.good())
				{
					f.seekg(0, std::ios::end);
					auto size = f.tellg();
					if (size > 0)
					{
						lpData = new(BYTE[size]);
						dwSize = size;
						f.seekg(0, std::ios::beg);
						f.read(reinterpret_cast<char*>(lpData), dwSize);
					}
				}
			}
			
			if (!lpData)
			{
				MessageBox("String file not found, using rules.ini names","Error");
				return;
			}
		}
		else
		{
			MessageBox("String file not found, using rules.ini names","Error");
			return;
		}
		
	}

	BYTE* orig=static_cast<BYTE*>(lpData);

	if(!(lpData=Search(&lpData, (BYTE*)" FSC"))) return;

	RA2STRFILEHEAD head;
	memcpy(&head, lpData, RA2STRFILEHEADSIZE);
	

	lpData+=RA2STRFILEHEADSIZE;
	
	map<CString, XCString, SortDummy2> strings;

	int i;
	//try{
	for(i=0;i<head.dwCount1;i++)
	{
		ASSERT(orig+dwSize>lpData);

		if(!(lpData=lpData+4))//Search(&lpData, (BYTE*)" LBL"))) 
		{
			
			return;
		}

		RA2STRINGENTRY entry;		
		memcpy(&entry.dwFlag, lpData, 4);
		lpData+=4;
		
		DWORD dwCharCount;
		memcpy(&dwCharCount, lpData, 4);
		lpData+=4;

		BYTE* lpID=new(BYTE[dwCharCount+1]);
		memcpy(lpID, lpData, dwCharCount);
		lpData+=dwCharCount;
		lpID[dwCharCount]=0;
		entry.id=(CHAR*)new(BYTE[dwCharCount+1]);
		strcpy(entry.id, (LPCSTR)lpID);
		entry.id[dwCharCount]=0;
		entry.id_size=dwCharCount;
		// enable this to show the string ID
		delete[](lpID);

		
		BOOL b2Strings=FALSE;

		if(lpData[0]=='W')
			 b2Strings=TRUE;
		
		if(!(lpData=lpData+4))//Search(&lpData, (BYTE*)" RTS")))
		{			
			return;
		}

		memcpy(&dwCharCount, lpData, 4);
		lpData+=4;

		WCHAR* lpwID=new(WCHAR[dwCharCount+1]);
		int e;
		for(e=0;e<dwCharCount;e++)
		{
			WCHAR w;
			memcpy(&w, lpData, 2);
			lpData+=2;
			lpwID[e]=~w;
		}
		lpwID[dwCharCount]=0;
		entry.value=lpwID;
		entry.value_size=dwCharCount;

		if(b2Strings)
		{
			
			memcpy(&dwCharCount, lpData, 4);
			lpData+=4;
			
			

			CHAR* lpwID2=new(CHAR[dwCharCount+1]);
			int e;
			for(e=0;e<dwCharCount;e++)
			{
				WCHAR w;
				memcpy(&w, lpData, 1);
				lpData+=1;
				lpwID2[e]=w;
			}
			lpwID2[dwCharCount]=0;
			entry.value_asc=lpwID2;
			entry.value_asc_size=dwCharCount;
		}

		/*BYTE* bByte=new(BYTE[entry.value_size+1]);
		wcstombs((LPSTR)bByte, entry.value, entry.value_size+1);
		bByte[entry.value_size]=0;*/
		strings[entry.id].SetString(entry.value, entry.value_size);
		AllStrings[entry.id].SetString(entry.value, entry.value_size);


		//delete[] bByte;
	}

	

	for(i=0;i<rules.sections.size();i++)
	{
		if(rules.GetSection(i)->FindName("UIName")>=0)
		{
			int e;
		
			if(strings.find(rules.GetSection(i)->values["UIName"])!=strings.end())
			{
				//MessageBox(strings[rules.GetSection(i)->values["UIName"]].cString);
				if(!strings[rules.GetSection(i)->values["UIName"]].bUsedDefault)
				{
					//CCStrings[*rules.GetSectionName(i)].cString=strings[rules.GetSection(i)->values["UIName"]].cString; //.SetString(strings[rules.GetSection(i)->values["UIName"]].wString, strings[rules.GetSection(i)->values["UIName"]].len);
					CCStrings[*rules.GetSectionName(i)].SetString(strings[rules.GetSection(i)->values["UIName"]].wString, strings[rules.GetSection(i)->values["UIName"]].len);
				}
				else
				{
					CCStrings[*rules.GetSectionName(i)].SetString(strings[rules.GetSection(i)->values["UIName"]].wString, strings[rules.GetSection(i)->values["UIName"]].len);
					CCStrings[*rules.GetSectionName(i)].cString=rules.GetSection(i)->GetValueByName("Name");
				}
			}
			else 
			{
				//MessageBox((LPSTR)(LPCSTR)rules.GetSection(i)->values["Name"], *rules.GetSectionName(i));
				CCStrings[*rules.GetSectionName(i)].SetString((LPSTR)(LPCSTR)rules.GetSection(i)->GetValueByName("Name"));
			}
		}
		else CCStrings[*rules.GetSectionName(i)].SetString((LPSTR)(LPCSTR)rules.GetSection(i)->GetValueByName("Name"));

	}
	
	
#else
	int i;
	for(i=0;i<rules.sections.size();i++)
	{
		if(rules.GetSection(i)->FindName("Name")>=0)
		{
			//CCStrings[*rules.GetSectionName(i)].cString=rules.GetSection(i)->values["Name"];
			//CCStrings[*rules.GetSectionName(i)].SetString=rul
			CCStrings[*rules.GetSectionName(i)].SetString((LPSTR)(LPCSTR)rules.GetSection(i)->values["Name"]);
		}
	}
#endif


}

void CLoading::HackRules()
{
	if(editor_mode==ra2_mode)
	{
		int i;
		int max_c=0;
		for(i=0;i<rules.sections["BuildingTypes"].values.size();i++)
		{
			int p=atoi(*rules.sections["BuildingTypes"].GetValueName(i));
			if(p>max_c) max_c=p;
		}

		char c[50];		
		itoa(max_c+1,c,10);

		rules.sections["BuildingTypes"].values[c]=rules.sections["General"].values["GDIGateOne"];

#ifdef RA2_MODE
		// RULES(MD).INI has the incorrect colors set for the following houses, let's remap them to the expected values.
		// Fixup YuriCountry colour
		if (rules.sections["YuriCountry"].GetValueByName("Color") == "DarkRed") {
			rules.sections["YuriCountry"].values["Color"] = "Purple";
		}
		// Fixup Allied colors
		std::list<CString> allied_houses;
		allied_houses.push_back("British");
		allied_houses.push_back("French");
		allied_houses.push_back("Germans");
		allied_houses.push_back("Americans");
		allied_houses.push_back("Alliance");
		for (std::list<CString>::iterator it = allied_houses.begin(); it != allied_houses.end(); ++it) {
			if (rules.sections[*it].GetValueByName("Color") == "Gold") {
				rules.sections[*it].values["Color"] = "DarkBlue";
			}
		}
		// Fixup Nod color
		if (rules.sections["Nod"].GetValueByName("Color") == "Gold") {
			rules.sections["Nod"].values["Color"] = "DarkRed";
		}
#endif

	}
	
}

void CLoading::PrepareBuildingTheaters()
{
	// stub

}

/*
This actually just checks what palette the unit (only buildings make sense)
uses. Used for the ObjectBrowser (so it only shows those buildings that really exist
in the specific terrain)

Added: MW March 20th 2001
*/
void CLoading::PrepareUnitGraphic(LPCSTR lpUnittype)
{
	CString _rules_image; // the image used
	CString filename; // filename of the image
	char theat=cur_theat; // standard theater char is t (Temperat). a is snow.

	BOOL bAlwaysSetChar; // second char is always theater, even if NewTheater not specified!
	WORD wStep=1; // step is 1 for infantry, buildings, etc, and for shp vehicles it specifies the step rate between every direction
	WORD wStartWalkFrame=0; // for examply cyborg reaper has another walk starting frame
	int iTurretOffset=0; // used for centering y pos of turret (if existing)
	BOOL bStructure=rules.sections["BuildingTypes"].FindValue(lpUnittype)>=0; // is this a structure?

	if(!bStructure) return; // make sure we only use it for buildings now

	BOOL bPowerUp=rules.sections[lpUnittype].values["PowersUpBuilding"]!="";
		
	HTSPALETTE hPalette;
	if(theat=='T') hPalette=m_hPalIsoTemp;
	if(theat=='A') hPalette=m_hPalIsoSnow;
	if(theat=='U') hPalette=m_hPalIsoUrb;
	if(theat=='L') hPalette=m_hPalIsoLun;
	if(theat=='D') hPalette=m_hPalIsoDes;
	if(theat=='N') hPalette=m_hPalIsoUbn;
	
	CIsoView& v=*((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;

	_rules_image=lpUnittype;
	if(rules.sections[lpUnittype].values.find("Image")!=rules.sections[lpUnittype].values.end())
		_rules_image=rules.sections[lpUnittype].values["Image"];

	CString _art_image = _rules_image;
	if(art.sections[_rules_image].values.find("Image")!=art.sections[_rules_image].values.end())
	{
		if(!isTrue(g_data.sections["IgnoreArtImage"].values[_rules_image]))
			_art_image=art.sections[_rules_image].values["Image"];
	}

	const CString& image = _art_image;
	const auto& rulesSection = rules.sections[lpUnittype];
	const auto& artSection = art.sections[image];

	if(!isTrue(art.sections[image].values["Voxel"])) // is it a shp graphic?
	{
		try
		{

		/*filename = image + ".shp";

				
		BYTE bTerrain=0;

		
		
		BOOL isNewTerrain=FALSE;
		if(isTrue(artSection.GetValueByName("NewTheater")))//&& isTrue(artSection.GetValueByName("TerrainPalette")))//(filename.GetAt(0)=='G' || filename.GetAt(0)=='N' || filename.GetAt(0)=='C') && filename.GetAt(1)=='A')
		{
			hPalette=m_hPalUnitTemp;
			if(theat=='A') hPalette=m_hPalUnitSnow;
			if(theat=='U') hPalette=m_hPalUnitUrb;
			if(theat=='L') hPalette=m_hPalUnitLun;
			if(theat=='D') hPalette=m_hPalUnitDes;
			if(theat=='N') hPalette=m_hPalUnitUbn;
			filename.SetAt(1, theat);
			isNewTerrain=TRUE;
		}
		
		
		HMIXFILE hShpMix=FindFileInMix(filename, &bTerrain);
		
		BYTE bIgnoreTerrain=TRUE;
		
		if(hShpMix==NULL && isNewTerrain)
		{
			filename.SetAt(1, 'G');
			hShpMix=FindFileInMix(filename, &bTerrain);
			if(hShpMix) theat='G';
			
		}


		if(hShpMix==NULL && isNewTerrain)
		{
			filename.SetAt(1, 'N');
			hShpMix=FindFileInMix(filename, &bTerrain);
			if(hShpMix) theat='N';
		}
		
		if(hShpMix==NULL && isNewTerrain)
		{
			filename.SetAt(1, 'D');
			hShpMix=FindFileInMix(filename, &bTerrain);
			if(hShpMix) theat='D';
		}

		if(hShpMix==NULL && isNewTerrain)
		{
			filename.SetAt(1, 'L');
			hShpMix=FindFileInMix(filename, &bTerrain);
			if(hShpMix) theat='L';
		}
		
		if(hShpMix==NULL && isNewTerrain)
		{
			filename.SetAt(1, 'A');
			hShpMix=FindFileInMix(filename, &bTerrain);
			if(hShpMix) theat='A';
		}
		
		if(hShpMix==NULL && isNewTerrain)
		{
			filename.SetAt(1, 'T');
			hShpMix=FindFileInMix(filename, &bTerrain);
			if(hShpMix){
				theat='T';
				hPalette=m_hIsoTemp;
			}
		}

		
		if(isTrue(artSection.GetValueByName("TerrainPalette")))
		{
			bIgnoreTerrain=FALSE;
			
			if(cur_theat=='T')
			hPalette=m_hPalIsoTemp;
			else if(cur_theat=='A')
				hPalette=m_hPalIsoSnow;
			else if (cur_theat=='U')
				hPalette=m_hPalIsoUrb;
			else if(cur_theat=='L')
				hPalette=m_hPalIsoLun;
			else if(cur_theat=='D')
				hPalette=m_hPalIsoDes;
			else if(cur_theat=='N')
				hPalette=m_hPalIsoUbn;
				
			
			
		}


		
		if(hShpMix==0) 
		{
			filename=image;
			filename+=".shp";
			hShpMix=FindFileInMix(filename, &bTerrain);

			
			
			if(hShpMix==NULL)
			{
				filename=image;
				if(theat=='T') filename+=".tem";
				if(theat=='A') filename+=".sno";
				if(theat=='U') filename+=".urb";
				if(theat=='L') filename+=".lun";
				if(theat=='D') filename+=".des";
				if(theat=='N') filename+=".ubn";
				filename.MakeLower();
				hShpMix=FindFileInMix(filename, &bTerrain);
				
				if(hShpMix==NULL)
				{
					filename=image;
					filename+=".tem";
					hShpMix=FindFileInMix(filename, &bTerrain);
					if(hShpMix)
					{
						hPalette=m_hPalIsoTemp;
					}
				}
				
				if(hShpMix!=NULL)
				{
				
					
					
				}
				else
				{
					filename=image+".shp";

					filename.SetAt(1, 'A');

					hShpMix=FindFileInMix(filename);

					if(hShpMix!=NULL)
					{
						bAlwaysSetChar=TRUE;
					}
					else
					{
						filename.SetAt(1, 'A');
						hShpMix=FindFileInMix(filename);

						if(hShpMix!=NULL)
						{
							theat='A';
							bAlwaysSetChar=TRUE;
						}
						else
						{
							filename.SetAt(1, 'U');
							hShpMix=FindFileInMix(filename);
							if(hShpMix) theat='U';
							else
							{
								filename.SetAt(1, 'L');
								hShpMix=FindFileInMix(filename);
								if(hShpMix) theat='L';
								else
								{
									filename.SetAt(1, 'D');
									hShpMix=FindFileInMix(filename);
									if(hShpMix) theat='D';
									else
									{
										filename.SetAt(1, 'N');
										hShpMix=FindFileInMix(filename);
										if(hShpMix) theat='N';
										else
										{
											filename.SetAt(1, 'T');
											hShpMix=FindFileInMix(filename);
											if(hShpMix) theat='T';
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				theat='T';
			}

		}
		else
		{

			// now we need to find out the palette
			
				if(isTrue(artSection.GetValueByName("TerrainPalette"))) // it´s a file in isotemp.mix/isosno.mix
				{
										
				}
				else // it´s a file in temperat.mix/snow.mix
				{
					if(cur_theat=='T') hPalette=m_hPalUnitTemp;
					if(cur_theat=='A') hPalette=m_hPalUnitSnow;
					if(cur_theat=='U') hPalette=m_hPalUnitUrb;
					if(cur_theat=='L') hPalette=m_hPalUnitLun;
					if(cur_theat=='D') hPalette=m_hPalUnitDes;
					if(cur_theat=='N') hPalette=m_hPalUnitUbn;
				}
			
		}*/

		auto shp = FindUnitShp(image, cur_theat, artSection);
		if (!shp)
			return;

		filename = shp->filename;
		hPalette = shp->palette;

		auto limited_to_theater = isTrue(artSection.GetValueByName("TerrainPalette")) ? shp->mixfile_theater : TheaterChar::None;

		if(filename=="tibtre01.tem" || filename=="tibtre02.tem" || filename=="tibtre03.tem" || filename=="veinhole.tem")
		{
			hPalette=m_hPalUnitTemp;
		}
				
			
		if(shp->mixfile>0)
		{			

			BOOL bSuccess=FSunPackLib::SetCurrentSHP(filename, shp->mixfile);
			if(
			!bSuccess
			)
			{
				filename=image+".sno";
				if(cur_theat=='T' || cur_theat=='U') hPalette=m_hPalIsoTemp;
				HMIXFILE hShpMix=FindFileInMix(filename);
				bSuccess=FSunPackLib::SetCurrentSHP(filename, hShpMix);
				
				if(!bSuccess)
				{
					return;
				}
			}
			
			if(bSuccess)
			{
		

				char ic[50];
				int i=0;
				itoa(i, ic, 10);

				// just fill in a stub entry - Final* will automatically retry loading once the graphic really must be loaded
				PICDATA p;
				p.pic=NULL;				
				p.x=0;
				p.y=0;
				p.wHeight=0;
				p.wWidth=0;
				p.wMaxWidth=0;
				p.wMaxHeight=0;
				p.bType=PICDATA_TYPE_SHP;
				p.bTerrain=limited_to_theater;

					
				pics[image+ic]=p;			
				
			
			}
			
		}
		
		}
		catch(...)
		{
			errstream << " exception " << endl;
			errstream.flush();
		}
	
		
	}

}

/*
Helper function that fetches the palette data from FsunPackLib
FSunPackLib doesn´t provide any special function to retrieve a color table entry,
so we have to build it ourself
Also builds color_conv
*/
void CLoading::FetchPalettes()
{
	// SetTSPaletteEntry(HTSPALETTE hPalette, BYTE bIndex, RGBTRIPLE* rgb, RGBTRIPLE* orig);
	// SetTSPaletteEntry can retrieve the current color table entry without modifying it!


	// iso palette
	HTSPALETTE hCur=0;
	if(Map->GetTheater()==THEATER0) hCur=m_hPalIsoTemp;
	if(Map->GetTheater()==THEATER1) hCur=m_hPalIsoSnow;
	if(Map->GetTheater()==THEATER2) hCur=m_hPalIsoUrb;
	if(Map->GetTheater()==THEATER3) hCur=m_hPalIsoUbn;
	if(Map->GetTheater()==THEATER4) hCur=m_hPalIsoLun;
	if(Map->GetTheater()==THEATER5) hCur=m_hPalIsoDes;

	int i;

	for(i=0;i<256;i++)
	{
		FSunPackLib::SetTSPaletteEntry(hCur, i, NULL /* don´t modify it!*/, &palIso[i] /*but retrieve it!*/);
	}


	// unit palette
	if(Map->GetTheater()==THEATER0) hCur=m_hPalUnitTemp;
	if(Map->GetTheater()==THEATER1) hCur=m_hPalUnitSnow;
	if(Map->GetTheater()==THEATER2) hCur=m_hPalUnitUrb;
	if(Map->GetTheater()==THEATER3) hCur=m_hPalUnitUbn;
	if(Map->GetTheater()==THEATER4) hCur=m_hPalUnitLun;
	if(Map->GetTheater()==THEATER5) hCur=m_hPalUnitDes;


	for(i=0;i<256;i++)
	{
		FSunPackLib::SetTSPaletteEntry(hCur, i, NULL /* don´t modify it!*/, &palUnit[i] /*but retrieve it!*/);
	}


	// theater palette
	if(Map->GetTheater()==THEATER0) hCur=m_hPalTemp;
	if(Map->GetTheater()==THEATER1) hCur=m_hPalSnow;
	if(Map->GetTheater()==THEATER2) hCur=m_hPalUrb;
	if(Map->GetTheater()==THEATER3) hCur=m_hPalUbn;
	if(Map->GetTheater()==THEATER4) hCur=m_hPalLun;
	if(Map->GetTheater()==THEATER5) hCur=m_hPalDes;



	for(i=0;i<256;i++)
	{
		FSunPackLib::SetTSPaletteEntry(hCur, i, NULL /* don´t modify it!*/, &palTheater[i] /*but retrieve it!*/);
	}


	// lib palette
	hCur=m_hPalLib;


	for(i=0;i<256;i++)
	{
		FSunPackLib::SetTSPaletteEntry(hCur, i, NULL /* don´t modify it!*/, &palLib[i] /*but retrieve it!*/);
	}

	CreateConvTable(palIso, iPalIso);
	CreateConvTable(palLib, iPalLib);
	CreateConvTable(palUnit, iPalUnit);
	CreateConvTable(palTheater, iPalTheater);

	CIsoView& v=*((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;

	DDPIXELFORMAT pf;
	memset(&pf, 0, sizeof(DDPIXELFORMAT));
	pf.dwSize=sizeof(DDPIXELFORMAT);

	v.lpds->GetPixelFormat(&pf);
	v.pf = pf;
	v.m_color_converter.reset(new FSunPackLib::ColorConverter(v.pf));

	FSunPackLib::ColorConverter conf(pf);

	const auto& rulesColors = rules.sections["Colors"];
	for(i=0;i< rulesColors.values.size();i++)
	{
		CString col=*rulesColors.GetValueName(i);
		COLORREF cref=v.GetColor("", col);		
		
		color_conv[col]=conf.GetColor(GetRValue(cref), GetGValue(cref), GetBValue(cref));
		colorref_conv[cref]=color_conv[col];
	}
}

void CLoading::CreateConvTable(RGBTRIPLE *pal, int *iPal)
{
	CIsoView& v=*((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;

	DDPIXELFORMAT pf;
	memset(&pf, 0, sizeof(DDPIXELFORMAT));
	pf.dwSize=sizeof(DDPIXELFORMAT);

	v.lpds->GetPixelFormat(&pf);

	FSunPackLib::ColorConverter conf(pf);

	int i;
	for(i=0;i<256;i++)
	{
		iPal[i]=conf.GetColor(pal[i].rgbtRed, pal[i].rgbtGreen, pal[i].rgbtBlue);
	}
}
