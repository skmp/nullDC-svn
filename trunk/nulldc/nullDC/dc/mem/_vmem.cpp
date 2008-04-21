//new memory mapping code ..."_vmem" ... Don't ask where i got the name , it somehow poped on my head :p
//
/*
_vmem v2 :

physical map :
	_vmem : generic functions
	dvmem : direct access (using memory mapping)
	nvmem : native acc (dyn/etc)

Translated map:
	tvmem : generic function, may use the exception mechanism
	dbg 

*/
#include "_vmem.h"
#include "dc/aica/aica_if.h"

//top registed handler
_vmem_handler			_vmem_lrp;

//handler tables
_vmem_ReadMem8FP*		_vmem_RF8[0x1000];
_vmem_WriteMem8FP*		_vmem_WF8[0x1000];

_vmem_ReadMem16FP*		_vmem_RF16[0x1000];
_vmem_WriteMem16FP*		_vmem_WF16[0x1000];

_vmem_ReadMem32FP*		_vmem_RF32[0x1000];
_vmem_WriteMem32FP*		_vmem_WF32[0x1000];

//upper 16b of the address
void* _vmem_MemInfo[0x10000];
u8* sh4_reserved_mem;
u8* sh4_ram_alt;	//alternative ram space map

//ReadMem/WriteMem functions
//ReadMem
u8 naked fastcall _vmem_ReadMem8(u32 Address)
{
	__asm
	{
	//copy address
	mov edx,ecx;//this is done here , among w/ the and , it should be possible to fully execute it on paraler (no depency)
	mov eax,ecx;
	and edx,0xFFFF;//lower 16b of address

	//shr 14 + and vs shr16 + mov eax,[_vmem_MemInfo+eax*4];
	//after testing , shr16+mov complex is faster , both on amd (a64 x2) and intel (northwood)

	//get upper 16 bits
	shr eax,16;
	
	//read mem info
	mov eax,[_vmem_MemInfo+eax*4];

	test eax,0xFFFF0000;
	jnz direct;
	jmp [_vmem_RF8+eax];
direct:
	mov eax,[eax+edx];	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
	ret;
	}
}

u16 naked fastcall _vmem_ReadMem16(u32 Address)
{
	//read comments on 8 bit ver ;)
	__asm
	{
	mov edx,ecx;
	mov eax,ecx;
	and edx,0xFFFF;
	shr eax,16;
	mov eax,[_vmem_MemInfo+eax*4];
	test eax,0xFFFF0000;
	jnz direct;
	jmp [_vmem_RF16+eax];
direct:
	mov eax,[eax+edx];	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
	ret;
	}
}

u32 naked fastcall _vmem_ReadMem32(u32 Address)
{
	//read comments on 8 bit ver ;)
	__asm
	{
	mov edx,ecx;
	mov eax,ecx;
	and edx,0xFFFF;
	shr eax,16;
	mov eax,[_vmem_MemInfo+eax*4];
	test eax,0xFFFF0000;
	jnz direct;
	jmp [_vmem_RF32+eax];
direct:
	mov eax,[eax+edx];	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
	ret;
	}
}
//WriteMem
void naked fastcall _vmem_WriteMem8(u32 Address,u8 data)
{
	__asm
	{
	//copy address , we can't corrupt edx here ;( we will have to calculate just before used (damn ;()
	mov eax,ecx;

	//shr 14 + and vs shr16 + mov eax,[_vmem_MemInfo+eax*4];
	//after testing , shr16+mov complex is faster , both on amd (a64 x2) and intel (northwood)

	//get upper 16 bits
	shr eax,16;

	//read mem info
	mov eax,[_vmem_MemInfo+eax*4];

	test eax,0xFFFF0000;
	jnz direct;
	jmp [_vmem_WF8+eax];
direct:
	and ecx,0xFFFF;//lower 16b of address
	//or eax,edx;	//get ptr to the value we want
	//mov eax,[eax]
	mov [eax+ecx],dl;
	ret;
	}
}

void naked fastcall _vmem_WriteMem16(u32 Address,u16 data)
{
	//for comments read 8 bit version
	__asm
	{
	mov eax,ecx;
	shr eax,16;
	mov eax,[_vmem_MemInfo+eax*4];
	test eax,0xFFFF0000;
	jnz direct;
	jmp [_vmem_WF16+eax];
direct:
	and ecx,0xFFFF;
	mov [eax+ecx],dx;
	ret;
	}
}
void naked fastcall _vmem_WriteMem32(u32 Address,u32 data)
{
	//for comments read 8 bit version
	__asm
	{
	mov eax,ecx;
	shr eax,16;
	mov eax,[_vmem_MemInfo+eax*4];
	test eax,0xFFFF0000;
	jnz direct;
	jmp [_vmem_WF32+eax];
direct:
	and ecx,0xFFFF;
	mov [eax+ecx],edx;
	ret;
	}
}

