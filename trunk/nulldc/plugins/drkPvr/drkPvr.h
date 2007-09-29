#pragma once
#include "config.h"



//bleh stupid windoze header
#include "..\..\nullDC\plugins\plugin_header.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

int msgboxf(char* text,unsigned int type,...);

#define BUILD 0
#define MINOR 1
#define MAJOR 0
#define DCclock (200*1000*1000)


float GetSeconds();


#define dbgbreak __asm {int 3}

#define fastcall __fastcall

#if DO_VERIFY==OP_ON
#define verify(x) if((x)==false){ static bool d_msg=true; if (d_msg) { d_msg = msgboxf("Verify Failed  : " #x "\n in %s -> %s : %d \nWant to report this error again ?",MB_ICONERROR|MB_YESNO,__FUNCTION__,__FILE__,__LINE__)==IDYES?true:false;} if (d_msg){ dbgbreak;}}
#define verifyf(x) if((x)==false){ msgboxf("Verify Failed  : " #x "\n in %s -> %s : %d \n",MB_ICONERROR,__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define verifyc(x) if(FAILED(x)){ msgboxf("Verify Failed  : " #x "\n in %s -> %s : %d \n",MB_ICONERROR,__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#else
#define verify(x)
#define verifyf(x) (x)
#define verifyc(x) (x)
#endif

#define die(reason) { printf("Fatal error : " #reason "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define fverify verify

#define log(xx) printf(xx " (from "__FUNCTION__ ")\n");
#define log1(xx,yy) printf(xx " (from "__FUNCTION__ ")\n",yy);
#define log2(xx,yy,zz) printf(xx " (from "__FUNCTION__ ")\n",yy,zz);
#define log3(xx,yy,gg) printf(xx " (from "__FUNCTION__ ")\n",yy,zz,gg);

#include "helper_classes.h"

extern bool render_end_pending;
extern u32 render_end_pending_cycles;

extern pvr_init_params params;
extern emu_info emu;
extern char emu_name[512];

void LoadSettings();
void SaveSettings();

#if REND_API == REND_D3D
	#define REND_NAME "Direct3D HAL"
	#define GetRenderer GetDirect3DRenderer
#elif REND_API == REND_OGL
	#define REND_NAME "OpenGL HAL"
	#define GetRenderer GetOpenGLRenderer
#elif  REND_API == REND_SW
	#define REND_NAME "Software SBR"
	#define GetRenderer GetSWRenderer
#elif REND_API == REND_D3D_V2
	#define REND_NAME "Direct3D HAL/V2"
	#define GetRenderer GetDirect3DRenderer2
#elif  REND_API == REND_NONE
	#define REND_NAME "No Rendering"
	#define GetRenderer GetNORenderer
#else
	#error invalid config.REND_API must be set with one of REND_D3D/REND_OGL/REND_SW/REND_D3D_V2
#endif

struct _settings_type
{
	struct 
	{
		u32 Enabled;
		u32 Res_X;
		u32 Res_Y;
		u32 Refresh_Rate;
	} Fullscreen;
	struct 
	{
		u32 MultiSampleCount;
		u32 MultiSampleQuality;
		u32 AspectRatioMode;
	} Enhancements;
	
	struct
	{
		u32 PaletteMode;
		u32 AlphaSortMode;
		u32 ModVolMode;
	} Emulation;
	struct
	{
		u32 ShowFPS;
		u32 ShowStats;
	} OSD;
};

extern _settings_type settings;
void UpdateRRect();