#include "types.h"
#include "pvr_if.h"
#include "pvrLock.h"
#include "gui/base.h"
#include "dc/sh4/intc.h"

//TODO : move code later to a plugin
//TODO : Fix registers arrays , they must be smaller now doe to the way SB registers are handled
#include "plugins/plugin_manager.h"

Array<u8> vram;
 

//
//Update
/* 4/3/2006 -> it's a macro now
void UpdatePvr(u32 cycles)
{
	libPvr->pvr_info.UpdatePvr(cycles);
}*/
//
//Regs

u32 pvr_readreg_TA(u32 addr,u32 sz)
{
	//EMUERROR3("Not implemented TA register read , addr=%d,sz=%d",addr,sz);
	return libPvr->pvr_info.ReadReg(addr,sz);;//__pvr_read__reg(addr);
}

void pvr_writereg_TA(u32 addr,u32 data,u32 sz)
{
	//__pvr_write_reg(addr,data);
	libPvr->pvr_info.WriteReg(addr,data,sz);
}

#define vram_64b
//vram 32-64b
#ifdef vram_64b
//read
u8 pvr_read_area1_8(u32 addr)
{
	printf("8 bit vram writes are not possible\n");
	return 0;
}

u16 pvr_read_area1_16(u32 addr)
{
	addr =vramlock_ConvAddrtoOffset64(addr);
	return *(u16*)&vram[addr];
}
u32 pvr_read_area1_32(u32 addr)
{
	addr =vramlock_ConvAddrtoOffset64(addr);
	return *(u32*)&vram[addr ];
}

//write
void pvr_write_area1_8(u32 addr,u8 data)
{
	printf("8 bit vram writes are not possible\n");
}
void pvr_write_area1_16(u32 addr,u16 data)
{
	u32 Address2=addr;
	addr=vramlock_ConvAddrtoOffset64(addr);

	u16* vptr=(u16*)&vram[addr ];
	if (*vptr != data)
	{
		vramlock_Test(Address2,0,0);
		*vptr=data;
	}
}
void pvr_write_area1_32(u32 addr,u32 data)
{
	u32 Address2=addr;
	addr=vramlock_ConvAddrtoOffset64(addr);

	u32* vptr=(u32*)&vram[addr];
	if (*vptr != data)
	{
		vramlock_Test(Address2,0,0);
		*vptr=data;
	}
}


