#include "basicblock.h"
#include "dc/sh4/shil/Compiler/shil_compiler_base.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_registers.h"
#include "emitter/emitter.h"
#include "emitter/regalloc/x86_sseregalloc.h"
#include <memory.h>
#include "recompiler.h"
#include "dc/sh4/sh4_interpreter.h"

#include "dc/sh4/rec_v1/blockmanager.h"

int compiled_basicblock_count=0;

//needed declarations
void bb_link_compile_inject_TF_stub(CompiledBlockInfo* ptr);
void bb_link_compile_inject_TT_stub(CompiledBlockInfo* ptr);
void RewriteBasicBlockCond(CompiledBasicBlock* cBB);

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
void FASTCALL RewriteBasicBlockGuess_NULL(CompiledBasicBlock* cBB);
void __fastcall basic_block_ClearBlock(CompiledBlockInfo* p_this,CompiledBlockInfo* block)
{
	bbthis;
	if (pthis->TF_block==block)
	{
		pthis->TF_block=0;
		pthis->pTF_next_addr=bb_link_compile_inject_TF_stub;
		if (p_this->block_type.exit_type==BLOCK_EXITTYPE_DYNAMIC ||
			p_this->block_type.exit_type==BLOCK_EXITTYPE_DYNAMIC_CALL)
		{
			pthis->TF_next_addr=0xFFFFFFFF;
			RewriteBasicBlockGuess_NULL((CompiledBasicBlock*)p_this);
		}
		if (pthis->RewriteType)
			RewriteBasicBlockCond((CompiledBasicBlock*)p_this);
	}

	if (pthis->TT_block==block)
	{
		pthis->TT_block=0;
		pthis->pTT_next_addr=bb_link_compile_inject_TT_stub;
		if (pthis->RewriteType)
			RewriteBasicBlockCond((CompiledBasicBlock*)p_this);
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
	
	cBB->cbi.block_type.ProtectionType=flags.ProtectionType;

	cBB->cbi.start=start;
	cBB->cbi.end=end;
	cBB->cbi.cpu_mode_tag=flags.FpuMode;
	cBB->cbi.lookups=0;
	cBB->cbi.Discarded=false;
	cBB->cbi.tbp_ticks=0;

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

void RewriteBasicBlockFixed(CompiledBasicBlock* cBB)
{
	u8 flags=0;
	if  (cBB->ebi.TF_block)
		flags=1;

	if (cBB->ebi.LastRewrite==flags)
		return;

	x86_block* x86e = new x86_block();

	x86e->Init();
	x86e->do_realloc=false;
	x86e->x86_buff=(u8*)cBB->cbi.Code + cBB->ebi.RewriteOffset;
	x86e->x86_size=32;

	cBB->ebi.LastRewrite=flags;

	if  (cBB->ebi.TF_block)
	{
		x86e->Emit(op_jmp,x86_ptr_imm(cBB->ebi.TF_block->Code));
	}
	else
	{
		x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
		x86e->Emit(op_jmp,x86_ptr_imm(bb_link_compile_inject_TF_stub));
	}
	x86e->Generate();

	delete x86e;
}
void RewriteBasicBlockCond(CompiledBasicBlock* cBB)
{
	if (cBB->ebi.RewriteType==2)
	{
		RewriteBasicBlockFixed(cBB);
		return;
	}

	u8 flags=0;
	if (cBB->ebi.TT_block!=0)
		flags|=1;
	if (cBB->ebi.TF_block)
		flags|=2;

	if (cBB->ebi.LastRewrite==flags)
		return;

	x86_block* x86e = new x86_block();
	
	x86e->Init();
	x86e->do_realloc=false;
	x86e->x86_buff=(u8*)cBB->cbi.Code + cBB->ebi.RewriteOffset;
	x86e->x86_size=32;

	cBB->ebi.LastRewrite=flags;

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
		//x86e->Emit(op_int3);
		x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
		x86e->Emit(op_je,x86_ptr_imm(bb_link_compile_inject_TF_stub));
		x86e->Emit(op_jmp,x86_ptr_imm(bb_link_compile_inject_TT_stub));
	}
	x86e->Generate();

	delete x86e;
}

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
		if (ptr->GetBB()->RewriteType)
			RewriteBasicBlockCond((CompiledBasicBlock*)ptr);
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
		if (ptr->GetBB()->RewriteType)
			RewriteBasicBlockCond((CompiledBasicBlock*)ptr);
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
void __fastcall CheckBlock(CompiledBasicBlock* block)
{
	verify(block->cbi.cpu_mode_tag==fpscr.PR_SZ);
	//verify(block->cbi.size==pc);
	verify(block->cbi.Discarded==false);
}
#ifdef COUNT_BLOCK_LOCKTYPE_USAGE
u32 manbs=0;
u32 lockbs=0;
void printfBBSS()
{
	printf ("Manual - %d : locked - %d : ratio(l:m) %f\n",manbs,lockbs,(float)lockbs/(float)manbs);
	manbs=0;
	lockbs=0;
}
#else
void printfBBSS() {}
#endif
extern u32 fast_lookups;


//eax == pc
//esi == cBB
void naked FASTCALL Resolve_FLUT(u32 pc,CompiledBasicBlock* cBB)
{
	//wrong optimisation; code left in for ideas
	//problem : Adding/removing links is too slow , slower than looking up the main block cache table
	__asm
	{
		mov ecx,eax;					//store a copy of pc on ecx
		and eax,(LOOKUP_HASH_MASK<<2);	//resolve eax to fast block

		mov eax,[BlockLookupGuess + eax]
	
		cmp [eax],ecx;		//if no fast block , call full lookup
		jne full_lookup;

		inc dword ptr[eax+16];	//block lookup count
#ifdef _BM_CACHE_STATS
		inc fast_lookups;
#endif
finaly:
		mov edx,[eax+8];	//read jump destination
		mov [esi+8 +0x24],eax;	//store block (TF_block)
		mov [esi+0 +0x24],ecx;	//store pc    (TF_address)
		mov [esi+16 +0x24],edx;	//store jump destination (pTF_address)
		jmp edx;			//jump to it :)

full_lookup:
		mov edx,eax;			//fastblock ptr is on edx
		mov edi,ecx;//store pc
		call FindBlock_full_compile;//ecx=pc , edx=fast block ptr
									//on return , eax=correct block ptr :).If not found its compiled
		mov ecx,edi;//restore pc
		jmp finaly;
	}
}
void FASTCALL RewriteBasicBlockGuess_FLUT(CompiledBasicBlock* cBB)
{
	//indirect call , rewrite & link , second time(does fast look up)
	x86_block* x86e = new x86_block();

	x86e->Init();
	x86e->do_realloc=false;
	x86e->x86_buff=(u8*)cBB->cbi.Code + cBB->ebi.RewriteOffset;
	x86e->x86_size=64;

	
	x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_no_update));
	
	/*
	//x86e->Emit(op_int3);
	x86e->Emit(op_cmp32,EAX,&cBB->ebi.TF_next_addr);
	x86e->Emit(op_mov32,ESI,(u32)cBB);
	x86e->Emit(op_jne,x86_ptr_imm(Resolve_FLUT));
	//x86e->Emit(op_int3);
	x86e->Emit(op_jmp32,x86_ptr(&cBB->ebi.pTF_next_addr));
	*/
	/*
	//mov ecx,pc;
	x86e->Emit(op_mov32,ECX,&pc);
	//mov edx,ecx;
	x86e->Emit(op_mov32,EDX,ECX);
	//and edx,(LOOKUP_HASH_MASK<<2);
	x86e->Emit(op_and32,EDX,(LOOKUP_HASH_MASK<<2));

	//mov edx,[BlockLookupGuess + edx]
	x86e->Emit(op_mov32,EDX,x86_mrm::create(EDX,BlockLookupGuess));

	//cmp [edx],ecx;
	x86e->Emit(op_cmp32,x86_mrm::create(EDX),ECX);
	//jne full_lookup;
	x86e->Emit(op_jne,x86_ptr_imm(Dynarec_Mainloop_no_update_fast));

	//inc dword ptr[edx+16];
	x86e->Emit(op_inc32,x86_mrm::create(EDX,x86_ptr::create(16)));
	#ifdef _BM_CACHE_STATS
		//inc fast_lookups;
		x86e->Emit(op_inc32,x86_ptr(&fast_lookups));
	#endif
	//jmp dword ptr[edx+8];
	x86e->Emit(op_jmp32,x86_mrm::create(EDX,x86_ptr::create(8)));
*/
	x86e->Generate();
	delete x86e;
}
//can corrupt anything apart esp
void naked RewriteBasicBlockGuess_FLUT_stub(CompiledBasicBlock* ptr)
{
	__asm
	{
		/*mov esi,ecx;//store block ptr
		mov edi,eax;//store pc
		call RewriteBasicBlockGuess_FLUT;
		mov eax,edi;//store pc

		jmp Resolve_FLUT;
		*/
		call RewriteBasicBlockGuess_FLUT;
		jmp [Dynarec_Mainloop_no_update];
	}
}
void* FASTCALL RewriteBasicBlockGuess_TTG(CompiledBasicBlock* cBB)
{
	//indirect call , rewrite & link , first time (hardlinks to target)
	CompiledBlockInfo*	new_block=FindOrRecompileBlock(pc);

	if (cBB->cbi.Discarded)
	{
		return new_block->Code;
	}
	//Add reference so we can undo the chain later
	new_block->AddRef(&cBB->cbi);
	cBB->ebi.TF_block=new_block;

	x86_block* x86e = new x86_block();

	x86e->Init();
	x86e->do_realloc=false;
	x86e->x86_buff=(u8*)cBB->cbi.Code + cBB->ebi.RewriteOffset;
	x86e->x86_size=64;

	
	cBB->ebi.TF_block=new_block;
	x86e->Emit(op_cmp32,EAX,pc);
	x86e->Emit(op_mov32,ECX,(u32)cBB);
	x86e->Emit(op_jne,x86_ptr_imm(RewriteBasicBlockGuess_FLUT_stub));
	x86e->Emit(op_jmp,x86_ptr_imm(new_block->Code));

	x86e->Generate();
	delete x86e;

	return new_block->Code;
}
void naked RewriteBasicBlockGuess_TTG_stub(CompiledBasicBlock* ptr)
{
	__asm
	{
		call RewriteBasicBlockGuess_TTG;
		jmp eax;
	}
}
//default behavior , calls _TTG rewrite
void FASTCALL RewriteBasicBlockGuess_NULL(CompiledBasicBlock* cBB)
{
	x86_block* x86e = new x86_block();

	x86e->Init();
	x86e->do_realloc=false;
	x86e->x86_buff=(u8*)cBB->cbi.Code + cBB->ebi.RewriteOffset;
	x86e->x86_size=32;
	x86e->Emit(op_mov32,ECX,(u32)cBB);
	x86e->Emit(op_jmp,x86_ptr_imm(RewriteBasicBlockGuess_TTG_stub));
	x86e->Generate();
	delete x86e;
}

