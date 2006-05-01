#ifndef _MVSCSI_H_
#define _MVSCSI_H_

#ifdef __BORLANDC__
#pragma option -a1
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define mvSCSI_OK               0x00
#define mvSCSI_NO_INIT          0x01
#define mvSCSI_CHECK            0x02
#define mvSCSI_ERROR            0x03
#define mvSCSI_NO_BUS           0x04
#define mvSCSI_NO_DEV           0x05

#define mvSCSI_MAX_BUS          0xFF
#define mvSCSI_MAX_PATH         0x02
#define mvSCSI_MAX_TARGET       0x10
#define mvSCSI_MAX_LUN          0x08

#define mvSCSI_MAX_CDB_LEN      0x10
#define mvSCSI_MAX_SENSE_LEN    0x12

#define mvSCSI_DATA_OUT         0x00
#define mvSCSI_DATA_IN          0x01
#define mvSCSI_DATA_UNSPECIFIED 0x02

#define mvSCSI_DEFAULT_TIMEOUT  0x0A

#define mvSCSI_DASD             0x00
#define mvSCSI_SEQD             0x01
#define mvSCSI_PRNT             0x02
#define mvSCSI_PROC             0x03
#define mvSCSI_WORM             0x04
#define mvSCSI_CDROM            0x05
#define mvSCSI_SCAN             0x06
#define mvSCSI_OPTI             0x07
#define mvSCSI_JUKE             0x08
#define mvSCSI_COMM             0x09
#define mvSCSI_UNKNOWN          0x1F

typedef struct {
	BYTE  busNumber;
	DWORD busMaxTransfer;
	DWORD busAlignmentMask;
} mvSCSI_Bus, *PmvSCSI_Bus, FAR *LPmvSCSI_Bus;

typedef struct {
	BYTE  devBus;
	BYTE  devPath;
	BYTE  devTarget;
	BYTE  devLun;
	BYTE  devType;
	DWORD devTimeOut;
} mvSCSI_Dev, *PmvSCSI_Dev, FAR *LPmvSCSI_Dev;

typedef struct {
	BYTE   devBus;
	BYTE   devPath;
	BYTE   devTarget;
	BYTE   devLun;
	BYTE   cmdCDBLen;
	BYTE   cmdCDB[mvSCSI_MAX_CDB_LEN];
	BYTE   cmdSenseLen;
	BYTE   cmdSense[mvSCSI_MAX_SENSE_LEN];
	BYTE   cmdDataDir;
	DWORD  cmdBufLen;
	LPVOID cmdBufPtr;
	BYTE   cmdStatus;
} mvSCSI_Cmd, *PmvSCSI_Cmd, FAR *LPmvSCSI_Cmd;

/*
#ifdef _mvSCSIEXPORTING_
#define DLLImport __declspec(dllexport)
#else
#define DLLImport __declspec(dllimport)
#endif
*/

#define DLLImport	// * not using a dll


DLLImport DWORD mvSCSI_Init(VOID);
DLLImport DWORD mvSCSI_RescanBus(LPmvSCSI_Bus);
DLLImport DWORD mvSCSI_InquiryBus(LPmvSCSI_Bus);
DLLImport DWORD mvSCSI_InquiryDev(LPmvSCSI_Dev);
DLLImport DWORD mvSCSI_SetDevTimeOut(LPmvSCSI_Dev);
DLLImport DWORD mvSCSI_ExecCmd(LPmvSCSI_Cmd);

#ifdef __cplusplus
}
#endif

#ifdef __BORLANDC__
#pragma option -a.
#endif

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif
