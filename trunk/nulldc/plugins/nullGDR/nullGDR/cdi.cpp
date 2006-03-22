#ifdef X86
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
int TrackCount;
void cdi_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
	printf("GDR->Read : Sector %d , size %d , mode %d \n",StartSector,SectorCount,secsz);
}

void cdi_DriveGetTocInfo(TocInfo& toc,DiskArea area)
{
	toc=cdi_toc;
	/*
	//Send a fake a$$ toc
	//toc->last.full		= toc->first.full	= CTOC_TRACK(1);
	toc->first.number=1;
	toc->last.number=TrackCount;

	toc->first.ControlInfo=Tracks[0].bCtrl;
	toc->last.ControlInfo=Tracks[TrackCount-1].bCtrl;

	toc->first.Addr=0;
	toc->last.Addr=0;

	toc->lba_leadout.full=0;
	toc->lba_leadout.FAD=pstToc->dwOuterLeadOut;

 	//toc->entry[0].full	= CTOC_LBA(150) | CTOC_ADR(0) | CTOC_CTRL(4);
	toc->entry[0].Addr=0;
	toc->entry[0].ControlInfo=4;
	//toc->entry[1].Addr=0;
	//toc->entry[1].ControlInfo=4;

	u32 lba_start=0;
	for (int i=0;i<TrackCount;i++)
	{
		toc->entry[i].Addr=0;
		toc->entry[i].ControlInfo=Tracks[i].bCtrl;
		if (Tracks[i].dwIndexCount!=2)
			printf("Tracks[i].dwIndexCount!=2\n");
		toc->entry[i].FAD=lba_start;
		lba_start+=Tracks[i].pdwIndex[1];
	}
	toc->entry[2].FAD=45000;

	for (int i=TrackCount;i<99;i++)
	{
		toc->entry[i].full=0xFFFFFFFF;
	}*/
	
}
//TODO : fix up
DiskType cdi_DriveGetDiskType()
{
	return cdi_disktype;
}


void CreateToc()
{
	int track=0;
	bool CD_DA=false;
	bool CD_M1=false;
	bool CD_M2=false;

	cdi_ses.SessionCount=pstToc->dwSessionCount;
	cdi_ses.SessionsEndFAD=pstToc->dwOuterLeadOut;
	
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

	u32 last_FAD=0;
	for (u32 s=0;s<pstToc->dwSessionCount;s++)
	{
		SPfcSession* ses=&pstToc->pstSession[s];

		if (s!=0)
			last_FAD+=11400;//sext session
		cdi_ses.SessionFAD[s]=last_FAD;

		for (u32 t=0;t< ses->dwTrackCount ;t++)
		{
			SPfcTrack* cdi_track=&ses->pstTrack[t];
			track++;

			if (cdi_track->bMode==2)
				CD_M2=true;
			if (cdi_track->bMode==1)
				CD_M1=true;
			if (cdi_track->bMode==0)
				CD_DA=true;
			
			cdi_toc.tracks[track].Addr=0;//hmm is that ok ?
			
			cdi_toc.tracks[track].Control=cdi_track->bCtrl;
			cdi_toc.tracks[track].FAD=last_FAD;

			last_FAD+=cdi_track->pdwIndex[1];
		}
	}

	if (CD_M1 && CD_DA==false)
		cdi_disktype = DiskType::CdRom;
	else if (CD_M2)
		cdi_disktype = DiskType::CdRom_XA;
	else if (CD_DA && CD_M1) 
		cdi_disktype = DiskType::CdRom_Extra;
	else
		cdi_disktype=GdRom;//hmm?

	TrackCount=track;
}
void cdi_init()
{

	DWORD dwSize;//
	DWORD dwErr = PfcGetToc("F:\\ct2\\Crazy_Taxi_2_Usa_Dc-HOOLiGANS\\adasdad\\STC-CT2U.CDI", pstToc, dwSize);
    if (dwErr == PFCTOC_OK) 
	{
		CreateToc();
    }
	else
	{
		printf("Failed to open file , %d",dwErr);
	}

	DriveNotifyEvent(DriveEvent::DiskChange,0);
}

void cdi_term()
{
	if (pstToc)
		PfcFreeToc(pstToc);
}

void cdi_GetSessionsInfo(SessionInfo& sessions)
{
	sessions=cdi_ses;
	//pout[0]=2;//standby
	//pout[1]=0;//0's
	/*
	if (session==0)
	{
		pout[2]=2;//count of sessions
		pout[3]=(ti.lba_leadout.FAD>>16)&0xFF;//fad is sessions end
		pout[4]=(ti.lba_leadout.FAD>>8)&0xFF;
		pout[5]=(ti.lba_leadout.FAD>>0)&0xFF;
	}
	else
	{
		pout[2]=1;//track count on this session
		pout[3]=(ti.entry[session-1].FAD>>16)&0xFF;//fad is session start
		pout[4]=(ti.entry[session-1].FAD>>8)&0xFF;
		pout[5]=(ti.entry[session-1].FAD>>0)&0xFF;
	}*/
}
#endif