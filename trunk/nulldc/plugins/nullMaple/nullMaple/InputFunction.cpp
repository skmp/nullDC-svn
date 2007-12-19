#include "MapleFunctions.h"
#include "InputProviders.h"
#include "Profiles.h"

enum IEventType { ET_None,ET_Pressed,ET_AxisP,ET_AxisN,ET_AxisFRP,ET_AxisFRN,ET_AxisFR,ET_AxisFRI };
struct IEvent {InputProvider* ip; u32 id;u32 type;};

struct InputFunctionClass : MapleFunction
{
	IEvent Events[32];//up to 32 events -- keyboard uses special mapping
	vector<InputProvider*> Providers;
	SubProfile* profile;

	u32 InputButtons;

	InputFunctionClass(MapleDevice* device,u32 functions)
	{
		InputButtons=functions;
		memset(Events,0,sizeof(Events));
		profile=0;
	}
	s32 GetProviderIdx(const GUID& prov)
	{
		for (u32 i=0;i<Providers.size();i++)
		{
			if (Providers[i]->GetGuid()==prov)
			{
				return i;
			}
		}
		
		return -1;
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
		vector<InputProvider*> pnew;
		u32 esid=0;
		for (u32 i=0;i<32;i++)
		{
			if (InputButtons & (1<<i))
			{
				GUID dev;
				u32 p1;
				u32 p2;
				profile->GetMapParams(esid,&dev,&p1,&p2);
				Events[i].id=p1;
				Events[i].type=p2;

				s32 pindx=GetProviderIdx(dev);
				if (pindx>=0)
				{
					pnew.push_back(Events[i].ip=Providers[pindx]);
					Providers.erase(Providers.begin() + pindx);
				}
				else
				{
					pnew.push_back(Events[i].ip=InputProvider::Find(dev));
				}

				esid++;
			}
		}

		for (u32 i=0;i<Providers.size();i++)
		{
			Providers[i]->Term();
		}
		Providers.clear();

		for (u32 i=0;i<pnew.size();i++)
		{
			Providers.push_back(pnew[i]);
		}
		pnew.clear();
	}

	virtual u32 GetID() { return MFID_0_Input; }
	virtual u32 GetDesc() { return InputButtons; }
	
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
		delete this;
	}

	virtual void Dma(u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
	{
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
void SetupSubProfile0(SubProfile* sp,u32 lparam,void* dparam)
{
	sp->AddMap(L"Up",0,0);
	sp->AddMap(L"Down",0,0);
	sp->AddMap(L"Left",0,0);
	sp->AddMap(L"Right",0,0);
	sp->AddMap(L"Analog Up",0,0);
	sp->AddMap(L"Analog Down",0,0);
	sp->AddMap(L"Analog Left",0,0);
	sp->AddMap(L"Analog Right",0,0);
	sp->AddMap(L"Start",0,0);
	sp->AddMap(L"Y",0,0);
	sp->AddMap(L"X",0,0);
	sp->AddMap(L"B",0,0);
	sp->AddMap(L"A",0,0);
	sp->AddMap(L"Left Slider",0,0);
	sp->AddMap(L"Right Slider",0,0);

	sp->Commit();
}