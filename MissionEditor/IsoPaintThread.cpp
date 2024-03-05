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

// IsoPaintThread.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "IsoPaintThread.h"
#include "variables.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIsoPaintThread

IMPLEMENT_DYNCREATE(CIsoPaintThread, CWinThread)

CIsoPaintThread::CIsoPaintThread()
{
}

CIsoPaintThread::~CIsoPaintThread()
{
}

BOOL CIsoPaintThread::InitInstance()
{
	// ZU ERLEDIGEN:  Initialisierungen für jeden Thread hier durchführen

	while(TRUE)
	{
		// this worker thread just updates the main window
		((CFinalSunDlg*)theApp.GetMainWnd())->m_view.m_isoview->UpdateWindow();
	}

	return TRUE;
}

int CIsoPaintThread::ExitInstance()
{
	// ZU ERLEDIGEN:  Bereinigungen für jeden Thread hier durchführen
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CIsoPaintThread, CWinThread)
	//{{AFX_MSG_MAP(CIsoPaintThread)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CIsoPaintThread 
