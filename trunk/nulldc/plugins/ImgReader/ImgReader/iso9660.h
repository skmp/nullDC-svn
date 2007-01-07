#pragma once
#include "common.h"


void iso_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
void iso_DriveGetTocInfo(TocInfo* toc,DiskArea area);
DiskType iso_DriveGetDiskType();
void iso_GetSessionsInfo(SessionInfo* sessions);
bool iso_init(char* file);
void iso_term(); 