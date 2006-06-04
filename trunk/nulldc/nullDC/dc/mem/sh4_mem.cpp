#include "types.h"
#include <memory.h>

#include "memutil.h"
#include "sh4_mem.h"
#include "sh4_area0.h"
#include "sh4_internal_reg.h"
#include "dc/pvr/pvr_if.h"
#include "dc/sh4/sh4_registers.h"
#include "dc/dc.h"
#include "dc/sh4/rec_v1/rec_v1_blockmanager.h"

//main system mem
VArray mem_b;

//bios rom
Array<u8> bios_b;

//flash rom
Array<u8> flash_b;


void mem_Init()
{
	//Allocate mem for memory/bios/flash
	mem_b.Init(RAM_SIZE);
	bios_b.Resize(BIOS_SIZE,false);
	flash_b.Resize(FLASH_SIZE,false);

	sh4_area0_Init();
	sh4_internal_reg_Init();

}

//Reset Sysmem/Regs -- Pvr is not changed , bios/flash are not zero'd out
void mem_Reset(bool Manual)
{
	//mem is reseted on hard restart(power on) , not manual...
	if (!Manual)
	{
		//fill mem w/ 0's
		mem_b.Zero();
		bios_b.Zero();
		flash_b.Zero();

		LoadBiosFiles();
		LoadSyscallHooks();
	}

	//Reset registers
	sh4_area0_Reset(Manual);
	sh4_internal_reg_Reset(Manual);
}

void mem_Term()
{
	//Free allocated mem for memory/bios/flash
	mem_b.Term();
	bios_b.Free();
	//write back flash ?
	char* temp_path=GetEmuPath("data\\");
	strcat(temp_path,"dc_flash_wb.bin");
	SaveSh4FlashromToFile(temp_path);
	free(temp_path);
	flash_b.Free();

	sh4_internal_reg_Term();
	sh4_area0_Term();
}

FILE* F_OUT;
FILE* F_IN;
#include "dc\sh4\sh4_interpreter.h"
void WriteTest(u32 dddd)
{
	if (!F_OUT)
	{
		if ((sh4_cpu->Run==Sh4_int_Run))
		{
			F_IN=fopen("f:/reg_rec.log","rb");
			F_OUT=fopen("f:/reg_int.log","wb");
		}
		else
		{
			F_IN=fopen("f:/reg_int.log","rb");
			F_OUT=fopen("f:/reg_rec.log","wb");
		}
	}

	fwrite(&dddd,1,4,F_OUT);
	u32 t_dddd;
	if (F_IN)
	{
		fread(&t_dddd,1,4,F_IN);

		if (t_dddd!=dddd)
		{
			MEM_DO_BREAK;
		}
	}
}
u8 MEMCALL ReadMem8(u32 addr)
{
	//if P4
	if (((addr>>29) &0x7)==7)
	{
		return (u8)ReadMem_P4(addr,1);
	}

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
		return (u8)ReadMem_area0(addr,1);
		break;

	//Area 1 : Vram 8 mb , {{64b,32b}x2}x2
	case 1:
		return pvr_read_area1_8(addr);
		break;

	//area 2 : not mapped
	case 2:
		EMUERROR2("Area 2 not mapped read , addr=%x",addr);
		break;

	//area 3 : System Ram 16 mb, {64b}x4
	case 3:
		return mem_b[addr&RAM_MASK];;
		break;

	//area 4 : TA -> needs work
	case 4:
		break;

	//area 5 : Expantion Port
	case 5:
		break;

	//area 6 : not mapped
	case 6:
		EMUERROR2("Area 6 not mapped read , addr=%d",addr);
		break;

	//area 7 - internal registers
	case 7:
		//EMUERROR2("Area 7 read , addr=%x , not form P4",addr);
		return (u8)ReadMem_area7(addr,1);
		break;
	}
	EMUERROR2("Mem Read not mapped , addr=%x",addr);
	return 0;
}

