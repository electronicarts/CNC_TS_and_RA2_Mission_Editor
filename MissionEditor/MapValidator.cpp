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

// MapValidator.cpp: implementation file
//

#include "stdafx.h"
#include "finalsun.h"
#include "MapValidator.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"
#include <algorithm>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// dialog field CMapValidator 


CMapValidator::CMapValidator(CWnd* pParent /*=NULL*/)
	: CDialog(CMapValidator::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapValidator)
		
	//}}AFX_DATA_INIT
}


void CMapValidator::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapValidator)
	DDX_Control(pDX, IDC_MAPPROBLEMS, m_MapProblemList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapValidator, CDialog)
	//{{AFX_MSG_MAP(CMapValidator)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// message handlers CMapValidator 

void CMapValidator::UpdateStrings()
{
	SetWindowText(GetLanguageStringACP("MapValidatorCap"));
	SetDlgItemText(IDC_LPROBLEMSFOUND, GetLanguageStringACP("MapValidatorProblemsFound"));
	SetDlgItemText(IDOK, GetLanguageStringACP("OK"));
	SetDlgItemText(IDCANCEL, GetLanguageStringACP("Cancel"));
}

BOOL CMapValidator::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateStrings();

	m_ProblemImages.Create(16,16,ILC_COLOR8 | ILC_MASK,0, 50 );
	
	CBitmap bmpIcons;
	bmpIcons.LoadBitmap(IDB_MV_ICONS);

	m_ProblemImages.Add(&bmpIcons, RGB(255,255,255));
	m_MapProblemList.SetImageList(&m_ProblemImages,LVSIL_SMALL);
	m_MapProblemList.SetImageList(&m_ProblemImages,LVSIL_NORMAL);

	auto col = m_MapProblemList.InsertColumn(0, "");
	//RECT r;
	//m_MapProblemList.GetClientRect(&r);
	


	BOOL bSaveAble=CheckMap();

	m_MapProblemList.SetColumnWidth(col, LVSCW_AUTOSIZE);


	if(bSaveAble==FALSE) GetDlgItem(IDOK)->EnableWindow(FALSE);
	
	return TRUE;  
}

void AddItemWithNewLine(CListCtrl& ctrl, CString s, int image)
{
	s.Replace("\\n", "\n");
	int n = ctrl.GetItemCount();
	for (const auto& line : Split(s, '\n'))
	{
		ctrl.InsertItem(n++, line, image);
		image = 2;
	}
}

