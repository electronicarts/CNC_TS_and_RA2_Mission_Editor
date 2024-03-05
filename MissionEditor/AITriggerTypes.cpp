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

// AITriggerTypes.cpp: implementation file
//

#include "stdafx.h"
#include "FinalSun.h"
#include "AITriggerTypes.h"
#include "Mapdata.h"
#include "structs.h"
#include "variables.h"
#include "functions.h"
#include "inlines.h"


// AI trigger type enumeration
enum AITriggerEnum {
	AIT_NONE = -1,
	AIT_ENEMY_OWNS_X_COND_N = 0,
	AIT_HOUSE_OWNS_X_COND_N,
	AIT_POWER_YELLOW,
	AIT_POWER_RED,
	AIT_ENEMY_MONEY_COND_N,
	AIT_CURTAIN_NEAR_READY,
	AIT_SPHERE_NEAR_READY,
	AIT_NEUTRAL_OWNS_X_COND_N,
};


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// property page CAITriggerTypes 

IMPLEMENT_DYNCREATE(CAITriggerTypes, CDialog)

CAITriggerTypes::CAITriggerTypes() : CDialog(CAITriggerTypes::IDD)
{
	//{{AFX_DATA_INIT(CAITriggerTypes)
	m_Flag3 = _T("");
	m_Flag4 = _T("");
	m_Name = _T("");
	m_Flag6 = _T("");
	m_Flag7 = _T("");
	m_Flag8 = _T("");
	m_Flag9 = _T("");
	m_Enabled = FALSE;
	m_Condition = -1;
	m_Number = 0;
	m_Easy = FALSE;
	m_Medium = FALSE;
	m_Hard = FALSE;
	m_BaseDefense = FALSE;
	m_Skirmish = FALSE;
	m_Flag5 = _T("");
	m_MultiSide = _T("");
	//}}AFX_DATA_INIT
}

CAITriggerTypes::~CAITriggerTypes()
{
}

void CAITriggerTypes::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAITriggerTypes)
	DDX_Control(pDX, IDC_TEAMTYPE1, m_TeamType1);
	DDX_Control(pDX, IDC_UNITTYPE, m_UnitType);
	DDX_Control(pDX, IDC_TEAMTYPE2, m_TeamType2);
	DDX_Control(pDX, IDC_FLAG2, m_Flag2);
	DDX_Control(pDX, IDC_FLAG1, m_Flag1);
	DDX_Control(pDX, IDC_DATA, m_Data);
	DDX_Control(pDX, IDC_OWNER, m_Owner);
	DDX_Control(pDX, IDC_FLOAT3, m_Float3);
	DDX_Control(pDX, IDC_FLOAT2, m_Float2);
	DDX_Control(pDX, IDC_FLOAT1, m_Float1);
	DDX_Control(pDX, IDC_AITRIGGERTYPE, m_AITriggerType);
	DDX_Text(pDX, IDC_FLAG4, m_Flag4);
	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Check(pDX, IDC_ENABLED, m_Enabled);
	DDX_CBIndex(pDX, IDC_CONDITION, m_Condition);
	DDX_Text(pDX, IDC_NUMBER, m_Number);
	DDV_MinMaxInt(pDX, m_Number, 0, 256);
	DDX_Check(pDX, IDC_EASY, m_Easy);
	DDX_Check(pDX, IDC_MEDIUM, m_Medium);
	DDX_Check(pDX, IDC_HARD, m_Hard);
	DDX_Check(pDX, IDC_BASEDEFENSE, m_BaseDefense);
	DDX_Check(pDX, IDC_SKIRMISH, m_Skirmish);
	DDX_CBString(pDX, IDC_MULTISIDE, m_MultiSide);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAITriggerTypes, CDialog)
	//{{AFX_MSG_MAP(CAITriggerTypes)
	ON_CBN_SELCHANGE(IDC_AITRIGGERTYPE, OnSelchangeAitriggertype)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	ON_CBN_EDITCHANGE(IDC_OWNER, OnEditchangeOwner)
	ON_CBN_EDITCHANGE(IDC_TEAMTYPE1, OnEditchangeTeamtype1)
	ON_CBN_EDITCHANGE(IDC_TEAMTYPE2, OnEditchangeTeamtype2)
	ON_CBN_EDITCHANGE(IDC_FLAG1, OnEditchangeFlag1)
	ON_CBN_EDITCHANGE(IDC_FLAG2, OnEditchangeFlag2)
	ON_CBN_EDITCHANGE(IDC_UNITTYPE, OnEditchangeUnittype)
	ON_CBN_EDITCHANGE(IDC_DATA, OnEditchangeData)
	ON_EN_CHANGE(IDC_FLOAT1, OnChangeFloat1)
	ON_EN_CHANGE(IDC_FLOAT2, OnChangeFloat2)
	ON_EN_CHANGE(IDC_FLOAT3, OnChangeFloat3)
	ON_EN_CHANGE(IDC_FLAG4, OnChangeFlag4)

	ON_BN_CLICKED(IDC_ENABLED, OnEnabled)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_CBN_EDITCHANGE(IDC_AITRIGGERTYPE, OnEditchangeAitriggertype)
	ON_CBN_SELCHANGE(IDC_CONDITION, OnSelchangeCondition)
	ON_EN_CHANGE(IDC_NUMBER, OnChangeNumber)
	ON_BN_CLICKED(IDC_EASY, OnEasy)
	ON_BN_CLICKED(IDC_MEDIUM, OnMedium)
	ON_BN_CLICKED(IDC_HARD, OnHard)
	ON_BN_CLICKED(IDC_BASEDEFENSE, OnBasedefense)
	ON_BN_CLICKED(IDC_SKIRMISH, OnSkirmish)
	ON_CBN_EDITCHANGE(IDC_MULTISIDE, OnEditchangeMultiside)
	ON_CBN_SELCHANGE(IDC_MULTISIDE, OnSelchangeMultiside)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// message handlers CAITriggerTypes 

