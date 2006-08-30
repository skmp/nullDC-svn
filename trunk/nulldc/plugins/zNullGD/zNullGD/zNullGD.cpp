/*
**	zNullGD.cpp - ZeZu (2006)
*/
#include <vector>
using namespace std;

#include "zNullGD.h"
#include <commctrl.h>


ConfigLoadStrFP	* ConfigLoadStr;
ConfigSaveStrFP	* ConfigSaveStr;


HINSTANCE hInst=NULL;

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD   ul_reason_for_call,
					  LPVOID  lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}



/*
**	NullDC specific Interface,
**	returns valid fucntion ptrs
*/

extern "C" __declspec(dllexport) 
void dcGetPluginInfo(ndcPluginIf* If)
{
	If->dwIfVersion	= 0x00020000;	// double check with nullgdr
	If->dwLibVersion= 0x01;			// 
	If->dwPluginType= 0x02;			// 2=GDROM
	strcpy_s(If->szName, "zNullGD, SCSI Passthru GDROM Plugin By _ZeZu_ [" __DATE__ "]");

	If->Init		= gdInit;
	If->Term		= gdTerm;
	If->Reset		= gdReset;
	If->ThreadInit	= gdThreadInit;
	If->ThreadTerm	= gdThreadTerm;
	If->Config		= gdConfig;

	ConfigLoadStr	= If->ConfigLoadStr;
	ConfigSaveStr	= If->ConfigSaveStr;
}



// ignore these, we need it all on same thread
void gdTerm(DWORD)		 { lprintf("gdTerm()\n"); }
void gdThreadTerm(DWORD) { lprintf("gdThreadTerm()\n"); }



struct gd_init { gdNotifyFP * notify; } *Notify;

DWORD dwGDMode = GDFORMAT_XA;	// 1=cdrom,2=cdxa,8=GDROM


void gdReset(bool,DWORD)
{
	lprintf("gdReset()\n");
//	Notify->notify(1,(void*)dwGDMode);
}


void gdInit(void* param, DWORD dwfuck)
{
	lprintf("gdInit()\n");

	scsiInit();

	Notify = (gd_init*)param;

	Notify->notify(1,NULL);
}

void gdThreadInit(DWORD)
{
	lprintf("gdThreadInit()\n");

	scsiInit();
}

extern "C" __declspec(dllexport) 
void dcGetGDRInfo(gdPluginIf* If)
{
	If->dwVersion		= 0x00000200;	

	If->ReadTOC			= gdReadTOC;
	If->ReadSector		= gdReadSector;
	If->ReadSubChannel	= gdReadSubChannel;
	If->ReadSession		= gdReadSession;
	If->ReadDiskType	= gdReadDiskType;
}



DWORD gdReadDiskType(void)
{
	return dwGDMode;

	lprintf("gdReadDiskType()\n");

	DWORD toc[102];
	gdReadTOC(&toc[0], (DiskArea)1);

	int i=0;
	for(i=98; i>=0; i--) {
		if (toc[i]!=0xFFFFFFFF)
			if(4 == (toc[i]&4))
				break;
	}
	if (i==-1)
		i=0;
	//	u32 addr = ((toc[i]&0xFF00)<<8) | ((toc[i]>>8)&0xFF00) | ((toc[i]>>24)&255);

	///////////////////////////
	BYTE * pmem = new BYTE[2048];
	gdReadSector(pmem, 150, 1, 2048);	// addr (must be 150 at least)

	if(0x41474553 == ((DWORD*)pmem)[0]) {
		printf("gdReadDiskType() Found IP, Using CDXA \n");
		dwGDMode = GDFORMAT_XA;
	} else {
		printf("gdReadDiskType() Found Not Found!, Using GDROM \n");
		dwGDMode = GDFORMAT_GDROM;
	}
	delete[] pmem;
	return dwGDMode;
}



