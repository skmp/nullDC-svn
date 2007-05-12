﻿#include "gdromv3.h"

#include "plugins\plugin_manager.h"
#include "dc/mem/sh4_mem.h"
#include "dc/mem/memutil.h"
#include "dc/mem/sb.h"
#include "dc/sh4/dmac.h"
#include "dc/sh4/intc.h"
#include "dc/sh4/sh4_registers.h"


enum gd_states
{
	//Generic
	gds_waitcmd,
	gds_procata,
	gds_waitpacket,
	gds_procpacket,
	gds_pio_send_data,
	gds_pio_get_data,
	gds_pio_end,
	gds_procpacketdone,

	//Command spec.
	gds_readsector_pio,
	gds_readsector_dma,
	gds_process_set_mode,
};

struct 
{
	u32 start_sector;
	u32 remaining_sectors;
	u32 sector_type;
} read_params;

struct 
{
	u32 index;
	union
	{
		u16 data_16[6];
		u8 data_8[12];
		//Spi command structs
		union 
		{
			struct 
			{
				u8 cc;

				u8 prmtype	: 1 ;	
				u8 expdtype	: 3 ;
				//	u8 datasel	: 4 ;
				u8 other	: 1 ;
				u8 data		: 1 ;
				u8 subh		: 1 ;
				u8 head		: 1 ;

				u8 block[10];
			};

			struct 
			{
				u8 b[12];
			};
		}GDReadBlock;
	};
} packet_cmd;

//Buffer for sector reads [ dma ]
struct 
{
	u32 cache_index;
	u32 cache_size;
	u8 cache[2352*32];	//up to 32 sectors
} read_buff;

//pio buffer
struct 
{
	gd_states next_state;
	u32 index;
	u32 size;
	u16 data[0x10000>>1];	//64 kb
} pio_buff;

u32 set_mode_offset;
struct 
{
	u8 command;
} ata_cmd;

struct 
{
	bool playing;
	u32 repeats;
	union
	{
		u32 FAD;
		struct
		{
			u8 B0;		//msb
			u8 B1;		//midle byte
			u8 B2;		//lsb
		};
	}CurrAddr,EndAddr,StartAddr;
} cdda;


gd_states gd_state;
DiscType gd_disk_type;
/*
	GD rom reset -> GDS_WAITCMD

	GDS_WAITCMD -> ATA/SPI command [Command code is on ata_cmd]
	SPI Command -> GDS_WAITPACKET -> GDS_SPI_* , depending on input

	GDS_SPI_READSECTOR -> Depending on features , it can do quite a few things
*/
u32 data_write_mode=0;

//Registers
	u32 DriveSel;
	GD_ErrRegT Error;
	GD_InterruptReasonT IntReason;
	GD_FeaturesT Features;
	GD_SecCountT SecCount;
	GD_SecNumbT SecNumber;
	
	GD_StatusT GDStatus;

	union
	{
		struct
		{
		u8 low;
		u8 hi;
		};
		u16 full;
	} ByteCount;
	
//end

void nilprintf(...){}

#define printf_rm nilprintf
#define printf_ata nilprintf
#define printf_spi nilprintf
#define printf_spicmd nilprintf

void FASTCALL gdrom_get_cdda(s16* sector)
{
	//silence ! :p
	if (cdda.playing)
	{
		libGDR.ReadSector((u8*)sector,cdda.CurrAddr.FAD,1,2352);
		cdda.CurrAddr.FAD++;
		if (cdda.CurrAddr.FAD==cdda.EndAddr.FAD)
		{
			if (cdda.repeats==0)
			{
				//stop
				cdda.playing=false;
				SecNumber.Status=GD_STANDBY;
			}
			else
			{
				//Repeat ;)
				if (cdda.repeats!=0xf)
					cdda.repeats--;
				cdda.CurrAddr.FAD=cdda.StartAddr.FAD;
			}
		}
	}
	else
	{
		memset(sector,0,2352);
	}
}
void gd_spi_pio_end(u8* buffer,u32 len,gd_states next_state=gds_pio_end);
void gd_process_spi_cmd();
void gd_process_ata_cmd();
void FillReadBuffer()
{
	read_buff.cache_index=0;
	u32 count = read_params.remaining_sectors;
	if (count>32)
		count=32;

	read_buff.cache_size=count*read_params.sector_type;

	libGDR.ReadSector(read_buff.cache,read_params.start_sector,count,read_params.sector_type);
	read_params.start_sector+=count;
	read_params.remaining_sectors-=count;
}

