#include "basicblock.h"
#include "dc/sh4/shil/Compiler/shil_compiler_base.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_registers.h"
#include "emitter/emitter.h"
#include "emitter/regalloc/x86_sseregalloc.h"
#include <memory.h>
#include "recompiler.h"

#include "dc/sh4/rec_v1/blockmanager.h"

//needed declarations
void bb_link_compile_inject_TF_stub(CompiledBlockInfo* ptr);
void bb_link_compile_inject_TT_stub(CompiledBlockInfo* ptr);

//BasicBlock I/F and compiler :)

//Compiled basic block Common interface
#define bbthis	verify(p_this->block_type.type==COMPILED_BASIC_BLOCK);\
	CompiledBasicBlockInfo* pthis=&((CompiledBasicBlock*)p_this)->ebi;

void __fastcall basic_block_AddRef(CompiledBlockInfo* p_this,CompiledBlockInfo* block)
{
	bbthis;
	verify(p_this->Discarded==false);
	//if we reference ourselfs we dont care ;) were suspended anyhow
	if (block !=p_this)
		pthis->blocks_to_clear.push_back(block);
}
void __fastcall basic_block_BlockWasSuspended(CompiledBlockInfo* p_this,CompiledBlockInfo* block)
{
	bbthis;
	for (u32 i=0;i<pthis->blocks_to_clear.size();i++)
	{
		if (pthis->blocks_to_clear[i]==block)
		{
			pthis->blocks_to_clear[i]=0;
		}
	}
}
void __fastcall basic_block_ClearBlock(CompiledBlockInfo* p_this,CompiledBlockInfo* block)
{
	bbthis;
	if (pthis->TF_block==block)
	{
		pthis->TF_block=0;
		pthis->pTF_next_addr=bb_link_compile_inject_TF_stub;
	}

	if (pthis->TT_block==block)
	{
		pthis->TT_block=0;
		pthis->pTT_next_addr=bb_link_compile_inject_TT_stub;
	}
}
void __fastcall basic_block_Suspend(CompiledBlockInfo* p_this)
{
	bbthis;
	for (u32 i=0;i<pthis->blocks_to_clear.size();i++)
	{
		if (pthis->blocks_to_clear[i])
		{
			pthis->blocks_to_clear[i]->ClearBlock(p_this);
		}
	}
	pthis->blocks_to_clear.clear();

	if (pthis->TF_block)
		pthis->TF_block->BlockWasSuspended(p_this);

	if (pthis->TT_block)
		pthis->TT_block->BlockWasSuspended(p_this);
}
void __fastcall basic_block_Free(CompiledBlockInfo* p_this)
{
	bbthis;
}

//Basic Block
bool BasicBlock::IsMemLocked(u32 adr)
{
	if (IsOnRam(adr)==false)
		return false;

	if (flags.ProtectionType==BLOCK_PROTECTIONTYPE_MANUAL)
		return false;

	//if block isnt on ram , then there's no way to tell if its locked (well , bios mem is allways locked :p)
	if (OnRam()==false)
		return false;

	verify(page_start()<=page_end());

	u32 adrP=GetRamPageFromAddress(adr);

	return (page_start() <=adrP) && (adrP<=page_end());
}

void BasicBlock::SetCompiledBlockInfo(CompiledBasicBlock* cBl)
{
	verify(cBl->cbi.block_type.type==COMPILED_BASIC_BLOCK);
	cBB= cBl;

	cBB->cbi.start=start;
	cBB->cbi.end=end;
	cBB->cbi.cpu_mode_tag=flags.FpuMode;
	cBB->cbi.lookups=0;
	cBB->cbi.Discarded=false;

	if (cBl->cbi.block_type.nullProf)
		cBB->cbi.GetNP()->cycles=cycles;

	cBB->ebi.TF_next_addr=TF_next_addr;
	cBB->ebi.TT_next_addr=TT_next_addr;

	cBB->ebi.TF_block=cBB->ebi.TT_block=0;

	if (cBB->cbi.block_type.nullProf)
	{
		cBB->cbi.GetNP()->time=0;
		cBB->cbi.GetNP()->calls=0;
	}
}
//BasicBlock compiler :D

int compiled_basicblock_count=0;

