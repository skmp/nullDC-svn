#define _CRT_SECURE_NO_DEPRECATE (1)
#include <windows.h>
#include "config.h"


static char appPath[MAX_PATH];
static char cfgPath[MAX_PATH];

s32 cfgLoadInt(const char * lpSection, const char * lpKey)
{
	return GetPrivateProfileInt(lpSection,lpKey,-1,cfgPath);
}

void cfgLoadStr(const char * lpSection, const char * lpKey, const char * lpReturn)
{
	GetPrivateProfileString(lpSection,lpKey,"NULL",(LPSTR)lpReturn,MAX_PATH,cfgPath);
}

void cfgSaveInt(const char * lpSection, const char * lpKey, s32 Int)
{
	char tmp[32];
	sprintf(tmp,"%d", Int);
	WritePrivateProfileString(lpSection,lpKey,tmp,cfgPath);
}


void cfgSaveStr(const char * lpSection, const char * lpKey, const char * lpString)
{
	WritePrivateProfileString(lpSection,lpKey,lpString,cfgPath);
}





/*
**	This will verify there is a working file @ ./szIniFn
**	- if not present, it will write defaults and set needsCfg
*/
bool cfgVerify()
{
	if(0==GetCurrentDirectory(MAX_PATH,appPath)) {
		printf("\n~ERROR: cfgVerify: GetCurrentDirector() Failed!\n");
		sprintf(appPath, ".\\");
	}
	strcat(appPath,"\\");
	sprintf(cfgPath,"%snullDC.cfg", appPath);

	FILE * fcfg = fopen(cfgPath,"r");
	if(!fcfg) {
		fcfg = fopen(cfgPath,"wt");
		fprintf(fcfg,";; nullDC cfg file ;;\n\n");
	}
	fclose(fcfg);

	if(-1 == cfgLoadInt("nullDC", "magic"))
	{
		cfgSaveStr("nullDC",NULL,NULL);			// this will delete all
		cfgSaveStr("nullDC","magic","0x420");	// now write it ...

		// defaults
		cfgSaveInt("nullDC","bNeedsCfg",TRUE);	// set configure needed

		// Default Paths:
		char finalPath[MAX_PATH];
		cfgSaveStr("nullDC_paths","AppPath",appPath);	// this is nice, you can always get real curr. path with this

		sprintf(finalPath,"%sdata\\", appPath);
		cfgSaveStr("nullDC_paths","DataPath",finalPath);

		sprintf(finalPath,"%splugins\\", appPath);
		cfgSaveStr("nullDC_paths","PluginPath",finalPath);

		return false;
	}

	cfgSaveStr("nullDC_paths","AppPath",appPath);
	return true;
}


// prospect to use, and use the id to parse out what section to use..
//void cfgLoadByID(cfgID id);