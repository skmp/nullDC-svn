#include "nullDC\plugins\plugin_header.h"

emu_info host;

#define _WIN32_WINNT 0x500
#include <windowsx.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#include <ws2tcpip.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <string.h>
#include <commctrl.h>

#define dbgbreak {while(1) __noop;}
#define verify(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#pragma pack(1)

char testJoy_strName_dreameye_1[64] = "Dreamcast Camera Flash  Devic\0";
char testJoy_strName_dreameye_2[64] = "Dreamcast Camera Flash LDevic\0";
char testJoy_strName_mic[64] = "MicDevice for Dreameye\0";
char testJoy_strBrand[64] = "Produced By or Under License From SEGA ENTERPRISES,LTD.\0";

enum MapleFunctionID
{
	MFID_0_Input		= 0x01000000,		//DC Controller, Lightgun buttons, arcade stick .. stuff like that
	MFID_1_Storage		= 0x02000000,		//VMU , VMS
	MFID_2_LCD			= 0x04000000,		//VMU
	MFID_3_Clock		= 0x08000000,		//VMU
	MFID_4_Mic			= 0x10000000,		//DC Mic (, dreameye too ?)
	MFID_5_ARGun		= 0x20000000,		//Artificial Retina gun ? seems like this one was never developed or smth -- i only remember of lightguns
	MFID_6_Keyboard		= 0x40000000,		//DC Keyboard
	MFID_7_LightGun		= 0x80000000,		//DC Lightgun
	MFID_8_Vibration	= 0x00010000,		//Puru Puru
	MFID_9_Mouse		= 0x00020000,		//DC Mouse
	MFID_10_StorageExt	= 0x00040000,		//Storage ? propably never used
	MFID_11_Camera		= 0x00080000,		//DreamEye
};
enum MapleDeviceCommand
{
	MDC_DeviceRequest	=0x01,			//7 words.Note : Initialises device
	MDC_AllStatusReq	=0x02,			//7 words + device depedant ( seems to be 8 words)
	MDC_DeviceReset		=0x03,			//0 words
	MDC_DeviceKill		=0x04,			//0 words
	MDC_DeviceStatus    =0x05,			//Same as MDC_DeviceRequest ?
	MDC_DeviceAllStatus =0x06,			//Same as MDC_AllStatusReq ?

	//Varius Functions
	MDCF_GetCondition=0x09,				//FT
	MDCF_GetMediaInfo=0x0A,				//FT,PT,3 pad
	MDCF_BlockRead   =0x0B,				//FT,PT,Phase,Block #
	MDCF_BlockWrite  =0x0C,				//FT,PT,Phase,Block #,data ...
	MDCF_GetLastError=0x0D,				//FT,PT,Phase,Block #
	MDCF_SetCondition=0x0E,				//FT,data ...
	MDCF_MICControl	 =0x0F,				//FT,MIC data ...
	MDCF_ARGunControl=0x10,				//FT,AR-Gun data ...
};
enum MapleDeviceRV
{
	MDRS_DeviseStatus=0x05,			//28 words
	MDRS_DeviseStatusAll=0x06,		//28 words + device depedant data
	MDRS_DeviceReply=0x07,			//0 words
	MDRS_DataTransfer=0x08,			//FT,depends on the command

	MDRE_UnknownFunction=0xFE,		//0 words
	MDRE_UnknownCmd=0xFD,			//0 words
	MDRE_TransminAgain=0xFC,		//0 words
	MDRE_FileError=0xFB,			//1 word, bitfield
	MDRE_LCDError=0xFA,				//1 word, bitfield
	MDRE_ARGunError=0xF9,			//1 word, bitfield
};

u8 GetBtFromSgn(s8 val)
{
	return val+128;
}

#define w32(data) {*(u32*)buffer_out_b=(data);buffer_out_b+=4;buffer_out_len+=4;}
#define w16(data) {*(u16*)buffer_out_b=(data);buffer_out_b+=2;buffer_out_len+=2;}
#define w8(data) {*(u8*)buffer_out_b=(data);buffer_out_b+=1;buffer_out_len+=1;}

