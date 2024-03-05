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

// TriggerEditorDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "TriggerEditorDlg.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"

extern ACTIONDATA AD;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTriggerEditorDlg 


CTriggerEditorDlg::CTriggerEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTriggerEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTriggerEditorDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CTriggerEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTriggerEditorDlg)
	DDX_Control(pDX, IDC_TRIGGER, m_Trigger);
	DDX_Control(pDX, IDC_TRIGGERTAB, m_tab);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTriggerEditorDlg, CDialog)
	//{{AFX_MSG_MAP(CTriggerEditorDlg)
	ON_BN_CLICKED(IDC_NEWTRIGGER, OnNewtrigger)
	ON_BN_CLICKED(IDC_DELETETRIGGER, OnDeletetrigger)
	ON_CBN_SELCHANGE(IDC_TRIGGER, OnSelchangeTrigger)
	ON_CBN_EDITCHANGE(IDC_TRIGGER, OnEditchangeTrigger)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TRIGGERTAB, OnSelchangeTriggertab)
	ON_BN_CLICKED(IDC_PLACEONMAP, OnPlaceonmap)
	ON_BN_CLICKED(IDC_CLONE, OnClone)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTriggerEditorDlg 

void CTriggerEditorDlg::OnOK() 
{
	
}

void CTriggerEditorDlg::OnCancel() 
{
	// TODO: Zusätzlichen Bereinigungscode hier einfügen
	
	CDialog::OnCancel();
}

BOOL CTriggerEditorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_tab.InsertItem(0, TranslateStringACP("Trigger options"));
	m_tab.InsertItem(1, TranslateStringACP("Events"));
	m_tab.InsertItem(2, TranslateStringACP("Actions"));

	m_TriggerOptions.Create(CTriggerOptionsDlg::IDD, &m_tab);
	RECT r;
	m_tab.GetItemRect(0, &r);
	int top=m_tab.GetRowCount()*r.bottom+5;
	m_tab.GetClientRect(&r);
	int bottom=r.bottom-top-3;
	m_TriggerOptions.SetWindowPos(NULL, 10, top, r.right-20, bottom, SWP_NOZORDER);
	m_TriggerOptions.ShowWindow(SW_SHOW);

	m_TriggerEvents.Create(CTriggerEventsDlg::IDD, &m_tab);
	m_TriggerEvents.SetWindowPos(NULL, 10, top, r.right-20, bottom, SWP_NOZORDER);

	m_TriggerActions.Create(CTriggerActionsDlg::IDD, &m_tab);
	m_TriggerActions.SetWindowPos(NULL, 10, top, r.right-20, bottom, SWP_NOZORDER);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CTriggerEditorDlg::UpdateDialog()
{
	int i;
	CIniFile& ini=Map->GetIniFile();
	
	int curSel=m_Trigger.GetCurSel();
	int curData=-1;
	if(curSel!=-1)
		curData=m_Trigger.GetItemData(curSel);

	
	while(m_Trigger.DeleteString(0)!=CB_ERR);
	m_Trigger.SetWindowText("");

	for(i=0;i<ini.sections["Triggers"].values.size();i++)
	{
		CString type;
		CString s;
		type=*ini.sections["Triggers"].GetValueName(i);
		
		s=GetParam(ini.sections["Triggers"].values[type], 2);

		if(ini.sections["Triggers"].values[type].GetLength()==0) continue;

		int id=m_Trigger.AddString(s);
		m_Trigger.SetItemData(id, i);		
	}

	if(curData<0) curData=0;
	if(curData>=m_Trigger.GetCount()) curData=m_Trigger.GetCount()-1;
	
	if(ini.sections["Triggers"].values.size()) m_Trigger.SetCurSel(0);

	for(i=0;i<m_Trigger.GetCount();i++)
		if(m_Trigger.GetItemData(i)==curData)
		{
			m_Trigger.SetCurSel(i);
			break;
		}
	
	OnSelchangeTrigger();
}

void CTriggerEditorDlg::OnNewtrigger() 
{
	CIniFile& ini=Map->GetIniFile();

	CString ID_T=GetFreeID();
	ini.sections["Triggers"].values[ID_T]=Map->GetHouseID(0, TRUE)+",<none>,New trigger,0,1,1,1,0";
	ini.sections["Events"].values[ID_T]="0";
	ini.sections["Actions"].values[ID_T]="0";

	//if(MessageBox("Trigger created. If you want to create a simple tag now, press Yes. The tag will be called ""New tag"", you should name it like the trigger (after you have set up the trigger).","Trigger created",MB_YESNO))
	{
		CString ID_TAG=GetFreeID();
		ini.sections["Tags"].values[ID_TAG]="0,New tag,";
		ini.sections["Tags"].values[ID_TAG]+=ID_T;
	}

	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);

	int i;
	for(i=0;i<m_Trigger.GetCount();i++)
	{
		if(m_Trigger.GetItemData(i)==ini.sections["Triggers"].FindName(ID_T))
			m_Trigger.SetCurSel(i);
	}
	OnSelchangeTrigger();
	
}

