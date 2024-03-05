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

// UserScriptsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "UserScriptsDlg.h"
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "variables.h"
#include "functions.h"
#include "inlines.h"
#include "combouinputdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace std;

struct FunctionData
{
	CString* name;
	CString* param;
public:
	void Init();
	FunctionData();
	int paramcount;
	void AddParam();
	~FunctionData();
};

struct JumpLineData
{
	CString name;
	int line;
};

class CUserScript  
{
public:
	void RaiseErr(int n, const char* str);
	char errortext[250];
	int error;
	int functioncount;
	int GetFunction(int index, CString* name, CString *params[], int* paramcount);
	int FindJumpLine(CString name);
	int LoadFile(const char* filename);
	CUserScript();
	virtual ~CUserScript();

private:
	int AllocateFunction();
	FunctionData* functiondata;
	map<CString, int> jumplinedata;


	CString filename;
};

CUserScript::CUserScript()
{
	functioncount=0;
	functiondata=NULL;

}

CUserScript::~CUserScript()
{
	if(functiondata!=NULL) free( functiondata);
}

int CUserScript::FindJumpLine(CString name)
{
	if(jumplinedata.find(name)==jumplinedata.end()) return -1;
	return jumplinedata[name];
}

int CUserScript::LoadFile(const char *setupfile)
{
	filename=setupfile;
	
	int file=_open(setupfile, _O_RDONLY);
	if(file==-1)
	{
		MessageBox(0, "Error opening script file", "Error", MB_ICONERROR);
		return -1;
	}

	int parsepos=0, filesize=0;
	BOOL inFunction=FALSE;
	BOOL inParam=FALSE;
	BOOL inComment=FALSE;
	BOOL inFunctionHead=FALSE;
	BOOL inNewOrder=TRUE;
	BOOL inJumpLine=FALSE;

	
	
	while(!(_eof(file)))
	{
		unsigned char c;
		int res=_read(file,(void*) &c, 1);
		if(res>0) filesize++; 
	}

	_lseek(file, 0, SEEK_SET);
	unsigned char *data=new(unsigned char[filesize+5]);
	data[filesize]=0;
	for (parsepos=0;parsepos<filesize;parsepos++)
	{
		_read(file, data+parsepos, 1);
	}
	_close(file);
		
    //MessageBox(0,(char*)data,"DEBUG: SETUPSCRIPT:/",0);

	BYTE jumplinename[512];
	memset(jumplinename, 512, 0);

	//// MAIN STUFF HERE ////
	for(parsepos=0;parsepos<filesize;parsepos++)
	{
		

		if(inComment==TRUE || inNewOrder==FALSE)
		{



			if(data[parsepos]=='\n')
			{
				//MessageBox(0, "NEWLINE", (char*)&data[parsepos+1], 0);
				inNewOrder=TRUE;
				inComment=FALSE;
				inJumpLine=FALSE;
				memset(jumplinename, 0, 512);
				
			}
		}
		
		else if(inFunction==FALSE && inComment==FALSE && inFunctionHead==FALSE && inJumpLine==FALSE)
		{
			// easy here, just seek for new position

			if(data[parsepos]=='/' && data[parsepos+1]=='/')
			{
				inComment=TRUE;
			}
			else if(data[parsepos]==';')
			{
				inNewOrder=TRUE;
			}
			else if(data[parsepos]==':')
			{
				inJumpLine=TRUE;
			}
			else if(IsCharAlphaNumeric(data[parsepos])!=0 && inNewOrder==TRUE && inJumpLine==FALSE)
			{
				inFunction=TRUE;
				int pos=AllocateFunction();
				//*functiondata[pos-1].name.append(data[parsepos]);
				*functiondata[pos-1].name = (char)data[parsepos];
			}

		}
		else if(data[parsepos]=='(' && inComment==FALSE && inParam==FALSE)
		{
			//MessageBox(0, "InHead","",0);
			inFunctionHead=TRUE;
		}
		else if(inFunctionHead==TRUE && inParam==TRUE)
		{
			if(data[parsepos]=='\\')
			{
				if(data[parsepos+1]=='n' || data[parsepos+1]=='N')
				{
					data[parsepos]='\n';
					functiondata[functioncount-1].param[functiondata[functioncount-1].paramcount-1]+=data[parsepos];
					parsepos++;
				}
				if(data[parsepos+1]=='r' || data[parsepos+1]=='R')
				{
					data[parsepos]='\r';
					functiondata[functioncount-1].param[functiondata[functioncount-1].paramcount-1]+=data[parsepos];
					parsepos++;
				}
			}

			else if(data[parsepos]=='"' && data[parsepos+1]!='"')
			{
				inParam=FALSE;
				//MessageBox(0, functiondata[functioncount-1].param[functiondata[functioncount-1].paramcount-1].data(), functiondata[functioncount-1].name->data(), 0);


			}
			else
			{

				// add character to param
				functiondata[functioncount-1].param[functiondata[functioncount-1].paramcount-1]+=data[parsepos];
				if(data[parsepos]=='"' && data[parsepos+1]=='"') parsepos++; // ignore the next "


			}
		}
		else if(inFunction==TRUE && inParam==FALSE && inFunctionHead==FALSE)
		{
			if(IsCharAlphaNumeric(data[parsepos])!=0)
			{
				*functiondata[functioncount-1].name+=data[parsepos];
			}
			else
			{
				inFunction=FALSE;
				//MessageBox(0, functiondata[functioncount-1].name->data(), "FUNCTIONAME", 0);
			}
		}

		else if(inFunctionHead==TRUE && inParam==FALSE)
		{
			if(data[parsepos]=='"')
			{
				// add a param!
				inParam=TRUE;
				functiondata[functioncount-1].AddParam();
				functiondata[functioncount-1].param[functiondata[functioncount-1].paramcount-1]="";
			}
			if(data[parsepos]==')')
			{
				inFunctionHead=FALSE;
				//MessageBox(0, (char*)&data[parsepos], "", 0);
			}
		}

		else if(inJumpLine==TRUE)
		{
			if(data[parsepos]!=':')
			{
				char d[2];
				d[0]=data[parsepos];
				d[1]=0;
				strcat((char*)jumplinename, d);
			}
			else
			{
				jumplinedata[jumplinename]=functioncount;
				//MessageBox(0,(char*)jumplinename,"",0);
				memset(jumplinename, 0, 512);
				inJumpLine=FALSE;
				inNewOrder=TRUE;
			}
		}
		

		

	}
	/////////////////////////



		
	delete[] (data);
	return 0;
}

int CUserScript::GetFunction(int index, CString *name, CString * params[], int* paramcount)
{
	if(index<0 || index>=functioncount) return -1;

	*name=*functiondata[index].name;
	*paramcount=functiondata[index].paramcount;
	
	//if(params==NULL) return 0;
	if (*paramcount>0)
	{
		*params=new(CString[*paramcount]);
	}
	else *params=0;
	

	int i;
	for(i=0;i<*paramcount;i++)
	{
		//params[i]=new(CString);
		CString* str=*params;
		str[i]=functiondata[index].param[i];
		//memcpy((void*)params[i], (void*) &functiondata[index].param[i], &function
		//MessageBox(0,functiondata[index].param[i].data(),"",0);
	}


	return 0;
}

void FunctionData::AddParam()
{
	//CString* tmp=NULL;
	//if(paramcount>0)
	//{
		
		//tmp=new(CString[paramcount]);
		//memcpy((void*)tmp, param, sizeof(CString)*paramcount);
				
		//delete[] (functiondata); //delete is bad-> CStrings would be deallocated!
		//delete[](param);
		//param=NULL;
	//}
	//param=(CString*)realloc(param, (paramcount+1)*sizeof(CString));
	//param[paramcount]=CString();
	//CString hack;
	//memset(&param[paramcount], 0, sizeof(CString));
	//memcpy(&param[paramcount], &hack, sizeof(CString));
	//param[paramcount]="";
	//param=new(CString[paramcount+1]);
	//
	//memcpy((void*) param, (void*) tmp, sizeof(CString)*paramcount);
	
	//name="";

	CString* newparam=new(CString[paramcount+1]);
	int i;
	for(i=0;i<paramcount;i++)
	{
		newparam[i]=param[i];
	}
	newparam[paramcount]="";

	delete[] param;

	param=newparam;

	paramcount++;
	//delete[](tmp);
	
}


