// drkMapleDevices.cpp : Defines the entry point for the DLL application.
//

#include "..\..\..\nullDC\plugins\plugin_header.h"
#include <memory.h>

#ifdef UNICODE
#undef UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

u16 kcode=0xFFFF;
s8 joyx=0,joyy=0;
s8 joy2x=0,joy2y=0;
u8 rt=0,lt=0;

#pragma pack(1)
char testJoy_strName[64] = "Emulated Dreamcast Controler\0";
char testJoy_strName_nul[64] = "Null Dreamcast Controler\0";
char testJoy_strName_vmu[64] = "Emulated VMU\0";
char testJoy_strBrand[64] = "Faked by drkIIRaziel && ZeZu , made for nullDC\0";

#define key_CONT_C  (1 << 0);
#define key_CONT_B  (1 << 1);
#define key_CONT_A  (1 << 2);
#define key_CONT_START  (1 << 3);
#define key_CONT_DPAD_UP  (1 << 4);
#define key_CONT_DPAD_DOWN  (1 << 5);
#define key_CONT_DPAD_LEFT  (1 << 6);
#define key_CONT_DPAD_RIGHT  (1 << 7);
#define key_CONT_Z  (1 << 8);
#define key_CONT_Y  (1 << 9);
#define key_CONT_X  (1 << 10);
#define key_CONT_D  (1 << 11);
#define key_CONT_DPAD2_UP  (1 << 12);
#define key_CONT_DPAD2_DOWN  (1 << 13);
#define key_CONT_DPAD2_LEFT  (1 << 14);
#define key_CONT_DPAD2_RIGHT  (1 << 15);	

struct VMU_info
{
	u8 data[256*1024];
	char file[512];
};
typedef INT_PTR CALLBACK dlgp( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
dlgp* oldptr;
INT_PTR CALLBACK sch( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg)
	{
	case WM_KEYDOWN:
		switch(wParam)
		{
		case 'Z':
			kcode &= 0xFFFF - key_CONT_A;
			break;
		case 'X':
			kcode &= 0xFFFF - key_CONT_B;
			break;
		case 'C':
			kcode &= 0xFFFF - key_CONT_C;
			break;
		case 'V':
			kcode &= 0xFFFF - key_CONT_D;
			break;
		
		case 'B':
			kcode &= 0xFFFF - key_CONT_Z;
			break;
		
		case 'N':
			kcode &= 0xFFFF - key_CONT_Y;
			break;
		
		case 'M':
			kcode &= 0xFFFF - key_CONT_X;
			break;

		case VK_SHIFT:
			kcode &= 0xFFFF - key_CONT_START;
			break;

		case VK_UP:
			kcode &= 0xFFFF - key_CONT_DPAD_UP;
			break;
		case VK_DOWN:
			kcode &= 0xFFFF - key_CONT_DPAD_DOWN;
			break;
		case VK_LEFT:
			kcode &= 0xFFFF - key_CONT_DPAD_LEFT;
			break;
		case VK_RIGHT:
			kcode &= 0xFFFF - key_CONT_DPAD_RIGHT;
			break;

		case 'K'://analog right
			joyx= +126;
			break;
		case 'H'://alalog left
			joyx= -126;
			break;

		case 'U'://analog up
			joyy= -126;
			break;
		case 'J'://analog down
			joyy= +126;
			break;

		case 'A'://ltriger
			lt=255;
			break;
		case 'S'://rtriger
			rt=255;
			break;
		}
		break;

	case WM_KEYUP:
		switch(wParam)
		{
		case 'Z':
			kcode |= key_CONT_A;
			break;
		case 'X':
			kcode |= key_CONT_B;
			break;
		case 'C':
			kcode |= key_CONT_C;
			break;
		case 'V':
			kcode |= key_CONT_D;
			break;

		case 'B':
			kcode |= key_CONT_Z;
			break;
		
		case 'N':
			kcode |= key_CONT_Y;
			break;
		
		case 'M':
			kcode |= key_CONT_X;
			break;

		case VK_SHIFT:
			kcode |= key_CONT_START;
			break;

		case VK_UP:
			kcode |= key_CONT_DPAD_UP;
			break;
		case VK_DOWN:
			kcode |= key_CONT_DPAD_DOWN;
			break;
		case VK_LEFT:
			kcode |= key_CONT_DPAD_LEFT;
			break;
		case VK_RIGHT:
			kcode |= key_CONT_DPAD_RIGHT;
			break;

		case 'K'://analog right
			joyx=0;
			break;
		case 'H'://alalog left
			joyx=0;
			break;

		case 'U'://analog up
			joyy=0;
			break;
		case 'J'://analog down
			joyy=0;
			break;

		case 'A'://ltriger
			lt=0;
			break;
		case 'S'://rtriger
			rt=0;
			break;
		}
		break;
	}
	return oldptr(hWnd,uMsg,wParam,lParam);
}

