#include "shil_compile_slow.h"
#include <assert.h>
#include "emitter.h"

#include "dc\sh4\sh4_registers.h"
#include "dc\sh4\rec_v1\rec_v1_blockmanager.h"
#include "dc\sh4\rec_v1\nullprof.h"
#include "dc\sh4\sh4_opcode_list.h"
#include "dc\mem\sh4_mem.h"
#include "regalloc\x86_sseregalloc.h"



void CompileBasicBlock_slow_c(rec_v1_BasicBlock* block)
{
	FILE* to;
	char temp[512];
	sprintf(temp,"c:\\shilz\\0x%X_block_shil.c",block->start);
	to=fopen(temp,"w");

	for (u32 rpc=block->start;rpc<=block->end;rpc+=2)
	{
		DissasembleOpcode(ReadMem16(rpc),rpc,temp);
		fprintf(to,"sh4 : 0x%X : %s \n",rpc,temp);
	}

u32 list_sz=(u32)block->ilst.opcodes.size();
	fprintf(to,"//Block size : %d\n",list_sz);

	//x86e->ADD32ItoM(&rec_cycles,block->cycles);
	fprintf(to,"//Block Cycles :%d\n",block->cycles);
	
	
	
	for (u32 i=0;i<list_sz;i++)
	{
		shil_opcode* op=&block->ilst.opcodes[i];
		
		fprintf(to,"%s",GetShilName((shil_opcodes)op->opcode));
		if ((op->flags & 3)== FLAG_8)
			fprintf(to,"8");
		else if  ((op->flags & 3)== FLAG_16)
			fprintf(to,"16");
		else if  ((op->flags & 3)== FLAG_32)
			fprintf(to,"32");
		else if  ((op->flags & 3)== FLAG_64)
			fprintf(to,"64");

		if (op->flags & FLAG_SX)
			fprintf(to,"sx ");
		else 
			fprintf(to,"zx ");

		if (op->flags & FLAG_R0)
			fprintf(to,",reg_0 [f] ");

		if (op->flags & FLAG_GBR)
			fprintf(to,",reg_GBR [f] ");

		if (op->flags & FLAG_MACH)
			fprintf(to,",reg_MACH [f] ");

		if (op->flags & FLAG_MACL)
			fprintf(to,",reg_MACL [f] ");

		if (op->flags & FLAG_REG1)
			fprintf(to,",reg_%d ",op->reg1);
		if (op->flags & FLAG_REG2)
			fprintf(to,",reg_%d ",op->reg2);
		if (op->flags & FLAG_IMM1)
			fprintf(to,",0x%X ",op->imm1);
		if (op->flags & FLAG_IMM2)
			fprintf(to,",0x%X",op->imm2);
		fprintf(to,"\n");
	}

	//end block acording to block type :)
	switch(block->flags.ExitType)
	{
	
	case BLOCK_EXITTYPE_DYNAMIC_CALL:
	case BLOCK_EXITTYPE_DYNAMIC:
		{
			fprintf(to,"//BLOCK_TYPE_DYNAMIC[CALL] \n");
			//x86e->RET();
			break;
		}

	case BLOCK_EXITTYPE_RET:
		{
			fprintf(to,"//BLOCK_TYPE_RET \n");
			
			break;
		}

	case BLOCK_EXITTYPE_COND_0:
	case BLOCK_EXITTYPE_COND_1:
		{
			//ok , handle COND_0/COND_1 here :)
			//mem address
			u32* TT_a=&block->TT_next_addr;
			u32* TF_a=&block->TF_next_addr;
			//functions
			u32* pTF_f=(u32*)&(block->pTF_next_addr);
			u32* pTT_f=(u32*)&(block->pTT_next_addr);
			
			if (block->flags.ExitType==BLOCK_EXITTYPE_COND_0)
			{
				TT_a=&block->TF_next_addr;
				TF_a=&block->TT_next_addr;
				pTF_f=(u32*)&(block->pTT_next_addr);
				pTT_f=(u32*)&(block->pTF_next_addr);
			}

			{
				//If our cycle count is expired
				//save the dest address to pc

				//x86e->MOV32MtoR(EAX,&T_jcond_value);
				//x86e->TEST32ItoR(EAX,1);//test for T
				//see witch pc to set

				//x86e->MOV32ItoR(EAX,*TF_a);//==
				//!=
				//x86e->CMOVNE32MtoR(EAX,TT_a);//!=
				//x86e->MOV32RtoM(GetRegPtr(reg_pc),EAX);

				//save exit block 
//				x86e->MOV32ItoM((u32*)&pExitBlock,(u32)block);
				//x86e->RET();//return to caller to check for interrupts
				fprintf(to,"//BLOCK_TYPE_COND_0/1 \n");
				fprintf(to,"if (T_jcond_value==0) \n{ pc=0x%x}\n",*TF_a);
				fprintf(to,"else \n{ pc=0x%x}\n",*TT_a);
			}
		} 
		break;

	case BLOCK_EXITTYPE_FIXED_CALL:
		//mov guess,pr
	case BLOCK_EXITTYPE_FIXED:
		{

			//If our cycle count is expired
			//x86e->MOV32ItoM(GetRegPtr(reg_pc),block->TF_next_addr);
			//save exit block 
//			x86e->MOV32ItoM((u32*)&pExitBlock,(u32)block);
			//x86e->RET();//return to caller to check for interrupts
			fprintf(to,"//BLOCK_TYPE_FIXED[CALL] \n");
			fprintf(to,"pc=0x%x\n",block->TF_next_addr);
		}
	}
	fclose(to);
}