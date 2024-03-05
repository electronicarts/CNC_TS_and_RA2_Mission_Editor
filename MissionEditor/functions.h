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

#ifndef FUNC_INCLUDED
#define FUNC_INCLUDED

#include <afx.h>
#include <memory>
#include <array>

bool deleteFile(const std::string& u8FilePath);

// set the status bar text in the main dialog
void SetMainStatusBar(const char* text);
// set the status bar text in the main dialog to ready
void SetMainStatusBarReady();

// change all %n (where n is an int) in an string, to another specified string
CString TranslateStringVariables(int n, const char* originaltext, const char* inserttext);

// Alliance->Korea etc...
CString TranslateHouse(CString original, BOOL bToUI=FALSE);

// show options dialog
void ShowOptionsDialog();

// repairs a trigger (sets flags correctly)
void RepairTrigger(CString& triggerdata);

// coordinate functions
void PosToXY(const char* pos, int* X, int* Y);

// HSV -> RGB
void HSVToRGB(const unsigned char hsv[3], unsigned char rgb[3]);
std::array<unsigned char, 3> HSVToRGB(float h, float s, float v);
std::array<unsigned char, 3> HSVToRGB(const unsigned char hsv[3]);

void GetDrawBorder(const BYTE* data, int width, int line, int& left, int& right, unsigned int flags, BOOL* TranspInside = NULL);

// String conversion
std::wstring utf8ToUtf16(const char* utf8);
std::wstring utf8ToUtf16(const std::string& utf8);
std::string utf16ToUtf8(const std::wstring& utf16);
std::string utf16ToACP(const std::wstring& utf16);

// map functions
int GetNodeAt(string& owner, string& type, int x, int y);
int SetNodeAt(string owner, string type, int x, int y);
void ClearNode(int n, string owner);
CString GetFreeID();


void HandleParamList(CComboBox &cb, int type);

// sets 0 if there is a space in the string, so truncates it at the first space occupation
void TruncSpace(string& str);
void TruncSpace(CString& str);

// checks if a file exists
BOOL DoesFileExist(LPCSTR szFile);


// Lists some things
void ListHouses(CComboBox &cb, BOOL bNumbers=FALSE, BOOL bCountries=FALSE, BOOL bPlayers=FALSE);
void ListTeamTypes(CComboBox &cb, BOOL bListNone=FALSE);
void ListWaypoints(CComboBox &cb);
void ListTargets(CComboBox& cb);
void ListTags(CComboBox& cb, BOOL bListNone);
void ListMovies(CComboBox& cb, BOOL bListNone, BOOL bListParam=FALSE);
void ListBuildings(CComboBox& cb, BOOL bININame=FALSE);
void ListUnits(CComboBox& cb);
void ListAircraft(CComboBox& cb);
void ListInfantry(CComboBox& cb);
void ListTutorial(CComboBox& cb);
void ListTriggers(CComboBox& cb);
void ListYesNo(CComboBox& cb);
void ListSounds(CComboBox& cb);
void ListThemes(CComboBox& cb);
void ListSpeeches(CComboBox& cb);
void ListSpecialWeapons(CComboBox& cb);
void ListAnimations(CComboBox& cb);
void ListParticles(CComboBox& cb);
void ListCrateTypes(CComboBox& cb);
void ListSpeechBubbleTypes(CComboBox& cb);
void ListGlobals(CComboBox& cb);
void ListRulesGlobals(CComboBox& cb);
void ListTechtypes(CComboBox& cb);


// some easy-to-use functions
CString GetText(CWnd* wnd);
CString GetText(CComboBox* wnd);
CString GetText(CSliderCtrl* wnd);

CString GetHouseSectionName(CString lpHouse);

// fix bug
int RepairRulesHouses();

// strcpy_safe allows overlapping of source & destination. strcpy may be faster though!
char *strcpy_safe( char *strDestination, const char *strSource );

/****************************************
 language support functions [12/18/1999]
****************************************/

// retrieve the string name in the correct language (name is an ID).
// the returned string is in the active codepage (UTF-8 on Windows 10 1909+)
CString GetLanguageStringACP(CString name);

// tranlate a string/word by using the table from english to the current language
// the returned string is in the active codepage (UTF-8 on Windows 10 1909+)
CString TranslateStringACP(CString u8EnglishString);

// tranlate a string/word by using the table from english to the current language
// the returned string is in the active codepage (UTF-8 on Windows 10 1909+)
CString TranslateStringACP(WCHAR* u16EnglishString);

/****************************************
 sound functions [03/16/2001]
 ****************************************/

// general play sound function. uses parameter in defines.h. Does only play if user has not deactivated sounds
void Sound(int ID);


class BitmapNotFound : std::runtime_error
{
public:
    BitmapNotFound(): std::runtime_error("Bitmap not found") {}
};

std::unique_ptr<CBitmap> BitmapFromResource(int resource_id);
std::unique_ptr<CBitmap> BitmapFromFile(const CString& filepath);

CComPtr<IDirectDrawSurface4> BitmapToSurface(IDirectDraw4 * pDD, const CBitmap& bitmap);


#endif