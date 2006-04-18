#include "rec_v1_analyser.h"
#include "sh4_cpu_shil.h"

#include "dc\mem\sh4_mem.h"

//w/ a block size of 256 we can have max 7 levels of basic blocks (heh)
//w/ a block size of 448 (bad number for inlinings , but good for vsyncs) we can have 13-14
//w/ a block size of 512 (prop i will use this) we can have 15

//target for analyser :
//stop blocks olny on unkown jumps [jump to reg]
//handle delayslots

//will alayse code and convert it to shil
//the basicblock (and suprtblock later) will be the send to the optimiser , and after that
//to the compiler
void rec_v1_AnalyseCode(u32 start,rec_v1_BasicBlock* to)
{
	u32 pc=start;
	u32 block_size=0;

	shil_DynarecInit();
	ilst=&to->ilst;
	while (block_size<448)
	{
		block_size++;
		u16 opcode=ReadMem16(pc);
		RecOpPtr[opcode](opcode,pc);

		if (OpTyp[opcode]&WritesPC)
		{
			//sh4
			//an opcode that writes to PC (branch)
			//call interpreter , it will execute delayslot opcode too
			ilst->shil_ifb(opcode,pc);//fallback to interpreter for now

			to->end=pc;
			break;
		}
		
		pc+=2;

		if ((OpTyp[opcode]&(WritesSR | WritesFPSCR)) || block_size==448)
		{
			//block end , but b/c last opcode does not set PC
			//after execution , resume to recompile from pc+2
			ilst->mov(reg_pc,pc);//save next opcode pc

			to->end=pc;
			break;
		}
		
	}
}