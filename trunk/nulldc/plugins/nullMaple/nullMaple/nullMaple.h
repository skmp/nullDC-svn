#pragma once
#include "..\..\..\nullDC\plugins\plugin_header.h"

#define PLUGIN_NAME_CFG L"nullMaple"

#define _WIN32_WINNT 0x500
#define CFG_NAME L"nullMaple"
#include <windowsx.h>
#include <winsock2.h>
#include <windows.h>
#include <tchar.h>

#include <ws2tcpip.h>
#include <windowsx.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <commctrl.h>
#include "resource.h"

#include <vector>
using namespace std;

#define dbgbreak {while(1) __noop;}
#define verify(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define die(reason) { msgboxf(L"Fatal error : %s\n in %s -> %s : %d \n",MB_ICONERROR,_T(reason),_T(__FUNCTION__),_T(__FILE__),__LINE__); dbgbreak;}

extern const GUID  NullInputProvider;
extern const GUID  NullInputProviderDevice;
extern emu_info host;
extern HINSTANCE hInstance; 

//---------------------
class OptionGroopCallBack
{
public:
	virtual void OnMenuClick(int val,void* pUser,void* window)=0;
};
class OptionGroop
{
private:
	void Add(bool n,u32 id,int val,wchar* ex_name,void*pUser) { itempair t={id,val,ex_name,pUser}; items.push_back(t); }
public:
	OptionGroop(OptionGroopCallBack* callb)
	{
		root_menu=0;
		format=0;
		callbackobj=callb;
		callback=0;
	}
	u32 root_menu;
	wchar* format;
	struct itempair { u32 id;int value;wchar* ex_name;void* pUser;};
	vector<itempair> items;

	void (*callback) (int val,void* pUser,void* window) ;
	OptionGroopCallBack* callbackobj;

	
	void Add(u32 root,wchar* name,int val,void* pUser,wchar* ex_name=0,int style=0) 
	{ 
		if (root_menu==0)
			root_menu=root;
		u32 ids=host.AddMenuItem(root,-1,name,handler,0);
		host.SetMenuItemStyle(ids,style,style);
		
		MenuItem t;
		t.PUser=this;
		host.SetMenuItem(ids,&t,MIM_PUser);

		Add(false,ids,val,ex_name,pUser);
	}
	void Add(wchar* name,int val,void* pUser,wchar* ex_name=0,int style=0) 
	{ 
		Add(root_menu,name,val,pUser,ex_name,style);
	}

	static void EXPORT_CALL handler(u32 id,void* win,void* puser)
	{
		OptionGroop* pthis=(OptionGroop*)puser;
		pthis->handle(id,win);
	}
	void SetValue(int val,void* w)
	{
		void* pUser=0;
		for (u32 i=0;i<items.size();i++)
		{
			if (val==items[i].value)
			{
				host.SetMenuItemStyle(items[i].id,MIS_Checked,MIS_Checked);
				pUser=items[i].pUser;
				if (root_menu && format)
				{
					MenuItem t;
					host.GetMenuItem(items[i].id,&t,MIM_Text);
					wchar temp[512];
					wsprintf(temp,format,items[i].ex_name==0?t.Text:items[i].ex_name);
					t.Text=temp;
					host.SetMenuItem(root_menu,&t,MIM_Text);
				}
			}
			else
				host.SetMenuItemStyle(items[i].id,0,MIS_Checked);
		}
		if (callback)
			callback(val,pUser,w);
		if (callbackobj)
			callbackobj->OnMenuClick(val,pUser,w);
	}
	void handle(u32 id,void* w)
	{
		int val=0;
		for (u32 i=0;i<items.size();i++)
		{
			if (id==items[i].id)
			{
				val=items[i].value;
			}
		}

		SetValue(val,w);
	}

};


//
//Maple Devices
struct MapleDevice;
struct Profile;
//maple Devices descriptors
struct MapleDeviceDesc
{
	virtual const wchar* GetName() const=0;

	virtual MapleDeviceType GetType() const=0;
	virtual u32 GetFlags() const=0;
	virtual u32 GetExtendedFlags() const=0;	//1 means dev. doesnt use iproviders
	virtual GUID GetGuid() const=0;
	virtual void SetupProfile(Profile* prof) const =0;
};
struct MapleDeviceFactory : virtual MapleDeviceDesc
{
	virtual MapleDevice* Create(maple_device_instance* inst)=0;
	virtual MapleDevice* Create(maple_subdevice_instance* inst)=0;
};

//
struct MapleDeviceDesc;
struct MapleDevice : virtual MapleDeviceDesc
{
	virtual void Init()=0;
	virtual void Term()=0;
	virtual void Destroy()=0;	//this should also delete the object

	virtual void Dma(u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce) =0;

	virtual u8 GetPort() const =0;
};

//Lists
struct InitListItem
{
	virtual bool Init() =0;
	virtual void Term() =0;
};

//
#define Devices (*_Devices)
#define Providers (*_Providers)
#define InitList (*_InitList)

extern vector<MapleDeviceFactory*>*_Devices;
extern vector<InitListItem*>*	_InitList;
//A nice trick to add elements staticaly ;)
template<class T>
class ListItem
{
public:
	template<class Tx>
	ListItem(Tx* &lst) { if (!lst) lst = new Tx(); lst->push_back(new T());}
};

GUID ParseGuid(const wchar* text);
void GuidToText(wchar* text,const GUID& guid);
inline wstring GuidToText(const GUID& guid) { wchar rv[39];GuidToText(rv,guid);return rv;}

u32 GetMapleSubPort(u32 addr);
//0 if not known
//duplicate instances are possible, thats why you must call Release instead of delete :)
//InputProvider* GetInputProvider(const GUID& instance);
MapleDeviceFactory* FindMDF(const GUID& mdev);

int msgboxf(wchar* text,unsigned int type,...);
void ReadConfig();
void WriteConfig();

GUID ReadGuid(GUID obj,u32 port);
void ReadGuid(GUID obj,u32 port,GUID& data);
void WriteGuid(GUID obj,u32 port,const GUID& data);


wstring ReadStr(GUID obj,u32 port);
void ReadStr(GUID obj,u32 port,wstring& data);
void WriteStr(GUID obj,u32 port,const wstring& data);

wstring ReadStr(const wstring& sect,const wstring& item);
void ReadStr(const wstring& sect,const wstring& item,wstring& data);
void WriteStr(const wstring& sect,const wstring& item,const wstring& data);

void Tokenize(const wstring& str,vector<wstring>& tokens,const wstring& delimiters = L" ");
inline vector<wstring> Tokenize(const wstring& str,const wstring& delimiters = L" ") { vector<wstring> rv;Tokenize(str,rv,delimiters); return rv; }
void JoinStr(const vector<wstring>& tokens,wstring& str,const wstring& delimiter = L" ");
inline wstring JoinStr(const vector<wstring>& tokens,const wstring& delimiter = L" ") { wstring rv;JoinStr(tokens,rv,delimiter);return rv;}