void BasicBlock::Compile()
{
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

	x86_Label* block_exit = x86e->CreateLabel(false,0);

	/*
	x86e->Emit(op_mov32,ECX,(u32)cBB);
	x86e->Emit(op_call,x86_ptr_imm(CheckBlock));
	*/

	x86e->Emit(op_sub32 ,&rec_cycles,cycles);
	x86e->Emit(op_js,block_exit);

	if (flags.ProtectionType==BLOCK_PROTECTIONTYPE_MANUAL)
	{
#ifdef COUNT_BLOCK_LOCKTYPE_USAGE
		x86e->Emit(op_add32,&manbs,1);
#endif
		int sz=Size();
		verify(sz!=0);

		int i=0;
		//that can be optimised a lota :p
		
		x86_Label* exit_discard_block= x86e->CreateLabel(false,0);
		x86_Label* execute_block= x86e->CreateLabel(false,8);
		verify(sz!=0);
		while(sz>=4)
		{
			u32* pmem=(u32*)GetMemPtr(start+i,4);
			x86e->Emit(op_cmp32 ,pmem,*pmem);
			
			if (sz==4)
			{
				x86e->Emit(op_je ,execute_block);
			}
			else
			{
				x86e->Emit(op_jne ,exit_discard_block);
			}
			i+=4;
			sz-=4;
		}
		if (sz>=2)
		{
			//die("lol");
			u16* pmem=(u16*)GetMemPtr(start+i,2);
			x86e->Emit(op_cmp16 ,pmem,*pmem);
			
			x86e->Emit(op_je ,execute_block);

			i+=2;
			sz-=2;
		}
		verify(sz==0);

		x86e->MarkLabel(exit_discard_block);
		x86e->Emit(op_mov32,ECX,(u32)cBB);
		x86e->Emit(op_mov32,GetRegPtr(reg_pc),start);
		x86e->Emit(op_call,x86_ptr_imm(SuspendBlock));
		x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_no_update));
		x86e->MarkLabel(execute_block);
	}
