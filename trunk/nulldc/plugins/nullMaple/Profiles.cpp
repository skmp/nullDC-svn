#include "Profiles.h"
#include "InputProviders.h"

struct ProfileData
{
	u32 pid;
	wstring Section;
};

struct SubProfileImpl : SubProfile,SubProfileDDI
{
	ProfileData* parent;
	wstring Name;
	bool Dirty;
	bool Saved;
	u32 id;
	u32 rev;

	struct mapping { wstring name;u32 t;wstring v;u32 p[8];};
	vector<mapping> maps;
	
	SubProfileImpl(u32 id,ProfileData* const parent,const wstring& name)
	{
		this->id=id;
		this->parent=parent;
		Name=name;
		Dirty=false;
		Saved=false;
		rev=0;
	}
	u32 GetSPRev() { return rev; }
	u32 AddMap(const wstring & n,u32 count)
	{
		mapping m;
		m.name=n;
		m.t=count;
		
		if (m.t==0)
			m.v=L"";
		else
			memset(m.p,0,sizeof(m.p));

		maps.push_back(m);

		return (u32)maps.size()-1;
	}

	void GetName(wstring* const name) const
	{
		*name=Name;
	}
	////
	bool SetStr(u32 id,const wstring& string)
	{
		if (id>=maps.size() || maps[id].t!=0)
			return false;

		Dirty=true;
		rev++;
		maps[id].v=string;
		return true;
	}
	bool GetStr(u32 id,wstring* const string)
	{
		if (id>=maps.size() || maps[id].t!=0)
			return false;

		*string=maps[id].v;
		return true;
	}
	bool SetArr(u32 id,const u32* const params,u32 count)
	{
		if (id>=maps.size() || maps[id].t==0)
			return false;
	
		Dirty=true;
		for (u32 i=0;i<count;i++)
		{
			maps[id].p[i]=params[i];
		}
		rev++;
		return true;
	}
	bool GetArr(u32 id,u32* const params,u32 count)
	{
		if (id>=maps.size() || params==0 || maps[id].t==0)
			return false;

		for (u32 i=0;i<count;i++)
		{
			params[i]=maps[id].p[i];
		}

		return true;
	}
	////
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
	////
	bool Read()
	{
		for (u32 i=0;i<maps.size();i++)
		{
			wstring en=Name;
			en+=L"_";
			en+= maps[i].name;
			if (host.ConfigExists(parent->Section.c_str(),en.c_str())!=2)
			{
				if (maps[i].t==0)
					maps[i].v=L"";
				else
					memset(maps[i].p,0,sizeof(maps[i].p));
				continue;
			}

			wstring v=ReadStr(parent->Section,en);
			if (maps[i].t==0)
				maps[i].v=v;
			else
			{
				vector<wstring> strs=Tokenize(v,L",");
				for (u32 j=0;j<maps[i].t;j++)
				{
					if (strs.size()>j)
						swscanf(strs[i].c_str(),L"%d",&maps[i].p[j]);
					else
						maps[i].p[j]=0;
				}
			}
		}
		
		Dirty=false;
		rev++;
		return true;
	}
	void Write()
	{
		
		//Name_MapName=value(s)

		for (u32 i=0;i<maps.size();i++)
		{
			wstring en=Name;
			en+=L"_";
			en+= maps[i].name;

			if (maps[i].t==0)
			{
				WriteStr(parent->Section,en,maps[i].v);
			}
			else
			{
				wstring val;
				wchar tstr[10];
				for (u32 j=0;j<maps[i].t;j++)
				{
					swprintf_s(tstr,L"%d",maps[i].p[j]);
					
					if (j!=0)
						val+=L",";
					val+=tstr;
				}

				WriteStr(parent->Section,en,val);
			}
		}
		
		Dirty=false;
		Saved=true;
	}
};

struct ProfileImpl : ProfileData,Profile,ProfileDDI
{
	vector<SubProfileImpl*> sprof;

	void Init(u32 id)
	{
		Section=CFG_NAME L"_Profile";
		wchar t[10];
		swprintf(t,L"%d",id);
		Section+=t;

		SubProfileDDI* spd= AddSub(L"Profile");
		
		spd->AddMap(L"PTID",1);	//mdid|ftid
		spd->AddMap(L"FTDK",0);	//string
	}
	ProfileImpl(u32 id) 
	{
		Init(id);

		sprof[0]->Read();

		FindMDF(GetPCID()>>16)->SetupProfile(this,GetPCID()&0xFFFF);
		
		Read();
	}
	ProfileImpl(u32 id,u32 pcid,const wstring& ftdk) 
	{
		Init(id);

		sprof[0]->SetArr(0,&pcid,1);
		sprof[0]->SetStr(1,ftdk);
		sprof[0]->Commit();

		FindMDF(GetPCID()>>16)->SetupProfile(this,GetPCID()&0xFFFF);
	}

	bool Saved()
	{
		for (u32 i=0;i<sprof.size();i++)
		{
			if (sprof[i]->Saved)
				return true;
		}
		return false;
	}
	//Returns true if the Profile isnt on sync with the config file.Else returns false.
	virtual bool IsDirty()
	{
		for (u32 i=0;i<sprof.size();i++)
		{
			if (sprof[i]->IsDirty())
				return true;
		}
		return false;
	}
	//if the profile is dirty it discards the changes to it
	virtual void Revert()
	{
		for (u32 i=0;i<sprof.size();i++)
		{
			sprof[i]->Revert();
		}
	}
	//if the profile is dirty it saves the changes on the config file
	virtual void Commit()
	{
		for (u32 i=0;i<sprof.size();i++)
		{
			sprof[i]->Commit();
		}
	}

	u32 GetPCID()
	{
		u32 rv;
		sprof[0]->GetArr(0,&rv,1);
		return rv;
	}

	wstring GetHint()
	{
		wstring rv;
		sprof[0]->GetStr(1,&rv);
		return rv;
	}
	
	SubProfileDDI* AddSub(const wstring& name)
	{
		SubProfileImpl* spi= new SubProfileImpl((u32)sprof.size(),this,name);
		sprof.push_back(spi);

		return spi;
	}
	SubProfile* GetSub(u32 id)
	{
		id++;
		if (id >=sprof.size())
			return 0;
		else
			return sprof[id];
	}

	bool Read()
	{
		for (u32 i=0;i<sprof.size();i++)
		{
			if (!sprof[i]->Read())
			{
				return false;
			}
		}
		return true;
	}
};



vector<ProfileImpl*>	Profiles;

Profile* Profile::GetProfile(const u32 PTID,const wstring& hint)
{
	//try to find one
	for (u32 i=0;i<Profiles.size();i++)
	{
		if (Profiles[i]->GetPCID()==PTID && Profiles[i]->GetHint().compare(hint)==0)
		{
			return Profiles[i];
		}
	}

	return 0;
}

Profile* Profile::Create(const u32 PTID,const wstring& hint)
{
	ProfileImpl* p;
	
	Profiles.push_back((p=new ProfileImpl(Profiles.size(),PTID,hint)));

	return p;
}

void Profile::Init()
{
	int pcount=host.ConfigLoadInt(CFG_NAME,L"ProfileCount",0);
	
	for (int i=0;i<pcount;i++)
	{
		Profiles.push_back(new ProfileImpl(i));
	}
}
void Profile::Term()
{
		
	host.ConfigSaveInt(CFG_NAME,L"ProfileCount",Profiles.size());
	

	for (u32 i=0;i<Profiles.size();i++)
	{
		delete Profiles[i];
	}
	Profiles.clear();
}
