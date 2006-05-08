#pragma once
#include "types.h"

#include "dc/sh4/sh4_if.h"
#include "dc/pvr/pvrLock.h"

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
	AICA=4,
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

//simple ehh ?
typedef void dcShowConfigFP(PluginType type,void* window);

//
typedef u32 ReadMemFP(u32 addr,u32 size);
typedef void WriteMemFP(u32 addr,u32 data,u32 size);
typedef void UpdateFP(u32 cycles);

//These are provided by the emu
typedef void ConfigLoadStrFP(const char * lpSection, const char * lpKey, char * lpReturn);
typedef void ConfigSaveStrFP(const char * lpSection, const char * lpKey, const char * lpString);

struct plugin_info
{
	VersionNumber	InterfaceVersion;	//interface version , current 0.0.1
	char			Name[128];			//plugin name
	VersionNumber	PluginVersion;		//plugin version
	PluginType		Type;				//plugin type
	
	//Functions that are used for all plugins , these are SET by the plugin
	dcInitFP*		Init;					//Init
	dcTermFP*		Term;					//Term
	dcResetFP*		Reset;					//Reset
	dcThreadInitFP*	ThreadInit;				//Thread init (called from cpu thread)
	dcThreadTermFP*	ThreadTerm;				//Thread term (called from cpu thread)
	dcShowConfigFP* ShowConfig;				//Show config ;)

	//Functions "Exported" from the emu , these are SET by the emu
	ConfigLoadStrFP*   ConfigLoadStr;
	ConfigSaveStrFP*   ConfigSaveStr;
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


typedef void TaFIFOFP(u32 address,u32* data,u32 size);

//duh stupid C need to have defined types above .. #$%^#$@!#%(&())(
struct pvr_plugin_if
{
	VersionNumber		InterfaceVersion;	//interface version

	UpdateFP*		UpdatePvr;		//called from sh4 context , should update pvr/ta state and evereything else
	TaFIFOFP*		TADma;			//called from sh4 context , in case of dma or SQ to TA memory , size is 32 byte transfer counts
	ReadMemFP*		ReadReg;
	WriteMemFP*		WriteReg;
	vramLockCBFP*	LockedBlockWrite;
};

//Give to the emu pointers for the PowerVR interface
typedef void dcGetPvrInfoFP(pvr_plugin_if* info);
#endif


//************************ GDRom ************************
#ifdef GDROM_CODE
//this MUST be big endian before sending it :)

enum DiskType
{
	CdDA=0x00,
	CdRom=0x10,
	CdRom_XA=0x20,
	CdRom_Extra=0x30,
	CdRom_CDI=0x40,
	GdRom=0x80,		// This is correct .. dont know about the rest ..
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

// FTW FTW FTW FTW
typedef void DriveReadSubChannelFP(u8 * buff, u32 format, u32 len);


//TODO : Design and implement this
#define GDR_PLUGIN_I_F_VERSION NDC_MakeVersion(0,2,0)

//duh stupid C need to have defined types above .. #$%^#$@!#%(&())(
struct gdr_plugin_if
{
	VersionNumber	InterfaceVersion;	//interface version , curr 0.0.1

	//IO
	DriveReadSectorFP* ReadSector;
	DriveReadSubChannelFP *ReadSubChannel;
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

//Ram/Regs are managed by plugin , exept RTC regs (managed by main emu)
struct aica_plugin_if
{
	VersionNumber	InterfaceVersion;	//interface version , curr 0.0.1

	ReadMemFP*  ReadMem_aica_reg;
	WriteMemFP* WriteMem_aica_reg;

	ReadMemFP*  ReadMem_aica_ram;
	WriteMemFP* WriteMem_aica_ram;
	UpdateFP*	UpdateAICA;
};

//passed on AICA init call
struct aica_init_params
{
	void* WindowHandle;
	RaiseInterruptFP*	RaiseInterrupt;
	u32* SB_ISTEXT;
};

//Give to the emu pointers for the aica interface
typedef void dcGetAICAInfoFP(aica_plugin_if* info);


//Maple Plugins :)

#define MAPLE_PLUGIN_I_F_VERSION NDC_MakeVersion(0,1,0)

struct maple_plugin_if;
//buffer_out_len and responce need to be filled w/ proper info by the plugin
//buffer_in must not be edited (its direct pointer on ram)
//output buffer must contain the frame data , the frame header is generated by the maple rooting code
typedef void MapleDeviceDMAFP(maple_plugin_if* plugin_instance,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce);

struct maple_plugin_if
{
	VersionNumber	InterfaceVersion;	//interface version , curr 0.1.0

	MapleDeviceDMAFP* MapleDeviceDMA;	//pointer to dma handling function
	void* PluginData;					//This pointer is to be used by the plugin author as he/she wants to ;)
	u8 Address;							//address on maple bus , filled by Maple rooting code
};




//For MapleDeviceMain
//Uses maple_plugin_if , diferent flags tho
typedef void dcGetMapleMainInfoFP(maple_plugin_if* info,u8 Address);

//For MapleDeviceSub
//Uses maple_plugin_if , diferent flags tho
typedef void dcGetMapleSubInfoFP(maple_plugin_if* info,u8 Address);

#undef PVR_CODE
#undef GDROM_CODE

#define EXPORT extern "C" __declspec(dllexport)