//0xDEADC0D3 or 0
#define MEM_ERROR_RETURN_VALUE 0xDEADC0D3

//phew .. that was lota asm code ;) lets go back to C :D
//default mem handlers ;)
//defualt read handlers
u8 fastcall _vmem_ReadMem8_not_mapped(u32 addresss)
{
	printf("[sh4]Read8 from 0x%X, not mapped [_vmem default handler]\n",addresss);
	return (u8)MEM_ERROR_RETURN_VALUE;
}
u16 fastcall _vmem_ReadMem16_not_mapped(u32 addresss)
{
	printf("[sh4]Read16 from 0x%X, not mapped [_vmem default handler]\n",addresss);
	return (u16)MEM_ERROR_RETURN_VALUE;
}
u32 fastcall _vmem_ReadMem32_not_mapped(u32 addresss)
{
	printf("[sh4]Read32 from 0x%X, not mapped [_vmem default handler]\n",addresss);
	return (u32)MEM_ERROR_RETURN_VALUE;
}
//defualt write handers
void fastcall _vmem_WriteMem8_not_mapped(u32 addresss,u8 data)
{
	printf("[sh4]Write8 to 0x%X=0x%X, not mapped [_vmem default handler]\n",addresss,data);
}
void fastcall _vmem_WriteMem16_not_mapped(u32 addresss,u16 data)
{
	printf("[sh4]Write16 to 0x%X=0x%X, not mapped [_vmem default handler]\n",addresss,data);
}
void fastcall _vmem_WriteMem32_not_mapped(u32 addresss,u32 data)
{
	printf("[sh4]Write32 to 0x%X=0x%X, not mapped [_vmem default handler]\n",addresss,data);
}
//code to register handlers
//0 is considered error :)
_vmem_handler _vmem_register_handler(
									 _vmem_ReadMem8FP* read8, 
									 _vmem_ReadMem16FP* read16,
									 _vmem_ReadMem32FP* read32,

									 _vmem_WriteMem8FP* write8,
									 _vmem_WriteMem16FP* write16,
									 _vmem_WriteMem32FP* write32
									 )
{
	_vmem_handler rv=_vmem_lrp++;

	_vmem_RF8[rv] =read8==0  ?	_vmem_ReadMem8_not_mapped  :	read8;
	_vmem_RF16[rv]=read16==0 ?	_vmem_ReadMem16_not_mapped :	read16;
	_vmem_RF32[rv]=read32==0 ?	_vmem_ReadMem32_not_mapped :	read32;

	_vmem_WF8[rv] =write8==0 ?	_vmem_WriteMem8_not_mapped :	write8;
	_vmem_WF16[rv]=write16==0?	_vmem_WriteMem16_not_mapped:	write16;
	_vmem_WF32[rv]=write32==0?	_vmem_WriteMem32_not_mapped:	write32;

	return rv;
}

