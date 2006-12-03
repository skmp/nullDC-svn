#pragma once

//SHUT UP M$ COMPILER !@#!@$#
#ifdef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#endif

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
		holly_RENDER_DONE_vd = holly_nrm | 0,	//bit 0 = End of Render interrupt : Video
		holly_RENDER_DONE_isp = holly_nrm | 1,	//bit 1 = End of Render interrupt : ISP
		holly_RENDER_DONE = holly_nrm | 2,		//bit 2 = End of Render interrupt : TSP

		holly_SCANINT1 = holly_nrm | 3,			//bit 3 = V Blank-in interrupt
		holly_SCANINT2 = holly_nrm | 4,			//bit 4 = V Blank-out interrupt
		holly_HBLank = holly_nrm | 5,			//bit 5 = H Blank-in interrupt
												
		holly_YUV_DMA = holly_nrm | 6,			//bit 6 = End of Transferring interrupt : YUV
		holly_OPAQUE = holly_nrm | 7,			//bit 7 = End of Transferring interrupt : Opaque List
		holly_OPAQUEMOD = holly_nrm | 8,		//bit 8 = End of Transferring interrupt : Opaque Modifier Volume List
		
		holly_TRANS = holly_nrm | 9,			//bit 9 = End of Transferring interrupt : Translucent List
		holly_TRANSMOD = holly_nrm | 10,		//bit 10 = End of Transferring interrupt : Translucent Modifier Volume List
		holly_PVR_DMA = holly_nrm | 11,			//bit 11 = End of DMA interrupt : PVR-DMA
		holly_MAPLE_DMA = holly_nrm | 12,		//bit 12 = End of DMA interrupt : Maple-DMA

		holly_MAPLE_VBOI = holly_nrm | 13,		//bit 13 = Maple V blank over interrupt
		holly_GDROM_DMA = holly_nrm | 14,		//bit 14 = End of DMA interrupt : GD-DMA
		holly_SPU_DMA = holly_nrm | 15,			//bit 15 = End of DMA interrupt : AICA-DMA
		
		holly_EXT_DMA1 = holly_nrm | 16,		//bit 16 = End of DMA interrupt : Ext-DMA1(External 1)
		holly_EXT_DMA2 = holly_nrm | 17,		//bit 17 = End of DMA interrupt : Ext-DMA2(External 2)
		holly_DEV_DMA = holly_nrm | 18,			//bit 18 = End of DMA interrupt : Dev-DMA(Development tool DMA)
		
		holly_CH2_DMA = holly_nrm | 19,			//bit 19 = End of DMA interrupt : ch2-DMA 
		holly_PVR_SortDMA = holly_nrm | 20,		//bit 20 = End of DMA interrupt : Sort-DMA (Transferring for alpha sorting)
		holly_PUNCHTHRU = holly_nrm | 21,		//bit 21 = End of Transferring interrupt : Punch Through List

		// asic9c/sh4 external holly external [EXTERNAL]
		holly_GDROM_CMD = holly_ext | 0x00,	//bit 0 = GD-ROM interrupt
		holly_SPU_IRQ = holly_ext | 0x01,	//bit 1 = AICA interrupt
		holly_EXP_8BIT = holly_ext | 0x02,	//bit 2 = Modem interrupt
		holly_EXP_PCI = holly_ext | 0x03,	//bit 3 = External Device interrupt

		// asic9b/sh4 external holly err only error [error]
		//missing quite a few ehh ?
		//bit 0 = RENDER : ISP out of Cache(Buffer over flow)
		//bit 1 = RENDER : Hazard Processing of Strip Buffer
		holly_PRIM_NOMEM = holly_err | 0x02,	//bit 2 = TA : ISP/TSP Parameter Overflow
		holly_MATR_NOMEM = holly_err | 0x03		//bit 3 = TA : Object List Pointer Overflow
		//bit 4 = TA : Illegal Parameter
		//bit 5 = TA : FIFO Overflow
		//bit 6 = PVRIF : Illegal Address set
		//bit 7 = PVRIF : DMA over run
		//bit 8 = MAPLE : Illegal Address set
		//bit 9 = MAPLE : DMA over run
		//bit 10 = MAPLE : Write FIFO over flow
		//bit 11 = MAPLE : Illegal command
		//bit 12 = G1 : Illegal Address set
		//bit 13 = G1 : GD-DMA over run
		//bit 14 = G1 : ROM/FLASH access at GD-DMA
		//bit 15 = G2 : AICA-DMA Illegal Address set
		//bit 16 = G2 : Ext-DMA1 Illegal Address set
		//bit 17 = G2 : Ext-DMA2 Illegal Address set
		//bit 18 = G2 : Dev-DMA Illegal Address set
		//bit 19 = G2 : AICA-DMA over run
		//bit 20 = G2 : Ext-DMA1 over run
		//bit 21 = G2 : Ext-DMA2 over run
		//bit 22 = G2 : Dev-DMA over run
		//bit 23 = G2 : AICA-DMA Time out
		//bit 24 = G2 : Ext-DMA1 Time out
		//bit 25 = G2 : Ext-DMA2 Time out
		//bit 26 = G2 : Dev-DMA Time out 
		//bit 27 = G2 : Time out in CPU accessing 	
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

