#include "dc\sh4\sh4_registers.h"
#include "dc\mem\sh4_mem.h"
#include "rec_v1_blockmanager.h"
#include "nullprof.h"

#include <vector>
using namespace std;

//#define block_cnt (0x1000) 
#define PAGE_SIZE_SW 64
#define PAGE_MASK_SW (PAGE_SIZE_SW-1)

#define LOOKUP_HASH_SIZE 0x1000
#define LOOKUP_HASH_MASK (LOOKUP_HASH_SIZE-1)

#define GetHash(addr) ((addr>>2)&LOOKUP_HASH_MASK)

vector<rec_v1_BasicBlock*> BlockLists[RAM_SIZE/PAGE_SIZE_SW];

vector<rec_v1_BasicBlock*> BlockLookupLists[LOOKUP_HASH_SIZE];
rec_v1_BasicBlock*		   BlockLookupGuess[LOOKUP_HASH_SIZE];

vector<rec_v1_BasicBlock*> SuspendedBlocks;

u8 WriteTest[RAM_SIZE/PAGE_SIZE_SW];

void AddToBlockList(vector<rec_v1_BasicBlock*>* list ,rec_v1_BasicBlock* block)
{
	u32 size=list->size();
	for (int i=0;i<size;i++)
	{
		if ((*list)[i]==0)
		{
			(*list)[i]=block;
			return;
		}
	}

	list->push_back(block);
}
void CheckEmptyList(vector<rec_v1_BasicBlock*>* list)
{
	u32 size=list->size();
	for (int i=0;i<size;i++)
	{
		if ((*list)[i]!=0)
		{
			return;
		}
	}
	list->clear();
}
void RemoveFromBlockList(vector<rec_v1_BasicBlock*>* list ,rec_v1_BasicBlock* block)
{
	u32 size=list->size();
	for (int i=0;i<size;i++)
	{
		if ((*list)[i]==block)
		{
			(*list)[i]=0;
			return;
		}
	}
	CheckEmptyList(list);
}

INLINE vector<rec_v1_BasicBlock*>* GetLookupBlockList(u32 address)
{
	address&=RAM_MASK;
	return &BlockLookupLists[GetHash(address)];
}


rec_v1_BasicBlock* rec_v1_FindBlock(u32 address)
{
	rec_v1_BasicBlock* thisblock;
	rec_v1_BasicBlock* fastblock;

	fastblock=BlockLookupGuess[GetHash(address)];

	if (
		(fastblock!=0) && 
		(fastblock->start==address) && 
		(fastblock->cpu_mode_tag ==fpscr.PR_SZ)
		)
	{
		fastblock->lookups++;
		return fastblock;
	}

	vector<rec_v1_BasicBlock*>* blklist = GetLookupBlockList(address);

	u32 listsz=(u32)blklist->size();
	for (u32 i=0;i<listsz;i++)
	{ 
		thisblock=(*blklist)[i];
		if ((thisblock!=0) && (thisblock->start==address))
		{
			if (thisblock->cpu_mode_tag==fpscr.PR_SZ)
			{
				if (fastblock==0 || fastblock->lookups==thisblock->lookups)
				{
					BlockLookupGuess[GetHash(address)]=thisblock;
				}
				thisblock->lookups++;
				return thisblock;
			}
		}
	}

	return 0;
}
#include <algorithm>



vector<rec_v1_BasicBlock*> all_block_list;
rec_v1_BasicBlock* rec_v1_NewBlock(u32 address)
{
	rec_v1_BasicBlock* rv=new rec_v1_BasicBlock();
	rv->start=address;
	rv->cpu_mode_tag=fpscr.PR_SZ;
	all_block_list.push_back(rv);
	
	return rv;
}



void rec_v1_RegisterBlock(rec_v1_BasicBlock* block)
{
	u32 start=(block->start&RAM_MASK)/PAGE_SIZE_SW;
	u32 end=(block->end&RAM_MASK)/PAGE_SIZE_SW;

	AddToBlockList(GetLookupBlockList(block->start),block);
	if (((block->start >>26)&0x7)==3)
	{
		for (int i=start;i<=end;i++)
		{
			//mem_b.LockRegion(start*PAGE_SIZE_SW,PAGE_SIZE_SW);
			AddToBlockList(&BlockLists[i],block);
			WriteTest[i]=0xFF;
		}
	}
}

