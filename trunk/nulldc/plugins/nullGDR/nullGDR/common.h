#pragma once
#include "gd_driver.h"

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
typedef void DriveGetTocInfoModFP(TocInfo* toc,DiskArea area);
typedef void DriveGetSessionInfoModFP(SessionInfo* ses);

enum gd_drivers
{
	none=-1,
	Iso=0,
	cdi=1,
	mds=2,
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

void ConvToc(u32* to,TocInfo* from);
void GetDriveToc(u32* to,DiskArea area);
void GetDriveSessionInfo(u8* to,u8 session);
void GetFile(TCHAR *szFileName, TCHAR *szParse);