struct vram_block
{
	u32 start;
	u32 end;
	u32 len;
	u32 type;
 
	void* userdata;
};

typedef void vramLockCBFP (vram_block* block,u32 addr);

//NDC_ so it's not confused w/ win32 one
#define NDC_SetVersion(to,major,minor,build) {to.major=major;to.minor=minor;to.build=build;}
#define NDC_MakeVersion(major,minor,build) ((build<<16)|(minor<<8)|(major))

enum PluginType
{
	PowerVR=1,
	GDRom=2,
	AICA=4,
	MapleDevice=8,	//controler ,mouse , vmu , ect
	ExtDevice=16	//BBA , Lan adapter , other 
};

#define PLUGIN_I_F_VERSION NDC_MakeVersion(0,0,2)

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

//Unhandled Write Exeption handler
typedef bool dcUnhandledWriteExeption(void* addr);

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
	dcUnhandledWriteExeption* UnhandledWriteExeption;//Called on unhandled write exeption ;)

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
	TaFIFOFP*		TaFIFO;			//called from sh4 context , in case of dma or SQ to TA memory , size is 32 byte transfer counts
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

#define AICA_PLUGIN_I_F_VERSION NDC_MakeVersion(0,2,0)

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

typedef void CDDA_SectorFP(s16* sector);
//passed on AICA init call
struct aica_init_params
{
	void* WindowHandle;
	RaiseInterruptFP*	RaiseInterrupt;
	u32* SB_ISTEXT;
	CDDA_SectorFP*	CDDA_Sector;
};

//Give to the emu pointers for the aica interface
typedef void dcGetAICAInfoFP(aica_plugin_if* info);

//Maple Plugins :)

#define MAPLE_PLUGIN_I_F_VERSION NDC_MakeVersion(0,1,0)

struct maple_device_instance;
//buffer_out_len and responce need to be filled w/ proper info by the plugin
//buffer_in must not be edited (its direct pointer on ram)
//output buffer must contain the frame data , the frame header is generated by the maple rooting code
typedef void MapleDeviceDMAFP(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce);

struct maple_device_instance
{
	//port
	u8 port;
	//user data
	void* DevData;
	//MapleDeviceDMA
	MapleDeviceDMAFP* MapleDeviceDMA;
	bool Connected;
};
struct maple_device;
typedef void MapleCreateInstanceFP(maple_device*dev,maple_device_instance& inst,u8 port);
typedef void MapleDestroyInstanceFP(maple_device*dev,maple_device_instance& inst);
struct maple_device
{
	//Device name
	char name[128];
	//Device type
	u8 type;

	//a duuplicate of the index
	u8 id;

	//Create Instance
	MapleCreateInstanceFP* CreateInstance;
	//Destroy Instance
	MapleDestroyInstanceFP* DestroyInstance;
};
//MapleDeviceDMAFP* MapleDeviceDMA;	//pointer to dma handling function
struct maple_plugin_if
{
	VersionNumber	InterfaceVersion;	//interface version , curr 0.1.0
	maple_device Devices[16];
	//MapleDevice*	Pointer to a list of devices
};

struct maple_init_params
{
	void* WindowHandle;
};



//For MapleDeviceMain
//Uses maple_plugin_if , diferent flags tho
typedef void dcGetMapleInfoFP(maple_plugin_if* info);

//Ext.Device
//TODO : Design and implement this

#define EXTDEVICE_PLUGIN_I_F_VERSION NDC_MakeVersion(1,0,0)

struct ext_device_plugin_if
{
	VersionNumber	InterfaceVersion;	//interface version

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

//passed on Ext.Device init call
struct ext_device_init_params
{
	void* WindowHandle;
	RaiseInterruptFP*	RaiseInterrupt;
	u32* SB_ISTEXT;
};

//Give to the emu pointers for the aica interface
typedef void dcGetExtDeviceInfoFP(ext_device_plugin_if* info);


#undef PVR_CODE
#undef GDROM_CODE

#define EXPORT extern "C" __declspec(dllexport)