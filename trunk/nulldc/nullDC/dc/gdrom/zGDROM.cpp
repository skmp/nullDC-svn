/*
**	zGDROM.cpp
*/

#include <vector>
using namespace std;

#include "gdrom_if.h"
#include "dc/mem/sb.h"
#include "dc/sh4/dmac.h"
#include "dc/sh4/sh4_if.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/rec_v1/rec_v1_blockmanager.h"

#ifdef ZGDROM
#ifndef BUILD_NAOMI


// A few vectors or lists for our shitt
vector<u16> cmdbuff;	// max 16 bytes
vector<u16> databuff;	// max ???


static u32 rFEATURES=0;
static u32 rSECTCNT=0;
static u32 rDRVSEL=0;

static u32 rIMPEDHI0=0, rIMPEDHI4=0;
static u32 rIMPEDHI8=0, rIMPEDHIC=0;

static gdErrorReg	rERROR;
static gdStatusReg	rSTATUS;
static gdIReasonReg	rIREASON;



static s32 dmaCount=0;
static u32 dmaOffset=0;

static u8 gdReadBuffer[1024*1024*16];		// TEMP *FIXME* -- star gladiator dma's 9515008 bytes


// Going to K.I.S.S. It ;) hehe
// One basic function for setting IO,DRQ,CoD never do it manually, 
// have one nice internal Status Reg that will get other needed info
// from plugin too ...

static gdromSR		gdSR;
static gdReqMode	gdRM;
static gdReqStat	gdRS;

static void gdSetSR(gdIntStatus sr)
{
	gdSR.iStatus = (u8)sr;

	rIREASON.IO		= gdSR.IO;
	rIREASON.CoD	= gdSR.CoD;
	rSTATUS.DRQ		= gdSR.DRQ;
	rSTATUS.CHECK	= gdSR.CHECK;
	rSTATUS.CORR	= gdSR.CORR;

	// ToDo: Use SR to add other bits as well ie: BSY or DRDY and STATE portion of iDevType
}


//////////////////////
void ProcessSPI(void);

void lprintf(char* szFmt, ... );
void logd(u32 rw, u32 addr, u32 data);

void DMAC_Ch3St(u32 data);
void TestWrite(u32 data) {
	printf(" \n(GD)Write to 74e4: %X \n", data);
	lprintf(" \n(GD)Write to 74e4: %X \n", data);
}
/////////////////////////////////////////////////////////////////
const static char DriveInfo[9]	= "SE      ";
const static char SystemInfo[9]	= "Rev 6.42";
const static char SystemDate[7]	= "990316";

void gdrom_reg_Init(void)
{
	logd(420,0,0);

	rERROR.Full	 = 0;
	rSTATUS.Full = 0;
	rIREASON.Full= 0;

	rSTATUS.DSC = 1;
	
	gdSR.iStatus	= GD_RESET_SR;
	gdSR.iDevType	= GDFORMAT_XA | GDSTATE_STANDBY;

	// Setup ReqMode
	memset(&gdRM, 0, sizeof(gdRM));
	gdRM.CDROM_Speed	= 0x0000;
	gdRM.Standby_Time	= 0xB400;	// 0x00B4 | B400 ?
	gdRM.Read_Settings	= 0x19;
	gdRM.Read_Retry		= 0x08;

	memcpy(gdRM.DriveInformation, DriveInfo, 8);
	memcpy(gdRM.SystemVersion, SystemInfo, 8);
	memcpy(gdRM.SystemDate, SystemDate, 6);

	// Setup ReqStat
	memset(&gdRS, 0, sizeof(gdRS));
	gdRS.Status = 2;	// Standby
	gdRS.Repeat = 8;	//
	gdRS.Format = (gdSR.iDevType >> 4) & 15;
	gdRS.FAD[0] = 0;	// home
	gdRS.FAD[1] = 0;	// home
	gdRS.FAD[2] = 0x96;	// home
	gdRS.ReadRetry = 15;
/*
	Cylinder Low = 14h,
	Cylinder High = EBh,
	Drive/Head = 00h 
	BSY = 0 following after any reset indicates that the task file register is already initialized for the host. 
*/
	sb_regs[(0x5F74E4-SB_BASE)>>2].flags = REG_32BIT_READWRITE | REG_16BIT_READWRITE | REG_8BIT_READWRITE | REG_READ_DATA;
	sb_regs[(0x5F74E4-SB_BASE)>>2].writeFunction = TestWrite;

	sb_regs[(SB_GDST_addr-SB_BASE)>>2].flags = REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[(SB_GDST_addr-SB_BASE)>>2].writeFunction = DMAC_Ch3St;
}

