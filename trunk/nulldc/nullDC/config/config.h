#pragma once

#include "types.h"

/*
**	cfg* prototypes, if you pass NULL to a cfgSave* it will wipe out the section
**	} if you pass it to lpKey it will wipe out that particular entry
**	} if you add write to something it will create it if its not present
**	} ** Strings passed to LoadStr should be MAX_PATH in size ! **
*/

bool cfgVerify();
s32  FASTCALL cfgLoadInt(const char * lpSection, const char * lpKey,s32 Default);
void FASTCALL cfgSaveInt(const char * lpSection, const char * lpKey, s32 Int);
void FASTCALL cfgLoadStr(const char * lpSection, const char * lpKey, char * lpReturn,const char* lpDefault);
void FASTCALL cfgSaveStr(const char * lpSection, const char * lpKey, const char * lpString);