//map a registed handler to a mem region :)
void _vmem_map_handler(_vmem_handler Handler,u32 start,u32 end)
{
	verify(start<0x10000);
	verify(end<0x10000);
	verify(start<=end);
	for (u32 i=start;i<=end;i++)
	{
		_vmem_MemInfo[i]=((u8*)0)+(Handler*4);
	}
}
//map a memory block to a mem region :)
void _vmem_map_block(void* base,u32 start,u32 end)
{
	verify(start<0x10000);
	verify(end<0x10000);
	verify(start<=end);
	u32 j=0;
	for (u32 i=start;i<=end;i++)
	{
		_vmem_MemInfo[i]=&(((u8*)base)[j]);
		j+=0x10000;
	}
}
void _vmem_mirror_mapping(u32 new_region,u32 start,u32 size)
{
	u32 end=start+size-1;
	verify(start<0x10000);
	verify(end<0x10000);
	verify(start<=end);
	verify(!((start>=new_region) && (end<=new_region)));

	u32 j=new_region;
	for (u32 i=start;i<=end;i++)
	{
		_vmem_MemInfo[j&0xFFFF]=_vmem_MemInfo[i&0xFFFF];
		j++;
	}
}
//benchmarking functions
#ifdef _VMEM_BENCHMARK
u32 fastcall _vmem_ReadMem32_bench(u32 addresss)
{
	return 0x1;
}
void _vmem_Benchmark()
{
#define Bench_count  20000
	u32 hanld=_vmem_register_handler(0,0,_vmem_ReadMem32_bench,0,0,0);
	_vmem_map_handler(hanld,0,9);

	{
		u32 bstart=GetTickCount();

		u32 rez=0;
		for (int i=0;i<Bench_count;i++)
		{
			rez+=_vmem_ReadMem32((0x666+i)&0xFFF);
		}
		u32 bdur=GetTickCount()-bstart;
		float secs=(float)bdur/1000.0f;
		secs=Bench_count/secs;
		secs/=1024*1024;
		printf("vmem : %f MB/s from function , value=%d\n",secs*4,rez);
	}

	void* pm=malloc(10*0x10000);

	memset(pm,0,10*0x10000);
	_vmem_map_block((u8*)pm,0,9);

	{
		u32 bstart=GetTickCount();

		u32 rez=0;
		for (int i=0;i<Bench_count;i++)
		{
			rez+=_vmem_ReadMem32((0x666+i)&0xFFF);
		}
		u32 bdur=GetTickCount()-bstart;
		float secs=(float)bdur/1000.0f;
		secs=Bench_count/secs;
		secs/=1024*1024;
		printf("vmem : %f MB/s from Array , value=%d\n",secs*4,rez);
	}

	free(pm);
}
#endif
//init/reset/term
void _vmem_init()
{
	_vmem_reset();
	#ifdef _VMEM_BENCHMARK
	_vmem_Benchmark();
	#endif
}

void _vmem_reset()
{
	//clear read tables
	memset(_vmem_RF8,0,sizeof(_vmem_RF8));
	memset(_vmem_RF16,0,sizeof(_vmem_RF16));
	memset(_vmem_RF32,0,sizeof(_vmem_RF32));
	//clear write tables
	memset(_vmem_WF8,0,sizeof(_vmem_WF8));
	memset(_vmem_WF16,0,sizeof(_vmem_WF16));
	memset(_vmem_WF32,0,sizeof(_vmem_WF32));
	//clear meminfo table
	memset(_vmem_MemInfo,0,sizeof(_vmem_MemInfo));

	//reset registation index
	_vmem_lrp=0;

	//register default functions (0) for slot 0 :)
	verify(_vmem_register_handler(0,0,0,0,0,0)==0);
}

void _vmem_term()
{

}
#include <windows.h>
#include "dc\pvr\pvr_if.h"
#include "sh4_mem.h"

#define _VMEM_FILE_MAPPING
#ifdef _VMEM_FILE_MAPPING
HANDLE mem_handle;

#define MAP_RAM_START_OFFSET  0
#define MAP_VRAM_START_OFFSET (MAP_RAM_START_OFFSET+RAM_SIZE)
#define MAP_ARAM_START_OFFSET (MAP_VRAM_START_OFFSET+VRAM_SIZE)

void* _nvmem_map_buffer(u32 dst,u32 addrsz,u32 offset,u32 size)
{
	void* ptr;
	void* rv;

	u32 map_times=addrsz/size;
	verify((addrsz%size)==0);
	verify(map_times>=1);

	rv= MapViewOfFileEx(mem_handle,FILE_MAP_READ |FILE_MAP_WRITE,0,offset,size,&sh4_reserved_mem[dst]);
	if (!rv)
		return 0;

	for (u32 i=1;i<map_times;i++)
	{
		dst+=size;
		ptr=MapViewOfFileEx(mem_handle,FILE_MAP_READ,0,offset,size,&sh4_reserved_mem[dst]);
		if (!ptr) return 0;
	}

	return rv;
}


void* _nvmem_unused_buffer(u32 start,u32 end)
{
	void* ptr=VirtualAlloc(&sh4_reserved_mem[start],end-start,MEM_RESERVE,PAGE_NOACCESS);
	if (ptr==0)
		return 0;
	return ptr;
}

#define map_buffer(dsts,dste,offset,sz) {ptr=_nvmem_map_buffer(dsts,dste-dsts,offset,sz);if (!ptr) return false;}
#define unused_buffer(start,end) {ptr=_nvmem_unused_buffer(start,end);if (!ptr) return false;}


