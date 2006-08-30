
#include "cdi.h"
#define BYTE u8
#define WORD u16
#define DWORD u32
#include "pfctoc.h"

// int main(int, char*)
// {
//   SPfcToc* pstToc;
//   DWORD dwSize;
// 
//   DWORD dwErr = PfcGetToc(_T("C:\\TEMP\\IMAGE1.CDI"), pstToc, dwSize);
//   if (dwErr == PFCTOC_OK) {
//     assert(IsBadReadPtr(pstToc, dwSize) == FALSE);
// 
//     //
//     // Do something with the TOC
//     //
// 
//     dwErr = PfcFreeToc(pstToc);
//   }
// 
//   return ((int)dwErr);

SPfcToc* pstToc;
SessionInfo cdi_ses;
TocInfo cdi_toc;
DiskType cdi_disktype;
struct file_TrackInfo
{
	u32 FAD;
	u32 Offset;
	u32 SectorSize;
};

file_TrackInfo Track[101];

int TrackCount;

u8 SecTemp[2352];
FILE* fp_cdi;
void cdi_ReadSSect(u8* p_out,u32 sector,u32 secsz)
{
	for (u32 i=0;i<TrackCount;i++)
	{
		if (Track[i+1].FAD>sector)
		{
			u32 fad_off=sector-Track[i].FAD;
			fseek(fp_cdi,Track[i].Offset+fad_off*Track[i].SectorSize,SEEK_SET);
			fread(SecTemp,Track[i].SectorSize,1,fp_cdi);

			ConvertSector(SecTemp,p_out,Track[i].SectorSize,secsz,sector);
			break;
		}
	}
}
void cdi_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
	//printf("GDR->Read : Sector %d , size %d , mode %d \n",StartSector,SectorCount,secsz);
	while(SectorCount--)
	{
		cdi_ReadSSect(buff,StartSector,secsz);
		buff+=secsz;
		StartSector++;
	}
}

void cdi_DriveGetTocInfo(TocInfo* toc,DiskArea area)
{
	memcpy(toc,&cdi_toc,sizeof(TocInfo));
}

DiskType cdi_DriveGetDiskType()
{
	return cdi_disktype;
}

void CreateToc()
{
	printf("TOC INFO\n");
	memset(Track,0xFF,sizeof(Track));
	int track=0;
	bool CD_DA=false;
	bool CD_M1=false;
	bool CD_M2=false;

	cdi_ses.SessionCount=pstToc->dwSessionCount;
	cdi_ses.SessionsEndFAD=pstToc->dwOuterLeadOut;
	
	printf("Last Sector : %d\n",pstToc->dwOuterLeadOut);
	printf("Session count : %d\n",pstToc->dwSessionCount);
	
	//0xFF sesion/toc
	for (int i=0;i<99;i++)
		cdi_ses.SessionFAD[i]=0xFFFFFFFF;

	for (int i=0;i<99;i++)
	{
		cdi_toc.tracks[i].Addr=0xFF;
		cdi_toc.tracks[i].Control=0xFF;
		cdi_toc.tracks[i].FAD=0xFFFFFFFF;
		cdi_toc.tracks[i].Session=0xFF;
	}

	cdi_toc.FistTrack=1;
	u32 last_FAD=0;
	u32 TrackOffset=0;

	for (u32 s=0;s<pstToc->dwSessionCount;s++)
	{
		printf("Session %d:\n",s);
		SPfcSession* ses=&pstToc->pstSession[s];

		printf("\tTrack Count: %d\n",ses->dwTrackCount);
		for (u32 t=0;t< ses->dwTrackCount ;t++)
		{
			SPfcTrack* cdi_track=&ses->pstTrack[t];

			//pre gap
			last_FAD	+=cdi_track->pdwIndex[0];
			TrackOffset	+=cdi_track->pdwIndex[0]*cdi_track->dwBlockSize;

			if (t==0)
			{
				cdi_ses.SessionFAD[s]=last_FAD;
				printf("\tSession start FAD: %d\n",last_FAD);
			}

			verify(cdi_track->dwIndexCount==2);
			printf("\ttrack %d:\n",t);
			printf("\t\t Type : %d:\n",cdi_track->bMode);

			if (cdi_track->bMode==2)
				CD_M2=true;
			if (cdi_track->bMode==1)
				CD_M1=true;
			if (cdi_track->bMode==0)
				CD_DA=true;
			
			

			cdi_toc.tracks[track].Addr=0;//hmm is that ok ?
			
			cdi_toc.tracks[track].Control=cdi_track->bCtrl;
			cdi_toc.tracks[track].FAD=last_FAD;


			Track[track].FAD=cdi_toc.tracks[track].FAD;
			Track[track].SectorSize=cdi_track->dwBlockSize;
			Track[track].Offset=TrackOffset;
			printf("\t\t Start FAD : %d:\n",Track[track].FAD);
			printf("\t\t SectorSize : %d:\n",Track[track].SectorSize);
			printf("\t\t File Offset : %d:\n",Track[track].Offset);

			printf("\t\t%d indexes \n",cdi_track->dwIndexCount);
			for (u32 i=0;i<cdi_track->dwIndexCount;i++)
			{
				printf("\t\t index %d : %d \n",i,cdi_track->pdwIndex[i]);
			}
			//main track data
			TrackOffset+=(cdi_track->pdwIndex[1])*cdi_track->dwBlockSize;
			last_FAD+=cdi_track->pdwIndex[1];
			track++;
		}
		last_FAD+=11400-150;///next session
	}

	if (CD_M1 && CD_DA==false)
		cdi_disktype = CdRom;
	else if (CD_M2)
		cdi_disktype = CdRom_XA;
	else if (CD_DA && CD_M1) 
		cdi_disktype = CdRom_Extra;
	else
		cdi_disktype=CdRom;//hmm?

	TrackCount=track;
	cdi_toc.LastTrack=track;
}

void cdi_init()
{
	char fn[512]="";
	GetFile(fn,"cdi images (*.cdi) \0*.cdi\0\0");
	DWORD dwSize;//
	DWORD dwErr = PfcGetToc(fn, pstToc, dwSize);
    if (dwErr == PFCTOC_OK) 
	{
		CreateToc();
    }
	else
	{
		printf("Failed to open file , %d",dwErr);
	}
	fp_cdi=fopen(fn,"rb");
	DriveNotifyEvent(DriveEvent::DiskChange,0);
}

void cdi_term()
{
	if (pstToc)
		PfcFreeToc(pstToc);
}

void cdi_GetSessionsInfo(SessionInfo* sessions)
{
	memcpy(sessions,&cdi_ses,sizeof(SessionInfo));
}