//Compile block and return pointer to it's code
void* __fastcall bb_link_compile_inject_TF(CompiledBlockInfo* ptr)
{
	CompiledBlockInfo* target= FindOrRecompileBlock(ptr->GetBB()->TF_next_addr);

	//if current block is Discared , we must not add any chain info , just jump to the new one :)
	if (ptr->Discarded==false)
	{
		//Add reference so we can undo the chain later
		target->AddRef(ptr);
		ptr->GetBB()->TF_block=target;
		ptr->GetBB()->pTF_next_addr=target->Code;
	}
	return target->Code;
}

void* __fastcall bb_link_compile_inject_TT(CompiledBlockInfo* ptr)
{
	CompiledBlockInfo* target= FindOrRecompileBlock(ptr->GetBB()->TT_next_addr);

	//if current block is Discared , we must not add any chain info , just jump to the new one :)
	if (ptr->Discarded==false)
	{
		//Add reference so we can undo the chain later
		target->AddRef(ptr);
		ptr->GetBB()->TT_block=target;
		ptr->GetBB()->pTT_next_addr=target->Code;
	}
	return target->Code;
} 

//call link_compile_inject_TF , and jump to code
void naked bb_link_compile_inject_TF_stub(CompiledBlockInfo* ptr)
{
	__asm
	{
		call bb_link_compile_inject_TF;
		jmp eax;
	}
}

void naked bb_link_compile_inject_TT_stub(CompiledBlockInfo* ptr)
{
	__asm
	{
		call bb_link_compile_inject_TT;
		jmp eax;
	}
}

extern u32 rec_cycles;

u32 call_ret_address=0xFFFFFFFF;//holds teh return address of the previus call ;)
CompiledBlockInfo* pcall_ret_address=0;//holds teh return address of the previus call ;)
CompiledBasicBlock* Curr_block;

