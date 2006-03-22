#include "ta_alloc.h"

//very fast allocator , optimised for mannyy small allocs ...
//can de alloc em all together olny (kind of stack based)
//should not be used to allocate things >1024 bytes ... it can't alloc more that TA_ALLOC_BLOCK_SIZE-16 (~ 256kb)
//allocating large blocks will lead to too much wasted mem ..

//alloc block size 256kb
#define TA_ALLOC_BLOCK_SIZE  (256*1024)		//block size
#define TA_ALLOC_MAX_BLOCK_SIZE  (TA_ALLOC_BLOCK_SIZE-1024)		//minimum free size of an active block
#define TA_ALLOC_OVERHEAD (sizeof(size_t)+sizeof(ta_alloc_memblock*))
struct ta_alloc_memblock
{
	u8 data[TA_ALLOC_BLOCK_SIZE-TA_ALLOC_OVERHEAD];
	size_t Used;					//count of used data
	ta_alloc_memblock* next;		//next block on alloc list
};

ta_alloc_memblock*	ta_alloc_first;
ta_alloc_memblock*	ta_alloc_current;
u32					ta_alloc_block_count;

void ta_alloc_init()
{
	ta_alloc_first=ta_alloc_current=0;
	ta_alloc_block_count=0;
}
//TODO: allocate 64 byte aligned blocks
void ta_alloc_new_block()
{
	ta_alloc_memblock* next=(ta_alloc_memblock*)malloc(sizeof(ta_alloc_memblock));
	//make sure these are correct values
	next->Used=TA_ALLOC_OVERHEAD;
	next->next=0;

	//add it to the list
	ta_alloc_memblock* last;
	if (ta_alloc_current!=0)
	{
		last=ta_alloc_current;
		//look for the block that has no next one
		while(last->next !=0)
			last=last->next;
		//and atatch our block at it
		last->next=next;
	}
	else
		ta_alloc_current=next;//first allocated block on list

	//do some more error check later
	if (ta_alloc_first==0)
		ta_alloc_first=ta_alloc_current;
}

ta_alloc_memblock* ta_alloc_can_alloc(size_t Size,u8 align)
{
	ta_alloc_memblock* start=ta_alloc_current;
	u32 align_mask=(align-1);

	while(start)
	{
		size_t realsize=Size+(start->Used & align_mask);
	
		//<= ?
		if ((start->Used + realsize)<TA_ALLOC_BLOCK_SIZE)
		{
			return start;
		}

		//if this block does not have any free space
		//just set teh next as current (this will kill the block before this one hacing space but .. duh..)
		if (start->Used >TA_ALLOC_MAX_BLOCK_SIZE)
		{
			ta_alloc_current=start->next;
		}

		//try next block , if availabe
		start=start->next;
	}

	return false;
}

void* ta_alloc_do_alloc(ta_alloc_memblock* block,size_t Size,u8 align)
{
	//align pointer
	block->Used+=(block->Used & (align-1));
	//get pointer
	void* rv=&block->data[block->Used];
	//increase used size
	block->Used+=Size;
	//return pointer
	return rv;
}

void* ta_alloc(size_t Size,u8 align)
{
	ta_alloc_memblock* blk=ta_alloc_can_alloc(Size,align);
	if (blk!=0)
	{
		return ta_alloc_do_alloc(blk,Size,align);
	}
	else
	{
		ta_alloc_new_block();
		blk=ta_alloc_can_alloc(Size,align);
		if (blk==0)
		{
			//error ... we can't affort alloc for some reason ...
			return 0;
		}
		else
		{
			return ta_alloc_do_alloc(blk,Size,align);
		}
	}
}

//Free all memory allocated
void ta_alloc_free_all()
{
	ta_alloc_memblock* cb=ta_alloc_first;
	while(cb)
	{
		cb->Used=TA_ALLOC_OVERHEAD;
		cb=cb->next;
	}
	ta_alloc_current= ta_alloc_first;
}

//Free and release all memory allocated to system
void ta_alloc_release_all()
{
	ta_alloc_memblock* cb=ta_alloc_first;
	while(cb)
	{
		ta_alloc_first=cb->next;
		free(cb);
		cb=ta_alloc_first;
	}
	//re init
	ta_alloc_init();
}

void ta_alloc_print_stats()
{
	printf("Ta alloc stats\n");
	printf("Allocated memory %d (%d blocks * %d block size)\n",ta_alloc_block_count*TA_ALLOC_BLOCK_SIZE,ta_alloc_block_count,TA_ALLOC_BLOCK_SIZE);
	ta_alloc_memblock* cb=ta_alloc_first;
	u32 blockid=0;
	while(cb)
	{
		printf("Block #%d: %f used ;%d bytes free\n",blockid++,(cb->Used*100)/TA_ALLOC_BLOCK_SIZE,TA_ALLOC_BLOCK_SIZE-cb->Used);
		cb=cb->next;
	}
}