#else
//read
u8 pvr_read_area1_8(u32 addr)
{
	addr &= 0x1ffffff;
	if (addr<0xFFFFFF)//(adr>>24)&0x1//using 64 bit interface
	{
		u32 offset=0;
		//Translate address to offset
		//if bit 2(0x4) is set then read from rambank2(4mb+>)
		//get rid of bit 2(0x4) and >> by 1 to fix the pos 
		//01111111111111111111100->0x3FFFFC
		//00000000000000000000011->0x3
		//Address=((Address>>1)&0x3FFFFC)+(Address&0x3)+0x3FFFFF*((Address>>2)&0x1);		
		if(addr < 0x800000) 
		{              /* input : 000000 - 7FFFFF */
			if(addr >= 0x400000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset;
		}
		else 
		{											/* input : 800000 - FFFFFF */
			if(addr >= (u32)0xC00000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset - 0x800000;
		}
	}
	else if ((addr > 0xFFFFFF)&& (addr<0x1FFFFFF))//using 32 bit interface
	{
		//addr=addr-0x1000000;//translate to vram offset
		addr=addr & 0x7FFFFF;
	}

	return vram[addr & VRAM_MASK];
}

u16 pvr_read_area1_16(u32 addr)
{
		addr &= 0x1ffffff;
	if (addr<0xFFFFFF)//(adr>>24)&0x1//using 64 bit interface
	{
		u32 offset=0;
		//Translate address to offset
		//if bit 2(0x4) is set then read from rambank2(4mb+>)
		//get rid of bit 2(0x4) and >> by 1 to fix the pos 
		//01111111111111111111100->0x3FFFFC
		//00000000000000000000011->0x3
		//Address=((Address>>1)&0x3FFFFC)+(Address&0x3)+0x3FFFFF*((Address>>2)&0x1);		
		if(addr < 0x800000) 
		{              /* input : 000000 - 7FFFFF */
			if(addr >= 0x400000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset;
		}
		else 
		{											/* input : 800000 - FFFFFF */
			if(addr >= (u32)0xC00000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset - 0x800000;
		}
	}
	else if ((addr > 0xFFFFFF)&& (addr<0x1FFFFFF))//using 32 bit interface
	{
		//addr=addr-0x1000000;//translate to vram offset
		addr=addr & 0x7FFFFF;
	}
	return *(u16*)&vram[addr & VRAM_MASK];
}
u32 pvr_read_area1_32(u32 addr)
{
		addr &= 0x1ffffff;
	if (addr<0xFFFFFF)//(adr>>24)&0x1//using 64 bit interface
	{
		u32 offset=0;
		//Translate address to offset
		//if bit 2(0x4) is set then read from rambank2(4mb+>)
		//get rid of bit 2(0x4) and >> by 1 to fix the pos 
		//01111111111111111111100->0x3FFFFC
		//00000000000000000000011->0x3
		//Address=((Address>>1)&0x3FFFFC)+(Address&0x3)+0x3FFFFF*((Address>>2)&0x1);		
		if(addr < 0x800000) 
		{              /* input : 000000 - 7FFFFF */
			if(addr >= 0x400000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset;
		}
		else 
		{											/* input : 800000 - FFFFFF */
			if(addr >= (u32)0xC00000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset - 0x800000;
		}
	}
	else if ((addr > 0xFFFFFF)&& (addr<0x1FFFFFF))//using 32 bit interface
	{
		//addr=addr-0x1000000;//translate to vram offset
		addr=addr & 0x7FFFFF;
	}
	return *(u32*)&vram[addr & VRAM_MASK];
}

//write
void pvr_write_area1_8(u32 addr,u8 data)
{
	addr &= 0x1ffffff;
	if (addr<0xFFFFFF)//(adr>>24)&0x1//using 64 bit interface
	{
		u32 offset=0;
		//Translate address to offset
		//if bit 2(0x4) is set then read from rambank2(4mb+>)
		//get rid of bit 2(0x4) and >> by 1 to fix the pos 
		//01111111111111111111100->0x3FFFFC
		//00000000000000000000011->0x3
		//Address=((Address>>1)&0x3FFFFC)+(Address&0x3)+0x3FFFFF*((Address>>2)&0x1);		
		if(addr < 0x800000) 
		{              /* input : 000000 - 7FFFFF */
			if(addr >= 0x400000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset;
		}
		else 
		{											/* input : 800000 - FFFFFF */
			if(addr >= (u32)0xC00000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset - 0x800000;
		}
	}
	else if ((addr > 0xFFFFFF)&& (addr<0x1FFFFFF))//using 32 bit interface
	{
		//addr=addr-0x1000000;//translate to vram offset
		//addr=addr & 0x7FFFFF;
	}
	vram[addr & VRAM_MASK]=data;
}
void pvr_write_area1_16(u32 addr,u16 data)
{
	addr &= 0x1ffffff;
	if (addr<0xFFFFFF)//(adr>>24)&0x1//using 64 bit interface
	{
		u32 offset=0;
		//Translate address to offset
		//if bit 2(0x4) is set then read from rambank2(4mb+>)
		//get rid of bit 2(0x4) and >> by 1 to fix the pos 
		//01111111111111111111100->0x3FFFFC
		//00000000000000000000011->0x3
		//Address=((Address>>1)&0x3FFFFC)+(Address&0x3)+0x3FFFFF*((Address>>2)&0x1);		
		if(addr < 0x800000) 
		{              /* input : 000000 - 7FFFFF */
			if(addr >= 0x400000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset;
		}
		else 
		{											/* input : 800000 - FFFFFF */
			if(addr >= (u32)0xC00000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset - 0x800000;
		}
	}
	else if ((addr > 0xFFFFFF)&& (addr<0x1FFFFFF))//using 32 bit interface
	{
		//addr=addr-0x1000000;//translate to vram offset
		//addr=addr & 0x7FFFFF;
	}
	*(u16*)&vram[addr & VRAM_MASK]=data;
}
void pvr_write_area1_32(u32 addr,u32 data)
{
	//data=0xFF000000;
	addr &= 0x1ffffff;
	if (addr<0xFFFFFF)//(adr>>24)&0x1//using 64 bit interface
	{
		u32 offset=0;
		if (addr>0x0053c000)
			data=data;
		//Translate address to offset
		//if bit 2(0x4) is set then read from rambank2(4mb+>)
		//get rid of bit 2(0x4) and >> by 1 to fix the pos 
		//01111111111111111111100->0x3FFFFC
		//00000000000000000000011->0x3
		//Address=((Address>>1)&0x3FFFFC)+(Address&0x3)+0x3FFFFF*((Address>>2)&0x1);		
		if(addr < 0x800000) 
		{              /* input : 000000 - 7FFFFF */
			if(addr >= 0x400000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset;
		}
		else 
		{											/* input : 800000 - FFFFFF */
			if(addr >= (u32)0xC00000) 
			{     /* Upper bank */
				offset = 4;
				addr -= 0x400000;
			}
			addr = ((addr & 0xFFFFFFFC) << 1) + (addr & 0x00000003) + offset - 0x800000;
		}
	}
	else if ((addr > 0xFFFFFF)&& (addr<0x1FFFFFF))//using 32 bit interface
	{
		//addr=addr-0x1000000;//translate to vram offset
		//addr=addr & 0x7FFFFF;
	}
	*(u32*)&vram[addr & VRAM_MASK]=data;
}



#endif
void TAWrite(u32 address,u32* data,u32 count)
{
	u32 address_w=address&0x1FFFFFF;//correct ?
	if (address_w<0x800000)//TA poly
	{
		try
		{
			libPvr->pvr_info.TADma(address,data,count);
		}
		catch(...){}
	}
	else if(address_w<0x1000000) //Yuv Converter
	{
		u32 TA_YUV_TEX_BASE=pvr_readreg_TA(0x5F8148,4);
		u32 TA_YUV_TEX_CTRL=pvr_readreg_TA(0x5F814C,4);
		printf("Yuv Converter 0x%X , size %d\n",address,count);
		printf("Yuv Format : %s , texture type %d ,  %d x %d\n",
			   (TA_YUV_TEX_CTRL & (1<<24))==0?"YUV420":"YUV422",
			   (TA_YUV_TEX_CTRL>>16 )&1,
			   (((TA_YUV_TEX_CTRL>>0)&0x3F)+1)*16,
			   (((TA_YUV_TEX_CTRL>>8)&0x3F)+1)*16
			   );
		if ((TA_YUV_TEX_CTRL & (1<<24))==0)
		{
			printf("%d blocks\n",count*32/384);
		}
		else
			printf("%d blocks\n",count*32/512);
		printf("Destination : 0x%X\n",TA_YUV_TEX_BASE);
		
		//TODO : Check if it's allrgiht to do it here (cosnidering it's yuv block end too)
		RaiseInterrupt(holly_YUV_DMA);
	}
	else //Vram Write
	{
		printf("Vram Write 0x%X , size %d",address,count);
	}
}
//Misc interface

//Init/Term , global
void pvr_Init()
{
	vram.Resize(VRAM_SIZE,false);
}
void pvr_Term()
{
	vram.Free();
}
//Reset -> Reset - Initialise to defualt values
void pvr_Reset(bool Manual)
{
	if (!Manual)
		vram.Zero();
}

