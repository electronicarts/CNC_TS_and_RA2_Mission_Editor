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

// standard functions

#include "stdafx.h"
#include "FinalSunDlg.h"
#include "TSoptions.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"
#include "mmsystem.h"


#define DBG
#undef DBG

#define DBG2
#undef DBG2

bool isValidUtf8(const char* utf8)
{
	// wstring_convert and codecvt_utf8_utf16 are deprecated in C++17, fallback to Win32
	auto utf8Count = strlen(utf8);
	if (utf8Count == 0)
		return true;

	// unterminatedCountWChars will be the count of WChars NOT including the terminating zero (due to passing in utf8.size() instead of -1)
	auto unterminatedCountWChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, utf8Count, nullptr, 0);
	return unterminatedCountWChars > 0;
}

std::wstring utf8ToUtf16(const std::string& utf8)
{
	// wstring_convert and codecvt_utf8_utf16 are deprecated in C++17, fallback to Win32
	if (utf8.size() == 0)
		// MultiByteToWideChar does not support passing in cbMultiByte == 0
		return L"";

	// unterminatedCountWChars will be the count of WChars NOT including the terminating zero (due to passing in utf8.size() instead of -1)
	auto utf8Count = utf8.size();
	auto unterminatedCountWChars = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8.data(), utf8Count, nullptr, 0);
	if (unterminatedCountWChars == 0)
	{
		throw std::runtime_error("UTF8 -> UTF16 conversion failed");
	}

	std::wstring utf16;
	utf16.resize(unterminatedCountWChars);
	if (MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8.data(), utf8Count, utf16.data(), unterminatedCountWChars) == 0)
	{
		throw std::runtime_error("UTF8 -> UTF16 conversion failed");
	}
	return utf16;
}

std::wstring utf8ToUtf16(const char* utf8)
{
	// wstring_convert and codecvt_utf8_utf16 are deprecated in C++17, fallback to Win32
	auto utf8Count = strlen(utf8);
	if (utf8Count == 0)
		// MultiByteToWideChar does not support passing in cbMultiByte == 0
		return L"";

	// unterminatedCountWChars will be the count of WChars NOT including the terminating zero (due to passing in utf8.size() instead of -1)
	auto unterminatedCountWChars = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8, utf8Count, nullptr, 0);
	if (unterminatedCountWChars == 0)
	{
		throw std::runtime_error("UTF8 -> UTF16 conversion failed");
	}

	std::wstring utf16;
	utf16.resize(unterminatedCountWChars);
	if (MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, utf8, utf8Count, utf16.data(), unterminatedCountWChars) == 0)
	{
		throw std::runtime_error("UTF8 -> UTF16 conversion failed");
	}
	return utf16;
}

std::string utf16ToCP(const std::wstring& utf16, int CP)
{
	// wstring_convert and codecvt_utf8_utf16 are deprecated in C++17, fallback to Win32
	if (utf16.size() == 0)
		// WideCharToMultiByte does not support passing in cbMultiByte == 0
		return "";

	// unterminatedCountWChars will be the count of WChars NOT including the terminating zero (due to passing in utf8.size() instead of -1)
	auto utf16Count = utf16.size();
	auto unterminatedCountChars = WideCharToMultiByte(CP, CP == CP_UTF8 ? WC_ERR_INVALID_CHARS : 0, utf16.data(), utf16Count, nullptr, 0, nullptr, nullptr);
	if (unterminatedCountChars == 0)
	{
		throw std::runtime_error(CP == CP_UTF8 ? "UTF16 -> UTF8 conversion failed" : "UTF16 -> MultiByte conversion failed");
	}

	std::string cps;
	cps.resize(unterminatedCountChars);
	if (WideCharToMultiByte(CP, CP == CP_UTF8 ? WC_ERR_INVALID_CHARS : 0, utf16.data(), utf16Count, cps.data(), unterminatedCountChars, nullptr, nullptr) == 0)
	{
		throw std::runtime_error(CP == CP_UTF8 ? "UTF16 -> UTF8 conversion failed" : "UTF16 -> MultiByte conversion failed");
	}
	return cps;
}

std::string utf16ToUtf8(const std::wstring& utf16)
{
	return utf16ToCP(utf16, CP_UTF8);
}

std::string utf16ToACP(const std::wstring& utf16)
{
	return utf16ToCP(utf16, CP_ACP);
}

// strcpy for overlapping strings
char *strcpy_safe( char *strDestination, const char *strSource )
{
	/*char* buffer=new(char[strlen(strSource)+1]);
	strcpy(buffer, strSource);

	strcpy(strDestination, buffer);*/

	int len=strlen(strSource)+1;
	memmove(strDestination, strSource, len);

	return strDestination;
}

CString TranslateHouse(CString original, BOOL bToUI)
{
#ifdef RA2_MODE
	if(bToUI)
	{
		// CCStrings[*rules.sections[HOUSES].GetValue(i)].wString
		int i;
		for(i=0;i<rules.sections[HOUSES].values.size();i++)
		{
			original.Replace(*rules.sections[HOUSES].GetValue(i), CCStrings[*rules.sections[HOUSES].GetValue(i)].cString);
		}
	}
	else
	{

		int i;
		for(i=0;i<rules.sections[HOUSES].values.size();i++)
		{
			original.Replace(CCStrings[*rules.sections[HOUSES].GetValue(i)].cString, *rules.sections[HOUSES].GetValue(i));
		}
	}
#endif
	return original;
}


bool deleteFile(const std::string& u8FilePath)
{
	return DeleteFileW(utf8ToUtf16(u8FilePath).c_str()) ? true : false;
}

// set the status bar text in the main dialog
void SetMainStatusBar(const char* text)
{
	CFinalSunDlg* dlg=(CFinalSunDlg*)theApp.GetMainWnd();
	dlg->SetText(text);
}

// set the status bar text in the main dialog to ready
void SetMainStatusBarReady()
{
	CFinalSunDlg* dlg=(CFinalSunDlg*)theApp.GetMainWnd();
	dlg->SetReady();;
}

// Should not be required anymore
void RepairTrigger(CString& triggerdata)
{
	if(GetParam(triggerdata, 3).GetLength()==0) triggerdata=SetParam(triggerdata, 3, "0");
	if(GetParam(triggerdata, 4).GetLength()==0) triggerdata=SetParam(triggerdata, 4, "1");
	if(GetParam(triggerdata, 5).GetLength()==0) triggerdata=SetParam(triggerdata, 5, "1");
	if(GetParam(triggerdata, 6).GetLength()==0) triggerdata=SetParam(triggerdata, 6, "1");
	if(GetParam(triggerdata, 7).GetLength()==0) {
		triggerdata=SetParam(triggerdata, 7, "0");
	}
}

