#include "ta.h"
//Tile Accelerator state machine
#include <emmintrin.h>

#if TA_SQ_CACHE==OP_ON
#define _CACHE_SQ_
#endif

 //128*32=4 kb cache max
#ifdef _CACHE_SQ_
#define CACHE_SQ_SIZE 16
#endif
namespace TASplitter
{
	void FASTCALL Dma_(u32* data,u32 size);
	//Splitter function (normaly ta_dma_main , modified for split dma's)
	TaListFP* TaCmd;
	TaListFP** TaCmd2;

	u32 iterations=0,oldz=0;
	u32 calls=0;
#ifdef _CACHE_SQ_
	int ta_buffer_used=0;
	__declspec(align(16)) u8 ta_buffer[32*CACHE_SQ_SIZE+32*2];//1024 entry buffer (32kb)
	//cached dma
	__forceinline void flush_buffer()
	{
		//printf("%d\n",ta_buffer_used);
		Dma_((u32*)ta_buffer,ta_buffer_used);
		ta_buffer_used=0;
	}
	__forceinline void FASTCALL WriteBuffer32(u32* data)
	{
		__m128* src=(__m128*)data;
		__m128* dst=(__m128*)&ta_buffer[ta_buffer_used*32];
		Ta_Dma* srcp=(Ta_Dma*)data;
		//Ta_Dma* dst=(Ta_Dma*)&ta_buffer[ta_buffer_used*32];
		ta_buffer_used++;
		//bool _must_flush=false;
		//while(sz)
		//{
		*dst++=*src++;
		*dst++=*src++;
		if (srcp->pcw.ParaType == ParamType_End_Of_List)
		{
			flush_buffer();
		}
			//*dst++=*src++;
		
		//	sz--;
		//}
		
//		if (_must_flush)
//			flush_buffer();
	}
#endif
	void FASTCALL SQ(u32* data)
	{
#ifdef _CACHE_SQ_
		WriteBuffer32(data);
		if (ta_buffer_used>=CACHE_SQ_SIZE)
			flush_buffer();
#else
		verify(TaCmd!=0);
		Ta_Dma* t=(Ta_Dma*)data; 
		//mov eax,[ecx]
		//shl eax,27
		//and eax,~3
		//mov edx,1
		//add eax,[TaCmd2]
		//jmp eax;
		//TaCmd2[t->pcw.ParaType](t,1);
		TaCmd(t,1);
#endif
	}
	void FASTCALL Dma(u32* data,u32 size)
	{
		/*
		if (size!=1)
		{*/
		/*
		if (size==1)
		{
			SQ(data);
			return;
		}
		*/
#ifdef _CACHE_SQ_
		if (ta_buffer_used)
			flush_buffer();
#endif
		Dma_(data,size);
		/*}
		else
		{
			WriteBuffer32(data);
			if (ta_buffer_used>=BUFFER_SIZE)
				flush_buffer();
		}*/
	}
	//DMA from emulator :)
	void FASTCALL Dma_(u32* data,u32 size)
	{
		verify(TaCmd!=0);
		Ta_Dma* ta_data=(Ta_Dma*)data;
		//oldz+=size;
		//calls++;
		while (size)
		{
			u32 sz =TaCmd(ta_data,size);
			size-=sz;
			ta_data+=sz;
		}
		/*
		if (oldz>300000)
		{
			printf("%d size  %d calls, %f ratio\n",oldz,calls,(float)oldz/(float)calls);
			oldz=0;calls=0;
		}*/
		
	}

	void TA_ListCont()
	{
#ifdef _CACHE_SQ_
		if (ta_buffer_used)
			flush_buffer();
#endif
	}
	void TA_ListInit()
	{
#ifdef _CACHE_SQ_
		if (ta_buffer_used)
			flush_buffer();
#endif
	}
	void TA_SoftReset()
	{
#ifdef _CACHE_SQ_
		if (ta_buffer_used)
			flush_buffer();
#endif
	}
}