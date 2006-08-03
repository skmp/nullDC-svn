/*
**	zNullPvr.h	David Miller (2006) - PowerVR CLX2 Emulation Plugin -
*/

#ifndef __ZNULLPVR_H__
#define __ZNULLPVR_H__

#include <stdio.h>


// only for msvc
typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef float f32;
typedef double f64;





//#ifdef _DEBUG
#define DEBUG_LIB (1)
//#endif

#ifdef DEBUG_LIB
#define ASSERT_T(cond,str) if((cond)) printf("#!\tERROR: ASSERTION FAILED: %s !\n", str);
#define ASSERT_F(cond,str) if(!(cond)) printf("#!\tERROR: ASSERTION FAILED: %s !\n", str);
#else
#define ASSERT_T(cond,str)
#define ASSERT_F(cond,str)
#endif

extern bool bLogEnabled;
extern u32  DebugOptions;


void lprintf(char* szFmt, ... );


/*
**	Modified Plugin Specs for NullDC
**	they are ugly but less so
*/


enum InterruptID
{
	holly_nrm = 0x20000000,
	holly_err = 0x22000000,

	// asic9a /sh4 external holly normal [internal]
	holly_RENDER_DONE_vd	= holly_nrm | 0x00,		//bit 0 = End of Render interrupt : Video
	holly_RENDER_DONE_isp	= holly_nrm | 0x01,		//bit 1 = End of Render interrupt : ISP
	holly_RENDER_DONE		= holly_nrm | 0x02,		//bit 2 = End of Render interrupt : TSP
	holly_SCANINT1			= holly_nrm | 0x03,		//bit 3 = V Blank-in interrupt
	holly_SCANINT2			= holly_nrm | 0x04,		//bit 4 = V Blank-out interrupt
	holly_HBLank			= holly_nrm | 0x05,		//bit 5 = H Blank-in interrupt
	holly_YUV_DMA			= holly_nrm | 0x06,		//bit 6 = End of YUV DMA
	holly_OPAQUE			= holly_nrm | 0x07,		//bit 7 = End of Opaque List
	holly_OPAQUEMOD			= holly_nrm | 0x08,		//bit 8 = End of Opaque Modifier Volume List
	holly_TRANS				= holly_nrm | 0x09,		//bit 9 = End of Transferring interrupt : Translucent List
	holly_TRANSMOD			= holly_nrm | 0x0a,		//bit 10 = End of Transferring interrupt : Translucent Modifier Volume List
	holly_PVR_DMA			= holly_nrm | 0x0b,		//bit 11 = End of DMA interrupt : PVR-DMA
	holly_PUNCHTHRU			= holly_nrm | 0x15,		//bit 21 = End of Transferring interrupt : Punch Through List

	// asic9b/sh4 external holly err only [error]
	holly_PRIM_NOMEM		= holly_err | 0x02,		//bit 2 = TA : ISP/TSP Parameter Overflow
	holly_MATR_NOMEM		= holly_err | 0x03		//bit 3 = TA : Object List Pointer Overflow
};


typedef void TermFP(u32 PluginType);
typedef void ThreadTermFP(u32 PluginType);
typedef void ThreadInitFP(u32 PluginType);
typedef void InitFP(void * param, u32 PluginType);
typedef void ResetFP(bool bManual, u32 PluginType);
typedef void ConfigFP(u32 PluginType, void* handle);

typedef void UpdateFP(u32 cycles);
typedef u32  ReadMemFP(u32 addr,u32 size);
typedef void WriteMemFP(u32 addr,u32 data,u32 size);

typedef void RaiseInterruptFP(InterruptID intr);

//These are provided by the emu
typedef void ConfigLoadStrFP(const char * Section, const char * Key, char * Return);
typedef void ConfigSaveStrFP(const char * Section, const char * Key, const char * String);

typedef struct
{
	u32	 IfVersion;
	char szName[128];
	u32	 LibVersion;
	u32	 PluginType;

	InitFP			* Init;
	TermFP			* Term;
	ResetFP			* Reset;
	ThreadInitFP	* ThreadInit;
	ThreadTermFP	* ThreadTerm;
	ConfigFP		* Config;
	void* UEH;		//unhandled write handler

	//Functions "Exported" from the emu , these are SET by the emu
	ConfigLoadStrFP	* ConfigLoadStr;
	ConfigSaveStrFP	* ConfigSaveStr;

} ndcPluginIf;




/*
**	PVR Specific
*/


struct vram_block
{
	u32 start;
	u32 end;
	u32 len;
	u32 type;

	void* userdata;
};

typedef void vramUnlockFP(vram_block *bl);
typedef void vramLockCBFP(vram_block *bl, u32 addr);
typedef vram_block* vramLock32FP(u32 start, u32 end, void* user);
typedef vram_block* vramLock64FP(u32 start, u32 end, void* user);

typedef void TaFifoFP(u32 address, u32* data, u32 size);


struct pvrInitParams
{
	//vram/regs arrays
	u8				* vram;
	RaiseInterruptFP* RaiseInterrupt;
	
	void			* handle;
	vramLock32FP	* vramLock32;
	vramLock64FP	* vramLock64;
	vramUnlockFP	* vramUnlock;
};


struct pvrPluginIf
{
	u32				Version;

	UpdateFP*		UpdatePvr;		//called from sh4 context , should update pvr/ta state and evereything else
	TaFifoFP*		TaFifo;			//called from sh4 context , in case of dma or SQ to TA memory , size is 32 byte transfer counts
	ReadMemFP*		ReadReg;
	WriteMemFP*		WriteReg;
	vramLockCBFP*	vramLockCB;
};


typedef void dcGetPvrInfoFP(pvrPluginIf *If);







/*
**	Prototypes for these ugly bastards
*/

void pvrTerm(u32);
void pvrInit(void*,u32);
void pvrReset(bool,u32);
void pvrThreadInit(u32);
void pvrThreadTerm(u32);
void pvrConfig(u32,void*);

extern ConfigLoadStrFP	*ConfigLoadStr;
extern ConfigSaveStrFP	*ConfigSaveStr;



void pvrUpdate(u32 cycles);
u32  pvrReadReg(u32 addr,u32 size);
void pvrWriteReg(u32 addr,u32 data,u32 size);


void TaFifo(u32 address, u32* data, u32 size);

//void vramUnlock(vram_block *bl);
void vramLockCB(vram_block *bl, u32 addr);
//vram_block* vramLock32(u32 start, u32 end, void *user);
//vram_block* vramLock64(u32 start, u32 end, void *user);

extern pvrInitParams emuIf;



static const InterruptID PvrInts[] =	// *FIXME* move back later ?
{
	holly_OPAQUE,
	holly_OPAQUEMOD,
	holly_TRANS,
	holly_TRANSMOD,
	holly_PUNCHTHRU,
	holly_PRIM_NOMEM,	// reserved so i use an error, if you ever see out of mem error ...
};







extern
struct PvrOpts
{
	u8  GfxApi;		// 0=OpenGL, 1=D3D9, 2=Other, 23=Software
	u16 GfXOps;		// We'll See
	u8  Reserved;	// 

} pvrOpts;

enum RendAPI
{
	R_OPENGL=0,
	R_DIRECTX=1,
	R_D3D=1,
	R_OTHER=2,
};



#endif //__ZNULLPVR_H__
