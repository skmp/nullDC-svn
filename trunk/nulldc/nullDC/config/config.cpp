#define _CRT_SECURE_NO_DEPRECATE (1)
#include <windows.h>
#include "config.h"


static char appPath[MAX_PATH];
static char pluginPath[MAX_PATH];
static char dataPath[MAX_PATH];
static char cfgPath[MAX_PATH];


void FASTCALL cfgSaveStr(const char * Section, const char * Key, const char * String)
{
	WritePrivateProfileString(Section,Key,String,cfgPath);
}
//New config code

/*
	I want config to be realy flexible .. so , here is the new implementation :
	
	Functions :
	cfgLoadInt	: Load an int , if it does not exist save the default value to it and return it
	cfgSaveInt	: Save an int
	cfgLoadStr	: Load a str , if it does not exist save the default value to it and return it
	cfgSaveStr	: Save a str
	cfgExists	: Returns true if the Section:Key exists. If Key is null , it retuns true if Section exists

	Config parameters can be readed from the config file , and can be given at the command line
	-cfg section:key=value -> defines a value at command line
	If a cfgSave* is made on a value defined by command line , then the command line value is replaced by it

	cfg values set by command line are not writen to the cfg file , unless a cfgSave* is used

	There are some special values , all of em are on the emu namespace :)

	These are readonly :

	emu:AppPath		: Returns the path where the emulator is stored
	emu:PluginPath	: Returns the path where the plugins are loaded from
	emu:DataPath	: Returns the path where the bios/data files are

	emu:FullName	: str,returns the emulator's name + version string (ex."nullDC v1.0.0 Private Beta 2 built on {datetime}")
	emu:ShortName	: str,returns the emulator's name + version string , short form (ex."nullDC 1.0.0pb2")
	emu:Name		: str,returns the emulator's name (ex."nullDC")

	These are read/write
	emu:Caption		: str , get/set the window caption
*/

///////////////////////////////
/*
**	This will verify there is a working file @ ./szIniFn
**	- if not present, it will write defaults
*/
bool cfgOpen()
{

	char * tmpPath = GetEmuPath("");
	strcpy(appPath, tmpPath);
	free(tmpPath);

	sprintf(cfgPath,"%snullDC.cfg", appPath);
	sprintf(dataPath,"%sdata\\", appPath);
	sprintf(pluginPath,"%splugins\\", appPath);

	FILE * fcfg = fopen(cfgPath,"r");
	if(!fcfg) {
		fcfg = fopen(cfgPath,"wt");
		fprintf(fcfg,";; nullDC cfg file ;;\n\n");
	}
	fclose(fcfg);

	//temp
	cfgSaveStr("emu","AppPath",appPath);
	cfgSaveStr("emu","PluginPath",pluginPath);
	cfgSaveStr("emu","DataPath",dataPath);

	cfgSaveStr("emu","FullName",VER_FULLNAME);
	cfgSaveStr("emu","ShortName",VER_SHORTNAME);
	cfgSaveStr("emu","Name",VER_EMUNAME);

	return true;
}
char* trim_str(char* str,char ch)
{
	if (str==0 || strlen(str)==0)
		return 0;

	while(*str)
	{
		if (*str!=ch)
			break;
		str++;
	}

	size_t l=strlen(str);
	
	if (l==0)
		return 0;

	while(l>0)
	{
		if (str[l-1]!=ch)
			break;
		str[l-1]=0;
		l--;
	}

	if (l==0)
		return 0;

	return str;
}

