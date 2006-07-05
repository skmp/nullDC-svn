//new memory mapping code ..."_vmem" ... Don't ask where i got the name , it somehow poped on my head :p
//
#include "_vmem.h"
#include "emitter\emitter.h"
#include <windows.h>


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

//reading from misc mem areas

//eax=function index
//ecx=full address of read
//edx=unused , but may be corrupted
void naked _vmem_ReadMisc8()
{
	__asm
	{
		//get function pointer
		mov eax , [_vmem_RF8+eax];
		jmp eax;
	}
}
void naked _vmem_ReadMisc16()
{
	__asm
	{
		//get function pointer
		mov eax , [_vmem_RF16+eax];
		jmp eax;
	}
}
void naked _vmem_ReadMisc32()
{
	__asm
	{
		//get function pointer
		mov eax , [_vmem_RF32+eax];
		jmp eax;
	}
}

//eax=function index
//ecx=full address of write
//edx=data to write
void naked _vmem_WriteMisc8()
{
	__asm
	{
		mov eax , [_vmem_WF8+eax];
		jmp eax;
	}
}

void naked _vmem_WriteMisc16()
{
	__asm
	{
		mov eax , [_vmem_WF16+eax];
		jmp eax;
	}
}

void naked _vmem_WriteMisc32()
{
	__asm
	{
		mov eax , [_vmem_WF32+eax];
		jmp eax;
	}
}

//ReadMem/WriteMem functions
//ReadMem
u8 naked __fastcall _vmem_ReadMem8(u32 Address)
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
	jmp _vmem_ReadMisc8;
direct:
	mov eax,[eax+edx];	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
	ret;
	}
}

u16 naked __fastcall _vmem_ReadMem16(u32 Address)
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
	jmp _vmem_ReadMisc16;
direct:
	mov eax,[eax+edx];	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
	ret;
	}
}

u32 naked __fastcall _vmem_ReadMem32(u32 Address)
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
	jmp _vmem_ReadMisc32;
direct:
	mov eax,[eax+edx];	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
	ret;
	}
}
//WriteMem
void naked __fastcall _vmem_WriteMem8(u32 Address,u8 data)
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
	jmp _vmem_WriteMisc8;
direct:
	and ecx,0xFFFF;//lower 16b of address
	//or eax,edx;	//get ptr to the value we want
	//mov eax,[eax]
	mov [eax+ecx],dl;
	ret;
	}
}

void naked __fastcall _vmem_WriteMem16(u32 Address,u16 data)
{
	//for comments read 8 bit version
	__asm
	{
	mov eax,ecx;
	shr eax,16;
	mov eax,[_vmem_MemInfo+eax*4];
	test eax,0xFFFF0000;
	jnz direct;
	jmp _vmem_WriteMisc16;
direct:
	and ecx,0xFFFF;
	mov [eax+ecx],dx;
	ret;
	}
}
void naked __fastcall _vmem_WriteMem32(u32 Address,u32 data)
{
	//for comments read 8 bit version
	__asm
	{
	mov eax,ecx;
	shr eax,16;
	mov eax,[_vmem_MemInfo+eax*4];
	test eax,0xFFFF0000;
	jnz direct;
	jmp _vmem_WriteMisc32;
direct:
	and ecx,0xFFFF;
	mov [eax+ecx],edx;
	ret;
	}
}