void gd_set_state(gd_states state)
{
	gd_states prev=gd_state;
	gd_state=state;
	switch(state)
	{
		case gds_waitcmd:
			GDStatus.DRDY=1;	//Can accept ata cmd :)
			GDStatus.BSY=0;		//Does not access command block
			break;

		case gds_procata:
			//verify(prev==gds_waitcmd);	//validate the previus cmd ;)

			GDStatus.DRDY=0;	//can't accept ata cmd
			GDStatus.BSY=1;		//accessing command block to process cmd
			gd_process_ata_cmd();
			break;

		case gds_waitpacket:
			verify(prev==gds_procata);	//validate the previus cmd ;)

			//prepare for packet cmd
			packet_cmd.index=0;

			//Set CoD, clear BSY and IO
			IntReason.CoD=1;
			GDStatus.BSY = 0;
			IntReason.IO=0;

			//Make DRQ valid
			GDStatus.DRQ = 1;

			//ATA can optionaly raise the interrupt ...
			//RaiseInterrupt(holly_GDROM_CMD);
			break;

		case gds_procpacket:
			verify(prev==gds_waitpacket);	//validate the previus state ;)

			GDStatus.DRQ=0;		//can't accept ata cmd
			GDStatus.BSY=1;		//accessing command block to process cmd
			gd_process_spi_cmd();
			break;
			//yep , get/set are the same !
		case gds_pio_get_data:
		case gds_pio_send_data:
			//	When preparations are complete, the following steps are carried out at the device.
			//(1)	Number of bytes to be read is set in "Byte Count" register. 
			ByteCount.full =(u16)(pio_buff.size<<1);
			//(2)	IO bit is set and CoD bit is cleared. 
			IntReason.IO=1;
			IntReason.CoD=0;
			//(3)	DRQ bit is set, BSY bit is cleared. 
			GDStatus.DRQ=1;
			GDStatus.BSY=0;
			//(4)	INTRQ is set, and a host interrupt is issued.
			RaiseInterrupt(holly_GDROM_CMD);
			/*
			The number of bytes normally is the byte number in the register at the time of receiving 
			the command, but it may also be the total of several devices handled by the buffer at that point.
			*/
			break;

		case gds_readsector_pio:
			{
				/*
				If more data are to be sent, the device sets the BSY bit and repeats the above sequence 
				from step 7. 
				*/
				GDStatus.BSY=1;

				u32 sector_count = read_params.remaining_sectors;
				gd_states next_state=gds_pio_end;

				if (sector_count>27)
				{
					sector_count=27;
					next_state=gds_readsector_pio;
				}
				libGDR.ReadSector((u8*)&pio_buff.data[0],read_params.start_sector,sector_count,
											read_params.sector_type);
				read_params.start_sector+=sector_count;
				read_params.remaining_sectors-=sector_count;

				gd_spi_pio_end(0,sector_count*read_params.sector_type,next_state);
			}
			break;
			
		case gds_readsector_dma:
			FillReadBuffer();
			break;

		case gds_pio_end:
			
			GDStatus.DRQ=0;//all data is sent !

			gd_set_state(gds_procpacketdone);
			break;

		case gds_procpacketdone:
			/*
			7.	When the device is ready to send the status, it writes the 
			final status (IO, CoD, DRDY set, BSY, DRQ cleared) to the "Status" register before making INTRQ valid. 
			After checking INTRQ, the host reads the "Status" register to check the completion status. 
			*/
			//Set IO, CoD, DRDY
			GDStatus.DRDY=1;
			IntReason.CoD=1;
			IntReason.IO=1;

			//Clear DRQ,BSY
			GDStatus.DRQ=0;
			GDStatus.BSY=0;
			//Make INTRQ valid
			RaiseInterrupt(holly_GDROM_CMD);

			//command finished !
			gd_set_state(gds_waitcmd);
			break;

		case gds_process_set_mode:
			memcpy(&g_aValues0x11[set_mode_offset],pio_buff.data ,pio_buff.size<<1);
			//end pio transfer ;)
			gd_set_state(gds_pio_end);
			break;
		default :
			die("Unhandled gdrom state ...");
			break;
	}
}


