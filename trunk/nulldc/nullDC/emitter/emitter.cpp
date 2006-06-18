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

void EmitAllocSet(void * ptr,u32 usedsize)
{
	realloc(ptr,usedsize);
	/*alloced_free-=usedsize;
	alloced_ptr+=usedsize;
	u8* oldtr=alloced_ptr;
	alloced_ptr=(u8*)(((u32)alloced_ptr+32)&(~31));
	alloced_free-=alloced_ptr-oldtr;
	if (alloced_free<0)
		alloced_free=0;*/
}