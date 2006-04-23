#include "emitter.h"

void TestEmit()
{

}

u8* alloced_ptr=0;
int alloced_free=0;

u32 gen_8_mb=0;
void* EmitAlloc(u32 minsize)
{
	return malloc(minsize);
	if (alloced_free>=(int)minsize)
	{
		return alloced_ptr;
	}
	else
	{
		alloced_ptr=(u8*)malloc(8*1024*1024);
		alloced_free=8*1024*1024;
		alloced_ptr=(u8*)(((u32)alloced_ptr+31)&(~31));
		printf("Dynarec Stats : Generated %d Mb of code , allocating more\n",gen_8_mb*8); 
		gen_8_mb++;
		return alloced_ptr;
	}
}

void EmitAllocSet(void * ptr,u32 usedsize)
{
	realloc(ptr,usedsize);
	alloced_free-=(usedsize+31)&(~31);
	alloced_ptr+=(usedsize+31)&(~31);
	if (alloced_free<0)
		alloced_free=0;
}