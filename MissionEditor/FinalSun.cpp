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

//
// This is the application code. Mainly shows the main dialog and initializes some application global stuff
//

#include "stdafx.h"
#include "FinalSun.h"
#include "FinalSunDlg.h"
#include "structs.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"
#include <ShlObj.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define APPID "FinalSun App"

extern char AppPath[];

extern ACTIONDATA AD;

/////////////////////////////////////////////////////////////////////////////
// CFinalSunApp

BEGIN_MESSAGE_MAP(CFinalSunApp, CWinApp)
	//{{AFX_MSG_MAP(CFinalSunApp)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFinalSunApp construction

void __cdecl term_func()
{
	if (MessageBox(0, "Fatal error. Exit FinalSun?", "Fatal error", MB_YESNO) == IDCANCEL) return;

	exit(-1);
}


CFinalSunApp::CFinalSunApp()
{
	m_cf = RegisterClipboardFormat("FINAL*FORMAT");

	if (!m_cf) MessageBox(0, "Failed to register clipboard format, clipboard functions not available", "", 0);

	tiledata_count = &t_tiledata_count;
	tiledata = &t_tiledata;

	// first: set up global variable AppPath
	// [02/16/2000] using GetModuleFileName() instead of GetCurrentDirectory(): always the correct path
	wchar_t AppPathUtf16[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, AppPathUtf16, MAX_PATH);
	strcpy_s(AppPath, utf16ToUtf8(AppPathUtf16).c_str());
	*(strrchr(AppPath, '\\') + 1) = 0;

	// Initialize AppData
	const std::wstring AppDataPathFolder = utf8ToUtf16(u8AppDataPath.substr(0, u8AppDataPath.size() - 1));
	auto create_dir_res = SHCreateDirectoryExW(NULL, AppDataPathFolder.c_str(), nullptr);
	if (ERROR_SUCCESS != create_dir_res && ERROR_ALREADY_EXISTS != create_dir_res)
	{
		wchar_t err_msg[1025] = { 0 };
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, create_dir_res, 0, err_msg, 1024, NULL);
		MessageBoxW(NULL, err_msg, (std::wstring(L"Failed to open ") + AppDataPathFolder).c_str(), 0);
		exit(1);
	}

	if ((GetFileAttributesW(AppDataPathFolder.c_str()) & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
	{
		MessageBoxW(NULL, (AppDataPathFolder + L" must be a directory, not a file").c_str(), (std::wstring(L"Failed to open ") + AppDataPathFolder).c_str(), 0);
		exit(1);
	}

	/*memset(t_tilepics, 0, sizeof(TILEPICDATA)*10000);
	memset(s_tilepics, 0, sizeof(TILEPICDATA)*10000);*/

	m_Options.LanguageName = "English";
	m_Options.bFlat = FALSE;
	m_Options.bEasy = FALSE;
	m_Options.bSupportMarbleMadness = FALSE;
	m_Options.bMarbleMadness = FALSE;

	auto log = u8AppDataPath;
#ifdef TS_MODE
	log += "finalsunlog.txt";
#else
	log += "finalalert2log.txt";
#endif
	m_u8LogFileName = log;
	errstream.open(m_u8LogFileName, ios_base::trunc);
	errstream << "\uFEFF"; // BOM

#ifdef TS_MODE
	errstream << "FinalSun log file" << std::endl << "----------------------" << std::endl << std::endl;
#else
	errstream << "FinalAlert 2 log file" << std::endl << "----------------------" << std::endl << std::endl;
#endif
	errstream << "CFinalSunApp::CFinalSunApp() called" << std::endl;

	errstream << "App Path: " << AppPath << std::endl;
	errstream << "AppData Path: " << u8AppDataPath << std::endl;
	errstream << "Locale: " << setlocale(LC_ALL, nullptr) << std::endl;
	errstream << "Windows ACP: " << GetACP() << std::endl;


	INITCOMMONCONTROLSEX ctr;
	ctr.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ctr.dwICC = ICC_STANDARD_CLASSES;
	if (!InitCommonControlsEx(&ctr))
	{
		errstream << "Error: Common controls could not be initialized" << std::endl;
		MessageBox(0, "Common controls could not be initialized.", "Error", 0);
	}

	errstream << std::endl << std::endl << std::endl;

	errstream.flush();

}


/////////////////////////////////////////////////////////////////////////////
// CFinalSunApp initialization

BOOL CFinalSunApp::InitInstance()
{
	m_hAccel = LoadAccelerators(this->m_hInstance, MAKEINTRESOURCE(IDR_MAIN));

#ifndef NOSURFACES
	if (GetDeviceCaps(GetDC(GetDesktopWindow()), BITSPIXEL) <= 8)
	{
		MessageBox(0, "You currently only have 8 bit color mode enabled. This is not recommended. You can continue, but this will cause a significant slowdown while loading graphics, and result in poor graphics quality", "Error", 0);
		//exit(0);
	}
#else
	if (GetDeviceCaps(GetDC(GetDesktopWindow()), BITSPIXEL) <= 8)
	{
		MessageBox(0, "You currently only have 8 bit color mode enabled. FinalSun/FinalAlert 2 will not work in 8 bit color mode. See readme.txt for further information!", "Error", 0);
		exit(0);
	}
#endif

	ParseCommandLine();

	// Load application data
	std::string datafile = AppPath;
#ifdef TS_MODE
	datafile += "\\FSData.ini";
#else
	datafile += "\\FAData.ini";
#endif

	g_data.LoadFile(datafile);

	// Load language data
	std::string languagefile = AppPath;
#ifndef RA2_MODE
	languagefile += "\\FSLanguage.ini";
#else
	languagefile += "\\FALanguage.ini";
#endif
	language.LoadFile(languagefile);

	if (language.sections.size() == 0)
	{
		MessageBox(0, "FALanguage.ini does not exist or is not valid (download corrupt?)", "", 0);
		exit(0);
	}

#ifndef RA2_MODE
	const std::string iniName = "FinalSun.ini";
	const std::string defaultIniName = "FinalSunDefaults.ini";
#else
	const std::string iniName = "FinalAlert.ini";
	const std::string defaultIniName = "FinalAlertDefaults.ini";
#endif

	// ok lets get some options
	CIniFile optini;
	std::string iniFile = u8AppDataPath + iniName;
	std::string templateIniFile = std::string(AppPath) + defaultIniName;

	bool copiedDefaultFile = false;
	if (!DoesFileExist(iniFile.c_str()))
	{
		if (CopyFileW(utf8ToUtf16(templateIniFile).c_str(), utf8ToUtf16(iniFile).c_str(), TRUE))
			copiedDefaultFile = true;
	}

	optini.LoadFile(iniFile);

#ifdef RA2_MODE
	CString game = "RA2";
	CString app = "FinalAlert";
#else
	CString game = "TS";
	CString app = "FinalSun";
#endif

	std::wstring key;
#ifdef RA2_MODE
	key = L"Software\\Westwood\\Red Alert 2";
#else
	key = L"Software\\Westwood\\Tiberian Sun";
#endif

	auto& opts = m_Options;

	HKEY hKey = 0;
	int res;
	res = RegOpenKeyExW(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_EXECUTE/*KEY_ALL_ACCESS*/, &hKey);
	if (res != ERROR_SUCCESS)
	{
		std::wstring s = L"Failed to access registry. Using manual setting. Error was:\n";
		wchar_t c[1024] = { 0 };
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, res, 0, c, 1023, NULL);
		MessageBoxW(0, (s + c).c_str(), L"Error", 0);
		opts.TSExe = optini.sections[game].values["Exe"];
	}
	else
	{
		// key opened
		wchar_t path[MAX_PATH + 1] = { 0 };
		DWORD pathsize = MAX_PATH;
		DWORD type = REG_SZ;
		if ((res = RegQueryValueExW(hKey, L"InstallPath", 0, &type, (unsigned char*)path, &pathsize)) != ERROR_SUCCESS)
		{
			std::wstring s = L"Failed to access registry. Using manual setting. Error was:\n";
			wchar_t c[1024] = { 0 };
			FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, res, 0, c, 1023, NULL);
			MessageBoxW(0, (s + c).c_str(), L"Error", 0);
			opts.TSExe = optini.sections[game].values["Exe"];
		}
		else
			opts.TSExe = path;
	}


	if (copiedDefaultFile ||
		optini.sections.size() == 0 ||
		opts.TSExe.GetLength() == 0 ||
		optini.sections[app].values["Language"].GetLength() == 0 ||
		optini.sections[app].values.find("FileSearchLikeGame") == optini.sections[app].values.end() ||
		optini.sections[app].values.find("PreferLocalTheaterFiles") == optini.sections[app].values.end())
	{
		opts.bSearchLikeTS = TRUE;

		bOptionsStartup = TRUE;
		ShowOptionsDialog();
		bOptionsStartup = FALSE;

	}
	else
	{
		opts.LanguageName = optini.sections[app].values["Language"];
		if (optini.sections[app].values["FileSearchLikeGame"] != "no")
			opts.bSearchLikeTS = TRUE;
		else opts.bSearchLikeTS = FALSE;
	}
	opts.bPreferLocalTheaterFiles = optini.sections[app].values.emplace("PreferLocalTheaterFiles", opts.bPreferLocalTheaterFiles ? "1" : "0").first->second == "1";

	opts.bDoNotLoadAircraftGraphics = optini.sections["Graphics"].values["NoAircraftGraphics"] == "1";
	opts.bDoNotLoadVehicleGraphics = optini.sections["Graphics"].values["NoVehicleGraphics"] == "1";
	opts.bDoNotLoadBuildingGraphics = optini.sections["Graphics"].values["NoBuildingGraphics"] == "1";
	opts.bDoNotLoadInfantryGraphics = optini.sections["Graphics"].values["NoInfantryGraphics"] == "1";
	opts.bDoNotLoadTreeGraphics = optini.sections["Graphics"].values["NoTreeGraphics"] == "1";
	opts.bDoNotLoadSnowGraphics = optini.sections["Graphics"].values["NoSnowGraphics"] == "1";
	opts.bDoNotLoadTemperateGraphics = optini.sections["Graphics"].values["NoTemperateGraphics"] == "1";
	opts.bDoNotLoadBMPs = optini.sections["Graphics"].values["NoBMPs"] == "1";
	opts.bDoNotLoadOverlayGraphics = optini.sections["Graphics"].values["NoOverlayGraphics"] == "1";
	opts.bVSync = optini.sections["Graphics"].values.emplace("VSync", opts.bVSync ? "1" : "0").first->second == "1";

	opts.bDisableAutoShore = optini.sections["UserInterface"].values["DisableAutoShore"] == "1";
	opts.bDisableAutoLat = optini.sections["UserInterface"].values["DisableAutoLat"] == "1";
	opts.bNoSounds = optini.sections["UserInterface"].values["Sounds"] != "1";
	opts.bDisableSlopeCorrection = optini.sections["UserInterface"].values["DisableSlopeCorrection"] == "1";
	opts.fLoadScreenDelayInSeconds = static_cast<float>(atof(optini.sections["UserInterface"].values.emplace("LoadScreenDelay", std::to_string(opts.fLoadScreenDelayInSeconds).c_str()).first->second));
	opts.bShowStats = optini.sections["UserInterface"].values.emplace("ShowStats", opts.bShowStats ? "1" : "0").first->second == "1";
	opts.bHighResUI = optini.sections["UserInterface"].values.emplace("HighRes", opts.bHighResUI ? "1" : "0").first->second == "1";	
	opts.useDefaultMouseCursor = optini.sections["UserInterface"].values.emplace("UseDefaultMouseCursor", opts.useDefaultMouseCursor ? "1" : "0").first->second == "1";

	opts.fMiniMapScale = static_cast<float>(atof(optini.sections["MiniMap"].values.emplace("Scale", std::to_string(opts.fMiniMapScale).c_str()).first->second));

	auto defaultViewSteps = CString(Join(",", opts.viewScaleSteps | std::views::transform([](auto v) {return std::to_string(v); })).c_str());
	auto viewScaleStepsRange = SplitParams(optini.sections["UserInterface"].values.emplace("ViewScaleSteps", defaultViewSteps).first->second) | std::views::transform([](auto v) { return static_cast<float>(std::atof(v)); });
	opts.viewScaleSteps.assign(viewScaleStepsRange.begin(), viewScaleStepsRange.end());
	opts.viewScaleUseSteps = optini.sections["UserInterface"].values.emplace("ViewScaleUseSteps", opts.viewScaleUseSteps ? "1" : "0").first->second == "1";
	opts.viewScaleSpeed = static_cast<float>(atof(optini.sections["UserInterface"].values.emplace("ViewScaleSpeed", std::to_string(opts.viewScaleSpeed).c_str()).first->second));

	// MW 07/19/01
	opts.bShowCells = optini.sections["UserInterface"].values["ShowBuildingCells"] == "1";

	optini.SaveFile(iniFile);

	// MW 07/20/01: Load file list
	int i;
	for (i = 0;i < 4;i++)
	{
		char c[50];
		itoa(i, c, 10);
		opts.prev_maps[i] = optini.sections["Files"].values[c];
	}

	if (opts.bDoNotLoadTemperateGraphics && opts.bDoNotLoadSnowGraphics)
	{
		MessageBox(0, "You have turned off loading of both snow and temperate terrain in 'FinalAlert.ini'. At least one of these must be loaded. The application will now quit.", "Error", 0);
		exit(-982);
	}

	int EasyView;
	if (optini.sections["UserInterface"].FindName("EasyView") < 0)
	{
		MessageBox(0, GetLanguageStringACP("ExplainEasyView"), GetLanguageStringACP("ExplainEasyViewCap"), 0);
		EasyView = 1;

		optini.LoadFile(iniFile);
		optini.sections["UserInterface"].values["EasyView"] = "1";
		optini.SaveFile(iniFile);
	}
	else
	{
		EasyView = atoi(optini.sections["UserInterface"].values["EasyView"]);
	}
	if (EasyView != 0) theApp.m_Options.bEasy = TRUE;




	CString cTSPath = theApp.m_Options.TSExe;
	auto lastSlash = cTSPath.ReverseFind('\\');
	if (lastSlash >= 0)
		cTSPath.SetAt(lastSlash + 1, 0);
	strcpy(TSPath, cTSPath);

	// MW 01/23/2013: changed the global CMapData Map to a global CMapData* to get rid of static initialization/shutdown problems
	{
		std::unique_ptr<CMapData> mapData(new CMapData());
		Map = mapData.get();

		CLoading loading(NULL);
		m_loading = &loading;

		CFinalSunDlg dlg;
		m_pMainWnd = &dlg;

		dlg.DoModal();
	}

	// Map and dialog closed, do further work if required	

	return FALSE;
}

