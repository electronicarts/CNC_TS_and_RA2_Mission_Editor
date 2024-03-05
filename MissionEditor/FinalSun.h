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
// Main header file
//

#if !defined(AFX_TIBERIANSUNMISSIONEDITOR_H__9F773422_63BB_11D3_99E0_C30F10710B17__INCLUDED_)
#define AFX_TIBERIANSUNMISSIONEDITOR_H__9F773422_63BB_11D3_99E0_C30F10710B17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "ddeml.h"
#include "loading.h"

struct OPTIONS{
	CString TSExe;
	CString LanguageName;
	CString prev_maps[4];
	bool bPreferLocalTheaterFiles = true;
	BOOL bFlat;
	BOOL bEasy;
	BOOL bMarbleMadness;
	BOOL bSupportMarbleMadness;
	BOOL bShowCells;
	BOOL bDoNotLoadBuildingGraphics;
	BOOL bDoNotLoadInfantryGraphics;
	BOOL bDoNotLoadAircraftGraphics;
	BOOL bDoNotLoadTreeGraphics;
	BOOL bDoNotLoadSnowGraphics;
	BOOL bDoNotLoadTemperateGraphics;
	BOOL bDoNotLoadVehicleGraphics;
	BOOL bDoNotLoadBMPs;
	BOOL bDoNotLoadOverlayGraphics;
	BOOL bDisableAutoShore;
	BOOL bDisableAutoLat;
	BOOL bNoSounds;
	BOOL bDisableSlopeCorrection;
	float fMiniMapScale = 2.0f;
	float fLoadScreenDelayInSeconds = 3.5f;
	bool bShowStats = false;
	bool bHighResUI = true;
	bool bVSync = false;
	std::vector<float> viewScaleSteps = { 0.75f, 0.5f, 0.25f };
	bool viewScaleUseSteps = true;
	float viewScaleSpeed = 15.0f;
	bool useDefaultMouseCursor = false;

public:
	BOOL bSearchLikeTS;
};


/////////////////////////////////////////////////////////////////////////////
// CFinalSunApp:
// Look at FinalSun.cpp for the implementation of this class
//

class CFinalSunApp : public CWinApp
{
public:
	DWORD pidInst;
	OPTIONS m_Options;
	CLoading* m_loading;
	CFinalSunApp();

	const std::string& getLogFileName() const
	{
		return m_u8LogFileName;
	}

	
// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CFinalSunApp)
	public:
	virtual BOOL InitInstance();
	virtual int Run();
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	virtual LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementierung

	//{{AFX_MSG(CFinalSunApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	public:
	UINT m_cf;
	HACCEL m_hAccel;
	void ParseCommandLine();
	void ShowTipAtStartup(void);
	void ShowTipOfTheDay(void);

private:
	std::string m_u8LogFileName;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_TIBERIANSUNMISSIONEDITOR_H__9F773422_63BB_11D3_99E0_C30F10710B17__INCLUDED_)