FunctionData::FunctionData()
{
	paramcount=0;
	param=NULL;
	name=new(CString);
}

FunctionData::~FunctionData()
{
	if (param!=NULL) delete[] param;
	paramcount=0;
	delete (name);
}

int CUserScript::AllocateFunction()
{
	/*FunctionData* tmp=NULL;
	if(functioncount>0)
	{
		
		tmp=new(FunctionData[functioncount]);
		memcpy((void*)tmp, functiondata, sizeof(FunctionData)*functioncount);
		delete[] functiondata;
		functiondata=NULL;
	}

	functiondata=new(FunctionData[functioncount+1]);
	
	memcpy((void*) functiondata, (void*) tmp, sizeof(FunctionData)*functioncount);
	
	functiondata[functioncount].name="";*/
	functiondata=(FunctionData*)realloc(functiondata, sizeof(FunctionData)*(functioncount+1));
	
	functiondata[functioncount].Init();

	*functiondata[functioncount].name="HUHU";

	functioncount++;
	//delete[] (tmp);
	return functioncount;
}

void CUserScript::RaiseErr(int n, const char *str)
{

}

void FunctionData::Init()
{
	paramcount=0;
	param=NULL;
	name=new(CString);
}


/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CUserScriptsDlg 


CUserScriptsDlg::CUserScriptsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUserScriptsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUserScriptsDlg)
	m_Script = _T("");
	m_Report = _T("");
	//}}AFX_DATA_INIT
}


void CUserScriptsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserScriptsDlg)
	DDX_LBString(pDX, IDC_SCRIPTS, m_Script);
	DDX_Text(pDX, IDC_REPORT, m_Report);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserScriptsDlg, CDialog)
	//{{AFX_MSG_MAP(CUserScriptsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CUserScriptsDlg 

#define ASK_CONTINUE "AskContinue"
#define ADD_TRIGGER "AddTrigger"
#define MESSAGE "Message"
#define SET_INI_KEY "SetIniKey"
#define SET_SAFE_MODE "SetSafeMode"
#define SET_VARIABLE "SetVariable"
#define SET_VARIABLE_IF "SetVariableIf"
#define ADD "Add"
#define MULTI "Multi"
#define SUBSTRACT "Substract"
#define DIVIDE "Divide"
#define SET_WAYPOINT "SetWaypoint"
#define REQUIRES_MP "RequiresMP"
#define REQUIRES_SP "RequiresSP"
#define ADD_AI_TRIGGER "AddAITrigger"
#define ADD_TAG "AddTag"
#define RESIZE "Resize"
#define IS "Is"
#define CANCEL "Cancel"
#define PRINT "Print"
#define TOLOWER "LowerCase"
#define TOUPPER "UpperCase"
#define GET_FREE_WAYPOINT "GetFreeWaypoint"
#define UINPUT_GET_INTEGER "UInputGetInteger"
#define UINPUT_GET_STRING "UInputGetString"
#define JUMP_TO_LINE "JumpTo"
#define SET_AUTO_UPDATE "SetAutoUpdate"
#define GET_RANDOM "GetRandom"
#define ADD_TERRAIN "AddTerrain"
#define GET_INI_KEY "GetIniKey"
#define MODULO "Mod"
#ifdef SMUDGE_SUPP
#define ADD_SMUDGE "AddSmudge"
#endif
#define INSERT "Insert"
#define LENGTH "Length"
#define TRIM "Trim"
#define GETCHAR "GetChar"
#define REPLACE "Replace"
#define REMOVE "Remove"
#define GET_WAYPOINT_POS "GetWaypointPos"
#define GET_PARAM "GetParam"
#define SET_PARAM "SetParam"
#define GET_PARAM_COUNT "GetParamCount"
#define ALLOW_DELETE "AllowDelete"
#define DELETE_TERRAIN "DeleteTerrain"
#define DELETE_INFANTRY "DeleteInfantry"
#define DELETE_AIRCRAFT "DeleteAircraft"
#define DELETE_STRUCTURE "DeleteStructure"
#define DELETE_VEHICLE "DeleteVehicle"
#define IS_INFANTRY_DELETED "IsInfantryDeleted"
#define IS_TERRAIN_DELETED "IsTerrainDeleted"
#define ADD_INFANTRY "AddInfantry"
#define ALLOW_ADD "AllowAdd"
#define ADD_VEHICLE "AddVehicle"
#define ADD_AIRCRAFT "AddAircraft"
#define ADD_STRUCTURE "AddStructure"
#define GET_INFANTRY "GetInfantry"
#define GET_AIRCRAFT "GetAircraft"
#define GET_STRUCTURE "GetStructure"
#define GET_VEHICLE "GetVehicle"
#define UINPUT_GET_HOUSE "UInputGetHouse"
#define UINPUT_GET_COUNTRY "UInputGetCountry"
#define UINPUT_GET_TRIGGER "UInputGetTrigger"
#define UINPUT_GET_TAG "UInputGetTag"
#define MESSAGE_YES_NO "Ask"
#define GET_HOUSE "GetHouse"
#define GET_COUNTRY "GetCountry"
#define GET_HOUSE_INDEX "GetHouseIndex"
#define OR "Or"
#define AND "And"
#define NOT "Not"
#define UINPUT_GET_MANUAL "UInputSelect"

#define ID_ASK_CONTINUE 0
#define ID_ADD_TRIGGER 1
#define ID_MESSAGE 2
#define ID_SET_INI_KEY 3
#define ID_SET_SAFE_MODE 4
#define ID_SET_VARIABLE 5
#define ID_SET_VARIABLE_IF 6
#define ID_ADD 7
#define ID_MULTI 8
#define ID_SUBSTRACT 9
#define ID_DIVIDE 10
#define ID_SET_WAYPOINT 11
#define ID_REQUIRES_MP 12
#define ID_REQUIRES_SP 13
#define ID_ADD_AI_TRIGGER 14
#define ID_ADD_TAG 15
#define ID_RESIZE 16
#define ID_IS 17
#define ID_CANCEL 18
#define ID_PRINT 19
#define ID_TOLOWER 20
#define ID_TOUPPER 21
#define ID_GET_FREE_WAYPOINT 22
#define ID_UINPUT_GET_INTEGER 23
#define ID_UINPUT_GET_STRING 24
#define ID_JUMP_TO_LINE 25
#define ID_SET_AUTO_UPDATE 26
#define ID_GET_RANDOM 27
#define ID_ADD_TERRAIN 28
#define ID_GET_INI_KEY 29
#define ID_MODULO 30
#ifdef SMUDGE_SUPP
#define ID_ADD_SMUDGE 31
#endif
#define ID_INSERT 32
#define ID_LENGTH 33
#define ID_TRIM 34
#define ID_GETCHAR 35
#define ID_REPLACE 36
#define ID_REMOVE 37
#define ID_GET_WAYPOINT_POS 38
#define ID_GET_PARAM 39
#define ID_SET_PARAM 40
#define ID_GET_PARAM_COUNT 41
#define ID_ALLOW_DELETE 42
#define ID_DELETE_TERRAIN 43
#define ID_DELETE_INFANTRY 44
#define ID_DELETE_STRUCTURE 45
#define ID_DELETE_AIRCRAFT 46
#define ID_DELETE_VEHICLE 47
#define ID_IS_INFANTRY_DELETED 48
#define ID_IS_TERRAIN_DELETED 49
#define ID_ADD_INFANTRY 50
#define ID_ALLOW_ADD 51
#define ID_ADD_VEHICLE 52
#define ID_ADD_AIRCRAFT 53
#define ID_ADD_STRUCTURE 54
#define ID_GET_INFANTRY 55
#define ID_GET_AIRCRAFT 56
#define ID_GET_STRUCTURE 57
#define ID_GET_VEHICLE 58
#define ID_UINPUT_GET_HOUSE 59
#define ID_UINPUT_GET_COUNTRY 60
#define ID_UINPUT_GET_TRIGGER 61
#define ID_UINPUT_GET_TAG 62
#define ID_MESSAGE_YES_NO 63
#define ID_GET_HOUSE 64
#define ID_GET_COUNTRY 65
#define ID_GET_HOUSE_INDEX 66
#define ID_OR 67
#define ID_AND 68
#define ID_NOT 69
#define ID_UINPUT_GET_MANUAL 70