// make some UI noise
void Sound(int ID)
{
	if(theApp.m_Options.bNoSounds) return;

	if(ID==SOUND_NONE) return;
	
	LPCSTR lpSound=NULL;

	if(ID==SOUND_POSITIVE)
	{
		lpSound=MAKEINTRESOURCE(IDR_WAVE1);
	}
	else if(ID==SOUND_NEGATIVE)
		lpSound=MAKEINTRESOURCE(IDR_WAVE2);
	else if(ID==SOUND_LAYDOWNTILE)
		lpSound=MAKEINTRESOURCE(IDR_WAVE3);


	if(lpSound)
	{
		PlaySound(lpSound, GetModuleHandle(NULL), SND_ASYNC | SND_RESOURCE);
	}
}

void HandleParamList(CComboBox &cb, int type)
{
	CString oldText;
	cb.GetWindowText(oldText);
	
	switch(type)
	{
	case PARAMTYPE_NOTHING:
		{			
			while(cb.DeleteString(0)!=CB_ERR);
			cb.SetWindowText(oldText);

					
			//cb.AddString("0");
		}
		break;
	case PARAMTYPE_HOUSES:
		ListHouses(cb, TRUE, TRUE, TRUE);
		break;
	case PARAMTYPE_WAYPOINTS:
		ListWaypoints(cb);
		break;
	case PARAMTYPE_TEAMTYPES:
		ListTeamTypes(cb, FALSE);
		break;
	case PARAMTYPE_UNITTYPES:
		ListUnits(cb);
		break;
	case PARAMTYPE_INFANTRYTYPES:
		ListInfantry(cb);
		break;
	case PARAMTYPE_AIRCRAFTTYPES:
		ListAircraft(cb);
		break;
	case PARAMTYPE_BUILDINGTYPES:
		ListBuildings(cb);
		break;
	case PARAMTYPE_VIDEOS:
		ListMovies(cb, FALSE, TRUE);
		break;
	case PARAMTYPE_TUTORIALTEXTS:
		ListTutorial(cb);
		break;
	case PARAMTYPE_TRIGGERS:
		ListTriggers(cb);
		break;
	case PARAMTYPE_YESNO:
		ListYesNo(cb);
		break;
	case PARAMTYPE_SOUNDS:
		ListSounds(cb);
		break;
	case PARAMTYPE_THEMES:
		ListThemes(cb);
		break;
	case PARAMTYPE_SPEECHES:
		ListSpeeches(cb);
		break;
	case PARAMTYPE_SPECIALWEAPONS:
		ListSpecialWeapons(cb);
		break;
	case PARAMTYPE_ANIMATIONS:
		ListAnimations(cb);
		break;
	case PARAMTYPE_PARTICLES:
		ListParticles(cb);
		break;
	case PARAMTYPE_CRATETYPES:
		ListCrateTypes(cb);
		break;
	case PARAMTYPE_SPEECHBUBBLETYPES:
		ListSpeechBubbleTypes(cb);
		break;
	case PARAMTYPE_GLOBALS:
		ListGlobals(cb);
		break;
	case PARAMTYPE_RULESGLOBALS:
		ListRulesGlobals(cb);
		break;
	case PARAMTYPE_BUILDINGTYPESINI:
		ListBuildings(cb, TRUE);
		break;
	case PARAMTYPE_TECHTYPES:
		ListTechtypes(cb);
		break;
	}
}

void ShowOptionsDialog()
{
	// show the options dialog, and save the options.

#ifdef RA2_MODE
	CString game = "RA2";
	CString app = "FinalAlert";
#else
	CString game = "TS";
	CString app = "FinalSun";
#endif

	std::string iniFile="";	
	CIniFile optini;
	iniFile=u8AppDataPath;
#ifndef RA2_MODE
	iniFile+="\\FinalSun.ini";
#else
	iniFile+="\\FinalAlert.ini";
#endif
	optini.LoadFile(iniFile);
	CTSOptions opt;
	opt.m_TSEXE=theApp.m_Options.TSExe;
	if(opt.DoModal()==IDCANCEL) return;
	theApp.m_Options.TSExe=opt.m_TSEXE;
	optini.sections[game].values["Exe"]=theApp.m_Options.TSExe;
	optini.sections[app].values["Language"]=opt.m_LanguageName;

	BOOL bOldSearch=theApp.m_Options.bSearchLikeTS;
	if(!(opt.m_LikeTS==1)) {
		optini.sections[app].values["FileSearchLikeGame"]="yes";
		theApp.m_Options.bSearchLikeTS=TRUE;
	}
	else {
		theApp.m_Options.bSearchLikeTS=FALSE;
		optini.sections[app].values["FileSearchLikeGame"]="no";
	}

	auto bOldPreferLocalTheaterFiles = theApp.m_Options.bPreferLocalTheaterFiles;
	theApp.m_Options.bPreferLocalTheaterFiles = opt.m_PreferLocalTheaterFiles ? true : false;
	optini.sections[app].values["PreferLocalTheaterFiles"] = theApp.m_Options.bPreferLocalTheaterFiles ? "1" : "0";


	if (
		(
			(bOldPreferLocalTheaterFiles != theApp.m_Options.bPreferLocalTheaterFiles) ||
			(bOldSearch != theApp.m_Options.bSearchLikeTS)
		) && bOptionsStartup == FALSE)
		MessageBox(0, GetLanguageStringACP("RestartNeeded"), "Restart", 0);
	

	CString oldLang=theApp.m_Options.LanguageName;
	theApp.m_Options.LanguageName=opt.m_LanguageName;
	if(oldLang!=theApp.m_Options.LanguageName && theApp.m_pMainWnd!=NULL && theApp.m_pMainWnd->m_hWnd!=NULL)
		((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateStrings();

	optini.SaveFile(iniFile);
}


BOOL DoesFileExist(LPCSTR szFile)
{
	std::wstring file = utf8ToUtf16(szFile);
	HANDLE hFound=CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, 0, NULL);
	
	if (hFound != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFound);
		return TRUE;
	}
	return FALSE;
}

CString ToACP(const CString& utf8)
{
	// convert to process codepage (should be UTF8 on newer systems)
	auto cp = GetACP();
	return (cp == CP_UTF8) ? utf8 : CString(utf16ToACP(utf8ToUtf16(utf8)).c_str());
}