void rec_v1_UnRegisterBlock(rec_v1_BasicBlock* block)
{
	u32 start=(block->start&RAM_MASK)/PAGE_SIZE_SW;
	u32 end=(block->end&RAM_MASK)/PAGE_SIZE_SW;

	RemoveFromBlockList(GetLookupBlockList(block->start),block);

	if (BlockLookupGuess[GetHash(block->start)]==block)
		BlockLookupGuess[GetHash(block->start)]=0;
		
	for (int i=start;i<=end;i++)
	{
		RemoveFromBlockList(&BlockLists[i],block);
		if (BlockLists[i].size()==0)
			WriteTest[i]=0x00;
	}
}


bool WriteTest_Hit(u32 address)
{
	u32 offset=address;

	u32 page=offset/PAGE_SIZE_SW;
	vector<rec_v1_BasicBlock*>* list=&BlockLists[page];

	for (int i=0;i<list->size();i++)
	{
		rec_v1_BasicBlock* bblock =(*list)[i];
		if (bblock && bblock->Contains(offset))
		{
			rec_v1_UnRegisterBlock(bblock);
			bblock->Suspend();
			SuspendedBlocks.push_back(bblock);
		}
	}

	//list->clear();
	//if (list->size()!=0)
	//	printf("list->size()!=0 !! WTFH ?\n");
	//printf("Ram lock write xD xD w000tt , page %d\n",page);
	//mem_b.UnLockRegion(offset&(~PAGE_MASK_SW),PAGE_SIZE_SW);
	return true;

}


void FreeSuspendedBlocks()
{
	//for (int i=0;i<SuspendedBlocks.size();i++)
	{
		//SuspendedBlocks[i]->Free();
		//delete SuspendedBlocks[i];
	}
	SuspendedBlocks.clear();
}

void __fastcall rec_v1_BlockTest(u32 addrf)
{
	//u32 addr_real=addr;
	//addrf&=;
	u32 addr=(addrf&RAM_MASK)/PAGE_SIZE_SW;
	if (WriteTest[addr])
	{
		WriteTest_Hit(addrf);
	}
	
}
void __fastcall rec_v1_NotifyMemWrite(u32 start , u32 size)
{
	start&=RAM_MASK;

	
	for (int i=0;i<size;i+=2)
	{
		rec_v1_BlockTest(start+i);
	}
	
	/*size=(size>>(HASH_BITS+3))+1;
	start=start>>(HASH_BITS+3);

	for (int curr=start;curr<=(start+size);)
	{
		if (RamTest[curr])
		{
			for (int u=0;u<HASH_P_SIZE;u++)
				rec_v1_BlockTest((curr<<(HASH_BITS+3))+u);
		}
		curr+=1;
	}*/
}

void rec_v1_CompileBlockTest(emitter<>* x86e,x86IntRegType r_addr,x86IntRegType temp)
{
	x86e->PUSH32R(r_addr);					//presrve r_addr ;)
	x86e->CALLFunc(rec_v1_BlockTest);		//do full test
	x86e->POP32R(r_addr);					//restore r_addr :D

	/*x86e->MOV32ItoR(temp,(u32)&RamTest[0]);	//get base ptr
	x86e->MOV32RtoR(EDX,r_addr);
	x86e->SHR32ItoR(EDX,(HASH_BITS+3));		//get correct offset
	x86e->ADD32RtoR(temp,EDX);				//add offset to it
	x86e->MOV8RmtoR(temp,temp);				//get hole test byte
	x86e->TEST8RtoR(temp,temp);				//if test byte is 0 , no need for full test
	u8* no_test=x86e->JZ8(0);				//jump over full test
	x86e->PUSH32R(r_addr);					//presrve r_addr ;)
	x86e->CALLFunc(rec_v1_BlockTest);		//do full test
	x86e->POP32R(r_addr);					//restore r_addr :D
	x86e->x86SetJ8(no_test);				//jump here if no test*/
}

void ConvBlockInfo(nullprof_block_info* to,rec_v1_BasicBlock* pblk)
{
	if (pblk==0)
	{
		to->addr=0xFFFFFFFF;
		return;
	}

	to->addr=pblk->start;
	to->sh4_code=GetMemPtr(pblk->start,4);
	to->x86_code=pblk->compiled->Code;
	to->sh4_bytes=pblk->end-pblk->start+2;
	to->x86_bytes=pblk->compiled->size;
	to->sh4_cycles=pblk->cycles;
	to->time=pblk->profile_time;
	to->calls=pblk->profile_calls;
}
void nullprof_GetBlock(nullprof_block_info* to,u32 type,u32 address)
{
	if (type!=0)
	{
		printf("Olny adress based method workies");
		to->addr=0xFFFFFFFF;
		return;
	}

	rec_v1_BasicBlock* pblk=rec_v1_FindBlock(address);


	ConvBlockInfo(to,pblk);
}

