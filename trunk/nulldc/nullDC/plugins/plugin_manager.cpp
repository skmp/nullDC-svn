#include "types.h"
#include "plugin_manager.h"
#include "dc/pvr/pvr_if.h"
#include "dc/gdrom/gdrom_if.h"
#include "gui/base.h"

#include <string.h>

//Change log [kept since 23/3/2006]
//Added support for AICA plugin Load/Init/Reset/Termination


//o.O plugins! :D

//Currently used plugins
nullDC_PowerVR_plugin* libPvr;
nullDC_GDRom_plugin*   libGDR;
nullDC_AICA_plugin*    libAICA;
sh4_if*				   sh4_cpu;
//more to come

char plugins_path[1000]="";
//Basic plugin interface
//dcGetPluginInfo(plugin_info* info);


//Handles generic loading of plugin

//Class nullDC_plugin
//must be implemented for each inherited class
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

PluginLoadError nullDC_plugin::LoadnullDCPlugin(char* plugin)
{
	if (!lib.Load(plugin))
	{
		//retry w/ path
		char temp[1024];
		strcpy(temp,plugins_path);
		strcat(temp,plugin);

		if (!lib.Load(temp))
			return PluginLoadError::DllLoadError;;
	}

	dcGetPluginInfo=(dcGetPluginInfoFP*)lib.GetProcAddress("dcGetPluginInfo");

	if (!dcGetPluginInfo)
	{
		lib.Unload();
		return PluginLoadError::PluginInterfaceMissing;
	}

	dcGetPluginInfo(&info);
	
	if (info.InterfaceVersion.full!=PLUGIN_I_F_VERSION)
	{
		lib.Unload();
		return PluginLoadError::PluginInterfaceVersionError;
	}
	
	//W00t , plugin loaded so far , call implementation speciacific loads [if any]
	PluginLoadError rv= PluginExLoad();
	
	//If error on ExLoad free lib
	if (rv!=PluginLoadError::NoError)
		lib.Unload();

	return rv;
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



//End nullDC_plugin

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

//Plguin loading shit
//temp struct
struct temp_123__2_23{GrowingList<PluginLoadInfo>* l;u32 typemask;};

void plugin_FileIsFound(char* file,void* param)
{
	temp_123__2_23 * b=(temp_123__2_23*)param;

	nullDC_plugin plg;
	char dllfile[1024]="";
	strcat(dllfile,plugins_path);
	strcat(dllfile,file);

	if (plg.LoadnullDCPlugin(dllfile)!=PluginLoadError::NoError)
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
	}
}

GrowingList<PluginLoadInfo>* PluginList_cached;
//Nos this function will cache the plugin list instead of loading dll's over and over :)
//Get a list of all plugins that exist on plugin directory and can be loaded , and plguin_type & typemask !=0
GrowingList<PluginLoadInfo>* EnumeratePlugins(u32 Typemask)
{
	if (strlen(plugins_path)==0)
		strcpy(plugins_path,"plugins\\");

	GrowingList<PluginLoadInfo>* rv=new GrowingList<PluginLoadInfo>();
	if (PluginList_cached==0)
	{
		PluginList_cached = new GrowingList<PluginLoadInfo>();
		temp_123__2_23 fag;
		fag.l=PluginList_cached;
		fag.typemask=0xFFFFFFFF;
		char dllfile[1024]="";
		strcat(dllfile,plugins_path);
		strcat(dllfile,"*.dll");
		FindAllFiles(plugin_FileIsFound,dllfile,&fag);
	}

	for (u32 i=0;i<PluginList_cached->itemcount;i++)
	{
		PluginLoadInfo* t=&(*PluginList_cached)[i];
		if (t->plugin_info.Type & Typemask) 
			rv->Add(t);
	}

	return rv;
}
//Handles Setting/Changing plugin

bool SetPlugin(nullDC_plugin* plugin,PluginType type)
{
	if (plugin->Loaded==false)
	{
		EMUERROR("Trying to load plugin that is not initialised");
		return false;
	}

	switch(type)
	{

		case PluginType::AICA:
			libAICA=(nullDC_AICA_plugin*)plugin;
			break;

		case PluginType::GDRom:
			libGDR=(nullDC_GDRom_plugin*)plugin;
			return true;
			break;

		case PluginType::MapleDeviceMain:
			return false;
			break;

		case PluginType::MapleDeviceSub:
			return false;
			break;

		case PluginType::PowerVR:
			//if previus exists
			libPvr=((nullDC_PowerVR_plugin*)plugin);
			break;

	}

	return false;
}

