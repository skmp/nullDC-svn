#pragma once
#include "types.h"
#include "plugins/plugin_types.h"

#define key_CONT_C  (1 << 0);
#define key_CONT_B  (1 << 1);
#define key_CONT_A  (1 << 2);
#define key_CONT_START  (1 << 3);
#define key_CONT_DPAD_UP  (1 << 4);
#define key_CONT_DPAD_DOWN  (1 << 5);
#define key_CONT_DPAD_LEFT  (1 << 6);
#define key_CONT_DPAD_RIGHT  (1 << 7);
#define key_CONT_Z  (1 << 8);
#define key_CONT_Y  (1 << 9);
#define key_CONT_X  (1 << 10);
#define key_CONT_D  (1 << 11);
#define key_CONT_DPAD2_UP  (1 << 12);
#define key_CONT_DPAD2_DOWN  (1 << 13);
#define key_CONT_DPAD2_LEFT  (1 << 14);
#define key_CONT_DPAD2_RIGHT  (1 << 15);	

//need proper plugins
extern u16 kcode;
extern s8 joyx,joyy;
extern s8 joy2x,joy2y;
extern u8 rt,lt;
//ok ?

struct MapleDeviceLoadInfo
{
	char dll[512];
	VersionNumber	PluginVersion;
	char name[512];
	u8 id;
	u8 type;
};
List<MapleDeviceLoadInfo>* GetMapleMainDevices();
List<MapleDeviceLoadInfo>* GetMapleSubDevices();

void maple_Init();
void maple_Reset(bool Manual);
void maple_Term();
void maple_plugins_Term();
void maple_plugins_Init();
void maple_plugins_enum_devices();