//phew .. that was lota asm code ;) lets go back to C :D
//default mem handlers ;)
//defualt read handlers
u8 __fastcall _vmem_ReadMem8_not_mapped(u32 addresss)
{
	printf("Read8 from 0x%X, not mapped [_vmem default handler]\n",addresss);
	return 0xDEADC0D3;
}
u16 __fastcall _vmem_ReadMem16_not_mapped(u32 addresss)
{
	printf("Read16 from 0x%X, not mapped [_vmem default handler]\n",addresss);
	return 0xDEADC0D3;
}
u32 __fastcall _vmem_ReadMem32_not_mapped(u32 addresss)
{
	printf("Read32 from 0x%X, not mapped [_vmem default handler]\n",addresss);
	return 0xDEADC0D3;
}
//defualt write handers
void __fastcall _vmem_WriteMem8_not_mapped(u32 addresss,u8 data)
{
	printf("Write8 to 0x%X=0x%X, not mapped [_vmem default handler]\n",addresss,data);
}
void __fastcall _vmem_WriteMem16_not_mapped(u32 addresss,u16 data)
{
	printf("Write16 to 0x%X=0x%X, not mapped [_vmem default handler]\n",addresss,data);
}
void __fastcall _vmem_WriteMem32_not_mapped(u32 addresss,u32 data)
{
	printf("Write32 to 0x%X=0x%X, not mapped [_vmem default handler]\n",addresss,data);
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
		_vmem_MemInfo[i]=(void*)(Handler*4);
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
//benchmarking functions
u32 __fastcall _vmem_ReadMem32_bench(u32 addresss)
{
	return 0x1;
}
void _vmem_Benchmark()
{
	u32 hanld=_vmem_register_handler(0,0,_vmem_ReadMem32_bench,0,0,0);
	_vmem_map_handler(hanld,0,9);

	{
		u32 bstart=GetTickCount();

		u32 rez=0;
		for (int i=0;i<20000000;i++)
		{
			rez+=_vmem_ReadMem32((0x666+i)&0xFFF);
		}
		u32 bdur=GetTickCount()-bstart;
		float secs=(float)bdur/1000.0f;
		secs=20000000/secs;
		secs/=1024*1024;
		printf("vmem : %f MB/s from function , value=%d\n",secs*4,rez);
	}

	void* pm=malloc(10*0x10000);

	memset(pm,0,10*0x10000);
	_vmem_map_block((u8*)pm,0,9);

	{
		u32 bstart=GetTickCount();

		u32 rez=0;
		for (int i=0;i<20000000;i++)
		{
			rez+=_vmem_ReadMem32((0x666+i)&0xFFF);
		}
		u32 bdur=GetTickCount()-bstart;
		float secs=(float)bdur/1000.0f;
		secs=20000000/secs;
		secs/=1024*1024;
		printf("vmem : %f MB/s from Array , value=%d\n",secs*4,rez);
	}

	free(pm);
}
//init/reset/term
void _vmem_init()
{
	_vmem_reset();
	_vmem_Benchmark();
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
	
	/*_vmem_ReadMem8(0xFFFFFFFF);
	_vmem_ReadMem8(0xDEADBEEF);
	_vmem_ReadMem16(0x2DEABEEF);
	_vmem_ReadMem32(0x4DEBEEF);

	_vmem_WriteMem8(0xDEADBEEF,0xBE);
	_vmem_WriteMem16(0x2DEABEEF,0xDEED);
	_vmem_WriteMem32(0x4DEABEEF,0xCAFECAFE);*/
}

void _vmem_term()
{

}

/*
04523993  mov         edx,ecx 
04523995  mov         eax,ecx 
04523997  and         edx,0FFFFh 
0452399D  shl         edx,10h 
045239A0  mov         eax,dword ptr [eax*4+54A810h] 
045239A7  test        eax,0FFFF0000h 
045239AC  jne         045239B3 
045239AE  jmp         _vmem_ReadMisc32 (40FFA0h) 
045239B3  mov         eax,dword ptr [eax+edx*2] 
045239B6  mov         esi,eax 
*/
//emitters
void _vmem_EmitReadMem(void* px86e,u32 addr,u32 out,u32 sz)
{	
	emitter<>* x86e=(emitter<>*)px86e;

	if (sz==1)
		x86e->CALLFunc(_vmem_ReadMem8);
	else if (sz==2)
		x86e->CALLFunc(_vmem_ReadMem16);
	else
		x86e->CALLFunc(_vmem_ReadMem32);
return;
	x86e->INT3();
//	//copy address
//	mov edx,ecx;//this is done here , among w/ the and , it should be possible to fully execute it on paraler (no depency)
	x86e->MOV32RtoR(EDX,(x86IntRegType)addr);
//	mov eax,ecx;
	x86e->MOV32RtoR(EAX,(x86IntRegType)addr);
	if ((x86IntRegType)addr!=ECX)
		x86e->MOV32RtoR(ECX,(x86IntRegType)addr);
//	and edx,0xFFFF;//lower 16b of address
	x86e->AND32ItoR(EDX,0xFFFF);
//	//get upper 16 bits
//	shr eax,16;
	x86e->SHR32ItoR(EAX,16);
//
//	//read mem info
//	mov eax,[_vmem_MemInfo+eax*4];
	//8B 04 85 base_address
	x86e->write8(0x8b);
	x86e->write8(0x04);
	x86e->write8(0x85);
	x86e->write32((u32)&_vmem_MemInfo[0]);
//	test eax,0xFFFF0000;
	x86e->TEST32ItoR(EAX,0xFFFF0000);
//	jnz direct;
	u8* l_direct=x86e->JNZ8(0);
//	jmp _vmem_ReadMisc8;
	if (sz==1)
		x86e->CALLFunc(_vmem_ReadMisc8);
	else if (sz==2)
		x86e->CALLFunc(_vmem_ReadMisc16);
	else
		x86e->CALLFunc(_vmem_ReadMisc32);
	u8* p2=x86e->JMP8(0);
//direct:
	x86e->x86SetJ8(l_direct);
	//mov eax,[eax+edx];	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
	x86e->MOV32RmStoR((x86IntRegType)out,EAX,EDX,1);
	//ret;
	x86e->x86SetJ8(p2);
}
void _vmem_EmitWriteMem(emitter<>* x86e,u32 addr,u32 data,u32 sz)
{
}