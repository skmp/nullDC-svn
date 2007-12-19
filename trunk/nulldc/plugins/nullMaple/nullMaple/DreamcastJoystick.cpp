#include "DreamcastDevices.h"
#include "Profiles.h"

#pragma warning (disable : 4065)
static MapleDeviceInfo joymdi = 
{
	//u32		func;//4
	0,			//Filled it automaticaly,(1 << 24)
	//u32		function_data[3];
	0,			//Filled it automaticaly,0xfe060f00
	0,			//Filled it automaticaly,0
	0,			//Filled it automaticaly,0
	//u8		area_code;
	0xFF,
	//u8		connector_direction;
	0,
	//char	product_name[30];
	"Dreamcast Controller\0",
	//char	product_license[60];
	"Produced By or Under License From SEGA ENTERPRISES,LTD.\0",
	//u16		standby_power;
	0xAE01,
	//u16		max_power;
	0xF401
};
struct JoystickDeviceDesc : virtual MapleDeviceDesc
{
	virtual MapleDeviceType GetType() const	{ return MDT_Main; }
	virtual u32 GetFlags() const	{ return MDTF_Hotplug|MDTF_Sub0|MDTF_Sub1; }
	virtual u32 GetExtendedFlags() const { return 0;}

	virtual const wchar* GetName() const
	{
		return L"nullDC Joystick/n";
	}

	virtual GUID GetGuid() const
	{
		GUID rv= {0x7611486b,0x58a4,0x46dd,{0x89,0xbc,0x37,0x74,0x93,0x24,0x05,0x26}};
		return rv;
	}
	virtual void SetupProfile(Profile* prof) const
	{
		SubProfile* s0=prof->AddSub(L"Function 0");
		MapleFunction::SetupSubProfile(s0,MFID_0_Input,0xfe060f00,0);
	}
};

struct JoystickDevice : virtual MapleDevBase< JoystickDeviceDesc >
{
public:
	maple_device_instance* inst;
	Profile* map;

	JoystickDevice(maple_device_instance* ins) 
	{	
		inst=ins;	
		map=Profile::GetProfile(GetGuid(),ReadGuid(GetGuid(),inst->port));
		WriteGuid(GetGuid(),inst->port,map->GetGuid());
		
		functs.push_back(MapleFunction::Create(this,map->GetSub(0),MFID_0_Input,0xfe060f00,0));
		
		memcpy(&mdi,&joymdi,sizeof(joymdi));
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


ListItem< MapleDevMFactBase< JoystickDeviceDesc , JoystickDevice > > djd_al(_Devices);

