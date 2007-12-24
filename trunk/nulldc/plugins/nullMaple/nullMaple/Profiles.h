#pragma once
#include "nullMaple.h"
/*
	Controller settings are stored on profiles.
	A profile can be identified by its profile GUID and a mdev GUID.
	A profile has a name, and an associated device list.
	Profiles hold user defined data in the Map%Name%={%values%} format.
*/

//The basic profile interface
//It can refer to a full profile, or to a subprofile
struct SubProfile
{	
	//Get/Set the name
	//NOTE: this is the name of the profile, not of the subprofile
	virtual void GetName(wstring* name) const=0;
	virtual void SetName(const wstring& name) = 0;

	//Create a new map
	//The new map is saved to file before this function returns (no need to call Commit)
	virtual u32 AddMap(const wstring& name,u32 count)=0;
	
	//Set mapping info
	virtual bool SetMapParams(u32 id,GUID device,u32* params,u32 count)=0;
	virtual bool SetMapName(u32 id,const wstring& name)=0;

	//Get Mapping info
	virtual bool GetMapParams(u32 id,GUID* device,u32* params,u32 count)=0;
	virtual bool GetMapName(u32 id,wstring* name)=0;

	//Returns true if the Profile isnt on sync with the config file.Else returns false.
	virtual bool IsDirty()=0;
	//if the profile is dirty it discards the changes to it
	virtual void Revert()=0;
	//if the profile is dirty it saves the changes on the config file
	virtual void Commit()=0;
};

struct Profile
{
	//unbuffered
	virtual SubProfile* AddSub(const wstring& title)=0;
	virtual SubProfile* GetSub(u32 id)=0;

	//buffered
	virtual void GetName(wstring* name) const=0;
	virtual void SetName(const wstring& name) = 0;

	//Get the profile guid
	virtual GUID GetGuid()=0;
	//Get the maple device guid
	virtual GUID GetMDevGuid()=0;
	
	static Profile* GetProfile(GUID mdev,GUID prguid,bool ret0=false);
	static Profile* Create(GUID mdev);
	static void Init();
	static void Term();
};

