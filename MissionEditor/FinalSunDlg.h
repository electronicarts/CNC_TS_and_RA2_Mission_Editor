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
// Header-Datei
//

#if !defined(AFX_TIBERIANSUNMISSIONEDITORDLG_H__9F773424_63BB_11D3_99E0_C30F10710B17__INCLUDED_)
#define AFX_TIBERIANSUNMISSIONEDITORDLG_H__9F773424_63BB_11D3_99E0_C30F10710B17__INCLUDED_

#include "ScriptTypes.h"
#include "Triggers.h"
#include "Houses.h"
#include "Basic.h"
#include "all.h"
#include "mapd.h"
#include "Lighting.h"
#include "SpecialFlags.h"
#include "IsoView.h"	
#include "TeamTypes.h"
#include "TaskForce.h"
#include "tags.h"
#include "myviewframe.h"
#include "AiTriggerTypesEnable.h"
#include "AITriggerTypes.h"
#include "resource.h"
#include "SingleplayerSettings.h"	// Hinzugefügt von der Klassenansicht
#include "loading.h"
#include "TileSetBrowserFrame.h"	// Hinzugefügt von der Klassenansicht
#include "ToolSettingsBar.h"
#include "TriggerEditorDlg.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLoading;

/////////////////////////////////////////////////////////////////////////////
// CFinalSunDlg Dialogfeld

class CFinalSunDlg : public CDialog
{
// Konstruktion
public:
	void OpenMap(LPCSTR lpFilename);
	void InsertPrevFile(CString lpFilename);
	void CheckAvail(CCmdUI* pCmdUI);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CTileSetBrowserFrame m_TerrainDlg;
	void RecalcLayout();
	CToolBar m_clifftoolbar;
	CToolBar m_terraintoolbar;
	CToolBar m_maintoolbar;
	CToolSettingsBar m_settingsbar; 
	CReBarCtrl m_bar;
	void HideAllDialogs();
	CSingleplayerSettings m_singleplayersettings;
	void UpdateStrings();
	CAITriggerTypes m_aitriggertypes;
	CAiTriggerTypesEnable m_aitriggertypesenable;
	void SetText(const char* text);
	void SaveMap(CString FileName);
	void SetReady();
	CScriptTypes m_Scripttypes;
	CTriggers m_triggers;
	CTriggerEditorDlg m_triggereditor;
	CTags m_tags;
	CTaskForce m_taskforces;
	CTeamTypes m_teamtypes;

	CHouses m_houses;

	CSplitterWnd m_splitter_view;
	CMyViewFrame m_view;
	CSpecialFlags m_specialflags;
	CLighting m_lighting;
	void UpdateDialogs(BOOL bOnlyMissionControl=FALSE, BOOL bNoRepos=FALSE);
	CAll m_all;
	CBasic m_basic;
	CMapD m_map;	
	CIniFile m_PKTHeader;
	CFinalSunDlg(CWnd* pParent = NULL);	// Standard-Konstruktor

// Dialogfelddaten
	//{{AFX_DATA(CFinalSunDlg)
	enum { IDD = IDD_FINALSUN_DIALOG };
	CTabCtrl	m_TabCtrl;
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CFinalSunDlg)
	public:
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	HICON m_hIcon;

	// Generierte Message-Map-Funktionen
	//{{AFX_MSG(CFinalSunDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnFileQuit();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnOptionsTiberiansunoptions();
	afx_msg void OnFileOpenmap();
	afx_msg void OnFileSaveas();
	afx_msg void OnOptionsExportrulesini();
	afx_msg void OnHelpInfo();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnFileSave();
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnFileRuntiberiansun();
	afx_msg void OnFileImportmod();
	afx_msg void OnDebugExportmappacknosections();
	afx_msg void OnDebugExportmappack();
	afx_msg void OnFileNew();
	afx_msg void OnHelpTipoftheday();
	afx_msg void OnOptionsSimpleview();
	afx_msg void OnOptionsShowminimap();
	afx_msg void OnFileValidatemap();
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	afx_msg void OnEditBasicsettings();
	afx_msg void OnEditSingleplayersettings();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditHouses();
	afx_msg void OnEditAitriggers();
	afx_msg void OnEditAitriggerenabling();
	afx_msg void OnEditIniediting();
	afx_msg void OnEditLighting();
	afx_msg void OnEditMap();
	afx_msg void OnEditScripts();
	afx_msg void OnEditSpecialflags();
	afx_msg void OnEditTags();
	afx_msg void OnEditTaskforces();
	afx_msg void OnEditTeams();
	afx_msg void OnEditTriggers();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnTerrainHeightenground();
	afx_msg void OnTerrainLowerground();
	afx_msg void OnTerrainLowertile();
	afx_msg void OnTerrainRaisetile();
	afx_msg void OnTerrainFlatten();
	afx_msg void OnTerrainCloak();
	afx_msg void OnTerrainShoweverytile();
	afx_msg void OnTerrainShowallfields();
	afx_msg void OnTerrainHidefield();
	afx_msg void OnEditTriggereditor();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMaptoolsChangemapheight();
	afx_msg void OnEditGlobalvariables();
	afx_msg void OnEditUndo();
	afx_msg void OnHelpManual();
	afx_msg void OnMaptoolsAutolevel();
	afx_msg void OnUpdateEditAitriggerenabling(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditAitriggers(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditBasicsettings(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditGlobalvariables(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditHouses(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditIniediting(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditLighting(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditMap(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditScripts(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditSingleplayersettings(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditSpecialflags(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditTags(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditTaskforces(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditTeams(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditTriggereditor(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditTriggers(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveas(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileValidatemap(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMaptoolsAutolevel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMaptoolsChangemapheight(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOptionsShowminimap(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTerrainCloak(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTerrainFlatten(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTerrainHeightenground(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTerrainHidefield(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTerrainLowerground(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTerrainLowertile(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTerrainRaisetile(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTerrainShowallfields(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTerrainShoweverytile(CCmdUI* pCmdUI);
	afx_msg void OnMaptoolsFrontcliff();
	afx_msg void OnMaptoolsBackcliff();
	afx_msg void OnEditRedo();
	afx_msg void OnMaptoolsAutocreateshores();
	afx_msg void OnOptionsDisableautoshore();
	afx_msg void OnOptionsDisableautolat();
	afx_msg void OnEditPaste();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCopywholemap();
	afx_msg void OnEditPastewholemap();
	afx_msg void OnMarblemadness();
	afx_msg void OnOptionsSounds();
	afx_msg void OnUpdateOptionsSounds(CCmdUI* pCmdUI);
	afx_msg void OnOptionsDisableslopecorrection();
	afx_msg void OnOptionsShowbuildingoutline();
	afx_msg void OnFileFile1();
	afx_msg void OnFileFile2();
	afx_msg void OnFileFile3();
	afx_msg void OnFileFile4();
	afx_msg void OnMaptoolsSearchwaypoint();
	afx_msg void OnMaptoolsToolscripts();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UnloadAll();
	
	HCURSOR m_hGameCursor;
	
public:
	afx_msg void OnHelpShowlogs();
	afx_msg void OnOptionsSmoothzoom();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnOptionsUsedefaultmousecursor();
};

extern HCURSOR m_hArrowCursor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_TIBERIANSUNMISSIONEDITORDLG_H__9F773424_63BB_11D3_99E0_C30F10710B17__INCLUDED_)