void CTriggerEditorDlg::OnDeletetrigger() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int curtrig=m_Trigger.GetItemData(sel);

	int res=MessageBox("If you want to delete all attached tags, too, press 'Yes'.\nIf you don´t want to delete these tags, press 'No'.\nIf you want to cancel deletion of the trigger, press 'Cancel'.\n\nNote: CellTags will never be deleted using this function","Delete trigger", MB_YESNOCANCEL);
	if(res==IDCANCEL) return;
	
	CString trigger=*ini.sections["Triggers"].GetValueName(curtrig);

	if(res==IDYES)
	{
		int i;
		for(i=0;i<ini.sections["Tags"].values.size();i++)
		{
			CString type=*ini.sections["Tags"].GetValueName(i);

			CString attTrigg=GetParam(ini.sections["Tags"].values[type], 2);

			if(trigger==attTrigg)
			{
				ini.sections["Tags"].values.erase(type);
			}
		}	

	}	

	ini.sections["Triggers"].values.erase((LPCTSTR)trigger);
	ini.sections["Events"].values.erase((LPCTSTR)trigger);
	ini.sections["Actions"].values.erase((LPCTSTR)trigger);

	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);

}



void CTriggerEditorDlg::OnSelchangeTrigger() 
{
	OnEditchangeTrigger();	
}

void CTriggerEditorDlg::OnEditchangeTrigger() 
{
	CIniFile& ini=Map->GetIniFile();

	int curSel=m_Trigger.GetCurSel();
	if(curSel<0) 
	{
		m_TriggerOptions.m_currentTrigger="";
		if(m_TriggerOptions.m_hWnd) m_TriggerOptions.UpdateDialog();	
		m_TriggerEvents.m_currentTrigger="";
		if(m_TriggerEvents.m_hWnd) m_TriggerEvents.UpdateDialog();
		m_TriggerActions.m_currentTrigger="";
		if(m_TriggerActions.m_hWnd) m_TriggerActions.UpdateDialog();		
		return;
	}

	int curInd=m_Trigger.GetItemData(curSel);

	CString Trigger=*ini.sections["Triggers"].GetValueName(curInd);

	m_TriggerOptions.m_currentTrigger=Trigger;
	if(m_TriggerOptions.m_hWnd) m_TriggerOptions.UpdateDialog();	
	m_TriggerEvents.m_currentTrigger=Trigger;
	if(m_TriggerEvents.m_hWnd) m_TriggerEvents.UpdateDialog();
	m_TriggerActions.m_currentTrigger=Trigger;
	if(m_TriggerActions.m_hWnd) m_TriggerActions.UpdateDialog();
}

void CTriggerEditorDlg::OnSelchangeTriggertab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_TriggerEvents.ShowWindow(SW_HIDE);
	m_TriggerOptions.ShowWindow(SW_HIDE);
	m_TriggerActions.ShowWindow(SW_HIDE);

	switch(m_tab.GetCurSel())
	{
	case 0:
		m_TriggerOptions.ShowWindow(SW_SHOW);
		m_TriggerOptions.UpdateDialog();
		break;
	case 1:
		m_TriggerEvents.ShowWindow(SW_SHOW);
		m_TriggerEvents.UpdateDialog();
		break;
	case 2:
		m_TriggerActions.ShowWindow(SW_SHOW);
		m_TriggerActions.UpdateData();
		break;
	}
	
	*pResult = 0;
}

void CTriggerEditorDlg::OnPlaceonmap() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int curtrig=m_Trigger.GetItemData(sel);

	CString trigger=*ini.sections["Triggers"].GetValueName(curtrig);

	int i;
	CString tag;
	for(i=0;i<ini.sections["Tags"].values.size();i++)
	{
		CString type=*ini.sections["Tags"].GetValueName(i);

		CString attTrigg=GetParam(ini.sections["Tags"].values[type], 2);

		if(trigger==attTrigg)
		{
			tag=type;
			break;
		}
	}	


	AD.mode=ACTIONMODE_CELLTAG;
	AD.type=4;
	AD.data_s=tag;
}

void CTriggerEditorDlg::OnClone() 
{
	CIniFile& ini=Map->GetIniFile();

	int sel=m_Trigger.GetCurSel();
	if(sel<0) return;
	int curtrig=m_Trigger.GetItemData(sel);

	CString trigger=*ini.sections["Triggers"].GetValueName(curtrig);

	CString ID_T=GetFreeID();
	ini.sections["Triggers"].values[ID_T]=ini.sections["Triggers"].values[trigger];
	ini.sections["Events"].values[ID_T]=ini.sections["Events"].values[trigger];
	ini.sections["Actions"].values[ID_T]=ini.sections["Actions"].values[trigger];	

	ini.sections["Triggers"].values[ID_T]=SetParam(ini.sections["Triggers"].values[ID_T], 2, GetParam(ini.sections["Triggers"].values[ID_T], 2) + " Clone");

	{
		CString ID_TAG=GetFreeID();
		ini.sections["Tags"].values[ID_TAG]=(CString)"0,"+GetParam(ini.sections["Triggers"].values[ID_T], 2)+",";
		ini.sections["Tags"].values[ID_TAG]+=ID_T;
	}

	((CFinalSunDlg*)theApp.m_pMainWnd)->UpdateDialogs(TRUE);

	int i;
	for(i=0;i<m_Trigger.GetCount();i++)
	{
		if(m_Trigger.GetItemData(i)==ini.sections["Triggers"].FindName(ID_T))
			m_Trigger.SetCurSel(i);
	}
	OnSelchangeTrigger();

}

void CTriggerEditorDlg::Clear()
{

}