u16 MEMCALL ReadMem16(u32 addr)
{
#ifdef TRACE
	if (addr&0x1)
	{
		EMUERROR3("Missaligned 2 byte read , addr=%x,pc=%x",addr,pc);
		TRACE_DO_BREAK;
	}
#endif
	//if P4
	if (((addr>>29) &0x7)==7)
	{
		return (u16)ReadMem_P4(addr,2);
	}

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
		return (u16)ReadMem_area0(addr,2);
		break;

	//Area 1 : Vram 8 mb , {{64b,32b}x2}x2
	case 1:
		return pvr_read_area1_16(addr);
		break;

	//area 2 : not mapped
	case 2:
		EMUERROR2("Area 2 not mapped read , addr=%x",addr);
		break;

	//area 3 : System Ram 16 mb, {64b}x4
	case 3:
		return *(u16*)&mem_b[addr&RAM_MASK];
		break;

	//area 4 : TA -> needs work
	case 4:
		break;

	//area 5 : Expantion Port
	case 5:
		break;

	//area 6 : not mapped
	case 6:
		EMUERROR2("Area 6 not mapped read , addr=%d",addr);
		break;

	//area 7 - internal registers
	case 7:
		//EMUERROR2("Area 7 read , addr=%x , not form P4",addr);
		return (u16)ReadMem_area7(addr,2);
		break;
	}
	EMUERROR2("Mem Read not mapped , addr=%x",addr);
	return 0;
}

u32 MEMCALL ReadMem32(u32 addr)
{
#ifdef TRACE
	if (addr&0x3)
	{
		EMUERROR3("Missaligned 4 byte read , addr=%x,pc=%x",addr,pc);
		TRACE_DO_BREAK;
	}
#endif
	//if P4
	if (((addr>>29) &0x7)==7)
	{
		return ReadMem_P4(addr,4);
	}

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
		return ReadMem_area0(addr,4);
		break;

	//Area 1 : Vram 8 mb , {{64b,32b}x2}x2
	case 1:
		return pvr_read_area1_32(addr);
		break;

	//area 2 : not mapped
	case 2:
		EMUERROR2("Area 2 not mapped read , addr=%x",addr);
		break;

	//area 3 : System Ram 16 mb, {64b}x4
	case 3:
		return *(u32*)&mem_b[addr&RAM_MASK];
		break;

	//area 4 : TA -> needs work
	case 4:
		break;

	//area 5 : Expantion Port
	case 5:
		break;

	//area 6 : not mapped
	case 6:
		EMUERROR2("Area 6 not mapped read , addr=%d",addr);
		break;

	//area 7 - internal registers
	case 7:
		//EMUERROR2("Area 7 read , addr=%x , not form P4",addr);
		return ReadMem_area7(addr,4);
		break;
	}
	EMUERROR2("Mem Read not mapped , addr=%x",addr);
	return 0;
}


void MEMCALL WriteMem8(u32 addr,u8 data)
{
	//WriteTest(addr,data);
	//if P4
	if (((addr>>29) &0x7)==7)
	{
		WriteMem_P4(addr,data,1);
		return;
	}

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
		WriteMem_area0(addr,data,1);
		return;
		break;

	//Area 1 : Vram 8 mb , {{64b,32b}x2}x2
	case 1:
		pvr_write_area1_8(addr,data);
		return;
		break;

	//area 2 : not mapped
	case 2:
		EMUERROR2("Area 2 not mapped write , addr=%x",addr);
		break;

	//area 3 : System Ram 16 mb, {64b}x4
	case 3:
//		rec_v1_BlockTest(addr);
		mem_b[addr&RAM_MASK]=data;
		return;
		break;

	//area 4 : TA -> needs work
	case 4:
		break;

	//area 5 : Expantion Port
	case 5:
		break;

	//area 6 : not mapped
	case 6:
		EMUERROR2("Area 6 not mapped write , addr=%d",addr);
		break;

	//area 7 - internal registers
	case 7:
		//EMUERROR2("Area 7 write , not from P4, addr=%x",addr);
		WriteMem_area7(addr,data,1);
		return;
		break;
	}

	EMUERROR3("Write to Mem not implemented , addr=%x,data=%x",addr,data);
}

void MEMCALL WriteMem16(u32 addr,u16 data)
{
	//WriteTest(addr,data);
#ifdef TRACE
	if (addr&0x1)
	{
		EMUERROR4("Missaligned 2 byte write , addr=%x,pc=%x,data=%x",addr,pc,data);
		TRACE_DO_BREAK;
	}
#endif
	//if P4
	if (((addr>>29) &0x7)==7)
	{
		WriteMem_P4(addr,data,2);
		return;
	}

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
		WriteMem_area0(addr,data,2);
		return;
		break;

	//Area 1 : Vram 8 mb , {{64b,32b}x2}x2
	case 1:
		pvr_write_area1_16(addr,data);
		return;
		break;

	//area 2 : not mapped
	case 2:
		EMUERROR2("Area 2 not mapped write , addr=%x",addr);
		break;

	//area 3 : System Ram 16 mb, {64b}x4
	case 3:
//		rec_v1_BlockTest(addr);
		*(u16*)&mem_b[addr&RAM_MASK]=data;
		return;
		break;

	//area 4 : TA -> needs work
	case 4:
		break;

	//area 5 : Expantion Port
	case 5:
		break;

	//area 6 : not mapped
	case 6:
		EMUERROR2("Area 6 not mapped write , addr=%d",addr);
		break;

	//area 7 - internal registers
	case 7:
		//EMUERROR2("Area 7 write , not from P4, addr=%x",addr);
		WriteMem_area7(addr,data,2);
		return;
		break;
	}

	EMUERROR3("Write to Mem not implemented , addr=%x,data=%x",addr,data);
}

