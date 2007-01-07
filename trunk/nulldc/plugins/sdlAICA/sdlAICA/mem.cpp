#include "mem.h"
#include "arm7.h"
#include "sgc_if.h"
#include "_vmem.h"

u8 *aica_reg;
u8 *aica_ram;

//00000000Å007FFFFF @DRAM_AREA* 
//00800000Å008027FF @CHANNEL_DATA 
//00802800Å00802FFF @COMMON_DATA 
//00803000Å00807FFF @DSP_DATA 
template<u32 sz,bool arm>
u32 ReadReg(u32 addr)
{
	if (addr<0x2800)
	{
		ReadMemArrRet(aica_reg,addr,sz);
	}
	if (addr < 0x2818)
	{
		if (sz==1)
		{
			ReadCommonReg8(addr);
			ReadMemArrRet(aica_reg,addr,1);
		}
		else
		{
			ReadCommonReg8(addr);
			ReadCommonReg8(addr+1);
			ReadMemArrRet(aica_reg,addr,2);
		}
	}

	ReadMemArrRet(aica_reg,addr,sz);
}
template<u32 sz,bool arm>
void WriteReg(u32 addr,u32 data)
{
	if (addr < 0x2000)
	{
		//Channel data
		u32 chan=addr>>7;
		u32 reg=addr&0x7F;
		if (sz==1)
		{
			WriteMemArr(aica_reg,addr,data,1);
			WriteChannelReg8(chan,reg);
		}
		else
		{
			WriteMemArr(aica_reg,addr,data,2);
			WriteChannelReg8(chan,reg);
			WriteChannelReg8(chan,reg+1);
		}
		return;
	}

	if (addr<0x2800)
	{
		if (sz==1)
		{
			WriteMemArr(aica_reg,addr,data,1);
		}
		else 
		{
			WriteMemArr(aica_reg,addr,data,2);
		}
		return;
	}

	if (addr < 0x2818)
	{
		if (sz==1)
		{
			WriteCommonReg8(addr,data);
		}
		else
		{
			WriteCommonReg8(addr,data&0xFF);
			WriteCommonReg8(addr+1,data>>8);
		}
		return;
	}

	//if (arm==false)
	{
		if ((addr ) == 0x2c00)
		{
			//printf("Write to ARM reset, value= %x\n",data);
			arm_SetEnabled((data&1)==0);
			data&=~1;
		}
	}

	if (sz==1)
		WriteAicaReg<1>(addr,data);
	else
		WriteAicaReg<2>(addr,data);
}
//Reg reads from arm side ..
template <u32 sz,class T>
inline T fastcall arm_ReadMem(u32 addr)
{
	if (sz==1)
		return (T)ReadReg<1,true>(addr & 0x7FFF);
	else
		return (T)ReadReg<2,true>(addr & 0x7FFF);
}		
template <u32 sz,class T>
inline void fastcall arm_WriteMem(u32 addr,T data)
{
	if (sz==1)
		WriteReg<1,true>(addr & 0x7FFF,data);
	else
		WriteReg<2,true>(addr & 0x7FFF,data);
}
//Reg reads from sh4 side ...
u32 sh4_ReadMem_reg(u32 addr,u32 size)
{
	if (size==1)
	{
		printf("AICA : 1 byte Read WTF\n");
	}

	if (size==1)
		return ReadReg<1,false>(addr & 0x7FFF);
	else
		return ReadReg<2,false>(addr & 0x7FFF);

	//must never come here
	return 0;
}

void sh4_WriteMem_reg(u32 addr,u32 data,u32 size)
{
	if (size==1)
		WriteReg<1,false>(addr & 0x7FFF,data);
	else
		WriteReg<2,false>(addr & 0x7FFF,data);
}

//Ram reads from sh4 side
u32 sh4_ReadMem_ram(u32 addr,u32 size)
{
	/*
	if (0x00800104==addr)
		return *(u32*)&aica_ram[addr&AICA_MEM_MASK]^=0xFFFFFFFF;
	if (0x00800164==addr)
		return *(u32*)&aica_ram[addr&AICA_MEM_MASK]^=0xFFFFFFFF;
	if (0x008001C4==addr)
		return *(u32*)&aica_ram[addr&AICA_MEM_MASK]^=0xFFFFFFFF;
	*/

	if (size==1)
		return aica_ram[addr&AICA_MEM_MASK];
	else if (size==2)
		return *(u16*)&aica_ram[addr&AICA_MEM_MASK];
	else if (size==4)
		return *(u32*)&aica_ram[addr&AICA_MEM_MASK];

	return 0;
}

void sh4_WriteMem_ram(u32 addr,u32 data,u32 size)
{
	if (size==1)
		aica_ram[addr&AICA_MEM_MASK]=(u8)data;
	else if (size==2)
		*(u16*)&aica_ram[addr&AICA_MEM_MASK]=(u16)data;
	else if (size==4)
		*(u32*)&aica_ram[addr&AICA_MEM_MASK]=data;
}
//Map using _vmem .. yay
void init_mem()
{
	aica_ram=(u8*)malloc(AICA_MEM_SIZE);
	aica_reg=(u8*)malloc(0x8000);
	memset(aica_ram,0,AICA_MEM_SIZE);
	memset(aica_reg,0,0x8000);
	_vmem_init();

	//00000000Å007FFFFF @DRAM_AREA* 
	//00800000Å008027FF @CHANNEL_DATA 
	//00802800Å00802FFF @COMMON_DATA 
	//00803000Å00807FFF @DSP_DATA 
	
	u32 total_map=0x800000;//8 mb
	while(total_map)
	{
		total_map-=AICA_MEM_SIZE;
		_vmem_map_block(aica_ram,(total_map)>>16,(total_map+AICA_MEM_MASK)>>16);
		printf("Mapped 0x%08X to 0x%08X to ram \n",total_map,total_map+AICA_MEM_MASK);
	}
	_vmem_handler reg_read= _vmem_register_handler_Template(arm_ReadMem,arm_WriteMem);
	_vmem_map_handler(reg_read,0x0080,0x0081);
	printf("Mapped 0x%08X to 0x%08X to register funcions \n",0x00800000,0x00810000);

	
	for (u32 i=1;i<0x100;i++)
	{
		_vmem_mirror_mapping((i<<24)>>16,0,(1<<24)>>16);
	}
}
//kill mem map & free used mem ;)
void term_mem()
{
	_vmem_term();
	free(aica_reg);
	free(aica_ram);
}