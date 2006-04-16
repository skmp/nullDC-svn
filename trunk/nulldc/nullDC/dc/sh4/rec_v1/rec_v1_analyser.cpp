#include "rec_v1_analyser.h"
#include "sh4_cpu_shil.h"

#include "dc\mem\sh4_mem.h"

//will alayse code and convert it to shil
//the basicblock (and suprtblock later) will be the send to the optimiser , and after that
//to the compiler
void rec_v1_AnalyseCode(u32 start,rec_v1_BasicBlock* to)
{
	u32 pc=start;
	u32 block_size=0;

	shil_DynarecInit();
	ilst=new shil_stream();
	while (block_size<448)
	{
		block_size++;
		u16 opcode=ReadMem16(pc);
		RecOpPtr[opcode](opcode,pc);
		if (RecOpPtr[opcode]==rec_shil_icpu_nimp)
		{
			//some system or misc opcode , block must end...
			ilst->shil_ifb(opcode,pc);//fallback to interpreter for now
			break;
		}
		if (block_size==448)
		{
			//end ! :P
			break;
		}
		pc+=2;
	}

	to->end=pc;
}