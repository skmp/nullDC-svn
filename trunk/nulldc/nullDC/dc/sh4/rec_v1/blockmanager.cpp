#include "dc\sh4\sh4_registers.h"
#include "dc\mem\sh4_mem.h"
#include "blockmanager.h"
#include "nullprof.h"
#include "windows.h"

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

//Profile resuts :
// Cache hits : 99.5% (ingame/menus)
// Average list size : 3.4 blocks
//

#include <vector>
#include <algorithm>

#define BLOCK_LUT_GUESS
//#define DEBUG_BLOCKLIST
//#define OPTIMISE_LUT_SORT

#define BLOCK_NONE (&BLOCK_NONE_B)
CompiledBlockInfo BLOCK_NONE_B;

u32* block_stack_pointer;

u32 full_lookups;
u32 fast_lookups;

void ret_cache_reset();

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
			qsort(_Myfirst, ItemCount, sizeof(CompiledBlockInfo*), compare_BlockLookups);
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
		vector<CompiledBlockInfo*>::clear();
		ItemCount=0;
	}
};
//page info
pginfo PageInfo[RAM_SIZE/PAGE_SIZE];

//block managment
BlockList all_block_list;

//block discard related vars
BlockList BlockPageLists[RAM_SIZE/PAGE_SIZE];
BlockList SuspendedBlocks;

//block lookup vars
#define GetLookupHash(addr) ((addr>>2)&LOOKUP_HASH_MASK)
BlockList					BlockLookupLists[LOOKUP_HASH_SIZE];

#ifdef BLOCK_LUT_GUESS
//even after optimising blocks , guesses give a good speedup :)
CompiledBlockInfo*			BlockLookupGuess[LOOKUP_HASH_SIZE];
#endif

u32 bm_locked_block_count=0;
u32 bm_manual_block_count=0;
//Memory managment :)
class MemoryChunk
{
public:
	void Init(u8* _ptr,u32 sz,u32 Ma,u32 Mi)
	{
		ptr=_ptr;
		index=0;
		size=sz;
	}
	u8* ptr;
	u32 index;
	u32 size;

	bool CanAlloc(u32 size)
	{
		return true;
	}
	u8* Allocate(u32 sz)
	{
		if (size>=(index+sz))
		{
			u8* rv= &ptr[index];
			index+=sz;
			return rv;
		}
		else
			return 0;
	}
	void Reset()
	{
		index=0;
		//memset(ptr,0xCC,size);
	}
	//BlockList blocks;
};

MemoryChunk* MemChunks=0;
u32 MemChunkCount=0;

u8* DynarecCache;
u32 DynarecCacheSize;

//implemented later
void FreeBlock(CompiledBlockInfo* block);
void init_memalloc(u32 scs,u32 scc,u32 bcc,u32 bcs);
void reset_memalloc();

//misc code & helper functions
//this should not be called from a running block , or it could crash
//Free a list of blocks
//will also clear the list
void FreeBlocks(BlockList* blocks)
{
	for (u32 i=0;i<blocks->ItemCount;i++)
	{
		if ((*blocks)[i]!=BLOCK_NONE)
		{
			FreeBlock((*blocks)[i]);
		}
	}
	blocks->clear();
}


