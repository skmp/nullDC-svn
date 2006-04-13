#pragma once
#include "common.h"


void iso_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
void iso_DriveGetTocInfo(mmTocInfo& toc,DiskArea area);
DiskType iso_DriveGetDiskType();
void iso_init();
void iso_term();