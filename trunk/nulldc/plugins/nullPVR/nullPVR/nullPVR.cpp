/*
**	nullPVR.cpp,	nullDC::nullPVR	(2007) ZeZu & drk||Raziel
*/

#include "nullPVR.h"
#include "pvrMemory.h"




HINSTANCE hInst=NULL;

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD   dwReason,
					  LPVOID  lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}






// PowerVR interface to main emulator
void EXPORT_CALL dcGetInterface(plugin_interface* plIf)
{
	common_info & plcIf		= plIf->common;
	pvr_plugin_if & pvrIf	= plIf->pvr;

	plcIf.Type				= Plugin_PowerVR;
	plcIf.PluginVersion		= DC_MakeVersion(0,9,0,DC_VER_NORMAL);;
	plcIf.InterfaceVersion	= PVR_PLUGIN_I_F_VERSION;
	plIf->InterfaceVersion	= PLUGIN_I_F_VERSION;
	sprintf(plcIf.Name,		 "nullPVR v1.0.0a");

	plcIf.Load				= pvrLoad;
	plcIf.Unload			= pvrUnload;

	pvrIf.Init				= pvrInit;
	pvrIf.Term				= pvrTerm;
	pvrIf.Reset				= pvrReset;

	pvrIf.ReadReg			= pvrReadReg;
	pvrIf.WriteReg			= pvrWriteReg;
	pvrIf.UpdatePvr			= pvrUpdate;
	pvrIf.LockedBlockWrite	= pvrLockCB;
	pvrIf.TaFIFO			= pvrWriteFifo;

	pvrIf.ExeptionHanlder	= 0;

//	pvrIf.Config			= pvrConfig;
}








s32  FASTCALL pvrInit(pvr_init_params*)
{
	return 0;
}

void FASTCALL pvrTerm()
{
}

void FASTCALL pvrReset(bool)
{
}

s32  FASTCALL pvrLoad(emu_info*,u32)
{
	printf("pvrLoad() \n");
	return 0;
}

void FASTCALL pvrUnload()
{
}

void FASTCALL pvrConfig(void*)
{
}



void FASTCALL pvrWriteFifo(u32 address, u32* data, u32 size)
{
}

void FASTCALL pvrLockCB(vram_block *bl, u32 addr)
{
}





///////////// to be moved ?  ////////////////////


#define VSYNC_WRAP	(200 * 1000000 / 60)


u32 vblState=0;
u32 vblCount=0;
u32 last_fps=0;
u32 FrameCount=0;

S_INLINE
void vblank_done()
{
	vblCount++;
	if ((timeGetTime()-(double)last_fps)>800)
	{
		double spd_fps=(double)FrameCount/(double)((double)(timeGetTime()-(double)last_fps)/1000);
		double spd_cpu=(double)vblCount/(double)((double)(timeGetTime()-(double)last_fps)/1000);
		spd_cpu*=VSYNC_WRAP;
		spd_cpu/=1000000;	//mrhz kthx
		double fullfps=(spd_fps/spd_cpu)*200;

		last_fps=timeGetTime();
		FrameCount=0;
		vblCount=0;

		char fpsStr[256];
		sprintf(fpsStr," FPS: %4.2f(%4.2f)  -  Sh4: %4.2f mhz (%4.2f%%) - nullDC v0.0.1", spd_fps,fullfps, spd_cpu,spd_cpu*100/200);
//		SetWindowText((HWND)emuIf.handle, fpsStr);

	}

	//printf(" vbl1 :: ListInit: %d\n", TA_State.ListInit);
	//if(0==TA_State.ListInit)
	//	PvrIf::Render();

}


u32 pvrCycles=0;

void FASTCALL pvrUpdate(u32 cycles)
{
	/*
	static u32 lastLine=8000;

	pvrCycles += (cycles);
	pvrCycles -= (VSYNC_WRAP<=pvrCycles) ? VSYNC_WRAP : 0 ;

	u32 vblCount= (*pSPG_LOAD &0x3FF)+1;		// Cycles per line
	u32 vblLine	= (pvrCycles / (vblCount * 7));	// Current Line

	if(vblLine == lastLine) return;				// might save us a bit of time
//	emuIf.RaiseInterrupt(holly_HBLank);			// yes its hblank time

	u32 vblStart= (*pSPG_VBLANK) &0x3FF;		// VSync Start
	u32 vblEnd	= (*pSPG_VBLANK>>16) &0x3FF;	// VSync End

	if(vblStart>600) vblStart = 500;

	*pSPG_STATUS = (*pSPG_STATUS & ~0x3FF) | (vblLine & 0x3FF);

	if(vblLine >= (*pSPG_VBLANK_INT &0x3FF) && !(vblState&2)) {
		emuIf.RaiseInterrupt(holly_SCANINT1);
		vblState |= 2;
	}
	if(vblLine >= (*pSPG_VBLANK_INT >>16 &0x3FF) && !(vblState&4)) {
		emuIf.RaiseInterrupt(holly_SCANINT2);
		vblState |= 4;
	}

	if(vblEnd > vblStart)
	{
		if(!(vblState&1) && (vblStart <= vblLine) && (vblEnd > vblLine))
		{
			++vblState;
			vblank_done();
			*pSPG_STATUS |= 0x00002000;

			emuIf.RaiseInterrupt(holly_HBLank);
		}
		if((vblState&1) && (vblEnd <= vblLine))
		{
			vblState = 0;
			*pSPG_STATUS &= ~0x00002000;
		}
	}
	else
	{
		if(!(vblState&1) && (vblStart <= vblLine))
		{
			++vblState;
			vblank_done();
			*pSPG_STATUS |= 0x00002000;

			emuIf.RaiseInterrupt(holly_HBLank);
		}
		if((vblState&1) && (vblEnd <= vblLine) && (vblStart > vblLine))
		{
			vblState = 0;
			*pSPG_STATUS &= ~0x00002000;
		}
	}
*/
}