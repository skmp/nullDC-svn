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
union FB_R_CTRL_type
{
	struct
	{
		u32 fb_enable:1;			//0
		u32 fb_line_double:1;		//1
		u32 fb_depth:2;			//3-2
		u32 fb_concat:3;			//6-4
		u32 R:1;					//7
		u32 fb_chroma_threshold:8;	//15-8
		u32 fb_stripsize:6;		//21-16
		u32 fb_strip_buf_en:1;		//22
		u32 vclk_div:1;	//		23
		u32 Reserved:8;	//		bit 31-24
	};
	u32 full;
};

union SPG_CONTROL_type
{
	struct
	{
		u32 mhsync_pol:1;			//0
		u32 mvsync_pol:1;			//1
		u32 mcsync_pol:1;			//2
		u32 spg_lock:1;			//3
		u32 interlace:1;			//4
		u32 force_field2:1;		//5
		u32 NTSC:1;				//6
		u32 PAL	:1;				//7
		u32 sync_direction:1;		//8
		u32 csync_on_h:1;			//9
		u32 Reserved:22;			//31-10
	};
	u32 full;
};

union SPG_LOAD_type
{
	struct
	{
		u32 hcount : 10;//9-0
		u32 res : 6 ;	//15-10	
		u32 vcount : 10;//25-16
		u32 res1 : 6 ;	//31-26
	};
	u32 full;
};


void PrintfInfo()
{
	FB_R_CTRL_type FB_R_CTRL_data;
	FB_R_CTRL_data.full=FB_R_CTRL;
	printf("FB_R_CTRL : fb_enable %x ,fb_line_double %x,fb_depth %x,fb_concat %x,fb_chroma_threshold %x,fb_stripsize %x,fb_strip_buf_en %x,vclk_div %x\n",
		FB_R_CTRL_data.fb_enable,FB_R_CTRL_data.fb_line_double,FB_R_CTRL_data.fb_depth,FB_R_CTRL_data.fb_concat
		,FB_R_CTRL_data.fb_chroma_threshold,FB_R_CTRL_data.fb_stripsize,FB_R_CTRL_data.fb_strip_buf_en,
		FB_R_CTRL_data.vclk_div);

	SPG_CONTROL_type spg_c;
	spg_c.full=SPG_CONTROL;

	printf("spg_c : interlace %x ,force_field2 %x,NTSC %x,PAL %x,sync_direction %x\n",
		spg_c.interlace,spg_c.force_field2,spg_c.NTSC,spg_c.PAL
		,spg_c.sync_direction);

	SPG_LOAD_type spg_l;
	spg_l.full=SPG_LOAD;
	printf("spg_l : hcount %x ,vcount %x\n",
		spg_l.hcount,spg_l.vcount);


	SPG_LOAD_type spg_vbl;
	spg_vbl.full=SPG_VBLANK_INT ;
		printf("spg_vbl : vbi %x ,vbo %x\n",
		spg_vbl.hcount,spg_vbl.vcount);
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
		//PrintfInfo();
		//TODO : fix that mess -- now uses hacksync ;) later will be async too :P:P
		
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

	if ((addr&RegMask)>=PALETTE_RAM_START_addr)
		pal_needs_update=true;
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