void gd_setdisc()
{
	gd_disk_type = (DiscType)libGDR.GetDiscType();
	
	switch(gd_disk_type)
	{
	case NoDisk:
		SecNumber.Status = GD_NODISC;
		//GDStatus.BSY=0;
		//GDStatus.DRDY=1;
		break;
	case Open:
		SecNumber.Status = GD_OPEN;
		//GDStatus.BSY=0;
		//GDStatus.DRDY=1;
		break;
	case Busy:
		SecNumber.Status = GD_BUSY;
		//GDStatus.BSY=1;
		//GDStatus.DRDY=0;
		break;
	default :
		if (SecNumber.Status==GD_BUSY)
			SecNumber.Status = GD_PAUSE;
		else
			SecNumber.Status = GD_STANDBY;
		//GDStatus.BSY=0;
		//GDStatus.DRDY=1;
		break;
	}

	SecNumber.DiscFormat=gd_disk_type>>4;
}
void gd_reset()
{
	//Reset the drive
	gd_setdisc();
	gd_set_state(gds_waitcmd);
}
u32 GetFAD(u8* data,bool msf)
{
	if( msf )	
	{
		printf("GDROM: MSF FORMAT\n");
		return ((data[0]*60*75) + (data[1]*75) | (data[2]));
	}
	else
	{
		return (data[0]<<16) | (data[1]<<8) | (data[2]);
	}
}
//disk changes ect
void FASTCALL NotifyEvent_gdrom(u32 info,void* param)
{
	if (info == DiskChange)
		gd_setdisc();
}

//This handles the work of setting up the pio regs/state :)
void gd_spi_pio_end(u8* buffer,u32 len,gd_states next_state)
{
	verify(len<0xFFFF);
	pio_buff.index=0;
	pio_buff.size=len>>1;
	pio_buff.next_state=next_state;
	if (buffer!=0)
		memcpy(pio_buff.data,buffer,len);
	if (len==0)
		gd_set_state(next_state);
	else
		gd_set_state(gds_pio_send_data);
}
void gd_spi_pio_read_end(u32 len,gd_states next_state)
{
	verify(len<0xFFFF);
	pio_buff.index=0;
	pio_buff.size=len>>1;
	pio_buff.next_state=next_state;
	if (len==0)
		gd_set_state(next_state);
	else
		gd_set_state(gds_pio_get_data);
}
void gd_process_ata_cmd()
{
	//Any ata cmd clears these bits , unless aborted/error :p
	Error.ABRT=0;
	GDStatus.CHECK=0;

	switch(ata_cmd.command)
	{
	case ATA_NOP:
		printf_ata("ATA_NOP\n");
		/*
			Setting "abort" in the error register 
			Setting an error in the status register 
			Clearing "busy" in the status register 
			Asserting the INTRQ signal
		*/
		
		Error.ABRT=1;
		//Error.Sense=0x00; //fixme ?
		GDStatus.BSY=0;
		GDStatus.CHECK=1;

		RaiseInterrupt(holly_GDROM_CMD);
		gd_set_state(gds_waitcmd);
		break;

	case ATA_SOFT_RESET:
		{
			printf_ata("ATA_SOFT_RESET\n");
			//DRV -> preserved -> wtf is it anyway ?
			gd_reset();
		}
		break;

	case ATA_EXEC_DIAG:
		printf_ata("ATA_EXEC_DIAG\n");
		printf("ATA_EXEC_DIAG -- not implemented\n");
		break;

	case ATA_SPI_PACKET:
		printf_ata("ATA_SPI_PACKET\n");
		gd_set_state(gds_waitpacket);
		break;

	case ATA_IDENTIFY_DEV:
		printf_ata("ATA_IDENTIFY_DEV\n");
		printf("ATA_IDENTIFY_DEV -- not implemented\n");
		break;

	case ATA_SET_FEATURES:
		printf_ata("ATA_SET_FEATURES\n");

		//Set features sets :
		//Error : ABRT
		Error.ABRT=0;	//command was not aborted ;) [hopefully ...]

		//status : DRDY , DSC , DF , CHECK
		//DRDY is set on state change
		GDStatus.DSC=0;
		GDStatus.DF=0;
		GDStatus.CHECK=0;
		RaiseInterrupt(holly_GDROM_CMD);  //???
		gd_set_state(gds_waitcmd);
		break;

	default:
		die("Unkown ATA command..");
		break;
	};
}

