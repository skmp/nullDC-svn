/*
			--nullDC plugin managment code--
	Does plugin enumeration and handles dynamic lib loading.
	Also has code for plugin init/reset/term calls :).
*/
#include "types.h"
#include "plugin_manager.h"
#include "dc/pvr/pvr_if.h"
#include "dc/aica/aica_if.h"
#include "dc/gdrom/gdrom_if.h"
#include "gui/base.h"
#include "dc/maple/maple_if.h"
#include "config/config.h"
#include "gui/emuWinUI.h"

#include <string.h>
#include "dc\mem\sb.h"

char* lcp_name;
s32 lcp_error=0;

//Currently Loaded plugins
nullDC_PowerVR_plugin		libPvr;
nullDC_GDRom_plugin			libGDR;
nullDC_AICA_plugin			libAICA;
List<nullDC_Maple_plugin*>	libMaple;
nullDC_ExtDevice_plugin		libExtDevice;

sh4_if*						sh4_cpu;
emu_info					eminf;
//more to come

bool plugins_inited=false;
char plugins_path[1000]="";
bool plugins_enumerated=false;
bool plugins_first_load=false;

List<PluginLoadInfo>			PluginList_cached;
List<MapleDeviceDefinition>		MapleDeviceList_cached;
List<cDllHandler*>				LoadedTempDlls;

