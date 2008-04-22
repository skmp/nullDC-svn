#include "cdi.h"
#include "mds_reader.h"

//#include <wxp\winioctl.h>
//#include <wxp\ntddcdvd.h>
//#include <wxp\Ntddcdrm.h>

#ifndef noheaders
#define RAW_SECTOR_SIZE			2352
#define CD_SECTOR_SIZE			2048
#define MAXIMUM_NUMBER_TRACKS	100
#define SECTORS_AT_READ			20
#define CD_BLOCKS_PER_SECOND	75
#define IOCTL_CDROM_RAW_READ	0x2403E
#define IOCTL_CDROM_READ_TOC	0x24000
#define IOCTL_CDROM_READ_TOC_EX 0x24054

// These structures are defined somewhere in the windows-api, but I did
//   not have the include-file.
typedef struct _TRACK_DATA
{
	UCHAR Reserved;
	UCHAR Control : 4;
	UCHAR Adr : 4;
	UCHAR TrackNumber;
	UCHAR Reserved1;
	UCHAR Address[4];
} TRACK_DATA;

typedef struct _CDROM_TOC
{
	UCHAR Length[2];
	UCHAR FirstTrack;
	UCHAR LastTrack;
	TRACK_DATA TrackData[MAXIMUM_NUMBER_TRACKS];
} CDROM_TOC;

typedef enum _TRACK_MODE_TYPE
{
	YellowMode2,
	XAForm2,
	CDDA
} TRACK_MODE_TYPE, *PTRACK_MODE_TYPE;

typedef struct __RAW_READ_INFO
{
	LARGE_INTEGER  DiskOffset;
	ULONG  SectorCount;
	TRACK_MODE_TYPE  TrackMode;
} RAW_READ_INFO, *PRAW_READ_INFO;
typedef struct _CDROM_TOC_FULL_TOC_DATA_BLOCK {  UCHAR  SessionNumber;  UCHAR  Control:4;  UCHAR  Adr:4;  UCHAR  Reserved1;  UCHAR  Point;  UCHAR  MsfExtra[3];  UCHAR  Zero;  UCHAR  Msf[3];} CDROM_TOC_FULL_TOC_DATA_BLOCK, *PCDROM_TOC_FULL_TOC_DATA_BLOCK;
typedef struct _CDROM_TOC_FULL_TOC_DATA
{
	UCHAR  Length[2];
	UCHAR  FirstCompleteSession;
	UCHAR  LastCompleteSession;
	CDROM_TOC_FULL_TOC_DATA_BLOCK  Descriptors[0];
} CDROM_TOC_FULL_TOC_DATA, *PCDROM_TOC_FULL_TOC_DATA;
/* CDROM_READ_TOC_EX.Format constants */
  #define CDROM_READ_TOC_EX_FORMAT_TOC      0x00
  #define CDROM_READ_TOC_EX_FORMAT_SESSION  0x01
  #define CDROM_READ_TOC_EX_FORMAT_FULL_TOC 0x02
  #define CDROM_READ_TOC_EX_FORMAT_PMA      0x03
  #define CDROM_READ_TOC_EX_FORMAT_ATIP     0x04
  #define CDROM_READ_TOC_EX_FORMAT_CDTEXT   0x05
  
typedef struct _CDROM_READ_TOC_EX 
{
    UCHAR  Format : 4;
    UCHAR  Reserved1 : 3;
    UCHAR  Msf : 1;
    UCHAR  SessionTrack;
   UCHAR  Reserved2;
    UCHAR  Reserved3;
 } CDROM_READ_TOC_EX, *PCDROM_READ_TOC_EX;
#endif
ULONG msf2fad( UCHAR Addr[4] )
{
	ULONG Sectors = ( Addr[0] * (CD_BLOCKS_PER_SECOND*60) ) + ( Addr[1]*CD_BLOCKS_PER_SECOND) + Addr[2];
	return Sectors;
}


// Msf: Hours, Minutes, Seconds, Frames
ULONG AddressToSectors( UCHAR Addr[4] );


