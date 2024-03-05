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

// GlobalsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "GlobalsDlg.h"
#include "mapdata.h"
#include "variables.h"
#include "inlines.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CGlobalsDlg 


CGlobalsDlg::CGlobalsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGlobalsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGlobalsDlg)
	m_Description = _T("");
	//}}AFX_DATA_INIT
}


void CGlobalsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalsDlg)
	DDX_Control(pDX, IDC_VALUE, m_Value);
	DDX_Control(pDX, IDC_GLOBAL, m_Global);
	DDX_Text(pDX, IDC_DESCRIPTION, m_Description);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalsDlg, CDialog)
	//{{AFX_MSG_MAP(CGlobalsDlg)
	ON_EN_CHANGE(IDC_DESCRIPTION, OnChangeDescription)
	ON_CBN_SELCHANGE(IDC_GLOBAL, OnSelchangeGlobal)
	ON_CBN_SELCHANGE(IDC_VALUE, OnSelchangeValue)
	ON_CBN_EDITCHANGE(IDC_VALUE, OnEditchangeValue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CGlobalsDlg 

void CGlobalsDlg::OnOK() 
{
	
	// CDialog::OnOK();
}

void CGlobalsDlg::OnCancel() 
{
	// TODO: Zusätzlichen Bereinigungscode hier einfügen
	
	CDialog::OnCancel();
}

void CGlobalsDlg::UpdateDialog()
{
	int oldsel=m_Global.GetCurSel();
	if(oldsel<0) oldsel=0;

	while(m_Global.DeleteString(0)!=CB_ERR);

	int i;
	BOOL bFailFind=FALSE;
	CIniFile& ini=Map->GetIniFile();
	for(i=0;i<100;i++)
	{
		char c[50];
		itoa(i,c,10);
		CString added=c;
		added+=" ";

		if(ini.sections["VariableNames"].FindName(c)>=0)
		{
			added+=ini.sections["VariableNames"].values[c];
		}
		else
		{
			bFailFind=TRUE;
			added+=" No name";
		}

		m_Global.SetItemData(m_Global.AddString(added),i);

		if(bFailFind) break;
	}

	m_Global.SetCurSel(oldsel);
	OnSelchangeGlobal();
}

void CGlobalsDlg::OnChangeDescription() 
{
	CIniFile& ini=Map->GetIniFile();

	int cursel=m_Global.GetCurSel();
	if(cursel<0) return;
	int curglob=m_Global.GetItemData(cursel);

	

	char c[50];
	itoa(curglob, c, 10);

	UpdateData(TRUE);

	if(m_Description.Find(",")>=0) m_Description.SetAt(m_Description.Find(","), 0);

	if(ini.sections["VariableNames"].values[c].GetLength()==0) ini.sections["VariableNames"].values[c]="text,0";
	ini.sections["VariableNames"].values[c]=SetParam(ini.sections["VariableNames"].values[c], 0, m_Description);

	// do not remove, Tiberian Sun seems to don´t like probably unused global numbers
	//if(m_Description.GetLength()==0)
	//  ini.sections["VariableNames"].values.erase(c);

	UpdateDialog();

}

void CGlobalsDlg::OnSelchangeGlobal() 
{
	CIniFile& ini=Map->GetIniFile();

	int cursel=m_Global.GetCurSel();
	if(cursel<0) return;
	int curglob=m_Global.GetItemData(cursel);

	char c[50];
	itoa(curglob, c, 10);

	if(ini.sections["VariableNames"].FindName(c)>=0)
	{
		m_Description=GetParam(ini.sections["VariableNames"].values[c],0);
		m_Value.SetWindowText(GetParam(ini.sections["VariableNames"].values[c],1));
	}
	else
		m_Description="";

	UpdateData(FALSE);	
}

BOOL CGlobalsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	UpdateDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CGlobalsDlg::OnSelchangeValue() 
{
	
}

void CGlobalsDlg::OnEditchangeValue() 
{
	CIniFile& ini=Map->GetIniFile();
	CString str;
	m_Value.GetWindowText(str);
	if(str.GetLength()==0) return;
	int cursel=m_Global.GetCurSel();
	if(cursel<0) return;
	int curglob=m_Global.GetItemData(cursel);
	
	char c[50];
	itoa(curglob, c, 10);

	if(ini.sections["VariableNames"].FindName(c)<0) return;

	
	UpdateData(TRUE);
	
	str=GetParam(str, 0);
	TruncSpace(str);
	ini.sections["VariableNames"].values[c]=SetParam(ini.sections["VariableNames"].values[c], 1, str);

	UpdateDialog();	
}
