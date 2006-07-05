#pragma once
#include "types.h"

#ifndef BUILD_NAOMI
#define RAM_SIZE (16*1024*1024)
#else
#define RAM_SIZE (32*1024*1024)
#endif


#define RAM_MASK (RAM_SIZE-1)


#define BIOS_SIZE (2*1024*1024)
#define BIOS_MASK (BIOS_SIZE-1)

#define FLASH_SIZE (128*1024)
#define FLASH_MASK (FLASH_SIZE-1)

//main system mem
extern VArray mem_b;

//bios rom
extern Array<u8> bios_b;

//flash rom
extern Array<u8> flash_b;

#define MEMCALL __fastcall

#include "_vmem.h"

//u8 MEMCALL ReadMem8(u32 addr);
#define ReadMem8 _vmem_ReadMem8
//u16 MEMCALL ReadMem16(u32 addr);
#define ReadMem16 _vmem_ReadMem16
//u32 MEMCALL ReadMem32(u32 addr);
#define ReadMem32 _vmem_ReadMem32

//void MEMCALL WriteMem8(u32 addr,u8 data);
#define WriteMem8 _vmem_WriteMem8
//void MEMCALL WriteMem16(u32 addr,u16 data);
#define WriteMem16 _vmem_WriteMem16
//void MEMCALL WriteMem32(u32 addr,u32 data);
#define WriteMem32 _vmem_WriteMem32

void MEMCALL WriteMemBlock(u32 addr,u32* data,u32 size);

//Init/Res/Term
void mem_Init();
void mem_Term();
void mem_Reset(bool Manual);

//Generic read/write functions for debugger
bool ReadMem_DB(u32 addr,u32& data,u32 size );
bool WriteMem_DB(u32 addr,u32 data,u32 size );

//Get pointer to ram area , 0 if error
//For debugger(gdb) - dynarec
u8* GetMemPtr(u32 Addr,u32 size);

//Get infomation about an area , eg ram /size /anything
//For dynarec - needs to be done
void GetMemInfo(u32 addr,u32 size);

#define 	ReadMemArrRet(arr,addr,sz)				\
			{if (sz==1)								\
				return arr[addr];					\
			else if (sz==2)							\
				return *(u16*)&arr[addr];			\
			else if (sz==4)							\
				return *(u32*)&arr[addr];}	

#define WriteMemArrRet(arr,addr,data,sz)				\
			{if(sz==1)								\
				{arr[addr]=(u8)data;return;}				\
			else if (sz==2)							\
				{*(u16*)&arr[addr]=(u16)data;return;}		\
			else if (sz==4)							\
			{*(u32*)&arr[addr]=data;return;}}	