void gdrom_reg_Term(void) { }
void gdrom_reg_Reset(bool Manual)
{
	gdrom_reg_Term();
	gdrom_reg_Init();
	gdSR.iDevType	= (u8)((u32)libGDR->gdr_info.GetDiskType() | GDSTATE_STANDBY);
}



u32  ReadMem_gdrom(u32 Addr, u32 sz)
{
	logd(0,Addr,0);
	if((1 != sz) && (GD_DATA != (Addr &255)))
		printf("!(GD)\tERROR: GDROM Read Size: %X from (%X) Unhandled!\n", sz, Addr);

	switch(Addr &255)
	{
	case GD_IMPEDHI0:	return rIMPEDHI0;
	case GD_IMPEDHI4:	return rIMPEDHI4;
	case GD_IMPEDHI8:	return rIMPEDHI8;
	case GD_IMPEDHIC:	return rIMPEDHIC;

	case GD_ERROR:		return rERROR.Full;
	case GD_IREASON:	return rIREASON.Full;

	case GD_SECTNUM:	return gdSR.iDevType;
	case GD_BYCTLLO:	return gdSR.ByteCountLO;
	case GD_BYCTLHI:	return gdSR.ByteCountHI;

	case GD_DRVSEL:		return rDRVSEL;

	case GD_STATUS:
		SB_ISTEXT &= ~1;

		// this actually makes it go past where it stops but just causes it to loop doing error checking?
		if(rERROR.ABRT) {
			RaiseInterrupt(InterruptID::holly_GDROM_CMD);	// Raise Interrupt for aborted cmd

			// clear error status
		}

		return rSTATUS.Full;

	case GD_ALTSTAT:	return rSTATUS.Full;


	case GD_DATA:

		if(GD_RECV_DATA == gdSR.iStatus)
		{
			if(databuff.size() > 0)
			{
				u16 tdata = databuff.back();
				databuff.pop_back();
			//	gdSR.ByteCount -= 2;

				if(0 == databuff.size())
				{
					// If chain dma or more data to send in bursts, 
					// reset to RECV_CMD and issue another DMA and reset bytecnt
					lprintf("(GD)\tData Buffer Read Complete !\n");

					rSTATUS.BSY = 0;
					rSTATUS.DRDY = 1;
					gdSetSR(GD_STATUS_OK);
					RaiseInterrupt(InterruptID::holly_GDROM_CMD);
					
					databuff.clear();
					gdSR.ByteCount = 0;
				}
				return tdata;
			}
			else
			{
				// check if bytecnt is zero and set drq = 0 / return 0 ?
				printf("\n!(GD)\tERROR: Read from GDROM DATA REG When EMPTY!\n\n");
				lprintf("\n!(GD)\tERROR: Read from GDROM DATA REG When EMPTY!\n\n");
			}
		}

		printf("!(GD)\tERROR: Read from GDROM DATA REG!\n");
		lprintf("!(GD)\tERROR: Read from GDROM DATA REG!\n");
		return 0;
	}

	printf("!(GD)\tERROR: Read to GDROM Reg %X Unhandled!\n", Addr);
	return 0;
}


