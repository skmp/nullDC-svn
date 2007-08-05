#define _CRT_SECURE_NO_DEPRECATE (1)
#include <windows.h>
#include "config.h"


char appPath[512];
char pluginPath[512];
char dataPath[512];
char cfgPath[512];

#define CEM_VIRTUAL 1
#define CEM_VALIDCV 2
#define CEM_READONLY 4
#define CEM_VALIDSV 8
struct ConfigEntry
{
	ConfigEntry(ConfigEntry* pp)
	{
		next=pp;
		flags=0;
		name=0;
		value=0;
		value2=0;
	}


	u32 flags;
	char* name;
	char* value;
	char* value2;
	ConfigEntry* next;
	void SaveFile(FILE* file)
	{
		if (!(flags&(CEM_VALIDCV|CEM_VALIDSV)))
			return;
		if (flags & CEM_READONLY)
			return;

		fprintf(file,"%s=%s\n",name,value);
	} 
	~ConfigEntry()
	{
		if (name)
			free(name);
		if(value)
			free(value);
		if(value2)
			free(value2);
	}
	void GetValue(char* v)
	{
		if (flags & CEM_VALIDCV)
			strcpy(v,value);
		else if (flags & CEM_VIRTUAL)
			strcpy(v,value2);
		else if (flags & CEM_VALIDSV)
			strcpy(v,value);
		else
			strcpy(v,"");
	}
};
struct ConfigSection
{
	u32 flags;
	char* name;
	ConfigEntry* entrys;
	ConfigSection* next;
	
	ConfigSection(ConfigSection* pp)
	{
		next=pp;
		flags=0;
		entrys=0;
		name=0;
	}
	ConfigEntry* FindEntry(const char* name)
	{
		ConfigEntry* c=	entrys;
		while(c)
		{
			if (stricmp(name,c->name)==0)
				return c;
			c=c->next;
		}
		return 0;
	}
	void SetEntry(const char* name,const char* value,u32 eflags)
	{
		ConfigEntry* c=FindEntry(name);
		if (!c)
		{
			entrys=c= new ConfigEntry(entrys);
			c->name=strdup(name);
		}

		//readonly is read only =)
		if (c->flags & CEM_READONLY)
			return;
		
		//virtual : save only if different value
		if (c->flags & CEM_VIRTUAL)
		{
			if(strcmpi(c->value2,value)==0)
				return;
		}

		if (eflags & CEM_VIRTUAL)
		{
			c->flags|=CEM_VIRTUAL;
			if (c->value2)
				free(c->value2);
			c->value2=strdup(value);
		}
		else if (eflags & CEM_VALIDSV)
		{
			flags|=CEM_VALIDCV;
			c->flags|=CEM_VALIDSV;
			if (c->value)
				free(c->value);
			c->value=strdup(value);
		}
		else
		{
			flags|=CEM_VALIDCV;
			c->flags|=CEM_VALIDCV;
			if (c->value)
				free(c->value);
			c->value=strdup(value);
		}
		
	}
	~ConfigSection()
	{
		ConfigEntry* n=entrys;
		
		while(n)
		{
			ConfigEntry* p=n;	
			n=n->next;
			delete p;
		}
	}
	void SaveFile(FILE* file)
	{
		if (!(flags&(CEM_VALIDCV|CEM_VALIDSV)))
			return;
		
		fprintf(file,"[%s]\n",name);
	
		vector<ConfigEntry*> stuff;

		ConfigEntry* n=entrys;
		
		while(n)
		{
			stuff.push_back(n);
			n=n->next;
		}

		for (int i=stuff.size()-1;i>=0;i--)
		{
			stuff[i]->SaveFile(file);
		}

		fprintf(file,"\n",name);
	}

};
struct ConfigFile
{
	ConfigSection* entrys;
	ConfigSection* FindSection(const char* name)
	{
		ConfigSection* c=	entrys;
		while(c)
		{
			if (stricmp(name,c->name)==0)
				return c;
			c=c->next;
		}
		return 0;
	}
	ConfigSection* GetEntry(const char* name,u32 flags)
	{
		ConfigSection* c=FindSection(name);
		if (!c)
		{
			entrys=c= new ConfigSection(entrys);
			c->name=strdup(name);
		}

		if (flags & CEM_VIRTUAL)
		{
			c->flags|=CEM_VIRTUAL;
		}
		/*else
		{
			c->flags|=CEM_VALIDCV;
		}*/
		return c;
	}
	~ConfigFile()
	{
		ConfigSection* n=entrys;
		
		while(n)
		{
			ConfigSection* p=n;	
			n=n->next;
			delete p;
		}
	}

	void PaseFile(FILE* file)
	{
		
	}
	void SaveFile(FILE* file)
	{
		fprintf(file,";; nullDC config file;;\n");
		vector<ConfigSection*> stuff;

		ConfigSection* n=entrys;
		
		while(n)
		{
			stuff.push_back(n);
			n=n->next;
		}

		for (int i=stuff.size()-1;i>=0;i--)
		{
			stuff[i]->SaveFile(file);
		}
	}
};

ConfigFile cfgdb;

