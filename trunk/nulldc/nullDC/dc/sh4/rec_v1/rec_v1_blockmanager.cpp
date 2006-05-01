#include "dc\sh4\sh4_registers.h"
#include "dc\mem\sh4_mem.h"
#include "rec_v1_blockmanager.h"

#include <vector>
using namespace std;

#define block_cnt (0x1000) 
#define HASH_BITS 3
vector<rec_v1_BasicBlock*> BlockLists[block_cnt];
rec_v1_BasicBlock*		   BlockListsCache[block_cnt];

u8 BitTest[8]={1<<0,1<<1,1<<2,1<<3,1<<4,1<<5,1<<6,1<<7};
//u8 BitRes[8]={255-(1<<0),255-(1<<1),255-(1<<2),255-(1<<3),255-(1<<4),255-(1<<5),255-(1<<6),255-(1<<7)};
u8 RamTest[RAM_SIZE>>(HASH_BITS+3)];

INLINE vector<rec_v1_BasicBlock*>* GetBlockList(u32 address)
{
	return &BlockLists[(address>>3)&(block_cnt-1)];
}

INLINE rec_v1_BasicBlock* GetBlockListCache(u32 address)
{
	return BlockListsCache[(address>>3)&(block_cnt-1)];
}
INLINE void SetBlockListCache(u32 address,rec_v1_BasicBlock* nb)
{
	BlockListsCache[(address>>3)&(block_cnt-1)]=nb;
}

rec_v1_BasicBlock* rec_v1_FindBlock(u32 address)
{
	rec_v1_BasicBlock* fast_block=GetBlockListCache(address);
	
	if ((fast_block !=0) && (fast_block->start==address)&&( fast_block->cpu_mode_tag==fpscr.PR_SZ))
		return fast_block;
	else
	{

		vector<rec_v1_BasicBlock*>* blklist = GetBlockList(address);

		u32 listsz=blklist->size();
		for (u32 i=0;i<listsz;i++)
		{ 
			if (((*blklist)[i]!=0) && ((*blklist)[i]->start==address))
			{
				if ((*blklist)[i]->cpu_mode_tag==fpscr.PR_SZ)
				{
					SetBlockListCache(address,(*blklist)[i]);
					return (*blklist)[i];
				}
			}
		}
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

	u32 listsz=blklist->size();
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



void rec_v1_SetBlockTest(u32 addr)
{
	addr&=RAM_MASK;
	addr>>=HASH_BITS;//32 kb chunks , 
	RamTest[addr>>3]|=BitTest[addr&7];
}


void rec_v1_ResetBlockTest(u32 addr)
{
	addr&=RAM_MASK;
	addr>>=HASH_BITS;//32 kb chunks , 
	RamTest[addr>>3]&=~BitTest[addr&7];
}

int rec_v1_GetBlockTest(u32 addr)
{
	addr&=RAM_MASK;
	addr>>=HASH_BITS;//32 kb chunks , 
	return RamTest[addr>>3]&BitTest[addr&7];
}


//Do a block test
void __fastcall rec_v1_BlockTest(u32 addr)
{
	//u32 addr_real=addr;
	addr&=RAM_MASK;
	if (rec_v1_GetBlockTest(addr))
	{
		//damn a block is overwrited
		for (u32 base=addr-512;base <addr+512;base+=2)
		{
			vector<rec_v1_BasicBlock*>* blklist=GetBlockList(base);

			u32 listsz=blklist->size();
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
	}
	
}
void rec_v1_NotifyMemWrite(u32 start , u32 size)
{

}