void ListObjects(CComboBox& cb)
{
	CComboBox& m_UnitType=cb;
	CIniFile& ini=Map->GetIniFile();

	int i;

	CString ss="InfantryTypes";
	for(i=0;i<rules.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*rules.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}
	for(i=0;i<ini.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*ini.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}

	ss="VehicleTypes";
	for(i=0;i<rules.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*rules.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}
	for(i=0;i<ini.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*ini.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}

	ss="AircraftTypes";
	for(i=0;i<rules.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*rules.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}
	for(i=0;i<ini.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*ini.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}

	ss="BuildingTypes";
	for(i=0;i<rules.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*rules.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}
	for(i=0;i<ini.sections[ss].values.size();i++)
	{
		CString type;
		CString s;
		type=*ini.sections[ss].GetValue(i);
		s=type;
		s+=" (";
		
		s+=Map->GetUnitName((char*)(LPCTSTR)type);

		s+=")";
		m_UnitType.AddString(s);
	}
}

void CAITriggerTypes::UpdateDialog()
{
	int sel=m_AITriggerType.GetCurSel();
	m_AITriggerType.SetRedraw(FALSE);

	while(m_AITriggerType.DeleteString(0)!=CB_ERR);

	ListHouses(m_Owner,FALSE,TRUE);
	m_Owner.AddString("<all>");
	ListTeamTypes(m_TeamType1, TRUE);
	ListTeamTypes(m_TeamType2, TRUE);

	int i;
	for(i=0;i<Map->GetAITriggerTypeCount();i++)
	{
		AITRIGGERTYPE aitt;
		Map->GetAITriggerType(i, &aitt);
		CString s=aitt.ID;
		s+=" (";
		s+=aitt.name;
		s+=")";
		m_AITriggerType.AddString(s);
	}

	m_AITriggerType.SetCurSel(0);
	if(sel!=-1)
		m_AITriggerType.SetCurSel(sel);






	OnSelchangeAitriggertype();

	m_AITriggerType.SetRedraw(TRUE);
	m_AITriggerType.RedrawWindow();
}


