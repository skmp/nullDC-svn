/*
**	zNullMaple.h	- Maple Bus Plugin by David Miller -
*/
#pragma once

typedef unsigned int u32;
typedef unsigned char u8;




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



struct MapleDevInst;

//buffer_out_len and responce need to be filled w/ proper info by the plugin
//buffer_in must not be edited (its direct pointer on ram)
//output buffer must contain the frame data , the frame header is generated by the maple rooting code
typedef void MapleDevDMA_FP(MapleDevInst* pDevInst, u32 Command,
							  u32* buffer_in,  u32 buffer_in_len,
							  u32* buffer_out, u32& buffer_out_len, u32& responce);


struct MapleDevInst
{
	u8					Port;
	void *				pDevData;
	MapleDevDMA_FP *	MapleDeviceDMA;
	bool				bConnected;
};



struct MapleDev;
typedef void MapleCreateInstFP(MapleDev *pDev, MapleDevInst& pDevInst, u8 port);
typedef void MapleDestroyInstFP(MapleDev*pDev, MapleDevInst& pDevInst);

struct MapleDev
{
	char name[128];
	u8 type;
	u8 id;

	MapleCreateInstFP* CreateInstance;
	MapleDestroyInstFP* DestroyInstance;
};



struct MapleInit
{
	void* WindowHandle;
};


struct MapleIf
{
	u32	IfVersion;			//interface version , curr 0.1.0
	MapleDev Devices[16];
};

typedef void dcGetMapleInfoFP(MapleIf * If);







/*
**	Prototypes for these ugly bastards
*/

void mplTerm(DWORD);
void mplInit(void*,DWORD);
void mplReset(bool,DWORD);
void mplThreadInit(DWORD);
void mplThreadTerm(DWORD);
void mplConfig(DWORD,void*);

extern ConfigLoadStrFP	* ConfigLoadStr;
extern ConfigSaveStrFP	* ConfigSaveStr;






