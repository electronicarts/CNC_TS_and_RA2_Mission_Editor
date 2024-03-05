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

// ImportINI.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "ImportINI.h"
#include "mapdata.h"
#include "variables.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CImportINI 


CImportINI::CImportINI(CWnd* pParent /*=NULL*/)
	: CDialog(CImportINI::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportINI)
	//}}AFX_DATA_INIT
	m_inicount=0;
}


void CImportINI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportINI)
	DDX_Control(pDX, IDC_AVAILABLE, m_Available);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportINI, CDialog)
	//{{AFX_MSG_MAP(CImportINI)
	ON_BN_CLICKED(IDC_ALLSECTIONS, OnAllsections)
	ON_BN_CLICKED(IDC_SPECIFYSECTIONS, OnSpecifysections)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CImportINI 

void CImportINI::OnAllsections() 
{
	this->m_Available.EnableWindow(FALSE);
	//this->m_Sections.EnableWindow(FALSE);
}

void CImportINI::OnSpecifysections() 
{
	this->m_Available.EnableWindow();
	//this->m_Sections.EnableWindow();	
}

BOOL CImportINI::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CButton* all;
	all=(CButton*)GetDlgItem(IDC_ALLSECTIONS);
	all->SetCheck(1);

	this->OnAllsections();

	CIniFile inifile;
	inifile.LoadFile(this->m_FileName);
	
	if(inifile.sections.size()<1){MessageBox("File does not have any ini content, abort.","Error");EndDialog(IDCANCEL);return TRUE;}
	
	m_inicount=inifile.sections.size();

	int i;
	for(i=0;i<inifile.sections.size();i++)
	{
		if(!Map->IsMapSection(*inifile.GetSectionName(i)))
		m_Available.InsertString(-1, *inifile.GetSectionName(i));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CImportINI::OnOK() 
{
	CIniFile& ini=Map->GetIniFile();

	CButton* all;
	all=(CButton*)GetDlgItem(IDC_ALLSECTIONS);
	
	if(all->GetCheck())
	{
		// all...
		ini.InsertFile(m_FileName, NULL);
	}
	else
	{
		// only the chosen sections

		int i;
		for(i=0;i<m_Available.GetCount();i++)
		{
			if(m_Available.GetSel(i)>0)
			{
				// ok the user wants to add that section!
				CString name;
				m_Available.GetText(i, name);

				ini.InsertFile(m_FileName, (char*)(LPCTSTR)name);
			}
		}
	}
	
	CDialog::OnOK();
}
