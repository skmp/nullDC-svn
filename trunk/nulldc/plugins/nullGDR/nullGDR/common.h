#pragma once
#include "gd_driver.h"


//these are converted to Big endian by gd handling code :)
struct natTocEntryInfo
{
	union
	{
		struct
		{
			u32 ControlInfo:4;
			u32 Addr:4;
			u32 FAD:24;
		};
		u32 full;
	};
};

struct natTocStartEndTrackInfo
{
	union
	{
		struct
		{
			u32 ControlInfo:4;
			u32 Addr: 4;
			u32 number: 8;
			u32 res: 16;
		};
		u32 full;
	};
};



struct natTocInfo
{
  natTocEntryInfo entry[99];
  natTocStartEndTrackInfo first, last;
  natTocEntryInfo lba_leadout;
};

void ConvToc(u32* to,natTocInfo* from);

struct TocTrackInfo
{
	u32 FAD;	//fad , intel format
	u8 Control;	//cotnrol info
	u8 Addr;	//addr info
	u8 Session; //Session where teh track belongs
};
struct TocInfo
{
	//0-98 ->1-99
	TocTrackInfo tracks[99];

	u8 FistTrack;
	u8 LastTrack;

	TocTrackInfo LeadOut;	//session set to 0 on that one
};

struct SessionInfo
{
	u32 SessionsEndFAD;	//end of disk (?)
	u8 SessionCount;	//must be at least 1
	u32 SessionFAD[98];	//for sessions 1-99 ;)
};

typedef void InitFP();
typedef void TermFP();
typedef void DriveGetTocInfoModFP(TocInfo& toc,DiskArea area);
typedef void DriveGetSessionInfoModFP(SessionInfo& ses);

enum gd_drivers
{
	none=-1,
	Iso=0,
	#ifdef X86
	//cdi is available olny on x86 :)
	cdi=1,
	#endif
	FM=2
};

struct DriveIF
{
	DriveReadSectorFP*  ReadSector;
	DriveGetTocInfoModFP*  GetToc;
	DriveGetDiskTypeFP* GetDiskType;
	DriveGetSessionInfoModFP*GetSessionInfo;
	InitFP*				Init;
	TermFP*				Term;

	bool Inited;
	gd_drivers driver;
};


extern DriveIF* CurrDrive;
extern DriveNotifyEventFP* DriveNotifyEvent;

bool ConvertSector(u8* in_buff , u8* out_buff , int from , int to,int sector);

void SetDrive(gd_drivers driver);
void TermDrive();

#define DIR_FLAG            0x00000010  

typedef void FileFoundCB(char* fullpath,char* file,void* param,u32 flags);
void FindAllFiles(FileFoundCB* callback,char* dir,void* param);

void GetDriveToc(u32* to,DiskArea area);
void GetDriveSessionInfo(u8* to,u8 session);