void gdReadTOC(DWORD * pTOC, DiskArea dwSection)
{
	lprintf("\ngdReadTOC(%X)\n", dwSection);

	TOC	  toc	= {0};
	GDTOC * gdtoc	= (GDTOC *)pTOC;



/*	if(0 == dwSection)
	{
	}
	else*/
	{
		/*
		**	Is this completely Broken?
		**	Does it only want Session1 Data?
		*/

		scsiReadTOC(pDev, &toc, 0,0);
		memset(gdtoc, 0xFF, 408);

		unsigned int LBA = 0;
		unsigned char * p = (unsigned char*)&LBA;

		//spi cmd: 30 28 0 2E 5A 0 0 0 0 0
		for(int i=0; i<(toc.LastTrack-toc.FirstTrack+1); i++)
		{
			LBA = (toc.Tracks[i].LBA[1] << 16) | (toc.Tracks[i].LBA[2] << 8) | (toc.Tracks[i].LBA[3]);

			if(GDFORMAT_GDROM == dwGDMode)
			{
			//	LBA += 0xB05E + 0x100;	// hopefully?

			//	LBA = 150;// + 0x100;

				LBA += 0xB05E;
			}
			else {
				//if(4 == toc.Tracks[i].ADR)
					LBA += 150;
			}

			gdtoc->Track[i].ADR		= toc.Tracks[i].ADR;
			gdtoc->Track[i].CTRL	= toc.Tracks[i].CTRL;
			gdtoc->Track[i].FAD[0]	= p[2];	//toc.Tracks[i].LBA[1];
			gdtoc->Track[i].FAD[1]	= p[1];	//toc.Tracks[i].LBA[2];
			gdtoc->Track[i].FAD[2]	= p[0];	//toc.Tracks[i].LBA[3];
		}

		// 99 : First Track
		gdtoc->First.ADR	= toc.Tracks[toc.FirstTrack-1].ADR;
		gdtoc->First.CTRL	= toc.Tracks[toc.FirstTrack-1].CTRL;
		gdtoc->First.TrackNo= toc.FirstTrack;	// Correct?

		// 100: Last Track
		gdtoc->Last.ADR		= toc.Tracks[toc.LastTrack-1].ADR;
		gdtoc->Last.CTRL	= toc.Tracks[toc.LastTrack-1].CTRL;
		gdtoc->Last.TrackNo	= toc.LastTrack;	// Correct?

		gdtoc->First.Reserved = gdtoc->Last.Reserved = 0;

		// 101: Lead Out
		gdtoc->LeadOut.ADR		= toc.Tracks[toc.LastTrack].ADR;
		gdtoc->LeadOut.CTRL		= toc.Tracks[toc.LastTrack].CTRL;

		LBA = (toc.Tracks[toc.LastTrack].LBA[1] << 16) |
			(toc.Tracks[toc.LastTrack].LBA[2] << 8) | (toc.Tracks[toc.LastTrack].LBA[3]);


		LBA += (GDFORMAT_GDROM == dwGDMode) ? 0xB05E : 150 ;	// hopefully?


		gdtoc->LeadOut.FAD[0]	= p[2];	//toc.Tracks[toc.LastTrack].LBA[1];
		gdtoc->LeadOut.FAD[1]	= p[1];	//toc.Tracks[toc.LastTrack].LBA[2];
		gdtoc->LeadOut.FAD[2]	= p[0];	//toc.Tracks[toc.LastTrack].LBA[3];
	}

}


void gdReadSector(BYTE * pBuffer, DWORD dwSector, DWORD dwNumSectors, DWORD dwSize)
{
	lprintf("gdReadSector(%X, %X, %X)\n", dwSector, dwNumSectors, dwSize);

	if(0x800 != dwSize)
		lprintf("\n\n\n~!~\tERROR: ReadSector() Size Not 2048 !\n\n\n");

	if(GDFORMAT_GDROM == dwGDMode)
	{
		lprintf("\nFixing GD Addr: %X To: ", dwSector);

		dwSector -= 0xB05E;

		TOC	  toc	= {0};
		scsiReadTOC(pDev, &toc, 0,0);

		dwSector =	toc.Tracks[toc.LastTrack-1].LBA[1]<<16 | 
					toc.Tracks[toc.LastTrack-1].LBA[2]<<8 | 
					toc.Tracks[toc.LastTrack-1].LBA[3] +
					dwSector;

		lprintf("%X \n\n", dwSector);

	}
	else
	{
	/*	TOC	  toc	= {0};
		scsiReadTOC(pDev, &toc, 1,0);	// get sessions

		DWORD dwBase =	toc.Tracks[0].LBA[1]<<16 | 
						toc.Tracks[0].LBA[2]<<8 | 
						toc.Tracks[0].LBA[3];

		if(dwSector > (dwBase+166)) {
			lprintf("- Address change from %X to %X Base=%X\n", dwSector, (dwSector-150), dwBase);
			dwSector -= 150;
		}*/

		dwSector -= 150;
	//	dwSector -= 0x100;
	}

	for(DWORD s=0; s<dwNumSectors; s++)
	{ 
	//	lprintf("-\tReadSector(%X, %X, %X)\n", pDev, (s*dwSize), (dwSector+s));
		scsiReadSector(pDev, &pBuffer[s*dwSize], dwSector+s);

	//	scsiReadSector(pDev, pBuffer, dwSector+s);	//[s*dwSize]
	//	pBuffer += s*dwSize;

/*		static int is=0;
		char filename[1024];
		sprintf(filename, "./reads/read%04X_sec%06X.bin", is++, dwSector+s);
		FILE * f = fopen(filename,"wb");
		fwrite(&pBuffer[s*dwSize],1,2048,f);
		fclose(f);

		printf("is: %X\n", is);*/
	}
}

