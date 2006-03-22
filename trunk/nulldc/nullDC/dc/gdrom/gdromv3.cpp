#include "gdromv3.h"

#include "..\..\plugins\plugin_manager.h"
#include "..\mem\sh4_mem.h"
#include "..\mem\memutil.h"
#include "..\mem\sb.h"
#include "..\sh4\dmac.h"
#include "..\sh4\intc.h"
#include "..\sh4\sh4_registers.h"

Fifo_List<u8> DataRead;
Fifo_List<u8> DataWrite;
//
u32 data_write_mode=0;

//Registers
	u32 DriveSel;
	GD_ErrRegT Error;
	GD_InterruptReasonT IntReason;
	GD_FeaturesT Features;
	GD_SecCountT SecCount;
	GD_SecNumbT SecNumber;
	
	GD_StatusT GDStatus;

	u8 ByteCount_Low;
	u8 ByteCount_Hi;
//end


void SendToHost(u8* data,u32 Count)
{
	if (DataRead.IsEmpty())
	{
		//Warn , fifo buffer has better to be empty
	}

	if (Count==0)
	{
		//do magic
	}
	else
		DataRead.PutItems(data,Count);

	
}
//disk changes ect
void GDNotifyEvent(DriveEvent info,void* param)
{

}

//Read handler
u32 ReadMem_gdrom(u32 Addr, u32 sz)
{	
	switch (Addr)
	{
		//cancel interrupt
		case GD_STATUS_Read :
			SB_ISTEXT &= ~1;	//Clear INTRQ signal
			printf("GDROM:\t STATUS [cancel int]\n");
			return GDStatus.full;

		case GD_ALTSTAT_Read:
			printf("GDROM:\t Read From AltStatus\n");
			return GDStatus.full;

		case GD_BYCTLLO	:
			printf("GDROM:\t Read From GD_BYCTLLO\n");
			return ByteCount_Low;
		
		case GD_BYCTLHI	:
			printf("GDROM:\t Read From GD_BYCTLHI\n");
			return ByteCount_Hi;
		

		case GD_DATA:
			if(2!=sz)
				printf("\nGDROM:\tBad size on DATA REG Read !\n");
			
			//If data len==0 , this is allready done, on the "send data to host" function
			if(!DataRead.IsEmpty())
			{
				u8 rv=DataRead.GetItem();
				if (DataRead.IsEmpty())
				{
					//10.	When the device is ready to send the status, it writes the 
					//final status to the "Status" register. CoD, IO, DRDY are set 
					//(before making INTRQ valid), and BSY and DRQ are cleared. 
					//end of transfer to host
					IntReason.CoD=1;
					IntReason.IO=1;
					GDStatus.DRDY=0;
					GDStatus.BSY=0;
					GDStatus.DRQ=0;
					//INTRQ is made valid
					RaiseInterrupt(holly_GDROM_CMD);
					

					//11.	After verifying that INTRQ & DRQ = 0, read the Status register on the host. 
					//If the Check bit is set, read the command completion status from the Error register.

					//Status register is set before the "send data to host" function , 
					//so no need to set anything here
				}
				return rv;
			}
			else
				printf("GDROM:\tIllegal Read From DATA (underflow) !\n");
			
			return 0;

		case GD_DRVSEL:
			printf("GDROM:\t Read From DriveSel\n");
			return DriveSel;
		
		case GD_ERROR_Read:
			printf("\nGDROM:\tRead from ERROR Register!\n");
			return Error.full;
		
		case GD_IREASON_Read:
			printf("\nGDROM:\tRead from INTREASON Register!\n");
			return IntReason.full;
		
		case GD_SECTNUM:
			printf("\nGDROM:\tRead from SecNumber Register (v=%X)\n", SecNumber.full);
			return SecNumber.full;
		
		default:
			printf("\nGDROM:\tUnhandled Read From %X sz:%X\n",Addr,sz);
			return 0;
	}
}

//Write Handler
void WriteMem_gdrom(u32 Addr, u32 data, u32 sz)
{
	switch(Addr)
	{
	case GD_BYCTLLO:
		printf("\nGDROM:\tWrite to %X <= %X sz:%X\n",Addr,data,sz);
		ByteCount_Low =(u8) data;
		break;

	case GD_BYCTLHI: 
		printf("\nGDROM:\tWrite to %X <= %X sz:%X\n",Addr,data,sz);
		ByteCount_Hi =(u8) data;
		break;

	case GD_DATA: 
		{
			if(2!=sz)
				printf("\nGDROM:\tBad size on DATA REG !\n");

			switch(data_write_mode)
			{

			default:
				printf("GDROM:\tIllegal Write to DATA!\n");
			}

			return;
		}
	case GD_DEVCTRL_Write:
		printf("\nGDROM:\tWrite GD_DEVCTRL ( Not implemented on dc)\n");
		break;

	case GD_DRVSEL: 
		printf("\nGDROM:\tWrite to GD_DRVSEL\n");
		DriveSel = data; 
		break;


		//	By writing "3" as Feature Number and issuing the Set Feature command,
		//	the PIO or DMA transfer mode set in the Sector Count register can be selected.
		//	The actual transfer mode is specified by the Sector Counter Register. 

	case GD_FEATURES_Write:
		printf("\nGDROM:\tWrite to GD_FEATURES\n");
		Features.full =(u8) data;
		break;

	case GD_SECTCNT_Write:
		printf("\nGDROM:\tWrite to SecCount <= %X !\n", data);
		SecCount.full =(u8) data;
		break;

	case GD_SECTNUM	:
		printf("\nGDROM:\tWrite to SecNum; not possible <= %X !\n", data);
		break;

	case GD_COMMAND_Write:
		printf("\nGDROM:\tCOMMAND: %X !\n", data);
		break;

	default:
		printf("\nGDROM:\tUnhandled Write to %X <= %X sz:%X\n",Addr,data,sz);
		break;
	}
}

//is this needed ?
void UpdateGDRom()
{

}
//Dma Start
void GDROM_DmaStart(u32 data)
{
	if((1==(data&0x1)) && (SB_GDEN &1))
	{
		printf(">>\tGDROM : DMA initiated\n");
		SB_GDST=1;
	}
	else
	{

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