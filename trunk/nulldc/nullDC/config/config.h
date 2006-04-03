#pragma once

#include "types.h"

/*
**	cfg* prototypes, if you pass NULL to a cfgSave* it will wipe out the section
**	} if you pass it to lpKey it will wipe out that particular entry
**	} if you add write to something it will create it if its not present
**	} ** Strings passed to LoadStr should be MAX_PATH in size ! **
*/

bool cfgVerify();
s32  cfgLoadInt(const char * lpSection, const char * lpKey);
void cfgSaveInt(const char * lpSection, const char * lpKey, s32 Int);
void cfgLoadStr(const char * lpSection, const char * lpKey, char * lpReturn);
void cfgSaveStr(const char * lpSection, const char * lpKey, const char * lpString);


