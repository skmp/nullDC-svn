#include "Profiles.h"
#include "InputProviders.h"

struct ProfileData
{
	GUID mdev;
	GUID prguid;
	wstring Name;
};

struct SubProfileImpl : SubProfile
{
	ProfileData* parent;
	wstring Name;
	bool Dirty;
	u32 id;

	struct mapping { wstring name;u32 p1;u32 p2;};
	vector<mapping> maps;
	
	SubProfileImpl(u32 id,ProfileData* parent)
	{
		this->id=id;
		this->parent=parent;
	}
	void GetName(wstring* name) const
	{
		*name=Name;
	}
	void SetName(const wstring& name) 
	{
		Dirty=true;
		Name=name;
	}

	u32 AddMap(const wstring& name,u32 param1,u32 param2)
	{
		Dirty=true;
		mapping t = { wstring(name),param1,param2 };
		maps.push_back(t);

		return (u32)maps.size()-1;
	}

	bool SetMapParams(u32 id,GUID device,u32 param1,u32 param2)
	{
		if (id>=maps.size())
			return false;
	
		Dirty=true;
		maps[id].p1=param1;
		maps[id].p2=param2;

		return true;
	}
	bool SetMapName(u32 id,const wstring& name)
	{
		if (id>=maps.size())
			return false;
		
		Dirty=true;
		maps[id].name=wstring(name);

		return true;
	}

	bool GetMapParams(u32 id,GUID* device,u32* param1,u32* param2)
	{
		if (id>=maps.size() || param1==0 || param2==0)
			return false;
		
		*param1=maps[id].p1;
		*param2=maps[id].p2;

		return true;
	}
	bool GetMapName(u32 id,wstring* name)
	{
		if (id>=maps.size())
			return false;

		*name=maps[id].name.c_str();

		return true;
	}

	bool IsDirty()
	{
		return Dirty;
	}
	void Revert()
	{
		if (Dirty)
			Read();
	}
	void Commit()
	{
		if (Dirty)
			Write();
	}
	bool Read()
	{
		wchar item[128];
		wstring value;
		u32 listsize=0;
		u32 p1,p2;
		wchar value2[2048];
		value2[0]=0;

		wstring sect=CFG_NAME L"_Profile_";
		sect+=GuidToText(parent->prguid);
		sect+=GuidToText(parent->mdev);

		wsprintf(item,L"s_%d_Info",id);
		value=ReadStr(sect,item);

		swscanf(value.c_str(),L"%[^:]:%d",value2,&listsize);
		Name=value2;
		for (u32 i=0;i<listsize;i++)
		{
			p1=p2=0;
			value2[0]=0;

			wsprintf(item,L"s_%d_map_%d",id,i);
			value=ReadStr(sect,item);
			
			swscanf(value.c_str(),L"%[^:]:%d,%d",value2,&p1,&p2);
			mapping mpi={value2,p1,p2};
			maps.push_back(mpi);
		}

		Dirty=false;
		return true;
	}
	void Write()
	{
		wchar item[128];
		wchar value[2048];

		wstring sect=CFG_NAME L"_Profile_";
		sect+=GuidToText(parent->prguid);
		sect+=GuidToText(parent->mdev);
		
		wsprintf(item,L"s_%d_Info",id);
		wsprintf(value,L"%s:%d",Name.c_str(),maps.size());

		WriteStr(sect,item,value);
		for (u32 i=0;i<maps.size();i++)
		{
			wsprintf(item,L"s_%d_map_%d",id,i);
			wsprintf(value,L"%s:%d,%d",maps[i].name.c_str(),maps[i].p1,maps[i].p2);

			WriteStr(sect,item,value);
		}
		Dirty=false;
	}
};

struct ProfileImpl : ProfileData,Profile
{
	vector<SubProfileImpl*> sprof;

	ProfileImpl() {}

	void GetName(wstring* name) const
	{
		*name=Name;
	}
	void SetName(const wstring& name) 
	{
		Name=name;
	}

	GUID GetGuid()
	{
		return prguid;
	}
	
	GUID GetMDevGuid()
	{
		return mdev;
	}

	SubProfile* AddSub(const wstring& name)
	{
		wchar t[32];
		swprintf(t,L"%d",sprof.size()+1);
		wstring sect=CFG_NAME L"_Profile_";
		sect+=GuidToText(prguid);
		sect+=GuidToText(mdev);
		WriteStr(sect,L"SubCount",t);

		SubProfileImpl* spi= new SubProfileImpl(sprof.size(),this);
		spi->Name=name;
		sprof.push_back(spi);
		spi->Commit();

		return spi;
	}
	SubProfile* GetSub(u32 id)
	{
		if (id >=sprof.size())
			return 0;
		else
			return sprof[id];
	}

