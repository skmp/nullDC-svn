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
List<nullDC_Maple_plugin>	libMaple;
nullDC_ExtDevice_plugin		libExtDevice;
/*
struct
{
	nullDC_plugin* plugin;
	char cfg_name[512];
} plugin_cfg_list[] = 
{
	{&libPvr,"Current_PVR"},
	{&libGDR,"Current_GDR"},
	{&libAICA,"Current_AICA"},
	{&libExtDevice,"Current_ExtDevice"},
	{0,""}
};
*/
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
	dcGetPluginInfoFP* dcGetPluginInfo=(dcGetPluginInfoFP*)lib.GetProcAddress("dcGetPluginInfo");
	dcGetPluginFP* dcGetPlugin=(dcGetPluginFP*)lib.GetProcAddress("dcGetPlugin");
	
	if (dcGetPluginInfo==0)
		return false;
	if (dcGetPlugin==0)
		return false;

	//Make sure i/f version is correct & rest info is valid
	plugin_info info;
	
	dcGetPluginInfo(&info);
	if (info.InterfaceVersion!=PLUGIN_I_F_VERSION)
		return false;

	if (info.count<1)
		return false;

	//Get plugin list
	plugin_info_entry temp;

	//Make sure plugin i/f version is valid
	for (u32 i=0;i<info.count;i++)
	{
		if (!dcGetPlugin(i,&temp))
			continue;
		switch(temp.common.Type)
		{
		case PowerVR:
			if (temp.common.InterfaceVersion!=PVR_PLUGIN_I_F_VERSION)
				continue;
			break;

		case GDRom:
			if (temp.common.InterfaceVersion!=GDR_PLUGIN_I_F_VERSION)
				continue;
			break;

		case AICA:
			if (temp.common.InterfaceVersion!=AICA_PLUGIN_I_F_VERSION)
				continue;
			break;

		case Maple:
			if (temp.common.InterfaceVersion!=MAPLE_PLUGIN_I_F_VERSION)
				continue;
			break;

		case MapleSub:
			if (temp.common.InterfaceVersion!=MAPLE_PLUGIN_I_F_VERSION)
				continue;
			break;

		case ExtDevice:
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
void Split(char* in,char* dll,int& id)
{
	strcpy(dll,in);
	char *first = strtok(dll, ":"); 
	char *second = strtok(NULL, "\0"); 
	//strcpy(dll,first);
	id=atoi (second);
}

bool nullDC_plugin::Open(char* plugin)
{
	Inited=false;
	Loaded=false;

	if (!strcmp(plugin,"NULL"))
	{
		return false;
	}
	strcpy(dll_file,plugins_path);
	
	char ttt[512];
	Split(plugin,ttt,id);
	
	strcat(dll_file,ttt);
	if (!dll.Load(dll_file))
		return false;
	dcGetPluginInfoFP* getinfo=(dcGetPluginInfoFP*)dll.GetProcAddress("dcGetPluginInfo");
	dcGetPluginFP* getplugin=(dcGetPluginFP*)dll.GetProcAddress("dcGetPlugin");

	
	if (getinfo==0 || getplugin==0)
	{
		dll.Unload();
		return false;
	}

	plugin_info t;
	getinfo(&t);
	if (t.InterfaceVersion!=PLUGIN_I_F_VERSION)
	{
		dll.Unload();
		return false;
	}

	plugin_info_entry t2;
	memset(&t2,0,sizeof(t2));
	if (!getplugin(id,&t2))
	{
		dll.Unload();
		return false;
	}

	if (!LoadI(&t2))
	{
		dll.Unload();
		return false;
	}
	return true;
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
bool nullDC_PowerVR_plugin::LoadI(plugin_info_entry* t)
{
	if (t->common.InterfaceVersion!=PVR_PLUGIN_I_F_VERSION)
		return false;

	common_info* p1= this;
	pvr_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->pvr,sizeof(*p2));

	return true;
}

bool nullDC_GDRom_plugin::LoadI(plugin_info_entry* t)
{
	if (t->common.InterfaceVersion!=GDR_PLUGIN_I_F_VERSION)
		return false;

	common_info* p1= this;
	gdr_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->gdr,sizeof(*p2));

	return true;
}

bool nullDC_AICA_plugin::LoadI(plugin_info_entry* t)
{
	if (t->common.InterfaceVersion!=AICA_PLUGIN_I_F_VERSION)
		return false;

	common_info* p1= this;
	aica_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->aica,sizeof(*p2));

	return true;
}

