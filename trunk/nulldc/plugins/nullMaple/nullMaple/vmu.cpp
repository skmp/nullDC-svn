#include "DreamcastDevices.h"
#pragma warning (disable : 4065)
static MapleDeviceInfo vmumdi = 
{
	//u32		func;//4
	2 << 24,
	//u32		function_data[3];
	0x00410f00,
	0,
	0,
	//u8		area_code;
	0xFF,
	//u8		connector_direction;
	0,
	//char	product_name[30];
	"Dreamcast VMU\0",
	//char	product_license[60];
	"Produced By or Under License From SEGA ENTERPRISES,LTD.\0",
	//u16		standby_power;
	0x7c00,
	//u16		max_power;
	0x8200
};

struct VMUDesc : virtual MapleDeviceDesc
{
	virtual MapleDeviceType GetType() const	{ return MDT_Sub; }
	virtual u32 GetFlags() const	{ return MDTF_Hotplug; }

	virtual const wchar* GetName() const
	{
		return L"nullDC VMU/n";
	}
	
	virtual u32 GetMDID() const
	{
		return 0;
	}

	virtual void SetupProfile(ProfileDDI* prof,u32 ftid) const
	{
	}
};

struct VMU : virtual MapleDevBase< VMUDesc >
{
public:
	maple_subdevice_instance* inst;
	VMU(maple_subdevice_instance* ins,u32 menu) 
	{	
		this->menu=menu;
		inst=ins;	
		functs.push_back(MapleFunction::Create(this,MFID_1_Storage,0x00410f00,0));
		
		memcpy(&mdi,&vmumdi,sizeof(vmumdi));
	}

	virtual void Init()
	{
		InitFunc();
	}
	virtual void Term()
	{
		TermFunc();
	}
	virtual void Destroy()
	{
		DestFunc();
		delete this;
	}

	virtual u8 GetPort() const
	{
		return inst->port;
	}
	
	virtual void MiscDma(u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
	{
		u8*buffer_out_b=(u8*)buffer_out;
		
		switch (Command)
		{
		default:
			{
				printf("UNKOWN MAPLE COMMAND %d\n",Command);
				responce=MDRE_UnkownCmd;
			}
			break;
		}
	}
};


ListItem< MapleDevSFactBase< VMUDesc , VMU > > djd_al(_Devices);

#ifdef LAZY

#define SWAP32(val) ((u32) ( \
	(((u32) (val) & (u32) 0x000000ffU) << 24) | \
	(((u32) (val) & (u32) 0x0000ff00U) <<  8) | \
	(((u32) (val) & (u32) 0x00ff0000U) >>  8) | \
	(((u32) (val) & (u32) 0xff000000U) >> 24)))
class VMU : public DreamcastSubDevice
{
public:
	VMU() {}
	VMU(maple_subdevice_instance* ins,DreamcastDeviceInfo* inf) 
	{	
		inst=ins;
		Info=inf;
	}

	virtual void Init()
	{
		
	}
	virtual void Term()
	{
		
	}
	virtual void Destroy()
	{
		
	}

	virtual u8 GetPort() const
	{
		return inst->port;
	}


	//Implementation from now on
	//Bios (and most likely gamecode) only handles default cards.Strange ...
	enum State
	{
		VDS_RESETED,	//just reseted
		VDS_READY,		//Has been initialised (GetDeviceInfo was called)
		VDS_READING,	//Reading a multi-phase block
		VDS_WRITING,	//Reading a multi-phase block
	};
	State state;
	u32 CurrentPhase;	//used for multi-phase operations
	virtual void Dma(maple_subdevice_instance* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
	{
		u8*buffer_out_b=(u8*)buffer_out;
		u32 port=device_instance->port>>6;
		switch (Command)
		{
			//Device info !
		case MDC_DeviceRequest:
			{
				wbuff(vmumdi);

				responce=MDRV_DeviseStatus;
			}
			break;

		case MDCS_GetMediaInfo:
			if (buffer_in[0]& (2<<24))//and or == ?
			{
				w32(2<<24);
				GetMediaInfo_data vmui =
				{
					0xFF,		//Total Size
					0,			//Partition ##
					0xFF,		//System Area block
					0xFE,		//Fat Area Block
					1,			//Number of Fat Area blocks
					0xFD,		//File Info block
					0x0D,		//Number of file info blocks
					0,			//Volume icon
					0,			//Reserved
					0xC8,		//Save Area Block
					0x1F,		//Number of save blocks
					0,			//Reserved
				};

				wbuff(vmui);
				
				responce=MDRV_DataTransfer;//data transfer
			}
			else
				responce=MDRV_UnkownFunction;
			break;

		case 11:
			if(buffer_in[0]&(2<<24))
			{
				//VMU_info* dev=(VMU_info*)((*device_instance).data);

				buffer_out[0] = (2<<24);
				u32 Block = (SWAP32(buffer_in[1]))&0xffff;
				buffer_out[1] = buffer_in[1];
				//if (Block>255)
				//{
					printf("Block read : %d\n",Block);
				//	printf("BLOCK READ ERROR\n");
				//	Block&=255;
				//}
				//memcpy(&buffer_out[2],(dev->data)+Block*512,512);
				buffer_out_len=(512+8);
				responce=8;//data transfer
			}
			else
				responce=-2;//bad function
			break;
		case 12:
			if(buffer_in[0]&(2<<24))
			{
				//VMU_info* dev=(VMU_info*)((*device_instance).data);

				u32 Block = (SWAP32(buffer_in[1]))&0xffff;
				u32 Phase = ((SWAP32(buffer_in[1]))>>16)&0xff; 
				printf("Block wirte : %d:%d , %d bytes\n",Block,Phase,(buffer_in_len-8));
				//memcpy(&dev->data[Block*512+Phase*(512/4)],&buffer_in[2],(buffer_in_len-8));
				buffer_out_len=0;
				//FILE* f=fopen(dev->file,"wb");
				//if (f)
				//{
				//	fwrite(dev->data,1,128*1024,f);
//					fclose(f);
//				}
//				else
//					printf("Failed to open %s for saving vmu data\n",dev->file);
				responce=7;//just ko
			}
			else
				responce=-2;//bad function
			break;

		case 13:
			responce=7;//just ko
			break;

		default:
			{
				printf("UNKOWN MAPLE COMMAND %d\n",Command);
			}
			break;
		}
	}
};
class VMUInfo : public DreamcastSubDeviceInfo
{
	virtual DreamcastSubDevice* Create(maple_subdevice_instance* inst)
	{
		return new VMU(inst,this);
	}

	
	virtual MapleDeviceType GetType() const	{ return MDT_Sub; }
	virtual u32 GetFlags() const	{ return MDTF_Hotplug; }

	virtual const wchar* GetName() const
	{
		return L"nullDC VMU/n";
	}
	virtual const wchar* GetGuid() const
	{
		return 0;
	}
};
ListItem<VMUInfo> djd_al(_Devices);

#endif