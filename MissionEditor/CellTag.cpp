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

// CellTag.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "CellTag.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CCellTag 


CCellTag::CCellTag(CWnd* pParent /*=NULL*/)
	: CDialog(CCellTag::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCellTag)
	m_tag = _T("");
	//}}AFX_DATA_INIT
}


void CCellTag::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCellTag)
	DDX_CBString(pDX, IDC_TAG, m_tag);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCellTag, CDialog)
	//{{AFX_MSG_MAP(CCellTag)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCellTag 

string GetParam(string data, int pos);

BOOL CCellTag::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CIniFile& ini=Map->GetIniFile();
	
	CComboBox& m_Tag=*((CComboBox*)GetDlgItem(IDC_TAG));

	if(ini.sections.find("Tags")==ini.sections.end())
	{
		MessageBox("No tags are specified.");
		OnCancel();
	}
	else
	{
		ListTags(m_Tag, FALSE);
		if(m_tag=="") m_Tag.SetCurSel(0);
	}
	
	UpdateStrings();

	return TRUE;  
}

void CCellTag::OnOK() 
{
	UpdateData();
	
	CDialog::OnOK();
	
	TruncSpace(m_tag);
}

void CCellTag::UpdateStrings()
{
	SetWindowText(GetLanguageStringACP("CellTagCap"));
	GetDlgItem(IDC_LTAG)->SetWindowText(GetLanguageStringACP("CellTagTag"));
	GetDlgItem(IDC_LDESC)->SetWindowText(GetLanguageStringACP("CellTagDesc"));
	
}