bool nullDC_Maple_plugin::LoadI(plugin_info_entry* t)
{
	if (t->common.InterfaceVersion!=MAPLE_PLUGIN_I_F_VERSION)
		return false;

	common_info* p1= this;
	maple_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->maple,sizeof(*p2));

	return true;
}

bool nullDC_Maple_Sub_plugin::LoadI(plugin_info_entry* t)
{
	if (t->common.InterfaceVersion!=MAPLE_PLUGIN_I_F_VERSION)
		return false;
	
	common_info* p1= this;
	maple_sub_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->maple_sub,sizeof(*p2));

	return true;
}


bool nullDC_ExtDevice_plugin::LoadI(plugin_info_entry* t)
{
	if (t->common.InterfaceVersion!=EXTDEVICE_PLUGIN_I_F_VERSION)
		return false;

	common_info* p1= this;
	ext_device_plugin_if* p2= this;
	memcpy(p1,&t->common,sizeof(*p1));
	memcpy(p2,&t->ext_dev,sizeof(*p2));

	return true;
}

//Class nullDC_plugin
//must be implemented for each inherited class
/*
PluginLoadError nullDC_plugin::PluginExLoad()
{
	Loaded=true;
	return PluginLoadError::NoError;
}


nullDC_plugin::nullDC_plugin()
{
	dcGetPluginInfo=0;
	Loaded=false;
}

nullDC_plugin::~nullDC_plugin()
{
	if (Loaded)
	{
		if (lib.IsLoaded())
		{
			#ifdef DEBUG_DLL
			EMUWARN("~nullDC_plugin()  : Plugin still loaded , unloading it");
			#endif
			lib.Unload();
		}
		else
		{
			EMUERROR("~nullDC_plugin() : Plugin still loaded , lib.loaded()=false");
		}
	}
}

//plugin must be in dll:index format
//dll:-1 , and :-1 are both invalid
/*
PluginLoadError nullDC_plugin::Load(char* plugin)
{
	if (!lib.Load(plugin))
	{
		//retry w/ path
		char temp[1024];
		strcpy(temp,plugins_path);
		strcat(temp,plugin);

		if (!lib.Load(temp))
			return PluginLoadError::DllLoadError;
	}

	dcGetPluginInfo=(dcGetPluginInfoFP*)lib.GetProcAddress("dcGetPluginInfo");

	if (!dcGetPluginInfo)
	{
		lib.Unload();
		return PluginLoadError::PluginInterfaceMissing;
	}

	memset(&info,0,sizeof(info));
	

	dcGetPluginInfo(&info);
	
	if (info.InterfaceVersion!=PLUGIN_I_F_VERSION)
	{
		lib.Unload();
		return PluginLoadError::PluginInterfaceVersionError;
	}
	
	//W00t , plugin loaded so far , call implementation speciacific loads [if any]
//	PluginLoadError rv= PluginExLoad();
	
	//If error on ExLoad free lib
	//if (rv!=PluginLoadError::NoError)
	//	lib.Unload();

	strcpy(dll,plugin);
	return NoError;
}
void nullDC_plugin::Unload()
{
	if (Loaded)
	{
		Loaded=false;
		dcGetPluginInfo=0;
		lib.Unload();
	}
}



*/
//End nullDC_plugin