void CAITriggerTypes::OnSelchangeAitriggertype() 
{
	int sel=m_AITriggerType.GetCurSel();
	if(sel<0) return;

	CString aitrigger;
	m_AITriggerType.GetLBText(sel,aitrigger);
	TruncSpace(aitrigger);
	

	AITRIGGERTYPE aitt;
	Map->GetAITriggerType(Map->GetAITriggerTypeIndex(aitrigger), &aitt);

	m_Name=aitt.name;
	m_TeamType1.SetWindowText(aitt.teamtype1);
	m_Owner.SetWindowText(TranslateHouse(aitt.owner, TRUE));
	m_Flag1.SetWindowText(aitt.techlevel);
	//m_Flag2.SetWindowText(aitt.type);
	int type=atoi(aitt.type)+1;
	m_Flag2.SetCurSel(type);
	m_UnitType.SetWindowText(aitt.unittype);
	m_Data.SetWindowText(aitt.data);
	m_Float1.SetWindowText(aitt.float1);
	m_Float2.SetWindowText(aitt.float2);
	m_Float3.SetWindowText(aitt.float3);
	//m_Flag3=aitt.skirmish;
	m_Skirmish=isTrue(aitt.skirmish);
	m_Flag4=aitt.flag4;
	//m_Flag5=aitt.multihouse;
	m_MultiSide=aitt.multihouse;
	m_BaseDefense=isTrue(aitt.basedefense);
	m_TeamType2.SetWindowText(aitt.teamtype2);
	m_Easy=isTrue(aitt.easy);
	m_Medium=isTrue(aitt.medium);
	m_Hard=isTrue(aitt.hard);
	
	m_Enabled=FALSE;
	CIniFile& ini=Map->GetIniFile();
	if(ini.sections["AITriggerTypesEnable"].values.find((LPCTSTR)aitrigger)!=ini.sections["AITriggerTypesEnable"].values.end())
		if(stricmp(ini.sections["AITriggerTypesEnable"].values[(LPCTSTR)aitrigger], "yes")==NULL)
			m_Enabled=TRUE;

	AITrigInfo info;
	info=ConvertToAITrigInfoFromHex((char*)(LPCSTR)aitt.data);
	m_Condition=info.Condition;
	m_Number=info.Number;

	ListObjects(m_UnitType);
	
	UpdateData(FALSE);
}

void CAITriggerTypes::OnChangeName() 
{
	UpdateData();

	CString value;
	value=m_Name;

	if(value.GetLength()==0) value=" ";

	SetAITriggerParam(value, 0);

	UpdateDialog();
}

void CAITriggerTypes::OnEditchangeOwner() 
{
	CString value;
	m_Owner.GetWindowText(value);

	value=TranslateHouse(value);

	SetAITriggerParam(value, 2);
}

void CAITriggerTypes::OnEditchangeTeamtype1() 
{
	CString value;
	m_TeamType1.GetWindowText(value);
	TruncSpace(value);
	
	SetAITriggerParam(value, 1);
}

void CAITriggerTypes::OnEditchangeTeamtype2() 
{
	CString value;
	m_TeamType2.GetWindowText(value);
	TruncSpace(value);
	
	SetAITriggerParam(value, 14);
}

void CAITriggerTypes::OnEditchangeFlag1() 
{
	CString value;
	m_Flag1.GetWindowText(value);

	SetAITriggerParam(value, 3);
}

void CAITriggerTypes::OnEditchangeFlag2() 
{
	CString value;
	m_Flag2.GetWindowText(value);

	TruncSpace(value);

	SetAITriggerParam(value, 4);
}

void CAITriggerTypes::OnEditchangeUnittype() 
{
	CString value;
	m_UnitType.GetWindowText(value);
	TruncSpace(value);

	SetAITriggerParam(value, 5);
}

void CAITriggerTypes::OnEditchangeData() 
{
	/*
	This function should also valid the data!
	*/

	CString value;
	m_Data.GetWindowText(value);
	TruncSpace(value);

	// max 64 chars... min 64 chars too...
	((char*)(LPCTSTR)value)[64]=0;
	
	int ToInsert=64-value.GetLength();
	CString nulls('0',ToInsert);

	value.Insert(value.GetLength()-ToInsert, nulls);
		

	SetAITriggerParam(value, 6);


	int editsel=m_Data.GetEditSel();
	UpdateDialog();		
	m_Data.SetEditSel(editsel,editsel+1);
}

void CAITriggerTypes::OnChangeFloat1() 
{
	CString value;
	m_Float1.GetWindowText(value);

	SetAITriggerParam(value, 7);
}

void CAITriggerTypes::OnChangeFloat2() 
{
	CString value;
	m_Float2.GetWindowText(value);

	SetAITriggerParam(value, 8);
}