// change all %n (where n is an int) in an string, to another specified string
CString TranslateStringVariables(int n, const char* originaltext, const char* inserttext)
{
	char c[50];
	itoa(n,c,10);

	char seekedstring[50];
	seekedstring[0]='%';
	seekedstring[1]=0;
	strcat(seekedstring, c);
	
	CString orig=originaltext;
	if(orig.Find(seekedstring)<0) return orig;

	orig.Replace(seekedstring, inserttext);
	
	return orig;
}

// retrieve the string name in the correct language (name is an ID).
CString GetLanguageStringACP(CString name)
{
#ifdef RA2_MODE
	CString sec2=theApp.m_Options.LanguageName+"-StringsRA2";
	if(language.sections[sec2].values.end()==language.sections[sec2].values.find(name))
	{
		if(language.sections["English-StringsRA2"].FindName(name)>=0)
		{
			CString s=language.sections["English-StringsRA2"].values[name];
			return ToACP(s);
		}
	}
	else
		return ToACP(language.sections[sec2].values[name]);
#endif


	if(language.sections[theApp.m_Options.LanguageName+"-Strings"].values.find(name)==language.sections[theApp.m_Options.LanguageName+"-Strings"].values.end())
	{
		CString s=language.sections["English-Strings"].values[name];	
#ifndef RA2_MODE
		s=TranslateStringVariables(9, s, "FinalSun");
#else
#ifdef YR_MODE
		s=TranslateStringVariables(9, s, "FinalAlert 2: Yuri's Revenge");
#else
		s=TranslateStringVariables(9, s, "FinalAlert 2");
#endif
#endif
		return ToACP(s);
	}

	CString s;
	s= language.sections[theApp.m_Options.LanguageName+"-Strings"].values[name];

#ifndef RA2_MODE
	if(s.Find("%9")>=0) s=TranslateStringVariables(9,s,"FinalSun");
#else
#ifdef YR_MODE
	if(s.Find("%9")>=0) s=TranslateStringVariables(9,s,"FinalAlert 2: Yuri's Revenge");
#else
	if(s.Find("%9")>=0) s=TranslateStringVariables(9,s,"FinalAlert 2");
#endif
#endif

	

	return ToACP(s);
}

CString TranslateStringACP(WCHAR* u16EnglishString)
{
	return TranslateStringACP(utf16ToUtf8(u16EnglishString).c_str());
}

// tranlate a string/word by using the table from english to the current language
CString TranslateStringACP(CString u8EnglishString)
{
	if (!isValidUtf8(u8EnglishString))
	{
		errstream << "TranslateStringACP(\"" << u8EnglishString << "\") called with an invalid UTF-8 string" << std::endl;
		return u8EnglishString;
	}

#ifdef RA2_MODE
	CString sec2=theApp.m_Options.LanguageName+"-TranslationsRA2";
	if(language.sections[sec2].values.end()==language.sections[sec2].values.find(u8EnglishString))
	{
		if(language.sections["English-TranslationsRA2"].FindName(u8EnglishString)>=0)
		{
			CString s=language.sections["English-TranslationsRA2"].values[u8EnglishString];
			return ToACP(s);
		}
	}
	else
		return ToACP(language.sections[sec2].values[u8EnglishString]);
#endif
	
	
	CString sec=theApp.m_Options.LanguageName+"-Translations";
	
	// check if the string can be translated
	if(language.sections[sec].values.end()==language.sections[sec].values.find(u8EnglishString))
	{
		CString seceng;
		seceng="English-Translations";
		if(language.sections[seceng].FindName(u8EnglishString)>=0)
		{
			CString s=language.sections[seceng].values[u8EnglishString];
#ifndef RA2_MODE
			s=TranslateStringVariables(9, s, "FinalSun");
#else
#ifdef YR_MODE
			s=TranslateStringVariables(9, s, "FinalAlert 2: Yuri's Revenge");
#else			
			s=TranslateStringVariables(9, s, "FinalAlert 2");
#endif
#endif
			return ToACP(s);
		}
#ifndef RA2_MODE
		return ToACP(TranslateStringVariables(9,u8EnglishString,"FinalSun"));
#else
#ifdef YR_MODE
		return ToACP(TranslateStringVariables(9,u8EnglishString,"FinalAlert 2: Yuri's Revenge"));
#else
		return ToACP(TranslateStringVariables(9,u8EnglishString,"FinalAlert 2"));
#endif
#endif
	}

	CString s=language.sections[sec].values[u8EnglishString];
#ifndef RA2_MODE
	s=TranslateStringVariables(9,s,"FinalSun");
#else
#ifdef YR_MODE
	s=TranslateStringVariables(9,s,"FinalAlert 2: Yuri's Revenge");
#else
	s=TranslateStringVariables(9,s,"FinalAlert 2");
#endif
#endif

	return ToACP(s);
}



void TruncSpace(string& str)
{
	CString cstr=str.data();
	TruncSpace(cstr);
	str=cstr;
}
void TruncSpace(CString& str)
{
	str.TrimLeft();
	str.TrimRight();
	if(str.Find(" ")>=0) str.Delete(str.Find(" "), str.GetLength()-str.Find(" "));
}

CString GetText(CWnd* wnd){
	CString str;
	wnd->GetWindowText(str);
	return str;
}

CString GetText(CSliderCtrl* wnd)
{
	int v=wnd->GetPos();
	char c[150];
	itoa(v,c,10);
	return(c);
}

CString GetText(CComboBox* wnd){

	CString str;
	if(wnd->GetCurSel()!=-1)
	{
		wnd->GetLBText(wnd->GetCurSel(), str);
		return str;
	}
	
	wnd->GetWindowText(str);

	return(str);
}



void Info(const char* data, string& house, string& type, int& strength, int& x, int& y, string & other)
{
	other="";
	house=GetParam(data, 0);	
	type=GetParam(data, 1);
	strength=atoi(GetParam(data, 2));
	y=atoi(GetParam(data, 3));
	x=atoi(GetParam(data, 4));

	CString tmp;
	BOOL takeABreak=FALSE;
	int i=1;
	do{
		
		tmp=GetParam(data, 4+i);
		//MessageBox(0,tmp.data(),"",0);
		if(tmp!="")
		{
			other+=",";
			other+=tmp;
		}
		else
		{
			takeABreak=TRUE;
			break;
		}
		i++;
	}while(takeABreak==FALSE);
};

