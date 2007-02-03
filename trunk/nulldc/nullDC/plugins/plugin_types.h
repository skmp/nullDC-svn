#pragma once
#ifndef _PLUGIN_HEADER_
#include "types.h"

#include "dc/sh4/sh4_if.h"
#include "dc/pvr/pvrLock.h"
#endif

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

#define EXPORT extern "C" __declspec(dllexport)

#define EXPORT_CALL __stdcall
#define FASTCALL __fastcall
#define CDECL __cdecl

//NDC_ so it's not confused w/ win32 one
#define NDC_MakeVersion(major,minor,build) ((build<<16)|(minor<<8)|(major))
enum PluginType
{
	PowerVR=1,			//3D ;)
	GDRom=2,			//guess it
	AICA=3,				//Sound :p
	Maple=4,			//controler ,mouse , ect
	MapleSub=5,			//vmu , ect
	ExtDevice=6			//BBA , Lan adapter , other 
};

enum ndc_error_codes
{
	rv_ok = 0,		//no error

	rv_error=-2,	//error
	rv_serror=-1,	//silent error , it has been reported to the user
};

#define PLUGIN_I_F_VERSION NDC_MakeVersion(1,0,0)

//These are provided by the emu
typedef void FASTCALL ConfigLoadStrFP(const char * lpSection, const char * lpKey, char * lpReturn);
typedef void FASTCALL ConfigSaveStrFP(const char * lpSection, const char * lpKey, const char * lpString);

struct emu_info
{
	//Some info about the emu (can be used form pvr plugins to set caption ect ..)
	char Name[128];

	void*			WindowHandle;		//Handle of the window that rendering is done

	ConfigLoadStrFP*	ConfigLoadStr;	//Can be used to Read/Write ndc's .cfg file ;)
	ConfigSaveStrFP*	ConfigSaveStr;
};

//common plugin functions
//called when plugin is used by emu (you should do first time init here)
typedef s32 FASTCALL PluginInitFP(emu_info* param);

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
typedef void FASTCALL PluginTermFP();

//Window is the parent (ie , the select plugins window)
typedef void FASTCALL ShowConfigFP(void* window);

//Unhandled Write Exeption handler
typedef bool FASTCALL ExeptionHanlderFP(void* addr);

//Some other commonly used function typesdef's ;)
typedef u32 FASTCALL ReadMemFP(u32 addr,u32 size);
typedef void FASTCALL WriteMemFP(u32 addr,u32 data,u32 size);
typedef void FASTCALL UpdateFP(u32 cycles);
typedef void FASTCALL PluginResetFP(bool Manual);

/*
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
	dcUnhandledWriteExeption* UnhandledWriteExeption;//Called on unhandled write exeption ;)
};
*/
struct plugin_info
{
	u32 InterfaceVersion;
	u32 count;
};
struct common_info
{
	char			Name[128];			//plugin name
	u32				PluginVersion;		//plugin version
	u32				Type;				//plugin type
	u32				InterfaceVersion;	//Note : this version is of the interface for this type of plugin :)

	//Functions that are used for all plugins , these are SET by the plugin
	PluginInitFP*	Load;					//Init
	PluginTermFP*	Unload;					//Term
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

//******************************************************
//*********************** PowerVR **********************
//******************************************************

#define PVR_PLUGIN_I_F_VERSION NDC_MakeVersion(1,0,0)
 
typedef void FASTCALL vramlock_Unlock_blockFP  (vram_block* block);
typedef vram_block* FASTCALL vramlock_Lock_32FP(u32 start_offset32,u32 end_offset32,void* userdata);
typedef vram_block* FASTCALL vramlock_Lock_64FP(u32 start_offset64,u32 end_offset64,void* userdata);

struct pvr_init_params
{
	RaiseInterruptFP*	RaiseInterrupt;

	//Vram is allocated by the emu.A pointer is given to the buffer here :)
	u8*					vram; 

	//This will work only when using the default exeption handler
	vramlock_Lock_32FP* vram_lock_32;
	vramlock_Lock_64FP* vram_lock_64;
	vramlock_Unlock_blockFP* vram_unlock;
};

typedef s32 FASTCALL PvrInitFP(pvr_init_params* param);
typedef void FASTCALL TaFIFOFP(u32 address,u32* data,u32 size);

struct pvr_plugin_if
{
	PvrInitFP*		Init;		//called from sh4 context
	PluginResetFP*	Reset;		//called from sh4 context
	PluginTermFP*	Term;		//called from sh4 context

