#include "nullMaple.h"
#include "MapleFunctions.h"
#pragma warning (disable : 4250)

template<typename DevInfo>
struct MapleDevBase:  virtual MapleDevice,  virtual DevInfo
{
	vector<MapleFunction*> functs;

	MapleDeviceInfo mdi;
	u32 menu;
	u32 GetMenu() const{ return menu; }

	//functions must be added on correct order before calling this
	//Inits functions, mdi.funct and mdi.function_data
	bool InitFunc()
	{
		if (functs.size()>3)
			return false;
		mdi.func=0;

		for (size_t i=0;i<functs.size();i++)
		{
			mdi.func|=functs[i]->GetID();
			mdi.function_data[i]=functs[i]->GetDesc();
			if (!functs[i]->Init())
				return false;
		}

		return true;
	}
	void TermFunc()
	{
		for (size_t i=functs.size();i-->0;)
			functs[i]->Term();
	}
	void DestFunc()
	{
		for (size_t i=functs.size();i-->0;)
			functs[i]->Destroy();
		functs.clear();
	}

	virtual void MiscDma(u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
	{
		u8*buffer_out_b=(u8*)buffer_out;
		
		printf("UNKOWN MAPLE COMMAND %d\n",Command);
		responce=MDRE_UnkownCmd;

	}
	virtual void Dma(u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len,u32& responce)
	{
		u8*buffer_out_b=(u8*)buffer_out;

		switch(Command)
		{
			case MDC_DeviceRequest:
			{
				wbuff(mdi);

				responce=MDRS_DeviseStatus;
			}
			break;

			//FT dmas
			case MDCF_GetCondition:
			case MDCF_GetMediaInfo:
			case MDCF_BlockRead:
			case MDCF_BlockWrite:
			case MDCF_GetLastError:
			case MDCF_SetCondition:
			case MDCF_MICControl:
			case MDCF_ARGunControl:
				{
					if (buffer_in_len<4)
					{
						//invalid
						//best guess ...
						//Shoudnt happen anyway
						printf("nullMaple Error : Function depedant command that has no FT param.This shoudnt happen,ever...\n");
						responce=MDRE_UnkownCmd;
					}
					else
					{
						u32 FTD=*buffer_in;
						buffer_in++;
						buffer_in_len-=4;

						if ((mdi.func & FTD)==FTD)
						{
							size_t i;
							for (i=0;i<functs.size();i++)
							{
								if (FTD==functs[i]->GetID())
								{
									//let the function handle that dma :)
									functs[i]->Dma(Command,buffer_in,buffer_in_len,buffer_out,buffer_out_len,responce);
									break;
								}
							}
							
							if (i==functs.size())
							{
								//this deff. shoudnt happen unless there is some broadcast mechanism across 
							    //functions.
								printf("nullMaple Error : Function exists, but cant be resolved.This shoudnt happen,ever...\n");
								responce=MDRE_UnkownFunction;
							}
						}
						else
						{
							//we dont support that function
							responce=MDRE_UnkownFunction;
						}
					}
				}
				break;

			default:
				//Some misc dma or invalid value
				//Let the maple device handle it :)
				MiscDma(Command,buffer_in,buffer_in_len,buffer_out,buffer_out_len,responce);
				break;
		}
	}

};

template<typename DevInfo,typename DevImpl>
struct MapleDevMFactBase :  virtual DevInfo, virtual MapleDeviceFactory
{
	virtual MapleDevice* Create(maple_device_instance* inst,u32 menu)
	{
		DevImpl* rv = new DevImpl(inst,menu);
		return rv;
	}
	virtual MapleDevice* Create(maple_subdevice_instance* inst,u32 menu)
	{
		return 0;//not supported
	}
};
template<typename DevInfo,typename DevImpl>
struct MapleDevSFactBase : virtual DevInfo,virtual MapleDeviceFactory
{
	virtual MapleDevice* Create(maple_device_instance* inst,u32 menu)
	{
		return 0;//not supported
	}
	virtual MapleDevice* Create(maple_subdevice_instance* inst,u32 menu)
	{
		DevImpl* rv = new DevImpl(inst,menu);
		return rv;
	}
};