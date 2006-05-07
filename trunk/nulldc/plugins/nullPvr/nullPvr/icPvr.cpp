/* Tunels dctest plugins <-> icarus plugins , power VR lle
 * this is prop not the fastest way to do this but anyway ....
 * pvr emulation is not the bootleneck :P
 */

#include "nullPvr.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Plugin types */
#define PLUGIN_TYPE_GFX				1
#define PLUGIN_TYLE_LLE				0x1000
#define PLUGIN_TYPE_PVR				0x1001


#define CALL						_cdecl

/* Interrupt defines */
#define RENDER_VIDEO	1
#define OPAQUE_LIST		6
#define OPAQUE_MOD_LIST 7
#define TRANS_LIST		8
#define TRANS_MOD_LIST	9
#define PUNCH_THRU_LIST	18

/* Debug */
#define DEBUG_DP_GFX	(1<<24)

/***** Structures *****/
typedef struct {
	WORD Version;        /* Should be set to 1 */
	WORD Type;           /* Set to PLUGIN_TYPE_GFX */
	char Name[100];      /* Name of the DLL */
} PLUGIN_INFO;


/********** PowerVR2 Plugin Specific **
****************************************/

typedef struct
{
	HWND hWnd;			// Render window 
	HWND hStatusBar;    // if render window has no status bar, this is NULL 

	u8	* VRAM;			// Access to 32-bit video ram (0xa5000000 following) 
	u32	* PVR_REGS;		// Access to PVR registers (0xa05f8000 following) 

	void (*Enqueue_IRQ)		(u32 type);
	void (*Debug_Printf)	(u32 dwDebugFlags, char* szFormat, ... );
	void (*CPU_Halt)		(char * szReason);

	// Silly stats for debug console 
	void (*StatsFrame)		(void);
	void (*StatsVtxStrip)	(void);

	// This can be used to easily save/load a 32-bit config struct for each gfx plugin 
	void (*Save_Config)		(char *RegStr, DWORD Config);
	void (*Load_Config)		(char *RegStr, DWORD *Config);

} GFX_INFO;

typedef BOOL PvrInitFP		( GFX_INFO GfxInfo );	// InitiateGFX
typedef void PvrTermFP		( void );	// GFXCloseDLL
typedef void PvrOpenFP		( void );	// GfxGameOpen
typedef void PvrCloseFP		( void );	// GfxGameClosed

typedef void PvrCommandFP	( u32 *pCmd, u32 dwQW_Len );	// ProcessDList

typedef void PvrWrite32FP	( DWORD dwAddr, DWORD * dwValue );

typedef void PvrUpdateFP	( u32 dwReason );	// SizeScreen | DrawScreen(FB) etc ..

typedef void PvrDllTestFP	( HWND hParent );	// GFXDllTest
typedef void PvrDllAboutFP	( HWND hParent );	// GFXDllAbout
typedef void PvrDllConfigFP( HWND hParent );	// GFXDllConfig

typedef void GetDllInfoFP ( PLUGIN_INFO * PluginInfo );

#if defined(__cplusplus)
}
#endif

struct icPluginfo
{
	PLUGIN_INFO plinfo;
	PvrInitFP*	PvrInit;
	PvrTermFP* PvrTerm;
	PvrOpenFP* PvrOpen;
	PvrCloseFP* PvrClose;
	PvrCommandFP* PvrCommand;
	PvrWrite32FP* PvrWrite32;	
	PvrUpdateFP* PvrUpdate;
	PvrDllTestFP* PvrDllTest;
	PvrDllAboutFP* 	PvrDllAbout;
	PvrDllConfigFP* PvrDllConfig;
	GetDllInfoFP* GetDllInfo;
	void * dllHanlde;
};

icPluginfo*cur_icpl;

u8 pvr_regs[1*1024*1024];
s32 clcount=0,clc_pvr_scanline=0;
u32 prv_cur_scanline=0;

u32 fps;
s32 pvr_numscanlines_clk=521;
s32 pvr_numscanlines=521;

u32 lasft_fps;

//helper functions
u8 HblankInfo()
{
	return 0;
}


u8 VblankInfo()
{
	u32 data = *(u32*)&pvr_regs[0xa05f80cc & 0x7FFF];
	u32 down=(data & 0x3FFF);
	u32 top=(data >> 16) & 0x3FFF;

	if (data==0)
		return 0;
	if ((prv_cur_scanline <= top) || ( prv_cur_scanline>= down))
		return 1;
	else
		return 0;
}
//end helper
u32 ints=0;