void MEMCALL WriteMemBlock(u32 addr,u32* data,u32 size)
{
#ifdef TRACE
	if (addr&0x3)
	{
		EMUERROR4("Missaligned Block write , addr=%x,pc=%x,data=%x",addr,pc,data);
		TRACE_DO_BREAK;
	}
#endif
	//if P4
	if (((addr>>29) &0x7)==7)
		goto fallback;
	

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
		goto fallback;

	//Area 1 : Vram 8 mb , {{64b,32b}x2}x2
	case 1:
		pvr_write_area1_block(addr,data,size);
		return;

	//area 2 : not mapped
	case 2:
		goto fallback;

	//area 3 : System Ram 16 mb, {64b}x4
	case 3:
		//rec_v1_NotifyMemWrite(addr,size);
		//*WATCH* , mem has to be copied forwards. i duno if that's what memcpy does :)
		memcpy(GetMemPtr(addr,size),data,size);
		return;

	//area 4 : TA -> needs work
	case 4:
		goto fallback;

	//area 5 : Expantion Port
	case 5:
		goto fallback;

	//area 6 : not mapped
	case 6:
		goto fallback;

	//area 7 - internal registers
	case 7:
		goto fallback;
	}

	return;

fallback:
	for (u32 i=0;i<size;i+=4)
	{
		WriteMem32(addr+i,data[i>>2]);
	}
}
void MEMCALL WriteMem32(u32 addr,u32 data)
{
	//WriteTest(addr,data);
#ifdef TRACE
	if (addr&0x3)
	{
		EMUERROR4("Missaligned 4 byte write , addr=%x,pc=%x,data=%x",addr,pc,data);
		TRACE_DO_BREAK;
	}
#endif
	//if P4
	if (((addr>>29) &0x7)==7)
	{
		WriteMem_P4(addr,data,4);
		return;
	}

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
		WriteMem_area0(addr,data,4);
		return;
		break;

	//Area 1 : Vram 8 mb , {{64b,32b}x2}x2
	case 1:
		pvr_write_area1_32(addr,data);
		return;
		break;

	//area 2 : not mapped
	case 2:
		EMUERROR2("Area 2 not mapped write , addr=%x",addr);
		break;

	//area 3 : System Ram 16 mb, {64b}x4
	case 3:
//		rec_v1_BlockTest(addr);
		*(u32*)&mem_b[addr&RAM_MASK]=data;
		return;
		break;

	//area 4 : TA -> needs work
	case 4:
		break;

	//area 5 : Expantion Port
	case 5:
		break;

	//area 6 : not mapped
	case 6:
		EMUERROR2("Area 6 not mapped write , addr=%d",addr);
		break;

	//area 7 - internal registers
	case 7:
		//EMUERROR2("Area 7 write , not from P4, addr=%x",addr);
		WriteMem_area7(addr,data,4);
		return;
		break;
	}

	EMUERROR3("Write to Mem not mapped , addr=%x,data=%x",addr,data);
}

//for debugger - needs to be done
bool ReadMem_DB(u32 addr,u32& data,u32 size )
{
	return false;
}
bool WriteMem_DB(u32 addr,u32 data,u32 size )
{
	return false;
}

//Get pointer to ram area , 0 if error
//For debugger(gdb) - dynarec
u8* GetMemPtr(u32 Addr,u32 size)
{
	switch ((Addr>>26)&0x7)
	{
		case 3:
		return &mem_b[Addr & RAM_MASK];
		
		case 0:
			Addr &= 0x01FFFFFF;//to get rid of non needed bits
			if ((Addr<=0x001FFFFF))//	:MPX	System/Boot ROM
				return &bios_b[Addr];
			
			printf("Area 0 GetMemPtr : out of bios area , addr=0x%X",Addr);
			return 0;

		case 1:
		case 2:
		case 4:
		case 5:
		case 6:
		case 7:
		default:
			printf("Get MemPtr not suported area ; addr=0x%X",Addr);
			return &bios_b[0];
	}
			
}

//Get infomation about an area , eg ram /size /anything
//For dynarec - needs to be done
void GetMemInfo(u32 addr,u32 size)
{
	//needs to be done
}