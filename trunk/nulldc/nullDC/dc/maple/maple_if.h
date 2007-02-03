#pragma once
#include "types.h"
#include "plugins/plugin_types.h"

extern maple_device_instance MapleDevices[4];

void maple_Init();
void maple_Reset(bool Manual);
void maple_Term();

void maple_vblank();