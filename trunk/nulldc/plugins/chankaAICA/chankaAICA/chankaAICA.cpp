// chankaAICA.cpp : Defines the entry point for the DLL application.
//

#include "chankaAICA.h"
#include "chanka_aica.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

void cfgdlg(PluginType type,void* window)
{
	printf("Chanka's AICA [port by drk||Raziel]:No config kthx\n");
}

//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	strcpy(info->Name,"Chanka's AICA [port by drk||Raziel](" __DATE__ ")");
	info->PluginVersion.full=NDC_MakeVersion(MAJOR,MINOR,BUILD);
	

	info->Init=dcInit;
	info->Term=dcTerm;
	info->Reset=dcReset;

	info->ThreadInit=dcThreadInit;
	info->ThreadTerm=dcThreadTerm;
	info->ShowConfig=cfgdlg;

	info->Type=PluginType::AICA;
}

//Give to the emu pointers for the PowerVR interface
EXPORT void dcGetAICAInfo(aica_plugin_if* info)
{
	info->InterfaceVersion.full=AICA_PLUGIN_I_F_VERSION;

	info->ReadMem_aica_ram=ReadMem_ram;
	info->WriteMem_aica_ram=WriteMem_ram;
	info->ReadMem_aica_reg=ReadMem_reg;
	info->WriteMem_aica_reg=WriteMem_reg;
	info->UpdateAICA=UpdateSystem;
}


void * win_handle;
//called when plugin is used by emu (you should do first time init here)
void dcInit(void* param,PluginType type)
{
	aica_init_params* ip=(aica_init_params*)param;
	Sh4RaiseInterrupt=ip->RaiseInterrupt;
	SB_ISTEXT=ip->SB_ISTEXT;
	win_handle=ip->WindowHandle;
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
	InitARM7(win_handle);
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTerm(PluginType type)
{
	TerminateARM7();
}