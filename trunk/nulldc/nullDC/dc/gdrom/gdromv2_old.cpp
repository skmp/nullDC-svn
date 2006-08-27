//new implementation of teh gd rom lle :)
//Needs to be cleaned up

#include "gdrom_if.h"

#ifdef OLD_GDROM
#include "types.h"
#include "plugins/plugin_manager.h"

#include "dc/mem/sh4_mem.h"
#include "dc/mem/memutil.h"
#include "dc/mem/sb.h"
#include "dc/sh4/dmac.h"
#include "dc/sh4/intc.h"
#include "dc/sh4/sh4_registers.h"

#include <list>

//function declarations start
bool InitGDROM(void);
void TermGDROM(void);
void NotifyEvent_gdrom(DriveEvent info,void* param);
u32 ReadMem_gdrom(u32 Addr, u32 sz);
void WriteMem_gdrom(u32 Addr, u32 data, u32 sz);
void UpdateGDRom();
bool ProcessSPI(SpiCommandInfo* cmd);
void gd_DoDMA();
void GDROM_DmaSt(void);
//end


//Registers
	u32 DriveSel;
	GD_ErrRegT Error;
	GD_InterruptReasonT IntReason;
	GD_FeaturesT Features;
	GD_SecCountT SecCount;
	GD_SecNumbT SecNumber;
	
	GD_StatusT GDStatus;
	GD_btCntT ByteCount;
//end

gdr_plugin_if* drive;

//These two can be implemented on a mem lighter way :)
//but this will come later :)
//the fifo buffer is implemented this way :)
std::list<u16> * gd_fifo=new std::list<u16>();
//the spi buffer too
Stack<u16> spi_buff;

//The Pending Command  :)
SpiCommandInfo t_cmdmem;
SpiCommandInfo* PendSpicmd=0;// this is 0 if none is pending


u8 DataBuffer[8*1024*1024];// for gd rom DMA :) , 1 mb , hope will not need more :) -> need to change to do reads dynamicaly
bool gd_dma_pending=false;
bool spi_set_md=false;
u32 uPosCommandBuffer=0;
u32 uNumBytesWrite=0;
bool gd_inBios=true;//boot from bios [hacks to get around gdrom offsets]
bool dma_data=false;
u32 DataIndex=0;
u32 DataLength=0;
u32 DmaSubCount=0;





//////// TEMP ?
# define _LOG_ (1)
#include "stdarg.h"

void lprintf(char* szFmt, ... )
{
#ifdef _LOG_
	FILE * f = fopen("gdv2.txt","a+t");

	va_list va;
	va_start(va, szFmt);
	vfprintf_s(f,szFmt,va);
	va_end(va);

	printf(szFmt,va);

	fclose(f);
#endif
}





//shits for debug printf's :P
//#define DebugGD
#ifndef DebugGD
#define printf_db NullPrintf_GD
void NullPrintf_GD(...){}
#else
#define printf_db lprintf
#endif


//
//Sends Bytes To Dreamcast From GDROM unit
void SendBytesToHost(u32 cnt,u8* buff)
{
	//ByteCount.Low = (u8)(cnt);
	//ByteCount.Hi = (u8)(cnt>>8);
	ByteCount.full=(u16)cnt;
	

	u32 dlen=cnt>>1;
	u16* buf16=(u16*)buff;

	for (u32 i=0;i<dlen;i++)
	{
		//!TODO need to change to a new fifo format
		gd_fifo->push_back(buf16[i]);
	}
}

//
//inits GD rom emulation and drive handler :)
FILE* gd_log;
bool InitGDROM(void)
{
	gd_log=fopen("gdrom.log.txt","w");
	//we must Get pointers here :)
	drive=&libGDR->gdr_info; //GetIsoDrive();//cdi_GetDrive();//GetIsoDrive();//Get_IcIsoDrive("Img_Iso.dll");

	//init registers
	DriveSel		=0xA1;		// right ?
	Error.full		= 1;	// no error
	IntReason.full 	= 1;
	Features.full 	= 0;
	SecCount.full 	= 0;
	SecNumber.full  = 0x82;

	GDStatus.full=0;
	GDStatus.DRDY=1;

	ByteCount.full=0;

	//init responces
	//gd_InitData();

	return true;
}

//
//Terminates GD rom emulation
void TermGDROM(void)
{
	//must be called first
	//drive->Term();
}

