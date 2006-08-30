#pragma once
#include "common.h"

void mds_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
void mds_DriveGetTocInfo(TocInfo* toc,DiskArea area);
DiskType mds_DriveGetDiskType();
void mds_init();
void mds_term();
void mds_GetSessionsInfo(SessionInfo* sessions);