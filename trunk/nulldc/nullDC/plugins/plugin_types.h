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
#define DC_MakeVersion(major,minor,build,flags) (((flags)<<24)|((build)<<16)|((minor)<<8)|(major))
#define DC_VER_NORMAL 0
#define DC_VER_BETA 1
#define DC_VER_PRIVATE 2
#define DC_VER_PRIVBETA (DC_VER_BETA|DC_VER_PRIVATE)

enum PluginType
{
	Plugin_PowerVR=1,			//3D ;)
	Plugin_GDRom=2,			//guess it
	Plugin_AICA=3,				//Sound :p
	Plugin_Maple=4,			//controler ,mouse , ect
	Plugin_MapleSub=5,			//vmu , ect
	Plugin_ExtDevice=6			//BBA , Lan adapter , other 
};

enum ndc_error_codes
{
	rv_ok = 0,		//no error

	rv_error=-2,	//error
	rv_serror=-1,	//silent error , it has been reported to the user
};

#define PLUGIN_I_F_VERSION DC_MakeVersion(1,0,0,DC_VER_BETA)

//These are provided by the emu
typedef void EXPORT_CALL ConfigLoadStrFP(const char * lpSection, const char * lpKey, char * lpReturn,const char* lpDefault);
typedef void EXPORT_CALL ConfigSaveStrFP(const char * lpSection, const char * lpKey, const char * lpString);

typedef s32 EXPORT_CALL ConfigLoadIntFP(const char * lpSection, const char * lpKey,const s32 Default);
typedef void EXPORT_CALL ConfigSaveIntFP(const char * lpSection, const char * lpKey, const s32 Value);
typedef s32 EXPORT_CALL ConfigExistsFP(const char * lpSection, const char * lpKey);

enum MenuItemStyles
{
	MIS_Seperator	=1,
	MIS_Radiocheck	=2,
	MIS_Bitmap		=4,

	MIS_Grayed		=0x40000000,
	MIS_Checked		=0x80000000,
};


#ifndef _MenuItemSelectedFP_
#define _MenuItemSelectedFP_
typedef void EXPORT_CALL MenuItemSelectedFP(u32 id,void* WindowHandle,void* user);


enum MenuItemMask
{
	MIM_Text=1,
	MIM_Handler=2,
	MIM_Bitmap=4,
	MIM_Style=8,
	MIM_PUser=16,
	MIM_All=0xFFFFFFFF,
};
struct MenuItem
{
	char* Text;			//Text of the menu item
	MenuItemSelectedFP* Handler;	//called when the menu is clicked
	void* Bitmap;		//bitmap handle
	u32 Style;			//MIS_* combination
	void* PUser;		//User defined pointer :)
};
#endif

typedef u32 EXPORT_CALL AddMenuItemFP(u32 parent,s32 pos,char* text,MenuItemSelectedFP* handler , u32 checked);
typedef void EXPORT_CALL SetMenuItemStyleFP(u32 id,u32 style,u32 mask);
typedef void EXPORT_CALL GetMenuItemFP(u32 id,MenuItem* info,u32 mask);
typedef void EXPORT_CALL SetMenuItemFP(u32 id,MenuItem* info,u32 mask);
typedef void EXPORT_CALL DeleteMenuItemFP(u32 id);
typedef void* EXPORT_CALL GetRenderTargetFP();

struct emu_info
{
	GetRenderTargetFP*	GetRenderTarget;		//Handle of the window that rendering is done

	ConfigLoadStrFP*	ConfigLoadStr;	//Can be used to Read/Write settings :)
	ConfigSaveStrFP*	ConfigSaveStr;
	ConfigLoadIntFP*	ConfigLoadInt;
	ConfigSaveIntFP*	ConfigSaveInt;
	ConfigExistsFP*		ConfigExists;

	AddMenuItemFP*		AddMenuItem;
	SetMenuItemStyleFP*	SetMenuItemStyle;
	SetMenuItemFP*		SetMenuItem;
	GetMenuItemFP*		GetMenuItem;
	DeleteMenuItemFP*	DeleteMenuItem;

	u32 RootMenu;
	u32 DebugMenu;
};

//common plugin functions
//called when plugin is used by emu (you should do first time init here)
typedef s32 FASTCALL PluginInitFP(emu_info* param);

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
typedef void FASTCALL PluginTermFP();


