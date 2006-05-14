#include "dc\sh4\sh4_registers.h"
#include "dc\mem\sh4_mem.h"
#include "rec_v1_blockmanager.h"

#include <vector>
using namespace std;

#define block_cnt (0x1000) 

vector<rec_v1_BasicBlock*> BlockLists[block_cnt];
rec_v1_BasicBlock*		   BlockListsCache[block_cnt];


#define DoHash /*(fpscr.PR_SZ*0x1000) |*/ (address>>3)&(block_cnt-1)
INLINE vector<rec_v1_BasicBlock*>* GetBlockList(u32 address)
{
	return &BlockLists[DoHash];
}

INLINE rec_v1_BasicBlock* GetBlockListCache(u32 address)
{
	return BlockListsCache[DoHash];
}
INLINE void SetBlockListCache(u32 address,rec_v1_BasicBlock* nb)
{
	BlockListsCache[DoHash]=nb;
}

rec_v1_BasicBlock* rec_v1_FindBlock(u32 address)
{
	rec_v1_BasicBlock* fast_block=GetBlockListCache(address);
	
	if (!((fast_block !=0) && (fast_block->start==address)&&( fast_block->cpu_mode_tag==fpscr.PR_SZ)))
	{
		vector<rec_v1_BasicBlock*>* blklist = GetBlockList(address);

		u32 listsz=(u32)blklist->size();
		for (u32 i=0;i<listsz;i++)
		{ 
			rec_v1_BasicBlock* thisblock=(*blklist)[i];
			if ((thisblock!=0) && (thisblock->start==address))
			{
				if (thisblock->cpu_mode_tag==fpscr.PR_SZ)
				{
					//thisblock->lookups++;
					if (fast_block==0 || fast_block->lookups<thisblock->lookups)
						SetBlockListCache(address,thisblock);
					return thisblock;
				}
			}
		}
	}
	else
	{
		fast_block->lookups++;
		return fast_block;
	}
	return 0;
}


rec_v1_BasicBlock* rec_v1_NewBlock(u32 address)
{
	rec_v1_BasicBlock* rv=new rec_v1_BasicBlock();
	rv->start=address;
	rv->cpu_mode_tag=fpscr.PR_SZ;

	return rv;
}

rec_v1_BasicBlock* rec_v1_AddBlock(u32 address)
{
	rec_v1_BasicBlock* rv=rec_v1_NewBlock(address);

	vector<rec_v1_BasicBlock*>* blklist = GetBlockList(address);

	u32 listsz=(u32)blklist->size();
	for (u32 i=0;i<listsz;i++)
	{
		if ((*blklist)[i]==0)
		{
			(*blklist)[i]=rv;
			return rv;
		}
	}


	blklist->push_back(rv);
	return rv;
}



//Do a block test
void __fastcall rec_v1_BlockTest(u32 addr)
{
	/*
	//u32 addr_real=addr;
	addr&=RAM_MASK;
	if (rec_v1_GetBlockTest(addr))
	{
		//damn a block is overwrited
		for (u32 base=addr-512;base <addr+512;base+=2)
		{
			vector<rec_v1_BasicBlock*>* blklist=GetBlockList(base);

			u32 listsz=(u32)blklist->size();
			if (listsz)
			{ 
				for (u32 j=0;j<listsz;j++)
				{
					rec_v1_BasicBlock* block= (*blklist)[j];
					if  ((block!=0) && block->Contains(addr))
					{
						block->Discard();
						delete block;
						if (GetBlockListCache(base)==block)
							SetBlockListCache(base,0);
						(*blklist)[j]=0;
						
					}
				}
			}
			//rec_v1_ResetBlockTest(addr);
		}
	}*/
}



bool RamLockedWrite(u8* address)
{


	u32 offset=address-mem_b.data;

	if ((offset<RAM_SIZE))
	{
		printf("Ram lock write xD xD w000tt\n");
		return true;
	}
	else
		return false;
}
