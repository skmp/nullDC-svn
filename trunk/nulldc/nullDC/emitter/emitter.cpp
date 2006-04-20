#include "emitter.h"

void TestEmit()
{

}

u8* alloced_ptr=0;
int alloced_free=0;

void* EmitAlloc(u32 minsize)
{
	if (alloced_free>=(int)minsize)
	{
		return alloced_ptr;
	}
	else
	{
		alloced_ptr=(u8*)malloc(5*1024*1024);
		alloced_free=5*1024*1024;
		
		return alloced_ptr;
	}
}

void EmitAllocSet(u32 usedsize)
{
	alloced_free-=(usedsize+63)& (~32);
	alloced_ptr+=(usedsize+63)& (~32);
	if (alloced_free<0)
		alloced_free=0;
}