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


//Change log [kept since 23/3/2006]
//Added support for AICA plugin Load/Init/Reset/Termination


//o.O plugins! :D

//Currently used plugins
nullDC_PowerVR_plugin		libPvr;
nullDC_GDRom_plugin			libGDR;
nullDC_AICA_plugin			libAICA;
List<nullDC_Maple_plugin*>	libMaple;
List<nullDC_Maple_Sub_plugin*>	libMapleSub;
nullDC_ExtDevice_plugin		libExtDevice;

sh4_if*						sh4_cpu;
//more to come

bool plugins_inited=false;
char plugins_path[1000]="";
//Basic plugin interface

bool GetPluginList(char* dll,List<PluginLoadInfo>* to)
{
	cDllHandler lib;

	//load dll
	if (!lib.Load(dll))
		return false;
	
	size_t dll_len=strlen(dll);

	//Get functions
	dcGetInterfaceInfoFP* dcGetInterfaceInfo=(dcGetInterfaceInfoFP*)lib.GetProcAddress("dcGetInterfaceInfo");
	dcGetInterfaceFP* dcGetInterface=(dcGetInterfaceFP*)lib.GetProcAddress("dcGetInterface");
	
	if (dcGetInterfaceInfo==0)
		return false;
	if (dcGetInterface==0)
		return false;

	//Make sure i/f version is correct & rest info is valid
	plugin_interface_info info;
	
	dcGetInterfaceInfo(&info);
	if (info.InterfaceVersion!=PLUGIN_I_F_VERSION)
		return false;

	if (info.count<1)
		return false;

	//Get plugin list
	plugin_interface temp;

	//Make sure plugin i/f version is valid
	for (u32 i=0;i<info.count;i++)
	{
		dcGetInterface(i,&temp);

		switch(temp.common.Type)
		{
		case Plugin_PowerVR:
			if (temp.common.InterfaceVersion!=PVR_PLUGIN_I_F_VERSION)
				continue;
			break;

		case Plugin_GDRom:
			if (temp.common.InterfaceVersion!=GDR_PLUGIN_I_F_VERSION)
				continue;
			break;

		case Plugin_AICA:
			if (temp.common.InterfaceVersion!=AICA_PLUGIN_I_F_VERSION)
				continue;
			break;

		case Plugin_Maple:
			if (temp.common.InterfaceVersion!=MAPLE_PLUGIN_I_F_VERSION)
				continue;
			break;

		case Plugin_MapleSub:
			if (temp.common.InterfaceVersion!=MAPLE_PLUGIN_I_F_VERSION)
				continue;
			break;

		case Plugin_ExtDevice:
			if (temp.common.InterfaceVersion!=EXTDEVICE_PLUGIN_I_F_VERSION)
				continue;
			break;
		default:
			continue;
		}

	
		PluginLoadInfo load_info;
		
		load_info.InterfaceVersion.full=temp.common.InterfaceVersion;
		load_info.PluginVersion.full=temp.common.PluginVersion;
		load_info.Type=(PluginType)temp.common.Type;

		load_info.subdev_info=temp.maple.subdev_info;
		strcpy(load_info.Name,temp.common.Name);
		strcpy(load_info.dll,dll);
		sprintf(&load_info.dll[dll_len],":%d",i);

		//if correct ver , add em to plugins list :)
		to->Add(load_info);
	}

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
	if (!Split_Dll_Name(plugin,dll_file,id))
		return -103;

	strcpy(ttt,plugins_path);
	strcat(ttt,dll_file);
	strcpy(dll_file,plugin);
	if (!dll.Load(ttt))
		return -104;
	dcGetInterfaceInfoFP* getinfo=(dcGetInterfaceInfoFP*)dll.GetProcAddress("dcGetInterfaceInfo");
	dcGetInterfaceFP* getplugin=(dcGetInterfaceFP*)dll.GetProcAddress("dcGetInterface");

	
	if (getinfo==0 || getplugin==0)
	{
		dll.Unload();
		return -105;
	}

	plugin_interface_info t;
	getinfo(&t);
	if (t.InterfaceVersion!=PLUGIN_I_F_VERSION)
	{
		dll.Unload();
		return -106;
	}
	if (id>=t.count)
	{
		dll.Unload();
		return -108;
	}

	plugin_interface t2;
	memset(&t2,0,sizeof(t2));
	getplugin(id,&t2);

	if (!LoadI(&t2))
	{
		dll.Unload();
		return -107;
	}

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
bool nullDC_PowerVR_plugin::LoadI(plugin_interface* t)
{
	if (t->common.InterfaceVersion!=PVR_PLUGIN_I_F_VERSION)
		return false;

	common_info* p1= this;
	pvr_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->pvr,sizeof(*p2));

	return true;
}

