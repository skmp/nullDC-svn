/*
**	zSCSI.cpp - ZeZu (2006)
*/
#include "zNullGD.h"

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;



void scsiEnumDevs(BYTE devType, vector<mvSCSI_Dev> &devList);

//vector<mvSCSI_Dev> devList;
mvSCSI_Dev cdDev = {0};
mvSCSI_Dev * pDev = &cdDev;





void scsiInit(void)
{
	char DriveStr[512];
	ConfigLoadStr("zNullGD","Drive", DriveStr);
	if(0==strcmp(DriveStr,"NULL")) {
		MessageBox(NULL,"You Need To Configure A Drive!","ERROR: Configure First!",MB_ICONERROR);
		return;
	}

	mvSPTI_Handle hDev(DriveStr);

	DWORD bytesReturnedIO = 0;
	SCSI_ADDRESS scsiAddr = {0};

	if(!DeviceIoControl(hDev, IOCTL_SCSI_GET_ADDRESS, NULL, 0, &scsiAddr, sizeof(scsiAddr), &bytesReturnedIO, NULL))
		printf("\nERROR: DevIoCtrl failed w/ IOCTL_SCSI_GET_ADDRESS !\n");

	printf("\nscsiInit()\n%s scsiAddr:  %X %X %X %X \n\n", DriveStr,
		scsiAddr.PortNumber, scsiAddr.PathId, scsiAddr.TargetId, scsiAddr.Lun );



	// Check Cfg File First, use that dev.

	cdDev.devTimeOut= 30;
	cdDev.devType	= mvSCSI_CDROM;
	cdDev.devBus	= scsiAddr.PortNumber;
	cdDev.devLun	= scsiAddr.Lun;
	cdDev.devPath	= scsiAddr.PathId;
	cdDev.devTarget	= scsiAddr.TargetId;

	mvSCSI_Init();
	//return true;
}






void scsiEnumDevs(BYTE devType, vector<mvSCSI_Dev> &devList)
{
	DWORD status         = mvSCSI_Init();
	BYTE  mvSCSIStatus   = HIBYTE(LOWORD(status));
	BYTE  mvSCSIBusCount = LOBYTE(LOWORD(status));

	for(int busIndex = 0; busIndex < mvSCSIBusCount; ++busIndex)
	{
		mvSCSI_Bus bus = {0};
		bus.busNumber = busIndex;

		mvSCSI_RescanBus(&bus);

		if(mvSCSI_InquiryBus(&bus) == mvSCSI_OK) {
			for(int devPath = 0; devPath < mvSCSI_MAX_PATH; ++devPath) {
				for(int devTarget = 0; devTarget < mvSCSI_MAX_TARGET; ++devTarget) {
					for(int devLun = 0; devLun < mvSCSI_MAX_LUN; ++devLun)
					{
						mvSCSI_Dev dev = {0};
						dev.devBus     = busIndex;
						dev.devPath    = devPath;
						dev.devTarget  = devTarget;
						dev.devLun     = devLun;
						dev.devTimeOut = 30;

						if (mvSCSI_OK == mvSCSI_SetDevTimeOut(&dev) &&
							mvSCSI_OK == mvSCSI_InquiryDev(&dev))
						{
							if(devType == dev.devType) {
								cout << "Found : "  << int(dev.devBus) << ":" << int(dev.devPath) << ":" << int(dev.devTarget) << ":" << int(dev.devLun) << ", type id: " << int(dev.devType) << ", timeout: " << int(dev.devTimeOut) << endl;
								devList.push_back(dev);
							}
						}
					}
				}
			}
		}
	}
}


/*
**	scsiReadTOC()
**	- pDev:	Device to use, includes BUS and TARG/LUN/ID
**	- pTOC:	Pointer to TOC Structure or valid memory capable of holding 804 bytes
**	- track:Track/Session # to start Off at, refer to SCSI3 or MMC5 Ref. manual
**	- fmt: # (MFS Valid)(Track/Session # useage), Info
**	.0: (V) (Track#)	Returns All Tracks, Track # 0xAA (can) be returned for the lead out
**	.1: (V) (Ignored)	Returns first (complete) Session, Last (complete) Session and Last Session Address (Track Invalid)
**	.2: (I) (Session#)	Returns all the Q sub-code data in TOC area, uses MSF only
**	.3: (I) (Ignored)	Returns all the Q sub-code data in PMA area, uses MSF only
**	.4: (I) (Ignored)	Returns all the ATIP data, uses MSF only
**	.5: (I) (Ignored)	Returns all the CD-TEXT data, uses MSF only
*/

