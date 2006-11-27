#include "../types.h"
#include "../dc/mem/sb.h"
#include "naomi.h"
#include "naomi_regs.h"

#ifdef BUILD_NAOMI	



void WriteMem_Xicor(u32 Addr, u32 data, u32 sz)
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
u32  ReadMem_Xicor(u32 Addr, u32 sz)
{
	return SB_NXICOR_WR;
}

u32  ReadMem_naomi(u32 Addr, u32 sz)
{
	switch(Addr&255)
	{
	case NAOMI_ROM_OFFSETH_addr&255:
	case NAOMI_ROM_OFFSETL_addr&255:
	case NAOMI_ROM_DATA_addr&255:
	case NAOMI_DMA_OFFSETH_addr&255:
	case NAOMI_DMA_OFFSETL_addr&255:
	case NAOMI_DMA_COUNT_addr&255:
	case NAOMI_BOARDID_WRITE_addr&255:
	case NAOMI_BOARDID_READ_addr&255:
	case NAOMI_COMM_OFFSET_addr&255:
	case NAOMI_COMM_DATA_addr&255:
//	case NAOMI_XICOR_WRITE_addr&255:
//	case NAOMI_XICOR_READ_addr&255:
		//printf("-> nReg handled %X \n", Addr);
		return 1;

	default: break;
	}
	printf("naomi ReadMem: %X, %d\n", Addr, sz);
	return 0;

}
void WriteMem_naomi(u32 Addr, u32 data, u32 sz)
{
	switch(Addr&255)
	{
	case NAOMI_ROM_OFFSETH_addr&255:
	case NAOMI_ROM_OFFSETL_addr&255:
	case NAOMI_ROM_DATA_addr&255:
	case NAOMI_DMA_OFFSETH_addr&255:
	case NAOMI_DMA_OFFSETL_addr&255:
	case NAOMI_DMA_COUNT_addr&255:
	case NAOMI_BOARDID_WRITE_addr&255:
	case NAOMI_BOARDID_READ_addr&255:
	case NAOMI_COMM_OFFSET_addr&255:
	case NAOMI_COMM_DATA_addr&255:
		//	case NAOMI_XICOR_WRITE_addr&255:
		//	case NAOMI_XICOR_READ_addr&255:
		//printf("-> nReg handled %X \n", Addr);
		return;

	default: break;
	}
	printf("naomi WriteMem: %X <= %X, %d\n", Addr, data, sz);
}







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