	ShowConfigFP* ShowConfig;			//Show config ;)
	ExeptionHanlderFP* ExeptionHanlder; //Called on unhandled write exeption ;)

	UpdateFP*		UpdatePvr;		//called from sh4 context , should update pvr/ta state and evereything else
	TaFIFOFP*		TaFIFO;			//called from sh4 context , in case of dma or SQ to TA memory , size is 32 byte transfer counts
	ReadMemFP*		ReadReg;
	WriteMemFP*		WriteReg;

	//Will be called only when pvr locking is enabled
	vramLockCBFP*	LockedBlockWrite;
};
//******************************************************
//************************ GDRom ***********************
//******************************************************
enum DiscType
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
typedef void FASTCALL DriveNotifyEventFP(u32 event,void* param);
//reads a sector xD
typedef void FASTCALL DriveReadSectorFP(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
//Gets toc for the specified area
typedef void FASTCALL DriveGetTocInfoFP(u32* toc,u32 area);
//Gets disk type
typedef u32 FASTCALL DriveGetDiscTypeFP();
//Get Session info for session "session" , put it to "pout" buffer (6 bytes)
typedef void FASTCALL DriveGetSessionInfoFP(u8* pout,u8 session);
//Get subchannel data
typedef void FASTCALL DriveReadSubChannelFP(u8 * buff, u32 format, u32 len);

#define GDR_PLUGIN_I_F_VERSION NDC_MakeVersion(1,0,0)

//passed on GDRom init call
struct gdr_init_params
{
	RaiseInterruptFP*	RaiseInterrupt;
	DriveNotifyEventFP* DriveNotifyEvent;
};

typedef s32 FASTCALL GdrInitFP(gdr_init_params* param);

struct gdr_plugin_if
{
	GdrInitFP*		Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	ShowConfigFP* ShowConfig;			//Show config ;)
	ExeptionHanlderFP* ExeptionHanlder; //Called on unhandled write exeption ;)

	//IO
	DriveReadSectorFP* ReadSector;
	DriveReadSubChannelFP *ReadSubChannel;
	DriveGetTocInfoFP* GetToc;
	DriveGetDiscTypeFP* GetDiscType;
	DriveGetSessionInfoFP* GetSessionInfo;
};
//******************************************************
//************************ AICA ************************
//******************************************************

typedef void FASTCALL CDDA_SectorFP(s16* sector);

#define AICA_PLUGIN_I_F_VERSION NDC_MakeVersion(1,0,0)

//passed on AICA init call
struct aica_init_params
{
	RaiseInterruptFP*	RaiseInterrupt;
	u8*				aica_ram;

	u32*			SB_ISTEXT;			//SB_ISTEXT register , so that aica can cancel interrupts =)
	CDDA_SectorFP*	CDDA_Sector;		//For CDDA , returns a silent sector or cdda :)
};

typedef s32 FASTCALL AicaInitFP(aica_init_params* param);

//Ram/Regs are managed by plugin , exept RTC regs (managed by main emu)
struct aica_plugin_if
{
	AicaInitFP*		Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	ShowConfigFP* ShowConfig;			//Show config ;)
	ExeptionHanlderFP* ExeptionHanlder; //Called on unhandled write exeption ;)

	ReadMemFP*  ReadMem_aica_reg;
	WriteMemFP* WriteMem_aica_reg;

	ReadMemFP*  ReadMem_aica_ram;
	WriteMemFP* WriteMem_aica_ram;
	UpdateFP*	UpdateAICA;
};
//******************************************************
//****************** Maple Subdevices ******************
//******************************************************

//buffer_out_len and responce need to be filled w/ proper info by the plugin
//buffer_in must not be edited (its direct pointer on ram)
//output buffer must contain the frame data , the frame header is generated by the maple rooting code
struct maple_subdevice_instance;
typedef void FASTCALL MapleSubDeviceDMAFP(maple_subdevice_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce);

//Version is shared betwen maple and maple sub :)

struct maple_subdevice_instance
{
	//port
	u8 port;
	//user data
	void* data;
	//MapleDeviceDMA
	MapleSubDeviceDMAFP* dma;
	bool connected;
};
typedef s32 FASTCALL MapleSubCreateInstanceFP(maple_subdevice_instance* inst,u8 port);
typedef void FASTCALL MapleSubDestroyInstanceFP(maple_subdevice_instance* inst);

//MapleDeviceDMAFP* MapleDeviceDMA;	//pointer to dma handling function

struct maple_sub_init_params
{
	RaiseInterruptFP*	RaiseInterrupt;
};
typedef s32 FASTCALL MapleSubInitFP(maple_sub_init_params* param);
typedef void FASTCALL ShowMapleConfigFP(void* window,u32 port,u32 subport); //subport : 0 is main , 1,2,3,4,5 are valid :)
struct maple_sub_plugin_if
{
	MapleSubInitFP*	Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	ShowMapleConfigFP* ShowConfig;			//Show config ;)

	//Create Instance
	MapleSubCreateInstanceFP* Create;
	//Destroy Instance
	MapleSubDestroyInstanceFP* Destroy;
};
//******************************************************
//******************* Maple Devices ********************
//******************************************************

//buffer_out_len and responce need to be filled w/ proper info by the plugin
//buffer_in must not be edited (its direct pointer on ram)
//output buffer must contain the frame data , the frame header is generated by the maple rooting code
struct maple_device_instance;
typedef void FASTCALL MapleDeviceDMAFP(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce);

//Version is shared betwen maple and maple sub :)
#define MAPLE_PLUGIN_I_F_VERSION NDC_MakeVersion(1,0,0)

struct maple_device_instance
{
	//port
	u8 port;
	//user data
	void* data;
	//MapleDeviceDMA
	MapleDeviceDMAFP* dma;
	bool connected;

	maple_subdevice_instance subdevices[5];
};
typedef s32 FASTCALL MapleCreateInstanceFP(maple_device_instance* inst,u8 port);
typedef void FASTCALL MapleDestroyInstanceFP(maple_device_instance* inst);

//MapleDeviceDMAFP* MapleDeviceDMA;	//pointer to dma handling function

struct maple_init_params
{
	RaiseInterruptFP*	RaiseInterrupt;
};

typedef s32 FASTCALL MapleInitFP(maple_init_params* param);

struct maple_plugin_if
{
	MapleInitFP*	Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	ShowMapleConfigFP* ShowConfig;			//Show config ;)

	//Create Instance
	MapleCreateInstanceFP* Create;
	//Destroy Instance
	MapleDestroyInstanceFP* Destroy;

	//Info about the devices can be attached to it
	//A bit set to 0 means a device can connect to the port
	//Bits 0 to 4 are valid :)
	//MAPLE_SUBDEVICE_DISABLE* consts :)
	u32 subdev_info;
};
#define MAPLE_SUBDEVICE_DISABLE_0 1
#define MAPLE_SUBDEVICE_DISABLE_1 2
#define MAPLE_SUBDEVICE_DISABLE_2 4
#define MAPLE_SUBDEVICE_DISABLE_3 8
#define MAPLE_SUBDEVICE_DISABLE_4 16
//******************************************************
//********************* Ext.Device *********************
//******************************************************

#define EXTDEVICE_PLUGIN_I_F_VERSION NDC_MakeVersion(1,0,0)

//passed on Ext.Device init call
struct ext_device_init_params
{
	RaiseInterruptFP*	RaiseInterrupt;
	u32* SB_ISTEXT;
};

typedef s32 FASTCALL ExtInitFP(ext_device_init_params* param);

struct ext_device_plugin_if
{
	ExtInitFP*		Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	ShowConfigFP* ShowConfig;			//Show config ;)
	ExeptionHanlderFP* ExeptionHanlder; //Called on unhandled write exeption ;)

	//Area 0 , 0x00600000- 0x006007FF	[MODEM]
	ReadMemFP*  ReadMem_A0_006;
	WriteMemFP* WriteMem_A0_006;

	//Area 0 , 0x01000000- 0x01FFFFFF	[Ext. Device]
	ReadMemFP*  ReadMem_A0_010;
	WriteMemFP* WriteMem_A0_010;
	
	//Area 5
	ReadMemFP*  ReadMem_A5;
	WriteMemFP* WriteMem_A5;

	UpdateFP*	UpdateExtDevice;
};

//Plugin Exports
//These are the functions the plugin has to export :)

struct plugin_info_entry
{
	common_info common;
	union 
	{
		pvr_plugin_if			pvr;
		gdr_plugin_if			gdr;
		aica_plugin_if			aica;
		maple_plugin_if			maple;
		maple_sub_plugin_if		maple_sub;
		ext_device_plugin_if	ext_dev;

		u32 pad[128];//padding & reserved space for future expantion :)
	};
};

//exported as dcGetPluginInfo
typedef void EXPORT_CALL dcGetPluginInfoFP(plugin_info* info);
//exported as dcGetPlugin
typedef bool EXPORT_CALL dcGetPluginFP(u32 id,plugin_info_entry* lst);