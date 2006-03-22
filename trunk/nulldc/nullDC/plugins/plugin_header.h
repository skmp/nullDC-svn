#pragma once

//SHUT UP M$ COMPILER !@#!@$#
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_NO_DEPRECATE 

//Do not complain when i use enum::member
#pragma warning( disable : 4482)

//unnamed struncts/unions
#pragma warning( disable : 4201)

//unused parameters
#pragma warning( disable : 4100)

//basic types
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

//intc function pointer and enums
enum InterruptType
{
	sh4_int   = 0x00000000,
	sh4_exp   = 0x01000000,
	holly_nrm = 0x20000000,
	holly_ext = 0x21000000,
	holly_err = 0x22000000,
	InterruptTypeMask = 0x7F000000,
	InterruptIDMask=0x00FFFFFF
};

enum InterruptID
{
		//internal interups
		//TODO : Add more internal interrrupts
		sh4_TMU0_TUNI0 = sh4_int |	0x0400,  /* TMU0 underflow */
		sh4_TMU1_TUNI1 = sh4_int |  0x0420,  /* TMU1 underflow */
		sh4_TMU2_TUNI2 = sh4_int |  0x0440,  /* TMU2 underflow */

		//sh4 exeptions 
		sh4_ex_USER_BREAK_BEFORE_INSTRUCTION_EXECUTION = sh4_exp | 0x1e0,
		sh4_ex_INSTRUCTION_ADDRESS_ERROR =sh4_exp | 0x0e0,
		sh4_ex_INSTRUCTION_TLB_MISS =sh4_exp | 0x040,
		sh4_ex_INSTRUCTION_TLB_PROTECTION_VIOLATION = sh4_exp |0x0a0,
		sh4_ex_GENERAL_ILLEGAL_INSTRUCTION = sh4_exp |0x180,
		sh4_ex_SLOT_ILLEGAL_INSTRUCTION = sh4_exp |0x1a0,
		sh4_ex_GENERAL_FPU_DISABLE = sh4_exp |0x800,
		sh4_ex_SLOT_FPU_DISABLE = sh4_exp |0x820,
		sh4_ex_DATA_ADDRESS_ERROR_READ =sh4_exp |0x0e0,
		sh4_ex_DATA_ADDRESS_ERROR_WRITE = sh4_exp | 0x100,
		sh4_ex_DATA_TLB_MISS_READ = sh4_exp | 0x040,
		sh4_ex_DATA_TLB_MISS_WRITE = sh4_exp | 0x060,
		sh4_ex_DATA_TLB_PROTECTION_VIOLATION_READ = sh4_exp | 0x0a0,
		sh4_ex_DATA_TLB_PROTECTION_VIOLATION_WRITE = sh4_exp | 0x0c0,
		sh4_ex_FPU = sh4_exp | 0x120,
		sh4_ex_TRAP = sh4_exp | 0x160,
		sh4_ex_INITAL_PAGE_WRITE = sh4_exp | 0x080,
		
		// asic9a /sh4 external holly normal [internal]
		holly_RENDER_DONE_vd = holly_nrm | 0x00,
		holly_RENDER_DONE_isp = holly_nrm | 0x01,
		holly_RENDER_DONE = holly_nrm | 0x02,
		holly_SCANINT1 = holly_nrm | 0x03,
		holly_SCANINT2 = holly_nrm | 0x04,
		holly_VBLank = holly_nrm | 0x05,
		holly_OPAQUE = holly_nrm | 0x07,
		holly_OPAQUEMOD = holly_nrm | 0x08,
		holly_TRANS = holly_nrm | 0x09,
		holly_TRANSMOD = holly_nrm | 0x0a,
		holly_MAPLE_DMA = holly_nrm | 0x0c,
		holly_MAPLE_ERR = holly_nrm | 0x0d,
		holly_GDROM_DMA = holly_nrm | 0x0e,
		holly_SPU_DMA = holly_nrm | 0x0f,
		holly_PVR_DMA = holly_nrm | 0x13,
		holly_PUNCHTHRU = holly_nrm | 0x15,

		// asic9c/sh4 external holly external [EXTERNAL]
		holly_GDROM_CMD = holly_ext | 0x00,
		holly_SPU_IRQ = holly_ext | 0x01,
		holly_EXP_8BIT = holly_ext | 0x02,
		holly_EXP_PCI = holly_ext | 0x03,

		// asic9b/sh4 external holly err only error [error]
		holly_PRIM_NOMEM = holly_err | 0x02,
		holly_MATR_NOMEM = holly_err | 0x03
};





typedef void RaiseInterruptFP(InterruptID intr);


struct VersionNumber
{
	union
	{
		struct
		{
			u8 major:8;
			u8 minnor:8;
			u16 build:16;
		};
		u32 full;
	};
};




//NDC_ so it's not confused w/ win32 one
#define NDC_SetVersion(to,major,minor,build) {to.major=major;to.minor=minor;to.build=build;}
#define NDC_MakeVersion(major,minor,build) ((build<<16)|(minor<<8)|(major))

enum PluginType
{
	PowerVR=1,
	GDRom=2,
	Aica=4,
	MapleDeviceMain=8,	//controler ,mouse ect
	MapleDeviceSub=16	//vms ect
};

#define PLUGIN_I_F_VERSION NDC_MakeVersion(0,0,1)

