// drkMapleDevices.cpp : Defines the entry point for the DLL application.
//
#include <memory.h>
#include "nullMaple.h"
#include "Profiles.h"

//Config per device instance, each dcdev uses one device instance.

const GUID  NullInputProvider = { 0x481e6345, 0xc2da, 0x4ef5, { 0xb8, 0x19,  0x5d,  0xf8,  0x7d,  0x64,  0xe9,  0xe2 } };
const GUID  NullInputProviderDevice = { 0x4531ef4d, 0xc2da, 0x4ef5, { 0xb8, 0x19,  0x5d,  0xf8,  0x7d,  0x64,  0xe9,  0xe2 } };

vector<MapleDeviceFactory*>* _Devices;
vector<InitListItem*>*		 _InitList;

HINSTANCE hInstance;

emu_info host;
#ifdef UNICODE
#undef UNICODE
#endif

int htoi(wchar* s) 
{ 
	size_t len;
	int  value = 1, digit = 0,  total = 0;
	int c, x, y, i = 0;
	char hexchars[] = "abcdef"; /* Helper string to find hex digit values */

	/* Test for 0s, '0x', or '0X' at the beginning and move on */

	if (s[i] == '0')
	{
		i++;
		if (s[i] == 'x' || s[i] == 'X')
		{
			i++;
		}
	}

	len = wcslen(s);

	for (x = i; x < (int)len; x++)
	{
		c = tolower(s[x]);
		if (c >= '0' && c <= '9')
		{
			digit = c - '0';
		} 
		else if (c >= 'a' && c <= 'f')
		{
			for (y = 0; hexchars[y] != '\0'; y++)
			{
				if (c == hexchars[y])
				{
					digit = y + 10;
				}
			}
		} else {
			return 0; /* Return 0 if we get something unexpected */
		}
		//value =16;
		total |= digit <<((int)(len-x-1)*4);
	}
	return total;
}



s32 FASTCALL Load(emu_info* emu)
{
	memcpy(&host,emu,sizeof(host));
//	if (!_Providers)
//		_Providers=new vector<InputProvider*>();
	if (!_InitList)
		_InitList=new vector<InitListItem*>();
	for (size_t i=0;i<InitList.size();i++)
	{
		if (!InitList[i]->Init())
			printf("%d failed init\n",i);
	}

	Profile::Init();
	//Read from the config file
	ReadConfig();

	return rv_ok;
}

//called when plugin is unloaded by emu , olny if dcInitPvr is called (eg , not called to enumerate plugins)
void FASTCALL  Unload()
{
	Profile::Term();
	//Release configs
	//Release input devices
	
	//run term list items
	for (size_t i=InitList.size();i-->0;)
	{
		InitList[i]->Term();
	}
}


HMODULE hModule;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	::hModule=hModule;
	hInstance=(HINSTANCE)hModule;
    return TRUE;
}