	bool Read()
	{
		wstring sect=CFG_NAME L"_Profile_";
		sect+=GuidToText(prguid);
		sect+=GuidToText(mdev);
		wstring v=ReadStr(sect,L"SubCount");
		u32 c;
		swscanf(v.c_str(),L"%d",&c);

		for (u32 i=0;i<c;i++)
		{
			SubProfileImpl* spi = new SubProfileImpl(i,this);
			if (!spi->Read())
			{
				delete spi;
				return false;
			}
		}

		return true;
	}
};



vector<ProfileImpl*>	Profiles;

Profile* Profile::GetProfile(GUID mdev,GUID prguid,bool ret0)
{
	//try to find one
	for (u32 i=0;i<Profiles.size();i++)
	{
		if (Profiles[i]->mdev==mdev && Profiles[i]->prguid==prguid)
		{
			return Profiles[i];
		}
	}

	if (ret0)
		return 0;
	else
	{
		verify(prguid!=NullInputProvider);
		return GetProfile(mdev,NullInputProvider);
	}
}


ProfileImpl* CreateFixed(GUID mdev,GUID prguid)
{
	MapleDeviceDesc* mdd = FindMDF(mdev);
	if (!mdd)
		return 0;
	
	ProfileImpl* p= new ProfileImpl();
	p->mdev=mdev;
	p->Name=mdd->GetName();

	p->prguid=prguid;

	mdd->SetupProfile(p);
	Profiles.push_back(p);

	return p;
}
Profile* Profile::Create(GUID mdev)
{
	GUID g;
	CoCreateGuid(&g);
	ProfileImpl* p= CreateFixed(mdev,g);

	return p;
}

u32 ReadProfilesSub(const GUID& mdev)
{
	u32 rv=0;
	wstring sentry=L"Profiles_";
	sentry+=GuidToText(mdev);
	vector<wstring> vec=Tokenize(ReadStr(CFG_NAME,sentry),L",");//{guid1} {guid2},...

	for (u32 i=0;i<vec.size();i++)
	{
		if (vec[i].size()>=38)
		{
			wstring pure=JoinStr(Tokenize(vec[i]),L"");
			GUID pg=ParseGuid(pure.c_str());
			if (pg!=GUID_NULL)
			{
				//read the config :)
				ProfileImpl* p= new ProfileImpl();
				p->mdev=mdev;
				p->prguid=pg;
				if (!p->Read())
				{
					delete p;
					continue;
				}

				Profiles.push_back(p);
				rv++;
			}
		}
	}
	return rv;
}
void Profile::Init()
{
	for (u32 i=0;i<Devices.size();i++)
	{
		u32 rps=ReadProfilesSub(Devices[i]->GetGuid());
		if (Devices[i]->GetExtendedFlags()&1)
		{
			for (u32 j=0;j<Profiles.size();j++)
			{
				if (Profiles[j]->mdev==Devices[i]->GetGuid() && Profiles[j]->prguid!=GUID_NULL)
				{
					Profiles.erase(Profiles.begin()+j);
					j--;
					rps--;
				}
			}
			if (rps==0)
			{
				//Add default null config
				verify(CreateFixed(Devices[i]->GetGuid(),GUID_NULL)!=0);
			}
			continue;
		}
		for (u32 j=0;j<Providers.size();j++)
		{
			if (Profile* p=Profile::GetProfile(Devices[i]->GetGuid(),Providers[j]->GetGuid(),true))
			{
				//p->Destroy();//destroy the copy
			}
			else
			{
				verify(CreateFixed(Devices[i]->GetGuid(),Providers[j]->GetGuid())!=0);
			}
		}
	}
}
void Profile::Term()
{
	for (u32 i=0;i<Devices.size();i++)
	{
		vector<wstring> guids;
		for (u32 j=0;j<Profiles.size();j++)
		{
			if (Profiles[j]->mdev==Devices[i]->GetGuid())
			{
				guids.push_back(GuidToText(Profiles[j]->prguid));
			}
		}

		wstring sentry=L"Profiles_";
		sentry+=GuidToText(Devices[i]->GetGuid());
		WriteStr(CFG_NAME,sentry,JoinStr(guids,L","));
	}

	for (u32 i=0;i<Profiles.size();i++)
	{
		delete Profiles[i];
	}
	Profiles.clear();
}
