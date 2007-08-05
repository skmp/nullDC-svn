#pragma once
#include "types.h"

#ifdef BUILD_DREAMCAST
#define RAM_SIZE (16*1024*1024)
#else
#define RAM_SIZE (32*1024*1024)
#endif


#define RAM_MASK (RAM_SIZE-1)

#ifndef BUILD_DEV_UNIT
#define BIOS_SIZE (2*1024*1024)
#else
#define BIOS_SIZE (4*1024*1024)
#endif
#define BIOS_MASK (BIOS_SIZE-1)


#define FLASH_SIZE (128*1024)
#define FLASH_MASK (FLASH_SIZE-1)

//main system mem
extern VArray2 mem_b;

//bios rom
extern Array<u8> bios_b;

//flash rom
extern Array<u8> flash_b;

#define MEMCALL __fastcall

#include "_vmem.h"
#include "mmu.h"

#ifdef NO_MMU
#define ReadMem8 _vmem_ReadMem8
#define ReadMem16 _vmem_ReadMem16
#define IReadMem16 ReadMem16
#define ReadMem32 _vmem_ReadMem32
#define ReadMem64(addr,reg) {  (reg)=_vmem_ReadMem32(addr);(&(reg))[1]=_vmem_ReadMem32((addr)+4); }

#define WriteMem8 _vmem_WriteMem8
#define WriteMem16 _vmem_WriteMem16
#define WriteMem32 _vmem_WriteMem32
#define WriteMem64(addr,reg) {  _vmem_WriteMem32(addr,(reg));_vmem_WriteMem32((addr)+4,(&(reg))[1]); }
#else
#define ReadMem8 mmu_ReadMem8
#define ReadMem16 mmu_ReadMem16
#define IReadMem16 mmu_IReadMem16
#define ReadMem32 mmu_ReadMem32
#define ReadMem64 mmu_ReadMem64

#define WriteMem8 mmu_WriteMem8
#define WriteMem16 mmu_WriteMem16
#define WriteMem32 mmu_WriteMem32
#define WriteMem64 mmu_WriteMem64
#endif


#define ReadMem8_nommu _vmem_ReadMem8
#define ReadMem16_nommu _vmem_ReadMem16
#define IReadMem16_nommu _vmem_IReadMem16
#define ReadMem32_nommu _vmem_ReadMem32


#define WriteMem8_nommu _vmem_WriteMem8
#define WriteMem16_nommu _vmem_WriteMem16
#define WriteMem32_nommu _vmem_WriteMem32

void MEMCALL WriteMemBlock_ptr(u32 dst,u32* src,u32 size);
void MEMCALL WriteMemBlock_nommu_ptr(u32 dst,u32* src,u32 size);
void MEMCALL WriteMemBlock_nommu_dma(u32 dst,u32 src,u32 size);
//Init/Res/Term
void mem_Init();
void mem_Term();
void mem_Reset(bool Manual);
void mem_map_defualt();

//Generic read/write functions for debugger
bool ReadMem_DB(u32 addr,u32& data,u32 size );
bool WriteMem_DB(u32 addr,u32 data,u32 size );

//Get pointer to ram area , 0 if error
//For debugger(gdb) - dynarec
u8* GetMemPtr(u32 Addr,u32 size);

//Get infomation about an area , eg ram /size /anything
//For dynarec - needs to be done
struct MemInfo
{
	//MemType:
	//Direct ptr   , just read/write to the ptr
	//Direct call  , just call for read , ecx=data on write (no address)
	//Generic call , ecx=addr , call for read , edx=data for write
	u32 MemType;		
	
	//todo
	u32 Flags;

	void* read_ptr;
	void* write_ptr;
};

void GetMemInfo(u32 addr,u32 size,MemInfo* meminfo);

bool IsOnRam(u32 addr);


u32 __fastcall GetRamPageFromAddress(u32 RamAddress);

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