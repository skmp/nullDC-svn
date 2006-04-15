#include "shil.h"

//IL and related code

INLINE bool shil_opcode::ReadsReg(Sh4RegType reg)
{
	return true;
}
INLINE bool shil_opcode::OverwritesReg(Sh4RegType reg)
{
	return true;
}
INLINE bool shil_opcode::UpdatesReg(Sh4RegType reg)
{
	return true;
}

void shil_stream::emit(shil_opcodes op,u16 flags,u32 source,u32 dest)
{
}

void shil_stream::mov(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::mov(Sh4RegType to,u32 from)
{
}

/*** Mem reads ***/
//readmem [reg]
void shil_stream::readm8(Sh4RegType to,Sh4RegType from,bool sx)
{
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType from,bool sx)
{
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::readm64(Sh4RegType to,Sh4RegType from)
{
}

//readmem [const]
void shil_stream::readm8(Sh4RegType to,u32 from,bool sx)
{
}
void shil_stream::readm16(Sh4RegType to,u32  from,bool sx)
{
}
void shil_stream::readm32(Sh4RegType to,u32  from)
{
}

//readmem base[offset]
void shil_stream::readm8(Sh4RegType to,Sh4RegType base,Sh4RegType offset,bool sx)
{
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType base,Sh4RegType offset,bool sx)
{
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
}
void shil_stream::readm64(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
}

/*** Mem writes ***/
//readmem base[const]
void shil_stream::readm8(Sh4RegType to,Sh4RegType base,u32 offset,bool sx)
{
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType base,u32 offset,bool sx)
{
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType base,u32 offset)
{
}

//writemem [reg]
void shil_stream::writem8(Sh4RegType from,Sh4RegType to,bool sx)
{
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType to,bool sx)
{
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType to)
{
}
void shil_stream::writem64(Sh4RegType from,Sh4RegType to)
{
}

//writemem [const]
void shil_stream::writem8(Sh4RegType from,u32 to,bool sx)
{
}
void shil_stream::writem16(Sh4RegType from,u32  to,bool sx)
{
}
void shil_stream::writem32(Sh4RegType from,u32  to)
{
}

//writemem base[offset]
void shil_stream::writem8(Sh4RegType from,Sh4RegType base,Sh4RegType offset,bool sx)
{
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType base,Sh4RegType offset,bool sx)
{
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{
}
void shil_stream::writem64(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{
}

//writemem base[const]
void shil_stream::writem8(Sh4RegType from,Sh4RegType base,u32 offset,bool sx)
{
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType base,u32 offset,bool sx)
{
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType base,u32 offset)
{
}


void shil_stream::cmp(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::cmp(Sh4RegType to,s8 from)
{
}
void shil_stream::cmp(Sh4RegType to,u8 from)
{
}

void shil_stream::test(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::test(Sh4RegType to,u8 from)
{
}

void shil_stream::SaveT(cmd_cond cond)
{
}
void shil_stream::LoadT(x86_flags to)
{
}


void shil_stream::dec(Sh4RegType to)
{
}
void shil_stream::inc(Sh4RegType to)
{
}
void shil_stream::neg(Sh4RegType to)
{
}
void shil_stream::not(Sh4RegType to)
{
}

//logical shifts
void shil_stream::shl(Sh4RegType to,u8 count)
{
}
void shil_stream::shr(Sh4RegType to,u8 count)
{
}

//arithmetic shifts
void shil_stream::sal(Sh4RegType to,u8 count)
{
}//<- is this used ?
void shil_stream::sar(Sh4RegType to,u8 count)
{
}

//rotate

void shil_stream::rcl(Sh4RegType to,u8 count)
{
}
void shil_stream::rcr(Sh4RegType to,u8 count)
{
}
void shil_stream::rol(Sh4RegType to,u8 count)
{
}
void shil_stream::ror(Sh4RegType to,u8 count)
{
}

//swaps
void shil_stream::bswap(Sh4RegType to)
{
}
void shil_stream::wswap(Sh4RegType to)
{
}

//extends
//signed
void shil_stream::movsxb(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::movsxw(Sh4RegType to,Sh4RegType from)
{
}
//unsigned
void shil_stream::movzxb(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::movzxw(Sh4RegType to,Sh4RegType from)
{
}

//maths (integer)
void shil_stream::adc(Sh4RegType to,Sh4RegType from)
{
}

void shil_stream::add(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::add(Sh4RegType to,u32 from)
{
}
void shil_stream::sub(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::sub(Sh4RegType to,u32 from)
{
}

//floating
void shil_stream::fadd(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::fsub(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::fmul(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::fmac(Sh4RegType to,Sh4RegType from)
{
}
void shil_stream::fdiv(Sh4RegType to,Sh4RegType from)
{
}

void shil_stream::fabs(Sh4RegType to)
{
}
void shil_stream::fneg(Sh4RegType to)
{
}
