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
	op_count++;
}

void shil_stream::mov(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::mov(Sh4RegType to,u32 from)
{
	emit(shil_opcodes::add,0,0,0);
}

/*** Mem reads ***/
//readmem [reg]
void shil_stream::readm8(Sh4RegType to,Sh4RegType from,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType from,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::readm64(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}

//readmem [const]
void shil_stream::readm8(Sh4RegType to,u32 from,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::readm16(Sh4RegType to,u32  from,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::readm32(Sh4RegType to,u32  from)
{
	emit(shil_opcodes::add,0,0,0);
}

//readmem base[offset]
void shil_stream::readm8(Sh4RegType to,Sh4RegType base,Sh4RegType offset,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType base,Sh4RegType offset,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::readm64(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
	emit(shil_opcodes::add,0,0,0);
}

/*** Mem writes ***/
//readmem base[const]
void shil_stream::readm8(Sh4RegType to,Sh4RegType base,u32 offset,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType base,u32 offset,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType base,u32 offset)
{
	emit(shil_opcodes::add,0,0,0);
}

//writemem [reg]
void shil_stream::writem8(Sh4RegType from,Sh4RegType to,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType to,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType to)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::writem64(Sh4RegType from,Sh4RegType to)
{
	emit(shil_opcodes::add,0,0,0);
}

//writemem [const]
void shil_stream::writem8(Sh4RegType from,u32 to,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::writem16(Sh4RegType from,u32  to,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::writem32(Sh4RegType from,u32  to)
{
	emit(shil_opcodes::add,0,0,0);
}

//writemem base[offset]
void shil_stream::writem8(Sh4RegType from,Sh4RegType base,Sh4RegType offset,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType base,Sh4RegType offset,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::writem64(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{
	emit(shil_opcodes::add,0,0,0);
}

//writemem base[const]
void shil_stream::writem8(Sh4RegType from,Sh4RegType base,u32 offset,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType base,u32 offset,bool sx)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType base,u32 offset)
{
	emit(shil_opcodes::add,0,0,0);
}


void shil_stream::cmp(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::cmp(Sh4RegType to,s8 from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::cmp(Sh4RegType to,u8 from)
{
	emit(shil_opcodes::add,0,0,0);
}

void shil_stream::test(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::test(Sh4RegType to,u8 from)
{
	emit(shil_opcodes::add,0,0,0);
}

void shil_stream::SaveT(cmd_cond cond)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::LoadT(x86_flags to)
{
	emit(shil_opcodes::add,0,0,0);
}


void shil_stream::dec(Sh4RegType to)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::inc(Sh4RegType to)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::neg(Sh4RegType to)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::not(Sh4RegType to)
{
	emit(shil_opcodes::add,0,0,0);
}

//bitwise ops
void shil_stream::and(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::and(Sh4RegType to,u32 from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::or(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::or(Sh4RegType to,u32 from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::xor(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::xor(Sh4RegType to,u32 from)
{
	emit(shil_opcodes::add,0,0,0);
}

//logical shifts
void shil_stream::shl(Sh4RegType to,u8 count)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::shr(Sh4RegType to,u8 count)
{
	emit(shil_opcodes::add,0,0,0);
}

//arithmetic shifts
void shil_stream::sal(Sh4RegType to,u8 count)
{
	emit(shil_opcodes::add,0,0,0);
}//<- is this used ?
void shil_stream::sar(Sh4RegType to,u8 count)
{
	emit(shil_opcodes::add,0,0,0);
}

//rotate

void shil_stream::rcl(Sh4RegType to,u8 count)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::rcr(Sh4RegType to,u8 count)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::rol(Sh4RegType to,u8 count)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::ror(Sh4RegType to,u8 count)
{
	emit(shil_opcodes::add,0,0,0);
}

//swaps
void shil_stream::bswap(Sh4RegType to)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::wswap(Sh4RegType to)
{
	emit(shil_opcodes::add,0,0,0);
}

//extends
//signed
void shil_stream::movsxb(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::movsxw(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
//unsigned
void shil_stream::movzxb(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::movzxw(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}

//maths (integer)
void shil_stream::adc(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}

void shil_stream::add(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::add(Sh4RegType to,u32 from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::sub(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::sub(Sh4RegType to,u32 from)
{
	emit(shil_opcodes::add,0,0,0);
}

//floating
void shil_stream::fadd(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::fsub(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::fmul(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::fmac(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::fdiv(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::add,0,0,0);
}

void shil_stream::fabs(Sh4RegType to)
{
	emit(shil_opcodes::add,0,0,0);
}
void shil_stream::fneg(Sh4RegType to)
{
	emit(shil_opcodes::add,0,0,0);
}

void shil_stream::shil_ifb(u32 opcode,u32 pc)
{
	emit(shil_opcodes::add,0,0,0);
}