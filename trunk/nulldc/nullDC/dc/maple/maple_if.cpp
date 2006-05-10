#include "types.h"
#include <string.h>

#include "maple_if.h"

#include "config/config.h"

#include "dc/sh4/intc.h"
#include "dc/mem/sb.h"
#include "dc/mem/sh4_mem.h"
#include "plugins/plugin_manager.h"
#include "dc/sh4/rec_v1/rec_v1_blockmanager.h"

maple_device_instance MapleDevices[4][6];

/*
	Maple ;)
	Maple IO is done on fames on both ways, in a very strict way

	Frame structure :

	input/output buffer:
	->buffer start<-
	Header
	Frame data
	Header
	Frame data
	Last Header [End bit set]
	Frame data
	....
	->buffer end<-

	Transfer info:
	Transfers are done w/ dma.
	Maple is a bidirectional bus , each device has an address

	Address format :
	7-6         |5				 |4			    |3			   |2			 |1				 |0				
	Port number |Main peripheral |Sub-periph. 5 |Sub-periph. 4 |Sub-periph. 3| Sub-periph. 2 |Sub-periph. 1 
	
	if bits 5-0 are 0 , the device is the port
	afaik , olny one or none of bits 5 to 0 can be set when a command is send
	on a resonce , the controller sets the bits of the connected subdevices to it :)

	Now , how to warp all that on a nice to use interface :
	Each dll can contain 1 maple device , and 1 maple subdevice (since they are considered diferent plugin types)
	Maple plugins should olny care about "user data" on maple frames , all else is handled by maple rooting code
*/

/*
typedef void MapleGotData(u32 header1,u32 header2,u32*data,u32 datalen);

struct MaplePluginInfo
{
	u32 InterfaceVersion;

	//UpdateCBFP* UpdateMaple;
	bool Connected;//:)
	MapleGotData* GotDataCB;
	InitFP* Init;
	TermFP* Term;
};*/

//MaplePluginInfo MaplePlugin[4][6];

void DoMapleDma();


void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen);


void maple_SB_MDST_Write(u32 data)
{
	if (data & 0x1)
	{
		SB_MDST=1;
		DoMapleDma();	
	}
}
u32 GetSubDeviceMask(u32 port)
{
	u32 rv=0;
	if (MapleDevices[port][0].Connected==false)
		return 0;

	for (int i=0;i<5;i++)
	{
		if (MapleDevices[port][i+1].Connected)
			rv|=(1<<i);
	}
	return rv;
}