void gd_process_spi_cmd()
{
	printf_spi("SPI cmd %02x;",packet_cmd.data_8[0]);
	printf_spi("params: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
		packet_cmd.data_8[0], packet_cmd.data_8[1], packet_cmd.data_8[2], packet_cmd.data_8[3], packet_cmd.data_8[4], packet_cmd.data_8[5],
		packet_cmd.data_8[6], packet_cmd.data_8[7], packet_cmd.data_8[8], packet_cmd.data_8[9], packet_cmd.data_8[10], packet_cmd.data_8[11] );

	GDStatus.CHECK=0;

	switch(packet_cmd.data_8[0])
	{
	case SPI_TEST_UNIT	:
		printf_spicmd("SPI_TEST_UNIT\n");

		GDStatus.CHECK=SecNumber.Status==GD_BUSY;	//drive is ready ;)

		gd_set_state(gds_procpacketdone);
		break;

	case SPI_REQ_MODE:
		printf_spicmd("SPI_REQ_MODE\n");
		gd_spi_pio_end((u8*)&g_aValues0x11[packet_cmd.data_8[2]>>1],packet_cmd.data_8[4]);
		break;

		/////////////////////////////////////////////////
		// *FIXME* CHECK FOR DMA, Diff Settings !?!@$#!@%
	case SPI_CD_READ:
		{
			printf_spicmd("SPI_CD_READ\n");

#define readcmd packet_cmd.GDReadBlock

			if( readcmd.head ||readcmd.subh || readcmd.other || (!readcmd.data) )	// assert
				printf("GDROM: *FIXME* ADD MORE CD READ SETTINGS\n");
			u32 sector_type=2048;

			u32 start_sector = GetFAD(&readcmd.b[2],readcmd.prmtype);

			u32 sector_count = (readcmd.b[8]<<16) | (readcmd.b[9]<<8) | (readcmd.b[10]);

			read_params.start_sector=start_sector;
			read_params.remaining_sectors=sector_count;
			read_params.sector_type = sector_type;//yeah i know , not realy many types supported ...

			if (Features.CDRead.DMA==1)
			{
				gd_set_state(gds_readsector_dma);
			}
			else
			{
				gd_set_state(gds_readsector_pio);
			}
		}
		break;

	case SPI_GET_TOC:
		{
			printf_spicmd("SPI_GET_TOC\n");
			//printf("SPI_GET_TOC - %d\n",(packet_cmd.data_8[4]) | (packet_cmd.data_8[3]<<8) );
			u32 toc_gd[102];
			
			//toc - dd/sd
			libGDR.GetToc(&toc_gd[0],packet_cmd.data_8[1]&0x1);
			 
			gd_spi_pio_end((u8*)&toc_gd[0], (packet_cmd.data_8[4]) | (packet_cmd.data_8[3]<<8) );
		}
		break;

		//mount/map drive ? some kind of reset/unlock ??
		//seems like a non data command :)
	case 0x70:
		printf_spicmd("SPI : unkown ? [0x70]\n");
		/*GDStatus.full=0x50; //FIXME
		RaiseInterrupt(holly_GDROM_CMD);*/

		gd_set_state(gds_procpacketdone);
		break;
	case 0x71:
		{
			printf_spicmd("SPI : unkown ? [0x71]\n");

			
			u32 uCount;
			static u32 iAux = 0;
			uCount  =ByteCount.full;

			if (iAux)
			{      
				gd_spi_pio_end((u8*)&g_aValues0x71[0],uCount);
			}
			else
			{
				gd_spi_pio_end((u8*)&g_aValues0x71_b[0],uCount);
			}
			iAux^=1;
		}
		break;
	case SPI_SET_MODE:
		{
			printf_spicmd("SPI_SET_MODE\n");
			u32 Offset = packet_cmd.data_8[2];
			u32 Count = packet_cmd.data_8[4];
			verify((Offset+Count)<11);	//cant set write olny things :P
			set_mode_offset=Offset;
			gd_spi_pio_read_end(Count,gds_process_set_mode);
		}

		break;

	case SPI_CD_READ2:
		printf_spicmd("SPI_CD_READ2\n");
		printf("GDROM: Unhandled Sega SPI frame: SPI_CD_READ2\n");

		gd_set_state(gds_procpacketdone);
		break;

		
	case SPI_REQ_STAT:
		{
			printf_spicmd("SPI_REQ_STAT\n");
			printf("GDROM: Unhandled Sega SPI frame: SPI_REQ_STAT\n");
			u8 stat[10];

			//0	0	0	0	0	STATUS
			stat[0]=SecNumber.Status;	//low nibble 
			//1	Disc Format	Repeat Count
			stat[1]=(u8)(SecNumber.DiscFormat<<4) | (cdda.repeats);
			//2	Address	Control
			stat[2]=0x4;
			//3	TNO
			stat[3]=2;
			//4	X
			stat[4]=0;
			//5	FAD
			stat[5]=cdda.CurrAddr.B0;
			//6	FAD
			stat[6]=cdda.CurrAddr.B1;
			//7	FAD
			stat[7]=cdda.CurrAddr.B2;
			//8	Max Read Error Retry Times
			stat[8]=0;
			//9	0	0	0	0	0	0	0	0
			stat[9]=0;

			
			verify((packet_cmd.data_8[2]+packet_cmd.data_8[4])<11);
			gd_spi_pio_end(&stat[packet_cmd.data_8[2]],packet_cmd.data_8[4]);
		}
		break;

	case SPI_REQ_ERROR:
		printf_spicmd("SPI_REQ_ERROR\n");
		printf("GDROM: Unhandled Sega SPI frame: SPI_REQ_ERROR\n");
		
		u8 resp[10];
		resp[0]=0xF0;
		resp[1]=0;
		resp[2]= SecNumber.Status==GD_BUSY ? 2:0;//sense
		resp[3]=0;
		resp[4]=resp[5]=resp[6]=resp[7]=0; //Command Specific Information
		resp[8]=0;//Additional Sense Code
		resp[9]=0;//Additional Sense Code Qualifier

		gd_spi_pio_end(resp,packet_cmd.data_8[4]);
		break;

	case SPI_REQ_SES:
		printf_spicmd("SPI_REQ_SES\n");

		u8 ses_inf[6];
		libGDR.GetSessionInfo(ses_inf,packet_cmd.data_8[2]);
		ses_inf[0]=SecNumber.Status;
		gd_spi_pio_end((u8*)&ses_inf[0],packet_cmd.data_8[4]);
		break;

	case SPI_CD_OPEN:
		printf_spicmd("SPI_CD_OPEN\n");
		printf("GDROM: Unhandled Sega SPI frame: SPI_CD_OPEN\n");
		
		
		gd_set_state(gds_procpacketdone);
		break;

	case SPI_CD_PLAY:
		{
			printf_spicmd("SPI_CD_PLAY\n");
			printf("GDROM: Unhandled Sega SPI frame: SPI_CD_PLAY\n");
			//cdda.CurrAddr.FAD=60000;

			cdda.playing=true;
			SecNumber.Status=GD_PLAY;

			u32 param_type=packet_cmd.data_8[1]&0x7;
			printf("param_type=%d\n",param_type);
			if (param_type==1)
			{
				cdda.StartAddr.FAD=cdda.CurrAddr.FAD=GetFAD(&packet_cmd.data_8[2],0);
				cdda.EndAddr.FAD=GetFAD(&packet_cmd.data_8[8],0);
				GDStatus.DSC=1;	//we did the seek xD lol
			}
			else if (param_type==2)
			{
				cdda.StartAddr.FAD=cdda.CurrAddr.FAD=GetFAD(&packet_cmd.data_8[2],1);
				cdda.EndAddr.FAD=GetFAD(&packet_cmd.data_8[8],1);
				GDStatus.DSC=1;	//we did the seek xD lol
			}
			else if (param_type==7)
			{
				//Resume from previus pos :)
			}
			else
			{
				die("SPI_CD_SEEK  : not known parameter..");
			}
			cdda.repeats=packet_cmd.data_8[6]&0xF;
			printf("cdda.StartAddr=%d\n",cdda.StartAddr.FAD);
			printf("cdda.EndAddr=%d\n",cdda.EndAddr.FAD);
			printf("cdda.repeats=%d\n",cdda.repeats);
			printf("cdda.playing=%d\n",cdda.playing);
			printf("cdda.CurrAddr=%d\n",cdda.CurrAddr.FAD);

			gd_set_state(gds_procpacketdone);
		}
		break;

	case SPI_CD_SEEK:
		{
			printf_spicmd("SPI_CD_SEEK\n");
			printf("GDROM: Unhandled Sega SPI frame: SPI_CD_SEEK\n");

			SecNumber.Status=GD_PAUSE;
			cdda.playing=false;

			u32 param_type=packet_cmd.data_8[1]&0x7;
			printf("param_type=%d\n",param_type);
			if (param_type==1)
			{
				cdda.StartAddr.FAD=cdda.CurrAddr.FAD=GetFAD(&packet_cmd.data_8[2],0);
				GDStatus.DSC=1;	//we did the seek xD lol
			}
			else if (param_type==2)
			{
				cdda.StartAddr.FAD=cdda.CurrAddr.FAD=GetFAD(&packet_cmd.data_8[2],1);
				GDStatus.DSC=1;	//we did the seek xD lol
			}
			else if (param_type==3)
			{
				//stop audio , goto home
				SecNumber.Status=GD_STANDBY;
				cdda.StartAddr.FAD=cdda.CurrAddr.FAD=150;
				GDStatus.DSC=1;	//we did the seek xD lol
			}
			else if (param_type==4)
			{
				//pause audio -- nothing more
			}
			else
			{
				die("SPI_CD_SEEK  : not known parameter..");
			}

			printf("cdda.StartAddr=%d\n",cdda.StartAddr.FAD);
			printf("cdda.EndAddr=%d\n",cdda.EndAddr.FAD);
			printf("cdda.repeats=%d\n",cdda.repeats);
			printf("cdda.playing=%d\n",cdda.playing);
			printf("cdda.CurrAddr=%d\n",cdda.CurrAddr.FAD);


			gd_set_state(gds_procpacketdone);
		}
		break;

	case SPI_CD_SCAN:
		printf_spicmd("SPI_CD_SCAN\n");
		printf("GDROM: Unhandled Sega SPI frame: SPI_CD_SCAN\n");
		

		gd_set_state(gds_procpacketdone);
		break;

	case SPI_GET_SCD:
		{
			printf_spicmd("SPI_GET_SCD\n");
			//printf("\nGDROM:\tUnhandled Sega SPI frame: SPI_GET_SCD\n");

			u32 format;
			format=packet_cmd.data_8[1]&0xF;
			u32 sz;
			u8 subc_info[100];


			//0	Reserved
			subc_info[0]=0;
			//1	Audio status
			if (SecNumber.Status==GD_STANDBY)
			{
				//13h	Audio playback ended normally
				subc_info[1]=0x13;
			}
			else if (SecNumber.Status==GD_PAUSE)
			{
				//12h	Audio playback paused
				subc_info[1]=0x12;
			}
			else if (SecNumber.Status==GD_PLAY)
			{
				//11h	Audio playback in progress
				subc_info[1]=0x11;
			}
			else
			{
				if (cdda.playing)
					subc_info[1]=0x11;//11h	Audio playback in progress
				else
					subc_info[1]=0x15;//15h	No audio status information
			}
			//2	DATA Length MSB (0 = 0h)
			subc_info[2]=0;
			//3	DATA Length LSB (14 = Eh)
			subc_info[3]=0xE;
			//4	Control	ADR
			subc_info[4]=1;	//Audio :p
			//5-13	DATA-Q
			u8* subc=&subc_info[5];
			//-When ADR = 1
			//Byte	Description
			//0	Control	1(ADR)
			subc[0]=1;
			//1	TNO
			subc[1]=4;//Track number .. duno whats it :P gota parse toc xD ;p
			//2	X
			subc[2]=1;//gap #1 (main track)
			//3-5	Elapsed FAD within track
			u32 FAD_el=cdda.CurrAddr.FAD-cdda.StartAddr.FAD;
			subc[3]=(u8)(FAD_el>>16);
			subc[4]=(u8)(FAD_el>>8);
			subc[5]=(u8)(FAD_el>>0);
			//6	0	0	0	0	0	0	0	0
			subc[6]=0;//
			//7-9	???
			//we'l see about em ;p

			if (format==0)
			{
				sz=100;
				memcpy(&subc_info[12+4],subc,12);
				//memset(&subc_info[4],11,100);
				subc_info[3]=100;
			}
			else
			{
				sz=12;
			}

			gd_spi_pio_end((u8*)&subc_info[0],sz);
		}
		break;

	default:
		printf("GDROM: Unhandled Sega SPI frame: %X\n", packet_cmd.data_8[0]);

		gd_set_state(gds_procpacketdone);
		break;
	}
}
//Read handler
u32 ReadMem_gdrom(u32 Addr, u32 sz)
{	
	switch (Addr)
	{
		//cancel interrupt
	case GD_STATUS_Read :
		SB_ISTEXT &= ~1;	//Clear INTRQ signal
		printf_rm("GDROM: STATUS [cancel int](v=%d)\n",GDStatus.full);
		return GDStatus.full;

	case GD_ALTSTAT_Read:
		printf_rm("GDROM: Read From AltStatus (v=%d)\n",GDStatus.full);
		return GDStatus.full;

	case GD_BYCTLLO	:
		printf_rm("GDROM: Read From GD_BYCTLLO\n");
		return ByteCount.low;

	case GD_BYCTLHI	:
		printf_rm("GDROM: Read From GD_BYCTLHI\n");
		return ByteCount.hi;


	case GD_DATA:
		if(2!=sz)
			printf("GDROM: Bad size on DATA REG Read\n");

		if (gd_state == gds_pio_send_data)
		{
			if (pio_buff.index==pio_buff.size)
				printf("GDROM: Illegal Read From DATA (underflow)\n");
			else
			{
				u32 rv= pio_buff.data[pio_buff.index];
				pio_buff.index+=1;
				ByteCount.full-=2;
				if (pio_buff.index==pio_buff.size)
				{
					verify(pio_buff.next_state != gds_pio_send_data);
					//end of pio transfer !
					gd_set_state(pio_buff.next_state);
				}
				return rv;
			}

		}
		else
			printf("GDROM: Illegal Read From DATA (wrong mode)\n");

		return 0;

	case GD_DRVSEL:
		printf_rm("GDROM: Read From DriveSel\n");
		return DriveSel;

	case GD_ERROR_Read:
		printf_rm("GDROM: Read from ERROR Register\n");
		return Error.full;

	case GD_IREASON_Read:
		printf_rm("GDROM: Read from INTREASON Register\n");
		return IntReason.full;

	case GD_SECTNUM:
		printf_rm("GDROM: Read from SecNumber Register (v=%X)\n", SecNumber.full);
		return SecNumber.full;

	default:
		printf("GDROM: Unhandled Read From %X sz:%X\n",Addr,sz);
		return 0;
	}
}