//
//Drive handler callback
void NotifyEvent_gdrom(DriveEvent info,void* param)
{
	switch(info)
	{
		case DiskChange:
			if (drive->GetDiskType()==NoDisk)
			{
				//y know , this must be filed some day :) lol
				SecNumber.DiscFormat= 0;
				SecNumber.Status=5;//opened
			}
			else
			{
				//o.O is this suposed to work ??
				SecNumber.DiscFormat=drive->GetDiskType()>>4;
				SecNumber.Status=2;
			}
			break;

		default:
			printf("Unk DriveEvent.. %d\n",info);
			break;
	}
}

//
//Memory Read Handler ;)
u32 ReadMem_gdrom2(u32 Addr, u32 sz);
u32 ReadMem_gdrom(u32 Addr, u32 sz)
{
	u32 data=ReadMem_gdrom2(Addr,sz);
	//if (Addr!=0x5F7080)
		//fprintf(gd_log,"Read: 0x%X=%x[%x]\n",Addr,data,pc);
	return data;
}
u32 ReadMem_gdrom2(u32 Addr, u32 sz)
{
	
	switch (Addr)
	{
		case GD_STATUS :
			SB_ISTEXT &= ~1;
			printf_db("GDROM:\t STATUS \\ cancel int;\n");
			return GDStatus.full;

		case GD_ALTSTAT:
			printf_db("GDROM:\t Read From AltStatus\n");
			return GDStatus.full;

		case GD_BYCTLLO	:
			printf_db("GDROM:\t Read From GD_BYCTLLO\n");
			return ByteCount.Low;
		
		case GD_BYCTLHI	:
			printf_db("GDROM:\t Read From GD_BYCTLHI\n");
			return ByteCount.Hi;
		

		case GD_DATA:	// *FIXME*
			if(2!=sz)
				printf_db("\nGDROM:\tBad size on DATA REG Read !\n");

			if( !gd_fifo->empty() )
			{
				if( gd_fifo->size() == 1 ) {
					GDStatus.DRDY=1;
					GDStatus.DRQ=0;
					GDStatus.BSY=0;

					IntReason.IO=1;
					IntReason.CoD=1;
					
					RaiseInterrupt(holly_GDROM_CMD);				// do this here then ?
				}
				u16 rv=gd_fifo->front();
				gd_fifo->pop_front();
	
				printf_db("\nGDROM:\tRead From DATA %x!\n",rv);
				return rv;
			}

			printf("GDROM:\tIllegal Read From DATA (underflow) !\n");
			return 0;

		case GD_DRVSEL:
			printf_db("GDROM:\t Read From DriveSel\n");
			return DriveSel;
		
		case GD_ERROR:	// Drv=BSY set ABRT ??? (for async io)
			printf_db("\nGDROM:\tRead from ERROR Register!\n");
			return Error.full;
		
		case GD_IREASON	:
			printf_db("\nGDROM:\tRead from INTREASON Register!\n");
			return IntReason.full;
		
	/*	Bit 0 (CoD): 	"0" indicates data and "1" indicates a command. 
		Bit 1 (IO): 	"1" indicates transfer from device to host, and "0" from host to device.
		IO	DRQ	CoD
		0	1	1	Command packet can be received.
		1	1	1	Message can be sent from device to host.
		1	1	0	Data can be sent to host.
		0	1	0	Data can be received from host.
		1	0	1	Status register contains completion status.
	*/
		// The info obtained by this reg is the same as obtained with the REQ_STAT command. 
		case GD_SECTNUM:
			printf_db("\nGDROM:\tRead from SecNumber Register (v=%X)\n", SecNumber.full);
			return SecNumber.full;
		
		default:
			printf("\nGDROM:\tUnhandled Read From %X sz:%X\n",Addr,sz);
			return 0;
	}
}
//
//Memory Write Handler ;)
void WriteMem_gdrom(u32 Addr, u32 data, u32 sz)
{
	//if (Addr!=0x5F7080)
	//fprintf(gd_log,"Write: 0x%X=%x[%x]\n",Addr,data,pc);
	switch(Addr)
	{
	case GD_BYCTLLO:
		printf_db("\nGDROM:\tWrite to %X <= %X sz:%X\n",Addr,data,sz);
		ByteCount.Low =(u8) data;
		break;

	case GD_BYCTLHI: 
		printf_db("\nGDROM:\tWrite to %X <= %X sz:%X\n",Addr,data,sz);
		ByteCount.Hi =(u8) data;
		break;

	case GD_DATA: 
		{	// *FIXME* ... yea
			if(2!=sz)
				printf("\nGDROM:\tBad size on DATA REG !\n");

			if (spi_set_md)
			{
				g_aValues0x11[uPosCommandBuffer] =(u16) data;
				uPosCommandBuffer++;
				uNumBytesWrite-=2;
				if (uNumBytesWrite==0)
				{
					spi_set_md = false;
					uPosCommandBuffer = 0;
					uNumBytesWrite = 0;

					GDStatus.DRQ=GDStatus.BSY=0;
					GDStatus.DRDY=1;
					IntReason.CoD=IntReason.IO=1;
					RaiseInterrupt(holly_GDROM_CMD);
				}

				return;
			}

			if( spi_buff.top < 5 )	// should never be > 6-1
				//SpiCmdBuf[ SpiCmdPtr++ ] = (u16)data;
				spi_buff.push((u16)data);
			else
			{
				spi_buff.push((u16)data);
				
				if (PendSpicmd)
					printf("Second SPI packet .. errrooorr\n");

				PendSpicmd=&t_cmdmem;

				for (int i=5;i>=0;i--)
				{
					PendSpicmd->CommandData_16[i]=spi_buff.pop();
				}

				//spi_buff->clear();

				// this doesn't have to be here, since this isn't an asyncrhonous atm
				GDStatus.DRQ=0;// &= ~GDSTATUS_DRQ;	// Clear DRQ
				GDStatus.BSY=1;// |= GDSTATUS_BSY;	// Set BSY

					// this should be done in the cmd, some wont need to set DRQ (needs data removal)
		//		GDStatus &= ~GDSTATUS_BSY;	// Clear BSY
		//		GDStatus |= GDSTATUS_DRQ;	// Set DRQ

				// goto _gd_int; // or else just add one here and return
				UpdateGDRom();
			}
			return;
		}
	case GD_DEVCTRL:
		printf("\nGDROM:\tWrite GD_DEVCTRL ( Not implemented on dc)\n");
		break;

	case GD_DRVSEL: 
		printf_db("\nGDROM:\tWrite to GD_DRVSEL\n");
			DriveSel = data; 
		break;


	//	By writing "3" as Feature Number and issuing the Set Feature command,
	//	the PIO or DMA transfer mode set in the Sector Count register can be selected.
	//	The actual transfer mode is specified by the Sector Counter Register. 

	case GD_FEATURES:
		printf_db("\nGDROM:\tWrite to GD_FEATURES\n");
			Features.full =(u8) data;
			// this  done  by  setfeature cmd ?
			// if data != 1
		//	if( data & 0x80 )
		//		SetFeature(data&0x7F);
		//	else
		//		ClrFeature(data&0x7F);
		break;

	case GD_SECTCNT	:	// *FIXME*
			printf("\nGDROM:\tWrite to SecCount <= %X !\n", data);
			SecCount.full =(u8) data;
		break;

	case GD_SECTNUM	:
			printf("\nGDROM:\tWrite to SecNum; not possible <= %X !\n", data);
		break;

	case GD_COMMAND:
			printf_db("\nGDROM:\tCOMMAND: %X !\n", data);

			if( (GDStatus.BSY ) && (data != GDC_SFT_RESET) ) {
				printf("\nGDROM:\tRecieved a Command While Device BSY=1\n");
				break;
			}

			switch (data)
			{

				case GDC_SPI_PACKET:
					// Set CoD | Clr IO
					IntReason.CoD=1;// |= IRES_COD;
					IntReason.IO=0;// &= ~IRES_IO;

					// Set DRQ | Clr BSY | = 0x01 works too
					GDStatus.DRQ=1;// |= GDSTATUS_DRQ;
					GDStatus.BSY=0;// &= ~(GDSTATUS_BSY|GDSTATUS_DRDY);	// Clear DRDY ?: no
					GDStatus.DRDY=1;
					//return;	// dont do IRQD here
					break;

				// all of these perform interrupts *FIXME* reset just resets all regs 
				case  GDC_NOP	:
					printf("\nGDROM:\tNOP!\n");
					//Abort command...
					GDStatus.DRQ=0;
					GDStatus.BSY=0;

					gd_dma_pending=false;
					dma_data=false;
					DataIndex=0;
					DataLength=0;
					DmaSubCount=0;

					RaiseInterrupt(InterruptID::holly_GDROM_CMD);
					break;

				case  GDC_SFT_RESET:
					printf("\nGDROM:\tSoft Reset !\n"); 
					break;

				case  GDC_EXEC_DIAG:
					printf("\nGDROM:\tExec Diagnostics !\n"); Error.full=1; 
					break;

				case  GDC_IDENTIFY_DEV:
					printf("\nGDROM:\tID Device !\n"); 
					break;

				case  GDC_SET_FEATURES:
					printf("\nGDROM:\tSet Features !\n");
					break;

				default:
					printf("\nGDROM:\tUnhandled GDROM Command %X !\n",data);
					break;
			}
			break;

		default:
		printf("\nGDROM:\tUnhandled Write to %X <= %X sz:%X\n",Addr,data,sz);
		break;
	}
}

