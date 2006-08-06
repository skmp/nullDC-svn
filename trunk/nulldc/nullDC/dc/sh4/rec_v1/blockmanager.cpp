#include "dc\sh4\sh4_registers.h"
#include "dc\mem\sh4_mem.h"
#include "blockmanager.h"
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

#define BLOCK_LUT_GUESS
//#define DEBUG_BLOCKLIST
//#define OPTIMISE_LUT_SORT

#define BLOCK_NONE (&BLOCK_NONE_B)
CompiledBlockInfo BLOCK_NONE_B;

//BasicBlock* Blockz[RAM_SIZE>>1];
//
//helper list class
int compare_BlockLookups(const void * a, const void * b)
{
	CompiledBlockInfo* ba=*(CompiledBlockInfo**)a;
	CompiledBlockInfo* bb=*(CompiledBlockInfo**)b;

	return bb->lookups-ba->lookups;
}


class BlockList:public vector<CompiledBlockInfo*>
{
public :
	size_t ItemCount;
	BlockList():vector<CompiledBlockInfo*>()
	{
		ItemCount=0;
	}
#ifdef DEBUG_BLOCKLIST
	void Test()
	{
	#ifdef DEBUG_BLOCKLIST
		verify(ItemCount>=0);
		verify(ItemCount<=size());

		for (size_t i=0;i<ItemCount;i++)
					verify(_Myfirst[i]!=BLOCK_NONE && _Myfirst[i]->Discarded==false);
		for (size_t i=ItemCount;i<size();i++)
					verify(_Myfirst[i]==BLOCK_NONE);
	#endif
	}
#else
	#define Test()
#endif
	u32 Add(CompiledBlockInfo* block)
	{
		Test();
		if (ItemCount==size())
		{	
			ItemCount++;
			push_back(block);
			Test();
			return (u32)ItemCount-1;
		}
		else
		{
			#ifdef DEBUG_BLOCKLIST
			verify(_Myfirst[ItemCount]==BLOCK_NONE);
			#endif
			_Myfirst[ItemCount]=block;
			ItemCount++;
			Test();
			return ItemCount-1;
			//for (u32 i=0;i<size();i++)
			//{
			//	if (_Myfirst[i]==BLOCK_NONE)
			//	{
			//		_Myfirst[i]=block;
			//		return i;
			//	}
			//}
		}
	}
	void Remove(CompiledBlockInfo* block)
	{
		Test();
		if (ItemCount==0)
			return;
		
		for (u32 i=0;i<ItemCount;i++)
		{
			if (_Myfirst[i]==block)
			{
				ItemCount--;
				if (ItemCount==0)
				{
					#ifdef DEBUG_BLOCKLIST
					verify(_Myfirst[0]==block && i==0);
					#endif
					_Myfirst[i]=BLOCK_NONE;
					CheckEmpty();
				}
				else if (ItemCount!=i)
				{
					#ifdef DEBUG_BLOCKLIST
					verify(_Myfirst[ItemCount]!=BLOCK_NONE);
					verify(ItemCount<size());
					verify(i<ItemCount);
					#endif
					_Myfirst[i]=_Myfirst[ItemCount];
					_Myfirst[ItemCount]=BLOCK_NONE;
				}
				else
				{
					#ifdef DEBUG_BLOCKLIST
					verify(ItemCount<size());
					verify(i==ItemCount);
					#endif
					_Myfirst[i]=BLOCK_NONE;
				}

				Test();
				return;
			}
		}
		Test();
		verify(false);
	}
	CompiledBlockInfo* Find(u32 address,u32 cpu_mode)
	{
		Test();
		for (u32 i=0;i<ItemCount;i++)
		{
			if ((_Myfirst[i]->start == address) &&
				(_Myfirst[i]->cpu_mode_tag == cpu_mode)
				)
			{
				return _Myfirst[i];
			}
		}
		return 0;
	}
	