int compare_usage (const void * a, const void * b)
{
	rec_v1_BasicBlock* ba=*(rec_v1_BasicBlock**)a;
	rec_v1_BasicBlock* bb=*(rec_v1_BasicBlock**)b;

	double ava=(double)ba->profile_time;//(double)ba->profile_calls;
	double avb=(double)bb->profile_time;//(double)bb->profile_calls;

	
	return ( avb>ava?1:-1);
}

int compare_time (const void * a, const void * b)
{
	rec_v1_BasicBlock* ba=*(rec_v1_BasicBlock**)a;
	rec_v1_BasicBlock* bb=*(rec_v1_BasicBlock**)b;

	double ava=(double)ba->profile_time/(double)ba->profile_calls;
	double avb=(double)bb->profile_time/(double)bb->profile_calls;

	
	return ( avb>ava?1:-1);
}
int compare_calls (const void * a, const void * b)
{
	rec_v1_BasicBlock* ba=*(rec_v1_BasicBlock**)a;
	rec_v1_BasicBlock* bb=*(rec_v1_BasicBlock**)b;

	double ava=(double)ba->profile_calls;
	double avb=(double)bb->profile_calls;

	
	return ( avb>ava?1:-1);
}

void nullprof_GetBlocks(nullprof_blocklist* to, u32 type,u32 count)
{
	if (type!=ALL_BLOCKS)
	{
		if (count==0 || count>all_block_list.size())
			count=all_block_list.size();

		to->count=count;
		to->blocks=(nullprof_block_info*)malloc(count*sizeof(nullprof_block_info));

		if (type==PSLOW_BLOCKS)
			qsort(&(all_block_list[0]), all_block_list.size(), sizeof(rec_v1_BasicBlock*), compare_usage);
		else if (type==PTIME_BLOCKS)
			qsort(&(all_block_list[0]), all_block_list.size(), sizeof(rec_v1_BasicBlock*), compare_time);
		else if (type==PCALL_BLOCKS)
			qsort(&(all_block_list[0]), all_block_list.size(), sizeof(rec_v1_BasicBlock*), compare_calls);


		for (int i=0;i<count;i++)
		{
			//double cpb=(double)all_block_list[i]->profile_time/(double)all_block_list[i]->profile_calls;
			//fprintf(to,"Block 0x%X , size %d[%d cycles] , %f cycles/call , %f cycles/emucycle,%f consumed Mhrz\n",
			//	all_block_list[i]->start,
			//	(all_block_list[i]->end-all_block_list[i]->start+2)>>1,
			//	all_block_list[i]->cycles,
			//	cpb,
			//	cpb/all_block_list[i]->cycles,
			//	all_block_list[i]->profile_time/(1000.0f*1000.0f));
			ConvBlockInfo(&to->blocks[i],all_block_list[i]);
			void CompileBasicBlock_slow_c(rec_v1_BasicBlock* block);;
			CompileBasicBlock_slow_c(all_block_list[i]);
		}
	}
	else
	{
		count=all_block_list.size();
		to->count=count;
		to->blocks=(nullprof_block_info*)malloc(count*sizeof(nullprof_block_info));
		for (int i=0;i<all_block_list.size();i++)
		{
			ConvBlockInfo(&to->blocks[i],all_block_list[i]);
		}
	}
}
void nullprof_ClearBlockPdata()
{
	for (int i=0;i<all_block_list.size();i++)
	{
		all_block_list[i]->profile_calls=0;
		all_block_list[i]->profile_time=0;
	}
}

//nullprof_GetBlockFP* GetBlockInfo;
	//nullprof_GetBlocksFP* GetBlocks;
	//nullprof_ClearBlockPdataFP* ClearPdata;
nullprof_prof_pointers null_prof_pointers=
{
	"nullDC v0.0.1; rec_v1",
	nullprof_GetBlock,
	nullprof_GetBlocks,
	nullprof_ClearBlockPdata
};