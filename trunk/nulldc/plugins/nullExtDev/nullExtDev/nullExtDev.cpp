// nullAICA.cpp : Defines the entry point for the DLL application.
//

#include "nullExtDev.h"

//"Emulates" the "nothing atached to ext.dev" :p

//006* , on area0
u32 FASTCALL ReadMem_A0_006(u32 addr,u32 size)
{
	return 0;
}
void FASTCALL WriteMem_A0_006(u32 addr,u32 data,u32 size)
{
}
//010* , on area0
u32 FASTCALL ReadMem_A0_010(u32 addr,u32 size)
{
	return 0;
}
void FASTCALL WriteMem_A0_010(u32 addr,u32 data,u32 size)
{
}
//Area 5
u32 FASTCALL ReadMem_A5(u32 addr,u32 size)
{
	return 0;
}
void FASTCALL WriteMem_A5(u32 addr,u32 data,u32 size)
{
}

//~ called every 1.5k cycles
void FASTCALL Update(u32 cycles)
{
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


void FASTCALL cfgdlg(void* window)
{
//	MessageBox((HWND)window,"Nothing to configure","nullExtDev plugin",MB_OK | MB_ICONINFORMATION);
}


//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Load(emu_info* param)
{
	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
void FASTCALL Unload()
{

}

//It's suposed to reset anything 
void FASTCALL edReset(bool Manual)
{

}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
s32 FASTCALL edInit(ext_device_init_params* p)
{
	return rv_ok;
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void FASTCALL edTerm()
{
}

//Give to the emu info for the plugin type
void EXPORT_CALL dcGetInterfaceInfo(plugin_interface_info* info)
{
	info->InterfaceVersion=PLUGIN_I_F_VERSION;
	info->count=1;
}

//Give to the emu pointers for the PowerVR interface
bool EXPORT_CALL dcGetInterface(u32 id,plugin_interface* info)
{
	#define c info->common 
	#define ed info->ext_dev

	c.Type=Plugin_ExtDevice;
	c.InterfaceVersion=PLUGIN_I_F_VERSION;

	strcpy(c.Name,"nullExtDev (" __DATE__ ")");
	c.PluginVersion=DC_MakeVersion(MAJOR,MINOR,BUILD,DC_VER_NORMAL);
	
	c.Load=Load;
	c.Unload=Unload;
	

	ed.Init=edInit;
	ed.Reset=edReset;
	ed.Term=edTerm;

	ed.ShowConfig=cfgdlg;
	

	ed.ReadMem_A0_006=ReadMem_A0_006;
	ed.WriteMem_A0_006=WriteMem_A0_006;
	
	ed.ReadMem_A0_010=ReadMem_A0_010;
	ed.WriteMem_A0_010=WriteMem_A0_010;

	ed.ReadMem_A5=ReadMem_A5;
	ed.WriteMem_A5=WriteMem_A5;

	ed.UpdateExtDevice=Update;

	return true;
}