bool nullDC_GDRom_plugin::LoadI(plugin_interface* t)
{
	if (t->common.InterfaceVersion!=GDR_PLUGIN_I_F_VERSION)
		return false;

	common_info* p1= this;
	gdr_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->gdr,sizeof(*p2));

	return true;
}

bool nullDC_AICA_plugin::LoadI(plugin_interface* t)
{
	if (t->common.InterfaceVersion!=AICA_PLUGIN_I_F_VERSION)
		return false;

	common_info* p1= this;
	aica_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->aica,sizeof(*p2));

	return true;
}

bool nullDC_Maple_plugin::LoadI(plugin_interface* t)
{
	if (t->common.InterfaceVersion!=MAPLE_PLUGIN_I_F_VERSION)
		return false;

	common_info* p1= this;
	maple_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->maple,sizeof(*p2));

	return true;
}

bool nullDC_Maple_Sub_plugin::LoadI(plugin_interface* t)
{
	if (t->common.InterfaceVersion!=MAPLE_PLUGIN_I_F_VERSION)
		return false;
	
	common_info* p1= this;
	maple_sub_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->maple_sub,sizeof(*p2));

	return true;
}


bool nullDC_ExtDevice_plugin::LoadI(plugin_interface* t)
{
	if (t->common.InterfaceVersion!=EXTDEVICE_PLUGIN_I_F_VERSION)
		return false;

	common_info* p1= this;
	ext_device_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->ext_dev,sizeof(*p2));

	return true;
}


//Plguin loading shit

void plugin_FileIsFound(char* file,void* param)
{
	char dllfile[1024]="";
	strcat(dllfile,plugins_path);
	strcat(dllfile,file);

	GetPluginList(dllfile,(List<PluginLoadInfo>*)param);
}

List<PluginLoadInfo>* PluginList_cached;
//Now this function will cache the plugin list instead of loading dll's over and over :)
//Get a list of all plugins that exist on plugin directory and can be loaded , and plguin_type & typemask !=0
List<PluginLoadInfo>* EnumeratePlugins(PluginType type)
{
	if (strlen(plugins_path)==0)
		strcpy(plugins_path,"plugins\\");

	List<PluginLoadInfo>* rv=new List<PluginLoadInfo>();
	
	if (PluginList_cached==0)
	{
		PluginList_cached = new List<PluginLoadInfo>();
		
		char dllfile[1024]="";
		strcat(dllfile,plugins_path);
		strcat(dllfile,"*.dll");
		FindAllFiles(plugin_FileIsFound,dllfile,PluginList_cached);
		//maple_plugins_enum_devices();
	}

	for (u32 i=0;i<PluginList_cached->itemcount;i++)
	{
		PluginLoadInfo* t=&(*PluginList_cached)[i];
		if (t->Type == type) 
			rv->Add(*t);
	}

	return rv;
}
//Handles Setting/Changing plugin
/*
bool SetPlugin(char* plugin,PluginType type)
{
	if (plugin==0 || strlen(plugin)==0)
	{
		EMUERROR("Trying to load invalid plugin file");
		return false;
	}

	switch(type)
	{
		case PluginType::ExtDevice:
			libExtDevice.Open(plugin);// =(nullDC_ExtDevice_plugin*)plugin;
			return true;

		case PluginType::AICA:
			libAICA.Open(plugin);//=(nullDC_AICA_plugin*)plugin;
			return true;

		case PluginType::GDRom:
			libGDR.Open(plugin);//=(nullDC_GDRom_plugin*)plugin;
			return true;

		case PluginType::Maple:
			return false;


		case PluginType::PowerVR:
			libPvr.Open(plugin);//=((nullDC_PowerVR_plugin*)plugin);
			return true;

	}

	return false;
}
*/

void SetPluginPath(char* path)
{
	strcpy(plugins_path,path);

	printf("Plugin path : \"%s\"\n",plugins_path);

	if (PluginList_cached)
	{
		delete PluginList_cached;
		PluginList_cached=0;
	}
}
#include "dc\mem\sb.h"
char* lcp_name;
s32 lcp_error=0;
#define load_plugin(name,plug) \
	lcp_error=-100;\
	lcp_name=plug.dll_file;\
	if (!plug.Loaded)\
	{\
		if ((lcp_error=plugins_load_a(name,& plug))!=rv_ok)\
		{\
			LOAD_ERR\
			return rv_error;\
		}\
		if (s32 rv = plug.Load(&eminf))\
		{\
			LOAD_ERR\
			return rv;\
		}\
		plug.Loaded=true;\
		printf("Loaded %s[%s]\n",plug.Name,lcp_name);\
	}
