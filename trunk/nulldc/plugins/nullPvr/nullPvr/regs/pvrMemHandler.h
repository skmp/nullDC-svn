#pragma once
#include "..\nullPvr.h"

//TODO : move to plugin

u32 plugin_pvr_readreg_PowerVR(u32 addr,u32 sz);
void plugin_pvr_writereg_PowerVR(u32 addr,u32 data,u32 sz);
u32 plugin_pvr_readreg_TA(u32 addr,u32 sz);
void plugin_pvr_writereg_TA(u32 addr,u32 data,u32 sz);