/*
CMapValidator::CheckMap();

Returns TRUE, when the map is saveable, but may not work.
If FALSE is returned, you should not anymore allow to save the map.
*/
BOOL CMapValidator::CheckMap()
{
	BOOL bAllow=TRUE;

	// now check the map

	Map->UpdateIniFile(MAPDATA_UPDATE_TO_INI);
	CIniFile& ini=Map->GetIniFile();

	if(ini.sections.find("Map")==ini.sections.end() )
	{
		bAllow=FALSE;
		AddItemWithNewLine(m_MapProblemList, GetLanguageStringACP("MV_NoMap"), 0);
	}
	if(ini.sections.find("Basic")==ini.sections.end() || ini.sections["Basic"].values.size()==0)
	{
		bAllow=FALSE;
		AddItemWithNewLine(m_MapProblemList, GetLanguageStringACP("MV_NoBasic"), 0);
		
	}
	else
	{
		if(ini.sections["Basic"].values["Name"].GetLength()==0)
			AddItemWithNewLine(m_MapProblemList, GetLanguageStringACP("MV_NoName"), 1);

		if(ini.sections["Basic"].values.find("Player")==ini.sections["Basic"].values.end())
		{
#ifdef TS_MODE
			if(ini.sections.find(MAPHOUSES)!=ini.sections.end() && ini.sections["Houses"].values.size()>0)
			{
				AddItemWithNewLine(m_MapProblemList, GetLanguageStringACP("MV_HousesButNoPlayer"), 1);
				AddItemWithNewLine(m_MapProblemList, GetLanguageStringACP("MV_HousesInMultiplayer"), 1);
			}
#endif
		}
	
		if(ini.sections.find(MAPHOUSES)==ini.sections.end())
		{
			int d=Map->GetWaypointCount();			
			int below8found=0;
			int i;
			for(i=0;i<d;i++)
			{
				DWORD pos;
				CString id;
				Map->GetWaypointData(i, &id, &pos);
				if(atoi(id)<8)
				{
					below8found++;
				}
			}

			if(below8found<8)
			{
				if(isFalse(ini.sections["Basic"].values["Official"]))
				{
					AddItemWithNewLine(m_MapProblemList, GetLanguageStringACP("MV_Not8Waypoints"), 1);
				}

				if(below8found<2)
					AddItemWithNewLine(m_MapProblemList, GetLanguageStringACP("MV_HousesNoWaypoints"), 1);
			}
			
#ifdef RA2_MODE
			if(isTrue(ini.sections["Basic"].values["Official"]))
				AddItemWithNewLine(m_MapProblemList, GetLanguageStringACP("MV_OfficialYes"), 1);
#endif

		}
		
		int i;
		for(i=0;i<ini.sections["Tags"].values.size();i++)
		{
			CString trigger=GetParam(*ini.sections["Tags"].GetValue(i),2);
			if(ini.sections["Triggers"].FindName(trigger)<0)
			{
				CString error;
				error=GetLanguageStringACP("MV_TriggerMissing");
				error=TranslateStringVariables(1, error, trigger);
				error=TranslateStringVariables(2, error, "Tag");
				error=TranslateStringVariables(3, error, *ini.sections["Tags"].GetValueName(i));
				AddItemWithNewLine(m_MapProblemList, error, 1);
			}
		}
		// repair triggers
		for(i=0;i<ini.sections["Triggers"].values.size();i++)
		{
			RepairTrigger(ini.sections["Triggers"].values[*ini.sections["Triggers"].GetValueName(i)]);
		}
		for(i=0;i<ini.sections["Triggers"].values.size();i++)
		{
			CString trigger=GetParam(*ini.sections["Triggers"].GetValue(i),1);
			if(ini.sections["Triggers"].FindName(trigger)<0 && trigger!="<none>")
			{
				CString error;
				error=GetLanguageStringACP("MV_TriggerMissing");
				error=TranslateStringVariables(1, error, trigger);
				error=TranslateStringVariables(2, error, "Trigger");
				error=TranslateStringVariables(3, error, *ini.sections["Triggers"].GetValueName(i));
				AddItemWithNewLine(m_MapProblemList, error, 1);
			}
		}

		for(i=0;i<ini.sections["TeamTypes"].values.size();i++)
		{
			CIniFileSection& sec=ini.sections[*ini.sections["TeamTypes"].GetValue(i)];
			CString taskforce=sec.values["TaskForce"];
			if(taskforce.GetLength()>0 && ini.sections["TaskForces"].FindValue(taskforce)<0)
			{
				CString error;
				error=GetLanguageStringACP("MV_TaskForceMissing");
				error=TranslateStringVariables(1, error, taskforce);
				error=TranslateStringVariables(2, error, *ini.sections["TeamTypes"].GetValue(i));
				AddItemWithNewLine(m_MapProblemList, error, 1);
			}
		}
		for(i=0;i<ini.sections["TeamTypes"].values.size();i++)
		{
			CIniFileSection& sec=ini.sections[*ini.sections["TeamTypes"].GetValue(i)];
			CString scripttype=sec.values["Script"];
			if(scripttype.GetLength()>0 && ini.sections["ScriptTypes"].FindValue(scripttype)<0)
			{
				CString error;
				error=GetLanguageStringACP("MV_ScripttypeMissing");
				error=TranslateStringVariables(1, error, scripttype);
				error=TranslateStringVariables(2, error, *ini.sections["TeamTypes"].GetValue(i));
				AddItemWithNewLine(m_MapProblemList, error, 1);
			}
		}
		for(i=0;i<ini.sections["TeamTypes"].values.size();i++)
		{
			CIniFileSection& sec=ini.sections[*ini.sections["TeamTypes"].GetValue(i)];
			if(sec.FindName("Tag")>=0)
			{
				CString tag=sec.values["Tag"];
				if(ini.sections["Tags"].FindName(tag)<0)
				{
					CString error;
					error=GetLanguageStringACP("MV_TagMissing");
					error=TranslateStringVariables(1, error, tag);
					error=TranslateStringVariables(2, error, "Teamtype");
					error=TranslateStringVariables(3, error, *ini.sections["TeamTypes"].GetValue(i));
					AddItemWithNewLine(m_MapProblemList, error, 1);
				}
			}
		}
		for(i=0;i<Map->GetCelltagCount();i++)
		{		
			CString tag;
			DWORD pos;
			Map->GetCelltagData(i, &tag, &pos);
			int x=pos%Map->GetIsoSize();
			int y=pos/Map->GetIsoSize();
			char cx[50];
			char cy[50];
			itoa(x, cx, 10);
			itoa(y, cy, 10);
			CString p=cx;
			p+="/";
			p+=cy;
			if(ini.sections["Tags"].FindName(tag)<0)
			{
				CString error;
				error=GetLanguageStringACP("MV_TagMissing");
				error=TranslateStringVariables(1, error, tag);
				error=TranslateStringVariables(2, error, "Celltag");
				error=TranslateStringVariables(3, error, p);
				AddItemWithNewLine(m_MapProblemList, error, 1);
			}
		}

	}

	const auto& tubes = Map->GetTubes();
	for (auto& tube : tubes)
	{
		auto n_reverse = std::count_if(tubes.begin(), tubes.end(), [&tube](const auto& other) { return tube->isCounterpart(*other); });
		if (n_reverse == 0)
		{
			CString error = TranslateTubeString(
				GetLanguageStringACP("MV_TubeCounterpartMissing"),
				*tube,
				n_reverse
			);
			AddItemWithNewLine(m_MapProblemList, error, 0);
		}
		auto n_same_start = std::count_if(tubes.begin(), tubes.end(), [&tube](const auto& other) { return tube->getStartCoords() == other->getStartCoords() && *tube != *other; });
		if (n_same_start)
		{
			CString error = TranslateTubeString(
				GetLanguageStringACP("MV_TubeStartNotUnique"),
				*tube,
				n_same_start
			);
			AddItemWithNewLine(m_MapProblemList, error, 0);
		}
		auto n_same_end = std::count_if(tubes.begin(), tubes.end(), [&tube](const auto& other) { return tube->getEndCoords() == other->getEndCoords() && *tube != *other; });
		if (n_same_end)
		{
			CString error = TranslateTubeString(
				GetLanguageStringACP("MV_TubeEndNotUnique"),
				*tube,
				n_same_end
			);
			AddItemWithNewLine(m_MapProblemList, error, 0);
		}
		auto n_invalid_counterpart_end = std::count_if(tubes.begin(), tubes.end(), [&tube](const auto& other) { return tube->getEndCoords() == other->getStartCoords() && tube->getStartCoords() != other->getEndCoords(); });
		if (n_invalid_counterpart_end)
		{
			CString error = TranslateTubeString(
				GetLanguageStringACP("MV_TubeInvalidCounterpartEnd"),
				*tube,
				n_invalid_counterpart_end
			);
			AddItemWithNewLine(m_MapProblemList, error, 0);
		}
		auto n_invalid_counterpart_start = std::count_if(tubes.begin(), tubes.end(), [&tube](const auto& other) { return tube->getStartCoords() == other->getEndCoords() && tube->getEndCoords() != other->getStartCoords(); });
		if (n_invalid_counterpart_start)
		{
			CString error = TranslateTubeString(
				GetLanguageStringACP("MV_TubeInvalidCounterpartStart"),
				*tube,
				n_invalid_counterpart_start
			);
			AddItemWithNewLine(m_MapProblemList, error, 0);
		}
	}

	int i;
	BOOL bWaypBig=FALSE;
	for(i=0;i<Map->GetWaypointCount();i++)
	{
		DWORD pos;
		CString id;

		Map->GetWaypointData(i, &id, &pos);

		if(atoi(id)>99) bWaypBig=TRUE;
	}

	if(bWaypBig)
		AddItemWithNewLine(m_MapProblemList, GetLanguageStringACP("MV_>100Waypoint"), 1);
	
	if(Map->IsYRMap())
	{
		AddItemWithNewLine(m_MapProblemList, GetLanguageStringACP("NeedsYR"), 1);
	}

	return bAllow;	
}

CString TranslateTubeString(const char* error_, const CTube& tube, int count)
{
	CString error = error_;
	error = TranslateStringVariables(1, error, std::to_string(tube.getStartX()).c_str());
	error = TranslateStringVariables(2, error, std::to_string(tube.getStartY()).c_str());
	error = TranslateStringVariables(3, error, std::to_string(tube.getEndX()).c_str());
	error = TranslateStringVariables(4, error, std::to_string(tube.getEndY()).c_str());
	error = TranslateStringVariables(5, error, std::to_string(tube.getId()).c_str());
	error = TranslateStringVariables(6, error, std::to_string(count).c_str());
	return error;
}
