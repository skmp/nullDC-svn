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
#include "_vmem.h"



//main system mem
VArray mem_b;

//bios rom
Array<u8> bios_b;

//flash rom
Array<u8> flash_b;



u8 MEMCALL ReadMem8_i(u32 addr);
u16 MEMCALL ReadMem16_i(u32 addr);
u32 MEMCALL ReadMem32_i(u32 addr);

void MEMCALL WriteMem8_i(u32 addr,u8 data);
void MEMCALL WriteMem16_i(u32 addr,u16 data);
void MEMCALL WriteMem32_i(u32 addr,u32 data);

void _vmem_init();
void _vmem_reset();
void _vmem_term();

//MEM MAPPINNGG

//AREA 1
_vmem_handler area1_32b;
void map_area1_init()
{
	area1_32b = _vmem_register_handler(pvr_read_area1_8,pvr_read_area1_16,pvr_read_area1_32,
									pvr_write_area1_8,pvr_write_area1_16,pvr_write_area1_32);
}

void map_area1(u32 base)
{
	//map vram
	
	//if vram is 8 mb , it mirrors ?

	//64b interface
	u32 start= 0x0400 | base;
	u32 end  = start+(VRAM_MASK>>16);
	_vmem_map_block(vram.data,start,end);
	
	//32b interface
	start= 0x0500 | base;
	end  = start+(VRAM_MASK>>16);
	_vmem_map_handler(area1_32b,start,end);

	//upper 32mb mirror lower 32 mb
	//0x0600 to 0x07FF
	_vmem_mirror_mapping(0x0600|base,0x0400|base,0x0200);
}

//AREA 2
void map_area2_init()
{
	//nothing to map :p
}

void map_area2(u32 base)
{
	//nothing to map :p
}


//AREA 3
void map_area3_init()
{
}

void map_area3(u32 base)
{
	u32 start = 0x0C00 | base;
	u32 end   = start+(RAM_MASK>>16);
	//Map top 32 mb (16 mb mirrored , or 32mb , depending on naomi/dc)
	for (u32 j=0;j<0x1FFFFFF;j+=RAM_SIZE)
	{
		_vmem_map_block(mem_b.data,start,end);
		start+=(RAM_SIZE)>>16;
		end+=(RAM_SIZE)>>16;
	}

	//upper 32mb mirror lower 32 mb
	_vmem_mirror_mapping(0x0E00|base,0x0C00|base,0x0200);
}

//AREA 4
void map_area4_init()
{
	
}

void map_area4(u32 base)
{
	//TODO : map later

	//upper 32mb mirror lower 32 mb
	_vmem_mirror_mapping(0x1200|base,0x1000|base,0x0200);
}
//AREA 5	--	Ext. Device
void map_area5_init()
{
	
}

void map_area5(u32 base)
{
	//TODO : map later
}

//AREA 6	--	Unassigned 
void map_area6_init()
{
	//nothing to map :p
}
void map_area6(u32 base)
{
	//nothing to map :p
}


