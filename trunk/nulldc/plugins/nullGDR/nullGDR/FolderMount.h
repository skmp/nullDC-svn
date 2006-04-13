#pragma once
#include "common.h"

void FM_ReadSector(u8 * buffer,u32 Sector,u32 count ,u32 type);
void FM_init();
void FM_term();
DiskType FM_DriveGetDiskType();
void FM_DriveGetTocInfo(mmTocInfo& toc,DiskArea area);