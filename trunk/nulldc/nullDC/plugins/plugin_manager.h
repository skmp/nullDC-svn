#pragma once
#include "types.h"
#include "plugin_types.h"
#include "dc/sh4/sh4_if.h"

//Plugin handling functions :)
struct nullDC_plugin
{
	cDllHandler dll;
	int id;
	char dll_file[512];
	bool Open(char* plugin);
	virtual bool LoadI(plugin_info_entry* plugin)=0;
	bool IsOpened();
	void Close();
	bool Inited;
	bool Loaded;
};
struct nullDC_PowerVR_plugin:common_info,pvr_plugin_if,nullDC_plugin
{
	bool LoadI(plugin_info_entry* plugin);
};

struct nullDC_GDRom_plugin:common_info,gdr_plugin_if,nullDC_plugin
{
	bool LoadI(plugin_info_entry* plugin);
};

struct nullDC_AICA_plugin:common_info,aica_plugin_if,nullDC_plugin
{
	bool LoadI(plugin_info_entry* plugin);
};

struct nullDC_Maple_plugin:common_info,maple_plugin_if,nullDC_plugin
{
	bool LoadI(plugin_info_entry* plugin);
};

struct nullDC_Maple_Sub_plugin:common_info,maple_sub_plugin_if,nullDC_plugin
{
	bool LoadI(plugin_info_entry* plugin);
};

struct nullDC_ExtDevice_plugin:common_info,ext_device_plugin_if,nullDC_plugin
{
	bool LoadI(plugin_info_entry* plugin);
};
//Struct to hold plugin info
struct PluginLoadInfo
{
	char			Name[128];			//plugin name
	VersionNumber	PluginVersion;		//plugin version
	PluginType		Type;				//plugin type
	VersionNumber	InterfaceVersion;	//Note : this version is of the interface for this type of plugin :)
	char			dll[512];

	u32 subdev_info;//for maple =p
};

List<PluginLoadInfo>* EnumeratePlugins(PluginType type);

//This is not used for maple
//bool SetPlugin(char* plugin,PluginType type);
void SetPluginPath(char* path);

extern sh4_if*				  sh4_cpu;
//Currently used plugins
extern nullDC_PowerVR_plugin	libPvr;
extern nullDC_GDRom_plugin		libGDR;
extern nullDC_AICA_plugin		libAICA;
extern List<nullDC_Maple_plugin*>libMaple;
extern nullDC_ExtDevice_plugin	libExtDevice;
//more to come

bool plugins_Load();
void plugins_Unload();
bool plugins_Config();
void plugins_Reset(bool Manual);

//sh4 thread
bool plugins_Init();
void plugins_Term();

