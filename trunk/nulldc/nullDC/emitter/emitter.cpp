#include "emitter.h"

void TestEmit()
{

}

u8* alloced_ptr=0;
int alloced_free=0;

u32 gen_8_mb=0;
void* EmitAlloc(u32 minsize)
{
	void * rv=malloc(minsize);
	
	return rv;
	/*if (alloced_free>=(int)minsize)
	{
		//printf("0x%X\n",alloced_ptr);
		return alloced_ptr;
	}
	else
	{
		alloced_ptr=(u8*)malloc(512*1024);
		alloced_free=512*1024;
		alloced_ptr=(u8*)(((u32)alloced_ptr+32)&(~31));
		printf("Dynarec Stats : Generated %d kb of code , allocating more\n",gen_8_mb*512); 
		gen_8_mb++;
		return alloced_ptr; 
	}*/
}
u32 total_size=0;
u32 alloc_count=0;
void EmitAllocSet(void * ptr,u32 usedsize)
{
	//alloc_count+=1;
	//total_size+=usedsize;
	realloc(ptr,usedsize);
	//printf("Used Mem : %d KB, %d bytes/alloc\n",total_size/1024,total_size/alloc_count);
	/*alloced_free-=usedsize;
	alloced_ptr+=usedsize;
	u8* oldtr=alloced_ptr;
	alloced_ptr=(u8*)(((u32)alloced_ptr+32)&(~31));
	alloced_free-=alloced_ptr-oldtr;
	if (alloced_free<0)
		alloced_free=0;*/
}

//ok! let's see
//Dynarec mem allocator
//This is non portable code
//Comited page size  : 4kb (x86)
//Internal page size : 1kb
/*
	Memory is reserved using Virtual Alloc on init , but not commited.
*/

void recAllocInit()
{
}

//start size is rounded up to 1kb
//At least start_size continius bytes are allocated
void recAllocCode(u32 start_size)
{
}

//Ensure we have size bytes allocated.If not , we allocate another slice , and emit a jmp to it :)
void recEnsureSize(u32 size)
{
}
/*
void rec

*/