char* chop_back_str(char* str,char ch)
{
	size_t len=strlen(str);
	if (str==0 || len==0)
		return 0;

	while(len>0)
	{
		if (str[len-1]==ch)
		{
			str[len-1]=0;
			len--;
			break;
		}
		str[len-1]=0;
		len--;
	}

	if (len==0)
		return 0;

	return str;
}
bool front_str(char* str,char ch)
{
	if (str==0 || strlen(str)==0)
		return false;
	while(*str)
	{
		if (*str==ch)
		{
			*str=0;
			break;
		}
		str++;
	}
	return true;
}
u32 strcmp_ic(const char* p1,const char* p2)
{
	char temp1[4096];
	char temp2[4096];
	strcpy(temp1,p1);
	strcpy(temp2,p2);

	_strlwr(temp1);
	_strlwr(temp2);
	return strcmp(temp1,temp2);
}
bool FindSection(const char* name,FILE* file)
{
	if (!file)
		return false;
	char temp[512];

	while (char* ptr=fgets(temp,512,file))
	{
		size_t len=strlen(ptr);
		if (len<2)
			continue;
		//remove \n
		ptr[len-1]=0;
		
		if (!front_str(ptr,';'))
			continue;	//no text

		if(!(ptr=trim_str(ptr,' ')))
			continue;
		if (ptr[0]!='[')
			continue;	//we do not care 
		ptr++;

		//must have ] , with non zero size
		if (!front_str(ptr,']'))
			continue;	//no text
		//must have non zero trimmed size
		if(!(ptr=trim_str(ptr,' ')))
			continue;
		
		if (strcmp_ic(name,ptr)==0)
		{
			return true;	//found it !
		}
	}
	return false;
}

bool FindKey(const char* name,FILE* file,char* out)
{
	if (!file)
		return false;

	char temp[512];

	while (char* ptr=fgets(temp,512,file))
	{
		size_t len=strlen(ptr);
		if (len<2)
			continue;
		//remove \n
		ptr[len-1]=0;
		
		if (!front_str(ptr,';'))
			continue;	//no text

		//trim
		if(!(ptr=trim_str(ptr,' ')))
			continue;
		if (*ptr=='[')
			return false;//got to next section allready ...

		char* middle=strstr(ptr,"=");
		if (!middle)
			continue;
		if (!front_str(ptr,'='))
			continue;	//no text

		if(!(ptr=trim_str(ptr,' ')))
			continue;

		if(strcmp_ic(ptr,name)==0)
		{
			if (out==0)
				return true;
			middle++;
			if(!(middle=trim_str(middle,' ')))
			{
				strcpy(out,"");//exists , but it is not set
			}
			else
			{
				strcpy(out,middle);
			}
			return true;	 
		}
		
		

	}
	return false;
}
s32 cfgRead(const char * Section, const char * Key,char* value)
{
	FILE* f=fopen(cfgPath,"r");
	if (!f)
		return 0;
	if (FindSection(Section,f))
	{
		if (!Key)
		{
			fclose(f);
			return 1;
		}
		else
		{
			if (FindKey(Key,f,value))
			{
				fclose(f);
				return 2;
			}
			else
			{
				fclose(f);
				return 0;
			}
		}
	}
	fclose(f);
	return 0;
}
//Implementations of the interface :)
//Section must be set
//If key is 0 , it looks for the function
//0 : not found
//1 : found section , key was 0
//2 : found section & key
s32 FASTCALL cfgExists(const char * Section, const char * Key)
{
	return cfgRead(Section,Key,0);
}
void FASTCALL cfgLoadStr(const char * Section, const char * Key, char * Return,const char* Default)
{
	verify(Section!=0 && strlen(Section)!=0);
	verify(Key!=0 && strlen(Key)!=0);
	verify(Return!=0);
	if (Default==0)
		Default="";
	if (!cfgRead(Section,Key,Return))
	{
		cfgSaveStr(Section,Key,Default);
		verify(cfgRead(Section,Key,Return)==2);
	}
	//GetPrivateProfileString(lpSection,lpKey,lpDefault,lpReturn,MAX_PATH,cfgPath);
}

//These are helpers , mainly :)
s32 FASTCALL cfgLoadInt(const char * Section, const char * Key,s32 Default)
{
	char temp_d[30];
	char temp_o[30];
	sprintf(temp_d,"%d",Default);
	cfgLoadStr(Section,Key,temp_o,temp_d);
	return atoi(temp_o);
}

void FASTCALL cfgSaveInt(const char * Section, const char * Key, s32 Int)
{
	char tmp[32];
	sprintf(tmp,"%d", Int);
	cfgSaveStr(Section,Key,tmp);
}