void cfgdlg(PluginType type,void* window)
{
	printf("drkIIRaziel's MAPLIE plugin:No config kthx\n");
	//if (cur_icpl->PvrDllConfig)
		//cur_icpl->PvrDllConfig((HWND)window);
}//called when plugin is used by emu (you should do first time init here)
void* handle;
void dcInitPvr(void* aparam,PluginType type)
{
	maple_init_params* mpi=(maple_init_params*)aparam;
	handle=mpi->WindowHandle;
	oldptr = (dlgp*)SetWindowLongPtr((HWND)handle,GWL_WNDPROC,(LONG)sch);
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void dcTermPvr(PluginType type)
{
	SetWindowLongPtr((HWND)handle,GWL_WNDPROC,(LONG)oldptr);
}

//It's suposed to reset anything but vram (vram is set to 0 by emu)
void dcResetPvr(bool Manual,PluginType type)
{
	//hahah do what ? ahahahahahaha
}

//called when entering sh4 thread , from the new thread context (for any thread speciacific init)
void dcThreadInitPvr(PluginType type)
{
	//maby init here ?
}

//called when exiting from sh4 thread , from the new thread context (for any thread speciacific de init) :P
void dcThreadTermPvr(PluginType type)
{
	//term here ?
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

//Give to the emu info for the plugin type
EXPORT void dcGetPluginInfo(plugin_info* info)
{
	info->InterfaceVersion.full=PLUGIN_I_F_VERSION;
	sprintf(info->Name,"drkMaple");
	info->PluginVersion.full=NDC_MakeVersion(1,2,3);
	

	info->Init=dcInitPvr;
	info->Term=dcTermPvr;
	info->Reset=dcResetPvr;

	info->ThreadInit=dcThreadInitPvr;
	info->ThreadTerm=dcThreadTermPvr;

	info->ShowConfig=cfgdlg;
	info->Type=PluginType::MapleDevice;
}



u8 GetBtFromSgn(s8 val);

#define w32(data) *(u32*)buffer_out_b=(data);buffer_out_b+=4;buffer_out_len+=4
#define w16(data) *(u16*)buffer_out_b=(data);buffer_out_b+=2;buffer_out_len+=2
#define w8(data) *(u8*)buffer_out_b=(data);buffer_out_b+=1;buffer_out_len+=1


void ControllerDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
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
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(1 << 24);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x04FF); 

			//2
			w16(0x0069); 
			break;

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


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			responce=0x08;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32((1 << 24));
			//struct data
			//2
			w16(kcode); 
			
			//triger
			//1 R
			w8(rt);
			//1 L
			w8(lt); 
			//joyx
			//1
			w8(GetBtFromSgn(joyx));
			//joyy
			//1
			w8(GetBtFromSgn(joyy));

			//1
			w8(GetBtFromSgn(joy2x)); 
			//1
			w8(GetBtFromSgn(joy2y)); 
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;

			break;

		default:
			printf("UNKOWN MAPLE COMMAND \n");
			break;
	}
}