int vblk_cnt=0;
double spd_fps=0;
double spd_cpu=0;
void icUpdatePvr (u32 cycles)
{
	clcount+=cycles;
	//RaiseInterrupt(InteruptID::
	if (clcount>(DCclock)/60)
	{
		vblk_cnt++;
		clcount-=(DCclock)/60;//faked
		//ok .. here , after much effort , we reached a full screen redraw :P
		//now , we will copy everything onto the screen (meh) and raise a vblank interupt -- there is no vblank interrupt :P
		RaiseInterrupt(InterruptID::holly_HBLank);// -> This turned out to be HBlank btw , needs to be emulated ;(
		if (cur_icpl->PvrUpdate)
			cur_icpl->PvrUpdate(1);

			// didn't look at read i guess this is not needed 
		*(u32*)&pvr_regs[0x5F810C &0x7fff] |= 0x2000;	// SPG_STATUS
//		u32 maple_vblnk_mode=ReadMem(SB_MDTSEL,4);
	//	if (maple_vblnk_mode&0x1)
	//	{
		//	printf("MAPLE VBLANK MODE = 1");
	//	}
		if ((timeGetTime()-(double)lasft_fps)>800)
		{
			spd_fps=(double)fps/(double)((double)(timeGetTime()-(double)lasft_fps)/1000);
			spd_cpu=(double)vblk_cnt/(double)((double)(timeGetTime()-(double)lasft_fps)/1000);
			spd_cpu*=(DCclock/1000000)/60;

			ints=0;
			lasft_fps=timeGetTime();
			fps=0;

			double fullfps=(spd_fps/spd_cpu)*200;

			char fpsStr[256];
			sprintf(fpsStr," FPS: %4.2f(%4.2f)  -  Sh4: %4.2f mhz (%4.2f%%) - nullDC v0.0.1", spd_fps,fullfps, spd_cpu,spd_cpu*100/200);
			SetWindowText((HWND)Hwnd, fpsStr);
			vblk_cnt=0;
		}
		
	}/* else	// this isn't fast enough, have to do it in read itself
	{
		*(u32*)&pvr_regs[0x5F810C &0x7fff] &= 0x1FFF;	// SPG_STATUS
	}*/


	clc_pvr_scanline += (int)cycles;
	if (clc_pvr_scanline > ((DCclock/60) / pvr_numscanlines))//60 ~herz = 200 mhz / 60=3333333.333 cycles per screen refresh
	{
		//ok .. here , after much effort , we did one line
		//now , we must chekc for raster beam pos interupts
		prv_cur_scanline=(prv_cur_scanline+1)%pvr_numscanlines;

	//	u32 data = *SPG_VBLANK_INT;
		//u32 data = ReadMem(0xa05f80cc,4);
		u32 data=*(u32*)&pvr_regs[0xa05f80cc & 0x7FFF];
		if ((data & 0x3FFF) == prv_cur_scanline)
			RaiseInterrupt(holly_SCANINT1);
		if (((data >> 16) & 0x3FFF) == prv_cur_scanline)
			RaiseInterrupt(holly_SCANINT2);
		
		clc_pvr_scanline -= (3495253 / pvr_numscanlines);
	}
}
#define Is_64_Bit(addr) ((Address &0x1000000)==0)


u32 icReadMem (u32 Address,u32 len)
{
	/*Address =vramlock_ConvAddrtoOffset64(Address);
	
	if (len==1)
		return pvr_vram_b[Address & 0x7FFFFF];
	else if (len==2)
		return pvr_vram_w[(Address & 0x7FFFFF)>>1];
	else if (len==4)
		return pvr_vram_dw[(Address & 0x7FFFFF)>>2];*/
		
	printf("icReadMem: Size is %x, witch is not 1,2 or 4\n",len);
	return 0;
}
void PvrSQWrite (u32*data,u32 len)
{
	if (cur_icpl->PvrCommand)
		cur_icpl->PvrCommand(data,len);
}
void icWriteMem (u32 Address,u32 data,u32 len)
{
	/*vramlock_Test(Address,0,0);
	Address =vramlock_ConvAddrtoOffset64(Address);

	if (len==1)
		pvr_vram_b[Address & 0x7FFFFF]=(u8)data;
	else if (len==2)
		pvr_vram_w[(Address & 0x7FFFFF)>>1]=(u16)data;
	else if (len==4)
		pvr_vram_dw[(Address & 0x7FFFFF)>>2]=data;
	else*/
		printf("SysWriteMem: Size is %x, witch is not 1,2 or 4\n",len);
}


u32 vbl_last=0, vbl_mask=0xFFFFFFFF;;