//Unhandled Write Exeption handler
typedef bool FASTCALL ExeptionHanlderFP(void* addr);

//Some other commonly used function typesdef's ;)
typedef u32 FASTCALL ReadMemFP(u32 addr,u32 size);
typedef void FASTCALL WriteMemFP(u32 addr,u32 data,u32 size);
typedef void FASTCALL UpdateFP(u32 cycles);
typedef void FASTCALL PluginResetFP(bool Manual);

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

#define PVR_PLUGIN_I_F_VERSION DC_MakeVersion(1,0,0,DC_VER_BETA)
 
typedef void FASTCALL vramlock_Unlock_blockFP  (vram_block* block);
typedef vram_block* FASTCALL vramlock_LockFP(u32 start_offset,u32 end_offset,void* userdata);

struct pvr_init_params
{
	RaiseInterruptFP*	RaiseInterrupt;

	//Vram is allocated by the emu.A pointer is given to the buffer here :)
	u8*					vram; 

	//This will work only when using the default exeption handler
	vramlock_LockFP* vram_lock_32;
	vramlock_LockFP* vram_lock_64;
	vramlock_Unlock_blockFP* vram_unlock;
};

typedef s32 FASTCALL PvrInitFP(pvr_init_params* param);
typedef void FASTCALL TaDMAFP(u32* data,u32 size);
typedef void FASTCALL TaSQFP(u32* data);

struct pvr_plugin_if
{
	PvrInitFP*		Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	ExeptionHanlderFP* ExeptionHanlder; //Called on unhandled write exeption, set to 0 if not used.If not 0 , 
										//it disables the internal locking system

	UpdateFP*		UpdatePvr;			//called every ~ 1800 cycles , set to 0 if not used
	TaDMAFP*		TaDMA;				//size is 32 byte transfer counts
	TaSQFP*			TaSQ;				//size is 32 byte transfer counts
	ReadMemFP*		ReadReg;
	WriteMemFP*		WriteReg;

	//Will be called only when pvr locking is enabled
	vramLockCBFP*	LockedBlockWrite;	//set to 0 if not used
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
	GdRom=0x80,		
	NoDisk=0x1,
	Open=0x2,			//tray is open :)
	Busy=0x3			//busy -> needs to be autmaticaly done by gdhost
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

#define GDR_PLUGIN_I_F_VERSION DC_MakeVersion(1,0,0,DC_VER_BETA)

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

#define AICA_PLUGIN_I_F_VERSION DC_MakeVersion(1,0,0,DC_VER_BETA)

//passed on AICA init call
struct aica_init_params
{
	RaiseInterruptFP*	RaiseInterrupt;
	CDDA_SectorFP*	CDDA_Sector;		//For CDDA , returns a silent sector or cdda :)

	u8*				aica_ram;
	u32*			SB_ISTEXT;			//SB_ISTEXT register , so that aica can cancel interrupts =)
};

typedef s32 FASTCALL AicaInitFP(aica_init_params* param);

//Ram/Regs are managed by plugin , exept RTC regs (managed by main emu)
struct aica_plugin_if
{
	AicaInitFP*		Init;
	PluginResetFP*	Reset;
	PluginTermFP*	Term;

	ExeptionHanlderFP* ExeptionHanlder; //Called on unhandled write exeption, set to 0 if not used

	ReadMemFP*  ReadMem_aica_reg;
	WriteMemFP* WriteMem_aica_reg;