//this should not be called from a running block , or it will crash
//Fully resets block hash/list , clears all entrys and free's any prevusly allocated blocks
void ResetBlocks(bool free_too=true)
{
	//this should clear all lists
	while(all_block_list.ItemCount!=0)
	{
		verify(all_block_list[0]!=BLOCK_NONE);
		SuspendBlock(all_block_list[0]);
	}
	all_block_list.CheckEmpty();
	verify(all_block_list.ItemCount==0);

	for (u32 i=0;i<LOOKUP_HASH_SIZE;i++)
	{
		verify(BlockLookupLists[i].ItemCount==0);
		BlockLookupLists[i].CheckEmpty();
		#ifdef BLOCK_LUT_GUESS
		verify(BlockLookupGuess[i] ==0 || BlockLookupGuess[i]==BLOCK_NONE);
		BlockLookupGuess[i]=BLOCK_NONE;
		#endif
	}

	for (u32 i=0;i<(RAM_SIZE/PAGE_SIZE);i++)
	{
		verify(BlockPageLists[i].ItemCount==0);
		BlockPageLists[i].CheckEmpty();
	}
	if (free_too)
		FreeBlocks(&SuspendedBlocks);
	//FreeBlocks(&all_block_list); << THIS BETTER BE EMPTY WHEN WE GET HERE
	verify(all_block_list.ItemCount==0);
	
	//Reset misc data structures (all blocks are removed , so we just need to initialise em to default !)
	reset_memalloc();
	memset(PageInfo,0,sizeof(PageInfo));
	
	bm_locked_block_count=0;
	bm_manual_block_count=0;
}
u32 manbs,lockbs;

