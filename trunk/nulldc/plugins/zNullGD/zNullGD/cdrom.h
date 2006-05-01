/*
**	cdrom.h
*/
#ifndef __CDROM_H__
#define __CDROM_H__


#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
typedef union
{
	struct {
		BYTE	FirstTrack;
		BYTE	LastTrack;
	};
	struct {
		BYTE	FirstTrack;
		BYTE	LastTrack;
	};
	struct {
		BYTE	FirstRsvd;
		BYTE	LastRsvd;
	};

} TOC_HEADER_0, TOC_HEADER_1, TOC_HEADER_2;



typedef struct
{
	BYTE	bRsvdA;
	BYTE	bAdrCtrl;
	BYTE	bTrackNum;
	BYTE	bRsvdB;
	DWORD	dwAddr;	// LBA || MSF

} TOC_ENTRY_0;

typedef struct
{
	BYTE	bRsvdA;
	BYTE	bAdrCtrl;
	BYTE	bFirstTrack;
	BYTE	bRsvdB;
	DWORD	dwAddr;	// LBA || MSF

} TOC_ENTRY_1;

typedef struct
{
	BYTE	bSession;
	BYTE	bAdrCtrl;

	BYTE	TNO;
	BYTE	POINT;

	BYTE	Min,Sec,Frame,Zero;
	BYTE	PMIN,PSEC,PFRAME;

} TOC_ENTRY_2;

typedef struct
{
	WORD	wDataLen;

	union {
		TOC_HEADER_0 th0;
		TOC_HEADER_1 th1;
		TOC_HEADER_2 th2;	// same as th1
	};

	union {
		TOC_ENTRY_0 te0[0x64];	// tracks
		TOC_ENTRY_1 te1;		// sessions
		TOC_ENTRY_2 te2[0x64];	// FULL
	};

} TOC;


typedef struct
{
	int		iTrackNum;	// why not dword ?

	DWORD	dwStartLBA;
	DWORD	dwTrackLen;

	BYTE	bTrackType;
	BYTE	bTrackPad[3];

	char	szName[0x100];

} TRACK;
*/


typedef struct
{
	WORD	wDataLen;

	union
	{
		struct {
			BYTE	FirstTrack;
			BYTE	LastTrack;
		};
		struct {
			BYTE	FirstSession;
			BYTE	LastSession;
		};
		struct {
			BYTE	FirstRsvd;
			BYTE	LastRsvd;
		};
	};

	struct
	{
		BYTE	Reserved;
		BYTE	ADR:	4;
		BYTE	CTRL:	4;
		BYTE	TrackNo;	// 
		BYTE	Reserved2;
		BYTE	LBA[4];

	} Tracks[100];

} TOC;



////////////////////////////////////

#define CMD_COMPLETE (1)

#define MSF_M(x)	((x>>0x08)&0xFF)
#define MSF_S(x)	((x>>0x10)&0xFF)
#define MSF_F(x)	((x>>0x18)&0xFF)

#define MSF_CODE(x)	MSF_M(x),MSF_S(x),MSF_F(x)
#define MSF_FMT		"[%02x/%02x/%02x]"


#define DEV_SET(HA,TGT,LUN)	((HA<<24) | (TGT<<16) | (LUN<<8))

#define DEV_HA(x)	(BYTE)((x>>0x18)&0xFF)
#define DEV_TGT(x)	(BYTE)((x>>0x10)&0xFF)
#define DEV_LUN(x)	(BYTE)((x>>0x08)&0xFF)


/*	Func. prototypes
*/	

//BOOL PlayAudio( DWORD dwDev, DWORD dwTrack );
//BOOL PauseAudio( DWORD dwDev, BOOL bResume );





#ifdef __cplusplus
}
#endif

#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif //__CDROM_H__