//sp is 0 if manual discard
void CBBs_BlockSuspended(CompiledBlockInfo* block,u32* sp)
{
	u32* sp_inblock=block_stack_pointer-1;

	if(sp_inblock==sp)
	{
		//printf("Exeption within the same block !\n");
	}
	else
	{
		if (sp!=0)
		{
			//printf("Exeption possibly within the same block ; 0x%X\n",sp_inblock[-1]);
			//printf("Block EP : 0x%X , sz : 0x%X\n",block->Code,block->size);
		}
	}
	if (pcall_ret_address == block)
	{
		call_ret_address=0xFFFFFFFF;
		pcall_ret_address=0;
	}
}
void __fastcall CheckBlock(CompiledBlockInfo* block)
{
	if (block->Discarded)
	{
		printf("Called a discarded block\n");
		__asm int 3;
	}
}
/*
void RewriteBasicBlockCond(CompiledBasicBlock* cBB)
{
	u32 flags=0;
	if (cBB->ebi.TT_block)
		flags|=1;
	if (cBB->ebi.TF_block)
		flags|=2;

	if (flags==1)
	{
		x86e->Emit(op_jne,x86_ptr_imm(cBB->ebi.TT_block->Code));
		x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
		x86e->Emit(op_jmp,x86_ptr_imm(bb_link_compile_inject_TF_stub));
	}
	else if  (flags==2)
	{
		x86e->Emit(op_je,x86_ptr_imm(cBB->ebi.TF_block->Code));
		x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
		x86e->Emit(op_jmp,x86_ptr_imm(bb_link_compile_inject_TT_stub));
	}
	else  if  (flags==3)
	{
		x86e->Emit(op_je,x86_ptr_imm(cBB->ebi.TF_block->Code));
		x86e->Emit(op_jmp,x86_ptr_imm(cBB->ebi.TT_block->Code));
	}
	else
	{
		x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
		x86e->Emit(op_je,x86_ptr_imm(bb_link_compile_inject_TF_stub));
		x86e->Emit(op_jmp,x86_ptr_imm(bb_link_compile_inject_TT_stub));
	}
}
void RewriteBasicBlockFixed(CompiledBasicBlock* cBB)
{
	if (cBB->ebi.TF_block)
	{
		x86e->Emit(op_jmp,x86_ptr_imm(cBB->ebi.TF_block->Code));
	}
	else
	{
		x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , cBB
		x86e->Emit(op_jmp,x86_ptr_imm((u32*)&(cBB->ebi.pTF_next_addr)));	//mov eax , [pTF_next_addr]
	}
}
*/
void BasicBlock::Compile()
{
	/*
	if (!inited)
	{
		Init();
		inited=true;
	}*/

	FloatRegAllocator*		fra;
	IntegerRegAllocator*	ira;

	x86_block* x86e=new x86_block();
	
	x86e->Init();

	flags.DisableHS=1;

	bool do_hs=(flags.ProtectionType!=BLOCK_PROTECTIONTYPE_MANUAL) && (flags.DisableHS==0) &&
		(flags.ExitType!=BLOCK_EXITTYPE_DYNAMIC) && (flags.ExitType!=BLOCK_EXITTYPE_DYNAMIC_CALL);

	u32 b_type=0;
	
	if (nullprof_enabled)
		b_type|=COMPILED_BLOCK_NULLPROF;
	
	if (do_hs)
		b_type|=COMPILED_BLOCK_HOTSPOT;
	
	b_type|=COMPILED_BASIC_BLOCK;

	cBB=(CompiledBasicBlock*)CreateBlock(b_type);

	SetCompiledBlockInfo(cBB);

	/*
	//that is a realy nice debug helper :)
	x86e->Emit(op_mov32,&Curr_block,(u32)cBB);
	*/
	x86_Label* block_start = x86e->CreateLabel(true,0);
	//x86e->MarkLabel(block_start);
	if (flags.ProtectionType==BLOCK_PROTECTIONTYPE_MANUAL)
	{
		int sz=end-start;
		//check at least 4 bytes
		sz=(sz +3) & (~3);

		if (sz<4)
			sz=1;
		else
			sz/=4;
		//sz++;
		int i=0;
		//that can be optimised a lota :p
		
		x86_Label* exit_discard_block= x86e->CreateLabel(false,0);
		x86_Label* execute_block= x86e->CreateLabel(false,0);

		for (i=0;i<sz;i++)
		{
			u32* pmem=(u32*)GetMemPtr(start+i*4,4);
			x86e->Emit(op_cmp32 ,GetMemPtr(start+i*4,4),*pmem);
			//u8* patch=x86e->JE8(0);
			//x86e->x86SetJ8(patch);
			if (i!=(sz-1))
			{
				x86e->Emit(op_jne ,exit_discard_block);
			}
			else
			{
				x86e->Emit(op_je ,execute_block);
			}
		}

		x86e->MarkLabel(exit_discard_block);
		x86e->Emit(op_mov32,ECX,(u32)cBB);
		x86e->Emit(op_mov32,GetRegPtr(reg_pc),start);
		x86e->Emit(op_call,x86_ptr_imm(SuspendBlock));
		x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_no_update));
		x86e->MarkLabel(execute_block);
	}

	verify(do_hs==false);
	/*
	if (do_hs)
	{
		//check for block promotion to superblock ;)
		x86e->Emit(op_dec32 ,&cBB->cbi.GetHS()->bpm_ticks);
		
		u8* not_zero=x86e->JNZ8(0);
		{
			//yay , 0 , see if it needs promotion kkthxdie
			x86e->Emit(op_mov32,EAX,&gcp_timer);//now
			x86e->SUB32MtoR(EAX,&cBB->cbi.GetHS()->gcp_lasttimer);//now-last
			x86e->CMP32ItoR(EAX,16);
			//if it took more that 16 ticks , then its less that 10% , no promotion
			u8*no_promote= x86e->JBE8(0);
			{
				//suspend block
				x86e->Emit(op_mov32,ECX,(u32)cBB);
				x86e->Emit(op_call,x86_ptr_imm(SuspendBlock));
				void*  __fastcall CompileCode_SuperBlock(u32 pc);
				x86e->Emit(op_mov32,ECX,(u32)cBB->cbi.start);
				x86e->Emit(op_call,x86_ptr_imm(CompileCode_SuperBlock));
				x86e->JMP32R(EAX);
			}
			x86e->x86SetJ8(no_promote);
			x86e->ADD32RtoM(&cBB->cbi.GetHS()->gcp_lasttimer,EAX);//last+now-last=now ;)
			x86e->Emit(op_mov32,&cBB->cbi.GetHS()->bpm_ticks,3022);
		}
		x86e->x86SetJ8(not_zero);

		//16 ticks or more to convert to zuper block
		//16 ticks -> 241760hrz /8 ~=30220 blocks
		//we promote to superblock if more that 20% of the time is spent on this block , 3022 ticks
		cBB->cbi.GetHS()->gcp_lasttimer=gcp_timer;
		cBB->cbi.GetHS()->bpm_ticks=3022*2;
	}
	*/
	
	//s8* start_ptr;
	
	/*
	if (nullprof_enabled)
	{
		//start_ptr=x86e->x86Ptr;
		x86e->Emit(op_call,x86_ptr_imm(dyna_profile_block_enter));
	}
	*/

	fra=GetFloatAllocator();
	ira=GetGPRtAllocator();
	
	ira->DoAllocation(this,x86e);
	fra->DoAllocation(this,x86e);

	ira->BeforeEmit();
	fra->BeforeEmit();

	
	shil_compiler_init(x86e,ira,fra);

	u32 list_sz=(u32)ilst.opcodes.size();
	for (u32 i=0;i<list_sz;i++)
	{
		shil_opcode* op=&ilst.opcodes[i];

		shil_compile(op);
	}

	ira->BeforeTrail();
	fra->BeforeTrail();
