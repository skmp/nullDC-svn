#include "MapleFunctions.h"
#include "InputProviders.h"
#include "Profiles.h"

enum IEventType { ET_None,ET_Pressed,ET_AxisP,ET_AxisN,ET_AxisFRP,ET_AxisFRN,ET_AxisFR,ET_AxisFRI };
struct IEvent {InputProvider* ip; u32 id;u32 type;};

#define BSWAP(v) (((v&0xFF)<<24) | ((v&0xFF00)<<8) | ((v>>8)&0xFF00) | (v>>24))

struct InputFunctionClass : MapleFunction,OptionGroopCallBack
{
	IEvent Events[32];//up to 32 events
	InputProvider* prov;
	SubProfile* sp;
	Profile* pr;
	MapleDevice* device;

	u32 Desc;
	u32 LastCfgRev;
	
	CRITICAL_SECTION spcs;
	OptionGroop* og;
	wstring cfgkey;

	void SetDevice(GUID dev)
	{
		if (prov)
		{
			prov->Term();
		}
		prov=InputProvider::Find(dev);
		wstring val=GuidToText(prov->GetGuid());
		host.ConfigSaveStr(CFG_NAME,cfgkey.c_str(),val.c_str());

		pr=Profile::GetProfile((device->GetMDID()<<16)|0,val);
		if (!pr)
		{
			pr=Profile::Create((device->GetMDID()<<16)|0,val);
		}
		sp=pr->GetSub(0);
		LastCfgRev=~sp->GetSPRev();
	}
	InputFunctionClass(MapleDevice* device,u32 functions)
	{
		this->device=device;
		Desc=functions;
		memset(Events,0,sizeof(Events));
		//find curr function
		wchar key[512];
		wchar val[512];
		swprintf_s(key,L"Setting_%02X_%d_%d",device->GetPort(),device->GetMDID(),0);
		cfgkey=key;
		host.ConfigLoadStr(CFG_NAME,key,val,L"{0}");

		prov=0;
		SetDevice(ParseGuid(val));

		InitializeCriticalSection(&spcs);
		
		og = new OptionGroop(this);
		for (u32 i=0;i<Providers.size();i++)
		{
			og->Add(device->GetMenu(),Providers[i]->GetName(),i,Providers[i]);
			if (Providers[i]->GetGuid()==prov->GetGuid())
				og->SetValue(i,0);
		}
	}

	virtual void OnMenuClick(int val,void* pUser,void* window)
	{
		//og->SetValue(val,window)// not needed ;)
		EnterCriticalSection(&spcs);
		InputProvider* ip=(InputProvider*)pUser;
		SetDevice(ip->GetGuid());
		LeaveCriticalSection(&spcs);
	}

	u32 GetBitState(int eid)
	{
		s32 AV=GetAxisState(eid);
		return AV!=0?1:0;
	}
	s32 GetAxisState(int eid)
	{
		IEvent& evt=Events[eid];
		InputProvider* ip=evt.ip;

		if (!ip)
			return 0;

		switch(evt.type)
		{
		case ET_None:
			return 0;
		case ET_Pressed:
			return ip->GetEventValue(evt.id)?255:0;

			//full range,poosible invertion
		case ET_AxisFR:
		case ET_AxisFRI:
			{
				s32 rv=ip->GetEventValue(evt.id);
				if (ip->GetEventType(evt.id)==IET_Button)
					rv*=255;
				if (evt.type == ET_AxisFRI)
				{
					rv*=-1;
				}
				return rv;
			}
			//full range tranformed to [-255...0] or [0...255]
		case ET_AxisFRP:
		case ET_AxisFRN:
			{
				s32 rv=ip->GetEventValue(evt.id);
				if (ip->GetEventType(evt.id)==IET_Button)
					rv*=255;
				rv+=255;

				if (evt.type == ET_AxisFRN)
					rv*=-1;
				return rv;
			}
		case ET_AxisP:
		case ET_AxisN:
			{
				s32 rv=ip->GetEventValue(evt.id);
				if (ip->GetEventType(evt.id)==IET_Button)
					rv*=255;
				if (evt.type == ET_AxisP)
				{
					if (rv<0)
						rv=0;
				}
				else
				{
					if (rv>0)
						rv=0;
				}
				if (rv<0)
					rv*=-1;
				return rv;
			}
		}
		//die("Should never ever get here");
		return 0;
	}


