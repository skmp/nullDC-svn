#include "types.h"
#include "pvr_if.h"
#include "pvrLock.h"
#include "gui/base.h"
#include "dc/sh4/intc.h"
#include "dc/mem/_vmem.h"

//TODO : move code later to a plugin
//TODO : Fix registers arrays , they must be smaller now doe to the way SB registers are handled
#include "plugins/plugin_manager.h"


 

//
//Update
/* 4/3/2006 -> it's a macro now
void UpdatePvr(u32 cycles)
{
	libPvr->pvr_info.UpdatePvr(cycles);
}*/
//

//YUV converter code :)
//inits the YUV converter
u32 YUV_tempdata[512/4];//512 bytes

u32 YUV_index=0;
u32	YUV_dest=0;
u32 YUV_doneblocks;
u32 YUV_blockcount;

u32 YUV_x_curr;
u32 YUV_y_curr;

u32 YUV_x_size;
u32 YUV_y_size;

//writes 2 pixels to vram
INLINE void YUV_putpixel2(u32 x ,u32 y, u32 pixdata)
{
	*(u32*) (&(vram.data [YUV_dest + (YUV_x_curr+x+(YUV_y_curr+y)*YUV_x_size)*2]))  =pixdata;
}

void YUV_init()
{
	YUV_index=0;
	YUV_x_curr=0;
	YUV_y_curr=0;

	YUV_dest=pvr_readreg_TA(0x5F8148,4)&VRAM_MASK;//TODO : add the masking needed
	YUV_doneblocks=0;
	u32 TA_YUV_TEX_CTRL=pvr_readreg_TA(0x5F814C,4);
	YUV_blockcount=(((TA_YUV_TEX_CTRL>>0)&0x3F)+1)*(((TA_YUV_TEX_CTRL>>8)&0x3F)+1);

	if ((TA_YUV_TEX_CTRL>>16 )&1)
	{	//w00t ?
		YUV_x_size=16;
		YUV_y_size=16;
	}
	else
	{	//yesh!!!
		YUV_x_size=(((TA_YUV_TEX_CTRL>>0)&0x3F)+1)*16;
		YUV_y_size=(((TA_YUV_TEX_CTRL>>8)&0x3F)+1)*16;
	}
}


INLINE u8 GetY420(int x, int y,u8* base)
{
	//u32 base=0;
	if (x>7)
	{
		x-=8;
		base+=64;
	}

	if (y>7)
	{
		y-=8;
		base+=128;
	}
	
	return base[x+y*8];
}