//
//UpdateGDRom -> if commands needs to be proccesed , it is done here :)

void UpdateGDRom()
{


	SpiCommandInfo* PendSpicmd2=PendSpicmd;
	PendSpicmd=0;
	if (PendSpicmd2)
	{
		//proccess spi :)
		if (ProcessSPI(PendSpicmd2))
		{
			//ok , done w/ it
			PendSpicmd2=0;
			return;//no dma
		}
	}

	//pending dma + data to send
	if (gd_dma_pending && dma_data)
	{
		gd_DoDMA();
		return;//do spi next time
	}
}

//
//Proccesses SPI Commands ;)
bool ProcessSPI(SpiCommandInfo* cmd)
{
	printf_db("SPI cmd %02x;",cmd->CommandCode);
	printf_db("params: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
					cmd->CommandData[0], cmd->CommandData[1], cmd->CommandData[2], cmd->CommandData[3], cmd->CommandData[4], cmd->CommandData[5],
					cmd->CommandData[6], cmd->CommandData[7], cmd->CommandData[8], cmd->CommandData[9], cmd->CommandData[10], cmd->CommandData[11] );

	switch(cmd->CommandCode)
	{
		case SPI_TEST_UNIT	:
			GDStatus.DRDY=1;
			GDStatus.BSY=GDStatus.DRQ=0;
			IntReason.CoD=IntReason.IO=0;
			RaiseInterrupt(holly_GDROM_CMD);
		break;

		case SPI_REQ_MODE:
			GDStatus.DRQ=1;
			GDStatus.DRDY=GDStatus.BSY=0;

			printf_db("\nGDROM:\tREQ_MODE: Addr:%X  Len:%i\n", cmd->CommandData[2], cmd->CommandData[4]);
		

			IntReason.IO=1;
			IntReason.CoD=0;
			RaiseInterrupt(holly_GDROM_CMD);
			SendBytesToHost(cmd->CommandData[4],(u8*)&g_aValues0x11[cmd->CommandData[2]>>1]);
		break;

		/////////////////////////////////////////////////
		// *FIXME* CHECK FOR DMA, Diff Settings !?!@$#!@%
		case SPI_CD_READ:
		{
			GDReadBlock_t* pGDr;
			u32 adr=0,len=0;

			if (Features.DMA==1)//if dma
			{
				if (DataLength!=0 || DataIndex !=0)
					printf(">>\tgdrom: TEH BROKE DataLength!=0 || DataIndex !=0 on dma zomg this is fuqed dl:%d di:%d\n",DataLength,DataIndex);

				DataLength=DataIndex=0;
				//SSet=GDSTATUS_BSY;
				GDStatus.BSY=1;
				//SClr=GDSTATUS_DRDY | GDSTATUS_DRQ;	// DRDY should prob be cleared ? *FIXME*
				GDStatus.DRDY=GDStatus.DRQ=0;

				GDStatus.DRDY=1;//is that ok ?
			}

			pGDr = (GDReadBlock_t*)cmd->CommandData;

			if( pGDr->head || pGDr->subh || pGDr->other || (!pGDr->data) )	// assert
				printf("\n!GDROM: *FIXME* ADD MORE CD READ SETTINGS \n\n");

			printf("GDROM:\tSPI_CD_READ: <%d-%d-%d-%d:%X::%s> - Start Addr <%s>(%X %X %X) XFer Len (%X %X %X) \n",
				pGDr->head, pGDr->subh, pGDr->data, pGDr->other, (pGDr->b[1] >> 1 & 7), szExDT[pGDr->b[1] >> 1 & 7],
				(pGDr->b[1]&1)?"MSF":"FAD", pGDr->b[2], pGDr->b[3], pGDr->b[4],	pGDr->b[8], pGDr->b[9], pGDr->b[10]  );

			/////////////////////// 
			if( pGDr->prmtype )	// ( SpiCmdBuf_u8[0]&1 )
				printf("\n!GDROM:\tMSF FORMAT READ!\n");
			else
				adr = (pGDr->b[2]<<16) | (pGDr->b[3]<<8) | (pGDr->b[4]);

			len = (pGDr->b[8]<<16) | (pGDr->b[9]<<8) | (pGDr->b[10]);

			drive->ReadSector(DataBuffer, adr, len,2048);

			if (Features.DMA==1)
			{
				// *FIXME*  if dma ...
				DataLength=len*2048;//FIXME sector size
				//Reason |= IRES_IO;
				IntReason.IO=1;
				//Reason &= ~IRES_COD;
				IntReason.CoD=0;
				GDStatus.DRDY=1;
				RaiseInterrupt(holly_GDROM_CMD);
				dma_data=true;//we do have data to dma ;)
				UpdateGDRom();
			}
			else
			{//pio
				//SSet=GDSTATUS_DRQ;
				GDStatus.DRQ=1;
				//SClr=GDSTATUS_DRDY| GDSTATUS_BSY;	// DRDY should prob be cleared ? *FIXME*
				GDStatus.DRDY=GDStatus.BSY=0;

				//Reason |= IRES_IO;
				IntReason.IO=1;
				//Reason &= ~IRES_COD;
				IntReason.CoD=0;

				RaiseInterrupt(holly_GDROM_CMD);
				SendBytesToHost(len*2048,(u8*)&DataBuffer[0]);
				printf_db("PIO \n");
			}
			// *FIXME*  else (pio)
		}
		break;

		case SPI_GET_TOC:
			{
				u32 toc_gd[102];

				if (cmd->CommandData[1]&0x1)
				{	//toc - dd
					drive->GetToc(&toc_gd[0],DoubleDensity);
				}
				else
				{	//toc - sd
					drive->GetToc(&toc_gd[0],SingleDensity);
				}

				//SSet=GDSTATUS_DRQ;
				GDStatus.DRQ=1;
				//SClr=GDSTATUS_DRDY| GDSTATUS_BSY;	// DRDY should prob be cleared ? *FIXME*
				GDStatus.DRDY=GDStatus.BSY=0;

				//GDStatus.DRDY=1;//is that ok ?
				//Reason |= IRES_IO;
				IntReason.IO=1;
				//Reason &= ~IRES_COD;
				IntReason.CoD=0;
				RaiseInterrupt(holly_GDROM_CMD);

				SendBytesToHost(sizeof(toc_gd),(u8*)&toc_gd[0]);

			}
			break;

		case 0x70:
			GDStatus.full=0x50; //FIXME
			RaiseInterrupt(holly_GDROM_CMD);
			break;
		case 0x71:
			{
				u32 uCount;
				static u32 iAux = 0;
				uCount  =ByteCount.full;

				if (iAux)
				{      
					//uCount = sizeof(g_aValues0x71);
					SendBytesToHost(uCount,(u8*)&g_aValues0x71[0]);
				}
				else
				{
					//uCount = sizeof(g_aValues0x71_b);
					SendBytesToHost(uCount,(u8*)&g_aValues0x71_b[0]);
				}
				iAux^=1;

				//SSet=GDSTATUS_DRQ;
				GDStatus.DRQ=1;
				//SClr=GDSTATUS_DRDY| GDSTATUS_BSY;	// DRDY should prob be cleared ? *FIXME*
				GDStatus.DRDY=GDStatus.BSY=0;

				//GDStatus.DRDY=1;//is that ok ?
				//Reason |= IRES_IO;
				IntReason.IO=1;
				//Reason &= ~IRES_COD;
				IntReason.CoD=0;
				RaiseInterrupt(holly_GDROM_CMD);
			}
			break;
		case SPI_SET_MODE:
			{
				spi_set_md=true;
				//u32 uOffset = cmd->CommandData[2];
				u32 uCount = cmd->CommandData[4];
				ByteCount.full=(u16) uCount;
				uNumBytesWrite = uCount;
				uPosCommandBuffer = 0;

				//Reason &= ~IRES_COD;
				//Reason &= ~IRES_IO;
				IntReason.CoD=IntReason.IO=0;
				

				//SSet=GDSTATUS_DRQ;	// this correct here ? *WATCH*
				//SClr=GDSTATUS_BSY;
				GDStatus.DRDY=1;//is that ok ? -> yes
				GDStatus.DRQ=1;
				GDStatus.BSY=0;

				RaiseInterrupt(holly_GDROM_CMD);		
			}

			break;

		case SPI_CD_READ2:
			printf("\nGDROM:\tUnhandled Sega SPI frame: SPI_CD_READ2\n");
			goto UnhSPI;
		break;

		case SPI_REQ_ERROR:
			printf("\nGDROM:\tUnhandled Sega SPI frame: SPI_REQ_ERROR\n");
			goto UnhSPI;
		break;

		case SPI_REQ_SES:
			printf("\nGDROM:\tUnhandled Sega SPI frame: SPI_REQ_SES\n");


			printf("Session Info for Session %d,%d bytes\n",cmd->CommandData[2],cmd->CommandData[4]);
			
			u8 ses_inf[6];
			libGDR->gdr_info.GetSessionInfo(ses_inf,cmd->CommandData[2]);

			//send requested data
			SendBytesToHost(cmd->CommandData[4],(u8*)&ses_inf[0]);
			//SSet=GDSTATUS_DRQ;
			GDStatus.DRQ=1;
			//SClr=GDSTATUS_DRDY| GDSTATUS_BSY;	// DRDY should prob be cleared ? *FIXME*
			GDStatus.BSY=0;

			//GDStatus.DRDY=1;//is that ok ?-> no
			GDStatus.DRDY=0;
			//Reason |= IRES_IO;
			IntReason.IO=1;
			//Reason &= ~IRES_COD;
			IntReason.CoD=0;
			RaiseInterrupt(holly_GDROM_CMD);
		break;

		case SPI_CD_OPEN:
			printf("\nGDROM:\tUnhandled Sega SPI frame: SPI_CD_OPEN\n");
			goto UnhSPI;
		break;

		case SPI_CD_PLAY:
			printf("\nGDROM:\tUnhandled Sega SPI frame: SPI_CD_PLAY\n");
			goto UnhSPI;
		break;

		case SPI_CD_SEEK:
			printf("\nGDROM:\tUnhandled Sega SPI frame: SPI_CD_SEEK\n");
			goto UnhSPI;
		break;

		case SPI_CD_SCAN:
			printf("\nGDROM:\tUnhandled Sega SPI frame: SPI_CD_SCAN\n");
			goto UnhSPI;
		break;

		case SPI_GET_SCD:
			printf("\nGDROM:\tUnhandled Sega SPI frame: SPI_GET_SCD\n");
			
			u32 format;
			format=cmd->CommandData[1]&0xF;
			u32 sz;
			char subc_info[96];

			if (format==0)
				sz=96;
			else
				sz=12;

			SendBytesToHost(sz,(u8*)&subc_info[0]);

			printf("Subcode format %d,%d bytes\n",format,sz);
			
			//SSet=GDSTATUS_DRQ;
			GDStatus.DRQ=1;
			//SClr=GDSTATUS_DRDY| GDSTATUS_BSY;	// DRDY should prob be cleared ? *FIXME*
			GDStatus.DRDY=GDStatus.BSY=0;

			//Reason |= IRES_IO;
			IntReason.IO=1;
			//Reason &= ~IRES_COD;
			IntReason.CoD=0;
			RaiseInterrupt(holly_GDROM_CMD);
		break;

		default:
UnhSPI:
			printf("\nGDROM:\tUnhandled Sega SPI frame: %X\n", cmd->CommandCode);
			//SSet=GDSTATUS_DRDY;	// this correct here ? *WATCH*
			GDStatus.DRDY=1;
			//SClr=GDSTATUS_DRQ | GDSTATUS_BSY;
			GDStatus.DRQ=GDStatus.BSY=0;
			RaiseInterrupt(holly_GDROM_CMD);
		break;
	}

	return true;//-> do not retry it later ;P :P
}
//
//Initialise Dma :)
void GDROM_DmaSt_Write(u32 data)
{
	if((1==(data&0x1)) && (SB_GDEN &1))
	{
		gd_dma_pending=true;
		if (!dma_data)
			printf(">>\tGDROM : DMA initiated\n");
		SB_GDST=1;
	}
	else
	{
		/*if (dma_data)
			dma_data=false;//?*/
	}

	UpdateGDRom();

	if (dma_data && gd_dma_pending)
		gd_DoDMA();

}
//
//Do DMA (and end it :0)
#define DMAOR_MASK	0xFFFF8201
//void LoadSyscallHooks(void);
void gd_DoDMA()
{

	//TODO : Fix dmaor
	u32 dmaor	= DMAC_DMAOR;

	u32	src		= SB_GDSTAR,
		len		= SB_GDLEN ;

		// do we need to do this for gdrom dma ?
	if(0x8201 != (dmaor &DMAOR_MASK)) {
		printf("\n!\tGDROM: DMAOR has invalid settings (%X) !\n", dmaor);
		//return;
	}
	if( len & 0x1F ) {
		printf("\n!\tGDROM: SB_GDLEN has invalid size (%X) !\n", len);
		return;
	}

	//notify rec that we change data
//	rec_InvalidateBlocks(src,src+len-1);
    // check if its going to system ram

	

	if( 1 == SB_GDDIR ) {
		memcpy( &mem_b[src&0xFFFFFF], &DataBuffer[DataIndex], len );
		
		if (len>=8*1024*1024)
			printf("ERROR , TRANSFER LENGHT IS TOO MUCH \n");

		printf_db("\n>>\tGDROM: DMA ADDR:%X LEN:%X \n", SB_GDSTAR, SB_GDLEN);
	}
	else
		printf("\n!\tGDROM: SB_GDDIR %X (TO AICA WAVE MEM?) !\n", SB_GDDIR);

	//rehook if needed ;)
	//TODO : Re-add hooks if needed
	LoadSyscallHooks();
	// Setup some of the regs so it thinks we've finished DMA

	SB_GDLEN = 0x00000000;
	SB_GDLEND=len;
	SB_GDSTAR = (src + len);
	SB_GDST=0;//done

	// The DMA end interrupt flag (SB_ISTNRM - bit 19: DTDE2INT) is set to "1."
	RaiseInterrupt(holly_GDROM_DMA);


	DataIndex+=len;
	DmaSubCount++;
	if (DataIndex==DataLength)
	{
		dma_data=false;
		printf(">>\tData Send finished\n",DataLength-DataIndex +1);
		DataLength=DataIndex=0;//fix em out
		DmaSubCount=0;

		// this really shouldn't be here
	
		IntReason.IO=0;//clear io (?)
		GDStatus.DRDY=1;// |= GDSTATUS_DRDY;
		GDStatus.BSY=GDStatus.DRQ=0;// &= ~(GDSTATUS_BSY | GDSTATUS_DRQ);
	}
	else if (DataIndex<DataLength)
	{
		//GDStatus.DRDY=1;//can accept ATA command
		printf(">>\tRemaining %d bytes to dma\n",DataLength-DataIndex );
		if ((DataLength-DataIndex )==1920)
		{
			printf("last transfer\n");
			//IntReason.IO=0;//clear io (?)
			//GDStatus.DRDY=1;// |= GDSTATUS_DRDY;
			//GDStatus.BSY=GDStatus.DRQ=0;// &= ~(GDSTATUS_BSY | GDSTATUS_DRQ);
		}
	}
	else 
		printf(">>\tDataIndex> DataLength  WTF THIS IS A BUG MAN IM REATARDED [%d,%d]\n",DataLength,DataIndex );



	gd_dma_pending=false;
}

void gdrom_reg_Init()
{
	InitGDROM();
	sb_regs[(SB_GDST_addr-SB_BASE)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[(SB_GDST_addr-SB_BASE)>>2].writeFunction=GDROM_DmaSt_Write;
}
void gdrom_reg_Term()
{
	TermGDROM();
}

void gdrom_reg_Reset(bool Manual)
{
	//huh?
}
#endif