void gdReadSubChannel(BYTE * pBuffer, DWORD dwFormat, DWORD dwLen)
{
	memset(pBuffer, 0x00, dwLen);
}

void gdReadSession(BYTE * pBuffer, BYTE Session)
{
	lprintf("gdReadSession(%X)\n", Session);

	
	TOC	  toc	= {0};
	scsiReadTOC(pDev, &toc, 1,0);	// read sessions

	BYTE nSessions = (toc.LastSession - toc.FirstSession + 1);

	lprintf("- nSessions: %X\n", nSessions);


	// 0 == session

	pBuffer[0] = 1;		// status
	pBuffer[1] = 0;		// reserved

	unsigned int LBA = 0;
	unsigned char * p = (unsigned char*)&LBA;

	if(0==Session)
	{
		pBuffer[2] = nSessions;

		scsiReadTOC(pDev, &toc, 0,0);	// read tracks

		LBA =	(toc.Tracks[toc.LastTrack].LBA[1] << 16) |
				(toc.Tracks[toc.LastTrack].LBA[2] << 8) | 
				(toc.Tracks[toc.LastTrack].LBA[3]);// + 150;

	//	lprintf("LBA: %X\n", LBA);
		LBA += 150;
	//	lprintf("LBA: %X\n\n", LBA);

		pBuffer[3] = p[2];	//toc.Tracks[toc.LastTrack].LBA[1];
		pBuffer[4] = p[1];	//toc.Tracks[toc.LastTrack].LBA[2];
		pBuffer[5] = p[0];	//toc.Tracks[toc.LastTrack].LBA[3];

		lprintf("- LastTrack: %X\n", toc.LastTrack);

	} else
	{
		pBuffer[2] = Session;

		scsiReadTOC(pDev, &toc,1,Session);	// read session #

		LBA =	(toc.Tracks[0].LBA[1] << 16) |
				(toc.Tracks[0].LBA[2] << 8) | 
				(toc.Tracks[0].LBA[3]);// + 150;

	//	lprintf("LBA: %X\n", LBA);
		LBA += 150;
	//	LBA += 0x100;
	//	lprintf("LBA: %X\n\n", LBA);

		pBuffer[3] = p[2];	//toc.Tracks[0].LBA[1];
		pBuffer[4] = p[1];	//toc.Tracks[0].LBA[2];
		pBuffer[5] = p[0];	//toc.Tracks[0].LBA[3];
	}

	lprintf(" SESS: %02X %02X %02X %02X %02X %02X\n",
		pBuffer[0], pBuffer[1], pBuffer[2],
		pBuffer[3], pBuffer[4], pBuffer[5]);
}












// Temp Spot for this prob.
#include "resource.h"

void WinErrMB(void)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL );
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	// Free the buffer.
	LocalFree( lpMsgBuf );
}

class DriveDesc
{
public:

	DriveDesc(char letter)
	{
		Letter = letter;
		sprintf_s(Path,"\\\\.\\%c:\\", Letter);	// 

		DWORD dwSerial, dwMaxLen, dwFSFlags;

		Type = GetDriveType(Path);
		if(0==GetVolumeInformation(Path, Label, 512, &dwSerial, &dwMaxLen, &dwFSFlags, NULL, 0))
			sprintf_s(Label, "EMPTY");	//WinErrMB();
	}

	UINT Type;
	char Letter;
	char Path[512];
	char Label[512];
};

vector<DriveDesc> devList;

void EnumDevs(UINT Type, vector<DriveDesc> &devList)
{
	DWORD dwDrives = GetLogicalDrives();

	for(int i=2; i<32; i++)		// skips A: && B:
		if(dwDrives & (1<<i))
		{
			DriveDesc dd(char('A'+i));

			if(Type == dd.Type) {
				printf("Found Drive[%x]: %c: %s\n", i, dd.Letter, dd.Label);
				devList.push_back(dd);
			}
		}
}