INLINE u8 GetUV420(int x, int y,u8* base)
{
	int realx=x>>1;
	int realy=y>>1;

	return base[realx+realy*8];
}
INLINE void YUV_ConvertMacroBlock()
{
	u32 TA_YUV_TEX_CTRL=pvr_readreg_TA(0x5F814C,4);

	//do shit
	YUV_doneblocks++;
	YUV_index=0;

	int block_size=(TA_YUV_TEX_CTRL & (1<<24))==0?384:512;
	
	//YUYV
	if (block_size==384)
	{
		u8* U=(u8*)&YUV_tempdata[0];
		u8* V=(u8*)&YUV_tempdata[64/4];
		u8* Y=(u8*)&YUV_tempdata[(64+64)/4];
		
		u8  yuyv[4];

		for (int y=0;y<16;y++)
		{
			for (int x=0;x<16;x+=2)
			{
				yuyv[1]=GetY420(x,y,Y);
				yuyv[0]=GetUV420(x,y,U);
				yuyv[3]=GetY420(x+1,y,Y);
				yuyv[2]=GetUV420(x,y,V);
				//pixel x,y , x+1,y
				YUV_putpixel2(x,y,*(u32*)yuyv);
			}
		}

		YUV_x_curr+=16;
		if (YUV_x_curr==YUV_x_size)
		{
			YUV_x_curr=0;
			YUV_y_curr+=16;
			if (YUV_y_curr==YUV_y_size)
			{
				YUV_y_curr=0;
			}
		}
	}
	else
	{
		printf("YUV4:2:2 not supported (YUV converter)");
		u8* U0=(u8*)&YUV_tempdata[0];
		u8* V0=(u8*)&YUV_tempdata[64/4];
		u8* Y0=(u8*)&YUV_tempdata[(64+64)/4];

		u8* U1=(u8*)&YUV_tempdata[256/4];
		u8* V1=(u8*)&YUV_tempdata[(64+256)/4];
		u8* Y1=(u8*)&YUV_tempdata[(64+64+256)/4];

	}


	if (YUV_blockcount==YUV_doneblocks)
	{
		YUV_init();
		//TODO : Check if it's allrgiht to do it here?
		RaiseInterrupt(holly_YUV_DMA);
	}
}
void YUV_data(u32* data , u32 count)
{
	if (YUV_blockcount==0)
	{
		printf("YUV_data : YUV decoder not inited , *WATCH*\n");
		//wtf ? not inited
		YUV_init();
	}
	u32 TA_YUV_TEX_BASE=pvr_readreg_TA(0x5F8148,4);
	u32 TA_YUV_TEX_CTRL=pvr_readreg_TA(0x5F814C,4);

	printf("Yuv Converter : size %d\n",count);
	printf("Yuv Format : %s , texture type %d ,  %d x %d [0x%X];",
		(TA_YUV_TEX_CTRL & (1<<24))==0?"YUV420":"YUV422",
		(TA_YUV_TEX_CTRL>>16 )&1,
		(((TA_YUV_TEX_CTRL>>0)&0x3F)+1)*16,
		(((TA_YUV_TEX_CTRL>>8)&0x3F)+1)*16,
		TA_YUV_TEX_CTRL);
	
	if ((TA_YUV_TEX_CTRL & (1<<24))==0)
	{
		printf("%d blocks;",count*32/384);
	}
	else
		printf("%d blocks;",count*32/512);

	printf("Destination : 0x%X\n",TA_YUV_TEX_BASE);


	//YUV420 is 384 bytes , YUV422 is 512 bytes
	u32 block_size=(TA_YUV_TEX_CTRL & (1<<24))==0?384:512;

	/*
	for (u32 i=0;i<count*32;i+=4)
	{
		if (YUV_index==block_size)
			YUV_ConvertMacroBlock();

		YUV_tempdata[YUV_index>>2]=*data;
		data++;
		YUV_index+=4;
	}
	if (YUV_index==block_size)
			YUV_ConvertMacroBlock();*/
	
	count*=32;
	while(count>0)
	{
		if ((YUV_index+count)>=block_size)
		{	//more or exactly one block remaining
			u32 dr=block_size-YUV_index;				//remaining byts til block end
			memcpy(&YUV_tempdata[YUV_index>>2],data,dr);	//copy em
			data+=dr>>2;								//count em
			count-=dr;
			YUV_ConvertMacroBlock();					//convert block
		}
		else
		{	//less that a hole block remaining
			memcpy(&YUV_tempdata[YUV_index>>2],data,count);	//append it
			YUV_index+=count;
			count=0;
		}
	}
}

//Regs

u32 pvr_readreg_TA(u32 addr,u32 sz)
{
	if ((addr&0xFFFFFF)==0x5F8150)
		return YUV_doneblocks;
	//EMUERROR3("Not implemented TA register read , addr=%d,sz=%d",addr,sz);
	return libPvr->pvr_info.ReadReg(addr,sz);//__pvr_read__reg(addr);
}

void pvr_writereg_TA(u32 addr,u32 data,u32 sz)
{
	//__pvr_write_reg(addr,data);
	libPvr->pvr_info.WriteReg(addr,data,sz);
	if ((addr&0xFFFFFF)==0x5F8148)
		YUV_init();
}

#define vram_64b
//vram 32-64b
#ifdef vram_64b
//read
u8 __fastcall pvr_read_area1_8(u32 addr)
{
	printf("8 bit vram writes are not possible\n");
	return 0;
}

u16 __fastcall pvr_read_area1_16(u32 addr)
{
	addr =vramlock_ConvOffset32toOffset64(addr);
	return *(u16*)&vram[addr];
}
u32 __fastcall pvr_read_area1_32(u32 addr)
{
	addr =vramlock_ConvOffset32toOffset64(addr);
	return *(u32*)&vram[addr];
}

//write
void __fastcall pvr_write_area1_8(u32 addr,u8 data)
{
	printf("8 bit vram writes are not possible\n");
}
void __fastcall pvr_write_area1_16(u32 addr,u16 data)
{
	addr=vramlock_ConvOffset32toOffset64(addr);
	*(u16*)&vram[addr]=data;
}
void __fastcall pvr_write_area1_32(u32 addr,u32 data)
{
	addr=vramlock_ConvOffset32toOffset64(addr);
	*(u32*)&vram[addr]=data;
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
		libPvr->pvr_info.TADma(address,data,count);
	}
	else if(address_w<0x1000000) //Yuv Converter
	{
		YUV_data(data,count);
	}
	else //Vram Write
	{
		//shoudn't realy get here (?)
		printf("Vram Write 0x%X , size %d",address,count);
	}
}
//Misc interface

//Init/Term , global
void pvr_Init()
{
	vram.Init(VRAM_SIZE);
}
void pvr_Term()
{
	vram.Term();
}
//Reset -> Reset - Initialise to defualt values
void pvr_Reset(bool Manual)
{
	if (!Manual)
		vram.Zero();
}