extern CString GetFree(const char* section);

BOOL IsValSet(CString val)
{
	val.MakeLower();
	if(val=="false" || val=="no") return FALSE; // val is not set
	if(val=="true" || val=="yes") return TRUE;
	if(atoi(val)) return TRUE;
	return FALSE;
}

int get_player_count()
{
	if(Map->IsMultiplayer()==FALSE) return 1;

	int i;
	int wp_count=0;
	for(i=0;i<Map->GetWaypointCount();i++)
	{
		CString id;
		DWORD pos;
		Map->GetWaypointData(i, &id, &pos);
		int idi;
		idi=atoi(id);
		if(idi!=i) break;
		if(idi>=0 && idi<8) 
		{
			wp_count++;	
		}		
	}

	return wp_count;
}

struct FUNC_INFO
{
	int type;
	CString name;
	CString* params;
	int paramcount;
};

void CUserScriptsDlg::OnOK() 
{
	UpdateData(TRUE);

	//srand((unsigned)time(NULL));

	if(m_Script.GetLength()==0) return;

	CUserScript s;
	s.LoadFile((CString)AppPath+(CString)"\\Scripts\\"+m_Script);

	CIniFile& ini=Map->GetIniFile();

	BOOL bUpdate=FALSE;
	BOOL bUpdateOnlyMission=TRUE;
	BOOL bNoRepos=TRUE;
	BOOL bSafeMode=TRUE;

	map<CString, CString> variables;
	char c[50];
	

	CString report=m_Script+" Report:\r\n\r\n";

	BOOL bAutoUpdate=TRUE;
	BOOL bAllowLoop=TRUE; // for now we enable this, as we´ve introduced a loop counter
	
	// stuff for faster CMapData processing:
	BOOL bUpdateWaypoints=FALSE;
	BOOL bOldUpdate=Map->m_noAutoObjectUpdate;

	BOOL bDeleteAllowed=FALSE; // shall the script be able to delete stuff?
	BOOL bAddAllowed=FALSE; // " delete stuff
	
	
	


	// for delete/add stuff:
	int lastInfantryDeleted=-1;
	CString lastStructureDeleted="";
	int lastTerrainDeleted=-1;
	CString lastUnitDeleted="";
	CString lastAircraftDeleted="";


	BOOL bIgnoreLoopCounts=FALSE;
	int loop_count=0;


	map<int, FUNC_INFO> functions;

	// get function ids
	int i;
	for(i=0;i<s.functioncount;i++)
	{
		FUNC_INFO info;
		CString& name=info.name;

		s.GetFunction(i, &info.name, &info.params, &info.paramcount);

		
		
		if(name==ASK_CONTINUE)
		{
			info.type=ID_ASK_CONTINUE;
		}
		else if(name==MESSAGE)
		{
			info.type=ID_MESSAGE;
		}
		else if(name==ADD_TRIGGER)
		{
			info.type=ID_ADD_TRIGGER;
		}
		else if(name==SET_INI_KEY)
		{
			info.type=ID_SET_INI_KEY;
		}
		else if(name==SET_SAFE_MODE)
		{
			info.type=ID_SET_SAFE_MODE;
		}
		else if(name==SET_VARIABLE)
		{
			info.type=ID_SET_VARIABLE;
		}
		else if(name==ADD)
		{
			info.type=ID_ADD;
		}
		else if(name==SUBSTRACT)
		{
			info.type=ID_SUBSTRACT;
		}
		else if(name==MULTI)
		{
			info.type=ID_MULTI;
		}
		else if(name==DIVIDE)
		{
			info.type=ID_DIVIDE;
		}
		else if(name==SET_WAYPOINT)
		{
			info.type=ID_SET_WAYPOINT;
		}
		else if(name==REQUIRES_MP)
		{
			info.type=ID_REQUIRES_MP;
		}
		else if(name==REQUIRES_SP)
		{
			info.type=ID_REQUIRES_SP;
		}
		else if(name==ADD_AI_TRIGGER)
		{
			info.type=ID_ADD_AI_TRIGGER;
		}
		else if(name==ADD_TAG)
		{
			info.type=ID_ADD_TAG;
		}
		else if(name==RESIZE)
		{
			info.type=ID_RESIZE;
		}
		else if(name==IS)
		{
			info.type=ID_IS;						
		}
		else if(name==CANCEL)
		{			
			info.type=ID_CANCEL;
		}
		else if(name==PRINT)
		{			
			info.type=ID_PRINT;			
		}
		else if(name==TOLOWER)
		{	
			info.type=ID_TOLOWER;			
		}
		else if(name==TOUPPER)
		{			
			info.type=ID_TOUPPER;
		}
		else if(name==GET_FREE_WAYPOINT)
		{
			info.type=ID_GET_FREE_WAYPOINT;
		}
		else if(name==UINPUT_GET_INTEGER)
		{
			info.type=ID_UINPUT_GET_INTEGER;
		}
		else if(name== UINPUT_GET_STRING )
		{
			info.type=ID_UINPUT_GET_STRING;
		}
		else if(name==JUMP_TO_LINE)
		{
			info.type=ID_JUMP_TO_LINE;
		}
		else if(name==SET_AUTO_UPDATE)
		{
			info.type=ID_SET_AUTO_UPDATE;
		}
		else if(name==GET_RANDOM)
		{
			info.type=ID_GET_RANDOM;
		}
		else if(name==ADD_TERRAIN)
		{
			info.type=ID_ADD_TERRAIN;
		}
		else if(name==GET_INI_KEY)
		{
			info.type=ID_GET_INI_KEY;
		}
		else if(name==MODULO)
		{
			info.type=ID_MODULO;
		}
#ifdef SMUDGE_SUPP
		else if(name==ADD_SMUDGE)
		{
			info.type=ID_ADD_SMUDGE;
		}
#endif
		else if(name==INSERT)
		{
			info.type=ID_INSERT;
		}
		else if(name==LENGTH)
		{
			info.type=ID_LENGTH;
		}
		else if(name==TRIM)
		{
			info.type=ID_TRIM;
		}
		else if(name==GETCHAR)
		{
			info.type=ID_GETCHAR;
		}
		else if(name==REPLACE)
		{
			info.type=ID_REPLACE;
		}
		else if(name==REMOVE)
		{
			info.type=ID_REMOVE;
		}
		else if(name==GET_WAYPOINT_POS)
		{
			info.type=ID_GET_WAYPOINT_POS;
		}
		else if(name==GET_PARAM)
		{
			info.type=ID_GET_PARAM;
		}
		else if(name==SET_PARAM)
		{
			info.type=ID_SET_PARAM;
		}
		else if(name==GET_PARAM_COUNT)
		{
			info.type=ID_GET_PARAM_COUNT;
		}
		else if(name==ALLOW_DELETE)
		{
			info.type=ID_ALLOW_DELETE;
		}
		else if(name==DELETE_TERRAIN)
		{
			info.type=ID_DELETE_TERRAIN;
		}
		else if(name==DELETE_INFANTRY)
		{
			info.type=ID_DELETE_INFANTRY;
		}
		else if(name==DELETE_STRUCTURE)
		{
			info.type=ID_DELETE_STRUCTURE;
		}
		else if(name==DELETE_AIRCRAFT)
		{
			info.type=ID_DELETE_AIRCRAFT;
		}
		else if(name==DELETE_VEHICLE)
		{
			info.type=ID_DELETE_VEHICLE;
		}
		else if(name==IS_INFANTRY_DELETED)
		{
			info.type=ID_IS_INFANTRY_DELETED;
		}
		else if(name==IS_TERRAIN_DELETED)
		{
			info.type=ID_IS_TERRAIN_DELETED;
		}
		else if(name==ADD_INFANTRY)
		{
			info.type=ID_ADD_INFANTRY;
		}
		else if(name==ALLOW_ADD)
		{
			info.type=ID_ALLOW_ADD;
		}
		else if(name==ADD_VEHICLE) info.type=ID_ADD_VEHICLE;
		else if(name==ADD_AIRCRAFT) info.type=ID_ADD_AIRCRAFT;
		else if(name==ADD_STRUCTURE) info.type=ID_ADD_STRUCTURE;
		else if(name==GET_INFANTRY) info.type=ID_GET_INFANTRY;
		else if(name==GET_AIRCRAFT) info.type=ID_GET_AIRCRAFT;
		else if(name==GET_STRUCTURE) info.type=ID_GET_STRUCTURE;
		else if(name==GET_VEHICLE) info.type=ID_GET_VEHICLE;
		else if(name==UINPUT_GET_HOUSE) info.type=ID_UINPUT_GET_HOUSE;
		else if(name==UINPUT_GET_COUNTRY) info.type=ID_UINPUT_GET_COUNTRY;
		else if(name==UINPUT_GET_TRIGGER) info.type=ID_UINPUT_GET_TRIGGER;
		else if(name==UINPUT_GET_TAG) info.type=ID_UINPUT_GET_TAG;
		else if(name==MESSAGE_YES_NO) info.type=ID_MESSAGE_YES_NO;
		else if(name==GET_HOUSE) info.type=ID_GET_HOUSE;
		else if(name==GET_COUNTRY) info.type=ID_GET_COUNTRY;
		else if(name==GET_HOUSE_INDEX) info.type=ID_GET_HOUSE_INDEX;
		else if(name==OR) info.type=ID_OR;
		else if(name==AND) info.type=ID_AND;
		else if(name==NOT) info.type=ID_NOT;
		else if(name==UINPUT_GET_MANUAL) info.type=ID_UINPUT_GET_MANUAL;

		else info.type=-1;
		
			
		

		functions[i]=info;
	}


	for(i=0;i<s.functioncount;i++)
	{
		// initialize global variables here so they can´t be overwritten!
		itoa(Map->GetWidth(), c, 10);
		variables["%Width%"]=c;
		itoa(Map->GetHeight(), c, 10);
		variables["%Height%"]=c;
		itoa(Map->GetIsoSize(), c, 10);
		variables["%IsoSize%"]=c;
		itoa(Map->GetWaypointCount(), c, 10);
		variables["%WaypointCount%"]=c;
		itoa(Map->GetUnitCount(), c, 10);
		variables["%UnitCount%"]=c;
		itoa(Map->GetInfantryCount(), c, 10);
		variables["%InfantryCount%"]=c;
		itoa(Map->GetStructureCount(), c, 10);
		variables["%StructureCount%"]=c;
		itoa(Map->GetAircraftCount(), c, 10);
		variables["%AircraftCount%"]=c;
		itoa(Map->GetTerrainCount(), c, 10);
		variables["%TerrainCount%"]=c;
		variables["%Theater%"]=Map->GetTheater();
		itoa(get_player_count(), c, 10);
		variables["%PlayerCount%"]=c;
		itoa(Map->GetHousesCount(FALSE), c, 10);
		variables["%HousesCount%"]=c;
		itoa(Map->GetHousesCount(TRUE), c, 10);
		variables["%CountriesCount%"]=c;

		if(bDeleteAllowed)
		{
			variables["%DeleteAllowed%"]="1";
		}
		else
			variables["%DeleteAllowed%"]="0";

		if(bAddAllowed)
		{
			variables["%AddAllowed%"]="1";
		}
		else
			variables["%AddAllowed%"]="0";

		if(bSafeMode)
			variables["%SafeMode%"]="1";
		else
			variables["%SafeMode%"]="0";
		
		//CString name;
		int name=functions[i].type;
		
		int paramcount=functions[i].paramcount;
		
		CString* params=NULL;

		if(paramcount)
		{
			params=new(CString[paramcount]);
			int e;
			for(e=0;e<paramcount;e++)
				params[e]=functions[i].params[e];
		}
		
		//s.GetFunction(i, &name, &params, &paramcount);


		BOOL * replaceVariables=new(BOOL[paramcount+4]); // make sure at least room for 4 variables

		int h;
		for(h=0;h<paramcount;h++) replaceVariables[h]=TRUE;

		if(name==ID_SET_VARIABLE || name==ID_ADD || name==ID_SUBSTRACT || name==ID_MULTI || name==ID_DIVIDE 
			|| name==ID_TOLOWER || name==ID_TOUPPER || name==ID_GET_FREE_WAYPOINT || name==ID_JUMP_TO_LINE
			|| name==ID_UINPUT_GET_INTEGER || name==ID_UINPUT_GET_STRING || name==ID_GET_RANDOM || name==ID_ADD_TRIGGER
			|| name==ID_ADD_AI_TRIGGER || name==ID_GET_INI_KEY || name==ID_MODULO || name==ID_INSERT
			|| name==ID_LENGTH || name==ID_TRIM || name==ID_GETCHAR || name==ID_REPLACE || name==ID_REMOVE
			|| name==ID_GET_PARAM || name==ID_SET_PARAM || name==ID_GET_PARAM_COUNT || name==ID_IS_INFANTRY_DELETED
			|| name==ID_IS_TERRAIN_DELETED || name==ID_GET_INFANTRY || name==ID_GET_AIRCRAFT || name==ID_GET_STRUCTURE
			|| name==ID_GET_VEHICLE || name==ID_UINPUT_GET_TRIGGER || name==ID_UINPUT_GET_TAG
			|| name==ID_UINPUT_GET_HOUSE || name==ID_UINPUT_GET_COUNTRY || name==ID_MESSAGE_YES_NO
			|| name==ID_OR || name==ID_AND || name==ID_NOT || name==ID_UINPUT_GET_MANUAL)
		{
			replaceVariables[0]=FALSE;			
		}

		if(name==ID_GET_WAYPOINT_POS )
		{
			replaceVariables[1]=FALSE;
			replaceVariables[2]=FALSE;
		}

		if(name==ID_IS)
		{
			replaceVariables[3]=FALSE;
		}

		

		map<CString, CString>::iterator e;
		
		
		for(e=variables.begin();e!=variables.end();e++)
		{
			for(h=0;h<paramcount;h++)
			{
				if(replaceVariables[h])
				{
					params[h].Replace(e->first, e->second);
				}
			}
		}


		delete[] replaceVariables;


		if(name==ID_ASK_CONTINUE)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			int res=MessageBox(params[0], "Continue?", MB_YESNO);
			if(res==IDNO) break;
		}
		else if(name==ID_MESSAGE)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			MessageBox(params[0], params[1]);
		}
		else if(name==ID_MESSAGE_YES_NO)
		{
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}

			int res=MessageBox(params[1], params[2], MB_YESNO);
			if(res==IDYES)
			{
				variables[params[0]]="1";
			}
			else
				variables[params[0]]="0";
		}
		else if(name==ID_ADD_TRIGGER)
		{
			if(paramcount<5) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>5)
			{
				if(params[5].GetLength()>0)
				{
					if(!IsValSet(params[5])) goto nextline;
				}
			}

			if(!bAddAllowed) goto nextline;

			
			CString ID_T=GetFreeID();

			if(params[0].GetLength()>0)
			{
				variables[params[0]]=ID_T;
			}			

			ini.sections["Triggers"].values[ID_T]=params[1];
			ini.sections["Events"].values[ID_T]=params[2];
			ini.sections["Actions"].values[ID_T]=params[3];

			BOOL tag=TRUE;
			params[4].MakeLower();
			if(params[4]=="false" || params[4]=="no") tag=FALSE;

			if(tag)
			{
				CString ID_TAG=GetFreeID();
				ini.sections["Tags"].values[ID_TAG]="0,";
				ini.sections["Tags"].values[ID_TAG]+=GetParam(params[1],2);
				ini.sections["Tags"].values[ID_TAG]+=",";
				ini.sections["Tags"].values[ID_TAG]+=ID_T;
			}

			report+="Trigger " + GetParam(params[1],2) + " added\r\n";
			
			bUpdate=TRUE;
		}
		else if(name==ID_SET_INI_KEY)
		{
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}
			
			//if(ini.sections[params[0]].FindName(params[1])>=0)
			{
				if(bSafeMode) goto nextline;				
			}

			ini.sections[params[0]].values[params[1]]=params[2];

			report +=params[0]+(CString)"->"+params[1]+(CString) " set to \"" + params[2] + "\"\r\n"; 
			
			bUpdate=TRUE;
			bUpdateOnlyMission=FALSE;
		}
		else if(name==ID_GET_INI_KEY)
		{
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}

			if(ini.sections.find(params[1])==ini.sections.end() || ini.sections[params[1]].FindName(params[2])<0)
			{
				variables[params[0]]="";				
			}
			else
			variables[params[0]]=ini.sections[params[1]].values[params[2]];
		}
		else if(name==ID_SET_SAFE_MODE)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			BOOL enabled=TRUE;
			params[0].MakeLower();
			if(params[0]=="false" || params[0]=="no") enabled=FALSE;

			if(!enabled)
			{
				CString s;
				s="This script wants to disable INI protection. For some scripts this may be necessary, but it can seriously damage your Map-> Reason why script wants to disable INI protection:\n";
				s+=params[1];
				s+="\n\nDisable INI protection?";

				int res=MessageBox(s, "Disable INI protection?", MB_YESNO | MB_DEFBUTTON2);
				if(res==IDNO) goto nextline;
			}

			if(!enabled) report+="INI Protection disabled\r\n";
			if(enabled) report+="INI Protection enabled\r\n";

			bSafeMode=enabled;
		}
		else if(name==ID_SET_VARIABLE)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			variables[params[0]]=params[1];
		}
		else if(name==ID_ADD)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int n;
			n=atoi(variables[params[0]]);
			int n2=atoi(params[1]);
			n+=n2;
			char c[50];
			itoa(n, c, 10);
			variables[params[0]]=c;
		}
		else if(name==ID_INSERT)
		{
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}

			int pos=atoi(params[2]);

			if(pos<0)
			{
				pos=variables[params[0]].GetLength();
			}

			variables[params[0]].Insert(pos, params[1]);
		}
		else if(name==ID_REPLACE)
		{
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}			

			variables[params[0]].Replace(params[1], params[2]);
		}
		else if(name==ID_TRIM)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			variables[params[0]].TrimLeft();
			variables[params[0]].TrimRight();
		}
		else if(name==ID_NOT)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			if(IsValSet(variables[params[0]]))
				variables[params[0]]="0";
			else
				variables[params[0]]="1";
		}
		else if(name==ID_AND)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}
			
			BOOL bSet=TRUE;

			int k;
			for(k=1;k<paramcount;k++)
			{
				if(!IsValSet(params[k])) { bSet=FALSE; break; }
			}

			CString s="0";
			if(bSet) s="1";
			
			variables[params[0]]=s;			
		}
		else if(name==ID_OR)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}
			
			BOOL bSet=FALSE;

			int k;
			for(k=1;k<paramcount;k++)
			{
				if(IsValSet(params[k])) { bSet=TRUE; break; }
			}

			CString s="0";
			if(bSet) s="1";
			
			variables[params[0]]=s;			
		}
		else if(name==ID_LENGTH)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int len=params[1].GetLength();
			char c[50];
			itoa(len, c, 10);
			variables[params[0]]=c;
		}
		else if(name==ID_REMOVE)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			if(atoi(params[1])>=variables[params[0]].GetLength() || atoi(params[1])<0 || atoi(params[2])<0)
			{
				MessageBox("Invalid index or length for remove command, script cancelled.","Error");
				delete[] params;
				break;
			}

			variables[params[0]].Delete(atoi(params[1]),atoi(params[2]));
		}
		else if(name==ID_GETCHAR)
		{
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}

			if(atoi(params[2])>=params[1].GetLength() || atoi(params[2])<0)
			{
				MessageBox("Invalid index for GetChar command, script cancelled.","Error");
				delete[] params;
				break;
			}

			variables[params[0]]=params[1].GetAt(atoi(params[2]));
		}
		else if(name==ID_SUBSTRACT)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int n;
			n=atoi(variables[params[0]]);
			int n2=atoi(params[1]);
			n-=n2;
			char c[50];
			itoa(n, c, 10);
			variables[params[0]]=c;
		}
		else if(name==ID_MULTI)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int n;
			n=atoi(variables[params[0]]);
			int n2=atoi(params[1]);
			n=n*n2;
			char c[50];
			itoa(n, c, 10);
			variables[params[0]]=c;
		}
		else if(name==ID_DIVIDE)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int n;
			n=atoi(variables[params[0]]);
			int n2=atoi(params[1]);

			if(n2==0)
			{
				MessageBox("Division through 0, script cancelled", "Division through 0");
				delete[] params;
				break;
			}

			n=n/n2;
			char c[50];
			itoa(n, c, 10);
			variables[params[0]]=c;
		}
		else if(name==ID_MODULO)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int n;
			n=atoi(variables[params[0]]);
			int n2=atoi(params[1]);

			if(n2==0)
			{
				MessageBox("Division through 0, script cancelled", "Division through 0");
				delete[] params;
				break;
			}

			n=n%n2;
			char c[50];
			itoa(n, c, 10);
			variables[params[0]]=c;
		}
		else if(name==ID_SET_WAYPOINT)
		{
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}

			int n=atoi(params[0]);
			CString id=params[0];
			if(n<0)
			{
				id="";
			}

			if(bSafeMode && n>=0)
			{
				if(ini.sections["Waypoints"].FindName(id)>=0)
				{
					goto nextline;
				}
			}


			DWORD pos=atoi(params[1])+atoi(params[2])*Map->GetIsoSize();
			
			if(pos<Map->GetIsoSize()*Map->GetIsoSize())
			{
				//Map->m_noAutoObjectUpdate=TRUE;				
				Map->AddWaypoint(id, pos);
				bUpdateWaypoints=TRUE;
				//Map->m_noAutoObjectUpdate=bOldUpdate;
				report+="Waypoint " + id + " set.\r\n";
			}
			else
			{
				report+="Waypoint " + id + " moving failed!\r\n";
			}

			bUpdate=TRUE;
			bUpdateOnlyMission=FALSE;
		}
		else if(name==ID_REQUIRES_MP)
		{
			// check bool
			if(paramcount>0)
			{
				if(params[0].GetLength()>0)
				{
					if(!IsValSet(params[0])) goto nextline;
				}
			}

			if(Map->IsMultiplayer()==FALSE)
			{
				MessageBox("This script requires a multiplayer map and cannot be used with singleplayer maps", "Error");
				break;
			}
		}
		else if(name==ID_REQUIRES_SP)
		{
			// check bool
			if(paramcount>0)
			{
				if(params[0].GetLength()>0)
				{
					if(!IsValSet(params[0])) goto nextline;
				}
			}

			if(Map->IsMultiplayer()==TRUE)
			{
				MessageBox("This script requires a singleplayer map and cannot be used with multiplayer maps", "Error");
				break;
			}
		}
		else if(name==ID_ADD_AI_TRIGGER)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			if(!bAddAllowed) goto nextline;

			CString ID_T=GetFreeID();

			if(params[0].GetLength()>0)
			{
				variables[params[0]]=ID_T;
			}	

			ini.sections["AITriggerTypes"].values[ID_T]=params[1];

			report+="AI Trigger " + GetParam(params[1],0) + " added\r\n";
			
			bUpdate=TRUE;
		}
		else if(name==ID_ADD_TAG)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			if(!bAddAllowed) goto nextline;
			
			CString ID_T=GetFreeID();

			if(params[0].GetLength()>0)
			{
				variables[params[0]]=ID_T;
			}		

			CString ID_TAG=ID_T; //GetFreeID();
			ini.sections["Tags"].values[ID_TAG]=params[1]; 

			report+="Tag " + GetParam(params[1],1) + " added\r\n";
			
			bUpdate=TRUE;
		}
		else if(name==ID_RESIZE)
		{
			if(paramcount<4) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>4)
			{
				if(params[4].GetLength()>0)
				{
					if(!IsValSet(params[4])) goto nextline;
				}
			}

			int res=MessageBox("This script wants to resize the map. Resize map?", "Resize map?", MB_YESNO);
			if(res==IDNO) goto nextline;


			if(atoi(params[2])>200 || atoi(params[3])>200)
			{
				MessageBox("Resizing map failed. Script cancelled.", "Error");
				break;
			}

			Map->ResizeMap(atoi(params[0]), atoi(params[1]), atoi(params[2]), atoi(params[3]));

			bUpdate=TRUE;
			bUpdateOnlyMission=FALSE;
			bNoRepos=FALSE;
		}
		else if(name==ID_IS)
		{
			if(paramcount<4) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>4)
			{
				if(params[4].GetLength()>0)
				{
					if(!IsValSet(params[4])) goto nextline;
				}
			}

			BOOL bIs=FALSE;

			if(params[1]=="<")
			{
				if(atoi(params[0])<atoi(params[2])) bIs=TRUE;
			}
			if(params[1]=="<=")
			{
				if(atoi(params[0])<=atoi(params[2])) bIs=TRUE;
			}
			if(params[1]=="=")
			{
				if(atoi(params[0])==atoi(params[2])) 
				{					
					bIs=TRUE;
					
				}
				if(params[0]==params[2]) {bIs=TRUE;}
			}
			if(params[1]==">=")
			{
				if(atoi(params[0])>=atoi(params[2])) bIs=TRUE;
			}
			if(params[1]==">")
			{
				if(atoi(params[0])>atoi(params[2])) bIs=TRUE;
			}
			if(params[1]=="!=")
			{
				if(atoi(params[0])!=atoi(params[2])) bIs=TRUE;
				if(params[0]!=params[2]) bIs=TRUE;
			}



			CString s="0";
			if(bIs) s="1";

			
						
			variables[params[3]]=s;
						
		}
		else if(name==ID_CANCEL)
		{			
			// check bool
			if(paramcount>0)
			{
				if(params[0].GetLength()>0)
				{
					if(!IsValSet(params[0])) goto nextline;
				}
			}

			break;
		}
		else if(name==ID_PRINT)
		{	
			
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			report+=params[0];
			report+="\r\n";

			
		}
		else if(name==ID_TOLOWER)
		{	
			
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			variables[params[0]].MakeLower();

			
		}
		else if(name==ID_TOUPPER)
		{	
			
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			variables[params[0]].MakeUpper();

		}
		else if(name==ID_GET_FREE_WAYPOINT)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			variables[params[0]]=GetFree("Waypoints");
		}
		else if(name==ID_UINPUT_GET_INTEGER)
		{
			
			if(paramcount<4) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>4)
			{
				if(params[4].GetLength()>0)
				{
					if(!IsValSet(params[4])) goto nextline;
				}
			}

			BOOL ok=FALSE;
			int n=0;
			while(!ok)
			{
				CString s=InputBox(params[1], "Enter Integer");	
				ok=TRUE;

				if(s.GetLength()==0) ok=FALSE;

				n=atoi(s);				
				if(params[2].GetLength()>0)
				{
					if(n<atoi(params[2])) ok=FALSE;
				}

				if(params[3].GetLength()>0)
				{
					if(n>atoi(params[3])) ok=FALSE;
				}
			}

			char c[50];
			itoa(n, c, 10);
			variables[params[0]]=c;
		}
		else if(name==ID_UINPUT_GET_STRING)
		{
			
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			BOOL ok=FALSE;
			CString s;
			while(!ok)
			{
				s=InputBox(params[1], "Enter String");	
				ok=TRUE;

				if (s.GetLength()==0) ok=FALSE;
			}
			variables[params[0]]=s;
		}
		else if(name==ID_UINPUT_GET_HOUSE)
		{			
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			CComboUInputDlg dlg;
			dlg.m_type=COMBOUINPUT_HOUSES;
			dlg.m_Caption=params[1];

			dlg.DoModal();
			
			variables[params[0]]=dlg.m_Combo;
		}
		else if(name==ID_UINPUT_GET_COUNTRY)
		{			
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			CComboUInputDlg dlg;
			dlg.m_type=COMBOUINPUT_COUNTRIES;
			dlg.m_Caption=params[1];

			dlg.DoModal();
			
			variables[params[0]]=dlg.m_Combo;
		}
		else if(name==ID_UINPUT_GET_TRIGGER)
		{			
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			CComboUInputDlg dlg;
			dlg.m_type=COMBOUINPUT_TRIGGERS;
			dlg.m_Caption=params[1];

			dlg.DoModal();
			
			variables[params[0]]=dlg.m_Combo;
		}
		else if(name==ID_UINPUT_GET_TAG)
		{			
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			CComboUInputDlg dlg;
			dlg.m_type=COMBOUINPUT_TAGS;
			dlg.m_Caption=params[1];

			dlg.DoModal();
			
			variables[params[0]]=dlg.m_Combo;
		}
		/*else if(name==ID_UINPUT_GET_MANUAL)
		{			
			if(paramcount<4) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>4)
			{
				if(params[4].GetLength()>0)
				{
					if(!IsValSet(params[4])) goto nextline;
				}
			}

			CComboUInputDlg dlg;
			dlg.m_type=COMBOUINPUT_MANUAL;
			dlg.m_Caption=params[1];
			dlg.bTruncateStrings=IsValSet(params[2]);

						

			dlg.DoModal();
			
			variables[params[0]]=dlg.m_Combo;
		}*/
		else if(name==ID_JUMP_TO_LINE)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			
			if(!bAllowLoop) 
			{
				int res=MessageBox("This script tries to use loops. Some scripts may require this. However, this holds the risk of infinite loops, which may cause the FinalSun/FinalAlert2 to crash. Do you want to allow looping for this script?", "Allow looping?", MB_YESNO);
				if(res==IDYES)
				{
					bAllowLoop=TRUE;
				}else
				
				goto nextline; // not allowed in safe mode because of possible infinte loops#
			}

			if(!bIgnoreLoopCounts)
			{
				loop_count++;

				if(loop_count>300)
				{
					int res=MessageBox("This script has exceeded the 300 loops limit. Do you want to remove the loop limit (not recommended, inherits risk of infinite loops if script has bugs)? If you press no, the script will stop after another 300 loops to ask you again. If you press cancel, the script will be cancelled.", "Loop Limit exceeded", MB_YESNOCANCEL | MB_DEFBUTTON2);
					if(res==IDYES) bIgnoreLoopCounts=TRUE;
					if(res==IDNO) loop_count=0;
					if(res==IDCANCEL)
					{
						delete[] params;
						break;
					}

				}
			}

			
			int n=s.FindJumpLine(params[0]);
			if(n<0 || n>s.functioncount)
			{
								
				ReportScriptError(i);
				delete[] params;
				break;
			}


			i=n-1; // not n, as the for loop adds 1 again!
			goto nextline_no_update;
		}
		else if(name==ID_SET_AUTO_UPDATE)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			bAutoUpdate=IsValSet(params[0]);

			goto nextline;
		}
		else if(name==ID_GET_RANDOM)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			int n=rand(); // 0 and 32767
			
			char c[50];
			itoa(n, c, 10);
			variables[params[0]]=c;
		}
		else if(name==ID_ADD_TERRAIN)
		{ 
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}

			if(!bAddAllowed) goto nextline;

			DWORD pos;
			pos=atoi(params[1])+atoi(params[2])*Map->GetIsoSize();

			if(Map->GetTerrainAt(pos)<0)
			{
				Map->AddTerrain(params[0], pos);

				report+="Terrain added: " + params[0] + (CString)" at " + params[1] + (CString)"/" + params[2] + "\r\n";

				bUpdate=TRUE;
				bUpdateOnlyMission=FALSE;
			}
		}
