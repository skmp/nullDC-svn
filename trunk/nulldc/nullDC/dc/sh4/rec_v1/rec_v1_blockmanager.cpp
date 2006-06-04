#include "dc\sh4\sh4_registers.h"
#include "dc\mem\sh4_mem.h"
#include "rec_v1_blockmanager.h"
#include "nullprof.h"

//block manager : new implementation 
//ideas :
//use native locks for code overwrites
//fix the never ending crash problems
//make code simpler , so future optimisation of code lookups is possible

//Using mmu pages to lock areas on mem is the best way to keep track of writes , fully done by h/w.
//the problem w/ sh4 is that its usual to have data close to code (same page), witch makes it had..
//i decided to use a mixed model :
//
//Pages are locked , and if a page is invalidated too much (like 10 times)
//the dynarec will switch to manual invalidation for blocks that are on
//that page.Code that modifies its next bytes wont work , its not
//possible to exit from a block due to an invalidation.
//
//When in fallback mode , the block's code should check if the block is still valid , propably some cheap
//trick , like comparing the first 4 bytes of the block (cmp m32,i32) or rep cmp.
//
//The blocks are allways on the page block list no matter their mode.
//hybrid blocks (eg , they are half on a lockable page and half on a non lockable one) are considered 
//non lockable.they can be eiter manualy invalidated , either from lock code :).

//There is a global list of blocks , mailny used by the profiler and the shutdown functions.
//
//There is a block list per page , that contains all the blocks that are within that page (that counts
// those "partialy in" too).This list is used to discard blocks in case of a page write.
//
//Now , an array of lists is used to keep the block start addresses.It is indexed by a hash made from the
//address (to keep the list size small so fast lookups).Also there is a cache list that holds a pointer
//to the most commonly used block for that hash range.

#include <vector>
#include <algorithm>
using namespace std;

//
//helper list class
class BlockList:public vector<rec_v1_BasicBlock*>
{
public :
	u32 Add(rec_v1_BasicBlock* block)
	{
		for (u32 i=0;i<size();i++)
		{
			if (_Myfirst[i]==0)
			{
				_Myfirst[i]=block;
				return i;
			}
		}

		push_back(block);
		return size();
	}
	void Remove(rec_v1_BasicBlock* block)
	{
		for (u32 i=0;i<size();i++)
		{
			if (_Myfirst[i]==block)
			{
				_Myfirst[i]=0;
				return;
			}
		}
	}
	rec_v1_BasicBlock* Find(u32 address,u32 cpu_mode)
	{
		for (u32 i=0;i<size();i++)
		{
			if (
				(_Myfirst[i]!=0) && 
				(_Myfirst[i]->start == address) &&
				(_Myfirst[i]->cpu_mode_tag == cpu_mode)
				)
			{
				return _Myfirst[i];
			}
		}
	}
}
//
//page info
//bit 0 :1-> manual check , 0 -> locked check
//bit 1-7: reserved
u8 PageInfo[RAM_SIZE/PAGE_SIZE];

//block managment
BlockList all_block_list;

//block discard related vars
BlockList BlockPageLists[RAM_SIZE/PAGE_SIZE];
BlockList SuspendedBlocks;

//block lookup vars
#define LOOKUP_HASH_SIZE	0x1000
#define LOOKUP_HASH_MASK	(LOOKUP_HASH_SIZE-1)
#define GetLookupHash(addr) ((addr>>2)&LOOKUP_HASH_MASK)
BlockList					BlockLookupLists[LOOKUP_HASH_SIZE];
rec_v1_BasicBlock*			BlockLookupGuess[LOOKUP_HASH_SIZE];

//misc code & helper functions
//Free a list of blocks
void FreeBlocks(BlockList* blocks)
{
	for (u32 i=0;i<blocks->size();i++)
	{
		if ((*blocks)[i])
		{
			((*blocks)[i])->Free();
		}
	}
	blocks->clear();
}
//this should not be called from a running block , or it will crash
//Fully resets block hash/list , clears all entrys and free's any prevusly allocated blocks
void ResetBlocks()
{
	for (u32 i=0;i<LOOKUP_HASH_SIZE;i++)
	{
		BlockLookupLists[i].clear();
		BlockLookupGuess[i]=0;
	}

	for (u32 i=0;i<(RAM_SIZE/PAGE_SIZE);i++)
	{
		BlockPageLists[i].clear();
	}

	FreeBlocks(SuspendedBlocks);
	FreeBlocks(all_block_list);
	memset(PageInfo,0,sizeof(PageInfo));
}
//
void FreeSuspendedBlocks()
{
	FreeBlocks(SuspendedBlocks);
}

//block lookup code
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
				if (fastblock==0 || fastblock->lookups<=thisblock->lookups)
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


//suspend/ free related ;)
void SuspendBlock_internal(rec_v1_BasicBlock* block)
{
	//remove the block from :
	//
	//full block list
	//page block list
	//look up block list
	//Look up guess block list
}





//void rec_v1_CompileBlockTest(emitter<>* x86e,x86IntRegType r_addr,x86IntRegType temp)
//{
	//x86e->PUSH32R(r_addr);					//presrve r_addr ;)
//	x86e->CALLFunc(rec_v1_BlockTest);		//do full test
//	x86e->POP32R(r_addr);					//restore r_addr :D

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
//}


//nullProf implementation
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