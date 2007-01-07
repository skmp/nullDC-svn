// nullAICA.cpp : Defines the entry point for the DLL application.
//

#include "sdlAICA.h"
#include "sgc_if.h"
#include "aica.h"
#include "arm7.h"
#include "mem.h"
#include "sdl_audiostream.h"

setts settings;
aica_init_params aica_params;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


void cfgdlg(PluginType type,void* window)
{
	printf("Config coming soon [blame ms/msvc for that , its their fault for making window related code suck so much]");
}

//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	strcpy(info->Name,"nullDC AICA plugin [sdl] , built :" __DATE__ "");
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

	info->ReadMem_aica_ram=sh4_ReadMem_ram;
	info->WriteMem_aica_ram=sh4_WriteMem_ram;
	info->ReadMem_aica_reg=sh4_ReadMem_reg;
	info->WriteMem_aica_reg=sh4_WriteMem_reg;
	info->UpdateAICA=UpdateAICA;
}


//called when plugin is used by emu (you should do first time init here)
void dcInit(void* param,PluginType type)
{
	aica_init_params* initp=(aica_init_params*)param;
	memcpy(&aica_params,initp,sizeof(aica_params));

	//load default settings before init
	settings.BufferSize=1024;

	init_mem();
	arm_Init();
	AICA_Init();
	InitAudio();
}

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
void dcTerm(PluginType type)
{
	TermAudio();
	AICA_Term();
	term_mem();
}

//It's suposed to reset anything 
void dcReset(bool Manual,PluginType type)
{
	arm_Reset();
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInit(PluginType type)
{
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTerm(PluginType type)
{
}