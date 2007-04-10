#include "spg.h"
#include "renderer_if.h"
#include "regs.h"

//SPG emulation; Scanline/Raster beam registers & interrupts
//Time to emulate that stuff correctly ;)

u32 in_vblank=0;
u32 clc_pvr_scanline;
u32 pvr_numscanlines=512;
u32 prv_cur_scanline=-1;
u32 vblk_cnt=0;

u32 last_fps=0;

//54 mhz pixel clock :)
#define PIXEL_CLOCK (54*1000*1000)
u32 Line_Cycles=0;
u32 Frame_Cycles=0;
void CalculateSync()
{
	//clc_pvr_scanline=0;

	u32 pixel_clock;

	if (FB_R_CTRL.vclk_div)
	{
		//VGA :)
		pixel_clock=PIXEL_CLOCK;
	}
	else
	{
		//It is half for NTSC/PAL
		pixel_clock=PIXEL_CLOCK/2;
	}

	//We need to caclulate the pixel clock
	u32 frame_cycles;
	u32 sync_cycles=(SPG_LOAD.hcount+1)*(SPG_LOAD.vcount+1);
	pvr_numscanlines=SPG_LOAD.vcount+1;
	if (SPG_CONTROL.interlace)
	{
		frame_cycles=sync_cycles*2;
	}
	else
	{
		frame_cycles=sync_cycles;
	}
	
	
	Frame_Cycles=(u64)DCclock*(u64)sync_cycles/(u64)pixel_clock;
	Line_Cycles=(u64)DCclock*(u64)(SPG_LOAD.hcount+1)/(u64)pixel_clock;

	/*
	printf("*******************************\n");
	printf("FB_R_CTRL : fb_enable %x ,fb_line_double %x,fb_depth %x,fb_concat %x,fb_chroma_threshold %x,fb_stripsize %x,fb_strip_buf_en %x,vclk_div %x\n",
		FB_R_CTRL.fb_enable,FB_R_CTRL.fb_line_double,FB_R_CTRL.fb_depth,FB_R_CTRL.fb_concat
		,FB_R_CTRL.fb_chroma_threshold,FB_R_CTRL.fb_stripsize,FB_R_CTRL.fb_strip_buf_en,
		FB_R_CTRL.vclk_div);

	printf("SPG_CONTROL : interlace %x ,force_field2 %x,NTSC %x,PAL %x,sync_direction %x\n",
		SPG_CONTROL.interlace,SPG_CONTROL.force_field2,SPG_CONTROL.NTSC,SPG_CONTROL.PAL
		,SPG_CONTROL.sync_direction);

	printf("SPG_LOAD : hcount %d ,vcount %d\n",
		SPG_LOAD.hcount,SPG_LOAD.vcount);


	printf("SPG_VBLANK_INT : vbi %x ,vbo %x\n",
		SPG_VBLANK_INT.vblank_in_interrupt_line_number,SPG_VBLANK_INT.vblank_out_interrupt_line_number);
	
	printf("Pixel Clock %d\n",pixel_clock);

	printf("frame_cycles %d\n",frame_cycles);
	printf("FRAME rate : %d\n",pixel_clock/frame_cycles);
	printf("VSYNC rate : %d\n",pixel_clock/sync_cycles);

	printf("SPG_WIDRH : bpwidth %d,eqwidth %d,vswidth %d,hswidth %d\n",	SPG_WIDTH.bpwidth,
									SPG_WIDTH.eqwidth,
									SPG_WIDTH.vswidth,
									SPG_WIDTH.hswidth);
	*/
}

bool render_end_pending=false;
u32 render_end_pending_cycles;
//called from sh4 context , should update pvr/ta state and evereything else
void FASTCALL spgUpdatePvr(u32 cycles)
{
	if (Line_Cycles==0)
		return;
	clc_pvr_scanline += cycles;

	if (clc_pvr_scanline >  Line_Cycles)//60 ~herz = 200 mhz / 60=3333333.333 cycles per screen refresh
	{
		//ok .. here , after much effort , we did one line
		//now , we must chekc for raster beam interupts and vblank
		prv_cur_scanline=(prv_cur_scanline+1)%pvr_numscanlines;
		clc_pvr_scanline -= Line_Cycles;
		//Check for scanline interrupts -- realy need to test the scanline values
		
		if (SPG_VBLANK_INT.vblank_in_interrupt_line_number == prv_cur_scanline)
			params.RaiseInterrupt(holly_SCANINT1);

		if (SPG_VBLANK_INT.vblank_out_interrupt_line_number == prv_cur_scanline)
			params.RaiseInterrupt(holly_SCANINT2);

		if (SPG_VBLANK.vstart == prv_cur_scanline)
			in_vblank=1;

		if (SPG_VBLANK.vbend == prv_cur_scanline)
			in_vblank=0;

		if (SPG_CONTROL.interlace)
			SPG_STATUS.fieldnum=~SPG_STATUS.fieldnum;
		else
			SPG_STATUS.fieldnum=0;

		SPG_STATUS.vsync=in_vblank;
		SPG_STATUS.scanline=prv_cur_scanline;

		//Vblank start -- realy need to test the scanline values
		if (prv_cur_scanline==0)
		{
			//Vblank counter
			vblk_cnt++;
			params.RaiseInterrupt(InterruptID::holly_HBLank);// -> This turned out to be HBlank btw , needs to be emulater ;(
			//TODO : rend_if_VBlank();
			rend_vblank();//notify for vblank :)

			if ((timeGetTime()-last_fps)>800)
			{
				double spd_fps=(double)(FrameCount)/(double)((double)(timeGetTime()-(double)last_fps)/1000);
				double spd_vbs=(double)(vblk_cnt)/(double)((double)(timeGetTime()-(double)last_fps)/1000);
				double spd_cpu=spd_vbs*Frame_Cycles;
				spd_cpu/=1000000;	//mrhz kthx
				double fullvbs=(spd_vbs/spd_cpu)*200;
				double mv=VertexCount /1000000.0;
				VertexCount=0;
				last_fps=timeGetTime();
				FrameCount=0;
				vblk_cnt=0;

				char fpsStr[256];
				sprintf(fpsStr,"%s - %4.2f%% - VPS: %4.2f(%4.2f) RPS: %4.2f Vert: %4.2fM Sh4: %4.2f mhz", 
					emu_name,spd_cpu*100/200,spd_vbs,fullvbs,
					spd_fps,mv, spd_cpu);

				rend_set_fps_text(fpsStr);
			}
		}
	}

	if (render_end_pending)
	{
		if (render_end_pending_cycles<cycles)
		{
			render_end_pending=false;
			params.RaiseInterrupt(InterruptID::holly_RENDER_DONE);
			params.RaiseInterrupt(InterruptID::holly_RENDER_DONE_isp);
			params.RaiseInterrupt(InterruptID::holly_RENDER_DONE_vd);
			rend_end_render();
		}
		render_end_pending_cycles-=cycles;
	}
}


bool spg_Init()
{
	return true;
}

void spg_Term()
{
}

void spg_Reset(bool Manual)
{
	CalculateSync();
}