/*
//Speciacific plugins
//PowerVR
//Class nullDC_PowerVR_plugin
PluginLoadError nullDC_PowerVR_plugin::PluginExLoad()
{
	dcGetPvrInfo=(dcGetPvrInfoFP*)lib.GetProcAddress("dcGetPvrInfo");

	if (!dcGetPvrInfo)
		return PluginLoadError::PluginInterfaceExMissing;

	dcGetPvrInfo(&pvr_info);

	if (pvr_info.InterfaceVersion.full!=PVR_PLUGIN_I_F_VERSION)
		return PluginLoadError::PluginInterfaceExVersionError;
	
	//All ok !
	Loaded=true;
	return PluginLoadError::NoError;
}


nullDC_PowerVR_plugin::nullDC_PowerVR_plugin():nullDC_plugin()
{
	dcGetPvrInfo=0;
}
nullDC_PowerVR_plugin::~nullDC_PowerVR_plugin()
{
}

//End nullDC_PowerVR_plugin

//Class nullDC_GDRom_plugin
PluginLoadError nullDC_GDRom_plugin::PluginExLoad()
{
	dcGetGDRInfo=(dcGetGDRInfoFP*)lib.GetProcAddress("dcGetGDRInfo");

	if (!dcGetGDRInfo)
		return PluginLoadError::PluginInterfaceExMissing;

	dcGetGDRInfo(&gdr_info);

	if (gdr_info.InterfaceVersion.full!=GDR_PLUGIN_I_F_VERSION)
		return PluginLoadError::PluginInterfaceExVersionError;
	
	//All ok !
	Loaded=true;
	return PluginLoadError::NoError;
}


nullDC_GDRom_plugin::nullDC_GDRom_plugin():nullDC_plugin()
{
	dcGetGDRInfo=0;
}
nullDC_GDRom_plugin::~nullDC_GDRom_plugin()
{
}

//End nullDC_GDRom_plugin

//Class nullDC_AICA_plugin
PluginLoadError nullDC_AICA_plugin::PluginExLoad()
{
	dcGetAICAInfo=(dcGetAICAInfoFP*)lib.GetProcAddress("dcGetAICAInfo");

	if (!dcGetAICAInfo)
		return PluginLoadError::PluginInterfaceExMissing;

	dcGetAICAInfo(&aica_info);

	if (aica_info.InterfaceVersion.full!=AICA_PLUGIN_I_F_VERSION)
		return PluginLoadError::PluginInterfaceExVersionError;
	
	//All ok !
	Loaded=true;
	return PluginLoadError::NoError;
}


nullDC_AICA_plugin::nullDC_AICA_plugin():nullDC_plugin()
{
	dcGetAICAInfo=0;
}
nullDC_AICA_plugin::~nullDC_AICA_plugin()
{
}
//End nullDC_AICA_plugin

//Class nullDC_Maple_plugin
PluginLoadError nullDC_Maple_plugin::PluginExLoad()
{
	dcGetMapleInfo=(dcGetMapleInfoFP*)lib.GetProcAddress("dcGetMapleInfo");

	if (!dcGetMapleInfo)
		return PluginLoadError::PluginInterfaceExMissing;

	dcGetMapleInfo(&maple_info);

	if (maple_info.InterfaceVersion.full!=MAPLE_PLUGIN_I_F_VERSION)
		return PluginLoadError::PluginInterfaceExVersionError;
	
	//All ok !
	Loaded=true;
	return PluginLoadError::NoError;
}


nullDC_Maple_plugin::nullDC_Maple_plugin():nullDC_plugin()
{
	dcGetMapleInfo=0;
}
nullDC_Maple_plugin::~nullDC_Maple_plugin()
{
}
//End nullDC_Maple_plugin

//Class nullDC_ExtDevice_plugin
PluginLoadError nullDC_ExtDevice_plugin::PluginExLoad()
{
	dcGetExtDeviceInfo=(dcGetExtDeviceInfoFP*)lib.GetProcAddress("dcGetExtDeviceInfo");

	if (!dcGetExtDeviceInfo)
		return PluginLoadError::PluginInterfaceExMissing;

	dcGetExtDeviceInfo(&ext_device_info);

	if (ext_device_info.InterfaceVersion.full!=EXTDEVICE_PLUGIN_I_F_VERSION)
		return PluginLoadError::PluginInterfaceExVersionError;
	
	//All ok !
	Loaded=true;
	return PluginLoadError::NoError;
}


nullDC_ExtDevice_plugin::nullDC_ExtDevice_plugin():nullDC_plugin()
{
	dcGetExtDeviceInfo=0;
}
nullDC_ExtDevice_plugin::~nullDC_ExtDevice_plugin()
{
}
//End nullDC_AICA_plugin
*/

