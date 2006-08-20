#include "ta.h"
//Tile Accelerator state machine
#include "ta_alloc.h"

namespace TASplitter
{
	//TA fifo state variables
	//Current List
	u32 CurrentList=ListType_None; 
	//Splitter function (normaly ta_dma_main , modified for split dma's)
	TaListFP* TaCmd;
	//Vertex Handler function :)
	TaListFP* VerxexDataFP=0;
	//finished lists
	bool ListIsFinished[5]={false,false,false,false,false};

	//splitter function lookup
	TaListFP* ta_poly_data_lut[15];

	//true if strip started :p
	bool StripStarted=false;

	//DMA from emulator :)
	void Dma(u32 address,u32* data,u32 size)
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