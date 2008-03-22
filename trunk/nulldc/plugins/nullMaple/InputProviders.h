#include "nullMaple.h"

////------------------
//
enum InputProviderType
{
	IPT_Keyboard=1,
	IPT_Mouse=2,
	IPT_Joystick=4,
	IPT_All=0x7FFFFFFF,
	IPT_Null=0x80000000,//This bit indicates the provider is null.Null providers typicaly have all the other bits set too , as they fall on any category

};

enum InputEventType
{
	IET_None,
	IET_Button,
	IET_Axis,
};
//input devices
class InputProvider
{
public:
	static InputProvider* Find(const GUID& guid);
	static void CreateMissing();

	u32 Found;
	InputProvider() { Found=0; }
	virtual const wchar* GetName() const=0;
	virtual const GUID GetGuid() const=0;
	
	virtual const wchar* GetDeviceName() const=0;
	virtual const GUID GetDeviceGuid() const=0;
	
	virtual u32 GetType() const=0;

	virtual void Init()=0;
	virtual void Term()=0;

	virtual void Pool()=0;
	virtual s32 GetEventValue(u32 id) =0; //-127 to 127 for axis ,0 .. 1 for buttons
	virtual InputEventType GetEventType(u32 id) =0;
	virtual void GetEventName(u32 id,wchar* dst) = 0;
	virtual u32 GetEventCount() const=0;
};

//config map :)
#define CONFIG_ID_NONE 0
class DreamcastMainDevice;
/*
class ConfigMap
{
public:
	//static ConfigMap* Create(InputFormat format);
	//Find a config map, or create a new one
	static ConfigMap* Get(u32 port);
	static ConfigMap* Find(const GUID& guid);
	static void Create(InputProvider* ip);
	static void ListMenuItems(OptionGroop* og,ConfigMap* selected);
	static void Set(DreamcastMainDevice*dev,ConfigMap* map);
	static void ReadConfig();
	static void SaveConfig();

	GUID guid;
	wchar name[128];
	GUID devguid;
	wchar devname[128];

	//do i need to explain these ?
	virtual bool NeedsConfig()=0;
	virtual void ShowConfig()=0;
	//True if config edited & not saved
	virtual bool IsDirty()=0;
	//for internal use only
	virtual bool WriteConfig(u32 id)=0;

	//fmt is only valid for joystick atm
	virtual u32 GetData(InputFormat fmt,u8* buffer) =0;
	
	//Initialise everything needed for using the config map
	virtual bool Init()=0;
	//term ..
	virtual void Term()=0;
	//Called when dll is about to be unloaded
	virtual void Release()=0;
};

*/
extern vector<InputProvider*>*  _Providers;
