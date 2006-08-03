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
//the basicblock (and superblock later) will be the send to the optimiser , and after that
//to the compiler

#define CPU_RATIO 1
#define CPU_BASIC_BLOCK_SIZE (BLOCKLIST_MAX_CYCLES/2)
u32 execution_groop_busy[sh4_eu::sh4_eu_max];
u32 known_pl_cycles=0;

void InitPipeline()
{
	known_pl_cycles=0;
	memset(execution_groop_busy,0,sizeof(execution_groop_busy));
}
void plSubCycles(u32 cycles)
{
	for (int i=0;i<sh4_eu::sh4_eu_max;i++)
	{
		if (execution_groop_busy[i]>0)
		{
			execution_groop_busy[i]-=cycles;
			if (execution_groop_busy[i]<0)
				execution_groop_busy[i]=0;

		}
	}
}
void StepPipeline(u32 opcode)
{
	s32 rv;
	if (OpDesc[opcode])
	{
		rv= OpDesc[opcode]->IssueCycles;
		if (rv==0)
			rv=OpDesc[opcode]->LatencyCycles;

		s32 lc=OpDesc[opcode]->LatencyCycles;
		lc-=rv;//if it has latency>issue
		
		if (lc>0)
			execution_groop_busy[OpDesc[opcode]->unit]+=lc;
		//we allways count issue cycles
		//latecny cycles are counted @ the end :)
		known_pl_cycles+=rv;
	}
	else
	{
		rv= 10;
	}
	//if (rv<2)
	//	rv=2;
	//return rv;
}
void TermPipeline()
{
	u32 mpc=0;
	for (int i=0;i<sh4_eu::sh4_eu_max;i++)
	{
		//if (execution_groop_busy[i]>mpc)
		{
			mpc+=execution_groop_busy[i];
		}
	}
	known_pl_cycles+=mpc;
}
void rec_v1_AnalyseCode(u32 start,BasicBlock* to)
{

	u32 pc=start;

	//u32 block_size=0;
	u32 block_ops=0;

	shil_DynarecInit();

	ilst=&to->ilst;
	to->flags.FpuMode = GET_CURRENT_FPU_MODE();

	InitPipeline();

	while (true)
	{
		u32 opcode=ReadMem16(pc);
		//block_size+=GetOpCycles(opcode);
		block_ops++;
		StepPipeline(opcode);
		/*if (((pc>>26)&0x7)==3)
			rec_v1_SetBlockTest(pc);*/

		/*if ((opcode&0xF000)==0xF000)
			ilst->shil_ifb(opcode,pc);
		else*/
		//u32 nop=ilst->opcodes.size();
			RecOpPtr[opcode](opcode,pc,to);

		/*if (ilst->opcodes.size()>nop)
			ilst->opcodes[nop]|=FLAG_NEXT_OP;*/

		if (to->flags.SynthOpcode)
		{
			switch (to->flags.SynthOpcode )
			{
			case BLOCK_SOM_SIZE_128:
				printf("Syth opcode found at pc 0x%X , bytelen = 128+2 , skiping 130 bytes\n",pc);
				pc+=128;
				block_ops+=128>>1;
				known_pl_cycles+=128>>1;
				break;
			
			case BLOCK_SOM_RESERVED1:
			case BLOCK_SOM_RESERVED2:
				break;
			}
			to->flags.SynthOpcode=BLOCK_SOM_NONE;
		}

		if (to->flags.EndAnalyse)
		{
			to->end=pc;
			break;
		}

		//if branch , then block end
		if (OpTyp[opcode]&WritesPC)
		{
			to->end=pc;
			to->flags.ExitType =BLOCK_EXITTYPE_DYNAMIC;
			break;
		}

		if ((OpTyp[opcode]&(WritesSR | WritesFPSCR)))
		{
			//block end , but b/c last opcode does not set PC
			//after execution , resume to recompile from pc+2
			//ilst->mov(reg_pc,pc);//save next opcode pc-2 , pc+2 is done after execution
			//opcode is interpreter so pc is set , if update shit() is called , pc must remain

			to->end=pc;
			to->flags.ExitType =BLOCK_EXITTYPE_DYNAMIC;
			ilst->add(reg_pc,2);
			break;
		}

		if (block_ops>=(CPU_BASIC_BLOCK_SIZE))
		{
			ilst->mov(reg_pc,pc);//save next opcode pc-2 , pc+2 is done after execution
			
			to->flags.ExitType=BLOCK_EXITTYPE_FIXED;
			to->end=pc;
			to->TF_next_addr=pc+2;
			break;
		}
 
		pc+=2;
	}

	//clear flags that are used olny for analysis
	to->flags.EndAnalyse = false;
	
	//add delayslot opcode :)
	if (to->flags.HasDelaySlot)
	{
		to->end+=2;
		u32 opcode=ReadMem16(to->end);
		//block_size+=GetOpCycles(opcode);
		StepPipeline(opcode);
		block_ops++;
	}

#ifdef PROFILE_DYNAREC
	if( (to->flags & BLOCK_TYPE_MASK)==BLOCK_TYPE_DYNAMIC)
	{
		char temp[1000];
		DissasembleOpcode(ReadMem16(to->end),to->end,temp);
		printf("Dynamic block ending at %s\n",temp);
	}
#endif
	TermPipeline();
	to->cycles=known_pl_cycles*CPU_RATIO;

	to->ilst.op_count=to->ilst.opcodes.size();
	//shil_opt_return srv;
//	perform_shil_opt(shil_opt_ntc,to,srv);

	/*printf("Block done %f avg cycl/op , %d total cycles\n",(float)to->cycles/(float)block_ops,to->cycles);
	if (((float)to->cycles/(float)block_ops)<1)
		__asm int 3;*/
}