void LoadSettings();
void SaveSettings();

s32 FASTCALL Load(emu_info* emu)
{
	memcpy(&host,emu,sizeof(host));

	LoadSettings();
	return rv_ok;
}

//called when plugin is unloaded by emu
void FASTCALL  Unload()
{

}

HMODULE hModule;
HINSTANCE hInstance;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	::hModule=hModule;
	hInstance=(HINSTANCE)hModule;
    return TRUE;
}



/*
	(06/01/07)[04:09] <BlueCrab> Device info for port A0:
	(06/01/07)[04:09] <BlueCrab> Functions: 01000000
	(06/01/07)[04:09] <BlueCrab> Function Data 0: 00080000
	(06/01/07)[04:09] <BlueCrab> Function Data 1: 00000000
	(06/01/07)[04:09] <BlueCrab> Function Data 2: 00000000
	(06/01/07)[04:09] <BlueCrab> Area code: FF
	(06/01/07)[04:09] <BlueCrab> Connector direction: 00
	(06/01/07)[04:09] <BlueCrab> Product name: Dreamcast Camera Flash  Devic
	(06/01/07)[04:09] <BlueCrab> Product licence: Produced By or Under License From SEGA ENTERPRISES,LTD.    
	(06/01/07)[04:09] <BlueCrab> Standby power: 07D0
	(06/01/07)[04:10] <BlueCrab> Max power: 0960
*/
u32 FASTCALL DreamEye_mainDMA(void* data,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;
	maple_device_instance* device_instance=(maple_device_instance*)data;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			//caps
			//4
			w32(0x01000000);

			//struct data
			//3*4
			w32( 0x00080000); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				if (!testJoy_strName_dreameye_1[i])
				{
					w8(0x20);
				}
				else
				{
					w8((u8)testJoy_strName_dreameye_1[i]);
				}
			}

			//60
			for (u32 i = 0; i < 60; i++)
			{
				if (!testJoy_strBrand[i])
				{
					w8(0x20);
				}
				else
				{
					w8((u8)testJoy_strBrand[i]);
				}
			}

			//2
			w16(0x04FF); 

			//2
			w16(0x0069); 
			return 5;
			/* controller condition structure 
		typedef struct {//8 bytes
		WORD buttons;			///* buttons bitfield	/2
		u8 rtrig;			///* right trigger			/1
		u8 ltrig;			///* left trigger 			/1
		u8 joyx;			////* joystick X 			/1
		u8 joyy;			///* joystick Y				/1
		u8 joy2x;			///* second joystick X 		/1
		u8 joy2y;			///* second joystick Y 		/1
		} cont_cond_t;*/
		case 9:

			//caps
			//4
			w32((1 << 24));
			//struct data
			//2
			w16(0xF7FF); //camera button not pressed
			
			//triger
			//1 R
			w8(0);
			//1 L
			w8(0); 
			//joyx
			//1
			w8(GetBtFromSgn(0));
			//joyy
			//1
			w8(GetBtFromSgn(0));

			//1
			w8(GetBtFromSgn(0)); 
			//1
			w8(GetBtFromSgn(0)); 

			return 8;
		default:
			printf("DreamEye_mainDMA : unknown MAPLE COMMAND %d \n",Command);
			return 7;
	}
}

