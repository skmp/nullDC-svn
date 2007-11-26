#include "cdi.h"
#include "mds_reader.h"


SessionInfo mds_ses;
TocInfo mds_toc;
DiscType mds_Disctype=CdRom;

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
void FASTCALL mds_DriveReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz)
{
//	printf("MDS/NRG->Read : Sector %d , size %d , mode %d \n",StartSector,SectorCount,secsz);
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

	strack* last_track=&sessions[nsessions-1].tracks[sessions[nsessions-1].ntracks-1];

	mds_ses.SessionCount=nsessions;
	mds_ses.SessionsEndFAD=last_track->sector+last_track->sectors+150;
	mds_toc.LeadOut.FAD=last_track->sector+last_track->sectors+150;
	mds_toc.LeadOut.Addr=0;
	mds_toc.LeadOut.Control=0;
	mds_toc.LeadOut.Session=0;

	printf("Last Sector : %d\n",mds_ses.SessionsEndFAD);
	printf("Session count : %d\n",mds_ses.SessionCount);

	mds_toc.FistTrack=1;
	

	for (int s=0;s<nsessions;s++)
	{
		printf("Session %d:\n",s);
		session* ses=&sessions[s];

		printf("  Track Count: %d\n",ses->ntracks);
		for (int t=0;t< ses->ntracks ;t++)
		{
			strack* c_track=&ses->tracks[t];

			//pre gap
			
			if (t==0)
			{
				mds_ses.SessionFAD[s]=c_track->sector+150;
				mds_ses.SessionStart[s]=track+1;
				printf("  Session start FAD: %d\n",mds_ses.SessionFAD[s]);
			}

			//verify(cdi_track->dwIndexCount==2);
			printf("  track %d:\n",t);
			printf("    Type : %d\n",c_track->mode);

			if (c_track->mode>=2)
				CD_M2=true;
			if (c_track->mode==1)
				CD_M1=true;
			if (c_track->mode==0)
				CD_DA=true;
			
			//verify((c_track->mode==236) || (c_track->mode==169))
			

			mds_toc.tracks[track].Addr=0;//hmm is that ok ?
			mds_toc.tracks[track].Session=s;
			mds_toc.tracks[track].Control=c_track->mode>0?4:0;//mode 1 , 2 , else are data , 0 is audio :)
			mds_toc.tracks[track].FAD=c_track->sector+150;


			mds_Track[track].FAD=mds_toc.tracks[track].FAD;
			mds_Track[track].SectorSize=c_track->sectorsize;
			mds_Track[track].Offset=(u32)c_track->offset;
			printf("    Start FAD : %d\n",mds_Track[track].FAD);
			printf("    SectorSize : %d\n",mds_Track[track].SectorSize);
			printf("    File Offset : %d\n",mds_Track[track].Offset);

			//main track data
			track++;
		}
	}

	//normal CDrom : mode 1 tracks .All sectors on the track are mode 1.Mode 2 was defined on the same book , but is it ever used? if yes , how can i detect
	//cd XA ???
	//CD Extra : session 1 is audio , session 2 is data
	//cd XA : mode 2 tracks.Form 1/2 are selected per sector.It allows mixing of mode1/mode2 tracks ?
	//CDDA  : audio tracks only <- thats simple =P
	/*
	if ((CD_M1==true) && (CD_DA==false) && (CD_M2==false))
		mds_Disctype = CdRom;
	else if (CD_M2)
		mds_Disctype = CdRom_XA;
	else if (CD_DA && CD_M1) 
		mds_Disctype = CdRom_Extra;
	else
		mds_Disctype=CdRom;//hmm?
	*/
	if (nsessions==1 && (CD_M1 | CD_M2))
		mds_Disctype = CdRom;		//hack so that non selfboot stuff works on utopia
	else
	{
		if ((CD_M1==true) && (CD_DA==false) && (CD_M2==false))
			mds_Disctype = CdRom;		//is that even correct ? what if its multysessions ? ehh ? what then ???
		else if (CD_M2)
			mds_Disctype = CdRom_XA;	// XA XA ! its mode 2 wtf ?
		else if (CD_DA && CD_M1) 
			mds_Disctype = CdRom_XA;	//data + audio , duno wtf as@!#$ lets make it _XA since it seems to boot
		else if (CD_DA && !CD_M1 && !CD_M2) 
			mds_Disctype = CdDA;	//audio
		else
			mds_Disctype=CdRom_XA;//and hope for the best
	}
/*
	bool data = CD_M1 | CD_M2;
	bool audio=CD_DA;

	if (data && audio)
		mds_Disctype = CdRom_XA;	//Extra/CdRom won't boot , so meh
	else if (data)
		mds_Disctype = CdRom;	//only data
	else
		mds_Disctype = CdDA;	//only audio
*/
	mds_toc.LastTrack=track;
	mds_TrackCount=track;
	printf("--GD toc info end--\n\n");
}
bool mds_init(wchar* file)
{
	wchar fn[512]=L"";
	
	bool rv=false;
	if (rv==false && parse_mds(file,false))
	{
		bool found=false;
		if (wcslen(file)>4)
		{
			wcscpy(&fn[0],file);
			int len=wcslen(fn);
			wcscpy(&fn[len-4],L".mdf");
			
			fp_mdf=_tfopen(fn,L"rb");
			found=fp_mdf!=0;
		}
		if (!found)
		{
			if (GetFile(fn,L"mds images (*.mds) \0*.mdf\0\0"))
			{
				fp_mdf=_tfopen(fn,L"rb");
				found=true;
			}
		}
			
		if (!found)
			return false;
		rv=true;
	}
	
	if (rv==false && parse_nrg(file,false))
	{
		rv=true;
		fp_mdf=_tfopen(file,L"rb");
	}
	if (rv==false)
		return false;
	/*
	for(int j=0;j<nsessions;j++)
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

u32 FASTCALL mds_DriveGetDiscType()
{
	return mds_Disctype;
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