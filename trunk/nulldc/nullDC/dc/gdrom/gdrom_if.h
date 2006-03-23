/*
**	gdrom_if.h
*/
#ifndef __GDROM_IF_H__
#define __GDROM_IF_H__

#include "types.h"
#include "plugins/plugin_manager.h"

#define ZGDROM
#ifndef ZGDROM
#include "gdromv3.h"
#else
#include "zGDROM.h"
#endif


void gdrom_reg_Init(void);
void gdrom_reg_Term(void);
void gdrom_reg_Reset(bool Manual);

u32  ReadMem_gdrom(u32 Addr, u32 sz);
void WriteMem_gdrom(u32 Addr, u32 data, u32 sz);
void NotifyEvent_gdrom(DriveEvent info,void* param);




#endif //__GDROM_IF_H__