//Write Handler
void WriteMem_gdrom(u32 Addr, u32 data, u32 sz)
{
	switch(Addr)
	{
	case GD_BYCTLLO:
		printf_rm("GDROM: Write to GD_BYCTLLO = %X sz:%X\n",data,sz);
		ByteCount.low =(u8) data;
		break;

	case GD_BYCTLHI: 
		printf_rm("GDROM: Write to GD_BYCTLHI = %X sz:%X\n",data,sz);
		ByteCount.hi =(u8) data;
		break;

	case GD_DATA: 
		{
			if(2!=sz)
				printf("GDROM: Bad size on DATA REG\n");
			if (gd_state == gds_waitpacket)
			{
				packet_cmd.data_16[packet_cmd.index]=(u16)data;
				packet_cmd.index+=1;
				if (packet_cmd.index==6)
					gd_set_state(gds_procpacket);
			}
			else if (gd_state == gds_pio_get_data)
			{
				pio_buff.data[pio_buff.index]=(u16)data;
				pio_buff.index+=1;
				if (pio_buff.size==pio_buff.index)
				{
					verify(pio_buff.next_state!=gds_pio_get_data);
					gd_set_state(pio_buff.next_state);
				}
			}
			else
				printf("GDROM: Illegal Write to DATA\n");
			return;
		}
	case GD_DEVCTRL_Write:
		printf("GDROM: Write GD_DEVCTRL ( Not implemented on dc)\n");
		break;

	case GD_DRVSEL: 
		printf("GDROM: Write to GD_DRVSEL\n");
		DriveSel = data; 
		break;


		//	By writing "3" as Feature Number and issuing the Set Feature command,
		//	the PIO or DMA transfer mode set in the Sector Count register can be selected.
		//	The actual transfer mode is specified by the Sector Counter Register. 

	case GD_FEATURES_Write:
		printf_rm("GDROM: Write to GD_FEATURES\n");
		Features.full =(u8) data;
		break;

	case GD_SECTCNT_Write:
		printf("GDROM: Write to SecCount = %X\n", data);
		SecCount.full =(u8) data;
		break;

	case GD_SECTNUM	:
		printf("GDROM: Write to SecNum; not possible = %X\n", data);
		break;

	case GD_COMMAND_Write:
		verify(sz==1);
		if ((data !=ATA_NOP) && (data != ATA_SOFT_RESET))
			verify(gd_state==gds_waitcmd);
		//printf("\nGDROM:\tCOMMAND: %X !\n", data);
		ata_cmd.command=(u8)data;
		gd_set_state(gds_procata);
		break;

	default:
		printf("\nGDROM:\tUnhandled Write to %X <= %X sz:%X\n",Addr,data,sz);
		break;
	}
}