bool _vmem_reserve()
{
	mem_handle=CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE ,0,RAM_SIZE + VRAM_SIZE +ARAM_SIZE,L"ndc_mem_dataazz");

	void* ptr=0;
	sh4_reserved_mem=(u8*)VirtualAlloc(0,512*1024*1024,MEM_RESERVE,PAGE_NOACCESS);
	if (sh4_reserved_mem==0)
		return false;
	VirtualFree(sh4_reserved_mem,0,MEM_RELEASE);
	
	//Area 0
	//[0x00000000 ,0x00800000) -> unused
	unused_buffer(0x00000000,0x00800000);

	//i wonder, aica ram warps here ?.?
	//i realy should check teh docs before codin ;p
	//[0x00800000,0x00A00000);
	map_buffer(0x00800000,0x01000000,MAP_ARAM_START_OFFSET,ARAM_SIZE);
	
	aica_ram.size=ARAM_SIZE;
	aica_ram.data=(u8*)ptr;
	//[0x01000000 ,0x04000000) -> unused
	unused_buffer(0x01000000,0x04000000);
	
	//Area 1
	//[0x04000000,0x05000000) -> vram (16mb, warped on dc)
	map_buffer(0x04000000,0x05000000,MAP_VRAM_START_OFFSET,VRAM_SIZE);
	
	vram.size=VRAM_SIZE;
	vram.data=(u8*)ptr;

	//[0x05000000,0x06000000) -> unused (32b path)
	unused_buffer(0x05000000,0x06000000);

	//[0x06000000,0x07000000) -> vram   mirror
	map_buffer(0x06000000,0x07000000,MAP_VRAM_START_OFFSET,VRAM_SIZE);

	//[0x07000000,0x08000000) -> unused (32b path) mirror
	unused_buffer(0x07000000,0x08000000);
	
	//Area 2
	//[0x08000000,0x0C000000) -> unused
	unused_buffer(0x08000000,0x0C000000);
	
	//Area 3
	//[0x0C000000,0x0D000000) -> main ram
	//[0x0D000000,0x0E000000) -> main ram mirror
	//[0x0E000000,0x0F000000) -> main ram mirror
	//[0x0F000000,0x10000000) -> main ram mirror
	map_buffer(0x0C000000,0x10000000,MAP_RAM_START_OFFSET,RAM_SIZE);
	
	mem_b.size=RAM_SIZE;
	mem_b.data=(u8*)ptr;
	
	//Area 4
	//Area 5
	//Area 6
	//Area 7
	//all -> Unused 
	//[0x10000000,0x20000000) -> unused
	unused_buffer(0x10000000,0x20000000);

	sh4_ram_alt= (u8*)MapViewOfFile(mem_handle,FILE_MAP_READ |FILE_MAP_WRITE,0,0,RAM_SIZE);	//alternative ram map location, BE CAREFULL THIS BYPASSES DYNAREC PROTECTION LOGIC
	if (sh4_ram_alt==0)
		return false;

	return sh4_reserved_mem!=0;
}