void savecfgf()
{
	FILE* cfgfile = fopen(cfgPath,"wt");
	if (!cfgfile)
		printf("Error : Unable to open file for saving \n");
	else
	{
		cfgdb.SaveFile(cfgfile);
		fclose(cfgfile);
	}
}
void EXPORT_CALL cfgSaveStr(const char * Section, const char * Key, const char * String)
{
	cfgdb.GetEntry(Section,CEM_VALIDCV)->SetEntry(Key,String,CEM_VALIDCV);
	savecfgf();
	//WritePrivateProfileString(Section,Key,String,cfgPath);
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
char* trim_ws(char* str);
bool cfgOpen()
{
	char * tmpPath = GetEmuPath("");
	strcpy(appPath, tmpPath);
	free(tmpPath);

	if (cfgPath[0]==0)
		sprintf(cfgPath,"%snullDC.cfg", appPath);

	sprintf(dataPath,"%sdata\\", appPath);
	sprintf(pluginPath,"%splugins\\", appPath);

	ConfigSection* cs= cfgdb.GetEntry("emu",CEM_VIRTUAL);

	cs->SetEntry("AppPath",appPath,CEM_VIRTUAL | CEM_READONLY);
	cs->SetEntry("PluginPath",pluginPath,CEM_VIRTUAL | CEM_READONLY);
	cs->SetEntry("DataPath",dataPath,CEM_VIRTUAL | CEM_READONLY);
	cs->SetEntry("FullName",VER_FULLNAME,CEM_VIRTUAL | CEM_READONLY);
	cs->SetEntry("ShortName",VER_SHORTNAME,CEM_VIRTUAL | CEM_READONLY);
	cs->SetEntry("Name",VER_EMUNAME,CEM_VIRTUAL | CEM_READONLY);

	FILE* cfgfile = fopen(cfgPath,"r");
	if(!cfgfile) {
		cfgfile = fopen(cfgPath,"wt");
		if(!cfgfile) 
			printf("Unable to open the config file for reading or writing\nfile : %s\n",cfgPath);
		else
		{
			fprintf(cfgfile,";; nullDC cfg file ;;\n\n");
			fseek(cfgfile,0,SEEK_SET);
			fclose(cfgfile);
			cfgfile = fopen(cfgPath,"r");
			if(!cfgfile) 
				printf("Unable to open the config file for reading\nfile : %s\n",cfgPath);
		}
	}

	char line[512];
	char cur_sect[512]={0};
	int cline=0;
	while(cfgfile && !feof(cfgfile))
	{
		cline++;
		fgets(line,512,cfgfile);
		if (strlen(line)<3)
			continue;
		if (line[strlen(line)-1]=='\r' || line[strlen(line)-1]=='\n')
			line[strlen(line)-1]=0;

		char* tl=trim_ws(line);
		if (tl[0]=='[' && tl[strlen(tl)-1]==']')
		{
			tl[strlen(tl)-1]=0;
			strcpy(cur_sect,tl+1);
			trim_ws(cur_sect);
		}
		else
		{
			if (cur_sect[0]==0)
				continue;//no open section
			char* str1=strstr(tl,"=");
			if (!str1)
			{
				printf("Malformed entry on cfg,  ingoring @ %d(%s)\n",cline,tl);
				continue;
			}
			*str1=0;
			str1++;
			char* v=trim_ws(str1);
			char* k=trim_ws(tl);
			if (v && k)
			{
				ConfigSection*cs=cfgdb.GetEntry(cur_sect,CEM_VIRTUAL);
				
				//if (!cs->FindEntry(k))
				cs->SetEntry(k,v,CEM_VALIDSV);
			}
			else
			{
				printf("Malformed entry on cfg,  ingoring @ %d(%s)\n",cline,tl);
			}
		}
	}

	if (cfgfile)
	{
		cfgdb.SaveFile(cfgfile);
		fclose(cfgfile);
	}
	return true;
}

//Implementations of the interface :)
//Section must be set
//If key is 0 , it looks for the section
//0 : not found
//1 : found section , key was 0
//2 : found section & key
s32 EXPORT_CALL cfgExists(const char * Section, const char * Key)
{
	if (Section==0)
		return -1;
	//return cfgRead(Section,Key,0);
	ConfigSection*cs= cfgdb.FindSection(Section);
	if (cs ==  0)
		return 0;

	if (Key==0)
		return 1;

	ConfigEntry* ce=cs->FindEntry(Key);
	if (ce!=0)
		return 2;
	else
		return 0;
}
void EXPORT_CALL cfgLoadStr(const char * Section, const char * Key, char * Return,const char* Default)
{
	verify(Section!=0 && strlen(Section)!=0);
	verify(Key!=0 && strlen(Key)!=0);
	verify(Return!=0);
	if (Default==0)
		Default="";
	ConfigSection* cs= cfgdb.GetEntry(Section,CEM_VALIDCV);
	ConfigEntry* ce=cs->FindEntry(Key);
	if (!ce)
	{
		cs->SetEntry(Key,Default,CEM_VALIDCV);
		strcpy(Return,Default);
	}
	else
	{
		ce->GetValue(Return);
	}
}

//These are helpers , mainly :)
s32 EXPORT_CALL cfgLoadInt(const char * Section, const char * Key,s32 Default)
{
	char temp_d[30];
	char temp_o[30];
	sprintf(temp_d,"%d",Default);
	cfgLoadStr(Section,Key,temp_o,temp_d);
	return atoi(temp_o);
}

void EXPORT_CALL cfgSaveInt(const char * Section, const char * Key, s32 Int)
{
	char tmp[32];
	sprintf(tmp,"%d", Int);
	cfgSaveStr(Section,Key,tmp);
}
void cfgSetVitual(const char * Section, const char * Key, const char * String)
{
	cfgdb.GetEntry(Section,CEM_VIRTUAL)->SetEntry(Key,String,CEM_VIRTUAL);
}