#ifdef COUNT_BLOCK_LOCKTYPE_USAGE
	else
		x86e->Emit(op_add32,&lockbs,1);
#endif
	//verify(do_hs==false);
	
	if (do_hs)
	{
		//check for block promotion to superblock ;)
		x86e->Emit(op_dec32 ,x86_ptr(&cBB->cbi.GetHS()->bpm_ticks));
		
		x86_Label* not_zero=x86e->CreateLabel(false,8);
		x86e->Emit(op_jnz,not_zero);
		{
			//yay , 0 , see if it needs promotion kkthxdie
			x86e->Emit(op_mov32,EAX,&gcp_timer);//now
			x86e->Emit(op_sub32,EAX,&cBB->cbi.GetHS()->gcp_lasttimer);//now-last
			x86e->Emit(op_cmp32,EAX,16);
			//if it took more that 16 ticks , then its less that 10% , no promotion
			x86_Label* no_promote=x86e->CreateLabel(false,8);
			x86e->Emit(op_jbe,no_promote);
			{
				//suspend block
				x86e->Emit(op_mov32,ECX,(u32)cBB);
				x86e->Emit(op_call,x86_ptr_imm(SuspendBlock));
				void*  __fastcall CompileCode_SuperBlock(u32 pc);
				x86e->Emit(op_mov32,ECX,(u32)cBB->cbi.start);
				x86e->Emit(op_call,x86_ptr_imm(CompileCode_SuperBlock));
				x86e->Emit(op_jmp32,EAX);
			}
			x86e->MarkLabel(no_promote);
			x86e->Emit(op_add32,&cBB->cbi.GetHS()->gcp_lasttimer,EAX);//last+now-last=now ;)
			x86e->Emit(op_mov32,&cBB->cbi.GetHS()->bpm_ticks,3022);
		}
		x86e->MarkLabel(not_zero);

		//16 ticks or more to convert to zuper block
		//16 ticks -> 241760hrz /8 ~=30220 blocks
		//we promote to superblock if more that 20% of the time is spent on this block , 3022 ticks
		cBB->cbi.GetHS()->gcp_lasttimer=gcp_timer;
		cBB->cbi.GetHS()->bpm_ticks=3022*2;
	}
	
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
	cBB->ebi.RewriteType=0;
	cBB->ebi.LastRewrite=0xFF;
	cBB->cbi.block_type.exit_type=flags.ExitType;

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
			cBB->ebi.RewriteOffset=x86e->x86_indx;
			x86e->Emit(op_mov32,ECX,(u32)cBB);
			x86e->Emit(op_jmp,x86_ptr_imm(RewriteBasicBlockGuess_TTG_stub));
			u32 extrasz=26-(x86e->x86_indx-cBB->ebi.RewriteOffset);
			for (int i=0;i<extrasz;i++)
				x86e->write8(0xCC);
		}
		break;
	case BLOCK_EXITTYPE_RET:			//guess
		{
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
			u32 TT_a=cBB->ebi.TT_next_addr;
			u32 TF_a=cBB->ebi.TF_next_addr;
			

			x86e->Emit(op_cmp32,&T_jcond_value,1);

			if (TT_a==cBB->cbi.start)
			{
				cBB->ebi.TT_block=&cBB->cbi;
			}
			else
			{
				cBB->ebi.TT_block=FindBlock(TT_a);
				if (cBB->ebi.TT_block)
					cBB->ebi.TT_block->AddRef(&cBB->cbi);
			}

			if  (TF_a==cBB->cbi.start)
			{
				cBB->ebi.TF_block=&cBB->cbi;
			}
			else
			{
				cBB->ebi.TF_block=FindBlock(TF_a);
				if (cBB->ebi.TF_block)
					cBB->ebi.TF_block->AddRef(&cBB->cbi);
			}

			cBB->ebi.RewriteType=1;
			cBB->ebi.RewriteOffset=x86e->x86_indx;
			

			x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
			x86e->Emit(op_jne,x86_ptr_imm(0));
			x86e->Emit(op_jmp,x86_ptr_imm(0));
			x86e->Emit(op_int3);
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
			if (cBB->ebi.TF_next_addr==cBB->cbi.start)
			{
				printf("Fast Link possible\n");
			}
			else
			{
				cBB->ebi.TF_block=FindBlock(cBB->ebi.TF_next_addr);
				if (cBB->ebi.TF_block)
					cBB->ebi.TF_block->AddRef(&cBB->cbi);
			}

			cBB->ebi.RewriteType=2;
			cBB->ebi.RewriteOffset=x86e->x86_indx;
			//link to next block :
			x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , cBB
			x86e->Emit(op_jmp32,x86_ptr((u32*)&(cBB->ebi.pTF_next_addr)));	//mov eax , [pTF_next_addr]
		}
		break;
	case BLOCK_EXITTYPE_FIXED_CSC:		//forced lookup , possible state chainge
		{
			//We have to exit , as we gota do mode lookup :)
			//We also have to reset return cache to ensure its ok

			//call_ret_address=0xFFFFFFFF;
			x86e->Emit(op_mov32,&call_ret_address,0xFFFFFFFF);
			//pcall_ret_address=0;
			x86e->Emit(op_mov32,(u32*)&pcall_ret_address,0);
			//Good , now return to caller :)
			x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_no_update));
		}
		break;
	}

	ira->AfterTrail();
	fra->AfterTrail();

	x86e->MarkLabel(block_exit);
	x86e->Emit(op_add32 ,&rec_cycles,cycles);
	x86e->Emit(op_mov32 ,GetRegPtr(reg_pc),start);
	x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_do_update));

	x86e->Emit(op_int3);

	//apply roml patches and generate needed code
	apply_roml_patches();

	void* codeptr=x86e->Generate();//heh

	cBB->cbi.Code=(BasicBlockEP*)codeptr;
	cBB->cbi.size=x86e->x86_indx;
	dyna_link(&cBB->cbi);

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

	if (cBB->ebi.RewriteType)
		RewriteBasicBlockCond(cBB);
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