//Plugin Enumeration/Validation
enum PluginValidationErrors
{
	pve_invalid_pointer=-200,
	pve_invalid_cif_ver=-201,
	pve_invalid_sif_ver=-202,
	pve_missing_pointers=-203,
	pve_invalid_plugin_type=-204,
};
s32 ValidatePlugin(plugin_interface* plugin)
{
	if (plugin==0)
		return  pve_invalid_pointer;

	if (plugin->InterfaceVersion!=PLUGIN_I_F_VERSION)
	{
		printf("%X != %X\n",plugin->InterfaceVersion,PLUGIN_I_F_VERSION);
		return pve_invalid_cif_ver;
	}


	switch(plugin->common.Type)
	{
	case Plugin_PowerVR:
		{
			if (plugin->common.InterfaceVersion!=PVR_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;

	case Plugin_GDRom:
		{
			if (plugin->common.InterfaceVersion!=GDR_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;

	case Plugin_AICA:
		{
			if (plugin->common.InterfaceVersion!=AICA_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;

	case Plugin_Maple:
		{
			if (plugin->common.InterfaceVersion!=MAPLE_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;

	case Plugin_MapleSub:
		{
			if (plugin->common.InterfaceVersion!=MAPLE_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;

	case Plugin_ExtDevice:
		{
			if (plugin->common.InterfaceVersion!=EXTDEVICE_PLUGIN_I_F_VERSION)
				return pve_invalid_sif_ver;
		}
		break;
	default:
		return pve_invalid_plugin_type;
	}

	return rv_ok;
}
bool AddToPluginList(char* dll)
{
	cDllHandler* lib=new cDllHandler();
	LoadedTempDlls.Add(lib);

	//load dll
	if (!lib->Load(dll))
		return false;
	
	//Get functions
	dcGetInterfaceFP* dcGetInterface=(dcGetInterfaceFP*)lib->GetProcAddress("dcGetInterface");

	if (dcGetInterface==0)
		return false;

	
	//Get plugin list
	plugin_interface info;
	memset(&info,0,sizeof(info));

	dcGetInterface(&info);

	//Make sure the plugin is valid :)
	if (s32 rv = ValidatePlugin(&info))
	{
		return false;
	}
	
	PluginLoadInfo load_info;

	load_info.PluginVersion.full=info.common.PluginVersion;
	load_info.Type=(PluginType)info.common.Type;

	//load_info.subdev_info=info.maple.subdev_info;
	strcpy(load_info.Name,info.common.Name);
	GetFileNameFromPath(dll,load_info.dll);

	//if correct ver , add em to plugins list :)
	PluginList_cached.Add(load_info);

	if (info.common.Type==Plugin_Maple)
	{
		MapleDeviceDefinition t;
		strcpy(t.dll_file,load_info.dll);
		t.PluginVersion.full=info.common.PluginVersion;
		for (int i=0;i<16;i++)
		{
			if (info.maple.devices[i].Type==MDT_EndOfList)
				break;
			t.id=i;
			sprintf(t.dll,"%s:%d",load_info.dll,i);
			maple_device_definition* mdd=&t;
			memcpy(mdd,&info.maple.devices[i],sizeof(maple_device_definition));

			MapleDeviceList_cached.Add(t);
		}
	}

	//lib.Unload();
	return true;
}

void plugin_FileIsFound(char* file,void* param)
{
	char dllfile[1024]="";
	strcat(dllfile,plugins_path);
	strcat(dllfile,file);
	AddToPluginList(dllfile);
}
//Get a list of all plugins that exist on plugin directory and can be loaded
List<PluginLoadInfo>* GetPluginList(PluginType type)
{
	List<PluginLoadInfo>* rv=new List<PluginLoadInfo>();

	for (u32 i=0;i<PluginList_cached.size();i++)
	{
		PluginLoadInfo* t=&PluginList_cached[i];
		if (t->Type == type) 
			rv->Add(*t);
	}

	return rv;
}
List<MapleDeviceDefinition>* GetMapleDeviceList(MapleDeviceType type)
{
	List<MapleDeviceDefinition>* rv = new List<MapleDeviceDefinition>();
	for (size_t i=0;i<MapleDeviceList_cached.size();i++)
	{
		if ((type==MDT_EndOfList) || MapleDeviceList_cached[i].Type==type)
		{
			rv->Add(MapleDeviceList_cached[i]);
		}
	}
	return rv;
}
void EnumeratePlugins()
{
	if (plugins_enumerated)
		return;
	plugins_enumerated=true;

	cfgLoadStr("emu","PluginPath",plugins_path,0);

	PluginList_cached.clear();
	MapleDeviceList_cached.clear();

	
	char dllfile[1024]="";
	strcat(dllfile,plugins_path);
	strcat(dllfile,"*.dll");
	//Look & load all dll's :)
	FindAllFiles(plugin_FileIsFound,dllfile,0);

	//We unload all dll's at once :) this gives a HUGE speedup , as it does not reload common
	//dll's while enumerating ...
	for (size_t i=0;i<LoadedTempDlls.size();i++)
	{
		if (LoadedTempDlls[i]->IsLoaded())
			LoadedTempDlls[i]->Unload();
		delete LoadedTempDlls[i];
	}
	LoadedTempDlls.clear();
}
//
//
//Implementation of the plugin classes
//
//
//-101 -> null Plugin
//-102 path too long
//-103 -> invalid name (split failed)
//-104 dll load failed
//-105 some of the exports are missing
//-106 wrong i/f version
//-107 LoadI (plugin specialised interface) failed
//-108 Invalid id (id >= count)
//0 (rv_ok) -> ok :)
s32 nullDC_plugin::Open(char* plugin)
{
	Inited=false;
	Loaded=false;

	if (!strcmp(plugin,"NULL"))
	{
		return -101;
	}

	if (strlen(plugin)>480)
	{
		msgboxf("Plugin dll path is way too long",MBX_ICONERROR | MBX_OK);
		return -102;
	}
	
	char ttt[512];
	strcpy(dll_file,plugin);
	size_t dllfl=strlen(dll_file);
	size_t pathl=strlen(plugins_path);
	if (dllfl<=pathl || (memcmp(dll_file,plugins_path,pathl)!=0))
	{
		strcpy(ttt,plugins_path);
		strcat(ttt,dll_file);
		strcpy(dll_file,plugin);
	}
	else
	{
		strcpy(ttt,dll_file);
		strcpy(dll_file,&dll_file[pathl]);
	}
	if (!dll.Load(ttt))
		return -104;
	dcGetInterfaceFP* getplugin=(dcGetInterfaceFP*)dll.GetProcAddress("dcGetInterface");

	
	if (getplugin==0)
	{
		dll.Unload();
		return -105;
	}

	plugin_interface t;
	memset(&t,0,sizeof(t));
	getplugin(&t);

	if (s32 rv=ValidatePlugin(&t))
	{
		return rv;
	}

	LoadI(&t);
	
	return rv_ok;
}
bool nullDC_plugin::IsOpened()
{
	return dll.IsLoaded();
}
void nullDC_plugin::Close()
{
	if (IsOpened())
		dll.Unload();
}
//*::LoadI
void nullDC_PowerVR_plugin::LoadI(plugin_interface* t)
{
	common_info* p1= this;
	pvr_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->pvr,sizeof(*p2));
}
void nullDC_GDRom_plugin::LoadI(plugin_interface* t)
{
	common_info* p1= this;
	gdr_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->gdr,sizeof(*p2));
}
void nullDC_AICA_plugin::LoadI(plugin_interface* t)
{
	common_info* p1= this;
	aica_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->aica,sizeof(*p2));
}
void nullDC_Maple_plugin::LoadI(plugin_interface* t)
{
	common_info* p1= this;
	maple_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->maple,sizeof(*p2));

	ReferenceCount=0;
}
void nullDC_ExtDevice_plugin::LoadI(plugin_interface* t)
{
	common_info* p1= this;
	ext_device_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->ext_dev,sizeof(*p2));
}
//
//Maple Plugins :
/*
	when nullDC is started , ALL maple plugins are loaded.Thats why its good to not allocate much things on
	Load().Then , right after Load() , the devices are created(using CreateMain/Sub()).As the user may edit the 
	config , devices can be deleted (using DestroyMain/Sub) and recrated.On CreateMain/Sub the plugin should
	add the menus (if any) and allocate resources as needed.

	When emulation starts , InitMain/Sub is called.Befor Term* is called before Destroy*.If a device is
	hotplugable , it can go into many Create->Init->Term->Destroy cycles while the emulation is running.

	Its quite simple :
	
	Load

	a:
	Create* ->Destroy* -> a or b
			
	        ->Init* -> Term* ->Destroy*-> if(hotplug) (a or b) else b

	b:
	Unload
*/
void maple_cfg_name(int i,int j,char * out)
{
	sprintf(out,"Current_maple%d_%d",i,j);
}
void maple_cfg_plug(int i,int j,char * out)
{
	char temp[512];
	maple_cfg_name(i,j,temp);
	cfgLoadStr("nullDC_plugins",temp,out,"NULL");
}
u8 GetMaplePort(u32 port,u32 device)
{
	u32 rv=port<<6;

	rv|=1<<device;
	return (u8)rv;
}

nullDC_Maple_plugin* FindMaplePlugin(MapleDeviceDefinition* mdd)
{
	for (u32 i=0;i<libMaple.size();i++)
	{
		if (strcmp(libMaple[i]->dll_file,mdd->dll_file)==0)
			return libMaple[i];
	}
	msgboxf("Fatal error :nullDC_Maple_plugin* FindMaplePlugin(MapleDeviceDefinition* mdd) failed",MBX_ICONERROR);
	return 0;
}
MapleDeviceDefinition* FindMapleDevice(char* mfdn)
{
	for (u32 i=0;i<MapleDeviceList_cached.size();i++)
	{
		if (strcmp(MapleDeviceList_cached[i].dll,mfdn)==0)
			return &MapleDeviceList_cached[i];
	}
	msgboxf("Fatal error :MapleDeviceDefinition* FindMapleDevice(char* mfdn) failed",MBX_ICONERROR);
	return 0;
}

s32 CreateMapleDevice(u32 pos,char* device,bool hotplug);
void cm_MampleSubEmpty(u32 root,u32 port,u32 subport)
{
	DeleteAllMenuItemChilds(root);
	//l8r
}
void cm_MampleSubUsed(u32 root,u32 port,u32 subport)
{
	DeleteAllMenuItemChilds(root);
	//l8r
}
void FASTCALL menu_handle_attach_main(u32 id,void* win,void* p)
{
	size_t offs=(u8*)p-(u8*)0;
	u32 plid=(u32)offs&0xFFFFFF;
	u32 port=((u32)offs>>24);

	CreateMapleDevice(port,MapleDeviceList_cached[plid].dll,true);
}
void cm_MampleMainEmpty(u32 root,u32 port)
{
	DeleteAllMenuItemChilds(root);
	//Add the
	
	for (size_t i=0;i<MapleDeviceList_cached.size();i++)
	{
		if (!(MapleDeviceList_cached[i].Flags & MDTF_Hotplug))
			continue;
		if (MapleDeviceList_cached[i].Type!=MDT_Main)
			continue;
		char text[512];
		sprintf(text,"Attach %s %d.%d.%d",MapleDeviceList_cached[i].Name,MapleDeviceList_cached[i].PluginVersion.major,MapleDeviceList_cached[i].PluginVersion.minnor,MapleDeviceList_cached[i].PluginVersion.build);
		//Attach NAME
		u32 menu=AddMenuItem(root,-1,text,menu_handle_attach_main,0);
		MenuItem mi;
		u8* t=0;
		t+=MapleDeviceList_cached[i].id;
		t+=(port<<24);
		mi.PUser=t;
		SetMenuItem(menu,&mi,MIM_PUser);
	}
}
void cm_MampleMainUsed(u32 root,u32 port,u32 flags)
{
	DeleteAllMenuItemChilds(root);
	//add the 

	//Subdevice X ->[default empty]
	for (u32 i=0;i<4;i++)
	{
		if (flags & (1<<i))
		{
			u32 sdr=AddMenuItem(root,-1,"Subdevice ..",0,0);
			cm_MampleSubEmpty(sdr,port,i);
		}
	}
	
	//-
	u32 sep=AddMenuItem(root,-1,"",0,0);
	SetMenuItemStyle(sep,MIS_Seperator,MIS_Seperator);
	//Unplug
	AddMenuItem(root,-1,"Unplug",0,0);
}
//These are the 'raw' functions , they handle creation/destruction of a device *only*
enum pmd_errors
{
	pmde_device_state=-250,		//device allready connected/disconected
	pmde_invalid_pos=-251,		//invalid device pos (0~3 , 0~4 are valid only)
	pmde_failed_create=-252,	//failed to create device
	pmde_failed_create_s=-253,	//failed to create device , silent error

};
s32 CreateMapleDevice(u32 pos,char* device,bool hotplug)
{
	if (pos>3)
		return pmde_invalid_pos;

	if (MapleDevices[pos].connected)
		return pmde_device_state;

	MapleDeviceDefinition* mdd=FindMapleDevice(device);

	nullDC_Maple_plugin* plg =FindMaplePlugin(mdd);
	
	cm_MampleMainUsed(Maple_menu_ports[pos],pos,mdd->Flags);
	MapleDevices[pos].port=GetMaplePort(pos,5);
	
	if (s32 rv= plg->CreateMain(&MapleDevices[pos],mdd->id,hotplug?MDCF_Hotplug:MDCF_None,Maple_menu_ports[pos]))
	{
		MapleDevices_dd[pos][5].mdd=mdd;

		cm_MampleMainEmpty(Maple_menu_ports[pos],pos);
		if (rv==rv_error)
			return pmde_failed_create;
		else
			return pmde_failed_create_s;
	}
	MapleDevices[pos].connected=true;

	return rv_ok;
}
s32 DestroyMapleDevice(u32 pos)
{
	if (pos>3)
		return pmde_invalid_pos;

	if (!MapleDevices[pos].connected)
		return pmde_device_state;

	MapleDeviceDefinition* mdd=MapleDevices_dd[pos][5].mdd;
	MapleDevices_dd[pos][5].mdd=0;
	nullDC_Maple_plugin* plg =FindMaplePlugin(mdd);

	plg->DestroyMain(&MapleDevices[pos]);
	cm_MampleMainEmpty(Maple_menu_ports[pos],pos);
}
s32 CreateMapleSubDevice(u32 pos,u32 subport,char* device)
{
	return 0;
}
s32 DestroyMapleSubDevice(u32 pos,u32 subport)
{
return 0;
}

//Misc handling code ~
/************************************/
/*********Plugin Load/Unload*********/
/************************************/
template<typename T>
s32 load_plugin(char* dll,T* plug,u32 rootmenu)
{
	lcp_error=-100;
	lcp_name=dll;
	if (!plug->Loaded)
	{
		SetMenuItemHandler(rootmenu,0);
		if ((lcp_error=plug->Open(dll))!=rv_ok)
		{
			return rv_error;
		}
		if (s32 rv = plug->Load(&eminf,rootmenu))
		{
			return rv;
		}
		plug->Loaded=true;
		printf("Loaded %s[%s]\n",plug->Name,lcp_name);
	}

	return rv_ok;
}
template<typename T>
void unload_plugin(T* plug,u32 rootmenu)
{
	lcp_name=plug->Name;
	DeleteAllMenuItemChilds(rootmenu);
	SetMenuItemHandler(rootmenu,0);
	if (plug->IsOpened()) 
	{
		if (plug->Loaded) 
		{
			plug->Unload(); 
			plug->Loaded=false;
			printf("Unloaded %s[%s]\n",plug->Name,plug->dll_file);
		}
		plug->Close(); 
	}
}

#define load_plugin_(cfg_name,to,menu) \
{ \
	char dllf[512]; \
	dllf[0]=0; \
	cfgLoadStr("nullDC_plugins",cfg_name,dllf,"NULL"); \
	if (s32 rv=load_plugin(dllf,to,menu)) \
		return rv; \
}
//Internal function for load/unload w/ error checking :)
s32 plugins_Load_()
{
	if (!plugins_first_load)
	{
		//if first time load , init the maple menus
		plugins_first_load=true;
		for (u32 i=0;i<4;i++)
			cm_MampleMainEmpty(Maple_menu_ports[i],i);
	}
	eminf.ConfigLoadStr=cfgLoadStr;
	eminf.ConfigSaveStr=cfgSaveStr;
	eminf.ConfigLoadInt=cfgLoadInt;
	eminf.ConfigSaveInt=cfgSaveInt;

	eminf.AddMenuItem=AddMenuItem;
	eminf.SetMenuItemStyle=SetMenuItemStyle;
	eminf.GetMenuItem=GetMenuItem;
	eminf.SetMenuItem=SetMenuItem;
	eminf.DeleteMenuItem=DeleteMenuItem;

	eminf.WindowHandle=GetRenderTargetHandle();

	load_plugin_("Current_PVR",&libPvr,PowerVR_menu);
	load_plugin_("Current_GDR",&libGDR,GDRom_menu);
	load_plugin_("Current_AICA",&libAICA,Aica_menu);
 	load_plugin_("Current_ExtDevice",&libExtDevice,ExtDev_menu);

	List<PluginLoadInfo>* mpl= GetPluginList(Plugin_Maple);

	if (libMaple.size()!=mpl->size())
	{
		for (size_t i=0;i<mpl->size();i++)
		{
			char fname[512];
			nullDC_Maple_plugin* cmp= new nullDC_Maple_plugin();

			GetFileNameFromPath((*mpl)[i].dll,fname);

			if (s32 rv=load_plugin(fname,cmp,Maple_menu))
			{
				delete cmp;
				return rv;
			}
			libMaple.Add(cmp);
		}
	}
	delete mpl;

	//Create Maple Devices

	char plug_name[512];
	for (int port=0;port<4;port++)
	{
		maple_cfg_plug(port,5,plug_name);
		if (strcmp(plug_name,"NULL")!=0)
		{
			if (!MapleDevices_dd[port][5].Created)
			{
				u32 rv=CreateMapleDevice(port,plug_name,false);
				if (rv!=rv_ok)
				{
					return rv;
				}
				MapleDevices_dd[port][5].Created=true;
			}
			/*
			for (int subport=0;subport<4;subport++)
			{
				if (!MapleDevices_dd[port][subport].Created)
				{
					//Create it
					MapleDevices_dd[port][subport].Created=true
				}
			}
			*/
		}
	}

	return rv_ok;
}

//Loads plugins , if allready loaded does nothing :)
bool plugins_Load()
{
	__try 
	{
		if (s32 rv=plugins_Load_())
		{
			if (rv==rv_error)
			{
				msgboxf("Unable to load %s plugin , errorlevel=%d",MB_ICONERROR,lcp_name,lcp_error);
			}

			plugins_Unload();
			return false;
		}
		else
			return true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		msgboxf("Unhandled exeption while loading %s plugin",MB_ICONERROR,lcp_name);
		plugins_Unload();
		return false;
	}
}

//Unloads plugins , if allready unloaded does nothing
void plugins_Unload()
{
	__try 
	{
		for (int port=3;port>=0;port--)
		{
			if (MapleDevices_dd[port][5].Created)
			{
				MapleDevices_dd[port][5].Created=false;
				u32 rv=DestroyMapleDevice(port);
				if (rv!=rv_ok)
				{
					printf("DestroyMapleDevice(port) failed: %d\n",rv);
				}
				/*
				for (int subport=0;subport<4;subport++)
				{
					if (MapleDevices_dd[port][subport].Created)
					{
					//destroy it
						MapleDevices_dd[port][subport].Created=false;
					}
				}
				*/
			}
		}

		for (size_t i=libMaple.size();i>0;i--)
		{
			unload_plugin(libMaple[i-1],Maple_menu);
		}
		libMaple.clear();

		unload_plugin(&libExtDevice,ExtDev_menu);
		unload_plugin(&libAICA,Aica_menu);
		unload_plugin(&libGDR,GDRom_menu);
		unload_plugin(&libPvr,PowerVR_menu);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		msgboxf("Unhandled exeption while unloading %s\n",MB_ICONERROR,lcp_name);
	}
}
/************************************/
/******Plugin selection-switch*******/
/************************************/
template<typename T>
void CheckPlugin(char*cfg_name, T* plug,u32 rootmenu)
{
	char dllf[512];
	dllf[0]=0;
	cfgLoadStr("nullDC_plugins",cfg_name,dllf,"NULL");
	if (strcmp(plug->dll_file,dllf)!=0)
	{
		unload_plugin(plug,rootmenu);
	}
}
bool plugins_Select()
{
	if (plugins_inited)
	{
		if (msgboxf("Emulation is started , plugins can't be changed now ..\r\nWant to Continue anyway ? (Changes will will take effect after retart)",MBX_YESNO | MBX_ICONEXCLAMATION | MBX_TASKMODAL)==MBX_RV_NO)
			return false;
	}

	bool rv= SelectPluginsGui();

	if (rv && plugins_inited==false)
	{
		CheckPlugin("Current_PVR",&libPvr,PowerVR_menu);
		CheckPlugin("Current_GDR",&libGDR,GDRom_menu);
		CheckPlugin("Current_AICA",&libAICA,Aica_menu);
 		CheckPlugin("Current_ExtDevice",&libExtDevice,ExtDev_menu);

		return plugins_Load();
	}
	return rv;
}
/************************************/
/*******Plugin Init/Term/Reset*******/
/************************************/
s32 plugins_Init_()
{
	if (plugins_inited)
		return rv_ok;
	plugins_inited=true;
	//pvr
	pvr_init_params pvr_info;
	pvr_info.RaiseInterrupt=sh4_cpu->RaiseInterrupt;
	pvr_info.vram=&vram[0];
	pvr_info.vram_lock_32=vramlock_Lock_32;
	pvr_info.vram_lock_64=vramlock_Lock_64;
	pvr_info.vram_unlock=vramlock_Unlock_block;

	lcp_name=libPvr.Name;
	if (s32 rv = libPvr.Init(&pvr_info))
		return rv;
	libPvr.Inited=true;
	

	gdr_init_params gdr_info;
	gdr_info.DriveNotifyEvent=NotifyEvent_gdrom;

	lcp_name=libGDR.Name;
	if (s32 rv = libGDR.Init(&gdr_info))
		return rv;
	libGDR.Inited=true;
	

	aica_init_params aica_info;
	aica_info.RaiseInterrupt=sh4_cpu->RaiseInterrupt;
	aica_info.SB_ISTEXT=&SB_ISTEXT;
	aica_info.CDDA_Sector=gdrom_get_cdda;
	aica_info.aica_ram=aica_ram;

	lcp_name=libAICA.Name;
	if (s32 rv = libAICA.Init(&aica_info))
		return rv;
	libAICA.Inited=true;

	ext_device_init_params ext_device_info;
	ext_device_info.RaiseInterrupt=sh4_cpu->RaiseInterrupt;
	ext_device_info.SB_ISTEXT=&SB_ISTEXT;

	lcp_name=libExtDevice.Name;
	if (s32 rv = libExtDevice.Init(&ext_device_info))
		return rv;
	libExtDevice.Inited=true;

	//Init Created maple devices
	for ( int i=0;i<4;i++)
	{
		if (MapleDevices_dd[i][5].Created)
		{
			//init 
			MapleDevices_dd[i][5].Inited=true;
		}
		else
			MapleDevices_dd[i][5].Inited=false;
	}

	return rv_ok;
}


bool plugins_Init()
{
	__try 
	{
		if (s32 rv = plugins_Init_())
		{
			if (rv==rv_error)
			{
				msgboxf("Failed to init %s",MB_ICONERROR,lcp_name);				
			}
			plugins_Term();
			return false;
		}
		return true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		msgboxf("Unhandled exeption while Initing %s plugin",MB_ICONERROR,lcp_name);
		plugins_Term();
		return false;
	}
}
template<typename T>
void term_plugin(T& pl)
{
	if (pl.Inited)
	{
		pl.Inited=false;
		pl.Term();
	}
}
void plugins_Term()
{
	if (!plugins_inited)
		return;
	plugins_inited=false;

	//Term inited maple devices
	for ( int i=4;i>=0;i--)
	{
		if (MapleDevices_dd[i][5].Inited)
		{ 
			MapleDevices_dd[i][5].Inited=false;
			//term
		}
	}
	
	//term all plugins
	term_plugin(libExtDevice);
	term_plugin(libAICA);
	term_plugin(libGDR);
	term_plugin(libPvr);
}

void plugins_Reset(bool Manual)
{
	libPvr.Reset(Manual);
	libGDR.Reset(Manual);
	libAICA.Reset(Manual);
	libExtDevice.Reset(Manual);
}