//Device Callbacks, they redirect to classes now
template<typename DIT>
void FASTCALL Class_DMA(DIT* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
{
	((MapleDevice*)device_instance->data)->Dma(Command,buffer_in,buffer_in_len,buffer_out,buffer_out_len,responce);
}

template<typename DIT>
s32 FASTCALL CreateDev(DIT* inst,u32 id,u32 flags,u32 rootmenu)
{
	if (id>=Devices.size())	{	return rv_error;	}

	inst->dma=Class_DMA<DIT>;
	
	inst->data=Devices[id]->Create(inst);
	
	return rv_ok;
}
template<typename DIT>
s32 FASTCALL InitDev(DIT* inst,u32 id,maple_init_params* params)
{
	if (id>=Devices.size())	{	return rv_error;	}

	((MapleDevice*)inst->data)->Init();

	return rv_ok;
}
template<typename DIT>
void FASTCALL TermDev(DIT* inst,u32 id)
{
	((MapleDevice*)inst->data)->Term();
}
template<typename DIT>
void FASTCALL DestroyDev(DIT* inst,u32 id)
{
	((MapleDevice*)inst->data)->Destroy();//it also deletes itself :)
}

//End of Device Callbacks
#define MDD(name,type,flags) \
	wcscpy(km.devices[mdi].Name,name);	\
	wcscat(km.devices[mdi].Name,L"(" _T(__DATE__) L")");\
	km.devices[mdi].Type=type;	\
	km.devices[mdi].Flags= flags;	\
	mdi++;

#define MDLE() km.devices[mdi].Type=MDT_EndOfList;

//Give a list of the devices to teh emu
void EXPORT_CALL dcGetInterface(plugin_interface* info)
{

#define km info->maple

#define c info->common
	
	info->InterfaceVersion=PLUGIN_I_F_VERSION;

	c.InterfaceVersion=MAPLE_PLUGIN_I_F_VERSION;

	c.Load=Load;
	c.Unload=Unload;
	c.Type=Plugin_Maple;
	c.PluginVersion=DC_MakeVersion(1,0,2,DC_VER_NORMAL);
	
	wcscpy(c.Name,L"nullDC Maple Devices (" _T(__DATE__) L")");

	km.CreateMain=CreateDev<maple_device_instance>;
	km.InitMain=InitDev<maple_device_instance>;
	km.TermMain=TermDev<maple_device_instance>;
	km.DestroyMain=DestroyDev<maple_device_instance>;

	km.CreateSub=CreateDev<maple_subdevice_instance>;
	km.InitSub=InitDev<maple_subdevice_instance>;
	km.TermSub=TermDev<maple_subdevice_instance>;
	km.DestroySub=DestroyDev<maple_subdevice_instance>;

	u32 mdi=0;
	for (size_t i=0;i<Devices.size();i++)
	{
		MDD(Devices[i]->GetName(),Devices[i]->GetType(),Devices[i]->GetFlags());
	}

	//list terminator :P
	MDLE();
}

GUID ParseGuid(const wchar* text)
{
	GUID rv=GUID_NULL;

	if (wcslen(text)!=38 || text[0]!='{' || text[37]!= '}')
		return GUID_NULL;
	
	wchar tmp[9];
	
	memcpy(tmp,&text[1],8*sizeof(wchar));
	tmp[8]=0;

	rv.Data1=htoi(tmp);

	memcpy(tmp,&text[1+8+1],4*sizeof(wchar));
	tmp[4]=0;

	rv.Data2=htoi(tmp);

	memcpy(tmp,&text[1+8+1+4+1],4*sizeof(wchar));
	tmp[4]=0;

	rv.Data3=htoi(tmp);

	memcpy(tmp,&text[1+8+1+4+1+4+1],2*sizeof(wchar));
	tmp[2]=0;

	rv.Data4[0]=htoi(tmp);

	memcpy(tmp,&text[1+8+1+4+1+4+1+2],2*sizeof(wchar));
	tmp[2]=0;

	rv.Data4[1]=htoi(tmp);

	for (int i=0;i<6;i++)
	{
		memcpy(tmp,&text[1+8+1+4+1+4+1+2+2+1+i*2],2*sizeof(wchar));
		tmp[2]=0;

		rv.Data4[i+2]=htoi(tmp);
	}

	return rv;
}
//{6F1D2B60-D5A0-11CF-BFC7-444553540000}
void GuidToText(wchar* text,const GUID& guid)
{
	wsprintf(text,L"{%08X-%04X-%04X-%04X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1,guid.Data2,guid.Data3,(guid.Data4[0]<<8) |guid.Data4[1]
		,guid.Data4[2],guid.Data4[3],guid.Data4[4],guid.Data4[5]
		,guid.Data4[6],guid.Data4[7]);
}
u32 GetMapleSubPort(u32 addr)
{
	for (int i=0;i<6;i++)
	{
		if ((1<<i)&addr)
			return i;
	}
	return 0;
}

MapleDeviceFactory* FindMDF(const GUID& mdev)
{
	for (u32 i=0;i<Devices.size();i++)
	{
		if (Devices[i]->GetGuid()==mdev)
			return Devices[i];
	}

	return 0;
}


void ReadConfig()
{
	//InputProvider::ReadConfig();
//	ConfigMap::ReadConfig();
}

void WriteConfig()
{
	//InputProvider::WriteConfig();
	//ConfigMap::SaveConfig();
}

GUID ReadGuid(GUID obj,u32 port)
{
	return ParseGuid(ReadStr(obj,port).c_str());
}
void ReadGuid(GUID obj,u32 port,GUID& data)
{
	data=ReadGuid(obj,port);
}
void WriteGuid(GUID obj,u32 port,const GUID& data)
{
	WriteStr(obj,port,GuidToText(data));
}

wstring ReadStr(GUID obj,u32 port)
{
	wchar buff[2048]=L"Port_%2X_";
	wchar fbuff[2048];
	wstring w=GuidToText(obj);
	wcscat(buff,w.c_str());
	wsprintf(fbuff,buff,port);
	
	return ReadStr(CFG_NAME,fbuff);
}
void ReadStr(GUID obj,u32 port,wstring& data)
{
	data=ReadStr(obj,port);
}
void WriteStr(GUID obj,u32 port,const wstring& data)
{
	wchar buff[2048]=L"Port_%2X_";
	wchar fbuff[2048];
	wstring w=GuidToText(obj);
	wcscat(buff,w.c_str());
	wsprintf(fbuff,buff,port);
	
	WriteStr(CFG_NAME,fbuff,data);
}

wstring ReadStr(const wstring& sect,const wstring& item)
{
	wchar buff[2048];
	host.ConfigLoadStr(sect.c_str(),item.c_str(),buff,L"");
	return buff;
}
void ReadStr(const wstring& sect,const wstring& item,wstring& data)
{
	data=ReadStr(sect,item);
}
void WriteStr(const wstring& sect,const wstring& item,const wstring& data)
{
	host.ConfigSaveStr(sect.c_str(),item.c_str(),data.c_str());
}

void JoinStr(const vector<wstring>& tokens,wstring& rv,const wstring& delimiter)
{
	for (u32 i=0;i<tokens.size();i++)
	{
		rv+=tokens[i];
		if (i+1!=tokens.size())
			rv+=delimiter;
	}
}
void Tokenize(const wstring& str,vector<wstring>& tokens,const wstring& delimiters)
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