bool IsOnSh4Ram(u32 addr)
{
	if (((addr>>26)&0x7)==3)
	{
		if ((((addr>>29) &0x7)!=7))
		{
			return true;
		}
	}

	return false;
}
u32 GetMaplePort(u32 addr)
{
	for (int i=0;i<6;i++)
	{
		if ((1<<i)&addr)
			return i==5?0:1+i;
	}
}
u32 GetConnectedDevices(u32 Port)
{
	u32 rv=0;
	if(MapleDevices[Port][1].Connected)
		rv|=0x01;
	if(MapleDevices[Port][2].Connected)
		rv|=0x02;
	if(MapleDevices[Port][3].Connected)
		rv|=0x04;
	if(MapleDevices[Port][4].Connected)
		rv|=0x08;
	if(MapleDevices[Port][5].Connected)
		rv|=0x10;
	return rv;
}
void DoMapleDma()
{
#if debug_maple
	printf("Maple :DoMapleDma\n");
#endif
	u32 addr = SB_MDSTAR;	//*MAPLE_DMAADDR;
	bool last = false;
	while (last != true)
	{
		u32 header_1 = ReadMem32(addr);
		u32 header_2 = ReadMem32(addr + 4) &0x1FFFFFE0;

		last = (header_1 >> 31) == 1;//is last transfer ?
		u32 plen = (header_1 & 0xFF )+1;//transfer lenght
		u32 device = (header_1 >> 16) & 0x3;

		if (!IsOnSh4Ram(header_2))
		{
			printf("MAPLE ERROR : DESTINATION NOT ON SH4 RAM\n");
			return;//a baaddd error
		}
		u32* p_out=(u32*)GetMemPtr(header_2,4);
		u32 outlen=0;

		u32* p_data =(u32*) GetMemPtr(addr + 8,(plen)*sizeof(u32));
		//Command / Response code 
		//Recipient address 
		//Sender address 
		//Number of additional words in frame 
		u32 command=p_data[0] &0xFF;
		u32 reci=(p_data[0] >> 8) & 0xFF;//0-5;
		u32 subport=GetMaplePort(reci);
		u32 send=(p_data[0] >> 16) & 0xFF;
		u32 inlen=(p_data[0]>>24) & 0xFF;
		u32 resp=0;
		inlen*=4;

		if (MapleDevices[device][0].Connected && MapleDevices[device][subport].Connected)
		{
			//MaplePlugin[device][0].GotDataCB(header_1,header_2,p_data,plen);
			//libMapleMain[device]->SendFrame(command,&p_data[0],inlen,&p_out[1],outlen,retv);
			//(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce);
			MapleDevices[device][subport].MapleDeviceDMA(
				&MapleDevices[device][subport],
				command,
				&p_data[1],
				inlen-4,
				&p_out[1],
				outlen,
				resp);
			
			
			if(reci&0x20)
				reci|=GetConnectedDevices((reci>>6)&3);

			p_out[0]=(resp<<0)|(send<<8)|(reci<<16)|((outlen/4)<<24);
			outlen+=4;
		}
		else
		{
			outlen=4;
			p_out[0]=0xFFFFFFFF;
		}
		rec_v1_NotifyMemWrite(header_2,outlen);

		//goto next command
		addr += 2 * 4 + plen * 4;
	}
	SB_MDST = 0;	//MAPLE_STATE = 0;//finished :P
	RaiseInterrupt(holly_MAPLE_DMA);
}

void Split(char* in,char* dll,int& id)
{
	char *first = strtok(in, ":"); 
	char *second = strtok(NULL, "\0"); 
	strcpy(dll,first);
	id=atoi (second);
}
List<MapleDeviceLoadInfo> maple_plugin_devices;
List<MapleDeviceLoadInfo>* GetMapleMainDevices()
{
	List<MapleDeviceLoadInfo>* rv=new List<MapleDeviceLoadInfo>();

	for (u32 i=0;i<maple_plugin_devices.size();i++)
	{
		if (maple_plugin_devices[i].type==0)
		rv->Add(maple_plugin_devices[i]);
	}

	return rv;
}

List<MapleDeviceLoadInfo>* GetMapleSubDevices()
{
	List<MapleDeviceLoadInfo>* rv=new List<MapleDeviceLoadInfo>();

	for (u32 i=0;i<maple_plugin_devices.size();i++)
	{
		if (maple_plugin_devices[i].type==1)
		rv->Add(maple_plugin_devices[i]);
	}

	return rv;
}
void maple_plugins_enum_devices()
{
	maple_plugin_devices.clear();
	List<PluginLoadInfo>* maplepl = EnumeratePlugins(PluginType::MapleDevice);

	for (u32 i=0;i<(*maplepl).size();i++)
	{
		nullDC_Maple_plugin mpi;
		mpi.LoadnullDCPlugin((*maplepl)[i].dll);
		for (int j=0;mpi.maple_info.Devices[j].CreateInstance;j++)
		{
			MapleDeviceLoadInfo dev;

			dev.id=(u8)j;
			strcpy(dev.dll,(*maplepl)[i].dll);

			dev.type=mpi.maple_info.Devices[j].type;
			strcpy(dev.name,mpi.maple_info.Devices[j].name);
			dev.PluginVersion=mpi.info.PluginVersion;
			maple_plugin_devices.Add(dev);
		}
		mpi.Unload();
	}
}

