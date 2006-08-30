
#pragma once
#include "common.h"

void cdi_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
void cdi_DriveGetTocInfo(TocInfo* toc,DiskArea area);
DiskType cdi_DriveGetDiskType();
void cdi_init();
void cdi_term();
void cdi_GetSessionsInfo(SessionInfo* sessions);