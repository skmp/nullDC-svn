#pragma once
#include "types.h"
#include "plugin_types.h"
#include "dc/sh4/sh4_if.h"

//Plugin loading functions

//Basic plugin interface
//dcGetPluginInfo(plugin_info* info);
enum PluginLoadError
{
	NoError,							//Success
	DllLoadError,						//Dll not found/ could not be loaded
	PluginInterfaceMissing,				//Missing one or more basic i/f exports
	PluginInterfaceExMissing,			//Missing one or more plugin type speciacific i/f exports
	PluginInterfaceVersionError,		//Basic plugin i/f version error
	PluginInterfaceExVersionError		//Plugin type speciacific i/f version error
};


//Handles generic loading of plugin
class nullDC_plugin
{
public:
	plugin_info info;
	bool Loaded;
	char dll[512];
protected:
	cDllHandler lib;
	dcGetPluginInfoFP* dcGetPluginInfo;
	//must be implemented for each inherited class
	virtual PluginLoadError PluginExLoad();

public :
	nullDC_plugin();

	~nullDC_plugin();

	PluginLoadError LoadnullDCPlugin(char* plugin);
	virtual void Unload();
};



//Speciacific plugins
//PowerVR
class nullDC_PowerVR_plugin: public nullDC_plugin
{
public :
	pvr_plugin_if pvr_info;
private:
	dcGetPvrInfoFP* dcGetPvrInfo;

	PluginLoadError PluginExLoad();
public:
	nullDC_PowerVR_plugin();
	~nullDC_PowerVR_plugin();
};
//GDRom
class nullDC_GDRom_plugin: public nullDC_plugin
{
public :
	gdr_plugin_if gdr_info;
private:
	dcGetGDRInfoFP* dcGetGDRInfo;

	PluginLoadError PluginExLoad();
public:
	nullDC_GDRom_plugin();
	~nullDC_GDRom_plugin();
};

//AICA
class nullDC_AICA_plugin: public nullDC_plugin
{
public :
	aica_plugin_if aica_info;
private:
	dcGetAICAInfoFP* dcGetAICAInfo;

	PluginLoadError PluginExLoad();
public:
	nullDC_AICA_plugin();
	~nullDC_AICA_plugin();
};

//Maple 
class nullDC_Maple_plugin: public nullDC_plugin
{
public :
	maple_plugin_if maple_info;
	/*void SendFrame(u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
	{
		//maple_info.MapleDeviceDMA(&maple_info,Command,buffer_in,buffer_in_len,buffer_out,buffer_out_len,responce);
	}*/
private:
	dcGetMapleInfoFP* dcGetMapleInfo;

	PluginLoadError PluginExLoad();
public:
	nullDC_Maple_plugin();
	~nullDC_Maple_plugin();
};

//Ext.Device
class nullDC_ExtDevice_plugin: public nullDC_plugin
{
public :
	ext_device_plugin_if ext_device_info;
private:
	dcGetExtDeviceInfoFP* dcGetExtDeviceInfo;

	PluginLoadError PluginExLoad();
public:
	nullDC_ExtDevice_plugin();
	~nullDC_ExtDevice_plugin();
};

//Struct to hold plugin info
struct PluginLoadInfo
{
	plugin_info plugin_info;
	char		dll[500];
};

List<PluginLoadInfo>* EnumeratePlugins(u32 Typemask);

//This is not used for maple
bool SetPlugin(nullDC_plugin* plugin,PluginType type);
void SetPluginPath(char* path);

extern sh4_if*				  sh4_cpu;
//Currently used plugins
extern nullDC_PowerVR_plugin* libPvr;
extern nullDC_GDRom_plugin*   libGDR;
extern nullDC_AICA_plugin*    libAICA;
extern List<nullDC_Maple_plugin*>libMaple;
extern nullDC_ExtDevice_plugin*	libExtDevice;
//more to come

void plugins_Init();
void plugins_Term();
void plugins_Reset(bool Manual);
void plugins_ThreadInit();
void plugins_ThreadTerm();