	void PopulateEvents()
	{
		EnterCriticalSection(&spcs);
		if (LastCfgRev!=sp->GetSPRev())
		{
			LastCfgRev=sp->GetSPRev();
			
			u32 InputButtons= BSWAP(Desc);

			u32 esid=0;
			for (u32 i=0;i<18;i++)
			{
				if (InputButtons & (1<<i))
				{
					u32 p[2];
					sp->GetArr(esid,p,2);
					//provider
					Events[i].id=p[0];
					Events[i].type=p[1];

					esid++;
				}
			}
			for (u32 i=18;i<22;i++)
			{
				if (InputButtons & (1<<i))
				{
					for (int j=0;j<2;j++)
					{
						u32 p[2];
						sp->GetArr(esid,p,2);
						
						Events[i].id=p[0];
						Events[i].type=p[1];

						esid++;
					}
				}
			}
		}
		LeaveCriticalSection(&spcs);
	}

	virtual u32 GetID() { return MFID_0_Input; }
	virtual u32 GetDesc() { return Desc; }
	
	virtual bool Init()
	{
		PopulateEvents();
		return true;
	}
	virtual void Term()
	{
		
	}
	virtual void Destroy()
	{
		prov->Term();
		prov=0;
		delete this;
	}

	virtual void Dma(u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
	{
		PopulateEvents();
		u8*buffer_out_b=(u8*)buffer_out;

		switch (Command)
		{
		case MDCF_GetCondition:
			{
				w32(MFID_0_Input);
				JoystickData* jd=(JoystickData*)buffer_out_b;
				
				jd->buttons=0xFFFF;
				for (u32 i=0;i<16;i++)
				{
					jd->buttons &=~(GetBitState(i)<<i);
				}

				jd->rtrig=GetAxisState(16);
				jd->ltrig=GetAxisState(17);
				jd->joyx=128+GetAxisState(18)/2 - GetAxisState(19)/2;
				jd->joyy=128+GetAxisState(20)/2 - GetAxisState(21)/2;
				jd->joy2x=128+GetAxisState(22)/2 - GetAxisState(23)/2;
				jd->joy2y=128+GetAxisState(24)/2 - GetAxisState(25)/2;

				wX(sizeof(*jd));
				responce=MDRS_DataTransfer;
			}
			break;

		default:
			printf("nullMaple::InputFunctionClass : Unknown command %d\n",Command);
			responce=MDRE_UnkownCmd;
			break;
		}
	}
};
MapleFunction* CreateFunction0(MapleDevice* dev,u32 lparam,void* dparam)
{
	InputFunctionClass* rv= new InputFunctionClass(dev,lparam);

	return rv;
}
wchar* KeyMapNames[32]=
{
	L"C",
	L"B",
	L"A",
	L"Start",
	//
	L"Up_1",
	L"Down_1",
	L"Left_1",
	L"Right_1",

	
	L"Z",
	L"Y",
	L"X",
	L"D",
	//
	L"Up_2",
	L"Down_2",
	L"Left_2",
	L"Right_2",

	L"Axis_LS",
	L"Axis_RS",
	L"Axis_X_1",
	L"Axis_Y_1",
	//
	L"Axis_X_2",
	L"Axis_Y_2",
	L"22",
	L"23",

	L"24",
	L"25",
	L"26",
	L"27",
	//
	L"28",
	L"29",
	L"30",
	L"31",

};
void SetupProfile0(ProfileDDI* p,u32 lparam,void* dparam)
{
	SubProfileDDI* sp=p->AddSub(L"Key");
	
	u32 InputButtons= BSWAP(lparam);
	u32 esid=0;
	for (u32 i=0;i<18;i++)
	{
		if (InputButtons & (1<<i))
		{
			sp->AddMap(KeyMapNames[i],2);
			esid++;
		}
	}
	for (u32 i=18;i<22;i++)
	{
		if (InputButtons & (1<<i))
		{
			wstring str;

			str=KeyMapNames[i];
			str+= L"_Pos";
			sp->AddMap(str,2);
			esid++;

			str=KeyMapNames[i];
			str+= L"_Neg";
			sp->AddMap(str,2);
			esid++;
		}
	}
}