#else
bool _vmem_reserve()
{
	sh4_reserved_mem=0;
	void* ptr=0;
	sh4_reserved_mem=(u8*)VirtualAlloc(0,512*1024*1024,MEM_RESERVE,PAGE_NOACCESS);
	if (sh4_reserved_mem==0)
		return false;
	//VirtualFree(sh4_reserved_mem,0,MEM_RELEASE);
	
	//Area 0
	//[0x00800000,0x00A00000);
	ptr=VirtualAlloc(&sh4_reserved_mem[0x00800000],0x00200000,MEM_COMMIT,PAGE_READWRITE);
	if (ptr==0)
		return false;
	aica_ram.size=0x00200000;
	aica_ram.data=(u8*)ptr;
	//[0 ,0x04000000) -> unused
	//ptr=VirtualAlloc(&sh4_reserved_mem[0x00000000],0x04000000,MEM_RESERVE,PAGE_NOACCESS);
	//if (ptr==0)
	//	return false;
	//Area 1
	//[0x04000000,0x05000000) -> vram | mirror
	//[0x05000000,0x06000000) -> unused
	//[0x06000000,0x07000000) -> vram   mirror
	//[0x07000000,0x08000000) -> unused mirror
	ptr=VirtualAlloc(&sh4_reserved_mem[0x04000000],VRAM_SIZE,MEM_COMMIT,PAGE_READWRITE);
	if (ptr==0)
		return false;
	vram.size=VRAM_SIZE;
	vram.data=(u8*)ptr;
/*
	//vram #0
	ptr= MapViewOfFileEx(mem_handle,FILE_MAP_READ |FILE_MAP_WRITE,0,RAM_SIZE,VRAM_SIZE,&sh4_reserved_mem[0x04000000]);
	ptr= MapViewOfFileEx(mem_handle,FILE_MAP_READ |FILE_MAP_WRITE,0,RAM_SIZE,VRAM_SIZE,&sh4_reserved_mem[0x05000000]);
	if (ptr==0)
		return false;
	vram.size=VRAM_SIZE;
	vram.data=(u8*)ptr;

	//vram #1
	ptr= MapViewOfFileEx(mem_handle,FILE_MAP_READ |FILE_MAP_WRITE,0,RAM_SIZE,VRAM_SIZE,&sh4_reserved_mem[0x06000000]);
	if (ptr==0)
		return false;
*/
	//Area 2
	//[0x08000000,0x0C000000) -> unused
	/*
	ptr=VirtualAlloc(&sh4_reserved_mem[0x08000000],0x04000000,MEM_RESERVE,PAGE_NOACCESS);
	if (ptr==0)
		return false;
	*/
	//Area 3
	//[0x0C000000,0x0D000000) -> main ram
	//[0x0D000000,0x0E000000) -> main ram mirror
	//[0x0E000000,0x0F000000) -> main ram mirror
	//[0x0F000000,0x10000000) -> main ram mirror
		//ram #0
	ptr=VirtualAlloc(&sh4_reserved_mem[0x0C000000],RAM_SIZE,MEM_COMMIT,PAGE_READWRITE);
	if (ptr==0)
		return false;
	mem_b.size=RAM_SIZE;
	mem_b.data=(u8*)ptr;
	/*
	ptr= MapViewOfFileEx(mem_handle,FILE_MAP_READ |FILE_MAP_WRITE,0,0,RAM_SIZE,&sh4_reserved_mem[0x0C000000]);
	if (ptr==0)
		return false;
	mem_b.size=RAM_SIZE;
	mem_b.data=(u8*)ptr;

	//ram #1
	ptr= MapViewOfFileEx(mem_handle,FILE_MAP_READ |FILE_MAP_WRITE,0,0,RAM_SIZE,&sh4_reserved_mem[0x0D000000]);
	if (ptr==0)
		return false;
*/
	//Area 4
	//Area 5
	//Area 6
	//Area 7
	//all -> Unused 
	//[0x10000000,0x20000000) -> unused
/*	
	ptr=VirtualAlloc(&sh4_reserved_mem[0x10000000],0x10000000,MEM_RESERVE,PAGE_NOACCESS);
	if (ptr==0)
		return false;
*/
	return sh4_reserved_mem!=0;
}

#endif
void _vmem_release()
{
	VirtualFree(sh4_reserved_mem,0,MEM_RELEASE);
	//more ? the data is freed anyway ;p
}

/*
	Some more notes :
	_vmem : for any *general* purpose access , with size 1/2/4/8/32.
	8/32 size , if not in memory , is splitted to many calls to 4 functions
	
	dynarec _vmem :
	When dynarec is using vmem , there are a few things to take in acount ...

	Access sizes are 1,2,4,8

	An access can be static (fixed address).If so , it can be fully optimised to call/movs 
		[even more if i implementthe register info table]. it is marked by an 's' (for static)

	An access can be block olny (to mapped buffers).In that case , the dynarec should provide 
		a fallback to full mode to ensure compat. It is marked by an 'n' (for native)

	An access can be anyware in the mem space.In that case , an optimised version of the full 
	_vmem lookup is generated.

	so , we have

	svmem (static  , handler)		: corrupts regs a call can corrupt
	nsvmem(static  , block)			: corrupts olny temp regs used
	nvmem (dynamic , block)			: corrupts olny temp regs used
	_vmem (dynamic , anywere)		: corrupts regs a call can corrupt

	These opcodes allways corrupt the input reg , and read/write to another reg (rm/wm).

	The memory address calculation will get a seperate opcode.

	so , we have the opcodes :
	calc_addr(...) -> temp
	r_*vmem(temp) -> reg
	w_*vmem(temp,reg) -> nil

	and the optimiser info :
	*_vmem(const) -> block(const) ? *_nsvmem : *_svmem
	*_nvmem(const) -> block(const) ? *_nsvmem : *_svmem
	
	The problem is , the current dynarec/il do not support any kind of metadata , so the implementation has to wait
	til it supports opcode/register metadata. Also , this idea fits best w/ an unlimited set of temp regs.
*/