/*
	(06/01/07)[04:10] <BlueCrab> Device info for port A1:
	(06/01/07)[04:10] <BlueCrab> Functions: 00080000
	(06/01/07)[04:10] <BlueCrab> Function Data 0: 30A800C0
	(06/01/07)[04:10] <BlueCrab> Function Data 1: 00000000
	(06/01/07)[04:10] <BlueCrab> Function Data 2: 00000000
	(06/01/07)[04:10] <BlueCrab> Area code: FF
	(06/01/07)[04:10] <BlueCrab> Connector direction: 00
	(06/01/07)[04:10] <BlueCrab> Product name: Dreamcast Camera Flash LDevic
	(06/01/07)[04:10] <BlueCrab> Product licence: Produced By or Under License From SEGA ENTERPRISES,LTD.    
	(06/01/07)[04:10] <BlueCrab> Standby power: 0000
	(06/01/07)[04:10] <BlueCrab> Max power: 0000
*/
u32 des_9_count=0;
u32 FASTCALL DreamEye_subDMA(void* data,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	maple_subdevice_instance* device_instance=(maple_subdevice_instance*)data;

	//printf("ControllerDMA Called 0x%X;Command %d\n",device_instance->port,Command);
//void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen)
	u8*buffer_out_b=(u8*)buffer_out;

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//caps
			//4
			w32(0x00080000);

			//struct data
			//3*4
			w32( 0x30A800C0); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				if (!testJoy_strName_dreameye_2[i])
				{
					w8(0x20);
				}
				else
				{
					w8((u8)testJoy_strName_dreameye_2[i]);
				}
			}

			//60
			for (u32 i = 0; i < 60; i++)
			{
				if (!testJoy_strBrand[i])
				{
					w8(0x20);
				}
				else
				{
					w8((u8)testJoy_strBrand[i]);
				}
			}

			//2
			w16(0); 

			//2
			w16(0); 
			return 5;

//(07/01/07)[02:36] <BlueCrab> dreameye: replied with 8, size 5
//(07/01/07)[02:36] <BlueCrab> 00080000, 000000D0, 1F000480, 1F000481, C0070094,
//(07/01/07)[02:46] <BlueCrab> ok... to the 6 command 9's after the one with data, it replies identically:
//(07/01/07)[02:46] <BlueCrab> dreameye: replied with 8, size 2
//(07/01/07)[02:46] <BlueCrab> 00080000, 000000D0, 
		case 9:

			//caps
			//4
			w32(0x00080000);
			//struct data
			if (des_9_count==0)
			{
			//4 dwords
			w32(0x000000D0);
			w32(0x1F000480);
			w32(0x1F000481);
			w32(0xC0070094);
			}
			else
			{
				//1 dword
				w32(0x000000D0); //? ok maby ?
			}

			des_9_count++;
			printf("DreamEye_subDMA[0x%x | %d %x] : unknown MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
			printf(" buffer in size : %d\n",buffer_in_len);
			for (u32 i=0;i<buffer_in_len;i+=4)
			{
				printf("%08X ",*buffer_in++);
			}
			printf("\n");
			//getchar();
			return 0x08;
//(07/01/07)[03:04] <BlueCrab> dreameye: replied with 11, size 2
//(07/01/07)[03:04] <BlueCrab>           responding to port B1
//(07/01/07)[03:04] <BlueCrab> 00080000, 000002FF, 
		case 17:
			{
				//responce=17; //? fuck ?
				//caps
				//4
				w32(0x00080000);
				//mmwaaa?
				//w32(0x000002FF);
				w32(rand());
			}
			printf("DreamEye_subDMA[0x%x | %d %x] : unknown MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
			printf(" buffer in size : %d\n",buffer_in_len);
			for (u32 i=0;i<buffer_in_len;i+=4)
			{
				printf("%08X ",*buffer_in++);
			}
			printf("\n");
			//getchar();
			return 17;
		default:
			printf("DreamEye_subDMA[0x%x | %d %x] : unknown MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
			printf(" buffer in size : %d\n",buffer_in_len);
			for (u32 i=0;i<buffer_in_len;i+=4)
			{
				printf("%08X ",*buffer_in++);
			}
			printf("\n");
			//getchar();
			return 7;//just ko
	}
}


u32 FASTCALL MicDMA(void* data,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len)
{
	maple_subdevice_instance* device_instance=(maple_subdevice_instance*)data;
	u8*buffer_out_b=(u8*)buffer_out;

	printf("MicDMA[0x%x | %d %x] : unknown MAPLE COMMAND %d \n",device_instance->port,device_instance->port>>6,device_instance->port&63,Command);
	printf(" buffer in size : %d\n",buffer_in_len);

	for (u32 i=0;i<buffer_in_len;i+=4)
	{
		printf("%08X ",*buffer_in++);
	}
	printf("\n");

	switch (Command)
	{
		/*typedef struct {
			DWORD		func;//4
			DWORD		function_data[3];//3*4
			u8		area_code;//1
			u8		connector_direction;//1
			char		product_name[30];//30*1
			char		product_license[60];//60*1
			WORD		standby_power;//2
			WORD		max_power;//2
		} maple_devinfo_t;*/
		case 1:
			//caps
			//4
			w32(0x10000000);

			//struct data
			//3*4
			w32( 0x3F000000);  // ?? wii pwns ps3 ...
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xF); //WTF ?
			//1	direction
			w8(1); //WTF ?
			//30
			for (u32 i = 0; i < 30; i++)
			{
				if (!testJoy_strName_mic[i])
				{
					w8(0x20);
				}
				else
				{
					w8((u8)testJoy_strName_mic[i]);
				}
			}

			//60
			for (u32 i = 0; i < 60; i++)
			{
				if (!testJoy_strBrand[i])
				{
					w8(0x20);
				}
				else
				{
					w8((u8)testJoy_strBrand[i]);
				}
			}
			//ptr_out += 60;

			//2
			w16(0x012C); 

			//2
			w16(0x012C); 
			return 5;
		case 9:
			//caps
			//4
			w32(0x10000000);
			//getchar();
			return 8;

		default:
			return 7;//just ko
	}
}