void CFinalSunApp::ParseCommandLine()
{
#ifdef 0 // Removed as it can conflict with Steam game arguments! -LF 23.02.2024
	char data[MAX_PATH + 30];

	strcpy(data, theApp.m_lpCmdLine);
	if (strlen(data) == 0)
	{
		strcpy(currentMapFile, "");
		return;
	}

	strcpy(currentMapFile, data);
#endif
}

void CFinalSunApp::ShowTipAtStartup(void)
{
	CTipDlg dlg;
	if (dlg.m_bStartup)
		dlg.DoModal();


}

void CFinalSunApp::ShowTipOfTheDay(void)
{
	CTipDlg dlg;
	dlg.DoModal();

}

int CFinalSunApp::Run()
{
	return CWinApp::Run();

}

BOOL CFinalSunApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	if (lpMsg->message == WM_KEYDOWN)
	{
		/*if(lpMsg->wParam==VK_F1)
		{
			if(ShellExecute(0, NULL, (CString)AppPath+"/help/index.htm", NULL, NULL, SW_NORMAL)==0)
			{
				MessageBox(0,(CString)"Could not open manual! Try opening "+(CString)AppPath+(CString)"/help/index.htm manually","Error",0);
			}
			return TRUE;
		}*/

		// check AD mode

		if (lpMsg->wParam == VK_PRIOR || lpMsg->wParam == VK_NEXT)
		{
			if (lpMsg->wParam == VK_PRIOR)
			{
				AD.z_data += 1;
			}
			if (lpMsg->wParam == VK_NEXT)
			{
				AD.z_data -= 1;
			}

			if (AD.mode == ACTIONMODE_SETTILE || AD.mode == ACTIONMODE_PASTE)
			{
				CIsoView* v = ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;
				CTerrainDlg& tdlg = ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_browser->m_bar;
				WORD isox, isoy;
				RECT r;
				POINT p;
				v->GetWindowRect(&r);
				GetCursorPos(&p);
				isox = p.x - r.left;
				isoy = p.y - r.top;

				if (isox > r.right - r.left || isoy > r.bottom - r.top)
				{

				}
				else
				{
					LPARAM lParam;
					memcpy(&lParam, &isox, 2);
					memcpy((BYTE*)&lParam + 2, &isoy, 2);
					v->SendMessage(WM_MOUSEMOVE, 0, lParam);
				}

			}
		}





		if (AD.mode == ACTIONMODE_SETTILE)
		{
			CIsoView* v = ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview;
			CTerrainDlg& tdlg = ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_browser->m_bar;
			WORD isox, isoy;
			RECT r;
			POINT p;
			v->GetWindowRect(&r);
			GetCursorPos(&p);
			isox = p.x - r.left;
			isoy = p.y - r.top;
			if (isox > r.right - r.left || isoy > r.bottom - r.top)
			{

			}
			else
			{
				LPARAM lParam;
				memcpy(&lParam, &isox, 2);
				memcpy((BYTE*)&lParam + 2, &isoy, 2);

				int searchedTileSet;
				int z;
				switch (lpMsg->wParam)
				{

				case VK_LEFT:
					if (AD.type > 0 && (*tiledata)[AD.type].wTileSet == (*tiledata)[AD.type - 1].wTileSet) { AD.type -= 1; AD.z_data = 0; }
					v->SendMessage(WM_MOUSEMOVE, 0, lParam);
					return 0;
					break;
				case VK_RIGHT:
					if (AD.type < (*tiledata_count) - 1 && (*tiledata)[AD.type].wTileSet == (*tiledata)[AD.type + 1].wTileSet) { AD.type += 1; AD.z_data = 0; }
					v->SendMessage(WM_MOUSEMOVE, 0, lParam);
					return 0;
					break;
				case VK_UP:
					searchedTileSet = (*tiledata)[AD.type].wTileSet - 1;
				case VK_DOWN:
					if (lpMsg->wParam == VK_DOWN)
						searchedTileSet = (*tiledata)[AD.type].wTileSet + 1;

					if (searchedTileSet >= 0 && searchedTileSet < tilesets_start.size())
					{
						int i;
						int s = 1;
						int start = 0;
						int count = tilesets_start.size() - searchedTileSet;

						if (searchedTileSet < (*tiledata)[AD.type].wTileSet)
						{
							s = -1;
							start = 0;
							count = searchedTileSet + 1;
						}




						for (i = start;i < count;i += 1)
						{
							if (searchedTileSet + i * s < 0 || searchedTileSet + i * s >= tilesets_start.size()) continue;

							int pos = tilesets_start[searchedTileSet + i * s];




							int e;
							CComboBox* cb = ((CComboBox*)tdlg.GetDlgItem(IDC_TILESET));
							BOOL bFound = FALSE;
							for (e = 0;e < cb->GetCount();e++)
							{
								if (cb->GetItemData(e) == searchedTileSet + i * s)
								{
									cb->SendMessage(CB_SETCURSEL, e, 0);
									bFound = TRUE;
									break;
								}
							}

							if (bFound)
							{
								AD.type = tilesets_start[searchedTileSet + i * s];
								AD.data = 0;
								AD.data2 = 0;
								AD.data3 = 0;
								AD.data_s = "";
								v->SendMessage(WM_MOUSEMOVE, 0, lParam);

								return 0;
							}


						}


						return 0;
						break;


					}

					return 0;
					break;


				}
			}

		}

	}

	return CWinApp::ProcessMessageFilter(code, lpMsg);
}

LRESULT CFinalSunApp::ProcessWndProcException(CException* e, const MSG* pMsg)
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	e->ReportError(MB_YESNO);

	return TRUE;
	//return CWinApp::ProcessWndProcException(e, pMsg);
}

BOOL CFinalSunApp::PreTranslateMessage(MSG* pMsg)
{

	if (!TranslateAccelerator(
		*this->m_pMainWnd,      // handle to receiving window 
		m_hAccel,        // handle to active accelerator table 
		pMsg))         // message data

		return CWinApp::PreTranslateMessage(pMsg);

	return FALSE;
}



BOOL CFinalSunApp::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{

	return CWinApp::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CFinalSunApp::OnIdle(LONG lCount)
{
	((CFinalSunDlg*)m_pMainWnd)->m_view.m_isoview->UpdateWindow();
	//MessageBox(0,"Idled","",0);
	return CWinApp::OnIdle(lCount);
}