void scsiReadTOC(mvSCSI_Dev * pDev, TOC *pTOC, BYTE fmt, BYTE track)
{
	mvSCSI_Cmd cmd	= {0};

	cmd.devBus		= pDev->devBus;
	cmd.devPath		= pDev->devPath;
	cmd.devTarget	= pDev->devTarget;
	cmd.devLun		= pDev->devLun;

	cmd.cmdCDBLen	= 0x0A;

	cmd.cmdCDB[0]	= SCSI_READ_TOC;//
	cmd.cmdCDB[1]	= 0x00;			// MSF = 0x02
	cmd.cmdCDB[2]	= fmt &0xF;		// TOC format

	cmd.cmdCDB[6]	= track;		// Track / Session #
	cmd.cmdCDB[7]	= 0x03;			// Length of TOC Buffer
	cmd.cmdCDB[8]	= 0x24;			// 0x324 100 x 8b + 4b header
	cmd.cmdCDB[9]	= 0x00;			// Control

	cmd.cmdSenseLen = mvSCSI_MAX_SENSE_LEN;
	//cmd.cmdSense[mvSCSI_MAX_SENSE_LEN];

	cmd.cmdDataDir  = mvSCSI_DATA_IN;
	cmd.cmdBufLen   = 0x324;			//sizeof(inqData);
	cmd.cmdBufPtr   = (BYTE*)pTOC;

	//cmd.cmdStatus;

	if(mvSCSI_OK != mvSCSI_ExecCmd(&cmd)) {
		lprintf("ERROR: scsiReadTOC() Failed! \n");
	}
	if(mvSCSI_OK != cmd.cmdStatus)
	{
		lprintf(" scsiReadTOC(%X,%X)\n",fmt,track);
		lprintf(" Status : %x \n", cmd.cmdStatus);	
		lprintf(" SENSE: %08x %08x %08x %08x \n",
			*(DWORD*)(cmd.cmdSense), *(DWORD*)(cmd.cmdSense+4),
			*(DWORD*)(cmd.cmdSense+8), *(DWORD*)(cmd.cmdSense+12) );
		//return FALSE;
	}
}


void scsiReadSector(mvSCSI_Dev * pDev, BYTE* pSecBuf, DWORD dwSecAddr)
{
	mvSCSI_Cmd cmd	= {0};

	cmd.devBus		= pDev->devBus;
	cmd.devPath		= pDev->devPath;
	cmd.devTarget	= pDev->devTarget;
	cmd.devLun		= pDev->devLun;

	cmd.cmdCDBLen	= 0x0A;

	cmd.cmdCDB[0]	= SCSI_READ10;
	cmd.cmdCDB[1]	= (pDev->devLun&7) << 5;// | DPO ;	DPO = 8

	//*((DWORD*)&srbExecCmd.CDBByte[2])	= dwSectorAddr;	// hrm ...
	cmd.cmdCDB[2]	= (BYTE)(dwSecAddr >> 0x18 & 0xFF);	// MSB
	cmd.cmdCDB[3]	= (BYTE)(dwSecAddr >> 0x10 & 0xFF);
	cmd.cmdCDB[4]	= (BYTE)(dwSecAddr >> 0x08 & 0xFF);
	cmd.cmdCDB[5]	= (BYTE)(dwSecAddr >> 0x00 & 0xFF);	// LSB

	cmd.cmdCDB[7]	= 0;
	cmd.cmdCDB[8]	= 1;

	cmd.cmdSenseLen = mvSCSI_MAX_SENSE_LEN;
	//cmd.cmdSense[mvSCSI_MAX_SENSE_LEN];

	cmd.cmdDataDir  = mvSCSI_DATA_IN;
	cmd.cmdBufLen   = 0x800;	// 0x800=2048 .. 0x930=2352
	cmd.cmdBufPtr   = pSecBuf;

	if(mvSCSI_OK != mvSCSI_ExecCmd(&cmd)) {
		lprintf("ERROR: scsiReadSector() Failed!\n");
	}
	if(mvSCSI_OK != cmd.cmdStatus)
	{
		lprintf(" scsiReadSector(%08X)\n", dwSecAddr);
		lprintf(" Status : %x \n", cmd.cmdStatus);	
		lprintf(" SENSE: %08x %08x %08x %08x \n",
			*(DWORD*)(cmd.cmdSense), *(DWORD*)(cmd.cmdSense+4),
			*(DWORD*)(cmd.cmdSense+8), *(DWORD*)(cmd.cmdSense+12) );
		//return FALSE;
	}
}









//////// TEMP ?
# define _LOG_ (1)

void lprintf(char* szFmt, ... )
{
#ifdef _LOG_
	FILE * f = fopen("zSCSI.txt","a+t");

	va_list va;
	va_start(va, szFmt);
	vfprintf_s(f,szFmt,va);
	va_end(va);

	fclose(f);
#endif
}