void UnitInfo(const char* data, string& house, string& type, int& strength, int& x, int& y, int& direction, string& action, string& actiontrigger, int & u1, int & u2, int& u3, int& u4, int& u5, int& u6)
{
	house=GetParam(data, 0);	
	type=GetParam(data, 1);
	strength=atoi(GetParam(data, 2));
	y=atoi(GetParam(data, 3));
	x=atoi(GetParam(data, 4));
	direction=atoi(GetParam(data, 5));
	action=GetParam(data, 6);
	actiontrigger=GetParam(data, 7);
	u1=atoi(GetParam(data, 8));
	u2=atoi(GetParam(data, 9));
	u3=atoi(GetParam(data, 10));
	u4=atoi(GetParam(data, 11));
	u5=atoi(GetParam(data, 12));
	u6=atoi(GetParam(data, 13));
}

void AirInfo(const char* data, string& house, string& type, int& strength, int& x, int& y, int& direction, string& action, string& actiontrigger, int & u1, int & u2, int& u3, int& u4)
{
	house=GetParam(data, 0);	
	type=GetParam(data, 1);
	strength=atoi(GetParam(data, 2));
	y=atoi(GetParam(data, 3));
	x=atoi(GetParam(data, 4));
	direction=atoi(GetParam(data, 5));
	action=GetParam(data, 6);
	actiontrigger=GetParam(data, 7);
	u1=atoi(GetParam(data, 8));
	u2=atoi(GetParam(data, 9));
	u3=atoi(GetParam(data, 10));
	u4=atoi(GetParam(data, 11));
}



void InfanteryInfo(const char* data, string& house, string& type, int& strength, int& x, int& y, int& pos, string& action, int& direction, string& actiontrigger, int & u1, int & u2, int& u3, int& u4, int& u5)
{
	house=GetParam(data, 0);	
	type=GetParam(data, 1);
	strength=atoi(GetParam(data, 2));
	y=atoi(GetParam(data, 3));
	x=atoi(GetParam(data, 4));
	pos=atoi(GetParam(data, 5));
	action=GetParam(data, 6);
	direction=atoi(GetParam(data, 7));
	actiontrigger=GetParam(data, 8);
	u1=atoi(GetParam(data, 9));
	u2=atoi(GetParam(data, 10));
	u3=atoi(GetParam(data, 11));
	u4=atoi(GetParam(data, 12));
	u5=atoi(GetParam(data, 12));	
}

void StructureInfo(const char* data, string& house, string& type, int& strength, int& x, int& y, int& direction, string& action, int & u1, int & u2, int& energy, int& upgrades, int& u5, string& upgrade1, string& upgrade2, string& upgrade3, int& u9, int& u10)
{
	house=GetParam(data, 0);	
	type=GetParam(data, 1);
	strength=atoi(GetParam(data, 2));
	y=atoi(GetParam(data, 3));
	x=atoi(GetParam(data, 4));
	direction=atoi(GetParam(data, 5));
	action=GetParam(data, 6);
	u1=atoi(GetParam(data, 7));
	u2=atoi(GetParam(data, 8));
	energy=atoi(GetParam(data, 9));
	upgrades=atoi(GetParam(data, 10));
	u5=atoi(GetParam(data, 11));
	upgrade1=GetParam(data, 12);
	upgrade2=GetParam(data, 13);
	upgrade3=GetParam(data, 14);
	u9=atoi(GetParam(data, 15));
	u10=atoi(GetParam(data, 16));
}

void PosToXY(const char* pos, int* X, int* Y)
{
  int Posleng;
  //int XX, YY;
  char Pos[100];
  strcpy(Pos, pos);
  char XS[10], YS[10];
  Posleng = strlen(Pos);
  strcpy(YS, Pos+Posleng-3);
  Pos[Posleng-3]=0;
  strcpy(XS, Pos);

  *X = atoi(XS);
  *Y = atoi(YS);

}

bool HSVToRGB(const float h, const float s, const float v, float& r, float& g, float& b)
{
	if (h < 0.0 || h >= 360.0 || s < 0.0 || s > 1.0 || v < 0.0 || v > 1.0)
		return false;
	const int h_ = floor(h / 60.0);
	const float c = s * v;
	const float x = c * (1 - fabs(fmod(h / 60.0, 2.0) - 1));
	const float m = v - c;
	switch(h_)
	{
	case 0:
		r = c, g = x, b = 0.0;
		break;
	case 1:
		r = x, g = c, b = 0.0;
		break;
	case 2:
		r = 0.0, g = c, b = x;
		break;
	case 3:
		r = 0.0, g = x, b = c;
		break;
	case 4:
		r = x, g = 0.0, b = c;
		break;
	case 5:
		r = c, g = 0.0, b = x;
		break;
	}
	r += m;
	g += m;
	b += m;
	return true;
}

void HSVToRGB(const unsigned char hsv[3], unsigned char rgb[3])
{
	float frgb[3];
	HSVToRGB(hsv[0] * 360.0 / 255.0, hsv[1] / 255.0, hsv[2] / 255.0, frgb[0], frgb[1], frgb[2]);
	for (int i = 0; i < 3; ++i)
		rgb[i] = (frgb[i] < 0.0 ? 0.0 : (frgb[i] > 1.0 ? 1.0 : frgb[i])) * 255.0;
}

std::array<unsigned char, 3> HSVToRGB(const float h, const float s, const float v)
{
	std::array<float, 3> frgb;
	HSVToRGB(h, s, v, frgb[0], frgb[1], frgb[2]);
	std::array<unsigned char, 3> ret;
	for (int i = 0; i < 3; ++i)
		ret[i] = (frgb[i] < 0.0 ? 0.0 : (frgb[i] > 1.0 ? 1.0 : frgb[i])) * 255.0;
	return ret;
}

std::array<unsigned char, 3> HSVToRGB(const unsigned char hsv[3])
{
	std::array<unsigned char, 3> ret;
	HSVToRGB(hsv, ret.data());
	return ret;
}

void ListBuildings(CComboBox& cb, BOOL bININame)
{
	while(cb.DeleteString(0)!=CB_ERR);

	int i;
	for(i=0;i<rules.sections["BuildingTypes"].values.size();i++)
	{
		if(rules.sections["BuildingTypes"].GetValueOrigPos(i)<0) continue;

		char c[50];
		itoa(rules.sections["BuildingTypes"].GetValueOrigPos(i),c,10);
		CString s=c;
		
		
		//s+=rules.sections[*rules.sections["BuildingTypes"].GetValue(i)].values["Name"];
		//s+=Map->GetUnitName(*rules.sections["BuildingTypes"].GetValue(i));
		
		if(bININame) s=*rules.sections["BuildingTypes"].GetValue(i);
		
		s+=" ";

		CString t=Map->GetUnitName(*rules.sections["BuildingTypes"].GetValue(i));
		//if(t!="MISSING")
		{
			s+=t;
			cb.AddString(s);
		}
	}
}