void SetPluginPath(char* path)
{
	strcpy(plugins_path,path);

	if (PluginList_cached)
	{
		delete PluginList_cached;
		PluginList_cached=0;
	}
	//just re enumerate the cache
	delete EnumeratePlugins(0);
}
#include "dc\mem\sb.h"
void plugins_Init()
{
	pvr_init_params pvr_info;
	pvr_info.RaiseInterrupt=sh4_cpu->RaiseInterrupt;
	pvr_info.vram=&vram[0];
	pvr_info.WindowHandle=GetRenderTargetHandle();
	
	pvr_info.vram_lock_32=vramlock_Lock_32;
	pvr_info.vram_lock_64=vramlock_Lock_64;
	pvr_info.vram_unlock=vramlock_Unlock_block;

	if (libPvr)
	{
		libPvr->info.Init(&pvr_info,PluginType::PowerVR);
	}
	else
	{
		EMUERROR("Error , PowerVR plugin is not loaded");
	}

	gdr_init_params gdr_info;
	gdr_info.DriveNotifyEvent=NotifyEvent_gdrom;
	if (libGDR)
	{
		libGDR->info.Init(&gdr_info,PluginType::GDRom);
	}
	else
	{
		EMUERROR("Error , GDrom plugin is not loaded");
	}

	aica_init_params aica_info;
	aica_info.WindowHandle=GetRenderTargetHandle();
	aica_info.RaiseInterrupt=sh4_cpu->RaiseInterrupt;
	aica_info.SB_ISTEXT=&SB_ISTEXT;
	if (libAICA)
	{
		libAICA->info.Init(&aica_info,PluginType::AICA);
	}
	else
	{
		EMUERROR("Error , AICA/arm7 plugin is not loaded");
	}
}

void plugins_Term()
{
	
	if (libAICA)
	{
		libAICA->info.Term(PluginType::AICA);
	}
	else
	{
		//EMUERROR("Error , AICA/arm7 plugin is not loaded");
	}

	if (libGDR)
	{
		libGDR->info.Term(PluginType::GDRom);
	}
	else
	{
		//EMUERROR("Error , GDrom plugin is not loaded");
	}

	if (libPvr)
	{
		libPvr->info.Term(PluginType::PowerVR);
	}
	else
	{
		//EMUERROR("Error , PowerVR plugin is not loaded");
	}

	delete libAICA;
	libAICA=0;
	delete libGDR;
	libGDR=0;
	delete libPvr;
	libPvr=0;

	if (PluginList_cached)
	{
		delete PluginList_cached;
	}
}

void plugins_Reset(bool Manual)
{
	if (libPvr)
	{
		libPvr->info.Reset(Manual,PluginType::PowerVR);
	}
	else
	{
		EMUERROR("Error , PowerVR plugin is not loaded");
	}

	if (libGDR)
	{
		libGDR->info.Reset(Manual,PluginType::GDRom);
	}
	else
	{
		EMUERROR("Error , GDrom plugin is not loaded");
	}

	if (libAICA)
	{
		libAICA->info.Reset(Manual,PluginType::AICA);
	}
	else
	{
		EMUERROR("Error , AICA/arm7 plugin is not loaded");
	}
}

void plugins_ThreadInit()
{
	if (libPvr)
	{
		libPvr->info.ThreadInit(PluginType::PowerVR);
	}
	else
	{
		EMUERROR("Error , pvr plugin is not loaded");
	}

	if (libGDR)
	{
		libGDR->info.ThreadInit(PluginType::GDRom);
	}
	else
	{
		EMUERROR("Error , gdr plugin is not loaded");
	}

	if (libAICA)
	{
		libAICA->info.ThreadInit(PluginType::AICA);
	}
	else
	{
		EMUERROR("Error , AICA/arm7 plugin is not loaded");
	}
}

void plugins_ThreadTerm()
{
	if (libAICA)
	{
		libAICA->info.ThreadTerm(PluginType::AICA);
	}
	else
	{
		EMUERROR("Error , AICA/arm7 plugin is not loaded");
	}

	if (libGDR)
	{
		libGDR->info.ThreadTerm(PluginType::GDRom);
	}
	else
	{
		EMUERROR("Error , gdr plugin is not loaded");
	}

	if (libPvr)
	{
		libPvr->info.ThreadTerm(PluginType::PowerVR);
	}
	else
	{
		EMUERROR("Error , pvr plugin is not loaded");
	}
}
