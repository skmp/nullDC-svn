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
enum PluginValidationErrors
{
	pve_invalid_pointer=-200,
	pve_invalid_cif_ver=-201,
	pve_invalid_sif_ver=-202,
	pve_missing_pointers=-203,
	pve_invalid_plugin_type=-204,
};
//Basic plugin interface
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
bool AddToPluginList(char* dll,List<PluginLoadInfo>* to)
{
	cDllHandler lib;

	//load dll
	if (!lib.Load(dll))
		return false;
	
	size_t dll_len=strlen(dll);

	//Get functions
	dcGetInterfaceFP* dcGetInterface=(dcGetInterfaceFP*)lib.GetProcAddress("dcGetInterface");

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
	strcpy(load_info.dll,dll);

	//if correct ver , add em to plugins list :)
	to->Add(load_info);

	lib.Unload();
	return true;
}
//Handles generic loading of plugin
bool Split_Dll_Name(char* in,char* dll,int& id)
{
	if (in==0 || dll==0)
		return false;
	strcpy(dll,in);
	
	size_t slen=strlen(dll);
	
	char* numpos=0;
	for (size_t i=0;i<slen;i++)
	{
		if (dll[i]==':')
		{
			dll[i]=0;
			if ((i+1)<slen)
				numpos=&dll[i+1];
			break;
		}
	}
	if (!numpos)
		return false;
	id=atoi (numpos);
	return true;
}

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

	strcpy(ttt,plugins_path);
	strcat(ttt,dll_file);
	strcpy(dll_file,plugin);
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

//0xbeef
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



//Enumeration stuff :)
void plugin_FileIsFound(char* file,void* param)
{
	char dllfile[1024]="";
	strcat(dllfile,plugins_path);
	strcat(dllfile,file);
	AddToPluginList(dllfile,(List<PluginLoadInfo>*)param);
}

List<PluginLoadInfo>* PluginList_cached;
//Now this function will cache the plugin list instead of loading dll's over and over :)
//Get a list of all plugins that exist on plugin directory and can be loaded , and plguin_type & typemask !=0
List<PluginLoadInfo>* GetPluginList(PluginType type)
{
	List<PluginLoadInfo>* rv=new List<PluginLoadInfo>();

	for (u32 i=0;i<PluginList_cached->itemcount;i++)
	{
		PluginLoadInfo* t=&(*PluginList_cached)[i];
		if (t->Type == type) 
			rv->Add(*t);
	}

	return rv;
}
void EnumeratePlugins()
{
	if (strlen(plugins_path)==0)
		strcpy(plugins_path,"plugins\\");

	if (PluginList_cached!=0)
		delete PluginList_cached;
	
	
	PluginList_cached = new List<PluginLoadInfo>();

	char dllfile[1024]="";
	strcat(dllfile,plugins_path);
	strcat(dllfile,"*.dll");
	FindAllFiles(plugin_FileIsFound,dllfile,PluginList_cached);

	//maple_plugins_enum_devices();
}

void SetPluginPath(char* path)
{
	strcpy(plugins_path,path);

	printf("Plugin path : \"%s\"\n",plugins_path);

	EnumeratePlugins();
}
#include "dc\mem\sb.h"