void ListInfantry(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
	int i;
	for(i=0;i<rules.sections["InfantryTypes"].values.size();i++)
	{
		if(rules.sections["InfantryTypes"].GetValueOrigPos(i)<0) continue;

		char c[50];
		itoa(rules.sections["InfantryTypes"].GetValueOrigPos(i),c,10);
		CString s=c;
		
		s+=" ";
		//s+=Map->GetUnitName(*rules.sections["InfantryTypes"].GetValue(i));
		CString t=Map->GetUnitName(*rules.sections["InfantryTypes"].GetValue(i));
		//if(t!="MISSING")
		{
			s+=t;
			cb.AddString(s);
		}
	}
}

void ListUnits(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
	int i;
	for(i=0;i<rules.sections["VehicleTypes"].values.size();i++)
	{
		if(rules.sections["VehicleTypes"].GetValueOrigPos(i)<0) continue;

		char c[50];
		itoa(rules.sections["VehicleTypes"].GetValueOrigPos(i),c,10);
		CString s=c;
		
		s+=" ";
		//s+=rules.sections[*rules.sections["VehicleTypes"].GetValue(i)].values["Name"];
		CString t=Map->GetUnitName(*rules.sections["VehicleTypes"].GetValue(i));
		//if(t!="MISSING")
		{
			s+=t;
			cb.AddString(s);
		}
	}
}

void ListAircraft(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
	int i;
	for(i=0;i<rules.sections["AircraftTypes"].values.size();i++)
	{
		if(rules.sections["AircraftTypes"].GetValueOrigPos(i)<0) continue;

		char c[50];
		itoa(rules.sections["AircraftTypes"].GetValueOrigPos(i),c,10);
		CString s=c;
		
		s+=" ";
		//s+=rules.sections[*rules.sections["AircraftTypes"].GetValue(i)].values["Name"];
		CString t=Map->GetUnitName(*rules.sections["AircraftTypes"].GetValue(i));
		//if(t!="MISSING")
		{
			s+=t;
			cb.AddString(s);
		}
	}
}

void ListTechtypes(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
	int i;
	for(i=0;i<rules.sections["AircraftTypes"].values.size();i++)
	{
		if(rules.sections["AircraftTypes"].GetValueOrigPos(i)<0) continue;

		//char c[50];
		//itoa(rules.sections["AircraftTypes"].GetValueOrigPos(i),c,10);
		CString s=*rules.sections["AircraftTypes"].GetValue(i);
		
		s+=" ";
		//s+=rules.sections[*rules.sections["AircraftTypes"].GetValue(i)].values["Name"];
		CString t=Map->GetUnitName(*rules.sections["AircraftTypes"].GetValue(i));
		//if(t!="MISSING")
		{
			s+=t;
			cb.AddString(s);
		}
	}
	for(i=0;i<rules.sections["InfantryTypes"].values.size();i++)
	{
		if(rules.sections["InfantryTypes"].GetValueOrigPos(i)<0) continue;

		//char c[50];
		//itoa(rules.sections["InfantryTypes"].GetValueOrigPos(i),c,10);
		CString s=*rules.sections["InfantryTypes"].GetValue(i);
		
		s+=" ";
		//s+=rules.sections[*rules.sections["InfantryTypes"].GetValue(i)].values["Name"];
		CString t=Map->GetUnitName(*rules.sections["InfantryTypes"].GetValue(i));
		//if(t!="MISSING")
		{
			s+=t;
			cb.AddString(s);
		}
	}
	for(i=0;i<rules.sections["VehicleTypes"].values.size();i++)
	{
		if(rules.sections["VehicleTypes"].GetValueOrigPos(i)<0) continue;

		//char c[50];
		//itoa(rules.sections["VehicleTypes"].GetValueOrigPos(i),c,10);
		CString s=*rules.sections["VehicleTypes"].GetValue(i);
		
		s+=" ";
		//s+=rules.sections[*rules.sections["VehicleTypes"].GetValue(i)].values["Name"];
		CString t=Map->GetUnitName(*rules.sections["VehicleTypes"].GetValue(i));
		//if(t!="MISSING")
		{
			s+=t;
			cb.AddString(s);
		}
	}
	for(i=0;i<rules.sections["BuildingTypes"].values.size();i++)
	{
		if(rules.sections["BuildingTypes"].GetValueOrigPos(i)<0) continue;

		//char c[50];
		//itoa(rules.sections["BuildingTypes"].GetValueOrigPos(i),c,10);
		CString s=*rules.sections["BuildingTypes"].GetValue(i);
		
		s+=" ";
		//s+=rules.sections[*rules.sections["BuildingTypes"].GetValue(i)].values["Name"];
		CString t=Map->GetUnitName(*rules.sections["BuildingTypes"].GetValue(i));
		//if(t!="MISSING")
		{
			s+=t;
			cb.AddString(s);
		}
	}
}

// should be ListLocals()
void ListGlobals(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
	int i;
	CIniFile& ini=Map->GetIniFile();
	for(i=0;i<ini.sections["VariableNames"].values.size();i++)
	{
		CString s=*ini.sections["VariableNames"].GetValueName(i);
		s+=" ";
		s+=*ini.sections["VariableNames"].GetValue(i);
		
		cb.AddString(s);
	}
}

void ListRulesGlobals(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
	int i;
	for(i=0;i<rules.sections["VariableNames"].values.size();i++)
	{
		CString s=*rules.sections["VariableNames"].GetValueName(i);
		s+=" ";
		s+=*rules.sections["VariableNames"].GetValue(i);
		
		cb.AddString(s);
	}
}

extern map<CString, XCString> AllStrings;
void ListTutorial(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);

#ifndef RA2_MODE
	int i;
	for(i=0;i<tutorial.sections["Tutorial"].values.size();i++)
	{
		CString s;
		s=*tutorial.sections["Tutorial"].GetValueName(i);

		s+=" ";

		s+=*tutorial.sections["Tutorial"].GetValue(i);

		cb.AddString(s);
	}
#else
	
	typedef map<CString, XCString>::iterator it;
	it _it=AllStrings.begin();
	/*it begin;
	it end;
	begin=CCStrings.begin();
	end=CCStrings.end();*/

	int i;
	for(i=0;i<CCStrings.size();i++)
	{
		
		CString s;
		s=_it->first;
		s+=" : ";
		s+=_it->second.cString;

		cb.AddString(s);
		_it++;
	}

