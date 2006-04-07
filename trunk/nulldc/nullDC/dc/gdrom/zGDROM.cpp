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


#ifdef ZGDROM



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

static u8 gdReadBuffer[1024*1024*8];		// TEMP *FIXME*


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
	gdRS.FAD[3] = 0x96;	// home
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

					if(1)	// no more data (check original expected bytecnt?)
					{
						rSTATUS.BSY = 0;
						rSTATUS.DRDY = 1;
						gdSetSR(GD_STATUS_OK);
						RaiseInterrupt(InterruptID::holly_GDROM_CMD);
					}
					else {
						// set bytecnt and raise int.
						RaiseInterrupt(InterruptID::holly_GDROM_CMD);
					}
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

		gdSR.ByteCount = len;
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


	case SPI_GET_SCD:
		break;


	case SPI_SET_MODE:	// pio from host

		if((cmd[2]+cmd[4]) > 32) {
			printf("!(GD)\tERROR: SPI_SET_MODE Value Out of Range! %02X+%02X Unhandled!\n", cmd[2], cmd[4]);
			break;
		}

		lprintf("(GD)\tSPI_SET_MODE: %02X+%02X !\n\n", cmd[2], cmd[4]);


	//	gdSR.ByteCount = cmd[4];
	//	goto pio_complete;				// *FIXME* this should be wrong ?
		break;


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

			gdSR.ByteCount = Length*2048;	// *FIXME* test against cmd[1]
			goto pio_complete;				// *FIXME* 
		}
	  }

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

	if( 1 == SB_GDDIR ) {
		memcpy( &mem_b[src&0xFFFFFF], &gdReadBuffer[dmaOffset], len );

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

		if(dmaCount < (last-0x1000)) {
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


#include <stdarg.h>
void lprintf(char* szFmt, ... )
{
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




/*
**	HLE Section
*/
#include <ctype.h>

enum IMG_ERR
{
	IMG_NOTHING	= 0x00000000,	/* IMG: No Error */
	IMG_OPEN	= 0x10000000,	/* IMG: Open */
	IMG_CLOSE	= 0x10000001,	/* IMG: Close */

	ISO_FS_INIT = 0x30000000,	/* ISO9660: Filesystem initialization */
	ISO_FS_LOCTRK=0x30000001,	/* ISO9660: Locate data track */
	ISO_FS_RDSEC= 0x30000002,	/* ISO9660: Read root sector failure */
	ISO_FS_CHK  = 0x30000003,	/* ISO9660: ISO String compare check error */
	ISO_FS_NOBOOT=0x30000004,	/* ISO9660: No Bootfile found */
	ISO_FS_RD_BT= 0x30000005,	/* ISO9660: Read Bootfile failure */

	DCGD_SOCKET = 0x60000000,	/* DC GDROM: Socket create failed */
	DCGD_GETHOST= 0x60000001,	/* DC GDROM: Get Host */
	DCGD_CONNECT= 0x60000002,	/* DC GDROM: Connect */
	DCGD_SEND	= 0x60000003,	/* DC GDROM: Send */
	DCGD_RECV	= 0x60000004,	/* DC GDROM: Recv */
};
typedef enum IMG_ERR	IMG_ERR;


typedef struct {
	
	u32 entry[99];
	u32 first;
	u32 last;
	u32 leadout;
} GDROM_TOC;

GDROM_TOC gdTOC;

extern u32 session_base;

#define TRUE	(1)
#define FALSE	(0)

#define _SWAP32(w)	((((w)>>24)&255) | (((w)>>8)&0xFF00) | (((w)&0xFF00)<<8) | (((w)&255)<<24))

/* TOC access macros 
#define TOC_LBA(n) ( _SWAP32(n) & 0x00ffffff)
#define TOC_ADR(n) ( _SWAP32((n) & 0x0f000000) >> 24 )
#define TOC_CTRL(n) ( _SWAP32((n) & 0xf0000000) >> 28 )
#define TOC_TRACK(n) ( _SWAP32((n) & 0x00ff0000) >> 16 )
*/
#define TOC_ADR(n)	( ((n) & 0x0000000F) )
#define TOC_CTRL(n)	( ((n) & 0x000000F0) >> 4)
#define TOC_TRACK(n)( ((n) & 0x0000FF00) >> 8)
#define TOC_LBA(n)	( _SWAP32(n) & 0x00FFFFFF)

IMG_ERR ISO_FSInit();
IMG_ERR ISO9660_LoadFile(char filename[16],u32 dwAddress,bool Descramble);

void SCR_Descramble(u8 *ptr, u8 *srcbuf, u32 filesz);



//#define USE_IC_SRC

bool iso9660_Init(u32 session_base);
bool iso9660_LoadFile(char filename[16], u32 dwAddress, bool Descramble);



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
	if (i==-1)
		i=0;
	u32 addr = ((toc[i]&0xFF00)<<8) | ((toc[i]>>8)&0xFF00) | ((toc[i]>>24)&255);

	///////////////////////////
	u8 * pmem = &mem_b[0x8000];
	libGDR->gdr_info.ReadSector(pmem, 0, 16, 2048);

	char bootfile[16] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	for(i=0; i<16; i++) {
		if(0x20 == pmem[0x60+i])
			break;
		bootfile[i] =  pmem[0x60+i];
	}

	printf("IP.BIN BootFile: %s\n", bootfile);



	if(!iso9660_Init(addr))
		printf("GDHLE: ERROR: iso9660_Init() Failed!\n\n");
	if(!iso9660_LoadFile(bootfile, 0x8c010000, true))
		printf("GDHLE: ERROR: iso9660_LoadFile() Failed!\n\n");

}
















//////// ISO 9660 Shit

#include <ctype.h>

/*************************************************************/

// This seems kinda silly, but it's important since it allows us
//to do unaligned accesses on a buffer 
static u32 htohl_32(const void *data) {
	const u8 *d = (const u8*)data;
	return (d[0] << 0) | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
}

// Read red-book section 7.3.3 number (32 bit LE / 32 bit BE) 
static u32 iso_733(const u8 *from) { return htohl_32(from); }

// ISO Directory entry 
typedef struct {
	u8	length;				// 711 //
	u8	ext_attr_length;	// 711 //
	u8	extent[8];			// 733 //
	u8	size[8];			// 733 //
	u8	date[7];			// 7x711 //
	u8	flags;
	u8	file_unit_size;		// 711 //
	u8	interleave;			// 711 //
	u8	vol_sequence[4];	// 723 //
	u8	name_len;			// 711 //
	char	name[1];
} iso_dirent_t;

// Root FS session location (in sectors) 
//u32 session_base = 0;

// Root directory extent and size in bytes 
static u32 root_extent = 0, root_size = 0;

// Root dirent 
static iso_dirent_t root_dirent;



/*
// Locate the LBA sector of the data track; use after reading TOC 
u32 ISO_LocateDataTrack(GDTOC toc)
{
	int i, first, last;

	first = TOC_TRACK(toc.first);
	last = TOC_TRACK(toc.last);

	printf("ISO9660: First track: %d, Last track: %d",first,last);
	if (last>1)
		printf("[BNote: This is a selfbootable dreamcast disc]");
	else
		printf("[GNote: This is a non-selfbootable dreamcast disc]");	

	if (first < 1 || last > 99 || first > last)
		return 0;

	// Find the last track which has a CTRL of 4 
	for (i=last; i>=first; i--) {
		printf("Track %i, Ctrl %d, Lba %d",i,TOC_CTRL(toc.entry[i - 1]),TOC_LBA(toc.entry[i - 1]));
		if (TOC_CTRL(toc.entry[i - 1]) == 4)
			return TOC_LBA(toc.entry[i - 1]);
	}
	printf("Warning: No track with CTRL=4. Returning last track info (%d,0x%08x)",last-1,toc.entry[last-1]);
	return TOC_LBA(toc.entry[last-1]);
	//return 0xFFFFFFFF;
}*/

/* Compare an ISO9660 filename against a normal filename. This takes into
account the version code on the end and is not case sensitive. Also
takes into account the trailing period that some CD burning software
adds. */
static int fncompare(const char *isofn, int isosize, const char *normalfn) {
	int i;

	/* Compare ISO name */
	for (i=0; i<isosize; i++) {
		/* Weed out version codes */
		if (isofn[i] == ';') break;

		/* Deal with crap '.' at end of filenames */
		if (isofn[i] == '.' &&
			(i == (isosize-1) || isofn[i+1] == ';'))
			break;

		/* Otherwise, compare the chars normally */
		if (tolower(isofn[i]) != tolower(normalfn[i]))
			return -1;
	}

	/* Catch ISO name shorter than normal name */
	if (normalfn[i] != '/' && normalfn[i] != '\0')
		return -1;
	else
		return 0;
}

/* Locate an ISO9660 object in the given directory; this can be a directory or
a file, it works fine for either one. Pass in:

fn:		object filename (relative to the passed directory)
dir:		0 if looking for a file, 1 if looking for a dir
dir_extent:	directory extent to start with
dir_size:	directory size (in bytes)

It will return a pointer to a transient dirent buffer (i.e., don't
expect this buffer to stay around much longer than the call itself).
*/

static iso_dirent_t * iso9660_FindFile(char filename[16], u32 dir_extent, u32 dir_size)
{
	u32		i;
	iso_dirent_t	*de;
	u8 dir=0; //We are just looking for a file
	int		len;
	u8		*pnt;
	char	rrname[16];
	int		rrnamelen;
	u8 Data[2048]; /* Sector data */

	while (dir_size > 0) {
		libGDR->gdr_info.ReadSector(Data, dir_extent, 1, 2048);
	//	if (!IMG_ReadSector(Data, dir_extent, 1))
	//		return NULL;

		for (i=0; i<2048 && i<dir_size; ) {
			/* Assume no Rock Ridge name */
			rrnamelen = 0;

			/* Locate the current dirent */
			de = (iso_dirent_t *)(&Data[i]);
			if (!de->length) break;

			/* Check for Rock Ridge NM extension */
			len = de->length - sizeof(iso_dirent_t)
				+ sizeof(de->name) - de->name_len;
			pnt = (u8*)de + sizeof(iso_dirent_t)
				- sizeof(de->name) + de->name_len;
			if ((de->name_len & 1) == 0) {
				pnt++; len--;
			}
			while ((len >= 4) && ((pnt[3] == 1) || (pnt[3] == 2))) {
				if (strncmp((const char *)pnt, "NM", 2) == 0) {
					rrnamelen = pnt[2] - 5;
					strncpy((char *)rrname, (const char *)pnt+5, rrnamelen);
					rrname[rrnamelen] = 0;
				}
				len -= pnt[2];
				pnt += pnt[2];
			}


			/* Check the filename against the requested one */
			if (rrnamelen > 0) {//Rock Ridge NM extension
				char *p = strchr(filename, '/');
				int fnlen;

				if (p)
					fnlen = p - filename;
				else
					fnlen = strlen(filename);

				if (!strnicmp(rrname, filename, fnlen)) {
					if (!((dir << 1) ^ de->flags))
						return de;
				}
			} else { //No Rock Ridge Name
//#ifdef ICARUS_LOG
				printf("File=%s : Extent=%d : Size=%d", de->name,iso_733(de->extent),iso_733(de->size));
//#endif
				if (!fncompare(de->name, de->name_len, filename)) {
					if (!((dir << 1) ^ de->flags))
						return de;
				}
			}

			i += de->length;
		}

		dir_extent++;
		dir_size -= 2048;
	}

	return NULL;
}


////// scramble.c

#define MAXCHUNK (2048*1024)

static unsigned int seed;

void my_srand(unsigned int n)
{
	seed = n & 0xffff;
}

unsigned int my_rand()
{
	seed = (seed * 2109 + 9273) & 0x7fff;
	return (seed + 0xc000) & 0xffff;
}

void load_chunk(u8 *ptr, u8 *srcbuf, u32 sz)
{
	static int idx[MAXCHUNK/32];
	int i;

	/* Convert chunk size to number of slices */
	sz /= 32;

	/* Initialize index table with unity,
	so that each slice gets loaded exactly once */
	for(i = 0; i < (signed long) sz; i++)
		idx[i] = i;

	for(i = sz-1; i >= 0; --i)
	{
		/* Select a replacement index */
		int x = (my_rand() * i) >> 16;

		/* Swap */
		int tmp = idx[i];
		idx[i] = idx[x];
		idx[x] = tmp;

		/* Load resulting slice */
		memcpy(ptr+32*idx[i],srcbuf,32);
		srcbuf+=32;
	}
}


void SCR_Descramble(u8 *ptr, u8 *srcbuf, u32 filesz)
{
	unsigned long chunksz;

	my_srand(filesz);

	/* Descramble 2 meg blocks for as long as possible, then
	gradually reduce the window down to 32 bytes (1 slice) */
	for(chunksz = MAXCHUNK; chunksz >= 32; chunksz >>= 1)
		while(filesz >= chunksz)
		{
			load_chunk(ptr, srcbuf, chunksz);
			filesz -= chunksz;
			srcbuf+=chunksz;
			ptr += chunksz;
		}

		/* Load final incomplete slice */
		if(filesz)
			memcpy(ptr,srcbuf,filesz);
}






/*

void load_chunk(unsigned char *ptr, unsigned long sz)
{
	static int idx[MAXCHUNK/32];
	int i;

	// Convert chunk size to number of slices 
	sz /= 32;

	// Initialize index table with unity,
	//so that each slice gets loaded exactly once 
	for(i = 0; i < sz; i++)
		idx[i] = i;

	for(i = sz-1; i >= 0; --i)
	{
		// Select a replacement index 
		int x = (my_rand() * i) >> 16;

		// Swap 
		int tmp = idx[i];
		idx[i] = idx[x];
		idx[x] = tmp;

		// Load resulting slice 
		memcpy(ptr+32*idx[i],  32);
	}
}

void load_file(unsigned char *ptr, unsigned long filesz)
{
	unsigned long chunksz;

	my_srand(filesz);

	// Descramble 2 meg blocks for as long as possible, then
	//gradually reduce the window down to 32 bytes (1 slice) 
	for(chunksz = MAXCHUNK; chunksz >= 32; chunksz >>= 1)
	{
		while(filesz >= chunksz)
		{
			load_chunk(fh, ptr, chunksz);
			filesz -= chunksz;
			ptr += chunksz;
		}
	
		// Load final incomplete slice 
		if(filesz)
			load(fh, ptr, filesz);
	}

}*/

void Descram(u8 * dst, u8 * src, u32 size)
{
	unsigned char *ptr = NULL;
	unsigned long sz = size;
	FILE *fh;

//	read_mem(src, &ptr, &sz);

//	load_file()

	fclose(fh);
	free(ptr);
}


bool iso9660_LoadFile(char filename[16], u32 dwAddress, bool Descramble)
{
	//IMG_ERR error;
	//	u32 bytesread;
	iso_dirent_t	*de;

	// Now we have to search through the directory record to find the relevant bootfile 
	de = iso9660_FindFile(filename,iso_733(root_dirent.extent), iso_733(root_dirent.size));
	if (de==NULL) { 
		printf("\n~!\tERROR: No %s file found in iso9660.\n",filename); 
		return false; 
	}

	u32 first_sector = iso_733(de->extent);
	u32 size = iso_733(de->size);

	// Now we have the descriptor info for the bootfile, we have to read its data 
	printf("\nGDHLE: First sector=0x%08x,Size=0x%08x,No.Full Sectors=%d,Last sec. bytes=%d\n",first_sector,size,size/2048,size%2048);

	// ISO9660_ReadBootFile(first_sector,size,dwAddress,Descramble);
	u8 Data[2048]; /* Sector data */

	u8 *pBuf=(u8 *)(&mem_b[dwAddress& 0xFFFFFF]);
	u32 no_full_sectors=size/2048;

	printf("ISO9660: Reading 1st_read.bin from disc...\n");
	libGDR->gdr_info.ReadSector(pBuf, first_sector, no_full_sectors, 2048);

	if ((size%2048)!=0) /* Copy the last remaining bytes manually */
	{
		libGDR->gdr_info.ReadSector(Data, no_full_sectors+first_sector, 1, 2048);
		pBuf=(u8 *)(&mem_b[(dwAddress + no_full_sectors*2048)& 0xFFFFFF]);
		memcpy(pBuf,&Data[0],size%2048);
	}	

	if (Descramble)
	{
		u8 *pTmp= new u8[size];
		u8 *pSrc=(u8 *)(&mem_b[0x10000]);
		SCR_Descramble(pTmp,pSrc,size);
		memcpy(pSrc,pTmp,size);
		delete[] pTmp;
		printf("ISO9660: [B%s descrambled successfully]",filename);
	}

	return true;
}




/******** ISO9660 Filesystem ****************************************/
/* Used to load the 1stread.bin or other binary specified in ip.bin */
/********************************************************************/

bool iso9660_Init(u32 session_base)
{
	u8 Data[2048]; // Sector data 
	libGDR->gdr_info.ReadSector(Data, (session_base+16), 1, 2048);

	if (memcmp(&Data[0], "\01CD001", 6)) {
		printf("ISO: Disc is not iso9660 Data:(%s)\n", (char *)&Data[0]);
		u8 string[7]="\01CD001";
		printf("IMAGE :%02x%02x%02x%02x%02x%02x\n",string[0],string[1],string[2],string[3],string[4],string[5]);
		printf("ISOCRC:%02x%02x%02x%02x%02x%02x\n",Data[0],Data[1],Data[2],Data[3],Data[4],Data[5]);
		return false;
	}

	// Locate the root directory 
	memcpy(&root_dirent, &Data[156], sizeof(iso_dirent_t));
	root_extent = iso_733(root_dirent.extent);
	root_size = iso_733(root_dirent.size);
	printf("ISO9660: dirent:extent=0x%08x, size=0x%08x, length=0x%08x\n",iso_733(root_dirent.extent),iso_733(root_dirent.size),root_dirent.length);
	printf("ISO_FSInit complete successfully\n");

	return true;
}









void gdop(u32 opcode)
{
	printf("(GD-HLE) UNIMPLEMENTED !\n\n");
}

#endif