SessionInfo ioctl_ses;
TocInfo ioctl_toc;
DiscType ioctl_Disctype=CdRom;
HANDLE ioctl_handle;
void FASTCALL ioctl_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
	static RAW_READ_INFO Info={{0,0},1,XAForm2};
	//Info.TrackMode = XAForm2 ;
	//Info.SectorCount = 1;
	u8 temp[2500];
	for (u32 soff=0;soff<SectorCount;soff++)
	{
		Info.DiskOffset.QuadPart = (StartSector+soff-150) * CD_SECTOR_SIZE;
		ULONG Dummy;
		for (int tr=0;tr<3;tr++)
		{
			if ( 0 == DeviceIoControl( ioctl_handle, IOCTL_CDROM_RAW_READ, &Info, sizeof(Info), temp, RAW_SECTOR_SIZE, &Dummy, NULL ) )
			{
				Info.TrackMode=(TRACK_MODE_TYPE)((Info.TrackMode+1)%3);
				if (tr==3)
					printf("GDROM: Totaly failed to read sector @LBA %d\n",StartSector+soff-150);
			}
		}
		ConvertSector(temp,buff,2352,secsz,StartSector+soff);
		buff+=secsz;
	}

}
void ioctl_DriveGetTocInfo(TocInfo* toc,DiskArea area)
{
	memcpy(toc,&ioctl_toc,sizeof(*toc));
}
u32 FASTCALL ioctl_DriveGetDiscType()
{
	return ioctl_Disctype;
}
bool ioctl_init(wchar* file)
{
	if (wcslen(file)==3 && GetDriveType(file)==DRIVE_CDROM)
	{
		wprintf(L"Opening device %s ...",file);
		wchar fn[]={ L'\\', L'\\', L'.', L'\\', file[0], L':', L'\0' };
		if ( INVALID_HANDLE_VALUE == ( ioctl_handle = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL ) ) )
		{
			return false;//its there .. but wont open ...
		}
		wprintf(L" Opened device %s, reading TOC ...",fn);
		// Get track-table and add it to the intern array
		CDROM_READ_TOC_EX tocrq={0};
		 
	 	tocrq.Format = CDROM_READ_TOC_EX_FORMAT_FULL_TOC;
	 	tocrq.Msf=1;
	 	tocrq.SessionTrack=1;
		u8 buff[2048];
	 	CDROM_TOC_FULL_TOC_DATA *ftd=(CDROM_TOC_FULL_TOC_DATA*)buff;
	
		ULONG BytesRead;
		memset(buff,0,sizeof(buff));
		int code = DeviceIoControl(ioctl_handle,IOCTL_CDROM_READ_TOC_EX,&tocrq,sizeof(tocrq),ftd, 2048, &BytesRead, NULL);
		wprintf(L" Readed TOC\n");
		
//		CDROM_TOC toc;
		int currs=-1;
		if (0==code)
		{
			ioctl_Disctype=NoDisk;
		}
		else
		{
			ioctl_Disctype=CdRom_XA;
			memset(&ioctl_toc,0xFF,sizeof(ioctl_toc));
			memset(&ioctl_ses,0xFF,sizeof(ioctl_ses));

			ioctl_toc.FistTrack=1;
			ioctl_toc.LastTrack=0;
			ioctl_ses.SessionCount=0;

			BytesRead-=sizeof(CDROM_TOC_FULL_TOC_DATA);
			BytesRead/=sizeof(ftd->Descriptors[0]);

			for (u32 i=0;i<BytesRead;i++)
			{
				if (ftd->Descriptors[i].Point==0xA2)
				{
					ioctl_ses.SessionsEndFAD=msf2fad(ftd->Descriptors[i].Msf);
					continue;
				}
				if (ftd->Descriptors[i].Point>=1 && ftd->Descriptors[i].Point<=0x63 &&
					ftd->Descriptors[i].Adr==1)
				{
					u32 trackn=ftd->Descriptors[i].Point-1;
					ioctl_toc.tracks[trackn].Addr=ftd->Descriptors[i].Adr;
					ioctl_toc.tracks[trackn].Control=ftd->Descriptors[i].Control;
					ioctl_toc.tracks[trackn].FAD=msf2fad(ftd->Descriptors[i].Msf);
					ioctl_toc.tracks[trackn].Session=ftd->Descriptors[i].SessionNumber;
					ioctl_toc.LastTrack++;

					if (currs!=ftd->Descriptors[i].SessionNumber)
					{
						currs=ftd->Descriptors[i].SessionNumber;
						ioctl_ses.SessionCount++;
						ioctl_ses.SessionStart[currs-1]=trackn+1;
						ioctl_ses.SessionFAD[currs-1]=ioctl_toc.tracks[trackn].FAD;
					}
				}
			}
			ioctl_toc.LeadOut.FAD=ioctl_ses.SessionsEndFAD;
			ioctl_toc.LeadOut.Addr=0;
			ioctl_toc.LeadOut.Control=0;
			ioctl_toc.LeadOut.Session=0;
		}
		printtoc(&ioctl_toc,&ioctl_ses);
/*		if ( 0 == DeviceIoControl( ioctl_handle, IOCTL_CDROM_READ_TOC, NULL, 0, &toc, sizeof(toc), &BytesRead, NULL ) )
		{
			ioctl_Disctype=NoDisk;
		}
		ioctl_toc.FistTrack=toc.FirstTrack;
		ioctl_toc.LastTrack=toc.LastTrack;
		for ( ULONG i=toc.FirstTrack-1; i<toc.LastTrack; i++ )
		{
			ioctl_toc.tracks[i].Addr=toc.TrackData[i].Adr;
			ioctl_toc.tracks[i].Control=toc.TrackData[i].Control;
			ioctl_toc.tracks[i].FAD=msf2fad( toc.TrackData[i].Address);
		}
*/
		//done !
		return true;
	}
	else
		return false;
}
void ioctl_term()
{
	CloseHandle(ioctl_handle);
}
void ioctl_GetSessionsInfo(SessionInfo* sessions)
{
	memcpy(sessions,&ioctl_ses,sizeof(*sessions));
}