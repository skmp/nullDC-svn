#pragma once

#define maple_sett(x,y) NDCS_PLUGIN_MAPLE_##x##_##y
#define maple_groop(x) maple_sett(x,0), maple_sett(x,1), maple_sett(x,2), maple_sett(x,3), maple_sett(x,4), maple_sett(x,5)


enum nullDCSettings
{
	NDCS_DYNAREC_ENABLED,
	NDCS_DYNAREC_CPPASS,
	NDCS_DYNAREC_UCFPU,

	NDCS_DREAMCAST_CABLE,
	NDCS_DREAMCAST_RTC,

	NDCS_EMULATOR_ASTART,

	NDCS_PLUGIN_PVR,
	NDCS_PLUGIN_GDR,
	NDCS_PLUGIN_AICA,
	NDCS_PLUGIN_EXTDEV,


	maple_groop(0),
	maple_groop(1),
	maple_groop(2),
	maple_groop(3),

	NDCS_COUNT,
};

#undef maple_sett
#undef maple_groop

struct PluginInfoList
{
	PluginInfoList* next;
	char Name[128];
	VersionNumber Version;
	char dll[512];
	u32 Flags;
};
typedef PluginInfoList* EXPORT_CALL GetPluginListFP(u32 PluginType);
typedef PluginInfoList* EXPORT_CALL GetMapleDeviceListFP(u32 DeviceType);
typedef s32 EXPORT_CALL FreePluginListFP(PluginInfoList* list);

typedef s32 EXPORT_CALL dbgWriteMemFP(u32 addr,u32 sz,void* dst);

typedef s32 EXPORT_CALL dbgReadMemFP(u32 addr,u32 sz,void* dst);
typedef s32 EXPORT_CALL dbgWriteMemFP(u32 addr,u32 sz,void* dst);
typedef int EXPORT_CALL guiMsgBoxFP(char* text,int type);

typedef bool EXPORT_CALL EmuStartedFP();	//returns if emulation is started.

typedef bool EXPORT_CALL EmuInitFP();

typedef bool EXPORT_CALL EmuStartFP(); //returns false if it failed.It can fail if non inited and implicit init failed.
typedef void EXPORT_CALL EmuStopFP();
typedef bool EXPORT_CALL EmuResetFP(bool Manual);	//well duh it resets =P.
typedef void EXPORT_CALL EmuResetCachesFP();	//resets any emulation caches, like dynarec's translation buffer and lookup cache

typedef bool EXPORT_CALL EmuStepFP(); //returns false if it failed.It can fail if non inited or allready running.
typedef bool EXPORT_CALL EmuSkipFP(); //returns false if it failed.It can fail if non inited or allready running.

typedef void EXPORT_CALL EmuSetPatchFP(u32 Value,u32 Mask);	//Enable/Disable a patch (system area hooks)


typedef bool EXPORT_CALL EmuBootHLEFP();	//Copies the bin file from the disc , descrambles it if needed and sets up needed regs for boot.False if it failed.
typedef bool EXPORT_CALL EmuLoadBinaryFP(char* file,u32 address);	//Loads a binary on the address.If the file is elf the address is ingored and the elf's
														//offsets are used
typedef bool EXPORT_CALL EmuSelectPluginsFP();	//Request the emu to show the select plugins interface
typedef void EXPORT_CALL EmuStartProfilerFP();	//Start the TBP
typedef void EXPORT_CALL EmuStopProfilerFP();	//Stop the TBP

typedef void EXPORT_CALL DissasembleOpcodeFP(u16 opcode,u32 pc,char* Dissasm);
typedef u32 EXPORT_CALL Sh4GetRegisterFP(u32 reg);
typedef void EXPORT_CALL Sh4SetRegisterFP(u32 reg,u32 value);
typedef int EXPORT_CALL GetSymbNameFP(u32 address,char *szDesc,bool bUseUnkAddress);
typedef bool EXPORT_CALL SelectPluginsGuiFP();
typedef s32 EXPORT_CALL EditEmuSettingFP(u32 sid,void* value);
typedef char* EXPORT_CALL GetAboutTextFP();
struct gui_emu_info
{
	ConfigLoadStrFP*	ConfigLoadStr;	//Can be used to Read/Write settings :)
	ConfigSaveStrFP*	ConfigSaveStr;
	ConfigLoadIntFP*	ConfigLoadInt;
	ConfigSaveIntFP*	ConfigSaveInt;
	ConfigExistsFP*		ConfigExists;

	dbgReadMemFP*		dbgReadMem;
	dbgWriteMemFP*		dbgWriteMem;

	EmuStartedFP*		EmuStarted;

	EmuInitFP*			EmuInit;

	EmuStartFP*			EmuStart;
	EmuStopFP*			EmuStop;
	EmuResetFP*			EmuReset;
	EmuResetCachesFP*	EmuResetCaches;

	EmuStepFP*			EmuStep;
	EmuSkipFP*			EmuSkip;

	EmuSetPatchFP*		EmuSetPatch;


	EmuBootHLEFP*		EmuBootHLE;
	EmuLoadBinaryFP*	EmuLoadBinary;
	
	EmuSelectPluginsFP*	EmuSelectPlugins;
	EmuStartProfilerFP*	EmuStartProfiler;
	EmuStopProfilerFP*	EmuStopProfiler;

	DissasembleOpcodeFP* DissasembleOpcode;
	Sh4GetRegisterFP* Sh4GetRegister;
	Sh4SetRegisterFP* Sh4SetRegister;
	GetSymbNameFP*		GetSymbName;

	EditEmuSettingFP* GetSetting;
	EditEmuSettingFP* SetSetting;
	GetAboutTextFP* GetAboutText;

	GetPluginListFP*   GetPluginList;    
	GetMapleDeviceListFP*  GetMapleDeviceList;
	FreePluginListFP*     FreePluginList;
};
struct MenuIDList
{
	u32 PowerVR;
	u32 GDRom;
	u32 Aica;
	u32 Maple;
	u32 Maple_port[4][6];
	u32 ExtDev;
	u32 Debug;
};

typedef s32 EXPORT_CALL GuiLoadFP(gui_emu_info* emu);
typedef void EXPORT_CALL GuiUnloadFP();

typedef void EXPORT_CALL GuiMainloopFP();
typedef void EXPORT_CALL GetMenuIDsFP(MenuIDList* mil);
typedef void EXPORT_CALL DeleteAllMenuItemChildsFP(u32 id);

struct gui_plugin_info
{
	u32 InterfaceVersion;
	char Name[128];

	GuiLoadFP* Load;
	GuiUnloadFP* Unload;
	GuiMainloopFP* Mainloop;
	GetMenuIDsFP* GetMenuIDs;
	guiMsgBoxFP*		MsgBox;
	GetRenderTargetFP* GetRenderTarget;
	SelectPluginsGuiFP* SelectPluginsGui;

	AddMenuItemFP*		AddMenuItem;
	SetMenuItemStyleFP*	SetMenuItemStyle;
	SetMenuItemFP*		SetMenuItem;
	GetMenuItemFP*		GetMenuItem;
	DeleteMenuItemFP*	DeleteMenuItem;
	DeleteAllMenuItemChildsFP* DeleteAllMenuItemChilds;
};

typedef void EXPORT_CALL ndcGetInterfaceFP(gui_plugin_info* gpi);

#define GuiPluginInterfaceVersion DC_MakeVersion(1,0,0,DC_VER_NORMAL);