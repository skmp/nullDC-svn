#include "regs.h"
#include "renderer_if.h"
#include "ta.h"


//seems to work allright .. ;P
//many things need to be emulated , especialy to support lle emulation but for now that's not needed
u8 regs[RegSize];

u32 ReadPvrRegister(u32 addr,u32 size)
{
	if (size!=4)
	{
		//error
		return 0;
	}

	return PvrReg(addr);
}

void WritePvrRegister(u32 addr,u32 data,u32 size)
{
	if (size!=4)
	{
		//error
		return;
	}
	if ((addr&RegMask)==ID_addr)
		return;//read olny
	if ((addr&RegMask)==REVISION_addr)
		return;//read olny

	if ((addr&RegMask)==STARTRENDER_addr)
	{
		//start render
		renderer->StartRender();
		//TODO : fix that mess
		
		//RaiseInterrupt(InterruptID::holly_RENDER_DONE);
		//RaiseInterrupt(InterruptID::holly_RENDER_DONE_isp);
		//RaiseInterrupt(InterruptID::holly_RENDER_DONE_vd);
		render_end_pending=true;
		return;
	}

	if ((addr&RegMask)==TA_LIST_INIT_addr)
	{
		if (data>>31)
		{
			renderer->Ta_ListInit();
			data=0;
		}
	}

	if ((addr&RegMask)==SOFTRESET_addr)
	{
		if (data!=0)
		{
			if (data&1)
				renderer->Ta_SoftReset();
			data=0;
		}
	}

	if ((addr&RegMask)==TA_LIST_CONT_addr)
	{
		//a write of anything works ?
		renderer->Ta_ListCont();
	}

	PvrReg(addr)=data;
}

bool Regs_Init()
{
	return true;
}

void Regs_Term()
{
}

void Regs_Reset(bool Manual)
{
	ID					= 0x17FD11DB;
	REVISION			= 0x00000011;
	SOFTRESET			= 0x00000007;
	SPG_HBLANK_INT		= 0x031D0000;
	SPG_VBLANK_INT		= 0x01500104;
	FPU_PARAM_CFG		= 0x0007DF77;
	HALF_OFFSET			= 0x00000007;
	ISP_FEED_CFG		= 0x00402000;
	SDRAM_REFRESH		= 0x00000020;
	SDRAM_ARB_CFG		= 0x0000001F;
	SDRAM_CFG			= 0x15F28997;
	SPG_HBLANK			= 0x007E0345;
	SPG_LOAD			= 0x01060359;
	SPG_VBLANK			= 0x01500104;
	SPG_WIDTH			= 0x07F1933F;
	VO_CONTROL			= 0x00000108;
	VO_STARTX			= 0x0000009D;
	VO_STARTY			= 0x00000015;
	SCALER_CTL			= 0x00000400;
	FB_BURSTCTRL		= 0x00090639;
	PT_ALPHA_REF		= 0x000000FF;
}