//Plguin loading shit

void plugin_FileIsFound(char* file,void* param)
{
	//List<plugin_info_entry> temp_list;
	
	char dllfile[1024]="";
	strcat(dllfile,plugins_path);
	strcat(dllfile,file);

	GetPluginList(dllfile,(List<PluginLoadInfo>*)param);
	/*size_t len=strlen(file);

	for (size_t i=0;i<temp_list.size();i++)
	{
		PluginLoadInfo temp;
		
		temp.InterfaceVersion.full=temp_list[i].common.InterfaceVersion;
		temp.PluginVersion.full=temp_list[i].common.PluginVersion;
		temp.Type=(PluginType)temp_list[i].common.Type;

		strcpy(temp.Name,temp_list[i].common.Name);
		
		strcpy(temp.dll,file);
		sprintf(&temp.dll[len],":%d",i);
		b->l->Add(temp);
	}*/

/*
	nullDC_plugin plg;
	

	if (plg.Load(dllfile)!=PluginLoadError::NoError)
	{
		plg.Unload();		//prop not needed but call it anyway
	}
	else
	{
		PluginLoadInfo pli;
		if (plg.info.Type & b->typemask)
		{
			pli.plugin_info=plg.info;//copy em :P
			strcpy(pli.dll,dllfile);//copy it :P
			b->l->Add(pli);
		}

		plg.Unload();
	}*/
}

List<PluginLoadInfo>* PluginList_cached;
//Nos this function will cache the plugin list instead of loading dll's over and over :)
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
s32 plugins_Init_()
{
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
	
	maple_init_params maple_info;
	for (u32 i=0;i<libMaple.size();i++)
	{
		lcp_name=libMaple[i].Name;
		if (s32 rv = libMaple[i].Init(&maple_info))
			return rv;
		libMaple[i].Inited=true;
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
		for ( int j=0;j<5;j++)
		{
			MapleDevices[i].subdevices[j].connected=false;
		}
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
		libMaple[i-1].Term();
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


bool plugins_Config()
{
	if (plugins_inited)
	{
		MessageBox(NULL,"Emulation is started , plugins can't be changed now ..","nullDC " VER_STRING,MB_OK | MB_ICONERROR | MB_TASKMODAL);
		return false;
	}
	else
	{
		return SelectPluginsGui();
	}
}

bool plugins_load_a(char* cfg_name,nullDC_plugin* plg)
{
	char dllf[512];
	dllf[0]=0;
	cfgLoadStr("nullDC_plugins",cfg_name,dllf);
	
	return plg->Open(dllf);
}
#define load_plugin(name,plug) \
	lcp_name=plug.Name;\
	if (!plugins_load_a(name,& plug))\
		return rv_error;\
	if (s32 rv = plug.Load(&eminf))\
			return rv;\
		plug.Loaded=true;

s32 plugins_Load_()
{
	emu_info eminf;
	eminf.ConfigLoadStr=cfgLoadStr;
	eminf.ConfigSaveStr=cfgSaveStr;
	strcpy(eminf.Name,"nullDC " VER_STRING);
	eminf.WindowHandle=GetRenderTargetHandle();

	
	load_plugin("Current_PVR",libPvr);
	load_plugin("Current_GDR",libGDR);
	load_plugin("Current_AICA",libAICA);
	load_plugin("Current_ExtDevice",libExtDevice);
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
				msgboxf("Unable to load %s plugin",MB_ICONERROR,lcp_name);
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
#define unload_plugin(plg) \
	lcp_name=plg.Name;\
	if (plg.IsOpened()) \
	{\
		if (plg.Loaded) \
			plg.Unload(); \
		plg.Close(); \
	}
void plugins_Unload()
{
	__try 
	{
		unload_plugin(libExtDevice);
		unload_plugin(libAICA);
		unload_plugin(libGDR);
		unload_plugin(libPvr);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		msgboxf("Unhandled exeption while unloading %s\n",MB_ICONERROR,lcp_name);
	}
}