#include "rec_v1_analyser.h"
#include "sh4_cpu_shil.h"
#include "rec_v1_blockmanager.h"

#include "dc\mem\sh4_mem.h"
#include "dc\sh4\sh4_registers.h"

//w/ a block size of 256 we can have max 7 levels of basic blocks (heh)
//w/ a block size of 448 (bad number for inlinings , but good for vsyncs) we can have 13-14
//w/ a block size of 512 (prop i will use this) we can have 15

//target for analyser :
//stop blocks olny on unkown jumps [jump to reg]
//handle delayslots

//will alayse code and convert it to shil
//the basicblock (and suprtblock later) will be the send to the optimiser , and after that
//to the compiler

#define CPU_RATIO 2
#define CPU_BASIC_BLOCK_SIZE (448/2)
void rec_v1_AnalyseCode(u32 start,rec_v1_BasicBlock* to)
{

	u32 pc=start;

	u32 block_size=0;

	shil_DynarecInit();

	ilst=&to->ilst;
	to->flags|=GET_CURRENT_FPU_MODE();

	while (true)
	{
		block_size+=CPU_RATIO;
		u16 opcode=ReadMem16(pc);

		if (((pc>>26)&0x7)==3)
			rec_v1_SetBlockTest(pc);

		RecOpPtr[opcode](opcode,pc,to);
		
		if (to->flags & BLOCK_ATSC_END)
		{
			to->end=pc;
			break;
		}

		//if branch , then block end
		if (OpTyp[opcode]&WritesPC)
		{
			to->end=pc;
			to->flags |=BLOCK_TYPE_DYNAMIC;
			break;
		}

		if ((OpTyp[opcode]&(WritesSR | WritesFPSCR)))
		{
			//block end , but b/c last opcode does not set PC
			//after execution , resume to recompile from pc+2
			//ilst->mov(reg_pc,pc);//save next opcode pc-2 , pc+2 is done after execution
			//opcode is interpreter so pc is set , if update shit() is called , pc must remain

			to->end=pc;
			to->flags |=BLOCK_TYPE_DYNAMIC;
			ilst->add(reg_pc,2);
			break;
		}

		if (block_size>=CPU_BASIC_BLOCK_SIZE)
		{
			ilst->mov(reg_pc,pc);//save next opcode pc-2 , pc+2 is done after execution
			
			to->flags |=BLOCK_TYPE_FIXED;
			to->end=pc;
			to->TF_next_addr=pc+2;
			break;
		}

		pc+=2;
	}

	//clear flags that are used olny for analysis
	to->flags &= ~BLOCK_ATSC_END;

	to->cycles=block_size;

	//printf("SH4: Analysed block pc:%x , block size : %d. Shil size %d , level = %d\n",to->start,block_size,to->ilst.op_count,nest_level);
}