void ControllerDMA_nul(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
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
			//header
			//WriteMem32(ptr_out,(u32)(0x05 | //response
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 0) << 16) & 0xFF0000) |
			//			(((112/4) << 24) & 0xFF000000))); ptr_out += 4;

			responce=5;

			//caps
			//4
			w32(1 << 24);

			//struct data
			//3*4
			w32( 0xfe060f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_nul[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x04FF); 

			//2
			w16(0x0069); 
			break;

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


			//header
			//WriteMem32(ptr_out, (u32)(0x08 | // data transfer (response)
			//			(((u16)sendadr << 8) & 0xFF00) |
			//			((((recadr == 0x20) ? 0x20 : 1) << 16) & 0xFF0000) |
			//			(((12 / 4 ) << 24) & 0xFF000000))); ptr_out += 4;
			responce=0x08;
			//caps
			//4
			//WriteMem32(ptr_out, (1 << 24)); ptr_out += 4;
			w32((1 << 24));
			//struct data
			//2
			w16(0xFFFF); 
			
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
			//are these needed ?
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;
			//1
			//WriteMem8(ptr_out, 10); ptr_out += 1;

			break;

		default:
			printf("UNKOWN MAPLE COMMAND \n");
			break;
	}
}

u8 GetBtFromSgn(s8 val)
{
	return val+128;
}

typedef struct {
	u16 total_size;
	u16 partition_number;
	u16 system_area_block;
	u16 fat_area_block;
	u16 number_fat_areas_block;
	u16 file_info_block;
	u16 number_info_blocks;
	u8 volume_icon;
	u8 reserved1;
	u16 save_area_block;
	u16 number_of_save_blocks;
	u16 reserverd0;  
}maple_getvmuinfo_t;

#define SWAP32(val) ((u32) ( \
	(((u32) (val) & (u32) 0x000000ffU) << 24) | \
	(((u32) (val) & (u32) 0x0000ff00U) <<  8) | \
	(((u32) (val) & (u32) 0x00ff0000U) >>  8) | \
	(((u32) (val) & (u32) 0xff000000U) >> 24)))
void VmuDMA(maple_device_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	u8*buffer_out_b=(u8*)buffer_out;
	//printf("VmuDMA Called for port 0x%X, Command %d\n",device_instance->port,Command);
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

			responce=5;

			//caps
			//4
			w32(2 << 24);

			//struct data
			//3*4
			w32( 0x00410f00); 
			w32( 0);
			w32( 0);
			//1	area code
			w8(0xFF);
			//1	direction
			w8(0);
			//30
			for (u32 i = 0; i < 30; i++)
			{
				w8((u8)testJoy_strName_vmu[i]);
				//if (!testJoy_strName[i])
				//	break;
			}
			//ptr_out += 30;

			//60
			for (u32 i = 0; i < 60; i++)
			{
				w8((u8)testJoy_strBrand[i]);
				//if (!testJoy_strBrand[i])
				//	break;
			}
			//ptr_out += 60;

			//2
			w16(0x7c00); 

			//2
			w16(0x8200); 
			break;

				//in[0] is function used ?
				//out[0] is function used ?
		case 10:
			if (buffer_in[0]& (2<<24))
			{
				buffer_out[0] = (2<<24);//is that ok ?
				maple_getvmuinfo_t* vmui = (maple_getvmuinfo_t*)(&buffer_out[1]);
				//ZeroMemory(pMediaInfo,sizeof(TMAPLE_MEDIAINFO));
				memset(vmui,0,sizeof(maple_getvmuinfo_t));
				vmui->total_size = 0xFF;//0x7FFF;//0xFF
				vmui->system_area_block = 0xFF;//0x7FFF;//0xff
				vmui->fat_area_block = 0xfe;//0x7F00;	//0xfe
				vmui->number_fat_areas_block = 1;//256;//1
				vmui->volume_icon = 0x0;//0
				vmui->save_area_block = 0xc8;//?
				vmui->number_of_save_blocks = 0x1f;
				//pMediaInfo->volume_icon = 0x0;
				vmui->file_info_block = 0xfd;//0x7E00;//0xfd
				vmui->number_info_blocks = 0xd;//0x100;//0xd
				vmui->reserverd0 = 0x0000;
				buffer_out_len=4+(sizeof(maple_getvmuinfo_t));
				responce=8;//data transfer
			}
			else
				responce=-2;//bad function
			break;

		case 11:
			if(buffer_in[0]&(2<<24))
			{
				VMU_info* dev=(VMU_info*)((*device_instance).DevData);

				buffer_out[0] = (2<<24);
				u32 Block = (SWAP32(buffer_in[1]))&0xffff;
				buffer_out[1] = buffer_in[1];
				printf("Block read : %d\n",Block);
				if (Block>255)
				{
					printf("BLOCK READ ERROR\n");
					Block&=256;
				}
				memcpy(&buffer_out[2],(dev->data)+Block*512,512);
				buffer_out_len=(512+8);
				responce=8;//data transfer
			}
			else
				responce=-2;//bad function
			break;
		case 12:
			if(buffer_in[0]&(2<<24))
			{
				VMU_info* dev=(VMU_info*)((*device_instance).DevData);

				u32 Block = (SWAP32(buffer_in[1]))&0xffff;
				u32 Phase = ((SWAP32(buffer_in[1]))>>16)&0xff; 
				printf("Block wirte : %d:%d , %d bytes\n",Block,Phase,(buffer_in_len-4));
				memcpy(&dev->data[Block*512+Phase*(512/4)],&buffer_in[2],(buffer_in_len-4));
				buffer_out_len=0;
				FILE* f=fopen(dev->file,"wb");
				if (f)
				{
					fwrite(dev->data,1,128*1024,f);
					fclose(f);
				}
				else
					printf("Failed to open %s for saving vmu data\n",dev->file);
				responce=7;//just ko
			}
			else
				responce=-2;//bad function
			break;

		case 13:
			responce=7;//just ko
			break;

		default:
			printf("UNKOWN MAPLE COMMAND \n");
			break;
	}
}


