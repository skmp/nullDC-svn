#include "ta.h"
//Tile Accelerator state machine

namespace TASplitter
{
	//Splitter function (normaly ta_dma_main , modified for split dma's)
	TaListFP* TaCmd;

	//DMA from emulator :)
	void FASTCALL Dma(u32 address,u32* data,u32 size)
	{
		verify(TaCmd!=0);
		Ta_Dma* ta_data=(Ta_Dma*)data;
		while (size)
		{
			u32 sz =TaCmd(ta_data,size);
			
			size-=sz;
			ta_data+=sz;
		}
	}
}