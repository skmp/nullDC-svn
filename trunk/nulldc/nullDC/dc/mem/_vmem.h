#pragma once
#include "types.h"
#include "sh4_mem.h"

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
void _vmem_map_handler(_vmem_handler Handler,u32 start,u32 end);
void _vmem_map_block(void* base,u32 start,u32 end);

//ReadMem(s)
u8 __fastcall _vmem_ReadMem8(u32 Address);
u16 __fastcall _vmem_ReadMem16(u32 Address);
u32 __fastcall _vmem_ReadMem32(u32 Address);
//WriteMem(s)
void __fastcall _vmem_WriteMem8(u32 Address,u8 data);
void __fastcall _vmem_WriteMem16(u32 Address,u16 data);
void __fastcall _vmem_WriteMem32(u32 Address,u32 data);

//Emitters
void _vmem_EmitReadMem(void* x86e,u32 addr,u32 out,u32 sz);
void _vmem_EmitWriteMem(void* x86e,u32 addr,u32 data,u32 sz);