//duh .. this will be dmac
#include "types.h"
#include "dc/mem/sh4_internal_reg.h"
#include "dc/mem/sb.h"
#include "dc/mem/sh4_mem.h"
#include "dmac.h"
#include "intc.h"
#include "plugins/plugin_manager.h"

#define DMAOR_MASK	0xFFFF8201

u32 DMAC_SAR0;
u32 DMAC_DAR0;
u32 DMAC_DMATCR0;
u32 DMAC_CHCR0;
u32 DMAC_SAR1;
u32 DMAC_DAR1;
u32 DMAC_DMATCR1;
u32 DMAC_CHCR1;
u32 DMAC_SAR2;
u32 DMAC_DAR2;
u32 DMAC_DMATCR2;
u32 DMAC_CHCR2;
u32 DMAC_SAR3;
u32 DMAC_DAR3;
u32 DMAC_DMATCR3;
u32 DMAC_CHCR3;
u32 DMAC_DMAOR;

void DMAC_Ch2St()
{
	u32 chcr	= DMAC_CHCR2,
		dmaor	= DMAC_DMAOR,
		dmatcr	= DMAC_DMATCR2;

	u32	src		= DMAC_SAR2,
		dst		= SB_C2DSTAT,
		len		= SB_C2DLEN ;

	if(0x8201 != (dmaor &DMAOR_MASK)) {
		printf("\n!\tDMAC: DMAOR has invalid settings (%X) !\n", dmaor);
		return;
	}
	if( len & 0x1F ) {
		printf("\n!\tDMAC: SB_C2DLEN has invalid size (%X) !\n", len);
		return;
	}

//	printf(">>\tDMAC: Ch2 DMA SRC=%X DST=%X LEN=%X\n", src, dst, len );

	// Direct DList DMA (Ch2)

			// Texture DMA 
	if( (dst >= 0x10000000) && (dst <= 0x10FFFFE0) )
	{
		//printf(".\tPVR DList DMA (direct unknown)\n\n");

		u32 *sys_buf=(u32 *)(&mem_b[src&0xFFFFFF]);

		//printf("%d\n",len/32);
		libPvr->pvr_info.TADma(dst,sys_buf,(len/32));

	}
	else	//	If SB_C2DSTAT reg is inrange from 0x11000000 to 0x11FFFFE0,	 set 1 in SB_LMMODE0 reg.
	if( (dst >= 0x11000000) && (dst <= 0x11FFFFE0) )
	{
		//printf(">>\tDMAC: TEX LNMODE0 Ch2 DMA SRC=%X DST=%X LEN=%X | LN(%X::%X)\n", src, dst, len, *pSB_LMMODE0, *pSB_LMMODE1 );

		u32 *sys_buf=(u32 *)(&mem_b[src&0xFFFFFF]);
		u32 dst_ptr=(dst&0xFFFFFF) |0xa4000000;//

		for (u32 i_c=0;i_c<len;i_c+=4)
		{
			WriteMem32(dst_ptr+i_c,*sys_buf++);
		}

	//	*pSB_LMMODE0 = 1;			// this prob was done by system already
	//	WriteMem(SB_LMMODE1, 0, 4);	// should this be done ?
	}
	else	//	If SB_C2DSTAT reg is inrange from 0x13000000 to 0x13FFFFE0,	 set 1 in SB_LMMODE1 reg.
	if( (dst >= 0x13000000) && (dst <= 0x13FFFFE0) )
	{
		printf(".\tPVR DList DMA LNMODE1\n\n");
	//	*pSB_LMMODE1 = 1;			// this prob was done by system already
	//	WriteMem(SB_LMMODE0, 0, 4);	// should this be done ?
	}
	else { printf("\n!\tDMAC: SB_C2DSTAT has invalid address (%X) !\n", dst); return; }


	// Setup some of the regs so it thinks we've finished DMA

	DMAC_SAR2 = (src + len);
	DMAC_CHCR2 &= 0xFFFFFFFE;
	DMAC_DMATCR2 = 0x00000000;

	SB_C2DST = 0x00000000;
	SB_C2DLEN = 0x00000000;
	SB_C2DSTAT = (src + len);

	// The DMA end interrupt flag (SB_ISTNRM - bit 19: DTDE2INT) is set to "1."
	RaiseInterrupt(holly_PVR_DMA);
}

