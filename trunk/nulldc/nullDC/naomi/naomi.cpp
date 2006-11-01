#include "../types.h"
#include "../dc/mem/sb.h"
#include "naomi.h"
#include "naomi_regs.h"

#ifdef BUILD_NAOMI	

void naomi_WriteXicor(u32 Addr, u32 data, u32 sz)
{
	int Dat	= data&1;
	int Clk	= data&2;
	int CS	= data&4;
	int Rst	= data&8;
/*
	GameEPROM.SetCS(CS);
	GameEPROM.SetRST(Rst);
	GameEPROM.SetSDA(Dat);
	GameEPROM.SetSCL(Clk);
*/
}
u32  naomi_ReadXicor(u32 Addr, u32 sz)
{
	return 0;//SB_NXICOR_WR;
}

u32  naomi_reg_ReadMem(u32 Addr, u32 sz)
{
	printf("naomi ReadMem: %X, %d\n", Addr, sz);
	return 0;

}
void naomi_reg_WriteMem(u32 Addr, u32 data, u32 sz)
{
	printf("naomi WriteMem: %X <= %X, %d\n", Addr, data, sz);
}



void naomi_reg_Init()
{
	sb_regs[((NAOMI_ROM_OFFSETH_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_ROM_OFFSETH_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_ROM_OFFSETH_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;
	sb_regs[((NAOMI_ROM_OFFSETH_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;
	sb_regs[((NAOMI_ROM_OFFSETH_addr-SB_BASE))>>2].data32=&NAOMI_ROM_OFFSETH;


	sb_regs[((NAOMI_ROM_OFFSETL_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_ROM_OFFSETL_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_ROM_OFFSETL_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;;
	sb_regs[((NAOMI_ROM_OFFSETL_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;;
	sb_regs[((NAOMI_ROM_OFFSETL_addr-SB_BASE))>>2].data32=&NAOMI_ROM_OFFSETL;


	sb_regs[((NAOMI_ROM_DATA_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_ROM_DATA_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_ROM_DATA_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;;
	sb_regs[((NAOMI_ROM_DATA_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;;
	sb_regs[((NAOMI_ROM_DATA_addr-SB_BASE))>>2].data32=&NAOMI_ROM_DATA;


	sb_regs[((NAOMI_DMA_OFFSETH_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_DMA_OFFSETH_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_DMA_OFFSETH_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;;
	sb_regs[((NAOMI_DMA_OFFSETH_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;;
	sb_regs[((NAOMI_DMA_OFFSETH_addr-SB_BASE))>>2].data32=&NAOMI_DMA_OFFSETH;


	sb_regs[((NAOMI_DMA_OFFSETL_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_DMA_OFFSETL_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_DMA_OFFSETL_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;;
	sb_regs[((NAOMI_DMA_OFFSETL_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;;
	sb_regs[((NAOMI_DMA_OFFSETL_addr-SB_BASE))>>2].data32=&NAOMI_DMA_OFFSETL;


	sb_regs[((NAOMI_DMA_COUNT_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_DMA_COUNT_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_DMA_COUNT_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;;
	sb_regs[((NAOMI_DMA_COUNT_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;;
	sb_regs[((NAOMI_DMA_COUNT_addr-SB_BASE))>>2].data32=&NAOMI_DMA_COUNT;


	sb_regs[((NAOMI_BOARDID_WRITE_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_BOARDID_WRITE_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_BOARDID_WRITE_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;;
	sb_regs[((NAOMI_BOARDID_WRITE_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;;
	sb_regs[((NAOMI_BOARDID_WRITE_addr-SB_BASE))>>2].data32=&NAOMI_BOARDID_WRITE;


	sb_regs[((NAOMI_BOARDID_READ_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_BOARDID_READ_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_BOARDID_READ_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;;
	sb_regs[((NAOMI_BOARDID_READ_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;;
	sb_regs[((NAOMI_BOARDID_READ_addr-SB_BASE))>>2].data32=&NAOMI_BOARDID_READ;


	sb_regs[((NAOMI_COMM_OFFSET_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_COMM_OFFSET_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_COMM_OFFSET_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;;
	sb_regs[((NAOMI_COMM_OFFSET_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;;
	sb_regs[((NAOMI_COMM_OFFSET_addr-SB_BASE))>>2].data32=&NAOMI_COMM_OFFSET;


	sb_regs[((NAOMI_COMM_DATA_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_COMM_DATA_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_COMM_DATA_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;;
	sb_regs[((NAOMI_COMM_DATA_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;;
	sb_regs[((NAOMI_COMM_DATA_addr-SB_BASE))>>2].data32=&NAOMI_COMM_DATA;


	sb_regs[((NAOMI_XICOR_WRITE_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_XICOR_WRITE_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_XICOR_WRITE_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;;
	sb_regs[((NAOMI_XICOR_WRITE_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;;
	sb_regs[((NAOMI_XICOR_WRITE_addr-SB_BASE))>>2].data32=&NAOMI_XICOR_WRITE;

	sb_regs[((NAOMI_XICOR_READ_addr-SB_BASE))>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA | REG_WRITE_DATA;
	sb_regs[((NAOMI_XICOR_READ_addr-SB_BASE))>>2].NextCange=0;
	sb_regs[((NAOMI_XICOR_READ_addr-SB_BASE))>>2].readFunction=(RegReadFP*)naomi_reg_ReadMem;;
	sb_regs[((NAOMI_XICOR_READ_addr-SB_BASE))>>2].writeFunction=(RegWriteFP*)naomi_reg_WriteMem;;
	sb_regs[((NAOMI_XICOR_READ_addr-SB_BASE))>>2].data32=&NAOMI_XICOR_READ;

}



void naomi_reg_Term(){}
void naomi_reg_Reset(bool Manual){}






u32 NAOMI_ROM_OFFSETH;
u32 NAOMI_ROM_OFFSETL;
u32 NAOMI_ROM_DATA;
u32 NAOMI_DMA_OFFSETH;
u32 NAOMI_DMA_OFFSETL;
u32 NAOMI_DMA_COUNT;
u32 NAOMI_BOARDID_WRITE;
u32 NAOMI_BOARDID_READ;
u32 NAOMI_COMM_OFFSET;
u32 NAOMI_COMM_DATA;
u32 NAOMI_XICOR_WRITE;
u32 NAOMI_XICOR_READ;



#endif // BUILD_NAOMI