u32 icReadMem_reg(u32 Address,u32 len)
{

//	if( (SPG_VBLANK_INT &0xFFFFFF) != Address )
	//	printf(" PVR: Read %X == %X\n\n", Address, *(u32*)&pvr_regs[Address &0x7fff] );

	switch (Address & 0xFFFF)
	{
	 
	case 0x8000:	//a05f8000: (id)
		return 0x17fd11db;// Set5.xx development box or consumer machine

	case 0x8004://a05f8004: (revision)
		//| 31-8 | 7-4   | 3-0   |
		//| n/a  | major | minor |
		return (1<<4)|(1<<0);

		// katana reads from this an awfull lot before resetting pvr all the time !~~
//	case 0x80CC:	// 
//		return 0x001500104;	// wtf?

		// ok vblank portion of this is broken or not fast enough,
		// this used to work for atomiswave anyways 
	
	case 0x8000 + 0x43 * 4: // [SYNC_STAT]
		//vbl_mask = (vbl_last &0x2000) ? ~0x2000 : 0xFFFFFFFF ;	// cheap trick 
		vbl_last = (u32)((VblankInfo()) << 13) | ((HblankInfo()) << 12) | ((0) << 10) | prv_cur_scanline;
		return vbl_last ;//& vbl_mask;


	default :
		return *(u32*)&pvr_regs[Address &0x7fff];
		break;

	}
	
}
#define SOFTRESET			0x005F8008		//RW	CORE & TA software reset
#define STARTRENDER		0x005F8014		//RW	Drawing start
#define TA_LIST_INIT		0x005F8144		//RW	TA initialization
#define TA_OL_BASE	0x00000124	//	RW	Object list write start address	
#define TA_ISP_BASE		0x00000128	//	RW	ISP/TSP Parameter write start address	
#define TA_OL_LIMIT		0x0000012C	//	RW	Start address of next Object Pointer Block	
#define TA_ISP_LIMIT		0x00000130	//	RW	Current ISP/TSP Parameter write address	
#define TA_NEXT_OPB		0x00000134	//	R	Global Tile clip control	
#define TA_ITP_CURRENT		0x00000138	//	R	Current ISP/TSP Parameter write address	
#define TA_GLOB_TILE_CLIP	0x0000013C	//	RW	Global Tile clip control
#define TA_ALLOC_CTRL		0x00000140	//	RW	Object list control	
#define TA_YUV_TEX_BASE	0x00000148	//	RW	YUV422 texture write start address	
#define TA_YUV_TEX_CTRL	0x0000014C	//	RW	YUV converter control	
#define TA_YUV_TEX_CNT		0x00000150	//	R	YUV converter macro block counter value	

#define TA_LIST_CONT		0x00000160	//	RW	TA continuation processing	
#define TA_NEXT_OPB_INIT	0x00000164	//	RW	Additional OPB starting address	


void icWriteMem_reg(u32 Address,u32 data,u32 len)
{
//	printf(" PVR: Write %X <= %X\n\n", Address, data );

	if (cur_icpl->PvrWrite32)
		cur_icpl->PvrWrite32(Address|0xA0000000,(DWORD*) &data);

	if( (SOFTRESET&0x7FFF) == (Address&0x7FFF) ) {
	//	if(data!=0)
	//		em.status = EMU_HALTED;	// fuck that, its katana resets every frame now smth is fucked up
		if(data!=0)
			data = 0;
	}
	if( (TA_LIST_INIT&0x7FFF) == (Address&0x7FFF) ){
		if( data >> 31 )
		{
			*(u32*)&pvr_regs[TA_ITP_CURRENT &0x7fff]=*(u32*)&pvr_regs[TA_ISP_BASE  &0x7fff];
			*(u32*)&pvr_regs[TA_OL_BASE &0x7fff]=*(u32*)&pvr_regs[TA_NEXT_OPB_INIT  &0x7fff];
			*(u32*)&pvr_regs[TA_NEXT_OPB &0x7fff]=*(u32*)&pvr_regs[TA_NEXT_OPB_INIT  &0x7fff];
		}
		data = 0;		// will this fix anything ?
	}
	if( (TA_LIST_CONT&0x7FFF) == (Address&0x7FFF) )
	{
		printf("LIST CONTNINUATION!!!\n");
	}
	if ((Address&0x7FFF)==(STARTRENDER&0x7FFF))
	{ 
		data=0;
	}
	*(u32*)&pvr_regs[Address &0x7fff]=data;
}


