#include "rec_v1_basicblock.h"
#include "emitter/shil_compile_slow.h"
#include "dc/mem/sh4_mem.h"
#include "emitter/emitter.h"

extern rec_v1_BasicBlock* rec_v1_pCurrentBlock;

void rec_v1_BasicBlock::AddRef(rec_v1_BasicBlock* bb)
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

void rec_v1_BasicBlock::Discard()
{
	//printf("Discard block %x\n",start);
	u32 ref_count=(u32)callers.size();
	for (u32 i=0;i<ref_count;i++)
	{
		if (callers[i]!=0)
		{
			rec_v1_BasicBlock* cal=callers[i];
			if (cal->TF_block==this)
			{
				cal->TF_block=0;
				cal->pTF_next_addr= link_compile_inject_TF_stub;
			}

			if (cal->TT_block==this)
			{
				cal->TT_block=0;
				cal->pTT_next_addr= link_compile_inject_TT_stub;
			}
		}
	}
	if (TT_block)
	{
		u32 cal_ls=(u32)TT_block->callers.size();
		for (u32 j=0;j<cal_ls;j++)
		{
			if (TT_block->callers[j]==this)
				TT_block->callers[j]=0;
		}
	}

	if (TF_block)
	{
		u32 cal_ls=(u32)TF_block->callers.size();
		for (u32 j=0;j<cal_ls;j++)
		{
			if (TF_block->callers[j]==this)
				TF_block->callers[j]=0;
		}
	}

	/*emitter<> *x86e = new emitter<>((u8*)compiled->Code);
	x86e->Pad(compiled->size);*/
	//if (rec_v1_pCurrentBlock!=this)
	//if (start==0x8c0000e8)
	//	start=0x8c0000e8;

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