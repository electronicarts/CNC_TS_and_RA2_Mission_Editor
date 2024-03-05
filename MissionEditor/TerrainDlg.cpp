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

// TerrainDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "TerrainDlg.h"
#include "TileSetBrowserFrame.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"
#include <string>

extern ACTIONDATA AD;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTerrainDlg 


CTerrainDlg::CTerrainDlg(CWnd* pParent /*=NULL*/)
	: CDialogBar()
{
	//{{AFX_DATA_INIT(CTerrainDlg)
	//}}AFX_DATA_INIT
}


void CTerrainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTerrainDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTerrainDlg, CDialogBar)
	//{{AFX_MSG_MAP(CTerrainDlg)
	ON_CBN_SELCHANGE(IDC_TILESET, OnSelchangeTileset)
	ON_CBN_SELCHANGE(IDC_OVERLAY, OnSelchangeOverlay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTerrainDlg 

void CTerrainDlg::OnOK()
{
	// stub
}

void CTerrainDlg::OnCancel()
{
	DestroyWindow();
}

BOOL CTerrainDlg::OnInitDialog()
{
	//CDialogBar::OnInitDialog();

	return FALSE;
}

void CTerrainDlg::PostNcDestroy()
{
	//delete this;
	//CDialog::PostNcDestroy();
}

void CTerrainDlg::OnSelchangeTileset()
{
	//while(m_Type.DeleteString(0)!=CB_ERR);

	CString currentTileSet;
	CComboBox* TileSet;
	TileSet = (CComboBox*)GetDlgItem(IDC_TILESET);
	TileSet->GetLBText(TileSet->GetCurSel(), currentTileSet);

	TruncSpace(currentTileSet);

	((CTileSetBrowserFrame*)GetParentFrame())->m_view.SetTileSet(atoi(currentTileSet));
}



BOOL CTerrainDlg::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{

	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);


}

// needed to find out if pic exists
extern PICDATA* ovrlpics[0xFF][max_ovrl_img];

void CTerrainDlg::Update()
{
	CComboBox* TileSet;
	TileSet = (CComboBox*)GetDlgItem(IDC_TILESET);

	while (TileSet->DeleteString(0) != CB_ERR);

	if (tiles)
	{
		int i;
		int tilecount = 0;
		for (i = 0;i < 10000;i++)
		{
			CString tset;
			char c[50];
			itoa(i, c, 10);
			int e;
			for (e = 0;e < 4 - strlen(c);e++)
				tset += "0";
			tset += c;
			CString sec = "TileSet";
			sec += tset;

			if (tiles->sections.find(sec) == tiles->sections.end())
				break;
			if (atoi(tiles->sections[sec].values["TilesInSet"]) == 0)
				continue;

			CString string;
			string = tset;
			string += " (";
			string += TranslateStringACP(tiles->sections[sec].values["SetName"]);
			string += ")";

			BOOL bForced = FALSE;
			BOOL bForcedNot = FALSE;


			// force yes
			CString datsec = (CString)"UseSet" + Map->GetTheater();
			auto tsetc = CString(std::to_string(atoi(tset)).c_str());

			if (g_data.sections[datsec].FindValue(tsetc) >= 0)
				bForced = TRUE;

			// force no
			datsec = (CString)"IgnoreSet" + Map->GetTheater();
			if (g_data.sections[datsec].FindValue(tsetc) >= 0)
				bForcedNot = TRUE;


			if (bForced || (!bForcedNot && (*tiledata)[tilecount].bAllowToPlace && !(*tiledata)[tilecount].bMarbleMadness))
				TileSet->SetItemData(TileSet->AddString(string), i);

			tilecount += atoi(tiles->sections[sec].values["TilesInSet"]);
		}

		TileSet->SetCurSel(0);
		OnSelchangeTileset();
	}

	CComboBox* Overlays;
	Overlays = (CComboBox*)GetDlgItem(IDC_OVERLAY);

	while (Overlays->DeleteString(0) != CB_ERR);

	int i;

	int e = 0;
	for (i = 0;i < rules.sections["OverlayTypes"].values.size();i++)
	{
		CString id = *rules.sections["OverlayTypes"].GetValue(i);
		id.TrimLeft();
		id.TrimRight();

		if (id.GetLength() > 0)
		{

			if (rules.sections.find(id) != rules.sections.end() && rules.sections[id].FindName("Name") >= 0)
			{
				int p;
				BOOL bListIt = TRUE;
				for (p = 0;p < max_ovrl_img;p++)
					if (ovrlpics[i][p] != NULL && ovrlpics[i][p]->pic != NULL)
						bListIt = TRUE;

#ifdef RA2_MODE
				if ((i >= 39 && i <= 60) || (i >= 180 && i <= 201) || i == 239 || i == 178 || i == 167 || i == 126
					|| (i >= 122 && i <= 125))
					bListIt = FALSE;
#endif

				if (bListIt)
				{
					CString str;
					str = TranslateStringACP(rules.sections[(*rules.sections["OverlayTypes"].GetValue(i))].values["Name"]);
					Overlays->SetItemData(Overlays->AddString(str), e);
				}
			}
			e++;
		}
	}
}


DWORD CTerrainDlg::GetTileID(DWORD dwTileSet, int iTile)
{
	int i, e;
	DWORD tilecount = 0;
	for (i = 0;i < 10000;i++)
	{
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for (e = 0;e < 4 - strlen(c);e++)
			tset += "0";
		tset += c;
		CString sec = "TileSet";
		sec += tset;

		if (tiles->sections.find(sec) == tiles->sections.end())
			return 0xFFFFFFFF;


		for (e = 0;e < atoi(tiles->sections[sec].values["TilesInSet"]);e++)
		{
			if (i == dwTileSet && e == iTile)
				return tilecount;
			tilecount++;

		}


	}

	return tilecount;

}


void CTerrainDlg::OnSelchangeOverlay()
{
	CComboBox* Overlay;
	Overlay = (CComboBox*)GetDlgItem(IDC_OVERLAY);
	//TileSet->GetLBText(TileSet->GetCurSel(), currentTileSet);
	int n = Overlay->GetCurSel();

	if (n < 0) return;

	int sel = Overlay->GetItemData(n);

	((CTileSetBrowserFrame*)GetParentFrame())->m_view.SetOverlay(sel);
}