#ifdef SMUDGE_SUPP
		else if(name==ID_ADD_SMUDGE)
		{ 
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}
			DWORD pos;
			pos=atoi(params[1])+atoi(params[2])*Map->GetIsoSize();

			FIELDDATA* fd=Map->GetFielddataAt(pos);
			if(fd->smudge<0)
			{
				SMUDGE s;
				s.deleted=0;
				s.type=params[0];
				s.x=atoi(params[1]);
				s.y=atoi(params[2]);
				Map->AddSmudge(&s);

				report+="Smudge added: " + params[0] + (CString) " at " + params[1] + (CString)"/" + params[2] + "\r\n";

				bUpdate=TRUE;
				bUpdateOnlyMission=FALSE;
			}
		}
#endif
		else if(name==ID_GET_WAYPOINT_POS)
		{ 
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}
			
			BOOL bFound=FALSE;
			DWORD pos;
			int k;
			for(k=0;k<Map->GetWaypointCount();k++)
			{
				CString id;
				Map->GetWaypointData(k, &id, &pos);

				if(id==params[0])
				{
					bFound=TRUE;
					break;
				}
			}

			if(!bFound) pos=0;

			int x=pos%Map->GetIsoSize();
			int y=pos/Map->GetIsoSize();

			char c[50];
			itoa(x, c, 10);
			variables[params[1]]=c;
			itoa(y, c, 10);
			variables[params[2]]=c;			
		}
		else if(name==ID_GET_PARAM)
		{ 
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}
			
			variables[params[0]]=GetParam(params[1], atoi(params[2]));
		}
		else if(name==ID_SET_PARAM)
		{ 
			if(paramcount<3) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>3)
			{
				if(params[3].GetLength()>0)
				{
					if(!IsValSet(params[3])) goto nextline;
				}
			}
			
			variables[params[0]]=SetParam(variables[params[0]], atoi(params[1]), params[2]);
		}
		else if(name==ID_GET_PARAM_COUNT)
		{ 
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int k;
			int count=1; // we start at 1 param even if no , exists!
			for(k=0;k<params[1].GetLength();k++)
			{
				if(params[1].GetAt(k)==',') count++;				
			}

			char c[50];
			itoa(count, c, 10);
			
			variables[params[0]]=c;
		}
		else if(name==ID_ALLOW_DELETE)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			
			{
				CString s;
				s="This script wants to delete objects or triggers from your map. For some scripts this may be necessary, but it can seriously damage your map. Reason why script wants to delete objects:\n";
				s+=params[0];
				s+="\n\nDo you want to allow the script to do this?";

				int res=MessageBox(s, "Allow deletion of objects?", MB_YESNO | MB_DEFBUTTON2);
				if(res==IDNO) goto nextline;
			}

			bDeleteAllowed=TRUE;
		}
		else if(name==ID_ALLOW_ADD)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			
			{
				CString s;
				s="This script wants to add objects or triggers to your map. For some scripts this may be necessary, but it can seriously damage your map. Reason why script wants to add objects:\n";
				s+=params[0];
				s+="\n\nDo you want to allow the script to do this?";

				int res=MessageBox(s, "Allow adding of objects?", MB_YESNO | MB_DEFBUTTON2);
				if(res==IDNO) goto nextline;
			}

			bAddAllowed=TRUE;
		}
		else if(name==ID_DELETE_TERRAIN)
		{ 
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}
			
			if(!bDeleteAllowed ) goto nextline;

			int index=atoi(params[0]);
			if(index<0 || index>=Map->GetTerrainCount()) 
			{
				report+="Terrain deletion failed, invalid index\r\n";
				goto nextline;
			}

			lastTerrainDeleted=index;
			Map->DeleteTerrain(index);

			report+="Terrain deleted\r\n";

			bUpdate=TRUE;
			bUpdateOnlyMission=FALSE;
		}
		else if(name==ID_DELETE_INFANTRY)
		{ 
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}
			
			if(!bDeleteAllowed ) goto nextline;

			int index=atoi(params[0]);
			if(index<0 || index>=Map->GetInfantryCount()) 
			{
				report+="Infantry deletion failed, invalid index\r\n";
				goto nextline;
			}

			lastInfantryDeleted=index;
			Map->DeleteInfantry(index);

			report+="Infantry deleted\r\n";

			bUpdate=TRUE;
			bUpdateOnlyMission=FALSE;
		}
		else if(name==ID_DELETE_STRUCTURE)
		{ 
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}
			
			if(!bDeleteAllowed ) goto nextline;

			int index=atoi(params[0]);
			if(index<0 || index>=Map->GetStructureCount()) 
			{
				report+="Structure deletion failed, invalid index\r\n";
				goto nextline;
			}

			lastStructureDeleted=*ini.sections["Structures"].GetValueName(index);
			Map->DeleteStructure(index);

			report+="Structure deleted\r\n";

			bUpdate=TRUE;
			bUpdateOnlyMission=FALSE;
		}
		else if(name==ID_DELETE_VEHICLE)
		{ 
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}
			
			if(!bDeleteAllowed ) goto nextline;

			int index=atoi(params[0]);
			if(index<0 || index>=Map->GetUnitCount()) 
			{
				report+="Vehicle deletion failed, invalid index\r\n";
				goto nextline;
			}

			lastUnitDeleted=*ini.sections["Units"].GetValueName(index);
			Map->DeleteUnit(index);

			report+="Vehicle deleted\r\n";

			bUpdate=TRUE;
			bUpdateOnlyMission=FALSE;
		}
		else if(name==ID_DELETE_AIRCRAFT)
		{ 
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}
			
			
			if(!bDeleteAllowed ) goto nextline;

			int index=atoi(params[0]);
			if(index<0 || index>=Map->GetAircraftCount()) 
			{
				report+="Aircraft deletion failed, invalid index\r\n";
				goto nextline;
			}
			
			lastAircraftDeleted=*ini.sections["Aircraft"].GetValueName(index);
			Map->DeleteAircraft(index);

			report+="Aircraft deleted\r\n";

			bUpdate=TRUE;
			bUpdateOnlyMission=FALSE;
		}
		else if(name==ID_IS_INFANTRY_DELETED)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			CString deleted="1";
			int index=atoi(params[1]);
			if(index>=0 && index<Map->GetInfantryCount())
			{
				INFANTRY id;
				Map->GetInfantryData(index, &id);
				if(id.deleted==0) deleted="0";
			}

			variables[params[0]]=deleted;
		}
		else if(name==ID_IS_TERRAIN_DELETED)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			CString deleted="1";
			int index=atoi(params[1]);
			if(index>=0 && index<Map->GetTerrainCount())
			{
				TERRAIN id;
				Map->GetTerrainData(index, &id);
				if(id.deleted==0) deleted="0";
			}

			variables[params[0]]=deleted;
		}
		else if(name==ID_ADD_INFANTRY)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			// check param count
			int count=1; // we start at 1 param even if no , exists!
			int k;
			for(k=0;k<params[0].GetLength();k++)
			{
				if(params[0].GetAt(k)==',') count++;				
			}

			if(count!=14)
			{
				report+="AddInfantry failed\r\n";
				goto nextline;
			}

			CString data=params[0];
				
			INFANTRY id;
			id.deleted=0;
			id.house=GetParam(data, 0);	
			id.type=GetParam(data, 1);
			id.strength=GetParam(data, 2);
			id.y=GetParam(data, 3);
			id.x=GetParam(data, 4);
			//id.pos=GetParam(data, 5);
			id.pos="-1"; // ignore pos values!
			id.action=GetParam(data, 6);
			id.direction=GetParam(data, 7);
			id.tag=GetParam(data, 8);
			id.flag1=GetParam(data, 9);
			id.flag2=GetParam(data, 10);
			id.flag3=GetParam(data, 11);
			id.flag4=GetParam(data, 12);
			id.flag5=GetParam(data, 13);

			if(Map->AddInfantry(&id, NULL, NULL, NULL, lastInfantryDeleted)==FALSE)
			{
				report+="AddInfantry failed\r\n";
			}
			else
			{
				report+="Infantry added\r\n";
				bUpdate=TRUE;
				bUpdateOnlyMission=FALSE;
			}
		}
		else if(name==ID_ADD_VEHICLE)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			// check param count
			int count=1; // we start at 1 param even if no , exists!
			int k;
			for(k=0;k<params[0].GetLength();k++)
			{
				if(params[0].GetAt(k)==',') count++;				
			}

			if(count!=14)
			{
				report+="AddVehicle failed\r\n";
				goto nextline;
			}

			CString data=params[0];
				
			UNIT unit;
			unit.house=GetParam(data, 0);	
			unit.type=GetParam(data, 1);
			unit.strength=GetParam(data, 2);
			unit.y=GetParam(data, 3);
			unit.x=GetParam(data, 4);
			unit.direction=GetParam(data, 5);
			unit.action=GetParam(data, 6);
			unit.tag=GetParam(data, 7);
			unit.flag1=GetParam(data, 8);
			unit.flag2=GetParam(data, 9);
			unit.flag3=GetParam(data, 10);
			unit.flag4=GetParam(data, 11);
			unit.flag5=GetParam(data, 12);
			unit.flag6=GetParam(data, 13);

			if(Map->GetUnitAt(atoi(unit.x)+atoi(unit.y)*Map->GetIsoSize())>=0)
			{
				report+="AddVehicle failed\r\n";
				goto nextline;
			}

			if(Map->AddUnit(&unit, NULL, NULL, NULL, lastUnitDeleted)==FALSE)
			{
				report+="AddVehicle failed\r\n";
			}
			else
			{
				report+="Vehicle added\r\n";
				bUpdate=TRUE;
				bUpdateOnlyMission=FALSE;
			}
		}
		else if(name==ID_ADD_AIRCRAFT)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			// check param count
			int count=1; // we start at 1 param even if no , exists!
			int k;
			for(k=0;k<params[0].GetLength();k++)
			{
				if(params[0].GetAt(k)==',') count++;				
			}

			if(count!=12)
			{
				report+="AddAircraft failed\r\n";
				goto nextline;
			}

			CString data=params[0];
				
			AIRCRAFT air;
			air.house=GetParam(data, 0);	
			air.type=GetParam(data, 1);
			air.strength=GetParam(data, 2);
			air.y=GetParam(data, 3);
			air.x=GetParam(data, 4);
			air.direction=GetParam(data, 5);
			air.action=GetParam(data, 6);
			air.tag=GetParam(data, 7);
			air.flag1=GetParam(data, 8);
			air.flag2=GetParam(data, 9);
			air.flag3=GetParam(data, 10);
			air.flag4=GetParam(data, 11);

			if(Map->GetAirAt(atoi(air.x)+atoi(air.y)*Map->GetIsoSize())>=0)
			{
				report+="AddAircraft failed\r\n";
				goto nextline;
			}

			if(Map->AddAircraft(&air, NULL, NULL, NULL, lastAircraftDeleted)==FALSE)
			{
				report+="AddAircraft failed\r\n";
			}
			else
			{
				report+="Aircraft added\r\n";
				bUpdate=TRUE;
				bUpdateOnlyMission=FALSE;
			}
		}
		else if(name==ID_ADD_STRUCTURE)
		{
			if(paramcount<1) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>1)
			{
				if(params[1].GetLength()>0)
				{
					if(!IsValSet(params[1])) goto nextline;
				}
			}

			// check param count
			int count=1; // we start at 1 param even if no , exists!
			int k;
			for(k=0;k<params[0].GetLength();k++)
			{
				if(params[0].GetAt(k)==',') count++;				
			}

			if(count!=17)
			{
				report+="AddStructure failed\r\n";
				goto nextline;
			}

			CString data=params[0];
				
			STRUCTURE structure;
			structure.house=GetParam(data, 0);	
			structure.type=GetParam(data, 1);
			structure.strength=GetParam(data, 2);
			structure.y=GetParam(data, 3);
			structure.x=GetParam(data, 4);
			structure.direction=GetParam(data, 5);
			structure.tag=GetParam(data, 6);
			structure.flag1=GetParam(data, 7);
			structure.flag2=GetParam(data, 8);
			structure.energy=GetParam(data, 9);
			structure.upgradecount=GetParam(data, 10);
			structure.spotlight=GetParam(data, 11);
			structure.upgrade1=GetParam(data, 12);
			structure.upgrade2=GetParam(data, 13);
			structure.upgrade3=GetParam(data, 14);
			structure.flag3=GetParam(data, 15);
			structure.flag4=GetParam(data, 16);

			if(Map->GetStructureAt(atoi(structure.x)+atoi(structure.y)*Map->GetIsoSize())>=0)
			{
				report+="AddStructure failed\r\n";
				goto nextline;
			}

			if(Map->AddStructure(&structure, NULL, NULL, NULL, lastStructureDeleted)==FALSE)
			{
				report+="AddStructure failed\r\n";
			}
			else
			{
				report+="Structure added\r\n";
				bUpdate=TRUE;
				bUpdateOnlyMission=FALSE;
			}
		}
		else if(name==ID_GET_INFANTRY)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int index=atoi(params[1]);
			CString s;
			if(index>=0 && index<Map->GetInfantryCount())
			{
				INFANTRY id;
				Map->GetInfantryINIData(index, &s);
			}

			variables[params[0]]=s;
		}
		else if(name==ID_GET_AIRCRAFT)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int index=atoi(params[1]);
			CString s;
			if(index>=0 && index<Map->GetAircraftCount())
			{
				s=*ini.sections["Aircraft"].GetValue(index);
			}

			variables[params[0]]=s;
		}
		else if(name==ID_GET_STRUCTURE)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{ 
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int index=atoi(params[1]);
			CString s;
			if(index>=0 && index<Map->GetStructureCount())
			{
				s=*ini.sections["Structures"].GetValue(index);
			}

			variables[params[0]]=s;
		}
		else if(name==ID_GET_VEHICLE)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int index=atoi(params[1]);
			CString s;
			if(index>=0 && index<Map->GetUnitCount())
			{
				s=*ini.sections["Units"].GetValue(index);
			}

			variables[params[0]]=s;
		}
		else if(name==ID_GET_HOUSE)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int index=atoi(params[1]);
			
			CString s;
			if(index>=0 && index<Map->GetHousesCount(FALSE))
			{
				s=Map->GetHouseID(index, FALSE);
			}

			variables[params[0]]=s;
		}
		else if(name==ID_GET_COUNTRY)
		{
			if(paramcount<2) 
			{
				ReportScriptError(i);
				delete[] params;
				break;
			}

			// check bool
			if(paramcount>2)
			{
				if(params[2].GetLength()>0)
				{
					if(!IsValSet(params[2])) goto nextline;
				}
			}

			int index=atoi(params[1]);
			
			CString s;
			if(index>=0 && index<Map->GetHousesCount(TRUE))
			{
				s=Map->GetHouseID(index, TRUE);
			}

			variables[params[0]]=s;
		}
		
		else
		{
			ReportScriptError(i);
			delete[] params;
			break;
		}
		