	//check if the list is empty (full of BLOCK_NONE's) , if so , clear it
	void CheckEmpty()
	{
		Test();
		if (ItemCount!=0)
			return;
		#ifdef DEBUG_BLOCKLIST
		u32 sz=size();
		for (u32 i=0;i<sz;i++)
		{

			if (_Myfirst[i]!=BLOCK_NONE)
			{
				printf("BlockList::CheckEmptyList fatal error , ItemCount!=RealItemCount\n");
				__asm int 3;
				return;
			}

		}
		#endif
		clear();
	}

	//optimise blocklist for best lookup times
	void Optimise()
	{
		if (ItemCount==0)
			return;

		if (size())
		{
			//using a specialised routine is gona be faster .. bah
			qsort(_Myfirst, ItemCount, sizeof(BasicBlock*), compare_BlockLookups);
			//sort(begin(), end());
			/*u32 max=_Myfirst[0]->lookups/100;
			//if (max==0)
			max++;
			for (u32 i=0;i<size();i++)
				_Myfirst[i]->lookups/=max;*/
		}
	}

	void clear()
	{
		vector::clear();
		ItemCount=0;
	}
};
//page info

#define PAGE_MANUALCHECK 1
struct pginfo
{
	u32 invalidates;
	union
	{
		struct
		{
			u32 ManualCheck:1;	//bit 0 :1-> manual check , 0 -> locked check
			u32 reserved:31;	//bit 1-31: reserved
		};
		u32 full;
	} flags;
};

pginfo PageInfo[RAM_SIZE/PAGE_SIZE];

//block managment
BlockList all_block_list;

//block discard related vars
BlockList BlockPageLists[RAM_SIZE/PAGE_SIZE];
BlockList SuspendedBlocks;

//block lookup vars
#define LOOKUP_HASH_SIZE	0x4000
#define LOOKUP_HASH_MASK	(LOOKUP_HASH_SIZE-1)
#define GetLookupHash(addr) ((addr>>2)&LOOKUP_HASH_MASK)
BlockList					BlockLookupLists[LOOKUP_HASH_SIZE];

#ifdef BLOCK_LUT_GUESS
//even after optimising blocks , guesses give a good speedup :)
CompiledBlockInfo*			BlockLookupGuess[LOOKUP_HASH_SIZE];
#endif
//implemented later
void FreeBlock(CompiledBlockInfo* block);

