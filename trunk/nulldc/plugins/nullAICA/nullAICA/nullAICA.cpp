// nullAICA.cpp : Defines the entry point for the DLL application.
//

#include "nullAICA.h"
#include "aica_hax.h"
#include "aica_hle.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


void cfgdlg(PluginType type,void* window)
{
	//printf("null AICA plugin [h4x0rs olny kthx]:No config kthx , however i will show ya some hle info :D\n");
//	MessageBox((HWND)window,"Nothing to configure","nullAICA plugin",MB_OK | MB_ICONINFORMATION);
	//ARM_Katana_Driver_Info();
}

//Give to the emu info for the plugin type
void EXPORT_CALL dcGetInterfaceInfo(plugin_interface_info* info)
{
	info->InterfaceVersion=PLUGIN_I_F_VERSION;
	info->count=1;

}
s32 FASTCALL PluginLoad(emu_info* param)
{
	return rv_ok;
}

void FASTCALL PluginUnload()
{
}
s32 FASTCALL Init(aica_init_params* param)
{
	aica_ram=param->aica_ram;
	init_mem();
	InitHLE();

	return rv_ok;
}
void FASTCALL Term()
{
	term_mem();
	TermHLE();
}
void FASTCALL Reset(bool Manual)
{
		ResetHLE();
}

//Give to the emu pointers for the PowerVR interface
void EXPORT_CALL dcGetInterface(u32 id , plugin_interface* info)
{
#define c info->common
#define a info->aica

	strcpy(c.Name,"nullAICA [no sound/reduced compat] (" __DATE__ ")");
	c.PluginVersion=NDC_MakeVersion(MAJOR,MINOR,BUILD);

	c.InterfaceVersion=AICA_PLUGIN_I_F_VERSION;
	c.Type=AICA;

	c.Load=PluginLoad;
	c.Unload=PluginUnload;

	a.Init=Init;
	a.Reset=Reset;
	a.Term=Term;
	a.ShowConfig=0;
	a.ExeptionHanlder=0;

	a.UpdateAICA=UpdateAICA;

	a.ReadMem_aica_reg=ReadMem_reg;
	a.WriteMem_aica_reg=WriteMem_reg;
	
/*
	info->Init=dcInit;
	info->Term=dcTerm;
	info->Reset=dcReset;

	info->ThreadInit=dcThreadInit;
	info->ThreadTerm=dcThreadTerm;
	info->ShowConfig=cfgdlg;
	info->Type=PluginType::AICA;

	info->InterfaceVersion.full=AICA_PLUGIN_I_F_VERSION;

	info->ReadMem_aica_ram=ReadMem_ram;
	info->WriteMem_aica_ram=WriteMem_ram;
	info->ReadMem_aica_reg=ReadMem_reg;
	info->WriteMem_aica_reg=WriteMem_reg;
	info->UpdateAICA=UpdateAICA;*/
}