void CreateInstance(maple_device*dev,maple_device_instance& inst,u8 port)
{
	if (dev->id==0)
	{
		inst.MapleDeviceDMA=ControllerDMA;
		inst.DevData=0;
	}
	else if (dev->id==1)
	{
		inst.DevData=malloc(sizeof(VMU_info));
		sprintf(((VMU_info*)inst.DevData)->file,"vmu_data_port%x.bin",port);
		FILE* f=fopen(((VMU_info*)inst.DevData)->file,"rb");
		if (f)
		{
			fread(((VMU_info*)inst.DevData)->data,1,128*1024,f);
			fclose(f);
		}
		inst.MapleDeviceDMA=VmuDMA;
	}
	else if (dev->id==2)
	{
		inst.MapleDeviceDMA=ControllerDMA_nul;
		inst.DevData=0;
	}
	printf("Created instance of device %s on port 0x%x\n",dev->name,port);
}


void DestroyInstance(maple_device*dev,maple_device_instance& inst)
{
	if (inst.DevData)
		free(inst.DevData);
	printf("Deleted instance of device %s \n",dev->name);
}
//Give a list of the devices to teh emu
EXPORT void dcGetMapleInfo(maple_plugin_if* info)
{
	info->InterfaceVersion.full=MAPLE_PLUGIN_I_F_VERSION;

	info->Devices[0].CreateInstance=CreateInstance;
	info->Devices[0].DestroyInstance=DestroyInstance;
	info->Devices[0].type=0;//Controller
	info->Devices[0].id=0;
	strcpy(info->Devices[0].name,"teh l33t DC controller (" __DATE__ ")");
 
	info->Devices[1].CreateInstance=CreateInstance;
	info->Devices[1].DestroyInstance=DestroyInstance;
	info->Devices[1].type=1;//Vmu
	info->Devices[1].id=1;
	strcpy(info->Devices[1].name,"teh l33t VMU(" __DATE__ ")");

	info->Devices[2].CreateInstance=CreateInstance;
	info->Devices[2].DestroyInstance=DestroyInstance;
	info->Devices[2].type=0;//Controller
	info->Devices[2].id=0;
	strcpy(info->Devices[2].name,"teh null DC controller [no input](" __DATE__ ")");

	info->Devices[3].CreateInstance=0;
	info->Devices[3].DestroyInstance=0;
}


