#pragma once
#include "nullGDR.h"

typedef void DriveRead(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
typedef void DriveGetToc(u32* toc,DiskArea area);

typedef DiskType DriveGetType();
typedef void DriveInit();
typedef void DriveTerm();