nextline:
		if(bAutoUpdate)
		{			
			m_Report=report;
			UpdateData(FALSE);
		}

nextline_no_update:

		delete[] params;	
		
	}

	for(i=0;i<functions.size();i++)
	{
		delete[] functions[i].params;
		functions[i].params=NULL;
	}

	m_Report=report;
	UpdateData(FALSE);
	
	//if(bUpdateWaypoints) Map->UpdateIniFile(MAPDATA_UPDATE_FROM_INI);

	if(bUpdate) ((CFinalSunDlg*)theApp.GetMainWnd())->UpdateDialogs(bUpdateOnlyMission, bNoRepos);
	
	// CDialog::OnOK();
}

BOOL CUserScriptsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int k;
	CFileFind ff;


	CString scripts=(CString)AppPath+"\\Scripts\\*.fscript";
	{
		
		CFileFind ff;
		if(ff.FindFile(scripts))
		{
			CListBox* lb=(CListBox*)GetDlgItem(IDC_SCRIPTS);
			
			BOOL bWorking=TRUE;
			
			while(bWorking)  
			{
				bWorking=ff.FindNextFile();						
				lb->AddString(ff.GetFileName());				
			}
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CUserScriptsDlg::ReportScriptError(int line)
{
	char c[50];
	itoa(line, c, 10);

	MessageBox("Script error in line " + (CString)c + ". Probably wrong parameter count or unknown function call.","Error");
}