void WriteMem_gdrom(u32 Addr, u32 data, u32 sz)
{
	logd(1,Addr,data);
	if((1 != sz) && (GD_DATA != (Addr &255)))
		printf("!(GD)\tERROR: GDROM Write Size: %X to (%X) Unhandled!\n", sz, Addr);


	switch(Addr &255)
	{
	case GD_IMPEDHI0:	rIMPEDHI0=data;	return;
	case GD_IMPEDHI4:	rIMPEDHI4=data;	return;
	case GD_IMPEDHI8:	rIMPEDHI8=data;	return;
	case GD_IMPEDHIC:	rIMPEDHIC=data;	return;

	case GD_FEATURES:	rFEATURES=data;	return;
	case GD_SECTCNT:	rSECTCNT=data;	return;

	case GD_SECTNUM:	printf("Write to SECTNUM\n");		break;

	case GD_BYCTLLO:	gdSR.ByteCountLO = (u8)data;		return;
	case GD_BYCTLHI:	gdSR.ByteCountHI = (u8)data;		return;

	case GD_DEVCTRL:	printf("Write to GD_DEVCTRL\n");	break;
	case GD_DRVSEL:		printf("Write to GD_DRVSEL\n");		break;

	case GD_DATA:

		if(2 != sz)
			printf("!(GD)\tERROR: GDROM Write to DATA REG Size: %X Unhandled!\n", sz);

		if(GD_RECV_CMD == gdSR.iStatus)
		{
			cmdbuff.push_back((u16)data);

			if(cmdbuff.size() >= 6)	// recv cmd complete
			{
				// If we do this async, we will set BSY
				// tell the plugin to callback [fn] when complete
				// start the command (which will return imm.
				// and we return imm. as well

				rSTATUS.BSY = 1;

					// *FIXME* (for async) we want to clear drq,
					// but do we change IO else it unhandled mode atm
				//gdSetSR(GD_STATUS_OK)

				lprintf("\n\tCMD_RECV Complete:\n%04X %04X %04X %04X %04X %04X\n\n",
					cmdbuff[0], cmdbuff[1], cmdbuff[2], cmdbuff[3], cmdbuff[4], cmdbuff[5]);

				ProcessSPI();
				cmdbuff.clear();	// make sure to clear it after we're done
			}
			return;
		}
		else if(GD_RECV_DATA == gdSR.iStatus)	// uh .. check on recv vs send here .. and in read ..
		{
			printf("(GDROM) Data Write (SET_MODE): %X\n", data);

			// Ok this is crash worthy ;)

			gdRM.Words[gdSR.ByteCount/2] = data;

			if(0 >= (gdSR.ByteCount -= 2))
			{
				rSTATUS.BSY = 0;
				rSTATUS.DRDY = 1;
				gdSetSR(GD_STATUS_OK);
				RaiseInterrupt(InterruptID::holly_GDROM_CMD);
			}
			return;
		}

		printf("!(GD)\tERROR: GDROM Write to DATA NOT RECV_CMD!\n");
		break;


	case GD_COMMAND:
	  {
		if((data>8) && rSTATUS.BSY)
			printf("!(GD)\tERROR: GDROM Command while BSY (%X) Unhandled!\n", data);

		switch(data&255)
		{
		case GDC_NOP:
		case GDC_SFT_RESET:		// *FIXME*
			gdrom_reg_Reset(0);

			if(dmaCount>0) {
				lprintf("\n~!\tAborting GDROM Dma Transfer!\n");
				RaiseInterrupt(InterruptID::holly_GDROM_DMA);
				gdSetSR(GD_STATUS_ERR);
				rERROR.ABRT	= 0x1;
				rERROR.Sense= 0xB; 
				rSTATUS.DRDY = 0;
				rSTATUS.DSC = 1;
				dmaCount	= 0;
			}

		//	dmaCount = 0;
			dmaOffset = 0;
			rSTATUS.BSY = 0;
			rSTATUS.DRDY = 1;
			gdSetSR(GD_STATUS_OK);
			RaiseInterrupt(InterruptID::holly_GDROM_CMD);
			return;

		case GDC_SPI_PACKET:
			rSTATUS.BSY	= 0;		// Not BSY Anymore
			gdSetSR(GD_RECV_CMD);	// Read to Recieve Command
			return;					// Now Wait for Data

		case GDC_EXEC_DIAG:
		case GDC_IDENTIFY_DEV:	// *FIXME* 
			RaiseInterrupt(InterruptID::holly_GDROM_CMD);
			break;

		case GDC_SET_FEATURES:	// *FIXME* 
			RaiseInterrupt(InterruptID::holly_GDROM_CMD);
			return;
		}
		printf("!(GD)\tERROR: GDROM Command (%X) Unhandled!\n", data);
		break;
	  }

	}

	printf("!(GD)\tERROR: Write to GDROM Reg %X with %X Unhandled!\n", Addr, data);
}

	// This is NOT ready for Async, each specific MUST set its own ByteCnt reg !

