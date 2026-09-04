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

// TSOptions.cpp: implementation
//

#include "stdafx.h"
#include "FinalSun.h"
#include "TSOptions.h"
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
// Dialogfeld CTSOptions 


CTSOptions::CTSOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CTSOptions::IDD, pParent)
	, m_PreferLocalTheaterFiles(FALSE)
{
	//{{AFX_DATA_INIT(CTSOptions)
	m_LikeTS = -1;
	//}}AFX_DATA_INIT
}


void CTSOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTSOptions)
	DDX_Control(pDX, IDC_LANGUAGE, m_Language);
	DDX_Control(pDX, IDC_EDIT1, m_TSExe);
	DDX_Radio(pDX, IDC_RULESLIKETS, m_LikeTS);
	DDX_Check(pDX, IDC_PREFER_LOCAL_THEATER_FILES, m_PreferLocalTheaterFiles);
	//}}AFX_DATA_MAP	
}


BEGIN_MESSAGE_MAP(CTSOptions, CDialog)
	//{{AFX_MSG_MAP(CTSOptions)
	ON_BN_CLICKED(IDC_CHOOSE, OnChoose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTSOptions 

void CTSOptions::OnChoose() 
{
#ifndef RA2_MODE
	CFileDialog fd(TRUE, NULL, "Sun.exe", OFN_FILEMUSTEXIST, "Tiberian Sun EXE|Sun.exe|");
#else
	CFileDialog fd(TRUE, NULL, "ra2.exe", OFN_FILEMUSTEXIST, "Red Alert 2 EXE|ra2.exe|");
#endif

	fd.DoModal();

	this->GetDlgItem(IDC_EDIT1)->SetWindowText((LPCTSTR)fd.GetPathName());

}

void CTSOptions::OnOK() 
{
	this->GetDlgItem(IDC_EDIT1)->GetWindowText(m_TSEXE);
	const int selection = m_Language.GetCurSel();
	if (selection == CB_ERR) return;
	int n = static_cast<int>(m_Language.GetItemData(selection));
	
	m_LanguageName=*language.sections["Languages"].GetValue(n);

	CDialog::OnOK();
}

BOOL CTSOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_TSExe.SetWindowText((LPCTSTR)theApp.m_Options.TSExe);
	
	if(theApp.m_Options.bSearchLikeTS) m_LikeTS=0;
	else m_LikeTS=1;

	m_PreferLocalTheaterFiles = theApp.m_Options.bPreferLocalTheaterFiles ? TRUE : FALSE;

	UpdateData(FALSE);

	for (int i = 0; i < language.sections["Languages"].values.size(); ++i)
	{
		const CString id = *language.sections["Languages"].GetValue(i);
		const CString name = language.sections[id + "Header"].values["Name"];
		// Use Unicode so every language name is readable regardless of current language.
		const int item = static_cast<int>(::SendMessageW(m_Language.m_hWnd, CB_ADDSTRING, 0,
			reinterpret_cast<LPARAM>(utf8ToUtf16(name.GetString()).c_str())));
		if (item >= 0) m_Language.SetItemData(item, i);
	}
	// CBS_SORT changes indices while adding strings: select only after insertion.
	for (int item = 0; item < m_Language.GetCount(); ++item)
	{
		const int index = static_cast<int>(m_Language.GetItemData(item));
		if (*language.sections["Languages"].GetValue(index) == theApp.m_Options.LanguageName)
			m_Language.SetCurSel(item);
	}
	if (m_Language.GetCurSel() == CB_ERR) m_Language.SetCurSel(0);

	return TRUE;  
}
