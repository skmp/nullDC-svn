#include "InputProviders.h"
#include<dinput.h> 
#include <math.h>

#pragma comment(lib,"Dinput8.lib")
#pragma comment(lib,"dxguid.lib")

IDirectInput8* di8;

BOOL __stdcall EnumAxesCallback(LPCDIDEVICEOBJECTINSTANCE pdidoi, LPVOID p)
{
	IDirectInputDevice8* dev=(IDirectInputDevice8*)p;
	DIPROPRANGE diprg; 

	diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
	diprg.diph.dwHow        = DIPH_BYID; 
	diprg.diph.dwObj        = pdidoi->dwType; 
	diprg.lMin              = -255; 
	diprg.lMax              = +255; 

	dev->SetProperty(DIPROP_RANGE, &diprg.diph);
	
	return DIENUM_CONTINUE;
}
struct GenericDInputProvider : public InputProvider
{
	DIDEVICEINSTANCE  devinst;
	IDirectInputDevice8* dev;
	DIDEVCAPS caps;

	struct 
	{
		InputEventType type;
		s32 Value;
	} EventMap[128+4*4+8];
	u32 EventCount;
	GenericDInputProvider(LPCDIDEVICEINSTANCE devins,u32 devcls) 
	{ 
		devinst=*devins;
		
		di8->CreateDevice(devinst.guidInstance,&dev,0);
		
		EventCount=0;

		if (devcls!=DI8DEVCLASS_GAMECTRL)
			return;

		dev->SetDataFormat(&c_dfDIJoystick);
		dev->SetCooperativeLevel((HWND)host.GetRenderTarget(),DISCL_EXCLUSIVE | DISCL_BACKGROUND);
		

		caps.dwSize = sizeof(DIDEVCAPS);
		dev->GetCapabilities(&caps);

		
		for (u32 i=0;i<caps.dwAxes;i++)
		{
			EventMap[EventCount].Value=0;
			EventMap[EventCount].type=IET_Axis;
			EventCount++;
		}

		for (u32 i=0;i<caps.dwPOVs;i++)
		{
			for (int j=0;j<4;j++)
			{
				EventMap[EventCount].Value=0;
				EventMap[EventCount].type=IET_Button;
				EventCount++;
			}
		}
		for (u32 i=0;i<caps.dwButtons;i++)
		{
			EventMap[EventCount].Value=0;
			EventMap[EventCount].type=IET_Button;
			EventCount++;
		}

		dev->EnumObjects(EnumAxesCallback, (VOID*)dev, DIDFT_AXIS);
	}
	
	//INPUTPROVIDERINFO
	virtual const wchar* GetName() const
	{
		return &devinst.tszInstanceName[0];
	}
	virtual const GUID GetGuid() const
	{
		return devinst.guidInstance;
	}

	virtual const wchar* GetDeviceName() const
	{
		return &devinst.tszProductName[0];
	}
	virtual const GUID GetDeviceGuid() const
	{
		return devinst.guidProduct;
	}

	virtual InputProvider* Create()
	{
		return this;
	}

	virtual u32 GetType() const
	{
		return IPT_Joystick;	//needs to realy check devinst -- nope it doesnt
	}

	//INPUT PROVIDER
	virtual void Init()
	{
		//nada
		dev->Acquire();
	}
	virtual void Term()
	{
		//nothin
		dev->Unacquire();
	}
	virtual void Destroy()
	{
		//who cares
		//dev->Release();
		//delete this;
	}

	virtual s32 GetEventValue(u32 id) //-128 to 128 for axis ,0 .. 1 for buttons
	{
		return EventMap[id].Value;
	}
	virtual s32 GetEventDiff(u32 id)  //returns diff. from last input. 0 if same, 1 if pressed, -1 if released.Axis delta for Axis
	{
		return 0;
	}
	virtual u32 GetEventCount() const 
	{ 
		return EventCount; 
	}
	virtual InputEventType GetEventType(u32 id)
	{
		return EventMap[id].type;
	}
	virtual void GetEventName(u32 id,wchar* dst) 
	{
		static const wchar* PovDirections[4]={L"Right",L"Left",L"Up",L"Down"};
		u32 Type;
		u32 tid;

		if (id < caps.dwAxes)
		{
			tid=id;
			Type=0;
		}
		else if (id < (caps.dwPOVs*4))
		{
			tid=id-caps.dwAxes;
			Type=1;
		}
		else
		{
			tid=id-caps.dwAxes-caps.dwPOVs*4;
			Type=2;
		}

		switch(EventMap[id].type)
		{
		case IET_Axis:
			verify(Type==0);
			wsprintf(dst,L"Axis #%d",id);
			break;
		case IET_Button:
			verify(Type!=0);
			if (Type==1)
				wsprintf(dst,L"POV #%d,%s",tid/4,PovDirections[tid%4]);
			else
				wsprintf(dst,L"Button #%d",tid);
			break;
		case IET_None:
			wsprintf(dst,L"None (%d,%d,%d)",id,tid,Type);
			break;
		}
	}
	virtual void Pool()
	{
		dev->Poll();
		DIJOYSTATE js;
		dev->GetDeviceState(sizeof(DIJOYSTATE), &js);
		
		u32 evid=0;
		for (u32 i=0;i<caps.dwAxes;i++)
		{
			if (i<6)
				EventMap[evid].Value=((LONG*)&js)[i];
			else
				EventMap[evid].Value=js.rglSlider[i-6];
			evid++;
		}

		for (u32 i=0;i<caps.dwPOVs;i++)
		{
			DWORD pv=js.rgdwPOV[i];
			
			float x_p,y_p;
			if (pv==0xFFFFFFFF)
			{
				x_p=0;y_p=0;
			}
			else
			{
				float rads=3.14159265f*pv/18000.0f;
				x_p=cosf(rads);
				y_p=sinf(rads);
			}

			EventMap[evid].Value=x_p>0.3?1:0;evid++;
			EventMap[evid].Value=x_p<-0.3?1:0;evid++;
			EventMap[evid].Value=y_p>0.3?1:0;evid++;
			EventMap[evid].Value=y_p<-0.3?1:0;evid++;
		}
		for (u32 i=0;i<caps.dwButtons;i++)
		{
			EventMap[evid].Value=js.rgbButtons[i]==0?0:1;
			evid++;
		}
		verify(EventCount==evid);
	}
};


BOOL WINAPI EnumDevsCB(LPCDIDEVICEINSTANCE lpddi, LPVOID Ref)
{
	Providers.push_back(new GenericDInputProvider(lpddi,DI8DEVCLASS_GAMECTRL));
	return DIENUM_CONTINUE;
}
class DInput : public InitListItem
{
	bool Init()
	{
		if(FAILED(DirectInput8Create(hInstance,DIRECTINPUT_VERSION,IID_IDirectInput8,(LPVOID*)&di8,0)))
			return false;

		if(DI_OK != di8->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumDevsCB,0, DIEDFL_ATTACHEDONLY))	{	return false;	}

		return true;
	}

	void Term()
	{
		di8->Release();
		di8=0;
	}
};

ListItem<DInput> DInputsctor (_InitList);