/*
	if (nullprof_enabled)
	{
		x86e->Emit(op_mov32,ECX,(u32)(cBB->cbi.GetNP()));
		x86e->Emit(op_call,x86_ptr_imm(dyna_profile_block_exit_BasicBlock));
	}
*/
	//end block acording to block type :)
	switch(flags.ExitType)
	{
	case BLOCK_EXITTYPE_DYNAMIC_CALL:	//same as below , sets call guess
		{
			//mov guess,pr
			x86e->Emit(op_mov32,&call_ret_address,cBB->ebi.TT_next_addr);
			//mov pguess,this
			x86e->Emit(op_mov32,(u32*)&pcall_ret_address,(u32)(cBB));
		}
	case BLOCK_EXITTYPE_DYNAMIC:		//not guess 
		{
			x86e->Emit(op_sub32 ,&rec_cycles,cycles);
			x86e->Emit(op_jns,x86_ptr_imm(Dynarec_Mainloop_no_update));
			x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_do_update));
		}
		break;
	case BLOCK_EXITTYPE_RET:			//guess
		{
			//x86_Label* not_ok=x86e->CreateLabel(false);

			//link end ?
			x86e->Emit(op_sub32 ,&rec_cycles,cycles);
			x86e->Emit(op_js,x86_ptr_imm(Dynarec_Mainloop_do_update));

			//cmp pr,guess
			x86e->Emit(op_mov32 ,EAX,GetRegPtr(reg_pc));
			x86e->Emit(op_cmp32 ,EAX,&call_ret_address);
			//je ok
			x86e->Emit(op_jne ,x86_ptr_imm(Dynarec_Mainloop_no_update));
			//ok:
			//mov ecx , pcall_ret_address
			x86e->Emit(op_mov32 ,ECX,(u32*)&pcall_ret_address);
			//mov eax,[pcall_ret_address+codeoffset]
			x86e->Emit(op_jmp32,x86_mrm::create(ECX,x86_ptr::create(offsetof(CompiledBasicBlock,ebi.pTT_next_addr))));
		}
		break;
	case BLOCK_EXITTYPE_COND:			//linkable
		{
			//ok , handle COND here :)
			//mem address
			u32* TF_a=&cBB->ebi.TT_next_addr;
			u32* TT_a=&cBB->ebi.TF_next_addr;
			
			//functions
			u32* pTF_f=(u32*)&(cBB->ebi.pTT_next_addr);
			u32* pTT_f=(u32*)&(cBB->ebi.pTF_next_addr);

			x86e->Emit(op_sub32 ,&rec_cycles,cycles);
			
			x86_Label* Exit_Link = x86e->CreateLabel(false,8);

			x86e->Emit(op_js ,Exit_Link);

			//Link:
			//if we can execute more blocks
			{
				x86e->Emit(op_test32,&T_jcond_value,1);
				if (*TT_a==cBB->cbi.start)
				{
					x86e->Emit(op_jne,block_start);
					x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
					x86e->Emit(op_jmp32,x86_ptr(pTF_f));
				}
				else if  (*TF_a==cBB->cbi.start)
				{
					x86e->Emit(op_je,block_start);
					x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
					x86e->Emit(op_jmp32,x86_ptr(pTT_f));
				}
				else
				{
					x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
					x86e->Emit(op_mov32,EAX,pTF_f);
					x86e->Emit(op_cmovne32 ,EAX,pTT_f);
					x86e->Emit(op_jmp32,EAX);
				}
				
			}
			
			//If our cycle count is expired
			x86e->MarkLabel(Exit_Link);
			{
				//save the dest address to pc
				x86e->Emit(op_test32,&T_jcond_value,1);
				//see witch pc to set

				x86e->Emit(op_mov32,EAX,*TF_a);//==
				//!=
				x86e->Emit(op_cmovne32 ,EAX,TT_a);//!=
				x86e->Emit(op_mov32,GetRegPtr(reg_pc),EAX);

				//return to caller to check for interrupts
				x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_do_update));
				//x86e->Emit(op_ret);
			}
		} 
		break;
	case BLOCK_EXITTYPE_FIXED_CALL:		//same as below
		{
			//mov guess,pr
			x86e->Emit(op_mov32,&call_ret_address,cBB->ebi.TT_next_addr);
			//mov pguess,this
			x86e->Emit(op_mov32,(u32*)&pcall_ret_address,(u32)(cBB));
		}
	case BLOCK_EXITTYPE_FIXED:			//linkable
		{
			//
			//x86e->Emit(op_cmp32 ,&rec_cycles,BLOCKLIST_MAX_CYCLES);

			x86e->Emit(op_sub32 ,&rec_cycles,cycles);
			x86_Label* No_Link = x86e->CreateLabel(false,8);

			x86e->Emit(op_js ,No_Link);

			//Link:
			//if we can execute more blocks
			if (cBB->ebi.TF_next_addr==cBB->cbi.start)
			{
				//__asm int 03;
				printf("Fast Link possible\n");
			}

			//link to next block :
			x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , cBB
			x86e->Emit(op_jmp32,x86_ptr((u32*)&(cBB->ebi.pTF_next_addr)));	//mov eax , [pTF_next_addr]
			//x86e->Emit(op_jmp32 ,EAX);									//jmp eax

			//If our cycle count is expired
			x86e->MarkLabel(No_Link);
			//save pc
			x86e->Emit(op_mov32,GetRegPtr(reg_pc),cBB->ebi.TF_next_addr);
			//and return to caller to check for interrupts
			x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_do_update));
		}
		break;
	case BLOCK_EXITTYPE_FIXED_CSC:		//forced lookup , possible state chainge
		{
			//We have to exit , as we gota do mode lookup :)
			//We also have to reset return cache to ensure its ok

			x86e->Emit(op_sub32 ,&rec_cycles,cycles);
			//call_ret_address=0xFFFFFFFF;
			x86e->Emit(op_mov32,&call_ret_address,0xFFFFFFFF);
			//pcall_ret_address=0;
			x86e->Emit(op_mov32,(u32*)&pcall_ret_address,0);
			//Good , now return to caller :)
			x86e->Emit(op_jns,x86_ptr_imm(Dynarec_Mainloop_no_update));
			x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_do_update));
		}
		break;
	}

	ira->AfterTrail();
	fra->AfterTrail();

	x86e->Emit(op_int3);
	void* codeptr=x86e->Generate(0,0);//heh

	cBB->cbi.Code=(BasicBlockEP*)codeptr;
	cBB->cbi.size=x86e->x86_indx;

	//make it call the stubs , unless otherwise needed
	cBB->ebi.pTF_next_addr=bb_link_compile_inject_TF_stub;
	cBB->ebi.pTT_next_addr=bb_link_compile_inject_TT_stub;
	//cBB->ebi
	compiled_basicblock_count++;
	
	/*
	if ((block_count%512)==128)
	{
		printf("Recompiled %d blocks\n",block_count);
		u32 rat=native>fallbacks?fallbacks:native;
		if (rat!=0)
			printf("Native/Fallback ratio : %d:%d [%d:%d]\n",native,fallbacks,native/rat,fallbacks/rat);
		else
			printf("Native/Fallback ratio : %d:%d [%d:%d]\n",native,fallbacks,native,fallbacks);
		printf("Average block size : %d opcodes ; ",(fallbacks+native)/block_count);
	}*/
	
	delete fra;
	delete ira;
	x86e->Free();
	delete x86e;
}

//
void BasicBlock::CalculateLockFlags()
{
	u32 addr=start;

	while(addr<end)
	{
		flags.ProtectionType |= GetPageInfo(addr).flags.ManualCheck;
		addr+=PAGE_SIZE;
	}
	//check the last one , it is possible to skip it on the above loop :)
	flags.ProtectionType |= GetPageInfo(end).flags.ManualCheck;
}