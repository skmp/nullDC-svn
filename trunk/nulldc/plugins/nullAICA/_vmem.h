#pragma once
#include "aica.h"

//Typedef's
//ReadMem 
typedef u8 __fastcall _vmem_ReadMem8FP(u32 Address);
typedef u16 __fastcall _vmem_ReadMem16FP(u32 Address);
typedef u32 __fastcall _vmem_ReadMem32FP(u32 Address);
//WriteMem
typedef void __fastcall _vmem_WriteMem8FP(u32 Address,u8 data);
typedef void __fastcall _vmem_WriteMem16FP(u32 Address,u16 data);
typedef void __fastcall _vmem_WriteMem32FP(u32 Address,u32 data);

//our own handle type :)
typedef u32 _vmem_handler;

//Functions

//init/reset/term
void _vmem_init();
void _vmem_reset();
void _vmem_term();

//functions to register and map handlers/memory
_vmem_handler _vmem_register_handler(_vmem_ReadMem8FP* read8,_vmem_ReadMem16FP* read16,_vmem_ReadMem32FP* read32, _vmem_WriteMem8FP* write8,_vmem_WriteMem16FP* write16,_vmem_WriteMem32FP* write32);

#define  _vmem_register_handler_Template(read,write) _vmem_register_handler \
									(read<1,u8>,read<2,u16>,read<4,u32>,	\
									write<1,u8>,write<2,u16>,write<4,u32>)	

#define  _vmem_register_handler_Template1(read,write,extra_Tparam) _vmem_register_handler \
									(read<1,u8,extra_Tparam>,read<2,u16,extra_Tparam>,read<4,u32,extra_Tparam>,	\
									write<1,u8,extra_Tparam>,write<2,u16,extra_Tparam>,write<4,u32,extra_Tparam>)	

#define  _vmem_register_handler_Template2(read,write,etp1,etp2) _vmem_register_handler \
									(read<1,u8,etp1,etp2>,read<2,u16,etp1,etp2>,read<4,u32,etp1,etp2>,	\
									write<1,u8,etp1,etp2>,write<2,u16,etp1,etp2>,write<4,u32,etp1,etp2>)	

void _vmem_map_handler(_vmem_handler Handler,u32 start,u32 end);
void _vmem_map_block(void* base,u32 start,u32 end);
void _vmem_mirror_mapping(u32 new_region,u32 start,u32 size);

//ReadMem(s)
u8 __fastcall _vmem_ReadMem8(u32 Address);
u16 __fastcall _vmem_ReadMem16(u32 Address);
u32 __fastcall _vmem_ReadMem32(u32 Address);
//WriteMem(s)
void __fastcall _vmem_WriteMem8(u32 Address,u8 data);
void __fastcall _vmem_WriteMem16(u32 Address,u16 data);
void __fastcall _vmem_WriteMem32(u32 Address,u32 data);