#include "common.h"
#include "cdi.h"
#include "iso9660.h"
#include "FolderMount.h"

DriveIF* CurrDrive;
DriveIF drives[]=
{
	{
		//iso
		iso_DriveReadSector,
		iso_DriveGetTocInfo,
		iso_DriveGetDiskType,
		0,
		iso_init,//these need to be filled
		iso_term
	},
	#ifdef X86
	//cdi is olny available on x86 :)
	{
		//cdi
		cdi_DriveReadSector,
		cdi_DriveGetTocInfo,
		cdi_DriveGetDiskType,
		cdi_GetSessionsInfo,
		cdi_init,//these need to be filled
		cdi_term
	},
	#endif
	{
		//File Mount
		FM_ReadSector,
		FM_DriveGetTocInfo,
		FM_DriveGetDiskType,
		0,
		FM_init,//these need to be filled
		FM_term
	}
};

DriveNotifyEventFP* DriveNotifyEvent;

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
	default :
		printf("Sector convertion from %d to %d not supported \n", from , to);
		break;
	}

	return true;
}



void SetDrive(gd_drivers driver)
{
	if (CurrDrive !=0 && CurrDrive->Inited==true)
	{
		if (CurrDrive->driver==driver)
			return;//no change
		//Terminate
		CurrDrive->Inited=false;
		CurrDrive->Term();
	}

	if (driver==none)
	{
		CurrDrive=0;
		return;
	}

	CurrDrive=&drives[driver];

	CurrDrive->Init();
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


void FindAllFiles(FileFoundCB* callback,char* dir,void* param)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char DirSpec[MAX_PATH + 1];  // directory specification
	DWORD dwError;
	char tempDD[MAX_PATH + 1];
	u32 ddl;
	ddl=(u32)strlen(dir)+1;

	strncpy (DirSpec, dir, ddl);
	ddl-=2;
	strncpy (tempDD, dir, ddl);

	hFind = FindFirstFileA( DirSpec, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	{
		return;
	} 
	else 
	{

		//if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
		{
			tempDD[ddl]=0;
			strcat(tempDD,FindFileData.cFileName);
			callback(tempDD,FindFileData.cFileName,param,FindFileData.dwFileAttributes);
		}

		while (FindNextFileA(hFind, &FindFileData) != 0) 
		{ 
			//if (( & FILE_ATTRIBUTE_DIRECTORY)==0)
			{
				tempDD[ddl]=0;
				strcat(tempDD,FindFileData.cFileName);
				callback(tempDD,FindFileData.cFileName,param,FindFileData.dwFileAttributes);
			}
		}

		dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES) 
		{
			return ;
		}
	}
	return ;
}


//
//convert our nice toc struct to dc's native one :)

void ConvToc(u32* to,natTocInfo* from)
{
	#define flipendian(valdat) ((((valdat)>>24)&0xFF)|((((valdat)>>16)&0xFF)<<8)|((((valdat)>>8)&0xFF)<<16)|(((((valdat)>>0)&0xFF))<<24))
	#define CreateTrackInfo(ctrl,adr,fad) flipendian(((((ctrl)<<4)|((adr)))<<24)|\
										(fad))
	#define CreateTrackInfo_se(ctrl,adr,trk) flipendian(((((ctrl)<<4)|((adr)))<<24)|\
										((trk)<<16))

	to[99]=CreateTrackInfo_se(from->first.ControlInfo,from->first.Addr,from->first.number); 
	to[100]=CreateTrackInfo_se(from->last.ControlInfo,from->last.Addr,from->last.number); 
	to[101]=CreateTrackInfo(from->lba_leadout.ControlInfo,from->lba_leadout.Addr,from->lba_leadout.FAD); 
	for (int i=0;i<99;i++)
	{
		to[i]=CreateTrackInfo(from->entry[i].ControlInfo,from->entry[i].Addr,from->entry[i].FAD); 
	}
	#undef flipendian
	#undef CreateTrackInfo
	#undef CreateTrackInfo_se
}



void GetDriveToc(u32* to,DiskArea area)
{
	TocInfo driveTOC;
	CurrDrive->GetToc(driveTOC,area);
}

void GetDriveSessionInfo(u8* to,u8 session)
{
	SessionInfo driveSeS;
	if (CurrDrive->GetSessionInfo)
		CurrDrive->GetSessionInfo(driveSeS);
}