void ProcessSPI(void)
{
	u8 cmd[12];

	for(u32 c=0; c<6; c++) {
		cmd[(c<<1)+0] = (u8)((u16)cmdbuff[c] &255);
		cmd[(c<<1)+1] = (u8)((u16)cmdbuff[c] >> 8);
	}

	switch(cmd[0])
	{
	case SPI_TEST_UNIT:
		lprintf("(GD)\tSPI_TEST_UNIT !\n\n");
	//	rERROR.Full = 0;
	//	gdSetSR(GD_STATUS_OK);
		goto complete;	// thats all

	case SPI_CD_OPEN:
	case SPI_CD_PLAY:
	case SPI_CD_SEEK:
	case SPI_CD_SCAN:
		lprintf("(GD)\tSPI_CD_* CMD !\n");
		break;


	case SPI_REQ_STAT:	// pio from host

		if((cmd[2]+cmd[4]) > 10) {
			printf("!(GD)\tERROR: SPI_REQ_STAT Value Out of Range! %02X+%02X Unhandled!\n", cmd[2], cmd[4]);
			break;
		}

		lprintf("(GD)\tSPI_REQ_STAT: %02X+%02X !\n\n", cmd[2], cmd[4]);

		for(int x=(int)((cmd[4]-2)>>1); x>=0; x--)
			databuff.push_back(gdRS.Words[(cmd[2]>>1)+x]);

		gdSR.ByteCount = cmd[4];
		goto pio_complete;

	case SPI_REQ_MODE:	// max 32 bytes

		if((cmd[2]+cmd[4]) > 32) {
			printf("!(GD)\tERROR: SPI_REQ_MODE Value Out of Range! %02X+%02X Unhandled!\n", cmd[2], cmd[4]);
			break;
		}

		lprintf("(GD)\tSPI_REQ_MODE: %02X+%02X !\n\n", cmd[2], cmd[4]);

		for(int x=(int)((cmd[4]-2)>>1); x>=0; x--)
			databuff.push_back(gdRM.Words[(cmd[2]>>1)+x]);

		gdSR.ByteCount = cmd[4];
		goto pio_complete;


	case SPI_REQ_ERROR:		break;

	case SPI_GET_TOC:
	 {
		u32 gdtoc[128];	// plus padding
		libGDR->gdr_info.GetToc(&gdtoc[0], DiskArea(cmd[1]&1));

		u32 len = (cmd[3]<<8) | cmd[4];

		lprintf("(GD)\tSPI_GET_TOC: len %d !\n\n", len);

		for(int x=(int)((len-2)>>1); x>=0; x--)
			databuff.push_back( ((u16*)gdtoc)[x] );	// ? uh.. i think so 

		lprintf("TOC: size: %X\n", len);
		for(int i=0; i<102; i++)
			lprintf(": %X\n", ((unsigned int*)gdtoc)[i]);

		gdSR.ByteCount = (u16)len;
		goto pio_complete;
	 }


	case SPI_REQ_SES:
		lprintf("(GD)\tSPI_REQ_SES: %02X Len:%02X !\n\n", cmd[2], cmd[4]);

		u16 SessionInfo[3];
		libGDR->gdr_info.GetSessionInfo((u8*)&SessionInfo[0], cmd[2]);

		for(int x=2; x>=0; x--)
			databuff.push_back(SessionInfo[x]);

		gdSR.ByteCount = cmd[4];
		goto pio_complete;



		// *FIXME* devkit demos seem to want this

	case SPI_SET_MODE:	// pio from host

		if((cmd[2]+cmd[4]) > 32) {
			printf("!(GD)\tERROR: SPI_SET_MODE Value Out of Range! %02X+%02X Unhandled!\n", cmd[2], cmd[4]);
			break;
		}

		lprintf("(GD)\tSPI_SET_MODE: %02X+%02X !\n\n", cmd[2], cmd[4]);

		gdSetSR(GD_RECV_DATA);
		gdSR.ByteCount = (u16) cmd[4];	// *FIXME* test against cmd[1]
		RaiseInterrupt(InterruptID::holly_GDROM_CMD);
		return;



	case SPI_CD_READ:	// dma or pio
	case SPI_CD_READ2:
	 {
		if(0x28 != cmd[1])
			printf("(GD)\tSPI_CD_READ: Params: %02X Unhandled atm !\n\n", cmd[1]);

		u32 Start  = cmd[2]<<16 | cmd[3]<<8 | cmd[4];
		u32 Length = cmd[8]<<16 | cmd[9]<<8 | cmd[10];

		lprintf("(GD)\tSPI_CD_READ (%s): Params: %02X, Start: %06X, Len:%d !\n\n", 
			((rFEATURES&1)?"DMA":"PIO"), cmd[1], Start, Length);

			// *FIXME* modify the gd spec, pass all of cmd[1] to lib.
		libGDR->gdr_info.ReadSector(gdReadBuffer, Start, Length, 2048);

		if(rFEATURES&1)	// DMA
		{
			dmaCount = Length*2048;			// *FIXME* test against cmd[1]
			goto dma_complete;				// *FIXME* 
		}
		else			// PIO
		{
			for(int x=(int)((Length*2048-2)>>1); x>=0; x--)
				databuff.push_back( ((u16*)gdReadBuffer)[x] );

			gdSR.ByteCount = (u16)(Length*2048);	// *FIXME* test against cmd[1]
			goto pio_complete;				// *FIXME* 
		}
	  }


	case SPI_GET_SCD:
	 {
		 u32 Fmt = cmd[1] &15;
		 u32 Len = cmd[3]<<8 | cmd[4];

		 lprintf("(GD)\tSPI_GET_SCD: Fmt: %X, Len:%d !\n\n", Fmt, Len);

		 // *FIXME* modify the gd spec, pass all of cmd[1] to lib.
		 libGDR->gdr_info.ReadSubChannel(gdReadBuffer, Fmt, Len);

		 for(int x=(int)((Len-2)>>1); x>=0; x--)
			 databuff.push_back( ((u16*)gdReadBuffer)[x] );

		 gdSR.ByteCount = (u16)Len;	// *FIXME* test against cmd[1]
	 }
	 goto pio_complete;				// *FIXME* 



	case 0x70:			// map drive 
		goto complete;


extern u16 g_aValues0x71[];
extern u16 g_aValues0x71_b[];

	case 0x71:			// ???

		lprintf("\n\tSPI CMD 71 len: %d\n\n", gdSR.ByteCount);

		static u32 flip=0; flip ^= 1;
		for(int i=0; i<(gdSR.ByteCount>>1); i++)
			databuff.push_back( flip?(g_aValues0x71_b[(gdSR.ByteCount>>1)-i-1]):(g_aValues0x71[(gdSR.ByteCount>>1)-i-1]) );

	//	gdSR.ByteCount = rBYCTL.Full;
		goto pio_complete;
	}
	printf("!(GD)\tERROR: GDROM SPI Command %02X Unhandled!\n\n", cmd[0]);
	lprintf("!(GD)\tERROR: GDROM SPI Command %02X Unhandled!\n\n", cmd[0]);
	goto complete;

complete:
	rSTATUS.BSY = 0;
	rSTATUS.DRDY = 1;
	gdSetSR(GD_STATUS_OK);
	RaiseInterrupt(InterruptID::holly_GDROM_CMD);
	return;

pio_complete:
	rSTATUS.BSY = 0;
	gdSetSR(GD_RECV_DATA);
	RaiseInterrupt(InterruptID::holly_GDROM_CMD);
	return;

dma_complete:
	rSTATUS.BSY = 1;
	// dma in progress, ...
	return;
}