void SaveSelDrive(UINT uiDrive)
{
}


BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static UINT uiSel=0;
	static HWND hList=NULL;
	static HIMAGELIST hSmall, hLarge;

	switch( uMsg )
	{
	case WM_INITDIALOG:
	{
		InitCommonControls();
		hList  = GetDlgItem(hWnd, IDL_DRIVELIST);
		hSmall = ImageList_Create(16, 16, FALSE, 3, 0);
		hLarge = ImageList_Create(32, 32, FALSE, 3, 0);

		HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CD));
		if(ImageList_AddIcon(hSmall, hIcon) == -1) printf( "ImageList_AddIcon(hSmall) Failed \n" );
		if(ImageList_AddIcon(hLarge, hIcon) == -1) printf( "ImageList_AddIcon(hLarge) Failed \n" );
		if(ImageList_GetImageCount(hSmall) < 1) printf("ERROR: ImageList_GetImageCount(hSmall) < 1 \n");
		if(ImageList_GetImageCount(hLarge) < 1) printf("ERROR: ImageList_GetImageCount(hLarge) < 1 \n");

		// Set the image lists.
		ListView_SetImageList(hList, hSmall, LVSIL_SMALL);
		ListView_SetImageList(hList, hLarge, LVSIL_NORMAL);

		const char szInfo[] = "test";
		const BYTE ColSize[] = { 0, 75, 220, 85, 80, 80 };

		LVCOLUMN lvC; 
		lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		for (int i=1; i <= 3; i++) 
		{ 
			lvC.iSubItem	= i;
			lvC.pszText		= (LPSTR)szInfo;	
			lvC.cx			= ColSize[i];	// width of column in pixels
			lvC.fmt			= LVCFMT_LEFT;	// left-aligned column

		//	LoadString(hInst, IDS_COLUMNS + i, szInfo, sizeof(szInfo));
			if(-1 == ListView_InsertColumn(hList, i, &lvC))
				printf("ERROR: ListView_InsertColumn() Failed \n");
		}

		LVITEM lvI;
		lvI.state = 0; 
		lvI.stateMask = 0; 
		lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE; 

		EnumDevs(DRIVE_CDROM, devList);
		for(UINT Dev=0; Dev<devList.size(); Dev++ )
		{
			lvI.iItem		= Dev;
			lvI.iImage		= 0;	// only 1 image
			lvI.iSubItem	= 0;
			lvI.lParam		= (LPARAM) Dev;//&rgPetInfo[index];
			lvI.pszText		= devList[Dev].Path;	//LPSTR_TEXTCALLBACK; // sends an LVN_GETDISPINFO message.			

			if(ListView_InsertItem(hList, &lvI) == -1)
				printf("ERROR: ListView_InsertColumn() Failed \n");
		}

		SetFocus(hList); 
		return TRUE;
	}

	case WM_COMMAND:
		if(IDOK == LOWORD(wParam))	//switch(LOWORD(wParam))
		{
			EndDialog(hWnd, IDOK);

			char DriveStr[512];
			sprintf_s(DriveStr, "%c:", devList[uiSel].Letter);
			ConfigSaveStr("zNullGD","Drive", DriveStr);

			printf("->> Selected Drive[%x]: %c: %s\n", uiSel, devList[uiSel].Letter, devList[uiSel].Label);
			return TRUE;
		}
	break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case LVN_ITEMCHANGED:
			uiSel = ((NMLISTVIEW*)lParam)->iItem;
			return TRUE;

		case NM_DBLCLK:
			uiSel = ((LPNMITEMACTIVATE)lParam)->iItem;

			EndDialog(hWnd, IDOK);

			char DriveStr[512];
			sprintf_s(DriveStr, "%c:", devList[uiSel].Letter);
			ConfigSaveStr("zNullGD","Drive", DriveStr);

			printf("->> Selected Drive[%x]: %c: %s\n", uiSel, devList[uiSel].Letter, devList[uiSel].Label);
			return TRUE;

		default: break;
		}
	return FALSE;


	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hWnd, 420);
	return TRUE; 

	default: break;
	}
	return FALSE;
}



void gdConfig(DWORD dwType, void * handle)
{
	if(-1==DialogBox(hInst, MAKEINTRESOURCE(IDD_CONFIG), (HWND)handle, DlgProc))
		WinErrMB();
}


