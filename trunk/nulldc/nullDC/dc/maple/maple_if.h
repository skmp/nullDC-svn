#pragma once
#include "types.h"
#include "plugins/plugin_types.h"

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
void maple_vblank();