//Init term res
void dmac_Init()
{
	//DMAC SAR0 0xFFA00000 0x1FA00000 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_SAR0_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_SAR0_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_SAR0_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_SAR0_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_SAR0_addr&0xFF)>>2].data32=&DMAC_SAR0;

	//DMAC DAR0 0xFFA00004 0x1FA00004 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_DAR0_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_DAR0_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_DAR0_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_DAR0_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_DAR0_addr&0xFF)>>2].data32=&DMAC_DAR0;

	//DMAC DMATCR0 0xFFA00008 0x1FA00008 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_DMATCR0_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_DMATCR0_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_DMATCR0_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_DMATCR0_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_DMATCR0_addr&0xFF)>>2].data32=&DMAC_DMATCR0;

	//DMAC CHCR0 0xFFA0000C 0x1FA0000C 32 0x00000000 0x00000000 Held Held Bclk
	DMAC[(DMAC_CHCR0_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_CHCR0_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_CHCR0_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_CHCR0_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_CHCR0_addr&0xFF)>>2].data32=&DMAC_CHCR0;

	//DMAC SAR1 0xFFA00010 0x1FA00010 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_SAR1_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_SAR1_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_SAR1_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_SAR1_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_SAR1_addr&0xFF)>>2].data32=&DMAC_SAR1;

	//DMAC DAR1 0xFFA00014 0x1FA00014 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_DAR1_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_DAR1_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_DAR1_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_DAR1_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_DAR1_addr&0xFF)>>2].data32=&DMAC_DAR1;

	//DMAC DMATCR1 0xFFA00018 0x1FA00018 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_DMATCR1_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_DMATCR1_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_DMATCR1_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_DMATCR1_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_DMATCR1_addr&0xFF)>>2].data32=&DMAC_DMATCR1;

	//DMAC CHCR1 0xFFA0001C 0x1FA0001C 32 0x00000000 0x00000000 Held Held Bclk
	DMAC[(DMAC_CHCR1_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_CHCR1_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_CHCR1_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_CHCR1_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_CHCR1_addr&0xFF)>>2].data32=&DMAC_CHCR1;

	//DMAC SAR2 0xFFA00020 0x1FA00020 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_SAR2_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_SAR2_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_SAR2_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_SAR2_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_SAR2_addr&0xFF)>>2].data32=&DMAC_SAR2;

	//DMAC DAR2 0xFFA00024 0x1FA00024 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_DAR2_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_DAR2_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_DAR2_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_DAR2_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_DAR2_addr&0xFF)>>2].data32=&DMAC_DAR2;

	//DMAC DMATCR2 0xFFA00028 0x1FA00028 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_DMATCR2_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_DMATCR2_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_DMATCR2_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_DMATCR2_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_DMATCR2_addr&0xFF)>>2].data32=&DMAC_DMATCR2;

	//DMAC CHCR2 0xFFA0002C 0x1FA0002C 32 0x00000000 0x00000000 Held Held Bclk
	DMAC[(DMAC_CHCR2_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_CHCR2_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_CHCR2_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_CHCR2_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_CHCR2_addr&0xFF)>>2].data32=&DMAC_CHCR2;

	//DMAC SAR3 0xFFA00030 0x1FA00030 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_SAR3_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_SAR3_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_SAR3_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_SAR3_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_SAR3_addr&0xFF)>>2].data32=&DMAC_SAR3;

	//DMAC DAR3 0xFFA00034 0x1FA00034 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_DAR3_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_DAR3_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_DAR3_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_DAR3_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_DAR3_addr&0xFF)>>2].data32=&DMAC_DAR3;

	//DMAC DMATCR3 0xFFA00038 0x1FA00038 32 Undefined Undefined Held Held Bclk
	DMAC[(DMAC_DMATCR3_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_DMATCR3_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_DMATCR3_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_DMATCR3_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_DMATCR3_addr&0xFF)>>2].data32=&DMAC_DMATCR3;

	//DMAC CHCR3 0xFFA0003C 0x1FA0003C 32 0x00000000 0x00000000 Held Held Bclk
	DMAC[(DMAC_CHCR3_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_CHCR3_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_CHCR3_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_CHCR3_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_CHCR3_addr&0xFF)>>2].data32=&DMAC_CHCR3;

	//DMAC DMAOR 0xFFA00040 0x1FA00040 32 0x00000000 0x00000000 Held Held Bclk
	DMAC[(DMAC_DMAOR_addr&0xFF)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	DMAC[(DMAC_DMAOR_addr&0xFF)>>2].NextCange=0;
	DMAC[(DMAC_DMAOR_addr&0xFF)>>2].readFunction=0;
	DMAC[(DMAC_DMAOR_addr&0xFF)>>2].writeFunction=0;
	DMAC[(DMAC_DMAOR_addr&0xFF)>>2].data32=&DMAC_DMAOR;
}
void dmac_Reset(bool Manual)
{
	
}
void dmac_Term()
{
}