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

// MapOpenDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "MapOpenDialog.h"
#include "inlines.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapOpenDialog

UINT CALLBACK OFNHookProc(
  HWND hdlg,      // handle to child dialog window
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
)
{
	if(uiMsg==WM_NOTIFY)
	{
		OFNOTIFY ofn;
		ofn=*((OFNOTIFY*)lParam);
		if(ofn.hdr.code==CDN_SELCHANGE)
		{
			// user selected another file
			wchar_t psz[MAX_PATH] = { 0 }; // filename
			CommDlg_OpenSave_GetFilePathW(GetParent(hdlg), psz, MAX_PATH);
			
			// lets parse the file. Only load Basic section
			CIniFile CurMap;
			CurMap.InsertFile(utf16ToUtf8(psz),"Basic");

			SetDlgItemText(hdlg, IDC_MAPNAME, CurMap.sections["Basic"].values["Name"]);
			
			
		}
		else if (ofn.hdr.code==CDN_FOLDERCHANGE)
			RedrawWindow(GetParent(hdlg),NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE );
	}

	return 0;
}


IMPLEMENT_DYNAMIC(CMapOpenDialog, CFileDialog)

CMapOpenDialog::CMapOpenDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_ofn.Flags|=OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
	m_ofn.lpTemplateName=MAKEINTRESOURCE(IDD_MYOPENDIALOG);
	m_ofn.lpfnHook=OFNHookProc;
}


BEGIN_MESSAGE_MAP(CMapOpenDialog, CFileDialog)
	//{{AFX_MSG_MAP(CMapOpenDialog)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