void bm_GetStats(bm_stats* stats)
{
	stats->block_count=all_block_list.ItemCount;

	u32 sz=0;
	for (u32 i=0;i<MemChunkCount;i++)
	{
		sz+=MemChunks[i].index;
	}
	stats->cache_size=sz;
	stats->manual_blocks=bm_manual_block_count;
	stats->manual_block_calls_delta=manbs;manbs=0;
	stats->locked_blocks=bm_locked_block_count;
	stats->locked_block_calls_delta=lockbs;lockbs=0;

	stats->fast_lookups=fast_lookups;fast_lookups=0;
	stats->full_lookups=full_lookups;full_lookups=0;

}
bool reset_cache=false;
void __fastcall _SuspendAllBlocks();
//this should not be called from a running block , or it cloud crash
//free's suspended blocks
void FreeSuspendedBlocks()
{
	if (reset_cache)
		_SuspendAllBlocks();
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

void CompileAndRunCode();
CompiledBlockInfo* __fastcall FindBlock_full(u32 address,CompiledBlockInfo* fastblock);
//Block lookups
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
CompiledBlockInfo*  __fastcall CompileCode(u32 pc);
CompiledBlockInfo* __fastcall FindBlock_full_compile(u32 address,CompiledBlockInfo* fastblock)
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
		return CompileCode(pc);

	thisblock->lookups++;
#ifdef BLOCK_LUT_GUESS
	if (fastblock->lookups<=thisblock->lookups)
	{
		BlockLookupGuess[GetLookupHash(address)]=thisblock;
	}
#endif
	return thisblock;
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

BasicBlockEP* __fastcall FindCode_full(u32 address,CompiledBlockInfo* fastblock);
//Code lookups
BasicBlockEP* __fastcall FindCode_fast(u32 address)
{
#ifdef BLOCK_LUT_GUESS
	CompiledBlockInfo* fastblock;

	fastblock=BlockLookupGuess[GetLookupHash(address)];

	if ((fastblock->start==address) && 
		(fastblock->cpu_mode_tag ==fpscr.PR_SZ)
		)
	{
		fastblock->lookups++;
		return fastblock->Code;
	}
	else
	{
		return FindCode_full(address,fastblock);
	}
#else
	return FindCode_full(address,BLOCK_NONE);
#endif

}
BasicBlockEP* __fastcall FindCode_full(u32 address,CompiledBlockInfo* fastblock)
{
	CompiledBlockInfo* thisblock;

	BlockList* blklist = GetLookupBlockList(address);
#ifdef _BM_CACHE_STATS
	full_lookups++;
#endif
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
		return CompileAndRunCode;

	thisblock->lookups++;
#ifdef BLOCK_LUT_GUESS
	if (fastblock->lookups<=thisblock->lookups)
	{
		BlockLookupGuess[GetLookupHash(address)]=thisblock;
	}
#endif
	return thisblock->Code;
}

pginfo GetPageInfo(u32 address)
{
	if (IsOnRam(address))
	{
		return PageInfo[GetRamPageFromAddress(address)];
	}
	else
	{
		pginfo rv;
		rv.flags.full=0;
		rv.flags.ManualCheck=0;
		rv.invalidates=0;
		return rv;
	}
}
/*
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
}*/

void RegisterBlock(CompiledBlockInfo* block)
{
	all_block_list.Add(block);

	u32 start=(block->start&RAM_MASK)/PAGE_SIZE;
	u32 end=(block->end&RAM_MASK)/PAGE_SIZE;

	//AddToBlockList(GetLookupBlockList(block->start),block);
	GetLookupBlockList(block->start)->Add(block);
	
	if (block->block_type.ProtectionType)
		bm_manual_block_count++;
	else
		bm_locked_block_count++;

	if (((block->start >>26)&0x7)==3)
	{	//Care about invalidates olny if on ram
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

	if (block->block_type.ProtectionType)
		bm_manual_block_count--;
	else
		bm_locked_block_count--;

	if (block->OnRam())
	{	//Care about invalidates olny if on ram
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
//can be called from a mem invalidation
void CBBs_BlockSuspended(CompiledBlockInfo* block,u32* sp);
void __fastcall SuspendBlock_exept(CompiledBlockInfo* block,u32* sp)
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
	CBBs_BlockSuspended(block,sp);

	//
	//add it to the "to be suspended" list
	SuspendedBlocks.Add(block);
}
void __fastcall SuspendAllBlocks()
{
	reset_cache=true;
}
void __fastcall _SuspendAllBlocks()
{
	printf("Reseting Dynarec Cache...\n");
	ResetBlocks(false);
	reset_cache=false;
}
//called from a manualy invalidated block
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
	CBBs_BlockSuspended(block,0);

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

bool RamLockedWrite(u8* address,u32* sp)
{
	size_t offset=address-mem_b.data;

	if (offset<RAM_SIZE)
	{
		size_t addr_hash = offset/PAGE_SIZE;
		BlockList* list=&BlockPageLists[addr_hash];
		if (list->ItemCount==0)
			return false;
		PageInfo[addr_hash].invalidates++;
					
		//for (size_t i=0;i<list->ItemCount;i++)
		//{
		//	if ((*list)[i])
		//	{
		while(list->ItemCount)
			SuspendBlock_exept((*list)[list->ItemCount-1],sp);
		//	}
		//}
		//list->clear();
		mem_b.UnLockRegion((u32)offset&(~(PAGE_SIZE-1)),PAGE_SIZE);

		if (PageInfo[addr_hash].invalidates>=1)
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
	init_memalloc(1,32*1024*1024,1,1024*1024);
}
void ResetBlockManager()
{
	ResetBlocks();
}
void TermBlockManager()
{
	ResetBlocks();
}

void DumpBlockMappings()
{
	FILE* out=fopen("C:\\blk.txt","wb");
	printf("nullDC block manager stats : tracing %d blocks\n",all_block_list.ItemCount);

	for (u32 i=0;i<all_block_list.ItemCount;i++)
	{
		fprintf(out,"block :0x%08X\t[%d]\n{\n",all_block_list[i]->start,i);
		{
			fprintf(out,"\tCode         : 0x%p\n",all_block_list[i]->Code);
			fprintf(out,"\tLookup count : %d\n",all_block_list[i]->lookups);
			fprintf(out,"\tSh4 size     : %d\n",all_block_list[i]->Size());
			fprintf(out,"\tx86 size     : %d\n",all_block_list[i]->size);
			
			if (all_block_list[i]->GetBB()->TF_next_addr!=0xFFFFFFFF)
				fprintf(out,"\tLink 1 : 0x%08X\t[%d]\n",all_block_list[i]->GetBB()->TF_next_addr,i);

			if (all_block_list[i]->GetBB()->TT_next_addr!=0xFFFFFFFF)
				fprintf(out,"\tLink 2 : 0x%08X\t[%d]\n",all_block_list[i]->GetBB()->TT_next_addr,i);

			fprintf(out,"}\n");
		}
	}
	fclose(out);
}

//Memory allocator
//we store blocks that are small on small buffers, and big ones on big buffers
//A small buffer is from 64 to 1024 KB , a big buffer is from 1024 to 4096 KB
//a block is considered small , if it is less than the 1/8th of the small buffer size :)

MemoryChunk* GetMCFromPtr(void* ptr)
{
	u32 offset=(u8*)ptr-(u8*)DynarecCache;
	for (u32 i=0;i<MemChunkCount;i++)
	{
		if (offset>MemChunks[i].size)
			offset-=MemChunks[i].size;
		else
			return &MemChunks[i];
	}
	return 0;
}
MemoryChunk* FindBestChunk(u32 size)
{
	MemoryChunk* rv=0;

	for (u32 i=0;i<MemChunkCount;i++)
	{
		if (MemChunks[i].CanAlloc(size))
		{
			if (rv==0)
				rv=&MemChunks[i];
			else
			{/*
				if (rv->usable_bytes<MemChunks[i].usable_bytes)
				{
					rv=&MemChunks[i];
				}
				else if (rv->usable_bytes == MemChunks[i].usable_bytes)
				{
					if (rv->freed_bytes>MemChunks[i].freed_bytes)
					{
						rv=&MemChunks[i];
					}
				}*/
			}
		}
	}

	return rv;
}

void init_memalloc(u32 scc,u32 scs,u32 bcc,u32 bcs)
{
	printf("Dynarec cache : %d*%dKB small buffers , %d*%dKB big buffers , for a total of %dKB dynarec cache\n",
		   scc,scs/1024,bcc,bcs/1024,(scc*scs+bcc*bcs)/1024);

	DynarecCacheSize=scc*scs+bcc*bcs;

	DynarecCache = (u8*)VirtualAlloc(0,DynarecCacheSize,MEM_COMMIT | MEM_RESERVE,PAGE_EXECUTE_READWRITE);
	verify(DynarecCache!=0);

	MemChunks = new MemoryChunk[scc + bcc];	
	MemChunkCount=scc + bcc;
	
	u8* DynarecMem=DynarecCache;

	for (u32 i =0;i<scc;i++)
	{
		MemChunks[i].Init(DynarecMem,scs,scs/8,0);
		DynarecMem+=scs;
	}
	for (u32 i = scc;i<(scc+bcc);i++)
	{
		MemChunks[i].Init(DynarecMem,bcs,bcs/2,scs/8);
		DynarecMem+=bcs;
		i++;
	}
}
void reset_memalloc()
{
	for (u32 i =0;i<MemChunkCount;i++)
	{
		MemChunks[i].Reset();
	}
}
u8 dyna_tempbuffer[1024*1024];
void* dyna_malloc(u32 size)
{
	verify(size<sizeof(dyna_tempbuffer));
	return &dyna_tempbuffer;
}
void* dyna_realloc(void*ptr,u32 oldsize,u32 newsize)
{
	if (ptr==0)
		return dyna_malloc(newsize);
	verify(newsize<sizeof(dyna_tempbuffer));
	return ptr;
}
void* dyna_finalize(void* ptr,u32 oldsize,u32 newsize)
{
//	__asm int 3;
	void* rv=0;

	MemoryChunk* mc=FindBestChunk(newsize);
	rv=mc->Allocate(newsize);

	if (rv==0)
	{
		printf("Must flush a buffer !\n");
		return 0;
	//	__asm int 3;
	}

	memcpy(rv,dyna_tempbuffer,newsize);
	
	return rv;
}
void dyna_link(CompiledBlockInfo* block)
{
	//MemoryChunk* mc =  GetMCFromPtr(block->Code);
	//mc->blocks.Add(block);
	//block->Code=(BasicBlockEP*)rv;
}
void dyna_free(CompiledBlockInfo* block)
{
	//MemoryChunk* mc =  GetMCFromPtr(block->Code);
	//mc->blocks.Remove(block);
	//mc->freed_bytes+=block->size;
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


int compare_time_g (const void * a, const void * b)
{
	CompiledBlockInfo* cba=*(CompiledBlockInfo**)a;CompiledBlockInfo* cbc=*(CompiledBlockInfo**)b;
	
	return cba->GetNP()->time < cbc->GetNP()->time ? 1:-1;
}
int compare_calls_g (const void * a, const void * b)
{
	CompiledBlockInfo* cba=*(CompiledBlockInfo**)a;CompiledBlockInfo* cbc=*(CompiledBlockInfo**)b;
	
	return cba->GetNP()->calls < cbc->GetNP()->calls ? 1:-1;
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
	return compare_calls_g(a,b);
}

void nullprof_GetBlocks(nullprof_blocklist* to, u32 type,u32 count)
{
	BlockList used_blocks;
	
	for (u32 i=0;i<all_block_list.ItemCount;i++)
	{
		if (all_block_list[i]!=BLOCK_NONE)
			used_blocks.Add(all_block_list[i]);
	}

	if (type!=ALL_BLOCKS)
	{
		if (count==0 || count>used_blocks.ItemCount)
			count=(u32)used_blocks.ItemCount;

		to->count=count;
		to->blocks=(nullprof_block_info*)malloc(count*sizeof(nullprof_block_info));

		if (type==PSLOW_BLOCKS)
			qsort(&(used_blocks[0]), used_blocks.ItemCount, sizeof(CompiledBlockInfo*), compare_usage_g);
		else if (type==PTIME_BLOCKS)
			qsort(&(used_blocks[0]), used_blocks.ItemCount, sizeof(CompiledBlockInfo*), compare_time_g);
		else if (type==PCALL_BLOCKS)
			qsort(&(used_blocks[0]), used_blocks.ItemCount, sizeof(CompiledBlockInfo*), compare_calls_g);


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
		if (all_block_list[i]!=BLOCK_NONE)
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

BlockList tbp_tick_blocks;
u32 dyna_ticz=0;
///TBP :)
int compare_tbp_ticks (const void * a, const void * b)
{
	CompiledBlockInfo* cba=*(CompiledBlockInfo**)a;
	CompiledBlockInfo* cbb=*(CompiledBlockInfo**)b;
	return cba->tbp_ticks-cbb->tbp_ticks;
}
void dyna_profiler_printf()
{
	printf("%d blocks in list\n",tbp_tick_blocks.size());
	qsort(&(tbp_tick_blocks[0]), tbp_tick_blocks.ItemCount, sizeof(CompiledBlockInfo*), compare_tbp_ticks);

	u32 tticks=0;
	for (u32 i=0;i<tbp_tick_blocks.size();i++)
	{
		printf("%d%%[%d] :0x%X : 0x%X %d-%d\n"
			,tbp_tick_blocks[i]->tbp_ticks*100/dyna_ticz
			,tbp_tick_blocks[i]->tbp_ticks
			,tbp_tick_blocks[i]->start
			,tbp_tick_blocks[i]->cpu_mode_tag
			,tbp_tick_blocks[i]->OpcodeCount()
			,tbp_tick_blocks[i]->size);
		tticks+=tbp_tick_blocks[i]->tbp_ticks;
		tbp_tick_blocks[i]->tbp_ticks=0;
		//if (i>10)
		//	break;
	}
	printf("%d%% total\n",tticks*100/dyna_ticz);
	tbp_tick_blocks.clear();
	dyna_ticz=0;
}

void dyna_profiler_tick(void* addr)
{
	if (dyna_ticz++>200)
		dyna_profiler_printf();
	u32 nca=(u32)addr;
	for (u32 i=0;i<all_block_list.size();i++)
	{
		CompiledBlockInfo* cb=all_block_list[i];
		u32 baddr=(u32)cb->Code;

		if ((nca>=baddr) && ((nca-baddr)<cb->size))
		{
			if (tbp_tick_blocks.Find(cb->start,cb->cpu_mode_tag)==0)
				tbp_tick_blocks.Add(cb);
			
			cb->tbp_ticks++;
			return;
		}
	}
	__asm int 3;
	printf("0x%X OMG! UNABLE TO MATCH BLOCK TEH NOES\n",addr);
}