void NotifyEvent_gdrom(DriveEvent info, void* param)
{
	switch((GD_Notify)info)
	{
	case Notify_DiskEject:
	case Notify_DiskInsert:
		gdSR.iDevType	= (u8)((u32)libGDR->gdr_info.GetDiskType() | GDSTATE_STANDBY);
		return;

	default: break;
	}
	lprintf("!(GD)\tERROR: GDROM Event %X Unhandled!\n\n", (u32)info);
}




void DMAC_Ch3St(u32 data)
{
	if(!(data&1) || !(SB_GDEN &1))
		return;

	//TODO : Fix dmaor
	u32 dmaor	= DMAC_DMAOR;

	u32	src		= SB_GDSTAR,
		len		= SB_GDLEN ;

	// do we need to do this for gdrom dma ?
	if(0x8201 != (dmaor &DMAOR_MASK)) {
		printf("\n!\tGDROM: DMAOR has invalid settings (%X) !\n", dmaor);
		//return;
	}
	if(len & 0x1F) {
		printf("\n!\tGDROM: SB_GDLEN has invalid size (%X) !\n", len);
		return;
	}

	if(0 == len) {
		printf("\n!\tGDROM: Len: %X, Normal Termination !\n", len);
		goto complete_dma_chain;	// *FIXME* it needs the proper INTRQ's too
	}

	if( 1 == SB_GDDIR ) 
	{
		//best solution ;) will automagically invalidate block too
		WriteMemBlock(src,(u32*)&gdReadBuffer[dmaOffset], len);

		//GetMemPtr perhaps ? it's better not to use the arrays like that ;P
		//memcpy( &mem_b[src&RAM_MASK], &gdReadBuffer[dmaOffset], len );		
		/*for (int i=0;i<len;i+=0x2)
		{
			rec_v1_BlockTest(src+i); 
		}*/
		if (len>=8*1024*1024)
			printf("\n~\tERROR: GDROM DMA LENGTH LARGER THAN BUFFER SIZE!\n\n");

	//	lprintf("\n(DMA)\tGDROM DMA Started! src: %08X, len: %08X, dmaor: %X (dmaCount=%d)\n\n", src, len, dmaor, dmaCount);
	}
	else
		printf("\n!\tGDROM: SB_GDDIR %X (TO AICA WAVE MEM?) !\n\n", SB_GDDIR);

	SB_GDLEN = 0x00000000;
	SB_GDSTAR = (src + len);
	SB_GDST=0;//done

	// The DMA end interrupt flag (SB_ISTNRM - bit 19: DTDE2INT) is set to "1."
	RaiseInterrupt(holly_GDROM_DMA);



	if(0 >= (dmaCount-len))	// DMAs Are Finished
	{
		dmaCount = 0;
		dmaOffset = 0;
		rSTATUS.BSY = 0;
		rSTATUS.DRDY = 1;
		gdSetSR(GD_STATUS_OK);
		RaiseInterrupt(InterruptID::holly_GDROM_CMD);
		lprintf("GD DMA Complete, Finished!\n");
	}
	else
	{
complete_dma_chain:
		static u32 last = 0x10000000;

		if(dmaCount < (int)(last-0x1000)) {
			last = dmaCount;
			lprintf("GD DMA Complete, %d Remaining!\n", dmaCount);
		}

		dmaCount -= len;
		dmaOffset += len;
	}

}







