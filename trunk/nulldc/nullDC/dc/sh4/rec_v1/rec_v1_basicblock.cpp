#include "rec_v1_basicblock.h"
#include "emitter/shil_compile_slow.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_registers.h"
#include "emitter/emitter.h"

#include <memory.h>

extern rec_v1_BasicBlock* rec_v1_pCurrentBlock;
extern u32 call_ret_address;//holds teh return address of the previus call ;)
extern rec_v1_BasicBlock* pcall_ret_address;//holds teh return address of the previus call ;)

void rec_v1_BasicBlock::AddCaller(rec_v1_BasicBlock* bb)
{
	//printf("AddRef block %x\n",bb->start);

	u32 ref_count=(u32)callers.size();
	for (u32 i=0;i<ref_count;i++)
	{
		if (callers[i]==0)
		{
			callers[i]=bb;
			return ;
		}
	}

	callers.push_back(bb);
}

rec_v1_BasicBlock* rec_v1_BasicBlock::FindCallee(u32 address)
{
	u32 size=(u32)callees.size();
	for (u32 i=0;i<size;i++)
	{
		if (this->callees[i]!=0 &&
			this->callees[i]->start == address && 
			this->callees[i]->cpu_mode_tag==fpscr.PR_SZ)
		{			
			return this->callees[i];
		}
	}
	return 0;
}
void rec_v1_BasicBlock::AddCallee(rec_v1_BasicBlock* bb)
{
	u32 callees_count=(u32)callees.size();
	for (u32 i=0;i<callees_count;i++)
	{
		if (callees[i]==0)
		{
			callees[i]=bb;
			return ;
		}
	}

	callees.push_back(bb);
	bb->AddCaller(this);
}

void rec_v1_BasicBlock::RemoveCaller(rec_v1_BasicBlock* bb)
{
	u32 ref_count=(u32)callers.size();
	for (u32 i=0;i<ref_count;i++)
	{
		if (callers[i]==bb)
			callers[i]=0;
	}

	if (TF_block==bb)
	{
		TF_block=0;
		pTF_next_addr= link_compile_inject_TF_stub;
	}

	if (TT_block==bb)
	{
		TT_block=0;
		pTT_next_addr= link_compile_inject_TT_stub;
	}
}
void rec_v1_BasicBlock::RemoveCallee(rec_v1_BasicBlock* bb)
{
	bb->RemoveCaller(this);
	u32 callee_count=callees.size();
	
	for (u32 i=0;i<callee_count;i++)
	{
		if (callees[i]==bb)
			callees[i]=0;
	}

	if (TF_block==bb)
	{
		TF_block=0;
		pTF_next_addr= link_compile_inject_TF_stub;
	}

	if (TT_block==bb)
	{
		TT_block=0;
		pTT_next_addr= link_compile_inject_TT_stub;
	}
}
void rec_v1_BasicBlock::Suspend()
{
	//printf("Discard block %x\n",start);
	if (pcall_ret_address==this)
	{
		pcall_ret_address=0;
		call_ret_address=0xFFFFFFFF;
	}

	u32 callers_count=callers.size();
	for (u32 i=0;i<callers_count;i++)
	{
		RemoveCaller(callers[i]);
	}

	u32 callee_count=callees.size();
	for (u32 i=0;i<callee_count;i++)
	{
		RemoveCallee(callees[i]);
	}
	Discarded=true;
}

void rec_v1_BasicBlock::Free()
{
	//memset(this,0xFF,sizeof(rec_v1_BasicBlock));
	//return;
	if (rec_v1_pCurrentBlock==this)
	{
		printf("FUCK!!!!\n");
		return;
	}
	if (compiled)
	{
		free(compiled->Code);
		compiled->Code=0;
		delete compiled;
		compiled=0;
	}
	else
	{
		printf("FREE!!! 0x%x\n",start);
	}	
}
bool rec_v1_BasicBlock::Contains(u32 pc)
{
	u32 pc_real=pc & RAM_MASK;
	u32 real_start=start & RAM_MASK;
	u32 real_end=end & RAM_MASK;

	return ((pc_real>=(real_start-4)) && (pc_real<=(real_end+6)));
}