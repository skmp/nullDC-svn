#include "rec_v1_analyser.h"
#include "sh4_cpu_shil.h"
#include "rec_v1_blockmanager.h"

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
u32 nest_level=0;
#define MAX_NEST 5
void rec_v1_AnalyseCode(u32 start,rec_v1_BasicBlock* to,u32 cycles_before)
{
	nest_level++;

	u32 pc=start;
	u32 block_size=cycles_before;

	shil_DynarecInit();
	ilst=&to->ilst;
	while (block_size<448)
	{
		block_size++;
		u16 opcode=ReadMem16(pc);

		if (((pc>>26)&0x7)==3)
			rec_v1_SetBlockTest(pc);

		//if branch , then block end , force it to stop inlining
		if ((OpTyp[opcode]&WritesPC) && (nest_level==MAX_NEST))
		{
			ilst->shil_ifb(opcode,pc);
			to->end=pc;

			break;
		}

		if ((opcode&0xF000)==0xF000)
			ilst->shil_ifb(opcode,pc);
		else
			RecOpPtr[opcode](opcode,pc,to);
		
		//if branch , then block end
		if (OpTyp[opcode]&WritesPC)
		{
			to->end=pc;

			break;
		}

		if ((OpTyp[opcode]&(WritesSR | WritesFPSCR)))
		{
			//block end , but b/c last opcode does not set PC
			//after execution , resume to recompile from pc+2
			//ilst->mov(reg_pc,pc);//save next opcode pc-2 , pc+2 is done after execution
			//opcode is interpreter so pc is set , if update shit() is called , pc must remain

			to->end=pc;
			break;
		}

		if (block_size==512)
		{
			ilst->mov(reg_pc,pc);//save next opcode pc-2 , pc+2 is done after execution

			to->end=pc;
			break;
		}

		pc+=2;
		
	}

	to->cycles=(block_size-cycles_before)*2;

	if (to->TF_next_addr!=0xFFFFFFFF)
	{
		//printf("Analysing block chain [TF] : addr=0x%x : level %d\n",to->TF_next_addr,nest_level);
		rec_v1_BasicBlock* bb_b;//=rec_v1_FindBlock(to->TF_next_addr);
		
		//if(bb_b==0)
		{
			bb_b= rec_v1_NewBlock(to->TF_next_addr);
			bb_b->flags|=BLOCK_TEMP;
			rec_v1_AnalyseCode(to->TF_next_addr,bb_b,block_size);
		}
		//else
		//	printf("HIT !!!!!\n");

		to->TF_next=bb_b;
		//printf("Analysing block chain [TF] : addr=0x%x -> [done!] : level %d\n",to->TF_next_addr,nest_level);
	}

	if (to->TT_next_addr!=0xFFFFFFFF)
	{
		//printf("Analysing block chain [TT] : addr=0x%x : level %d\n",to->TT_next_addr,nest_level);
		rec_v1_BasicBlock* bb_b;//=rec_v1_FindBlock(to->TT_next_addr);
		
		//if(bb_b==0)
		{
			bb_b= rec_v1_NewBlock(to->TT_next_addr);
			bb_b->flags|=BLOCK_TEMP;
			rec_v1_AnalyseCode(to->TT_next_addr,bb_b,block_size);
		}
		//else 
		//	printf("HIT !!!!!\n");

		to->TT_next=bb_b;
		//printf("Analysing block chain [TT] : addr=0x%x -> [done!] : level %d\n",to->TT_next_addr,nest_level);
	}

	//printf("SH4: Analysed block pc:%x , block size : %d. Shil size %d , level = %d\n",to->start,block_size,to->ilst.op_count,nest_level);
	nest_level--;
}