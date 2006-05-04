/*
**	zNullPvrGL.cpp	David Miller (2006) - PowerVR CLX2 Emulation Plugin -
*/
#include <windows.h>
#include "PowerVR2.h"

pvrInitParams emuIf;

ConfigLoadStrFP	* ConfigLoadStr;
ConfigSaveStrFP	* ConfigSaveStr;


HINSTANCE hInst=NULL;

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD   ul_reason_for_call,
					  LPVOID  lpReserved)
{
	switch (ul_reason_for_call)
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





/*
**	NullDC specific Interface,
**	returns valid function ptrs
*/

extern "C" __declspec(dllexport) 
void dcGetPluginInfo(ndcPluginIf *If)
{
	If->IfVersion	= 0x00010000;	// double check with nullgdr
	If->LibVersion= 0x01;			// 
	If->PluginType= 0x01;			// 1=PowerVR2
	strcpy_s(If->szName, "zNullPvrGL, OpenGL PowerVR2 Emulation Plugin By _ZeZu_");

	If->Init		= pvrInit;
	If->Term		= pvrTerm;
	If->Reset		= pvrReset;
	If->ThreadInit	= pvrThreadInit;
	If->ThreadTerm	= pvrThreadTerm;
	If->Config		= pvrConfig;

	ConfigLoadStr	= If->ConfigLoadStr;
	ConfigSaveStr	= If->ConfigSaveStr;
}

extern "C" __declspec(dllexport) 
void dcGetPvrInfo(pvrPluginIf *If)
{
	If->Version		= 0x00000200;
	If->TaFifo		= TaFifo;
	If->UpdatePvr	= pvrUpdate;
	If->vramLockCB	= vramLockCB;
	If->ReadReg		= pvrReadReg;
	If->WriteReg	= pvrWriteReg;
}






void pvrReset(bool, u32 PluginType)
{
	InitTA_Regs();
	printf("pvrReset()\n");
}



void pvrInit(void *params, u32 PluginType)
{
	emuIf = *(pvrInitParams *)params;

	InitTA_Regs();

	printf("pvrInit()\n");


	if(sizeof(PCW) != 4)
		MessageBox(NULL,"WTFH","FUCK",MB_ICONERROR);

}
void pvrTerm(u32 PluginType)
{
	printf("pvrTerm()\n");
}


void pvrThreadInit(u32 PluginType)
{
	printf("pvrThreadInit()\n");

	PvrIf.Init();
}

void pvrThreadTerm(u32 PluginType)
{
	printf("pvrThreadTerm()\n");

	PvrIf.Term();
}







void pvrConfig(u32 PluginType, void* whoknows)
{
	printf("no config~\n");


	//PvrIf.Config();
}






///////////// to be moved ?


#define VSYNC_WRAP	(200 * 1000000 / 60)


u32 vblState=0;
u32 vblCount=0;
u32 last_fps=0;

void vblank_done()
{
	vblCount++;
	if ((timeGetTime()-(double)last_fps)>800)
	{
		double spd_fps=(double)PvrIf.FrameCount/(double)((double)(timeGetTime()-(double)last_fps)/1000);
		double spd_cpu=(double)vblCount/(double)((double)(timeGetTime()-(double)last_fps)/1000);
		spd_cpu*=VSYNC_WRAP;
		spd_cpu/=1000000;	//mrhz kthx

		last_fps=timeGetTime();
		PvrIf.FrameCount=0;
		vblCount=0;

		char fpsStr[256];
		sprintf_s(fpsStr," FPS: %f  -  Sh4: %f mhz  - nullDC v0.0.1", spd_fps, spd_cpu);
		SetWindowText((HWND)emuIf.handle, fpsStr);

	}
}


u32 pvrCycles=0;

void pvrUpdate(u32 cycles)
{
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
	//	lprintf("ScanInt1\n");
	}
	if(vblLine >= (*pSPG_VBLANK_INT >>16 &0x3FF) && !(vblState&4)) {
		emuIf.RaiseInterrupt(holly_SCANINT2);
		vblState |= 4;
	//	lprintf("ScanInt2\n");
	}


	if(vblEnd > vblStart)
	{
		if(!(vblState&1) && (vblStart <= vblLine) && (vblEnd > vblLine))
		{
			++vblState;
			vblank_done();
			*pSPG_STATUS |= 0x00002000;

			emuIf.RaiseInterrupt(holly_HBLank);
		//	lprintf("VblankStart1\n");
		}
		if((vblState&1) && (vblEnd <= vblLine))
		{
			vblState = 0;
			*pSPG_STATUS &= ~0x00002000;

		//	lprintf("VblankEnd1\n");
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
		//	lprintf("VblankStart2\n");
		}
		if((vblState&1) && (vblEnd <= vblLine) && (vblStart > vblLine))
		{
			vblState = 0;
			*pSPG_STATUS &= ~0x00002000;

		//	lprintf("VblankEnd2\n");
		}
	}

}







//////// TEMP ?
# define _LOG_ (1)

u32  DebugOptions=0;
bool bLogEnabled=true;

void lprintf(char* szFmt, ... )
{
	if(bLogEnabled)
	{
		FILE * f;// = 
		fopen_s(&f, "zNullPVR.txt","a+t");

		va_list va;
		va_start(va, szFmt);
		vfprintf_s(f,szFmt,va);
		va_end(va);

		fclose(f);
	}
}