//is this needed ?
void UpdateGDRom()
{
	/*
	die("UpdateGDRom()");
	*/
}
//Dma Start
void GDROM_DmaStart(u32 data)
{
	if(!(data&1) || !(SB_GDEN &1))
		return;

	//printf(">>\tGDROM : DMA initiated\n");
	SB_GDST=0;

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

	if(0 == len) 
	{
		printf("\n!\tGDROM: Len: %X, Normal Termination !\n", len);
	}
	u32 len_backup=len;
	if( 1 == SB_GDDIR ) 
	{
		while(len)
		{
			u32 buff_size =read_buff.cache_size - read_buff.cache_index;
			if (buff_size==0)
			{
				verify(read_params.remaining_sectors>0);
				//buffer is empty , fill it :)
				FillReadBuffer();
			}

			//transfer up to len bytes
			if (buff_size>len)
			{
				buff_size=len;
			}
			WriteMemBlock_nommu(src,(u32*)&read_buff.cache[read_buff.cache_index], buff_size);
			read_buff.cache_index+=buff_size;
			src+=buff_size;
			len-=buff_size;
		}
	}
	else
		msgboxf("GDROM: SB_GDDIR %X (TO AICA WAVE MEM?)",MBX_ICONERROR, SB_GDDIR);

	SB_GDLEN = 0x00000000;
	SB_GDLEND=len_backup;
	SB_GDSTAR = (src + len_backup);
	SB_GDST=0;//done

	// The DMA end interrupt flag
	RaiseInterrupt(holly_GDROM_DMA);

	//Readed ALL sectors
	if (read_params.remaining_sectors==0)
	{
		u32 buff_size =read_buff.cache_size - read_buff.cache_index;
		//And all buffer :p
		if (buff_size==0)
			gd_set_state(gds_procpacketdone);
	}
}


//Init/Term/Res
void gdrom_reg_Init()
{
	sb_regs[(SB_GDST_addr-SB_BASE)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[(SB_GDST_addr-SB_BASE)>>2].writeFunction=GDROM_DmaStart;
}
void gdrom_reg_Term()
{
	
}

void gdrom_reg_Reset(bool Manual)
{
	//huh?
}