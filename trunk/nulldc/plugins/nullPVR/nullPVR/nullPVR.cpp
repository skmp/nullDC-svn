/*
**	nullPVR.cpp,	nullDC::nullPVR	(2007) ZeZu & drk||Raziel
*/

#include "nullPVR.h"





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


void FASTCALL pvrUpdate(u32 cycles)
{
}

u32  FASTCALL pvrReadReg(u32 addr,u32 size)
{
	return 0;
}

void FASTCALL pvrWriteReg(u32 addr,u32 data,u32 size)
{
}

void FASTCALL pvrWriteFifo(u32 address, u32* data, u32 size)
{
}

void FASTCALL pvrLockCB(vram_block *bl, u32 addr)
{
}