s32 FASTCALL CreateMain(maple_device_instance* inst,u32 id,u32 flags,u32 rootmenu)
{
	inst->data=inst;

	wchar temp[512];

	if (id>=2)
		

	if (id==0)
	{
		inst->dma=DreamEye_mainDMA;
		swprintf(temp,L"Controller[winhook] : 0x%02X",inst->port);
	}
	else
		return rv_error;
	

	host.AddMenuItem(rootmenu,-1,temp,0,0);

	return rv_ok;
}


s32 FASTCALL CreateSub(maple_subdevice_instance* inst,u32 id,u32 flags,u32 rootmenu)
{
	inst->data=inst;
	
	wchar temp[512];

	if (id==1)
	{
		inst->dma=MicDMA;
		swprintf(temp,L"Controller[winhook,net] : 0x%02X",inst->port);
	}
	else
		return rv_error;

	host.AddMenuItem(rootmenu,-1,temp,0,0);

	return rv_ok;
}
s32 FASTCALL Init(void* data,u32 id,maple_init_params* params)
{

	return rv_ok;
}
void FASTCALL Term(void* data,u32 id)
{
}
void FASTCALL Destroy(void* data,u32 id)
{

}

#define MMD(name,flags) \
	wcscpy(km.devices[mdi].Name,name);	\
	km.devices[mdi].Type=MDT_Main;	\
	km.devices[mdi].Flags= flags;	\
	mdi++;

#define MSD(name,flags)	\
	wcscpy(km.devices[mdi].Name,name);	\
	km.devices[mdi].Type=MDT_Sub;	\
	km.devices[mdi].Flags= flags;	\
	mdi++;

#define MDLE() km.devices[mdi].Type=MDT_EndOfList;

//Give a list of the devices to the emu
#define __T(x) L##x
#define _T(x) __T(x)
void EXPORT_CALL dcGetInterface(plugin_interface* info)
{

#define km info->maple

#define c info->common
	
	info->InterfaceVersion=PLUGIN_I_F_VERSION;

	c.InterfaceVersion=MAPLE_PLUGIN_I_F_VERSION;

	c.Load=Load;
	c.Unload=Unload;
	c.Type=Plugin_Maple;
	
	wcscpy(c.Name,L"DreamMaple Devices [" _T(__DATE__) L"]");

	km.CreateMain=CreateMain;
	km.CreateSub=CreateSub;
	km.Init=Init;
	km.Term=Term;
	km.Destroy=Destroy;

	u32 mdi=0;
	//0
	MMD(L"nullDC DreamEye (" _T(__DATE__) L")",MDTF_Hotplug);

	//1
	MSD(L"nullDC Mic (" _T(__DATE__) L")",MDTF_Hotplug);

	MDLE();
}
void LoadSettings()
{

}

void SaveSettings()
{

}