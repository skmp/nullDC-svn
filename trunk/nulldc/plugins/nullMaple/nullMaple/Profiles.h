#pragma once
#include "nullMaple.h"
/*
	Controller settings are stored on profiles.
	A profile can be identified by its profile GUID and a mdev GUID.
	A profile has a name, and an associated device list.
	Profiles hold user defined data in the Map%Name%={%values%} format.
*/

struct SubProfileDDI
{
	//Create a new map
	//The new map is saved to file before this function returns (no need to call Commit)
	virtual u32 AddMap(const wstring& name,u32 count)=0;
};

//The basic profile interface
//It can refer to a full profile, or to a subprofile
struct SubProfile
{	
	//Set mapping info

	virtual bool SetStr(u32 id,const wstring& string)=0;
	virtual bool GetStr(u32 id,wstring* const string)=0;

	//Get Mapping info
	virtual bool GetArr(u32 id,u32* const params,u32 count)=0;
	virtual bool SetArr(u32 id,const u32* const params,u32 count)=0;

	//Returns true if the Profile isnt on sync with the config file.Else returns false.
	virtual bool IsDirty()=0;
	//if the profile is dirty it discards the changes to it
	virtual void Revert()=0;
	//if the profile is dirty it saves the changes on the config file
	virtual void Commit()=0;

	virtual u32 GetSPRev()=0;
};
struct ProfileDDI
{
	virtual SubProfileDDI* AddSub(const wstring& title)=0;
};
struct Profile
{
	virtual SubProfile* GetSub(u32 id)=0;

	//Returns true if the Profile isnt on sync with the config file.Else returns false.
	virtual bool IsDirty()=0;
	//if the profile is dirty it discards the changes to it
	virtual void Revert()=0;
	//if the profile is dirty it saves the changes on the config file
	virtual void Commit()=0;

	//Get the profile id
	virtual u32 GetPCID()=0;
	virtual wstring GetHint()=0;
	
	static Profile* GetProfile(const u32 PTID,const wstring& hint=L"");
	static Profile* Create(const u32 PTID,const wstring& hint=L"");
	static Profile* GetOrCreate(const u32 PTID,const wstring& hint=L"") { Profile* rv=GetProfile(PTID,hint);if (!rv) rv=Create(PTID,hint); return rv; }
	static void Init();
	static void Term();
};