#endif
}

void ListTriggers(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
	int i;
	CIniFile& ini=Map->GetIniFile();
	for(i=0;i<ini.sections["Triggers"].values.size();i++)
	{
		CString type;
		CString s;
		type=*ini.sections["Triggers"].GetValueName(i);
		
		s=type;
		s+=" (";
		s+=GetParam(ini.sections["Triggers"].values[type], 2);
		s+=")";

		cb.AddString(s);
	}	
}

void ListYesNo(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
	cb.AddString("1 " + GetLanguageStringACP("Yes"));
	cb.AddString("0 " + GetLanguageStringACP("No"));
}

void ListSounds(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
#ifdef RA2_MODE
	int i;
	for(i=0;i<sound.sections["SoundList"].values.size();i++)
	{
		CString s;
		s=*sound.sections["SoundList"].GetValue(i);

		cb.AddString(s);
	}
#endif
}
void ListThemes(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
#ifdef RA2_MODE
	int i;
	for(i=0;i<theme.sections["Themes"].values.size();i++)
	{
		CString s;
		s=*theme.sections["Themes"].GetValue(i);

		TruncSpace(s);

		if(s.GetLength()==0) continue;

		s+=" ";
		s+=AllStrings[sound.sections[s].values["Name"]].cString;

		cb.AddString(s);
	}
#endif
}
void ListSpeeches(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
#ifdef RA2_MODE
	int i;
	for(i=0;i<eva.sections["DialogList"].values.size();i++)
	{
		CString s;
		s=*eva.sections["DialogList"].GetValue(i);

		cb.AddString(s);
	}
#endif
}
void ListSpecialWeapons(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
	int i;
	for(i=0;i<rules.sections["SuperWeaponTypes"].values.size();i++)
	{
		CString s;
		char c[50];
		itoa(rules.sections["SuperWeaponTypes"].GetValueOrigPos(i),c,10);
		s=c;

		s+=" ";

		s+=*rules.sections["SuperWeaponTypes"].GetValue(i);

		cb.AddString(s);
	}	
}

void ListAnimations(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
	int i;
	for(i=0;i<rules.sections["Animations"].values.size();i++)
	{
		CString s;
		char c[50];
		itoa(rules.sections["Animations"].GetValueOrigPos(i),c,10);
		s=c;

		s+=" ";
		s+=*rules.sections["Animations"].GetValue(i);

		cb.AddString(s);
	}	
}

void ListParticles(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
	int i;
	for(i=0;i<rules.sections["Particles"].values.size();i++)
	{
		CString s;
		char c[50];
		itoa(rules.sections["Particles"].GetValueOrigPos(i),c,10);
		s=c;

		s+=" ";
		s+=*rules.sections["Particles"].GetValue(i);

		cb.AddString(s);
	}
}

void ListCrateTypes(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
}
void ListSpeechBubbleTypes(CComboBox& cb)
{
	while(cb.DeleteString(0)!=CB_ERR);
}

void ListMovies(CComboBox& cb, BOOL bListNone, BOOL bListParam)
{
	if(!bListParam)
	{
		int sel=cb.GetCurSel();

		while(cb.DeleteString(0)!=CB_ERR);

		int i;
		if(bListNone) cb.AddString("<none>");
		for(i=0;i<art.sections["Movies"].values.size();i++)
		{
			if(i<atoi(g_data.sections["MovieList"].values["Start"])) continue;

			CString s=*art.sections["Movies"].GetValue(i);
			cb.AddString(s);
		}
		if(sel>=0) cb.SetCurSel(sel);
	}
	else
	{
		
		while(cb.DeleteString(0)!=CB_ERR);

		int i;
		for(i=0;i<art.sections["Movies"].values.size();i++)
		{
			if(i<atoi(g_data.sections["MovieList"].values["Start"])) continue;

			CString s;
			char c[50];
			itoa(art.sections["Movies"].GetValueOrigPos(i),c,10);
			s=c;
			s+=" ";
			s+=*art.sections["Movies"].GetValue(i);
			
			cb.AddString(s);

		}

	}
}

void ListTags(CComboBox& cb, BOOL bListNone)
{
	CIniFile& ini=Map->GetIniFile();

	int sel=cb.GetCurSel();

	while(cb.DeleteString(0)!=CB_ERR);

	int i;
	if(bListNone) cb.AddString("None");
	for(i=0;i<ini.sections["Tags"].values.size();i++)
	{
		CString type=*ini.sections["Tags"].GetValueName(i);
		CString s=type;
		s+=" ";
		s+=GetParam(*ini.sections["Tags"].GetValue(i), 1);

		cb.AddString(s);
	}

	if(sel>=0) cb.SetCurSel(sel);
}

int GetRulesHousesSize()
{
	int i;
	int count=0;
	for(i=0;i<rules.sections[HOUSES].values.size();i++)
	{
		if(rules.sections[HOUSES].GetValueOrigPos(i)<0) continue;
		count++;
	}

	return count;
}

// a bug adds an empty house to the rules section, delete it here
int RepairRulesHouses()
{
	int i;
	int count=0;
	int delcount=0;
	CString* toDelete;
	toDelete=new(CString[rules.sections[HOUSES].values.size()]);

	for(i=0;i<rules.sections[HOUSES].values.size();i++)
	{
		if(rules.sections[HOUSES].GetValueOrigPos(i)<0)
		{
			toDelete[delcount]=*rules.sections[HOUSES].GetValueName(i);						
			delcount++;
		}
		else
			count++;
	}

	for(i=0;i<delcount;i++)
		rules.sections[HOUSES].values.erase(toDelete[i]);

	delete[] toDelete;

	return count;
}