void icEnqueue_IRQ	(u32 type)
{
	switch (type)
	{
		case RENDER_VIDEO:	//renderdone
			fps++;
			RaiseInterrupt(holly_RENDER_DONE);
			RaiseInterrupt(holly_RENDER_DONE_vd);
			RaiseInterrupt(holly_RENDER_DONE_isp);
			ints|=1;
			break;
		case OPAQUE_LIST :
			RaiseInterrupt(holly_OPAQUE);
			ints|=2;
			break;
		case OPAQUE_MOD_LIST :
			RaiseInterrupt(holly_OPAQUEMOD);
			ints|=4;
			break;
		case TRANS_LIST :
			RaiseInterrupt(holly_TRANS);
			ints|=8;
			break;
		case TRANS_MOD_LIST :
			RaiseInterrupt(holly_TRANSMOD);
			ints|=16;
			break;
		case PUNCH_THRU_LIST :
			RaiseInterrupt(holly_PUNCHTHRU);
			ints|=32;
			break;
		default:
			//dc.dcon.WriteLine("DLL:InteruptHandler");
			printf("\n\nERROR : UNK INTERUPT CALLBACK CALL , number = %d\n\n",type);
			break;
	}
	//if speed hack :)
//		clcount=((DCclock)/60)-0x1000;		//hack , raise vblank after this
//						//should realy speed up apps using olny 3d
//						//and wait vblank :)
//		prv_cur_scanline=( ReadMem(0xa05f80cc,4) & 0x3FFF)-1;
//		clc_pvr_scanline=((3495253 / pvr_numscanlines))-1000;

}
void icDebug_Printf	(u32 dwDebugFlags, char* szFormat, ... )
{
	 char szErr[1024];
	va_list va;
	va_start(va, szFormat);
	vsprintf(szErr,szFormat,va);
	va_end(va);
	printf( "%s\n",szErr );
}
void icCPU_Halt	(char * szReason)
{
	printf(szReason);
}

// Silly stats for debug console 
void icStatsFrame		(void)
{
	
}
void icStatsVtxStrip	(void){}

// This can be used to easily save/load a 32-bit config struct for each gfx plugin 
void icSave_Config		(char *RegStr, DWORD Config)
{
}
void icLoad_Config		(char *RegStr, DWORD *Config)
{
}


char * GetName()
{
	if (cur_icpl->GetDllInfo)
		cur_icpl->GetDllInfo(&cur_icpl->plinfo);
	return &cur_icpl->plinfo.Name[0];
}
bool icInit ()
{
	GFX_INFO GfxInfo;
	GfxInfo.CPU_Halt=icCPU_Halt;
	GfxInfo.Debug_Printf=icDebug_Printf;
	GfxInfo.Enqueue_IRQ=icEnqueue_IRQ;
	GfxInfo.hWnd=(HWND)Hwnd;
	GfxInfo.Load_Config=icLoad_Config;
	GfxInfo.PVR_REGS=(u32*)pvr_regs;
	GfxInfo.Save_Config=icSave_Config;
	GfxInfo.StatsFrame=icStatsFrame;
	GfxInfo.StatsVtxStrip=icStatsVtxStrip;
	GfxInfo.VRAM=vram_64;

	if (cur_icpl->PvrInit)
		cur_icpl->PvrInit(GfxInfo);

	if (cur_icpl->PvrOpen)
		cur_icpl->PvrOpen();

	icWriteMem_reg(0xA05F8060,0,4);
return true;
}

void icTerm ()
{
	if (cur_icpl->PvrClose)
		cur_icpl->PvrClose();

	if (cur_icpl->PvrTerm)
		cur_icpl->PvrTerm();
}


void LoadIcPvrDll(char *dll)
{
	icPluginfo*pl=cur_icpl=(icPluginfo*)malloc(sizeof(icPluginfo));
	HMODULE dllh=LoadLibraryA(dll);


	pl->dllHanlde=dllh;

	pl->GetDllInfo=(GetDllInfoFP*)GetProcAddress(dllh,"GetDllInfo");
	pl->PvrInit=(PvrInitFP*)GetProcAddress(dllh,"PvrInit");
	pl->PvrTerm=(PvrTermFP*)GetProcAddress(dllh,"PvrTerm");
	pl->PvrOpen=(PvrOpenFP*)GetProcAddress(dllh,"PvrOpen");
	pl->PvrClose=(PvrCloseFP*)GetProcAddress(dllh,"PvrClose");
	pl->PvrCommand=(PvrCommandFP*)GetProcAddress(dllh,"PvrCommand");
	pl->PvrWrite32=(PvrWrite32FP*)GetProcAddress(dllh,"PvrWrite32");
	pl->PvrUpdate=(PvrUpdateFP*)GetProcAddress(dllh,"PvrUpdate");
	pl->PvrDllTest=(PvrDllTestFP*)GetProcAddress(dllh,"PvrDllTest");
	pl->PvrDllAbout=(PvrDllAboutFP*)GetProcAddress(dllh,"PvrDllAbout");
	pl->GetDllInfo=(GetDllInfoFP*)GetProcAddress(dllh,"GetDllInfo");
	pl->PvrDllConfig=(PvrDllConfigFP*)GetProcAddress(dllh,"PvrDllConfig");

}


void UnLoadIcPvrDll(void *pl)
{
//neeeds to be done
	
}