void maple_plugins_add(char* device)
{
	char dll[512];
	int id;
	Split(device,dll,id);
	for (u32 i=0;i<libMaple.size();i++)
	{
		if ((strcmp(libMaple[i]->dll,dll)==0))
			return;
	}
	nullDC_Maple_plugin* t=new nullDC_Maple_plugin();
	t->LoadnullDCPlugin(dll);
	libMaple.Add(t);
}
maple_device* FindMapleDevice(char* device)
{
	char dll[512];
	int id;
	Split(device,dll,id);
	for (int i=0;i<libMaple.size();i++)
	{
		if (strcmp(libMaple[i]->dll,dll)==0)
			return &(libMaple[i]->maple_info.Devices[id]);
	}
	return 0;
}

u32 GetMapleAddress(u32 port,u32 device)
{
	u32 rv=port<<6;
	if (device==0)
		device=5;
	else
		device-=1;
	rv|=1<<device;
}
//after plugin init
void maple_plugins_Init()
{
	char plugin[512];
	char temp[512];

	//Clear all plugin slots
	for (int i=0;i<4;i++)
	{
		for (int j=0;j<6;j++)
		{
			sprintf(temp,"Current_maple%d_%d",i,j);
			cfgLoadStr("nullDC_plugins",temp,plugin);
			if (strcmp(plugin,"NULL")!=0)
			{
				maple_device* plugin_dev=FindMapleDevice(plugin);
				plugin_dev->CreateInstance(plugin_dev,MapleDevices[i][j],(i<<6)|(1<<j));
				MapleDevices[i][j].port=(i<<6)|(1<<j);
				MapleDevices[i][j].Connected=true;
			}
			else
			{
				MapleDevices[i][j].Connected=false;
			}
		}
	}
}
//before plugin termination
void maple_plugins_Term()
{
	char plugin[512];
	char temp[512];

	//Clear all plugin slots
	for (int i=0;i<4;i++)
	{
		for (int j=0;j<6;j++)
		{
			sprintf(temp,"Current_maple%d_%d",i,j);
			cfgLoadStr("nullDC_plugins",temp,plugin);
			if (strcmp(plugin,"NULL")!=0)
			{
				maple_device* plugin_dev=FindMapleDevice(plugin);
				plugin_dev->DestroyInstance(plugin_dev,MapleDevices[i][j]);
				MapleDevices[i][j].Connected=false;
			}
			else
			{
				MapleDevices[i][j].Connected=false;
			}
		}
	}
}
	

void maple_plugins_create_list()
{
	char plugin[512];
	char temp[512];
	
	plugin[0]=0;

	for (int i=0;i<4;i++)
	{
		for (int j=0;j<6;j++)
		{
			//(const char * lpSection, const char * lpKey, char * lpReturn)
			sprintf(temp,"Current_maple%d_%d",i,j);
			cfgLoadStr("nullDC_plugins",temp,plugin);
			if (strcmp(plugin,"NULL")!=0)
				maple_plugins_add(plugin);
			plugin[0]=0;
		}
	}
}

//Maple plugin enumeration
//what is needed here is :
//Enumerate all maple plugin
//Get a list of all maple devices on em , and keep em on name,file;id list-pair (id is a number)
//Give out teh lists when called for em :P
//Also , cache the lists (no need to go trhu all plugins each time)
//This code is called from main plugin enumeration code (so its easy to redo shit on plugin reenumeration)

//On maple init a list of all used maple plugins is created (so it can be used by plugin manager)
//These plugins are loaded (but not inited)
//Maple plugin init is called after plugin manager , so we can atach the varius devices :)
//done BEFORE initilising plugins
void maple_Init()
{
	sb_regs[(SB_MDST_addr-SB_BASE)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[(SB_MDST_addr-SB_BASE)>>2].writeFunction=maple_SB_MDST_Write;
	maple_plugins_create_list();
}

void maple_Reset(bool Manual)
{
}
//On maple term the list of teh created plugins is deleted ;). The plugins are allready unloaded by plugin termination code
//Maple plugin term is called before plugin manager , so we can de-atach the varius devices :)
//done AFTER terminating plugins
void maple_Term()
{
	libMaple.clear();
}