char gdreg_names[64][64] =
{
	"IMPEDHI0", "IMPEDHI4", "IMPEDHI8", "IMPEDHIC", 
	"_ERROR_", "_ERROR_", 
	"ALTSTAT/DEVCTRL",
	"_ERROR_",
	"_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", 
	"_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", 
	"_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", 
	"DATA",
	"ERROR/FEATURES",
	"IREASON/SECTCNT",
	"SECTNUM",
	"BYCTLO",
	"BYCTHI",
	"DRVSEL",
	"STATUS/COMMAND",
	"_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", 
	"_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", 
	"_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_", "_ERROR_"
};


//#define _LOG_GD_

#include <stdarg.h>
void lprintf(char* szFmt, ... )
{
#ifdef _LOG_GD_
	static u32 First=1;

	FILE * f;
	if(First)
		f = fopen("GDROM.log",(First--)?"wt":"wt");
	else
		f = fopen("GDROM.log","a+t");
	if(!f) return;

	va_list va;
	va_start(va, szFmt);
	vfprintf_s(f,szFmt,va);
	va_end(va);

	fclose(f);
#endif //_LOG_GD_
}


void logd(u32 rw, u32 addr, u32 data)
{
	if(0==rw && GD_DATA==(addr&255))
	{
		if(gdSR.ByteCount > 0x20) {
			if(0x7F != (databuff.size() &0x7F))
				return;
			lprintf("(R)[%08X] %s (%i remaining)\n", addr, gdreg_names[(addr&255)>>2], databuff.size());
			return;
		}
	}	

	if(1==rw && GD_COMMAND==(addr&255))
	{
		switch(data&255) {
		case GDC_NOP:			lprintf("\nGDC:\tNOP!\n\n");		break;
		case GDC_SFT_RESET:		lprintf("\nGDC:\tSFT_RESET\n\n");	break;
		case GDC_EXEC_DIAG:		lprintf("\nGDC:\tEXEC_DIAG\n\n");	break;
		case GDC_IDENTIFY_DEV:	lprintf("\nGDC:\tIDENTIFY_DEV\n\n");break;

		case GDC_SET_FEATURES:
			lprintf("\nGDC:\tSET_FEATURES: %s feature %X %s r(%X)\n\n",
				(rFEATURES&0x80)?"Set":"Clear", (rFEATURES&0x7F),
				(rFEATURES&0x80)?":=":"-", rSECTCNT);
			break;

		case GDC_SPI_PACKET:
			lprintf("\nGDC:\tSPI_PACKET\n\n");
			break;

		default: lprintf("\n!GDC:\tERROR: Unknown GDROM COMMAND!\n\n");	break;
		}
		return;
	}

	if(0==rw)
		lprintf("(R)[%08X] %s \n", addr, gdreg_names[(addr&255)>>2]);
	else if(1==rw)
		lprintf("(W)[%08X] %s := %X \n", addr, gdreg_names[(addr&255)>>2], data);

}



















