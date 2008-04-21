#include <stdio.h>
#include "dc\sh4\sh4.h"
#include "dc\mem\mmap.h"

u32 pc;
u32 next_pc;
u32 r[16];
u32 r_bank[16];
f32 fr[16];
#define fr_hex ((u32*)fr)
u32 macl;
u32 fpul;
struct 
{
	u32 T;
} sr;

#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
#define GetImm4(str) ((str>>0) & 0xf)
#define GetImm8(str) ((str>>0) & 0xff)
#define GetSImm8(str) ((s8)((str>>0) & 0xff))
#define GetImm12(str) ((str>>0) & 0xfff)
#define GetSImm12(str) (((s16)((GetImm12(str))<<4))>>3)

//Read Mem macros

#define ReadMemU32(to,addr) to=(u32)sh4MemRead32(addr)
#define ReadMemS32(to,addr) to=(s32)sh4MemRead32(addr)
#define ReadMemS16(to,addr) to=(u32)sh4MemRead16(addr)
#define ReadMemS8(to,addr) to=(u32)sh4MemRead8(addr)

//Base,offset format
#define ReadMemBOU32(to,addr,offset)	ReadMemU32(to,addr+offset)
#define ReadMemBOS16(to,addr,offset)	ReadMemS16(to,addr+offset)
#define ReadMemBOS8(to,addr,offset)		ReadMemS8(to,addr+offset)

//Write Mem Macros
#define WriteMemU32(addr,data)				sh4MemWrite32(addr,(u32)data)
#define WriteMemU16(addr,data)				sh4MemWrite16(addr,(u16)data)
#define WriteMemU8(addr,data)				sh4MemWrite8(addr,(u8)data)

//Base,offset format
#define WriteMemBOU32(addr,offset,data)		WriteMemU32(addr+offset,data)
#define WriteMemBOU16(addr,offset,data)		WriteMemU16(addr+offset,data)
#define WriteMemBOU8(addr,offset,data)		WriteMemU8(addr+offset,data)


void missing_op(u32 opcode)
{
	dbgf("Unsuported opcode 0x%X ;(\n",opcode);
}
void invalid_op(u32 opcode)
{
	dbgf("invalid opcode 0x%X ;(\n",opcode);
}
void missing_op(u32 opcode,char* text)
{
	dbgf("Unsuported opcode 0x%X %s;(\n",opcode,text);
}

void sh4_exec_op(u32 pc);
void sh4_exec_dslot()
{
	sh4_exec_op(pc+2);
}
#define opcode_name(n) sh4_##n
#define opcode(n) void opcode_name(n)(u32 op)
#define call_opcode(n) opcode_name(n)(opcode)

#include "sh4_handlers.h"

void inline sh4_exec_op(u32 pc)
{
	u16 opcode=(u16)sh4MemRead16(pc);
	switch(opcode>>12)
	{
		#include "sh4_decoder.h"
	}
}
void peridical_stuff();
#include <time.h>

void run_sh4()
{
	pc=0x8C010000;
	int oldtime=time(0);
	while(true)
	{
		int c=512;
		
		do
		{
			sh4_exec_op(pc);
			pc+=2;
		}while(c-->=0);
		//UpdateSystem();
	}
}