	ReadMemFP*  ReadMem_aica_ram;
	WriteMemFP* WriteMem_aica_ram;
	UpdateFP*	UpdateAICA;				//called every ~1800 cycles, set to 0 if not used
};
//******************************************************
//****************** Maple devices ******************
//******************************************************

#define MAPLE_PLUGIN_I_F_VERSION DC_MakeVersion(1,0,0,DC_VER_BETA)

enum MapleDeviceCreationFlags
{
	MDCF_None=0,
	MDCF_Hotplug=1
};

struct maple_subdevice_instance;
struct maple_device_instance;

//buffer_out_len and responce need to be filled w/ proper info by the plugin
//buffer_in must not be edited (its direct pointer on ram)
//output buffer must contain the frame data , the frame header is generated by the maple rooting code
typedef void FASTCALL MapleSubDeviceDMAFP(maple_subdevice_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce);
typedef void FASTCALL MapleDeviceDMAFP(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce);

struct maple_subdevice_instance
{
	//port
	u8 port;
	//user data
	void* data;
	//MapleDeviceDMA
	MapleSubDeviceDMAFP* dma;
	bool connected;
	u32 reserved;	//reserved for the emu , DO NOT EDIT
};
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

struct maple_init_params
{
	RaiseInterruptFP*	RaiseInterrupt;
};

typedef s32 FASTCALL MapleSubCreateInstanceFP(maple_subdevice_instance* inst,u32 id,u32 flags,u32 rootmenu);
typedef s32 FASTCALL MapleSubInitInstanceFP(maple_subdevice_instance* inst,u32 id,maple_init_params* params);
typedef void FASTCALL MapleSubTermInstanceFP(maple_subdevice_instance* inst,u32 id);
typedef void FASTCALL MapleSubDestroyInstanceFP(maple_subdevice_instance* inst,u32 id);

typedef s32 FASTCALL MapleCreateInstanceFP(maple_device_instance* inst,u32 id,u32 flags,u32 rootmenu);
typedef s32 FASTCALL MapleInitInstanceFP(maple_device_instance* inst,u32 id,maple_init_params* params);
typedef void FASTCALL MapleTermInstanceFP(maple_device_instance* inst,u32 id);
typedef void FASTCALL MapleDestroyInstanceFP(maple_device_instance* inst,u32 id);

typedef s32 FASTCALL MapleInitFP(maple_init_params* param);

enum MapleDeviceType
{
	MDT_EndOfList=0,
	MDT_Main=1,
	MDT_Sub=2,
};
enum MapleDeviceTypeFlags
{
	MDTF_Sub0=1,		//these
	MDTF_Sub1=2,		//are
	MDTF_Sub2=4,		//ingored
	MDTF_Sub3=8,		//in
	MDTF_Sub4=16,		//subdevices :)

	MDTF_Hotplug=32,		//Can be added/removed at runtime
};
struct maple_device_definition
{
	char Name[128];
	u32 Type;
	u32 Flags;
};
struct maple_plugin_if
{
	//*Main functions are ingored if no main devices are exported
	//*Sub functions are ingored if no main devices are exported
	//Create Instance
	MapleCreateInstanceFP* CreateMain;
	MapleSubCreateInstanceFP* CreateSub;
	//Destroy Instance
	MapleDestroyInstanceFP* DestroyMain;
	MapleSubDestroyInstanceFP* DestroySub;
	//Init
	MapleInitInstanceFP*	InitMain;
	MapleSubInitInstanceFP*	InitSub;
	//Term
	MapleTermInstanceFP*	TermMain;
	MapleSubTermInstanceFP*	TermSub;
	
	maple_device_definition devices[16];	//Last one must be of type MDT_EndOfList , uless all 16 are used
};

//******************************************************
//********************* Ext.Device *********************
//******************************************************

#define EXTDEVICE_PLUGIN_I_F_VERSION DC_MakeVersion(1,0,0,DC_VER_BETA)

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

	ExeptionHanlderFP* ExeptionHanlder; //Called on unhandled write exeption, set to 0 if not used

	//Area 0 , 0x00600000- 0x006007FF	[MODEM]
	ReadMemFP*  ReadMem_A0_006;
	WriteMemFP* WriteMem_A0_006;

	//Area 0 , 0x01000000- 0x01FFFFFF	[Ext. Device]
	ReadMemFP*  ReadMem_A0_010;
	WriteMemFP* WriteMem_A0_010;
	
	//Area 5
	ReadMemFP*  ReadMem_A5;
	WriteMemFP* WriteMem_A5;

	UpdateFP*	UpdateExtDevice;//Called every ~1800 cycles, set to 0 if not used
};

//Plugin Exports
//These are the functions the plugin has to export :)

struct plugin_interface
{
	u32 InterfaceVersion;

	common_info common;
	union 
	{
		pvr_plugin_if			pvr;
		gdr_plugin_if			gdr;
		aica_plugin_if			aica;
		maple_plugin_if			maple;
		ext_device_plugin_if	ext_dev;

		u32 pad[4096];//padding & reserved space for future expantion :)
	};
};

//Dropped 
//exported as dcGetInterfaceInfo
//typedef void EXPORT_CALL dcGetInterfaceInfoFP(plugin_interface_info* info);

//exported as dcGetInterface
typedef void EXPORT_CALL dcGetInterfaceFP(plugin_interface* lst);