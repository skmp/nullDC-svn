/* Tunels dctest plugins <-> icarus plugins , power VR lle
 * this is prop not the fastest way to do this but anyway ....
 * pvr emulation is not the bootleneck :P
 */
#pragma once
#include "nullPvr.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Plugin types */
#define PLUGIN_TYPE_GFX				1
#define PLUGIN_TYLE_LLE				0x1000
#define PLUGIN_TYPE_PVR				0x1001

#define CALL						_cdecl

/* Interrupt defines */
#define RENDER_VIDEO	1
#define OPAQUE_LIST		6
#define OPAQUE_MOD_LIST 7
#define TRANS_LIST		8
#define TRANS_MOD_LIST	9
#define PUNCH_THRU_LIST	18

/* Debug */
#define DEBUG_DP_GFX	(1<<24)

/***** Structures *****/
typedef struct {
	WORD Version;        /* Should be set to 1 */
	WORD Type;           /* Set to PLUGIN_TYPE_GFX */
	char Name[100];      /* Name of the DLL */
} PLUGIN_INFO;


/********** PowerVR2 Plugin Specific **
****************************************/

typedef struct
{
	HWND hWnd;			// Render window 
	HWND hStatusBar;    // if render window has no status bar, this is NULL 

	u8	* VRAM;			// Access to 32-bit video ram (0xa5000000 following) 
	u32	* PVR_REGS;		// Access to PVR registers (0xa05f8000 following) 

	void (*Enqueue_IRQ)		(u32 type);
	void (*Debug_Printf)	(u32 dwDebugFlags, char* szFormat, ... );
	void (*CPU_Halt)		(char * szReason);

	// Silly stats for debug console 
	void (*StatsFrame)		(void);
	void (*StatsVtxStrip)	(void);

	// This can be used to easily save/load a 32-bit config struct for each gfx plugin 
	void (*Save_Config)		(char *RegStr, DWORD Config);
	void (*Load_Config)		(char *RegStr, DWORD *Config);

} GFX_INFO;

typedef BOOL PvrInitFP		( GFX_INFO GfxInfo );	// InitiateGFX
typedef void PvrTermFP		( void );	// GFXCloseDLL
typedef void PvrOpenFP		( void );	// GfxGameOpen
typedef void PvrCloseFP		( void );	// GfxGameClosed

typedef void PvrCommandFP	( u32 *pCmd, u32 dwQW_Len );	// ProcessDList

typedef void PvrWrite32FP	( DWORD dwAddr, DWORD * dwValue );

typedef void PvrUpdateFP	( u32 dwReason );	// SizeScreen | DrawScreen(FB) etc ..

typedef void PvrDllTestFP	( HWND hParent );	// GFXDllTest
typedef void PvrDllAboutFP	( HWND hParent );	// GFXDllAbout
typedef void PvrDllConfigFP( HWND hParent );	// GFXDllConfig

typedef void GetDllInfoFP ( PLUGIN_INFO * PluginInfo );

#if defined(__cplusplus)
}
#endif

struct icPluginfo
{
	PLUGIN_INFO plinfo;
	PvrInitFP*	PvrInit;
	PvrTermFP* PvrTerm;
	PvrOpenFP* PvrOpen;
	PvrCloseFP* PvrClose;
	PvrCommandFP* PvrCommand;
	PvrWrite32FP* PvrWrite32;	
	PvrUpdateFP* PvrUpdate;
	PvrDllTestFP* PvrDllTest;
	PvrDllAboutFP* 	PvrDllAbout;
	PvrDllConfigFP* PvrDllConfig;
	GetDllInfoFP* GetDllInfo;
	void * dllHanlde;
};

extern icPluginfo*cur_icpl;



//helper functions
u8 HblankInfo();
u8 VblankInfo();
//end helper

void icUpdatePvr (u32 cycles);
u32 icReadMem (u32 Address,u32 len);
void PvrSQWrite (u32*data,u32 len);
void icWriteMem (u32 Address,u32 data,u32 len);
u32 icReadMem_reg(u32 Address,u32 len);
#define SOFTRESET			0x005F8008		//RW	CORE & TA software reset
#define STARTRENDER		0x005F8014		//RW	Drawing start
#define TA_LIST_INIT		0x005F8144		//RW	TA initialization

void icWriteMem_reg(u32 Address,u32 data,u32 len);
void icEnqueue_IRQ	(u32 type);
void icDebug_Printf	(u32 dwDebugFlags, char* szFormat, ... );
void icCPU_Halt	(char * szReason);

	// Silly stats for debug console 
	void icStatsFrame		(void);
	void icStatsVtxStrip	(void);

	// This can be used to easily save/load a 32-bit config struct for each gfx plugin 
	void icSave_Config		(char *RegStr, DWORD Config);
	void icLoad_Config		(char *RegStr, DWORD *Config);

bool icInit ();


void icTerm ();

void LoadIcPvrDll(char *dll);


void UnLoadIcPvrDll(void *pl);