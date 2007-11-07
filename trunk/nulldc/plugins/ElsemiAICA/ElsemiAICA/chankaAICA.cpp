// chankaAICA.cpp : Defines the entry point for the DLL application.
//

#include "chankaAICA.h"
#include "chanka_aica.h"
bool FrameLimit;
aica_init_params params;
emu_info emu;
BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	return TRUE;
}

void EXPORT_CALL handle_About(u32 id,void* w,void* p)
{
	MessageBox((HWND)w,"Made by the Chankast Team\nPort by drk||Raziel","About Chankast Aica...",MB_ICONINFORMATION);
}
void EXPORT_CALL handle_SyncAudio(u32 id,void* w,void* p)
{
	FrameLimit=!FrameLimit;
	emu.ConfigSaveInt("ElsemiAICA","SyncAudio",FrameLimit);
	emu.SetMenuItemStyle(id,FrameLimit?MIS_Checked:0,MIS_Checked);
}
//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Load(emu_info * p)
{
	memcpy(&emu,p,sizeof(*p));

	FrameLimit = emu.ConfigLoadInt("ElsemiAICA","SyncAudio",1)!=0;

	emu.AddMenuItem(emu.RootMenu,-1,"Sync Audio",handle_SyncAudio,FrameLimit);
	emu.AddMenuItem(emu.RootMenu,-1,0,0,0);
	emu.AddMenuItem(emu.RootMenu,-1,"About",handle_About,0);

	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInit is called (eg , not called to enumerate plugins)
void FASTCALL Unload( )
{

}
extern u8* g_pSH4SoundRAM;
//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
s32 FASTCALL Init(aica_init_params* p)
{
	memcpy(&params,p,sizeof(*p));
	g_pSH4SoundRAM=params.aica_ram;
	InitARM7();
	return rv_ok;
}


//It's suposed to reset anything 
void FASTCALL Reset(bool Manual)
{
}


//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void FASTCALL Term()
{
	TerminateARM7();
}

//Give to the emu pointers for the PowerVR interface
EXPORT void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
	info->InterfaceVersion=PLUGIN_I_F_VERSION;
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
	info->UpdateAICA=UpdateSystem;
	*/
#define c info->common
#define a info->aica

	strcpy(c.Name,"Elsemi's AICA (" __DATE__ ")");
	c.PluginVersion=DC_MakeVersion(MAJOR,MINOR,BUILD,DC_VER_NORMAL);

	c.InterfaceVersion=AICA_PLUGIN_I_F_VERSION;
	c.Type=Plugin_AICA;

	c.Load=Load;
	c.Unload=Unload;

	a.Init=Init;
	a.Reset=Reset;
	a.Term=Term;
	a.ExeptionHanlder=0;

	a.UpdateAICA=UpdateSystem;

	a.ReadMem_aica_reg=ReadMem_reg;
	a.WriteMem_aica_reg=WriteMem_reg;
}


