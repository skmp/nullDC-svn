#include "MapleFunctions.h"

MapleFunction* CreateFunction0(MapleDevice* dev,u32 lparam,void* dparam);
void SetupProfile0(ProfileDDI* sp,u32 lparam,void* dparam);

MapleFunction* CreateFunction1(MapleDevice* dev,u32 lparam,void* dparam);

void MapleFunction::SetupProfile(ProfileDDI* sp,u32 function,u32 lparam,void* dparam)
{
	switch(function)
	{
	case MFID_0_Input:	//input
		SetupProfile0(sp,lparam,dparam);
		return;
	case MFID_1_Storage:	//storage
	case MFID_2_LCD:	//LCD
	case MFID_3_Clock:	//Clock
	case MFID_4_Mic:	//Mic
	case MFID_5_ARGun: //AR-gun
	case MFID_6_Keyboard: //Keyboard
	case MFID_7_LightGun: //Light Gun
	case MFID_8_PuruPuru: //Puru-Puru pack
	case MFID_9_Mouse: //Mouse
	case MFID_10_ExngData: //Exchange Media/External storage (wtf o.O)
	case MFID_11_Camera: //DreamEye
	default:
		return;
	}
}

MapleFunction* MapleFunction::Create(MapleDevice* dev,u32 function,u32 lparam,void* dparam)
{
	switch(function)
	{
	case MFID_0_Input:	//input
		return CreateFunction0(dev,lparam,dparam);
	case MFID_1_Storage:	//storage
		return CreateFunction1(dev,lparam,dparam);
	case MFID_2_LCD:	//LCD
	case MFID_3_Clock:	//Clock
	case MFID_4_Mic:	//Mic
	case MFID_5_ARGun: //AR-gun
	case MFID_6_Keyboard: //Keyboard
	case MFID_7_LightGun: //Light Gun
	case MFID_8_PuruPuru: //Puru-Puru pack
	case MFID_9_Mouse: //Mouse
	case MFID_10_ExngData: //Exchange Media/External storage (wtf o.O)
	case MFID_11_Camera: //DreamEye
	default:
		return 0;
	}
}