#define unload_plugin(plug) \
	lcp_name=plug.Name;\
	if (plug.IsOpened()) \
	{\
		if (plug.Loaded) \
		{\
			plug.Unload(); \
			plug.Loaded=false;\
			printf("Unloaded %s[%s]\n",plug.Name,lcp_name);\
		}\
		plug.Close(); \
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
template<typename T>
s32 maple_plugins_add(char* dll,char* plugin,emu_info& eminf,List<T*>& list)
{
	if (strcmp(dll,"NULL")==0)
		return rv_ok;
	for (u32 i=0;i<list.size();i++)
	{
		if ((strcmp(list[i]->dll_file,dll)==0))
			return rv_ok;
	}
	T* plug=new T();
	plug->Loaded=false;

#define LOAD_ERR delete plug;
	load_plugin(plugin,(*plug));
/*
	lcp_name=plug->dll_file;
	if ((lcp_error=plug->Open(dll))!=rv_ok)
	{
		return rv_error;
	}
	
	if (s32 rv = plug->Load(info))
	{
		plug->Close();
		delete plug;
		return rv;
	}
	else
	{
		plug->Loaded=true;
	}
	*/
#undef LOAD_ERR

	list.Add(plug);

	return rv_ok;
}
template<typename T>
T* FindMaplePlugin(int i,int j,List<T*>& list)
{
	char dll[512];
	maple_cfg_plug(i,j,dll);
	for (u32 i=0;i<list.size();i++)
	{
		if (strcmp(list[i]->dll_file,dll)==0)
			return list[i];
	}
	return 0;
}

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
			if (strcmp(dll,"NULL")!=0)
			{
				if (j==5)
				{
					if (s32 rv=maple_plugins_add(dll,plugin,*info,libMaple))
					{
						return rv;
					}
				}
				else
				{
					if (s32 rv=maple_plugins_add(dll,plugin,*info,libMapleSub))
					{
						return rv;
					}
				}
			}
			plugin[0]=0;
		}
	}
	return rv_ok;
}
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
				plug->Create(&MapleDevices[i].subdevices[j],GetMaplePort(i,j));
				MapleDevices[i].subdevices[j].connected=true;
			}
			else
			{
				MapleDevices[i].subdevices[j].connected=false;
			}
		}
		lcp_name=plug_m->Name;
		MapleDevices[i].port=GetMaplePort(i,5);
		plug_m->Create(&MapleDevices[i],GetMaplePort(i,5));
		MapleDevices[i].connected=true;
	}

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
	
	if (PluginList_cached)
	{
		delete PluginList_cached;
		PluginList_cached=0;
	}
}

void plugins_Reset(bool Manual)
{
	
	libPvr.Reset(Manual);
	libGDR.Reset(Manual);
	libAICA.Reset(Manual);
	libExtDevice.Reset(Manual);
	
}


bool plugins_Select()
{
	if (plugins_inited)
	{
		if (msgboxf("Emulation is started , plugins can't be changed now ..\r\nWant to Continue anyway ? (Changes will will take effect after retart)",MBX_YESNO | MBX_ICONEXCLAMATION | MBX_TASKMODAL)==MBX_RV_NO)
			return false;
	}

	return SelectPluginsGui();
}

s32 plugins_load_a(char* cfg_name,nullDC_plugin* plg)
{
	char dllf[512];
	dllf[0]=0;
	cfgLoadStr("nullDC_plugins",cfg_name,dllf,"NULL");
	
	return plg->Open(dllf);
}
s32 plugins_Load_()
{
	emu_info eminf;
	eminf.ConfigLoadStr=cfgLoadStr;
	eminf.ConfigSaveStr=cfgSaveStr;
	eminf.ConfigLoadInt=cfgLoadInt;
	eminf.ConfigSaveInt=cfgSaveInt;
	
	eminf.WindowHandle=GetRenderTargetHandle();

	#define LOAD_ERR
	load_plugin("Current_PVR",libPvr);
	load_plugin("Current_GDR",libGDR);
	load_plugin("Current_AICA",libAICA);
	load_plugin("Current_ExtDevice",libExtDevice);
	#undef LOAD_ERR

	if (s32 rv = maple_plugins_create_list(&eminf))
	{
		return rv;
	}
	return rv_ok;
}
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
		return true;
	}
}

void plugins_Unload()
{
	__try 
	{
		unload_plugin(libExtDevice);
		unload_plugin(libAICA);
		unload_plugin(libGDR);
		unload_plugin(libPvr);

		for (size_t i=libMaple.size();i>0;i--)
		{
			unload_plugin((*libMaple[i-1]));
		}
		libMaple.clear();

		for (size_t i=libMapleSub.size();i>0;i--)
		{
			unload_plugin((*libMapleSub[i-1]));
		}
		libMapleSub.clear();
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		msgboxf("Unhandled exeption while unloading %s\n",MB_ICONERROR,lcp_name);
	}
}