// MW 07/27/01: Modified for <Player @ A> etc in YR
void ListHouses(CComboBox &cb, BOOL bNumbers, BOOL bCountries, BOOL bPlayers)
{
	CIniFile& ini=Map->GetIniFile();

	int i;
	int sel=cb.GetCurSel();
	int crulesh=GetRulesHousesSize();

	if(Map->IsMultiplayer()==FALSE) bPlayers=FALSE; // really only for multi maps!

	//for(i=0;i<ini.sections[HOUSES]

	CString sSection=MAPHOUSES;
	if(bCountries) sSection=HOUSES;

	while(cb.DeleteString(0)!=CB_ERR);
	// houses:  rules.ini + map definitions!
	if(ini.sections.find(sSection)!=ini.sections.end())
	{
		if(ini.sections[sSection].values.size()==0) goto wasnohouse;
		// we use the map definitions!
		
		if(yuri_mode && bPlayers)
		{
			if(bNumbers)
			{
				cb.AddString("4475 <Player @ A>");
				cb.AddString("4476 <Player @ B>");
				cb.AddString("4477 <Player @ C>");
				cb.AddString("4478 <Player @ D>");
				cb.AddString("4479 <Player @ E>");
				cb.AddString("4480 <Player @ F>");
				cb.AddString("4481 <Player @ G>");
				cb.AddString("4482 <Player @ H>");								
			}
			else
			{
				cb.AddString("<Player @ A>");
				cb.AddString("<Player @ B>");
				cb.AddString("<Player @ C>");
				cb.AddString("<Player @ D>");
				cb.AddString("<Player @ E>");
				cb.AddString("<Player @ F>");
				cb.AddString("<Player @ G>");
				cb.AddString("<Player @ H>");
			}

			
		}
				
		for(i=0;i<ini.sections[sSection].values.size();i++)
		{
			CString j;

#ifdef RA2_MODE
			j=*ini.sections[sSection].GetValue(i);
			j.MakeLower();
			if(j=="nod" || j=="gdi") continue;
#endif	
			
			if(bNumbers)
			{
				char c[50];
				int n=atoi(*ini.sections[sSection].GetValueName(i));				
				itoa(n, c, 10);
#ifdef RA2_MODE
				if(bCountries)
				{
					int preexisting=0;
					int e;
					for(e=0;e<i;e++)
					{
						if(rules.sections[sSection].FindValue(*ini.sections[sSection].GetValue(e))>=0)
							preexisting++;
					}
					if(rules.sections[sSection].FindValue(*ini.sections[sSection].GetValue(i))>=0)
					{
						itoa(rules.sections[sSection].value_orig_pos[*rules.sections[sSection].GetValueName(rules.sections[sSection].FindValue(*ini.sections[sSection].GetValue(i)))], c, 10);
					}
					else
					{
						itoa(n+crulesh-preexisting, c, 10);
					}
				}
#endif
				j=c;
				j+=" ";
				j+=TranslateHouse(*ini.sections[sSection].GetValue(i), TRUE);
			}
			else
				j=TranslateHouse(*ini.sections[sSection].GetValue(i), TRUE);

			cb.AddString(j);		
		}
	}
	else
	{
		wasnohouse:

		if(bNumbers)
		{

			for(i=0;i<rules.sections[HOUSES].values.size();i++)
			{
				CString j;

#ifdef RA2_MODE
				j=*rules.sections[HOUSES].GetValue(i);
				j.MakeLower();
				if(j=="nod" || j=="gdi") continue;
#endif				
				j=*rules.sections[HOUSES].GetValueName(i);
				j+=" ";
				j+=TranslateHouse(*rules.sections[HOUSES].GetValue(i), TRUE);
				
				cb.AddString(j);	
			}


			if(!yuri_mode || !bPlayers )
			{
				for(i=0;i<8;i++)
				{
					int k=i;
	#ifdef RA2_MODE
					k+=crulesh;
					
					
					//rules.sections[HOUSES].values.size();
	#endif
					CString j;
					char c[50];
					itoa(k,c,10);
					j+=c;
					j+=" Multi-Player ";
					itoa(i,c,10);				
					j+=c;
					cb.AddString(j);					
				}
			}
			else
			{
				cb.AddString("4475 <Player @ A>");
				cb.AddString("4476 <Player @ B>");
				cb.AddString("4477 <Player @ C>");
				cb.AddString("4478 <Player @ D>");
				cb.AddString("4479 <Player @ E>");
				cb.AddString("4480 <Player @ F>");
				cb.AddString("4481 <Player @ G>");
				cb.AddString("4482 <Player @ H>");
			}
		}
		else
		{
			if(yuri_mode && bPlayers)
			{
				cb.AddString("<Player @ A>");
				cb.AddString("<Player @ B>");
				cb.AddString("<Player @ C>");
				cb.AddString("<Player @ D>");
				cb.AddString("<Player @ E>");
				cb.AddString("<Player @ F>");
				cb.AddString("<Player @ G>");
				cb.AddString("<Player @ H>");				
			}

			for(i=0;i<rules.sections[HOUSES].values.size();i++)
			{
				CString j;

#ifdef RA2_MODE
				j=*rules.sections[HOUSES].GetValue(i);
				j.MakeLower();
				if(j=="nod" || j=="gdi") continue;
#endif				

				if(bNumbers)
				{
					j=*rules.sections[HOUSES].GetValueName(i);
					j+=" ";
					j+=TranslateHouse(*rules.sections[HOUSES].GetValue(i), TRUE);
				}
				else
					j=TranslateHouse(*rules.sections[HOUSES].GetValue(i), TRUE);

				cb.AddString(j);		
			}
		}
	}

	if(sel>=0) cb.SetCurSel(sel);

}


void ListTeamTypes(CComboBox &cb, BOOL bListNone)
{
	CIniFile& ini=Map->GetIniFile();

	int sel=cb.GetCurSel();

	while(cb.DeleteString(0)!=CB_ERR);

	int i;
	if(bListNone) cb.AddString("<none>");
	for(i=0;i<ini.sections["TeamTypes"].values.size();i++)
	{
		CIniFileSection& sec=ini.sections[*ini.sections["TeamTypes"].GetValue(i)];
		CString type=*ini.sections["TeamTypes"].GetValue(i);
		CString s=type;
		s+=" ";
		s+=sec.values["Name"];

		cb.AddString(s);
	}

	if(sel>=0) cb.SetCurSel(sel);
}

void ListWaypoints(CComboBox &cb)
{
	CIniFile& ini=Map->GetIniFile();

	int sel=cb.GetCurSel();

	while(cb.DeleteString(0)!=CB_ERR);

	int i;
	for(i=0;i<ini.sections["Waypoints"].values.size();i++)
	{
		
		CString s=*ini.sections["Waypoints"].GetValueName(i); //type;
		
		cb.AddString(s);
	}

	if(sel>=0) cb.SetCurSel(sel);
}

void ListTargets(CComboBox &cb)
{
	int sel=cb.GetCurSel();

	while(cb.DeleteString(0)!=CB_ERR);

	cb.AddString("1 - Not specified");
	cb.AddString("2 - Buildings");
	cb.AddString("3 - Harvesters");
	cb.AddString("4 - Infantry");
	cb.AddString("5 - Vehicles");
	cb.AddString("6 - Factories");
	cb.AddString("7 - Base defenses");
	cb.AddString("9 - Power plants");

	if(sel>=0) cb.SetCurSel(sel);

}


