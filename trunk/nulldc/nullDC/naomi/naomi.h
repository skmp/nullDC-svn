/*
**	naomi.h
*/

#pragma once
#ifdef BUILD_NAOMI





u32  ReadMem_naomi(u32 Addr, u32 sz);
void WriteMem_naomi(u32 Addr, u32 data, u32 sz);

u32  ReadMem_Xicor(u32 Addr, u32 sz);
void WriteMem_Xicor(u32 Addr, u32 data, u32 sz);


#endif





















