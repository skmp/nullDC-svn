#pragma once
#include "types.h"
#include "plugin_types.h"
#include "gui_plugin_header.h"
#include "dc/sh4/sh4_if.h"

//Plugin handling functions :)
struct nullDC_plugin
{
	nullDC_plugin()
	{
		Inited=false;
		Loaded=false;
	}
	cDllHandler dll;
	int id;
	char dll_file[512];
	s32 Open(char* plugin);
	virtual void LoadI(plugin_interface* plugin)=0;
	bool IsOpened();
	void Close();
	bool Inited;
	bool Loaded;
};
struct nullDC_PowerVR_plugin:common_info,pvr_plugin_if,nullDC_plugin
{
	void LoadI(plugin_interface* plugin);
};

struct nullDC_GDRom_plugin:common_info,gdr_plugin_if,nullDC_plugin
{
	void LoadI(plugin_interface* plugin);
};

struct nullDC_AICA_plugin:common_info,aica_plugin_if,nullDC_plugin
{
	void LoadI(plugin_interface* plugin);
};

struct nullDC_Maple_plugin:common_info,maple_plugin_if,nullDC_plugin
{
	u32 ReferenceCount;
	void LoadI(plugin_interface* plugin);
};

struct nullDC_ExtDevice_plugin:common_info,ext_device_plugin_if,nullDC_plugin
{
	void LoadI(plugin_interface* plugin);
};
//Struct to hold plugin info
struct PluginLoadInfo
{
	char			Name[128];			//plugin name
	VersionNumber	PluginVersion;		//plugin version
	PluginType		Type;				//plugin type
	char			dll[512];
};
struct MapleDeviceDefinition:maple_device_definition
{
	char dll[512];		//xxxx.dll:id
	char dll_file[512];	//xxxx.dll
	VersionNumber	PluginVersion;
	u32 id;
};

void EnumeratePlugins();
List<PluginLoadInfo>* GetPluginList(PluginType type);
List<MapleDeviceDefinition>* GetMapleDeviceList(MapleDeviceType type);

//This is not used for maple

extern sh4_if*				  sh4_cpu;
//Currently used plugins
extern nullDC_PowerVR_plugin	libPvr;
extern nullDC_GDRom_plugin		libGDR;
extern nullDC_AICA_plugin		libAICA;
extern List<nullDC_Maple_plugin*>libMaple;
extern nullDC_ExtDevice_plugin	libExtDevice;
extern gui_plugin_info			libgui;
//more to come

bool plugins_Load();
void plugins_Unload();
bool plugins_Select();
void plugins_Reset(bool Manual);

//sh4 thread
bool plugins_Init();
void plugins_Term();

