#include "InputProviders.h"
vector<InputProvider*>*	_Providers;

///////////////////////////////////////////////
/////////////Null Input Provider !/////////////
///////////////////////////////////////////////

struct nillInputProvider : InputProvider
{
	virtual const wchar* GetName() const
	{
		return L"No Input";
	}
	virtual const GUID GetGuid() const
	{
		return GUID_NULL;
	}

	virtual const wchar* GetDeviceName() const
	{
		return L"No device";
	}
	virtual const GUID GetDeviceGuid() const
	{
		return GUID_NULL;
	}
		
	virtual u32 GetType() const
	{
		return IPT_Null;
	}

	virtual s32 GetEventValue(u32 id)
	{
		//die("This should never be called");
		return 0;
	}
	virtual InputEventType GetEventType(u32 id) 
	{ 
		//die("This should never be called");
		return IET_None; 
	} 

	virtual u32 GetEventCount() const { return 0; }

	

	virtual void Init()
	{
	}
	virtual void Term()
	{
	}
	virtual void Release()
	{
	}

	virtual void Pool()
	{
	}
	virtual void GetEventName(u32 id,wchar* dst)
	{
		
	}

};
//CoCreateGuid 
ListItem<nillInputProvider> nillInputProviderInfo_sctor(_Providers);

//0 if not known
//duplicate instances are possible, thats why you must call Release instead of delete :)
InputProvider* InputProvider::Find(const GUID& instance)
{
	for (size_t i=0;i<Providers.size();i++)
	{
		if (Providers[i]->GetGuid()==instance)
		{
			Providers[i]->Found=1;
			return Providers[i];
		}
	}
	return 0;
}
void InputProvider::CreateMissing()
{
	for (size_t i=0;i<Providers.size();i++)
	{
		if (Providers[i]->Found==0)
		{
			//ConfigMap::Create(Providers[i]);
		}
	}
}
