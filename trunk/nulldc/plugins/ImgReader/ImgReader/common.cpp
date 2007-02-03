#include "common.h"
#include "cdi.h"
#include "mds.h"
#include "iso9660.h"
#include <memory.h>
#include <windows.h>

DriveIF* CurrDrive;
DriveIF drives[]=
{
	{
		//cdi
		cdi_DriveReadSector,
		cdi_DriveGetTocInfo,
		cdi_DriveGetDiscType,
		cdi_GetSessionsInfo,
		cdi_init,
		cdi_term,
		"CDI reader"
	},
	{
		//cdi
		mds_DriveReadSector,
		mds_DriveGetTocInfo,
		mds_DriveGetDiscType,
		mds_GetSessionsInfo,
		mds_init,
		mds_term,
		"NRG/MDS/MDF reader"
	},
		{
		//iso
		iso_DriveReadSector,
		iso_DriveGetTocInfo,
		iso_DriveGetDiscType,
		iso_GetSessionsInfo,
		iso_init,//these need to be filled
		iso_term,
		"ISO reader"
	},
};

DriveNotifyEventFP* DriveNotifyEvent;

int msgboxf(char* text,unsigned int type,...)
{
	va_list args;

	char temp[2048];
	va_start(args, type);
	vsprintf(temp, text, args);
	va_end(args);


	return MessageBox(NULL,temp,emu.Name,type);
}

bool ConvertSector(u8* in_buff , u8* out_buff , int from , int to,int sector)
{
	//if no convertion
	if (to==from)
	{
		memcpy(out_buff,in_buff,to);
		return true;
	}

	switch (to)
	{
	case 2048:
		{
			verify(from>=2048);
			verify((from==2448) || (from==2352) || (from==2336));
			if ((from == 2352) || (from == 2448))
				memcpy(out_buff,&in_buff[0x18],2048); //0x18 or 0x10 ? cdrwin for some reason needs 0x10 o.O
			else
				memcpy(out_buff,&in_buff[0x8],2048);
		}
		break;
	case 2352:
		//if (from >= 2352)
		{
			memcpy(out_buff,&in_buff[0],2352);
		}
		break;
	default :
		printf("Sector convertion from %d to %d not supported \n", from , to);
		break;
	}

	return true;
}



bool InitDrive()
{
	char fn[512]="";
	if (GetFile(fn,"CD/GD Images (*.cdi;*.mds;*.nrg;*.gdi) \0*.cdi;*.mds;*.nrg;*.gdi\0\0")==false)
	{
		CurrDrive=&drives[Iso];
		return false;
	}

	if (CurrDrive !=0 && CurrDrive->Inited==true)
	{
		//Terminate
		CurrDrive->Inited=false;
		CurrDrive->Term();
	}

	CurrDrive=0;

	for (u32 i=0;i<3;i++)
	{
		if (drives[i].Init(fn))
		{
			CurrDrive=&drives[i];
			printf("Using %s \n",CurrDrive->name);
			return true;
		}
	}
	msgboxf("Unkown format type",MB_ICONERROR);
	//CurrDrive=&drives[Iso];
	return false;
}

void TermDrive()
{
	if (CurrDrive !=0 && CurrDrive->Inited==true)
	{
		//Terminate
		CurrDrive->Inited=false;
		CurrDrive->Term();
		CurrDrive=0;
	}
}


//
//convert our nice toc struct to dc's native one :)

void ConvToc(u32* to,TocInfo* from)
{
	#define flipendian(valdat) ((((valdat)>>24)&0xFF)|((((valdat)>>16)&0xFF)<<8)|((((valdat)>>8)&0xFF)<<16)|(((((valdat)>>0)&0xFF))<<24))
	#define CreateTrackInfo(ctrl,adr,fad) flipendian(((((ctrl)<<4)|((adr)))<<24)|\
										(fad))
	#define CreateTrackInfo_se(ctrl,adr,trk) flipendian(((((ctrl)<<4)|((adr)))<<24)|\
										((trk)<<16))

	to[99]=CreateTrackInfo_se(from->tracks[from->FistTrack-1].Control,from->tracks[from->FistTrack-1].Addr,from->FistTrack); 
	to[100]=CreateTrackInfo_se(from->tracks[from->LastTrack-1].Control,from->tracks[from->LastTrack-1].Addr,from->LastTrack); 
	to[101]=CreateTrackInfo(from->LeadOut.Control,from->LeadOut.Addr,from->LeadOut.FAD); 
	for (int i=0;i<99;i++)
	{
		to[i]=CreateTrackInfo(from->tracks[i].Control,from->tracks[i].Addr,from->tracks[i].FAD); 
	}
	#undef flipendian
	#undef CreateTrackInfo
	#undef CreateTrackInfo_se
}



void GetDriveToc(u32* to,DiskArea area)
{
	TocInfo driveTOC;
	CurrDrive->GetToc(&driveTOC,area);
	ConvToc(to,&driveTOC);
}

void GetDriveSessionInfo(u8* to,u8 session)
{
	SessionInfo driveSeS;
	if (CurrDrive->GetSessionInfo)
		CurrDrive->GetSessionInfo(&driveSeS);
	
	to[0]=2;//standby
	to[1]=0;//0's
	
	if (session==0)
	{
		to[2]=2;//count of sessions
		to[3]=(driveSeS.SessionsEndFAD>>16)&0xFF;//fad is sessions end
		to[4]=(driveSeS.SessionsEndFAD>>8)&0xFF;
		to[5]=(driveSeS.SessionsEndFAD>>0)&0xFF;
	}
	else
	{
		to[2]=1;//track count on this session
		to[3]=(driveSeS.SessionFAD[session-1]>>16)&0xFF;//fad is session start
		to[4]=(driveSeS.SessionFAD[session-1]>>8)&0xFF;
		to[5]=(driveSeS.SessionFAD[session-1]>>0)&0xFF;
	}
}

#include <windows.h>
bool GetFile(TCHAR *szFileName, TCHAR *szParse)
{
	static OPENFILENAME ofn;
	static TCHAR szFile[MAX_PATH]="";    
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= NULL;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFilter		= szParse;
	ofn.nFilterIndex	= 1;
	ofn.nMaxFileTitle	= 128;
	ofn.lpstrFileTitle	= szFile;
	ofn.lpstrInitialDir	= NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if(GetOpenFileName(&ofn)<=0)
		return false;
	return true;
}