/*
**	zGDROM.cpp
*/

#ifndef OLD_GDROM
#include <vector>
using namespace std;

#include "gdrom_if.h"
#include "dc/mem/sb.h"
#include "dc/sh4/sh4_if.h"


// A few vectors or lists for our shitt
vector<u16> cmdbuff;	// max 16 bytes
vector<u16> databuff;	// max ???


static u32 rERROR=0;
static u32 rFEATURES=0;
static u32 rSECTCNT=0;
static u32 rDRVSEL=0;

static u32 rIMPEDHI0=0, rIMPEDHI4=0;
static u32 rIMPEDHI8=0, rIMPEDHIC=0;

static gdStatusReg	rSTATUS;
static gdIReasonReg	rIREASON;



// Going to K.I.S.S. It ;) hehe
// One basic function for setting IO,DRQ,CoD never do it manually, 
// have one nice internal Status Reg that will get other needed info
// from plugin too ...

static gdromSR		gdSR;
static gdReqMode	gdRM;

static void gdSetSR(gdIntStatus sr)
{
	gdSR.iStatus = (u8)sr;

	rIREASON.IO	= gdSR.IO;
	rIREASON.CoD= gdSR.CoD;
	rSTATUS.DRQ	= gdSR.DRQ;

	// ToDo: Use SR to add other bits as well ie: BSY or DRDY and STATE portion of iDevType
}


//////////////////////
void ProcessSPI(void);

void lprintf(char* szFmt, ... );
void logd(u32 rw, u32 addr, u32 data);

void TestWrite(u32 data) {
	printf(" \n\n ******* GDROM TESTWRITE to 4e4 : %X \n", data);
}
/////////////////////////////////////////////////////////////////
const static char DriveInfo[9]	= "SE      ";
const static char SystemInfo[9]	= "Rev 6.42";
const static char SystemDate[7]	= "990316";

void gdrom_reg_Init(void)
{
	logd(420,0,0);

	rERROR = 1;
	rSTATUS.Full = 0;
	rIREASON.Full= 0;
	
	gdSR.iStatus	= GD_RESET_SR;
	gdSR.iDevType	= GDFORMAT_XA | GDSTATE_STANDBY;

	// Setup ReqMode
	gdRM.zResv1	= 0;
	gdRM.zResv2	= 0;
	gdRM.zResv3	= 0;
	gdRM.CDROM_Speed	= 0;
	gdRM.Standby_Time	= 0xB400;	// 0x00B4 | B400 ?
	gdRM.Read_Settings	= 0x19;
	gdRM.Read_Retry		= 0x08;

	memcpy(gdRM.DriveInformation, DriveInfo, 8);
	memcpy(gdRM.SystemVersion, SystemInfo, 8);
	memcpy(gdRM.SystemDate, SystemDate, 6);

/*
	Cylinder Low = 14h,
	Cylinder High = EBh,
	Drive/Head = 00h 
	BSY = 0 following after any reset indicates that the task file register is already initialized for the host. 
*/
	sb_regs[(0x5F74E4-SB_BASE)>>2].flags = REG_32BIT_READWRITE | REG_16BIT_READWRITE | REG_8BIT_READWRITE | REG_READ_DATA;
	sb_regs[(0x5F74E4-SB_BASE)>>2].writeFunction = TestWrite;
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

	case GD_ERROR:		return rERROR;

	case GD_IREASON:	return rIREASON.Full;

	case GD_SECTNUM:	return gdSR.iDevType;
	case GD_BYCTLLO:	return gdSR.ByteCountLO;
	case GD_BYCTLHI:	return gdSR.ByteCountHI;

	case GD_DRVSEL:		return rDRVSEL;

	case GD_STATUS:
		SB_ISTEXT &= ~1;
		return rSTATUS.Full;

	case GD_ALTSTAT:	return rSTATUS.Full;


	case GD_DATA:

		if(GD_RECV_DATA == gdSR.iStatus)
		{
			if(databuff.size() > 0)
			{
				u16 tdata = databuff.back();
				databuff.pop_back();
				gdSR.ByteCount -= 2;

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
	case GD_BYCTLLO:	printf("Write to GD_BYCTLLO\n");	break;
	case GD_BYCTLHI:	printf("Write to GD_BYCTLHI\n");	break;
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
		goto complete;	// thats all

	case SPI_CD_OPEN:
	case SPI_CD_PLAY:
	case SPI_CD_SEEK:
	case SPI_CD_SCAN:
		break;


	case SPI_REQ_STAT:	// pio from host
		break;

	case SPI_REQ_MODE:	// max 32 bytes

		if((cmd[2]+cmd[4]) >= 32) {
			printf("!(GD)\tERROR: SPI_REQ_MODE Value Out of Range! %02X+%02X Unhandled!\n", cmd[2], cmd[4]);
			break;
		}

		lprintf("(GD)\tSPI_REQ_MODE: %02X+%02X !\n\n", cmd[2], cmd[4]);

		for(int x=(int)(cmd[4]-2); x>=0; x-=2)
			databuff.push_back(gdRM.Words[cmd[2]+x]);	// ? uh.. i think so 

		gdSR.ByteCount = cmd[4];
		goto pio_complete;


	case SPI_REQ_ERROR:		break;
	case SPI_GET_TOC:		break;

	case SPI_REQ_SES:
		lprintf("(GD)\tSPI_REQ_SES: %02X Len:%02X !\n\n", cmd[2], cmd[4]);

//#define bswap16(x)	((((x)&255)<<8) | (((x)>>8)&255))

		u16 SessionInfo[3];
		libGDR->gdr_info.GetSessionInfo((u8*)&SessionInfo[0], cmd[2]);

		for(int x=2; x>=0; x--)
			databuff.push_back(SessionInfo[x]);

		gdSR.ByteCount = cmd[4];
		goto pio_complete;


	case SPI_GET_SCD:
		break;

	case SPI_SET_MODE:	// pio to host
		break;

	case SPI_CD_READ:	// dma or pio
	case SPI_CD_READ2:
		break;
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

//	7.	When preparations are complete, the following steps are carried out at the device. 
//		(1)	Number of bytes to be read is set in "Byte Count" register. 
	return;

dma_complete:

	return;
}




void NotifyEvent_gdrom(DriveEvent info, void* param)
{
	lprintf("!(GD)\tERROR: GDROM Event %X Unhandled!\n\n", (u32)info);
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

#endif