CString GetHouseSectionName(CString lpHouse)
{
#ifndef RA2_MODE
	return lpHouse;
#else
	return lpHouse+" House";
#endif

}


CString GetFreeID()
{
	CIniFile& ini=Map->GetIniFile();

	int n=1000000;
	while(TRUE)
	{
		char p[50];
		p[0]='0';
		itoa(n,p+1,10);
		if(ini.sections["ScriptTypes"].FindValue(p)==-1)
		{
			if(ini.sections["TaskForces"].FindValue(p)==-1)
			{
				if(ini.sections["TeamTypes"].FindValue(p)==-1)
				{
					if(ini.sections["Triggers"].values.find(p)==ini.sections["Triggers"].values.end())
					{
						if(ini.sections["Events"].values.find(p)==ini.sections["Events"].values.end())
						{
							if(ini.sections["Tags"].values.find(p)==ini.sections["Tags"].values.end())
							{
								if(ini.sections["Actions"].values.find(p)==ini.sections["Actions"].values.end())
								{
									if(ini.sections["AITriggerTypes"].values.find(p)==ini.sections["AITriggerTypes"].values.end())
									{
										if(ini.sections.find(p)==ini.sections.end())
											return p;
									}
								}
							}
						}
					}
				}
			}
		}
		n++;
	}		
	return "";
}

void GetNodeName(CString & name, int n)
{
	char c[5];
	char p[6];
	memset(p,0,6);
	itoa(n,c,10);
	strcpy(p,c);
	
	if(strlen(c)==1)
	{
		memcpy(c,"00", 2);
		strcpy(c+2, p);	
	}
	else if(strlen(c)==2)
	{
		memcpy(c,"0", 1);
		strcpy(c+1, p);						
	}
	else if(strlen(c)==3)
	{
		strcpy(c, p);						
	}

	name=c;
}

int GetNodeAt(CString& owner, CString& type, int x, int y)
{
	CIniFile& ini=Map->GetIniFile();

	type="";
	owner="";

	int owners;
	if(ini.sections.find(HOUSES)!=ini.sections.end())
	{
		for(owners=0;owners<ini.sections[HOUSES].values.size();owners++)
		{
			owner=*ini.sections[HOUSES].GetValue(owners);
			
			// okay now owner is correct!
			int i,c=atoi(ini.sections[owner].values["NodeCount"]);

			for(i=0;i<c;i++)
			{
				CString p;
				GetNodeName(p, i);
				
				CString sx, sy;
				type=GetParam(ini.sections[owner].values[p], 0); 
				sy=GetParam(ini.sections[owner].values[p], 1);
				sx=GetParam(ini.sections[owner].values[p], 2);

				CString arttype=type;
				if(rules.sections[type].values.find("Image")!=rules.sections[type].values.end())
				{
					// other art!
					arttype=rules.sections[type].values["Image"];
				}
				if(ini.sections.find(type)!=ini.sections.end())
				if(ini.sections[type].values.find("Image")!=ini.sections[type].values.end())
				{
					// other art!
					arttype=rules.sections[type].values["Image"];
				}

				int w,h;
				char d[6];
				memcpy(d, (LPCTSTR)art.sections[arttype].values["Foundation"],1);
				d[1]=0;
				w=atoi(d);
				if(w==0) w=1;
				memcpy(d, (LPCTSTR)art.sections[arttype].values["Foundation"]+2,1);
				d[1]=0;
				h=atoi(d);
				if(h==0) h=1;

				int j,k;
				for(j=0;j<h;j++)
				{
					for(k=0;k<w;k++)
					{
						if(atoi(sx)+j==x && atoi(sy)+k==y)
							return i;
					}
				}
			}
		}
	}
	else
		return -1;
	
	
	
	return -1;
}


std::unique_ptr<CBitmap> BitmapFromResource(int resource_id)
{
	std::unique_ptr<CBitmap> bm(new CBitmap);
	if (!bm->LoadBitmap(resource_id))
		throw BitmapNotFound();
	return bm;
}

std::unique_ptr<CBitmap> BitmapFromFile(const CString& filepath)
{
	std::unique_ptr<CBitmap> bm(new CBitmap);
	if (!bm->LoadBitmap(filepath))
		throw BitmapNotFound();
	return bm;
}

/*
Returns the area in the current line that should be painted
Truncates areas that are transparent, and therefore increases display speed!
flags must be set to 0
*/
void GetDrawBorder(const BYTE* data, int width, int line, int& left, int& right, unsigned int flags, BOOL* TranspInside)
{
	int i;
	const BYTE* lpStart = data + line * width;

	if (flags == 0)
	{
		// left border:
		for (i = 0;i < width;i++)
		{
			if (lpStart[i] || i == width - 1)
			{
				left = i;
				break;
			}
		}

		// right border:
		for (i = width - 1;i >= 0;i--)
		{
			if (lpStart[i] || i == 0)
			{
				right = i;
				break;
			}
		}

		if (TranspInside)
		{
			for (i = left;i <= right;i++)
			{
				if (!lpStart[i])
				{
					*TranspInside = TRUE;
					break;
				}
			}
		}
	}
}

CComPtr<IDirectDrawSurface4> BitmapToSurface(IDirectDraw4 * pDD, const CBitmap& bitmap)
{
	BITMAP bm;
	GetObject(bitmap, sizeof(bm), &bm);

	DDSURFACEDESC2 desc = { 0 };
	ZeroMemory(&desc, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	desc.dwWidth = bm.bmWidth;
	desc.dwHeight = bm.bmHeight;

	auto pSurface = CComPtr<IDirectDrawSurface4>();
	if (pDD->CreateSurface(&desc, &pSurface, nullptr) != DD_OK)
		return nullptr;

	pSurface->Restore();

	CDC bitmapDC;
	if (!bitmapDC.CreateCompatibleDC(nullptr))
		return nullptr;
	bitmapDC.SelectObject(bitmap);

	HDC hSurfaceDC = nullptr;
	if (pSurface->GetDC(&hSurfaceDC) != DD_OK)
		return nullptr;

	CDC surfaceDC;
	surfaceDC.Attach(hSurfaceDC);

	auto success = surfaceDC.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &bitmapDC, 0, 0, SRCCOPY);
	surfaceDC.Detach();
	pSurface->ReleaseDC(hSurfaceDC);
    
	return pSurface;
}