//Loading & stuff
char* lcp_name;
s32 lcp_error=0;
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
u32 GetMaplePort(u32 port,u32 device)
{
	u32 rv=port<<6;

	rv|=1<<device;
	return rv;
}
s32 maple_plugins_add(char* dll)
{
	if (strcmp(dll,"NULL")==0)
		return rv_ok;
	for (u32 i=0;i<libMaple.size();i++)
	{
		if ((strcmp(libMaple[i]->dll_file,dll)==0))
		{
			libMaple[i]->ReferenceCount++;
			return rv_ok;
		}
	}
	nullDC_Maple_plugin* plug=new nullDC_Maple_plugin();
	plug->Loaded=false;

	if (s32 rv=load_plugin(dll,plug,Maple_menu))
	{
		delete plug;
		return rv;
	}

	libMaple.Add(plug);

	return rv_ok;
}
nullDC_Maple_plugin* FindMaplePlugin(char* dll)
{
	for (u32 i=0;i<libMaple.size();i++)
	{
		if (strcmp(libMaple[i]->dll_file,dll)==0)
			return libMaple[i];
	}
	return 0;
}
s32 maple_plugins_remove(char* dll,char* plugin)
{
	nullDC_Maple_plugin* plg=FindMaplePlugin(dll);
	if (!plg)
		return rv_error;

	plg->ReferenceCount--;

	if (plg->ReferenceCount!=0)
		return rv_ok;
#define LOAD_ERR delete plug;
	unload_plugin(plg,Maple_menu);
#undef LOAD_ERR

	for(size_t i=0;i<libMaple.size();i++)
	{
		if (libMaple[i]==plg)
		{
			libMaple.erase(libMaple.begin()+i);
			return rv_ok;
		}
	}

	return rv_error;
}
enum pmd_errors
{
	pmde_device_state=-250,		//device allready connected/disconected
	pmde_invalid_pos=-251,		//invalid device pos (0~3 , 0~4 are valid only)
	pmde_failed_create=-252,	//failed to create device
	pmde_failed_create_s=-253,	//failed to create device , silent error

};
s32 AttachMapleDevice(u32 pos,char* device,bool hotplug)
{
	if (pos>3)
		return pmde_invalid_pos;

	if (MapleDevices[pos].connected)
		return pmde_device_state;

	char dll[512];
	u32 id;

	if (s32 rv=maple_plugins_add(dll))
	{
		return rv;
	}

	nullDC_Maple_plugin* plg =FindMaplePlugin(dll);

	if (s32 rv= plg->CreateMain(&MapleDevices[pos],id,GetMaplePort(pos,5),hotplug?MDCF_Hotplug:MDCF_None))
	{
		if (rv==rv_error)
			return pmde_failed_create;
		else
			return pmde_failed_create_s;
	}

	return rv_ok;
}
s32 DetachMapleDevice(u32 pos,char* device)
{
	if (pos>3)
		return pmde_invalid_pos;

	if (!MapleDevices[pos].connected)
		return pmde_device_state;

	char dll[512];
	u32 id;

	//for (u32 i=0;i<4;i++)
	//	AttachMapleSubDevice(pos,i, &MapleDevices[pos].subdevices[i]);

	nullDC_Maple_plugin* plg =FindMaplePlugin(dll);
	verify(plg!=0);
	plg->DestroyMain(&MapleDevices[pos]);
}
s32 AttachMapleSubDevice(u32 pos,u32 subport,char* device)
{
	return 0;
}
s32 DetachMapleSubDevice(u32 pos,u32 subport,char* device)
{
return 0;
}
/*
s32 maple_plugins_create_list(emu_info* info)
{
	char plugin[512];
	char dll[512];

	plugin[0]=0;

	for (int i=0;i<4;i++)
	{
		for (int j=0;j<6;j++)
		{
			maple_cfg_name(i,j,plugin);
			maple_cfg_plug(i,j,dll);
			if (s32 rv=maple_plugins_add(dll,plugin,*info,libMaple))
			{
				return rv;
			}
			plugin[0]=0;
		}
	}
	return rv_ok;
}
*/
s32 plugins_Init_()
{
	if (plugins_inited)
		return rv_ok;
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
	
	maple_init_params maple_info;
	for (u32 i=0;i<libMaple.size();i++)
	{
		lcp_name=libMaple[i]->Name;
		if (s32 rv = libMaple[i]->Init(&maple_info))
			return rv;
		libMaple[i]->Inited=true;
	}

	ext_device_init_params ext_device_info;
	ext_device_info.RaiseInterrupt=sh4_cpu->RaiseInterrupt;
	ext_device_info.SB_ISTEXT=&SB_ISTEXT;

	lcp_name=libExtDevice.Name;
	if (s32 rv = libExtDevice.Init(&ext_device_info))
		return rv;
	libExtDevice.Inited=true;


	/*
	for ( int i=0;i<4;i++)
	{
		MapleDevices[i].connected=false;
		nullDC_Maple_plugin* plug_m= FindMaplePlugin(i,5,libMaple);
		if (!plug_m)
			continue;		
		u32 bitmask=plug_m->subdev_info;
		for ( int j=0;j<5;j++)
		{
			if ((bitmask & (1<<j))==0)
			{
				nullDC_Maple_Sub_plugin* plug= FindMaplePlugin(i,j,libMapleSub);
				if (!plug)
					continue;
				MapleDevices[i].subdevices[j].port=GetMaplePort(i,j);
				plug->Create(&MapleDevices[i].subdevices[j],GetMaplePort(i,j),MSCF_None);
				MapleDevices[i].subdevices[j].connected=true;
			}
			else
			{
				MapleDevices[i].subdevices[j].connected=false;
			}
		}
		lcp_name=plug_m->Name;
		MapleDevices[i].port=GetMaplePort(i,5);
		plug_m->Create(&MapleDevices[i],GetMaplePort(i,5),MSCF_None);
		MapleDevices[i].connected=true;
	}
	*/

	plugins_inited=true;
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
void plugins_Term()
{
	//destroy maple devices 

	//Term all plugins
	libExtDevice.Term();


	for (size_t i=libMaple.size();i>0;i--)
	{
		libMaple[i-1]->Term();
	}

	libAICA.Term();
	libGDR.Term();
	libPvr.Term();
}

void plugins_Reset(bool Manual)
{
	
	libPvr.Reset(Manual);
	libGDR.Reset(Manual);
	libAICA.Reset(Manual);
	libExtDevice.Reset(Manual);
	
}


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


#define load_plugin_(cfg_name,to,menu) \
{ \
	char dllf[512]; \
	dllf[0]=0; \
	cfgLoadStr("nullDC_plugins",cfg_name,dllf,"NULL"); \
	if (s32 rv=load_plugin(dllf,to,menu)) \
		return rv; \
}

s32 plugins_Load_()
{
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

	/*
	if (s32 rv = maple_plugins_create_list(&eminf))
	{
		return rv;
	}
	*/
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
		unload_plugin(&libExtDevice,ExtDev_menu);
		unload_plugin(&libAICA,Aica_menu);
		unload_plugin(&libGDR,GDRom_menu);
		unload_plugin(&libPvr,PowerVR_menu);

		for (size_t i=libMaple.size();i>0;i--)
		{
			unload_plugin(libMaple[i-1],Maple_menu);
		}
		libMaple.clear();
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		msgboxf("Unhandled exeption while unloading %s\n",MB_ICONERROR,lcp_name);
	}
}