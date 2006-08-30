/*
**	gdrom_if.h
*/
#ifndef __GDROM_IF_H__
#define __GDROM_IF_H__

#include "types.h"
#include "plugins/plugin_manager.h"

#define ZGDROM
//#define OLD_GDROM
//#define TEST_GDROM

#ifdef OLD_GDROM
#include "gdromv2_old.h"
#endif

#ifdef ZGDROM
#include "zGDROM.h"
#endif


void gdrom_reg_Init(void);
void gdrom_reg_Term(void);
void gdrom_reg_Reset(bool Manual);

u32  ReadMem_gdrom(u32 Addr, u32 sz);
void WriteMem_gdrom(u32 Addr, u32 data, u32 sz);
void NotifyEvent_gdrom(DriveEvent info,void* param);

bool gdBootHLE();// Load Scrambled binary off of Current CD Media and boot ip.bin



#endif //__GDROM_IF_H__