//common functions
//called when plugin is used by emu (you should do first time init here)
//param tpye deepends on plugin type
//PluginType is for dlls that have 2 plugins and need to detect witch one is inited/term/resete'd
typedef void dcInitFP(void* param,PluginType type);

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
typedef void dcTermFP(PluginType type);

//It's suposed to reset everything managed by plugin
typedef void dcResetFP(bool Manual,PluginType type);

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
typedef void dcThreadInitFP(PluginType type);

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
typedef void dcThreadTermFP(PluginType type);

struct plugin_info
{
	VersionNumber	InterfaceVersion;	//interface version , current 0.0.1
	char			Name[128];			//plugin name
	VersionNumber	PluginVersion;		//plugin version
	PluginType		Type;				//plugin type
	
	//Functions that are used for all plugins
	dcInitFP*		Init;					//Init
	dcTermFP*		Term;					//Term
	dcResetFP*		Reset;					//Reset
	dcThreadInitFP*	ThreadInit;				//Thread init (called from cpu thread)
	dcThreadTermFP*	ThreadTerm;				//Thread term (called from cpu thread)
};


//*******************Common Exports
//Give to the emu info for the plugin type
typedef void dcGetPluginInfoFP(plugin_info* info);


#define PVR_CODE
#define GDROM_CODE

////////////////////////////////////////////////////////////////////////////////////////////////////////////

//*********************** PowerVR ***********************
struct vram_block
{
	u32 start;
	u32 end;
	u32 len;
	u32 type;
 
	void* userdata;
};

typedef void vramLockCBFP (vram_block* block,u32 addr);

#ifdef PVR_CODE
//Version : beta 1

//TODO : Design and implement this
#define PVR_PLUGIN_I_F_VERSION NDC_MakeVersion(0,2,0)

typedef void vramlock_Unlock_blockFP  (vram_block* block);
typedef vram_block* vramlock_Lock_32FP(u32 start_offset32,u32 end_offset32,void* userdata);
typedef vram_block* vramlock_Lock_64FP(u32 start_offset64,u32 end_offset64,void* userdata);

struct pvr_init_params
{
	//vram/regs arrays
	u8*					vram;
	RaiseInterruptFP*	RaiseInterrupt;
	void*				WindowHandle;
	vramlock_Lock_32FP* vram_lock_32;
	vramlock_Lock_64FP* vram_lock_64;
	vramlock_Unlock_blockFP* vram_unlock;
};

typedef u32 ReadRegisterPvrFP(u32 addr,u32 size);
typedef void WriteRegisterPvrFP(u32 addr,u32 data,u32 size);

//called from sh4 context , should update pvr/ta state and evereything else
typedef void dcUpdatePvrFP(u32 cycles);

//called from sh4 context , in case of dma or SQ to TA memory , size is 32 byte transfer counts
typedef void dcTaFiFoFP(u32 address,u32* data,u32 size);

//duh stupid C need to have defined types above .. #$%^#$@!#%(&())(
struct pvr_plugin_if
{
	VersionNumber		InterfaceVersion;	//interface version

	dcUpdatePvrFP*		UpdatePvr;
	dcTaFiFoFP*			dcTaFiFo;
	ReadRegisterPvrFP*	ReadReg;
	WriteRegisterPvrFP* WriteReg;
	vramLockCBFP*		LockedBlockWrite;
};

//Give to the emu pointers for the PowerVR interface
typedef void dcGetPvrInfoFP(pvr_plugin_if* info);
#endif


//************************ GDRom ************************
#ifdef GDROM_CODE

enum DiskType
{
	CdDA=0x0,
	CdRom=0x1,
	CdRom_XA=0x2,
	CdRom_Extra=0x3,
	CdRom_CDI=0x4,
	GdRom=0x8,		// This is correct .. dont know about the rest ..
	NoDisk=0x9
};

enum DiskArea
{
	SingleDensity,
	DoubleDensity
};

enum DriveEvent
{
	DiskChange=1	//disk ejected/changed
};

//sends an event to the gd rom lle emulation code (from the gd rom input code)
typedef void DriveNotifyEventFP(DriveEvent info,void* param);
//reads a sector xD
typedef void DriveReadSectorFP(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
//Gets toc for the specified area
typedef void DriveGetTocInfoFP(u32* toc,DiskArea area);
//Gets disk type
typedef DiskType DriveGetDiskTypeFP();
//Get Session info for session "session" , put it to "pout" buffer (6 bytes)
typedef void DriveGetSessionInfoFP(u8* pout,u8 session);


#define GDR_PLUGIN_I_F_VERSION NDC_MakeVersion(0,2,0)

//duh stupid C need to have defined types above .. #$%^#$@!#%(&())(
struct gdr_plugin_if
{
	VersionNumber	InterfaceVersion;	//interface version , curr 0.0.1

	//IO
	DriveReadSectorFP* ReadSector;
	DriveGetTocInfoFP* GetToc;
	DriveGetDiskTypeFP* GetDiskType;
	DriveGetSessionInfoFP* GetSessionInfo;
};

//passed on GDRom init call
struct gdr_init_params
{
	DriveNotifyEventFP* DriveNotifyEvent;
};

//Give to the emu pointers for the gd rom interface
typedef void dcGetGDRInfoFP(gdr_plugin_if* info);
#endif

//For Aica
//TODO : Design and implement this

//For MapleDeviceMain
//TODO : Design and implement this

//For MapleDeviceSub
//TODO : Design and implement this

#undef PVR_CODE
#undef GDROM_CODE

#define EXPORT extern "C" __declspec(dllexport)


