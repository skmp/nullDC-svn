#pragma once
#include "..\types.h"

#include "..\dc\sh4\sh4_if.h"
#include "..\dc\pvr\pvrLock.h"

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

	INLINE void operator= ( const u32 y);
};



bool operator ==(VersionNumber& x,  const u32 y);

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

//
typedef u32 ReadMemFP(u32 addr,u32 size);
typedef void WriteMemFP(u32 addr,u32 data,u32 size);

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
	dcTaFiFoFP*			TADma;
	ReadRegisterPvrFP*	ReadReg;
	WriteRegisterPvrFP* WriteReg;
	vramLockCBFP*		LockedBlockWrite;
};

//Give to the emu pointers for the PowerVR interface
typedef void dcGetPvrInfoFP(pvr_plugin_if* info);
#endif


//************************ GDRom ************************
#ifdef GDROM_CODE
//this MUST be big endian before sending it :)

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


//TODO : Design and implement this
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

#define AICA_PLUGIN_I_F_VERSION NDC_MakeVersion(0,1,0)

struct aica_plugin_if
{
	VersionNumber	InterfaceVersion;	//interface version , curr 0.0.1
};

//passed on AICA init call
struct aica_init_params
{
	ReadMemFP* ReadMem_aica;
	WriteMemFP* aica_regs;
};

//Give to the emu pointers for the aica interface
typedef void dcGetAICAInfoFP(aica_plugin_if* info);


//For MapleDeviceMain
//TODO : Design and implement this

//For MapleDeviceSub
//TODO : Design and implement this

#undef PVR_CODE
#undef GDROM_CODE

#define EXPORT extern "C" __declspec(dllexport)