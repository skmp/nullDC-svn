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
	{
		fast_block->lookups++;
		return fast_block;
	}
	else
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
	}
	
}
void rec_v1_NotifyMemWrite(u32 start , u32 size)
{

}

void rec_v1_CompileBlockTest(emitter<>* x86e,x86IntRegType r_addr,x86IntRegType temp)
{
	x86e->MOV32RtoR(EDX,r_addr);
	x86e->SHR32ItoR(EDX,(HASH_BITS+3));		//get correct offset
	x86e->MOV32ItoR(temp,(u32)&RamTest[0]);	//get base ptr
	x86e->ADD32RtoR(temp,EDX);			//add offset to it
	x86e->MOV8RmtoR(temp,temp);				//get hole test byte
	x86e->TEST8RtoR(temp,temp);				//if test byte is 0 , no need for full test
	u8* no_test=x86e->JZ8(0);				//jump over full test
	x86e->PUSH32R(r_addr);					//presrve r_addr ;)
	x86e->CALLFunc(rec_v1_BlockTest);		//do full test
	x86e->POP32R(r_addr);					//restore r_addr :D
	x86e->x86SetJ8(no_test);				//jump here if no test
}