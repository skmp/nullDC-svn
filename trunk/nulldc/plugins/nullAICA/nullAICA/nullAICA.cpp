// nullAICA.cpp : Defines the entry point for the DLL application.
//

#include "nullAICA.h"
#include "aica_hax.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	strcpy(info->Name,"null AICA plugin [h4x0rs olny kthx]");
	info->PluginVersion.full=NDC_MakeVersion(MAJOR,MINOR,BUILD);
	

	info->Init=dcInit;
	info->Term=dcTerm;
	info->Reset=dcReset;

	info->ThreadInit=dcThreadInit;
	info->ThreadTerm=dcThreadTerm;

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
	info->UpdateAICA=UpdateAICA;
}


//called when plugin is used by emu (you should do first time init here)
void dcInit(void* param,PluginType type)
{
	init_mem();
}

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
void dcTerm(PluginType type)
{
	term_mem();
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