#include "spg.h"
//#include "renderer_if.h"
#include "regs.h"
//#include "pvr.h"
#include "tavideo.h"

//SPG emulation; Scanline/Raster beam registers & interrupts

u32 clc_pvr_scanline;
u32 pvr_numscanlines=512;
u32 prv_cur_scanline=-1;
u32 vblk_cnt=0;

u8 VblankInfo()
{
	u32 data = SPG_VBLANK_INT;
	if (data==0)
		return 0;
	if (((data & 0x3FFF) <= prv_cur_scanline) || (((data >> 16) & 0x3FFF) <= prv_cur_scanline))
		return 1;
	else
		return 0;
}

int frame_count=0;
u32 lasft_fps;
double spd_fps=0;
double spd_cpu=0;


extern int CurrentFrame;
//u32 vblLine	= (pvrCycles / (vblCount * 7));	// Current Line
void vblank_done()
{
	//vblk_cnt++;

	CurrentFrame++;
	if ((timeGetTime()-(double)lasft_fps)>800)
	{
		spd_fps=(double)frame_count/(double)((double)(timeGetTime()-(double)lasft_fps)/1000);
		spd_cpu=(double)vblk_cnt/(double)((double)(timeGetTime()-(double)lasft_fps)/1000);
		spd_cpu*=(DCclock/1000000)/60;

		//ints=0;
		lasft_fps=timeGetTime();
		frame_count=0;

		double fullfps=(spd_fps/spd_cpu)*200;

		char fpsStr[256];
		sprintf(fpsStr," FPS: %4.2f(%4.2f)  -  Sh4: %4.2f mhz (%4.2f%%) - nullDC v0.0.1", spd_fps,fullfps, spd_cpu,spd_cpu*100/200);
		SetWindowText((HWND)Hwnd, fpsStr);
		vblk_cnt=0;
	}
}

//called from sh4 context , should update pvr/ta state and evereything else
void spgUpdatePvr(u32 cycles)
{
	/*
	clcount+=cycles;
	if (clcount>dc)
	{
		
		clcount-=(DCclock)/60;//faked
		//ok .. here , after much effort , we reached a full screen redraw :P
		//now , we will copy everything onto the screen (meh) and raise a vblank interupt
		RaiseInterrupt(holly_VBLank);//weeeee
		if (cur_icpl->PvrUpdate)
			cur_icpl->PvrUpdate(1);

			// didn't look at read i guess this is not needed 
		*(u32*)&pvr_regs[0x5F810C &0x7fff] |= 0x2000;	// SPG_STATUS

		if ((timeGetTime()-(double)lasft_fps)>800)
		{
			spd_fps=(double)fps/(double)((double)(timeGetTime()-(double)lasft_fps)/1000);
			//printf("FPS : %f ; list type : %x\n",_spd_fps,ints);
			ints=0;
			lasft_fps=timeGetTime();
			fps=0;

			char fpsStr[256];
			extern HWND g_hWnd;
			//sprintf(fpsStr," FPS: %f  -  Sh4: %f mhz  - Rx: %d kb/s - Tx %d kb/s  DCtest0r v0.0.0 ", spd_fps, 0,0,0);
			//SetWindowText(g_hWnd, fpsStr);
			vblk_cnt=0;
		}
		
	}*/

	clc_pvr_scanline += cycles;
	if (clc_pvr_scanline >  Line_Cycles)//60 ~herz = 200 mhz / 60=3333333.333 cycles per screen refresh
	{
		//ok .. here , after much effort , we did one line
		//now , we must chekc for raster beam interupts and vblank
		prv_cur_scanline=(prv_cur_scanline+1)%pvr_numscanlines;
		clc_pvr_scanline -= Line_Cycles;

		//Check for scanline interrupts -- realy need to test the scanline values
		u32 data=SPG_VBLANK_INT;
		if ((data & 0x3FFF) == prv_cur_scanline)
			RaiseInterrupt(holly_SCANINT1);
		if (((data >> 16) & 0x3FFF) == prv_cur_scanline)
			RaiseInterrupt(holly_SCANINT2);

		SPG_STATUS=((VblankInfo()/*Vblank*/) << 13) | ((0/*hblank*/) << 12) | ((0/*?*/) << 10) | prv_cur_scanline;

		//Vblank start -- realy need to test the scanline values
		if (prv_cur_scanline==0)
		{
			//Vblank counter
			vblk_cnt++;
			RaiseInterrupt(InterruptID::holly_HBLank);
			TAStartVBlank();
			vblank_done();
			//TODO : Renderer->PresentFB();
//			renderer->PresentFB();//present FB data
		}
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
}