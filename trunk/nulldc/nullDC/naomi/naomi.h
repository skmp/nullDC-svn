/*
**	naomi.h
*/

#pragma once
#ifdef BUILD_NAOMI




void naomi_reg_Init();
void naomi_reg_Term();
void naomi_reg_Reset(bool Manual);


u32  naomi_reg_ReadMem(u32 Addr, u32 sz);
void naomi_reg_WriteMem(u32 Addr, u32 data, u32 sz);

u32  naomi_ReadXicor(u32 Addr, u32 sz);
void naomi_WriteXicor(u32 Addr, u32 data, u32 sz);


#endif





