////////////////////////////////////////////////////////////////////////////////////////////////////////////




/*
**	HLE Section
*/

void gdBootHLE(void)
{
	printf("\n~~~\tgdBootHLE()\n\n");

	u32 toc[102];
	libGDR->gdr_info.GetToc(&toc[0], (DiskArea)1);

	int i=0;
	for(i=98; i>=0; i--)
	{
		if (toc[i]!=0xFFFFFFFF)
		{
			if(4 == (toc[i]&4))
				break;
		}
	}
	if (i==-1) i=0;
//	u32 addr = ((toc[i]&0xFF00)<<8) | ((toc[i]>>8)&0xFF00) | ((toc[i]>>24)&255);

	///////////////////////////
	u8 * pmem = &mem_b[0x8000];
	libGDR->gdr_info.ReadSector(pmem, 0, 16, 2048);	// addr?

	char bootfile[16] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	for(i=0; i<16; i++) {
		if(0x20 == pmem[0x60+i])
			break;
		bootfile[i] =  pmem[0x60+i];
	}

	printf("IP.BIN BootFile: %s\n", bootfile);

}










void gdop(u32 opcode)
{
	printf("(GD-HLE) UNIMPLEMENTED !\n\n");
}

#else	// BUILD_NAOMI TRUE


void gdop(u32 opcode) { printf("ERROR - (GD-HLE) OP IN NAOMI SECTION - !\n\n"); }
void gdBootHLE(void) { printf("ERROR - (GD-HLE) BOOT IN NAOMI SECTION - !\n\n"); }

void NotifyEvent_gdrom(DriveEvent info, void* param) { 
	printf("ERROR - NotifyEvent_gdrom() IN NAOMI SECTION - !\n\n");
}


void gdrom_reg_Init(void)
{
}
void gdrom_reg_Reset(bool)
{
}
void gdrom_reg_Term(void)
{
}


// DIMM_ might be bad naming, and some goto IO, all these regs are handled
// by the asic, so oh well?

enum
{
	DIMM_ROM_OFFSHI	= 0x00,
	DIMM_ROM_OFFSLO	= 0x04,
	DIMM_ROM_DATA	= 0x08,

	DIMM_DMA_OFFSHI	= 0x0C,
	DIMM_DMA_OFFSLO	= 0x10,
	DIMM_DMA_COUNT	= 0x14,

	DIMM_COMM_OFFS	= 0x50,
	DIMM_COMM_DATA	= 0x54,

	DIMM_ID_WRITE	= 0x78,
	DIMM_ID_READ	= 0x7C,
};


u32  ReadMem_gdrom(u32 Addr, u32 sz)
{
	switch((Addr&255))
	{

	case 0x4C:		// no clue, status?
		return 1;

	case 0:
	default:
		break;
	}

	printf("(N) ReadMem_gdrom(%08X, %d) Unhandled\n",Addr,sz);
	return 0;
}

void WriteMem_gdrom(u32 Addr, u32 data, u32 sz)
{
	printf("(N) WriteMem_gdrom(%08X, %08X, %d)\n",Addr,data,sz);

}












#endif	// BUILD_NAOMI

#endif	// ZGDROM