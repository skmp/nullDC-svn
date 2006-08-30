#include "cdi.h"
#include "mds_reader.h"


SessionInfo mds_ses;
TocInfo mds_toc;
DiskType mds_disktype=CdRom;

struct file_TrackInfo
{
	u32 FAD;
	u32 Offset;
	u32 SectorSize;
};

file_TrackInfo mds_Track[101];
FILE* fp_mdf=0;
u8 mds_SecTemp[5120];
u32 mds_TrackCount;
void mds_ReadSSect(u8* p_out,u32 sector,u32 secsz)
{
	for (u32 i=0;i<mds_TrackCount;i++)
	{
		if (mds_Track[i+1].FAD>sector)
		{
			u32 fad_off=sector-mds_Track[i].FAD;
			fseek(fp_mdf,mds_Track[i].Offset+fad_off*mds_Track[i].SectorSize,SEEK_SET);
			fread(mds_SecTemp,mds_Track[i].SectorSize,1,fp_mdf);

			ConvertSector(mds_SecTemp,p_out,mds_Track[i].SectorSize,secsz,sector);
			break;
		}
	}
}
void mds_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
	printf("MDS->Read : Sector %d , size %d , mode %d \n",StartSector,SectorCount,secsz);
	while(SectorCount--)
	{
		mds_ReadSSect(buff,StartSector,secsz);
		buff+=secsz;
		StartSector++;
	}
}

void mds_CreateToc()
{
	//clear structs to 0xFF :)
	memset(mds_Track,0xFF,sizeof(mds_Track));
	memset(&mds_ses,0xFF,sizeof(mds_ses));
	memset(&mds_toc,0xFF,sizeof(mds_toc));

	printf("\n--GD toc info start--\n");
	int track=0;
	bool CD_DA=false;
	bool CD_M1=false;
	bool CD_M2=false;

	mds_strack* last_track=&sessions[mds_nsessions-1].tracks[sessions[mds_nsessions-1].ntracks-1];

	mds_ses.SessionCount=mds_nsessions;
	mds_ses.SessionsEndFAD=last_track->sector+last_track->sectors+150;
	mds_toc.LeadOut.FAD=last_track->sector+last_track->sectors+150;
	mds_toc.LeadOut.Addr=0;
	mds_toc.LeadOut.Control=0;
	mds_toc.LeadOut.Session=0;

	printf("Last Sector : %d\n",mds_ses.SessionsEndFAD);
	printf("Session count : %d\n",mds_ses.SessionCount);

	mds_toc.FistTrack=1;
	

	for (int s=0;s<mds_nsessions;s++)
	{
		printf("Session %d:\n",s);
		mds_session* ses=&sessions[s];

		printf("  Track Count: %d\n",ses->ntracks);
		for (int t=0;t< ses->ntracks ;t++)
		{
			mds_strack* c_track=&ses->tracks[t];

			//pre gap
			
			if (t==0)
			{
				mds_ses.SessionFAD[s]=c_track->sector+150;
				printf("  Session start FAD: %d\n",mds_ses.SessionFAD[s]);
			}

			//verify(cdi_track->dwIndexCount==2);
			printf("  track %d:\n",t);
			printf("    Type : %d:\n",c_track->mode);

			if (c_track->mode==236)
				CD_M2=true;
			if (c_track->mode==236)
				CD_M1=true;
			if (c_track->mode==169)
				CD_DA=true;
			
			verify((c_track->mode==236) || (c_track->mode==169))
			

			mds_toc.tracks[track].Addr=0;//hmm is that ok ?
			mds_toc.tracks[track].Session=s;
			mds_toc.tracks[track].Control=c_track->mode==236?4:0;
			mds_toc.tracks[track].FAD=c_track->sector+150;


			mds_Track[track].FAD=mds_toc.tracks[track].FAD;
			mds_Track[track].SectorSize=c_track->sectorsize;
			mds_Track[track].Offset=(u32)c_track->offset;
			printf("    Start FAD : %d:\n",mds_Track[track].FAD);
			printf("    SectorSize : %d:\n",mds_Track[track].SectorSize);
			printf("    File Offset : %d:\n",mds_Track[track].Offset);

			//main track data
			track++;
		}
	}

	if ((CD_M1==true) && (CD_DA==false) && (CD_M2==false))
		mds_disktype = CdRom;
	else if (CD_M2)
		mds_disktype = CdRom_XA;
	else if (CD_DA && CD_M1) 
		mds_disktype = CdRom_Extra;
	else
		mds_disktype=CdRom;//hmm?

	mds_toc.LastTrack=track;
	mds_TrackCount=track;
	printf("--GD toc info end--\n\n");
}
bool mds_init(char* file)
{
	char fn[512]="";
	
	if (parse_mds(file,false)==false)
		return false;

	GetFile(fn,"mds images (*.mds) \0*.mdf\0\0");
	fp_mdf=fopen(fn,"rb");
	/*
	for(int j=0;j<mds_nsessions;j++)
	for(int i=0;i<sessions[j].ntracks;i++)
	{
		printf("Session %d Track %d mode %d/%d sector %d count %d offset %I64d\n",
			sessions[j].session_,
			sessions[j].tracks[i].track,
			sessions[j].tracks[i].mode,
			sessions[j].tracks[i].sectorsize,
			sessions[j].tracks[i].sector,
			sessions[j].tracks[i].sectors,
			sessions[j].tracks[i].offset);
	}*/
	
	mds_CreateToc();
	return true;
}
void mds_term()
{
	if (fp_mdf)
		fclose(fp_mdf);
	fp_mdf=0;
}

DiskType mds_DriveGetDiskType()
{
	return mds_disktype;
}
void mds_DriveGetTocInfo(TocInfo* toc,DiskArea area)
{
	verify(area==SingleDensity);
	memcpy(toc,&mds_toc,sizeof(TocInfo));
}
void mds_GetSessionsInfo(SessionInfo* sessions)
{
	memcpy(sessions,&mds_ses,sizeof(SessionInfo));
}