void CAITriggerTypes::OnChangeFloat3() 
{
	CString value;
	m_Float3.GetWindowText(value);

	SetAITriggerParam(value, 9);
}

void CAITriggerTypes::OnChangeFlag3() 
{
	

}

void CAITriggerTypes::OnChangeFlag4() 
{
	UpdateData();

	CString value;
	value=m_Flag4;

	SetAITriggerParam(value, 11);

}

void CAITriggerTypes::OnChangeFlag5() 
{
	

}

void CAITriggerTypes::OnChangeFlag6() 
{
	
}

void CAITriggerTypes::OnChangeFlag7() 
{
	
}

void CAITriggerTypes::OnChangeFlag8() 
{
	UpdateData();

	CString value;
	value=m_Flag8;

	SetAITriggerParam(value, 16);
}

void CAITriggerTypes::OnChangeFlag9() 
{
	UpdateData();

	CString value;
	value=m_Flag9;

	SetAITriggerParam(value, 17);
}

void CAITriggerTypes::OnEnabled() 
{
	// enable or disable trigger

	UpdateData();

	int sel=m_AITriggerType.GetCurSel();
	if(sel<0) return;

	CString aitrigger;
	m_AITriggerType.GetLBText(sel,aitrigger);
	TruncSpace(aitrigger);

	CIniFile& ini=Map->GetIniFile();

	if(m_Enabled)
	{
		// enable it
		ini.sections["AITriggerTypesEnable"].values[(LPCTSTR)aitrigger]="yes";
	}
	else
		ini.sections["AITriggerTypesEnable"].values.erase((LPCTSTR)aitrigger);
}

void CAITriggerTypes::SetAITriggerParam(const char *value, int param)
{
	int sel=m_AITriggerType.GetCurSel();
	if(sel<0) return;

	CString aitrigger;
	m_AITriggerType.GetLBText(sel,aitrigger);
	TruncSpace(aitrigger);

	CIniFile& ini=Map->GetIniFile();
	ini.sections["AITriggerTypes"].values[aitrigger]=SetParam(ini.sections["AITriggerTypes"].values[aitrigger],param,value);
}

void CAITriggerTypes::OnAdd() 
{
	CString ID=GetFreeID();
	CIniFile& ini=Map->GetIniFile();
	CString data="New AI Trigger,";
	
	// now try to set a teamtype
	if(ini.sections["TeamTypes"].values.size()>0)
	{
		data+=*ini.sections["TeamTypes"].GetValue(0);
	}
	else
	{
		data+="<none>";
	}
	
	data+=",<all>,";
	
	// the flag here... i first thought it´s an ID, but it more seems to be an value specifying the importance of the trigger
	data+="1,";

	// the flag 2 seems to be the type of the trigger. -1 seems to be pool
	data+="0,";

	// now stuff
	data+="<none>,0000000000000000000000000000000000000000000000000000000000000000,50.000000,30.000000,50.000000,1,0,1,1,";
	
	// a pool seems to need both teamtypes the same
	if(ini.sections["TeamTypes"].values.size()>0)
	{
		data+="<none>";//*ini.sections["TeamTypes"].GetValue(0);
	}
	else
	{
		data+="<none>";
	}

	data+=",1,1,1";
	
	ini.sections["AITriggerTypes"].values[ID]=data;

	UpdateDialog();
	
	// now make current id visible
	int i;
	for(i=0;i<m_AITriggerType.GetCount();i++)
	{
		CString cuString;
		m_AITriggerType.GetLBText(i, cuString);
		TruncSpace(cuString);

		if(cuString==ID)
		{
			m_AITriggerType.SetCurSel(i);
			OnSelchangeAitriggertype();
		}
	}
}

void CAITriggerTypes::OnDelete() 
{
	int sel=m_AITriggerType.GetCurSel();
	if(sel<0) return;

	CString aitrigger;
	m_AITriggerType.GetLBText(sel,aitrigger);
	TruncSpace(aitrigger);

	CIniFile& ini=Map->GetIniFile();
	
	ini.sections["AITriggerTypes"].values.erase(aitrigger);
	ini.sections["AITriggerTypesEnable"].values.erase(aitrigger);

	UpdateDialog();
}