//misc code & helper functions
//this should not be called from a running block , or it could crash
//Free a list of blocks
//will also clear the list
void FreeBlocks(BlockList* blocks)
{
	for (u32 i=0;i<blocks->ItemCount;i++)
	{
		if ((*blocks)[i])
		{
			FreeBlock((*blocks)[i]);
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
		#ifdef BLOCK_LUT_GUESS
		BlockLookupGuess[i]=BLOCK_NONE;
		#endif
	}

	for (u32 i=0;i<(RAM_SIZE/PAGE_SIZE);i++)
	{
		BlockPageLists[i].clear();
	}

	FreeBlocks(&SuspendedBlocks);
	FreeBlocks(&all_block_list);
	memset(PageInfo,0,sizeof(PageInfo));
}
//this should not be called from a running block , or it cloud crash
//free's suspended blocks
void FreeSuspendedBlocks()
{
	FreeBlocks(&SuspendedBlocks);
}


//fast rand function (gota check that too)
u32 f_rand_seed=0xFEDC;
u32 f_rand_last=0;
u32 frand()
{ 
	f_rand_seed=(f_rand_seed<<3) ^ f_rand_last;
	return f_rand_last=(f_rand_seed^(f_rand_seed>>11));
}

//block lookup code
INLINE BlockList* GetLookupBlockList(u32 address)
{
	address&=RAM_MASK;
	return &BlockLookupLists[GetLookupHash(address)];
}



u32 luk=0;
u32 r_value=0x112;

CompiledBlockInfo* __fastcall FindBlock_full(u32 address,CompiledBlockInfo* fastblock);

INLINE CompiledBlockInfo* __fastcall FindBlock_fast(u32 address)
{
#ifdef BLOCK_LUT_GUESS
	CompiledBlockInfo* fastblock;

	fastblock=BlockLookupGuess[GetLookupHash(address)];

	if ((fastblock->start==address) && 
		(fastblock->cpu_mode_tag ==fpscr.PR_SZ)
		)
	{
		fastblock->lookups++;
		return fastblock;
	}
	else
	{
		return FindBlock_full(address,fastblock);
	}
#else
	return FindBlock_full(address,BLOCK_NONE);
#endif

}
CompiledBlockInfo* __fastcall FindBlock_full(u32 address,CompiledBlockInfo* fastblock)
{
	CompiledBlockInfo* thisblock;

	BlockList* blklist = GetLookupBlockList(address);

	//u32 listsz=(u32)blklist->size();
	//for (u32 i=0;i<listsz;i++)
	//{ 
	//	thisblock=(*blklist)[i];
	//	if (thisblock->start==address && thisblock->cpu_mode_tag==fpscr.PR_SZ)
	//	{
	//		thisblock->lookups++;
	//		return thisblock;
	//	}
	//}

#ifdef OPTIMISE_LUT_SORT
	luk++;
	if (luk==r_value)
	{
		luk=0;
		r_value=(frand() & 0x1FF) + 0x800;
		blklist->Optimise();
	}
#endif
	thisblock=blklist->Find(address,fpscr.PR_SZ);
	if (thisblock==0)
		return 0;

	thisblock->lookups++;
#ifdef BLOCK_LUT_GUESS
	if (fastblock->lookups<=thisblock->lookups)
	{
		BlockLookupGuess[GetLookupHash(address)]=thisblock;
	}
#endif
	return thisblock;
}

void FillBlockLockInfo(BasicBlock* block)
{
	if (block->OnRam())
	{
		u32 start=block->page_start();
		u32 end=block->page_end();

		bool ManualCheck=false;
		for (u32 i=start;i<=end;i++)
		{
			ManualCheck|=PageInfo[i].flags.ManualCheck;
		}

		if (ManualCheck)
			block->flags.ProtectionType=BLOCK_PROTECTIONTYPE_MANUAL;
		else
			block->flags.ProtectionType=BLOCK_PROTECTIONTYPE_LOCK;
	}
	else//On rom , LOCK protection , never gets invalidated :)
	{
		block->flags.ProtectionType=BLOCK_PROTECTIONTYPE_LOCK;
	}
}

void RegisterBlock(CompiledBlockInfo* block)
{
	all_block_list.Add(block);

	u32 start=(block->start&RAM_MASK)/PAGE_SIZE;
	u32 end=(block->end&RAM_MASK)/PAGE_SIZE;

	//AddToBlockList(GetLookupBlockList(block->start),block);
	GetLookupBlockList(block->start)->Add(block);

	if (((block->start >>26)&0x7)==3)
	{	//Care about invalidates olny if on ram
		//Blockz[((block->start&RAM_MASK)>>1)]=block;
		for (u32 i=start;i<=end;i++)
		{
			if (PageInfo[i].flags.ManualCheck==0)
			{	//if manual checks , we must not lock
				mem_b.LockRegion(i*PAGE_SIZE,PAGE_SIZE);
				BlockPageLists[i].Add(block);
			}
		}
	}
}

void UnRegisterBlock(CompiledBlockInfo* block)
{
	u32 start=(block->start&RAM_MASK)/PAGE_SIZE;
	u32 end=(block->end&RAM_MASK)/PAGE_SIZE;

	GetLookupBlockList(block->start)->Remove(block);

	all_block_list.Remove(block);

	#ifdef BLOCK_LUT_GUESS
	if (BlockLookupGuess[GetLookupHash(block->start)]==block)
		BlockLookupGuess[GetLookupHash(block->start)]=BLOCK_NONE;
	#endif

	if (block->OnRam())
	{	//Care about invalidates olny if on ram
		//Blockz[((block->start&RAM_MASK)>>1)]=0;
		for (u32 i=start;i<=end;i++)
		{
			if (PageInfo[i].flags.ManualCheck==0)
			{	//if manual checks , we don't need to unlock (its not locked to start with :p)
				BlockPageLists[i].Remove(block);
				BlockPageLists[i].CheckEmpty();
				if (BlockPageLists[i].ItemCount==0)
				{
					mem_b.UnLockRegion(i*PAGE_SIZE,PAGE_SIZE);
				}
			}
		}
	}
}


//suspend/ free related ;)
//called to suspend a block
//can be called from a mem invalidation , or directly from a manualy invalidated block
void CBBs_BlockSuspended(CompiledBlockInfo* block);
void __fastcall SuspendBlock(CompiledBlockInfo* block)
{
	//remove the block from :
	//
	//not full block list , its removed from here olny when deleted (thats why its "full block" list ..)
	//page block list
	//look up block list
	//Look up guess block list
	//unregisting a block does exactly all that :)

	verify(block!=BLOCK_NONE);
	UnRegisterBlock(block);
	block->Suspend();
	CBBs_BlockSuspended(block);

	//
	//add it to the "to be suspended" list
	SuspendedBlocks.Add(block);
}
//called to free a suspended block
void FreeBlock(CompiledBlockInfo* block)
{
	//free the block
	//all_block_list.Remove(block);
	verify(block!=BLOCK_NONE);
	block->Free();
	DeleteBlock(block);
}

bool RamLockedWrite(u8* address)
{
	size_t offset=address-mem_b.data;

	if (offset<RAM_SIZE)
	{
		size_t addr_hash = offset/PAGE_SIZE;
		BlockList* list=&BlockPageLists[addr_hash];
		PageInfo[addr_hash].invalidates++;
					
		//for (size_t i=0;i<list->ItemCount;i++)
		//{
		//	if ((*list)[i])
		//	{
		while(list->ItemCount)
			SuspendBlock((*list)[list->ItemCount-1]);
		//	}
		//}
		//list->clear();
		mem_b.UnLockRegion((u32)offset&(~(PAGE_SIZE-1)),PAGE_SIZE);

		if (PageInfo[addr_hash].invalidates>20)
			PageInfo[addr_hash].flags.ManualCheck=1;

		return true;
	}
	else
		return false;
}

void InitBlockManager()
{
	BLOCK_NONE->start=0xFFFFFFFF;
	BLOCK_NONE->cpu_mode_tag=0xFFFFFFFF;
	BLOCK_NONE->lookups=0;
}
void ResetBlockManager()
{
	ResetBlocks();
}
void TermBlockManager()
{
	ResetBlocks();
}

///////////////////////////////////////////////
//			nullProf implementation			 //
///////////////////////////////////////////////
void ConvBlockInfo(nullprof_block_info* to,CompiledBlockInfo* pblk)
{
	if (pblk==BLOCK_NONE)
	{
		to->addr=0xFFFFFFFF;
		return;
	}

	to->addr=pblk->start;
	to->sh4_code=GetMemPtr(pblk->start,4);
	to->x86_code=pblk->Code;
	to->sh4_bytes=pblk->end-pblk->start+2;
	to->x86_bytes=pblk->size;
	to->sh4_cycles=pblk->GetNP()->cycles;
	to->time=pblk->GetNP()->time;
	to->calls=pblk->GetNP()->calls;
}
void nullprof_GetBlock(nullprof_block_info* to,u32 type,u32 address)
{
	if (type!=0)
	{
		printf("Olny adress based method workies");
		to->addr=0xFFFFFFFF;
		return;
	}

	CompiledBlockInfo* pblk=FindBlock(address);


	ConvBlockInfo(to,pblk);
}


template <class Ta,class Tb>
int compare_usage (const Ta * ba, const Tb * bb)
{
	double ava=(double)ba->profile_time;//(double)ba->profile_calls;
	double avb=(double)bb->profile_time;//(double)bb->profile_calls;

	return ( avb>ava?1:-1);
}
template <class Ta,class Tb>
int compare_time (const Ta * ba, const Tb * bb)
{
	double ava=(double)ba->profile_time/(double)ba->profile_calls;
	double avb=(double)bb->profile_time/(double)bb->profile_calls;

	return ( avb>ava?1:-1);
}
template <class Ta,class Tb>
int compare_calls (const Ta * ba, const Tb * bb)
{
	double ava=(double)ba->profile_calls;
	double avb=(double)bb->profile_calls;

	return ( avb>ava?1:-1);
}

int compare_usage_g (const void * a, const void * b)
{
	//CompiledBlockInfo* cba=*(CompiledBlockInfo*)a,CompiledBlockInfo* cbc=*(CompiledBlockInfo*)b;
	//verify(cba->block_type & COMPILED_BLOCK_NULLPROF);
	//verify(cbb->block_type & COMPILED_BLOCK_NULLPROF);

	//switch(cba->block_type & COMPILED_BLOCK_TYPE_MASK)
	//{
	//case COMPILED_BASIC_BLOCK:
	//	break;
	//case COMPILED_SUPER_BLOCK:
	//	break;
	//}
	return 0;
}
int compare_time_g (const void * a, const void * b)
{
	//CompiledBlockInfo* cba=*(CompiledBlockInfo**)a,CompiledBlockInfo* cbc=*(CompiledBlockInfo**)b;
	//verify(cba->block_type & COMPILED_BLOCK_NULLPROF);
	//verify(cbb->block_type & COMPILED_BLOCK_NULLPROF);
	return 0;
}
int compare_calls_g (const void * a, const void * b)
{
	//CompiledBlockInfo* cba=*(CompiledBlockInfo**)a,CompiledBlockInfo* cbc=*(CompiledBlockInfo**)b;
	//verify(cba->block_type & COMPILED_BLOCK_NULLPROF);
	//verify(cbb->block_type & COMPILED_BLOCK_NULLPROF);
	return 0;
}


void nullprof_GetBlocks(nullprof_blocklist* to, u32 type,u32 count)
{
	BlockList used_blocks;
	
	for (u32 i=0;i<all_block_list.ItemCount;i++)
	{
		if (all_block_list[i])
			used_blocks.Add(all_block_list[i]);
	}

	if (type!=ALL_BLOCKS)
	{
		if (count==0 || count>used_blocks.ItemCount)
			count=(u32)used_blocks.ItemCount;

		to->count=count;
		to->blocks=(nullprof_block_info*)malloc(count*sizeof(nullprof_block_info));

		if (type==PSLOW_BLOCKS)
			qsort(&(used_blocks[0]), used_blocks.ItemCount, sizeof(BasicBlock*), compare_usage_g);
		else if (type==PTIME_BLOCKS)
			qsort(&(used_blocks[0]), used_blocks.ItemCount, sizeof(BasicBlock*), compare_time_g);
		else if (type==PCALL_BLOCKS)
			qsort(&(used_blocks[0]), used_blocks.ItemCount, sizeof(BasicBlock*), compare_calls_g);


		for (u32 i=0;i<count;i++)
		{
			//double cpb=(double)used_blocks[i]->profile_time/(double)used_blocks[i]->profile_calls;
			//fprintf(to,"Block 0x%X , size %d[%d cycles] , %f cycles/call , %f cycles/emucycle,%f consumed Mhrz\n",
			//	used_blocks[i]->start,
			//	(used_blocks[i]->end-used_blocks[i]->start+2)>>1,
			//	used_blocks[i]->cycles,
			//	cpb,
			//	cpb/used_blocks[i]->cycles,
			//	used_blocks[i]->profile_time/(1000.0f*1000.0f));
			ConvBlockInfo(&to->blocks[i],used_blocks[i]);
			//void CompileBasicBlock_slow_c(BasicBlock* block);;
			//CompileBasicBlock_slow_c(used_blocks[i]);
		}
	}
	else
	{
		count=(u32)used_blocks.ItemCount;
		to->count=count;
		to->blocks=(nullprof_block_info*)malloc(count*sizeof(nullprof_block_info));
		for (u32 i=0;i<used_blocks.size();i++)
		{
			ConvBlockInfo(&to->blocks[i],used_blocks[i]);
		}
	}
}
void nullprof_ClearBlockPdata()
{
	BlockList used_blocks;

	for (u32 i=0;i<all_block_list.ItemCount;i++)
	{
		if (all_block_list[i])
			used_blocks.Add(all_block_list[i]);
	}

	for (u32 i=0;i<used_blocks.ItemCount;i++)
	{
		used_blocks[i]->GetNP()->calls=0;
		used_blocks[i]->GetNP()->time=0;
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