//set vmem to defualt values
void mem_map_defualt()
{
	//vmem - init/reset :)
	_vmem_init();

	
	//*TEMP*
	//setup a fallback handler , that calls old code :)
	//_vmem_handler def_handler =
	//	_vmem_register_handler(ReadMem8_i,ReadMem16_i,ReadMem32_i,WriteMem8_i,WriteMem16_i,WriteMem32_i);
	//_vmem_map_handler(def_handler,0,0xFFFF);

	//U0/P0
	//0x0xxx xxxx	-> normal memmap
	//0x2xxx xxxx	-> normal memmap
	//0x4xxx xxxx	-> normal memmap
	//0x6xxx xxxx	-> normal memmap
	//-----------
	//P1
	//0x8xxx xxxx	-> normal memmap
	//-----------
	//P2
	//0xAxxx xxxx	-> normal memmap
	//-----------
	//P3
	//0xCxxx xxxx	-> normal memmap
	//-----------
	//P4
	//0xExxx xxxx	-> internal area

	//Init Memmaps (register handlers)
	map_area0_init();
	map_area1_init();
	map_area2_init();
	map_area3_init();
	map_area4_init();
	map_area5_init();
	map_area6_init();
	map_area7_init();

	//0x0-0xD : 7 times the normal memmap mirrors :)
	//some areas can be customised :)
	for (int i=0x0;i<0xE;i+=0x2)
	{
		map_area0(i<<12);	//Bios,Flahsrom,i/f regs,Ext. Device,Sound Ram
		map_area1(i<<12);	//Vram
		map_area2(i<<12);	//Unassigned
		map_area3(i<<12);	//Ram
		map_area4(i<<12);	//TA
		map_area5(i<<12);	//Ext. Device
		map_area6(i<<12);	//Unassigned
		map_area7(i<<12);	//Sh4 Regs
	}

	//map p4 region :)
	map_p4();
}
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
	sh4_internal_reg_Reset(Manual);;
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
	//vmem
	_vmem_term();
}

FILE* F_OUT;
FILE* F_IN;
#include "dc\sh4\sh4_interpreter.h"
u8 MEMCALL ReadMem8_i(u32 addr)
{
	//if P4
	/*if (((addr>>29) &0x7)==7)
	{
		return (u8)ReadMem_P4(addr,1);
	}*/

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
//		return (u8)ReadMem_area0(addr,1);
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
//		return (u8)ReadMem_area7(addr,1);
		break;
	}
	EMUERROR2("Mem Read not mapped , addr=%x",addr);
	return 0;
}

u16 MEMCALL ReadMem16_i(u32 addr)
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
//		return (u16)ReadMem_P4(addr,2);
	}

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
//		return (u16)ReadMem_area0(addr,2);
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
//		return (u16)ReadMem_area7(addr,2);
		break;
	}
	EMUERROR2("Mem Read not mapped , addr=%x",addr);
	return 0;
}

u32 MEMCALL ReadMem32_i(u32 addr)
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
//		return ReadMem_P4(addr,4);
	}

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
//		return ReadMem_area0(addr,4);
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
//		return ReadMem_area7(addr,4);
		break;
	}
	EMUERROR2("Mem Read not mapped , addr=%x",addr);
	return 0;
}


void MEMCALL WriteMem8_i(u32 addr,u8 data)
{
	//WriteTest(addr,data);
	//if P4
	if (((addr>>29) &0x7)==7)
	{
//		WriteMem_P4(addr,data,1);
		return;
	}

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
//		WriteMem_area0(addr,data,1);
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
//		WriteMem_area7(addr,data,1);
		return;
		break;
	}

	EMUERROR3("Write to Mem not implemented , addr=%x,data=%x",addr,data);
}

void MEMCALL WriteMem16_i(u32 addr,u16 data)
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
//		WriteMem_P4(addr,data,2);
		return;
	}

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
//		WriteMem_area0(addr,data,2);
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
//		WriteMem_area7(addr,data,2);
		return;
		break;
	}

	EMUERROR3("Write to Mem not implemented , addr=%x,data=%x",addr,data);
}

void MEMCALL WriteMemBlock(u32 addr,u32* data,u32 size)
{
	//not worth the trouble to decode :p
	//vmem is faster
	/*
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

fallback:*/
	for (u32 i=0;i<size;i+=4)
	{
		WriteMem32(addr+i,data[i>>2]);
	}
}
void MEMCALL WriteMem32_i(u32 addr,u32 data)
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
//		WriteMem_P4(addr,data,4);
		return;
	}

	//switch area
	switch((addr>>26)&0x7)
	{
	//Area 0 : Bios/FlashRom/DC registers
	case 0:
//		WriteMem_area0(addr,data,4);
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
//		WriteMem_area7(addr,data,4);
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