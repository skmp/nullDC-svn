/*
**	zNullGD.h - ZeZu (2006)
*/
#ifndef __ZNULLGD_H__
#define __ZNULLGD_H__

#include <windows.h>
#include "mvscsi/mvscsi.h"
#include "mvscsi/mvspti.h"

#include "scsidefs.h"
#include "cdrom.h"
#include "gdrom.h"
#include "zSCSI.h"




/*
**	Modified Plugin Specs for NullDC
**	they are ugly but less so
*/

typedef void TermFP(DWORD dwPluginType);
typedef void ThreadTermFP(DWORD dwPluginType);
typedef void ThreadInitFP(DWORD dwPluginType);
typedef void InitFP(void * param, DWORD dwPluginType);
typedef void ResetFP(bool bManual, DWORD dwPluginType);
typedef void ConfigFP(DWORD dwPluginType, void* handle);

//These are provided by the emu
typedef void ConfigLoadStrFP(const char * lpSection, const char * lpKey, char * lpReturn);
typedef void ConfigSaveStrFP(const char * lpSection, const char * lpKey, const char * lpString);

typedef struct
{
	DWORD	dwIfVersion;
	char	szName[128];
	DWORD	dwLibVersion;
	DWORD	dwPluginType;

	InitFP			* Init;
	TermFP			* Term;
	ResetFP			* Reset;
	ThreadInitFP	* ThreadInit;
	ThreadTermFP	* ThreadTerm;
	ConfigFP		* Config;
	void* UEH;		//unhandled write handler

	//Functions "Exported" from the emu , these are SET by the emu
	ConfigLoadStrFP	* ConfigLoadStr;
	ConfigSaveStrFP	* ConfigSaveStr;

} ndcPluginIf;


enum DiskArea
{
	SingleDensity,
	DoubleDensity
};



enum GD_DiskFormat
{
	GDFORMAT_CDDA	= 0x00,
	GDFORMAT_CDROM	= 0x10,
	GDFORMAT_XA		= 0x20,
	GDFORMAT_EXTRA	= 0x30,
	GDFORMAT_CDI	= 0x40,
	GDFORMAT_GDROM	= 0x80,
};

enum GD_Notify
{
	Notify_DiskEject,		// Params, DiskType cast to (void*)
	Notify_DiskInsert,		// Could use Eject for both, we'll see

	Notify_CDAudioStop,		// inform the fucking authorities
	Notify_CDAudioStart,	// this is better
	Notify_CDAudioChange,	// change track
};



typedef DWORD gdDiskTypeFP(void);
typedef void gdNotifyFP(DWORD dwEvent, void *param);	// GD_Notify, GD_DiscFormat
typedef void gdReadSectorFP(BYTE * pBuffer, DWORD dwStartUNK, DWORD dwSector, DWORD dwSize);
typedef void gdReadSubChannelFP(BYTE * pBuffer, DWORD dwFormat, DWORD dwLen);
typedef void gdReadTocFP(DWORD * pTOC, DiskArea dwSection);
typedef void gdReadSessionFP(BYTE * pBuffer, BYTE Session);

struct gdInitShit
{

};



typedef struct
{
	DWORD dwVersion;

	gdReadSectorFP		* ReadSector;
	gdReadSubChannelFP	* ReadSubChannel;
	gdReadTocFP			* ReadTOC;
	gdDiskTypeFP		* ReadDiskType;
	gdReadSessionFP		* ReadSession;

} gdPluginIf;



/*
**	Prototypes for these ugly bastards
*/

void gdTerm(DWORD);
void gdInit(void*,DWORD);
void gdReset(bool,DWORD);
void gdThreadInit(DWORD);
void gdThreadTerm(DWORD);
void gdConfig(DWORD,void*);

extern ConfigLoadStrFP	* ConfigLoadStr;
extern ConfigSaveStrFP	* ConfigSaveStr;

DWORD gdReadDiskType(void);

void gdReadTOC(DWORD * pTOC, DiskArea dwSection);
void gdReadSector(BYTE * pBuffer, DWORD dwStartUNK, DWORD dwSector, DWORD dwSize);
void gdReadSubChannel(BYTE * pBuffer, DWORD dwFormat, DWORD dwLen);
void gdReadSession(BYTE * pBuffer, BYTE Session);


//temp?
void lprintf(char* szFmt, ... );

#endif //__ZNULLGD_H__
