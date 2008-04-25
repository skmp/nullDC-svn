/*
	sh4 memory map, based on the vmem code on nullDC with some heavy modifications to reduce memory usage.

	sh4 memmap :
	sh4 divides the mem space into 5 regions u0(2GB),p1/2/3/4 (512 MB each).
	u0 : cached, lower 29 bits are phyiscal address
	p4 : internal register address space

	physical address space :
	the sh4 has 29 bits(512 mb) of physical address space, divided into 8 areas of 64MB
	Area7 points to internal registers, other areas can be used for varius things (configurable with registers)
	On the dreamacast area0 is used for registers/bios/flash/aica, (fill here later).

	The memmap will use the same idea of 'remap' tables with special values for handlers and pointers for ram.Unlike nullDC the map
	size will be the minimum possible.The smallest region we need to support is 16 mb (for vram).

	the look up table will be 256-entry :)
*/

#include "nulldce.h"
#include <stdlib.h>
#include <stdio.h>

uptr sh4mmap_ptr[256];
uptr sh4mmap_mask[256];
u8* sh4_ram;
u8* sh4_vram;
u8* sh4_bios;
u8 sq_full[64];

void memset(void* ptr,int v,int bt)
{
	u8* p=(u8*)ptr;
	while(bt--)
		*p++=v;
}
bool sh4MemInit()
{
	sh4_ram=(u8*)malloc(RAM_SIZE);
	sh4_vram=(u8*)malloc(VRAM_SIZE);
	sh4_bios=(u8*)malloc(BIOS_SIZE);
	memset(sh4mmap_ptr,0xFFFFFFFF,sizeof(sh4mmap_ptr));
	memset(sh4mmap_mask,0xFFFFFFFF,sizeof(sh4mmap_mask));
	//REALY hacky-temporary
	for (int i=0;i<256;i++)
	{
		u32 area=(i>>2)&7;
		u32 region=(i>>5)&7;
		if (region==7)
		{
			if ((i&0xFC)==0xE0)
			{
				sh4mmap_ptr[i]=(uptr)sq_full;
				sh4mmap_mask[i]=0x3F;
			}
		}
		else
		{
			switch(area)
			{
			case 0://regs
				sh4mmap_ptr[i]=-1;
				break;
			case 1://vram--its wrong
				sh4mmap_ptr[i]=(uptr)sh4_vram;
				sh4mmap_mask[i]=VRAM_MASK;
				break;
			case 2://nothingness
				sh4mmap_ptr[i]=-1;
				break;
			case 3://ram
				sh4mmap_ptr[i]=(uptr)sh4_ram;
				sh4mmap_mask[i]=RAM_MASK;
				break;
			case 4://TA
				sh4mmap_ptr[i]=-1;
				break;
			case 5://ExtDev
				sh4mmap_ptr[i]=-1;
				break;
			case 6://nothingness
				sh4mmap_ptr[i]=-1;
				break;
			case 7://Sh4Regs
				sh4mmap_ptr[i]=-1;
				break;
			}
		}
	}
	return true;
}
bool sh4MemReset(bool phys)
{
	memset(sh4_ram,0,RAM_SIZE);
	memset(sh4_vram,0,VRAM_SIZE);
	memset(sh4_bios,0,BIOS_SIZE);
	FILE* f=fopen("3dtest.bin","rb");
	fseek(f,0,SEEK_END);
	u32 flen=ftell(f);
	fseek(f,0,SEEK_SET);
	fread(&sh4_ram[0x10000],flen,1,f);
	fclose(f);
	return true;
}
void sh4MemTerm()
{
	free(sh4_ram);
	free(sh4_vram);
	free(sh4_bios);
}

template<typename T,u32 sz>
s32 sh4MemFullRead(u32 addr)
{
	printf("sh4MemRead 0x%08X!\n",addr);
	if (0xA05F810C==addr)
	{
		static u32 rv=0;
		rv^=1;
		return rv;
	}
	return 0;
}

template<typename T,u32 sz>
s32 sh4MemFullWrite(u32 addr,T data)
{
	printf("sh4MemWrite 0x%08X!\n",addr);
	return 0;
}

template<typename T,u32 sz>
s32 inline sh4MR(u32 addr)
{
	u32 upper=addr>>24;
	uptr ptr=sh4mmap_ptr[upper];
	if ((s32)ptr<0)
		return sh4MemFullRead<T,sz>(addr);
	addr&=sh4mmap_mask[upper];
	u8* ptrn=(u8*)ptr;
	return *(T*)&ptrn[addr];
}

template<typename T,u32 sz>
void inline sh4MW(u32 addr,T data)
{
	u32 upper=addr>>24;
	uptr ptr=sh4mmap_ptr[upper];
	if ((s32)ptr<0)
	{
		sh4MemFullWrite<T,sz>(addr,data);
		return;
	}
	addr&=sh4mmap_mask[upper];
	u8* ptrn=(u8*)ptr;
	*(T*)&ptrn[addr]=data;
}

s32 sh4MemRead8(u32 addr)
{
	return sh4MR<s8,8>(addr);
}
s32 sh4MemRead16(u32 addr)
{
	return sh4MR<s16,16>(addr);
}
s32 sh4MemRead32(u32 addr)
{
	return sh4MR<s32,32>(addr);
}

void sh4MemWrite8(u32 addr,u8 data)
{
	sh4MW<u8,8>(addr,data);
}
void sh4MemWrite16(u32 addr,u16 data)
{
	sh4MW<u16,16>(addr,data);
}
void sh4MemWrite32(u32 addr,u32 data)
{
	sh4MW<u32,32>(addr,data);
}
