#include "types.h"
#include "dc/mem/sb.h"
#include "dc/sh4/dmac.h"
#include "pvr_sb_regs.h"

u32 pvr_sb_readreg_Pvr(u32 addr,u32 sz);
void pvr_sb_writereg_Pvr(u32 addr,u32 data,u32 sz);

void RegWrite_SB_C2DST(u32 data)
{
	if(1&data)
	{
		SB_C2DST=1;
		DMAC_Ch2St();
	}
}

//Init/Term , global
void pvr_sb_Init()
{
	//0x005F7C18	SB_PDST	RW	PVR-DMA start
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].readFunction=0;
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].writeFunction=RegWrite_SB_C2DST;
	sb_regs[((SB_C2DST_addr-SB_BASE))>>2].data32=&SB_PDST;
}
void pvr_sb_Term()
{
}
//Reset -> Reset - Initialise
void pvr_sb_Reset(bool Manual)
{
}