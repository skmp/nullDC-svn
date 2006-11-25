// nullAICA.cpp : Defines the entry point for the DLL application.
//

#include "nullExtDev.h"

//"Emulates" the "nothing atached to ext.dev" :p

//006* , on area0
u32 ReadMem_A0_006(u32 addr,u32 size)
{
	return 0;
}
void WriteMem_A0_006(u32 addr,u32 data,u32 size)
{
}
//010* , on area0
u32 ReadMem_A0_010(u32 addr,u32 size)
{
	return 0;
}
void WriteMem_A0_010(u32 addr,u32 data,u32 size)
{
}
//Area 5
u32 ReadMem_A5(u32 addr,u32 size)
{
	return 0;
}
void WriteMem_A5(u32 addr,u32 data,u32 size)
{
}

//~ called every 1.5k cycles
void Update(u32 cycles)
{
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


void cfgdlg(PluginType type,void* window)
{
	MessageBox((HWND)window,"Nothing to configure","nullExtDev plugin",MB_OK | MB_ICONINFORMATION);
}

//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	strcpy(info->Name,"nullExtDev (" __DATE__ ")");
	info->PluginVersion.full=NDC_MakeVersion(MAJOR,MINOR,BUILD);
	
	info->Init=dcInit;
	info->Term=dcTerm;
	info->Reset=dcReset;

	info->ThreadInit=dcThreadInit;
	info->ThreadTerm=dcThreadTerm;
	info->ShowConfig=cfgdlg;
	info->Type=PluginType::ExtDevice;
}

//Give to the emu pointers for the PowerVR interface
EXPORT void dcGetExtDeviceInfo(ext_device_plugin_if* info)
{
	info->InterfaceVersion.full=EXTDEVICE_PLUGIN_I_F_VERSION;
	info->ReadMem_A0_006=ReadMem_A0_006;
	info->WriteMem_A0_006=WriteMem_A0_006;
	
	info->ReadMem_A0_010=ReadMem_A0_010;
	info->WriteMem_A0_010=WriteMem_A0_010;

	info->ReadMem_A5=ReadMem_A5;
	info->WriteMem_A5=WriteMem_A5;

	info->UpdateExtDevice=Update;
}


//called when plugin is used by emu (you should do first time init here)
void dcInit(void* param,PluginType type)
{
	ext_device_init_params* params=(ext_device_init_params*)param;
}

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
void dcTerm(PluginType type)
{

}

//It's suposed to reset anything 
void dcReset(bool Manual,PluginType type)
{

}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInit(PluginType type)
{
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTerm(PluginType type)
{
}