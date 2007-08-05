// nullAICA.cpp : Defines the entry point for the DLL application.
//

#include "nullExtDev.h"
#include "modem.h"
#include "lan.h"
emu_info emu;
__settings settings;

//u32 mode=1;//0=null, 1=modem,2=lan,3=bba
//"Emulates" the "nothing atached to ext.dev" :p

//006* , on area0
u32 FASTCALL ReadMem_A0_006(u32 addr,u32 size)
{
	switch(settings.mode)
	{
	case 0:
		return 0;
	case 1:
		return ModemReadMem_A0_006(addr,size);
	case 2:
		return LanReadMem_A0_006(addr,size);
	default:
		printf("Read from modem area on mode %d, addr[%d] 0x%08Xn",settings.mode,size,addr);
		return 0;
	}
	
}
void FASTCALL WriteMem_A0_006(u32 addr,u32 data,u32 size)
{
	switch(settings.mode)
	{
	case 0:
		return;
	case 1:
		ModemWriteMem_A0_006(addr,data,size);
		return;
	case 2:
		LanWriteMem_A0_006(addr,data,size);
		return;
	default:
		printf("Write to modem area on mode %d, addr[%d] 0x%08X=0x%08X\n",settings.mode,size,addr,data);
		return;
	}
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
u32 update_timer;
void (*update_callback) ();
void SetUpdateCallback(void (*callback) (),u32 ms)
{
	verify(update_callback==0);
	update_callback=callback;
	update_timer=ms*DCclock;
}
void ExpireUpdate(bool v)
{
	void (*t)()=update_callback;
	update_callback=0;
	if (v)
		t();
}
//~ called every 1.5k cycles
void FASTCALL Update(u32 cycles)
{
	if (update_callback)
	{
		if (update_timer>cycles)
		{
			void (*t)()=update_callback;
			update_callback=0;
			t();
		}
		update_timer-=cycles;
	}
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


void EXPORT_CALL handle_about(u32 id,void* window,void* p)
{
	MessageBox((HWND)window,"Made by the nullDC team\nThis plugin partialy emulates the modem for now.\n\nNow , go back before its too late ...","nullExtDev plugin",MB_OK | MB_ICONINFORMATION);
}
u32 mids[4];
void nide_set_selected()
{
	for (int i=0;i<4;i++)
	{
		if (i==settings.mode)
			emu.SetMenuItemStyle(mids[i],MIS_Checked,MIS_Checked);
		else
			emu.SetMenuItemStyle(mids[i],0,MIS_Checked);
	}
}
template<u32 v>
void EXPORT_CALL handle_mode(u32 id,void* window,void* p)
{
	settings.mode=v;
	nide_set_selected();
	SaveSettings();
}

//called when plugin is used by emu (you should do first time init here)
s32 FASTCALL Load(emu_info* param)
{
	memcpy(&emu,param,sizeof(emu));

	LoadSettings();

	mids[0]=emu.AddMenuItem(emu.RootMenu,-1,"None",handle_mode<0>,0);
	mids[1]=emu.AddMenuItem(emu.RootMenu,-1,"Modem",handle_mode<1>,0);
	mids[2]=emu.AddMenuItem(emu.RootMenu,-1,"Lan Adapter",handle_mode<2>,0);
	mids[3]=emu.AddMenuItem(emu.RootMenu,-1,"BBA",handle_mode<3>,0);

	nide_set_selected();
	
	emu.SetMenuItemStyle(emu.AddMenuItem(emu.RootMenu,-1,"--",0,settings.mode==0),MIS_Seperator,MIS_Seperator);
	
	emu.AddMenuItem(emu.RootMenu,-1,"About",handle_about,0);
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


//Give to the emu pointers for the PowerVR interface
bool EXPORT_CALL dcGetInterface(plugin_interface* info)
{
	#define c info->common 
	#define ed info->ext_dev

	info->InterfaceVersion=PLUGIN_I_F_VERSION;

	c.Type=Plugin_ExtDevice;
	c.InterfaceVersion=PLUGIN_I_F_VERSION;

	strcpy(c.Name,"nullExtDev (" __DATE__ ")");
	c.PluginVersion=DC_MakeVersion(MAJOR,MINOR,BUILD,DC_VER_NORMAL);
	
	c.Load=Load;
	c.Unload=Unload;
	

	ed.Init=edInit;
	ed.Reset=edReset;
	ed.Term=edTerm;

	ed.ReadMem_A0_006=ReadMem_A0_006;
	ed.WriteMem_A0_006=WriteMem_A0_006;
	
	ed.ReadMem_A0_010=ReadMem_A0_010;
	ed.WriteMem_A0_010=WriteMem_A0_010;

	ed.ReadMem_A5=ReadMem_A5;
	ed.WriteMem_A5=WriteMem_A5;

	ed.UpdateExtDevice=Update;

	return true;
}

void LoadSettings()
{
	settings.mode=max(0,min(emu.ConfigLoadInt("nullExtDev","mode",0),3));
}
void SaveSettings()
{
	emu.ConfigSaveInt("nullExtDev","mode",settings.mode);
}