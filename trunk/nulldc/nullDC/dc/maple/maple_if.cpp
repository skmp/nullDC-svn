#include "types.h"
#include "maple_if.h"

#include "dc/sh4/intc.h"
#include "dc/mem/sb.h"
#include "dc/mem/sh4_mem.h"

//typedef bool mInitFP ();
//typedef void mTermFP ();

typedef void MapleGotData(u32 header1,u32 header2,u32*data,u32 datalen);

struct MaplePluginInfo
{
	u32 InterfaceVersion;

	//UpdateCBFP* UpdateMaple;
	bool Connected;//:)
	MapleGotData* GotDataCB;
	InitFP* Init;
	TermFP* Term;
};

MaplePluginInfo MaplePlugin[4][6];

void DoMapleDma();


void testJoy_GotData(u32 header1,u32 header2,u32*data,u32 datalen);


void InitMaple()
{
	MaplePlugin[0][0].Connected=true;
	MaplePlugin[0][0].GotDataCB=testJoy_GotData;
	//MaplePlugin[0].Init=0;
	//MaplePlugin[0].InterfaceVersion=0;
}

void maple_SB_MDST_Write(u32 data)
{
	if (data & 0x1)
	{
		SB_MDST=1;
		DoMapleDma();	
	}
}

void DoMapleDma()
{
#if debug_maple
	printf("Maple :DoMapleDma\n");
#endif
	u32 addr = SB_MDSTAR;	//*MAPLE_DMAADDR;
	bool last = false;
	while (last != true)
	{
		u32 header_1 = ReadMem32(addr);
		u32 header_2 = ReadMem32(addr + 4) &0x1FFFFFE0;

		last = (header_1 >> 31) == 1;//is last transfer ?
		u32 plen = (header_1 & 0xFF )+1;//transfer lenght
		u32 device = (header_1 >> 16) & 0x3;
		
		if (MaplePlugin[device][0].Connected)
		{
			u32* p_data =(u32*) malloc((int)plen*sizeof(u32));

			for (u32 i=0;i<plen;i++)
			{
				p_data[i] = ReadMem32(addr + 8 + (i << 2));
			}

			u32 recv=(p_data[0] >> 8) & 0xFF;//0-5;
			if (recv==0x20)
			{
				MaplePlugin[device][0].GotDataCB(header_1,header_2,p_data,plen);
			}
			else
			{ 
				//FIXME -> must be tested ;)
				u32 subdevice=0;
				while((recv&(1<<subdevice))==0)
					subdevice++;

				if (MaplePlugin[device][subdevice].Connected)
					MaplePlugin[device][subdevice].GotDataCB(header_1,header_2,p_data,plen);
				else
					WriteMem32(header_2, 0xFFFFFFFF);//not conected

			}
			free(p_data);
		}
		else
		{
			WriteMem32(header_2, 0xFFFFFFFF);//not conected
		}
		//goto next command
		addr += 2 * 4 + plen * 4;
	}
	SB_MDST = 0;	//MAPLE_STATE = 0;//finished :P
	RaiseInterrupt(holly_MAPLE_DMA);
}

void maple_Init()
{
	sb_regs[(SB_MDST_addr-SB_BASE)>>2].flags=REG_32BIT_READWRITE | REG_READ_DATA;
	sb_regs[(SB_MDST_addr-SB_BASE)>>2].writeFunction=maple_SB_MDST_Write;
	InitMaple();
}

void maple_Reset(bool Manual)
{
}

void maple_Term()
{
}