void CAITriggerTypes::OnEditchangeAitriggertype() 
{
	OnSelchangeAitriggertype();	
}

AITrigInfo CAITriggerTypes::ConvertToAITrigInfoFromHex(char *aitinfo)
{
	int index=0;
	AITrigInfo info;
	char* rawdata=(char*)&info;
	char hexbuff[3]="00";
	char* dummy=NULL;

	while(*aitinfo!='\0')
	{
		while(isspace((int)*aitinfo)) aitinfo++;

		hexbuff[0]=*aitinfo++;
		ASSERT(*aitinfo!='\0');
		if(*aitinfo!='\0'){
			hexbuff[1]=*aitinfo++;
		} else {
			hexbuff[1]='\0';
		}

		ASSERT(index<sizeof(AITrigInfo));
		if(index>=sizeof(AITrigInfo)) break;

		*rawdata++=strtol(hexbuff, &dummy, 16);
		index++;
	}

	return info;
}

char* CAITriggerTypes::ConvertToHexFromAITrigInfo(AITrigInfo info, char* buffer)
{
	unsigned char* aitinfobuffer=(unsigned char*)buffer;
	/*
	** Convert the AITrigInfo union data structure to ASCII format string
	*/
	{
		
		int index = 0;
		unsigned char *rawdata = (unsigned char *)&info;
		unsigned char *aitinfobuffptr = &aitinfobuffer[0];

		for (; index < sizeof(AITrigInfo); index++, rawdata++, aitinfobuffptr += 2) {
			sprintf((char *)aitinfobuffptr, "%02x", *rawdata);
		}

		*aitinfobuffptr = '\0';
	}
	
	return (char*)aitinfobuffer;
}

void CAITriggerTypes::OnSelchangeCondition() 
{
	int sel=m_AITriggerType.GetCurSel();
	if(sel<0) return;

	UpdateData(TRUE);

	AITrigInfo info;
	info.Condition=(ConditionEnum)m_Condition;
	info.Number=m_Number;

	char buffer[65];
	ConvertToHexFromAITrigInfo(info, buffer);
	
	
	SetAITriggerParam(buffer, 6);

	UpdateDialog();	
}

void CAITriggerTypes::OnChangeNumber() 
{
	int sel=m_AITriggerType.GetCurSel();
	if(sel<0) return;

	UpdateData(TRUE);

	AITrigInfo info;
	memset(&info, 0, sizeof(AITrigInfo));
	info.Condition=(ConditionEnum)m_Condition;
	info.Number=m_Number;

	char buffer[65];
	ConvertToHexFromAITrigInfo(info, buffer);

	//MessageBox(buffer);

	//m_Data.SetWindowText(buffer);

	SetAITriggerParam(buffer, 6);

	UpdateDialog();		

}

void CAITriggerTypes::OnEasy() 
{
	UpdateData();

	CString value;
	value="1";
	if(!m_Easy) value="0";

	SetAITriggerParam(value, 15);
}

void CAITriggerTypes::OnMedium() 
{
	UpdateData();

	CString value;
	value="1";
	if(!m_Medium) value="0";

	SetAITriggerParam(value, 16);
}

void CAITriggerTypes::OnHard() 
{
	UpdateData();

	CString value;
	value="1";
	if(!m_Hard) value="0";

	SetAITriggerParam(value, 17);
}

void CAITriggerTypes::OnBasedefense() 
{
	UpdateData();

	CString value;
	value="1";
	if(!m_BaseDefense) value="0";

	SetAITriggerParam(value, 13);
}

void CAITriggerTypes::OnSkirmish() 
{
	UpdateData();

	CString value;
	value="1";
	if(!m_Skirmish) value="0";

	SetAITriggerParam(value, 10);
}

void CAITriggerTypes::OnEditchangeMultiside() 
{
	UpdateData();

	CString value;
	value=m_MultiSide;

	TruncSpace(value);

	SetAITriggerParam(value, 12);
}

void CAITriggerTypes::OnSelchangeMultiside() 
{
	CString s;
	CComboBox& box=*(CComboBox*)GetDlgItem(IDC_MULTISIDE);
	box.GetLBText(box.GetCurSel(), s);
	box.SetWindowText(s);

	OnEditchangeMultiside();
}
