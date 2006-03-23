//Bus state controller registers

#include "types.h"
#include "dc/mem/sh4_internal_reg.h"
#include "bsc.h"


//32 bits
//All bits exept A0MPX,MASTER,ENDIAN are editable and reseted to 0
BCR1_type BSC_BCR1;

//16 bit
//A0SZ0,A0SZ1 are read olny , ohters are are editable and reseted to 0
BCR2_type BSC_BCR2;

//32 bits
WCR1_type BSC_WCR1;

//32 bits
WCR2_type BSC_WCR2;

//32 bits
WCR3_type BSC_WCR3;

//32 bits
MCR_type BSC_MCR;

//16 bits
PCR_type BSC_PCR;

//16 bits -> misstype on manual ? RTSCR vs RTCSR...
RTCSR_type BSC_RTCSR;

//16 bits
RTCNT_type BSC_RTCNT;

//16 bits
RTCOR_type BSC_RTCOR;

//16 bits
RFCR_type BSC_RFCR;

//32 bits
PCTRA_type BSC_PCTRA;

//16 bits
PDTRA_type BSC_PDTRA;

//32 bits
PCTRB_type BSC_PCTRB;

//16 bits
PDTRB_type BSC_PDTRB;

//16 bits
GPIOIC_type BSC_GPIOIC;

u32 read_BSC_PDTRA()
{
	/* chankast */
	u32 tpctra = BSC_PCTRA.full;
	u32 tpdtra = BSC_PDTRA.full;
	u32 tfinal=0;
	// magic values
	if ((tpctra&0xf) == 0x8)
		tfinal = 3;
	else if ((tpctra&0xf) == 0xB)
		tfinal = 3;
	else			
		tfinal = 0;


	if ((tpctra&0xf) == 0xB && (tpdtra&0xf) == 2)
		tfinal = 0;
	else if ((tpctra&0xf) == 0xC && (tpdtra&0xf) == 2)
		tfinal = 3;      

	if (tpctra == 0x0)
		tfinal = 3<<8;  

	return tfinal | 3<<8;
}
//Init term res
void bsc_Init()
{
	//BSC BCR1 0xFF800000 0x1F800000 32 0x00000000 Held Held Held Bclk
	BSC[(BSC_BCR1_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_BCR1_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_BCR1_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_BCR1_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_BCR1_addr&0xFF)>>2].data32=&BSC_BCR1.full;

	//BSC BCR2 0xFF800004 0x1F800004 16 0x3FFC Held Held Held Bclk
	BSC[(BSC_BCR2_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_BCR2_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_BCR2_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_BCR2_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_BCR2_addr&0xFF)>>2].data16=&BSC_BCR2.full;

	//BSC WCR1 0xFF800008 0x1F800008 32 0x77777777 Held Held Held Bclk
	BSC[(BSC_WCR1_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_WCR1_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_WCR1_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_WCR1_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_WCR1_addr&0xFF)>>2].data32=&BSC_WCR1.full;

	//BSC WCR2 0xFF80000C 0x1F80000C 32 0xFFFEEFFF Held Held Held Bclk
	BSC[(BSC_WCR2_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_WCR2_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_WCR2_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_WCR2_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_WCR2_addr&0xFF)>>2].data32=&BSC_WCR2.full;

	//BSC WCR3 0xFF800010 0x1F800010 32 0x07777777 Held Held Held Bclk
	BSC[(BSC_WCR3_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_WCR3_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_WCR3_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_WCR3_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_WCR3_addr&0xFF)>>2].data32=&BSC_WCR3.full;

	//BSC MCR 0xFF800014 0x1F800014 32 0x00000000 Held Held Held Bclk
	BSC[(BSC_MCR_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_MCR_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_MCR_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_MCR_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_MCR_addr&0xFF)>>2].data32=&BSC_MCR.full;

	//BSC PCR 0xFF800018 0x1F800018 16 0x0000 Held Held Held Bclk
	BSC[(BSC_PCR_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_PCR_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_PCR_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_PCR_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_PCR_addr&0xFF)>>2].data16=&BSC_PCR.full;

	//BSC RTCSR 0xFF80001C 0x1F80001C 16 0x0000 Held Held Held Bclk
	BSC[(BSC_RTCSR_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_RTCSR_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_RTCSR_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_RTCSR_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_RTCSR_addr&0xFF)>>2].data16=&BSC_RTCSR.full;

	//BSC RTCNT 0xFF800020 0x1F800020 16 0x0000 Held Held Held Bclk
	BSC[(BSC_RTCNT_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_RTCNT_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_RTCNT_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_RTCNT_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_RTCNT_addr&0xFF)>>2].data16=&BSC_RTCNT.full;

	//BSC RTCOR 0xFF800024 0x1F800024 16 0x0000 Held Held Held Bclk
	BSC[(BSC_RTCOR_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_RTCOR_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_RTCOR_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_RTCOR_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_RTCOR_addr&0xFF)>>2].data16=&BSC_RTCOR.full;

	//BSC RFCR 0xFF800028 0x1F800028 16 0x0000 Held Held Held Bclk
	BSC[(BSC_RFCR_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_RFCR_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_RFCR_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_RFCR_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_RFCR_addr&0xFF)>>2].data16=&BSC_RFCR.full;

	//BSC PCTRA 0xFF80002C 0x1F80002C 32 0x00000000 Held Held Held Bclk
	BSC[(BSC_PCTRA_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_PCTRA_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_PCTRA_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_PCTRA_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_PCTRA_addr&0xFF)>>2].data32=&BSC_PCTRA.full;

	//BSC PDTRA 0xFF800030 0x1F800030 16 Undefined Held Held Held Bclk
	BSC[(BSC_PDTRA_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_WRITE_DATA;
	BSC[(BSC_PDTRA_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_PDTRA_addr&0xFF)>>2].readFunction=read_BSC_PDTRA;
	BSC[(BSC_PDTRA_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_PDTRA_addr&0xFF)>>2].data16=&BSC_PDTRA.full;

	//BSC PCTRB 0xFF800040 0x1F800040 32 0x00000000 Held Held Held Bclk
	BSC[(BSC_PCTRB_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_PCTRB_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_PCTRB_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_PCTRB_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_PCTRB_addr&0xFF)>>2].data32=&BSC_PCTRB.full;

	//BSC PDTRB 0xFF800044 0x1F800044 16 Undefined Held Held Held Bclk
	BSC[(BSC_PDTRB_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_PDTRB_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_PDTRB_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_PDTRB_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_PDTRB_addr&0xFF)>>2].data16=&BSC_PDTRB.full;

	//BSC GPIOIC 0xFF800048 0x1F800048 16 0x00000000 Held Held Held Bclk
	BSC[(BSC_GPIOIC_addr&0xFF)>>2].flags=REG_16BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	BSC[(BSC_GPIOIC_addr&0xFF)>>2].NextCange=0;
	BSC[(BSC_GPIOIC_addr&0xFF)>>2].readFunction=0;
	BSC[(BSC_GPIOIC_addr&0xFF)>>2].writeFunction=0;
	BSC[(BSC_GPIOIC_addr&0xFF)>>2].data16=&BSC_GPIOIC.full;
}
void bsc_Reset(bool Manual)
{
}
void bsc_Term()
{
}