#include "shil.h"

//IL and related code
//shil opcode forms:

//[]-> optional
//operations on a single register
//op reg,[imm32]
//op [imm32],reg

//operations on 2 regs
//op reg reg [imm32]
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

void shil_stream::emit32(shil_opcodes op,u32 imm1)
{
	emit(op,NoReg,NoReg,imm1,0,FLAG_32 | FLAG_IMM1);
}


void shil_stream::emit32(shil_opcodes op,Sh4RegType reg1)
{
	emit(op,reg1,NoReg,0,0,FLAG_32 | FLAG_REG1);
}


void shil_stream::emit32(shil_opcodes op,Sh4RegType reg1,u32 imm1)
{
	emit(op,reg1,NoReg,imm1,0,FLAG_32 | FLAG_REG1| FLAG_IMM1);
}

void shil_stream::emit32(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2)
{
	emit(op,reg1,reg2,0,0,FLAG_32 | FLAG_REG1| FLAG_REG2);
}


void shil_stream::emit(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2,u32 imm1,u32 imm2,u16 flags)
{
	op_count++;
	shil_opcode sh_op;

	sh_op.opcode=(u16)op;
	sh_op.reg1=(u8)reg1;
	sh_op.reg2=(u8)reg2;
	sh_op.imm1=imm1;
	sh_op.imm2=imm2;
	sh_op.flags=flags;

	opcodes.push_back(sh_op);
}

void shil_stream::emitReg(shil_opcodes op,Sh4RegType reg1,u16 flags)
{
	//emit
	emit(op,reg1,NoReg,0,0,flags|FLAG_REG1);
}

void shil_stream::emitRegImm(shil_opcodes op,Sh4RegType reg1,u32 imm1,u16 flags)
{
	//emit
	emit(op,reg1,NoReg,imm1,0,flags|FLAG_IMM1|FLAG_REG1);
}

void shil_stream::emitRegImmImm(shil_opcodes op,Sh4RegType reg1,u32 imm1,u32 imm2,u16 flags)
{
	//emit
	emit(op,reg1,NoReg,imm1,imm2,flags|FLAG_IMM1|FLAG_IMM2|FLAG_REG1);
}

void shil_stream::emitRegReg(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2,u16 flags)
{
	//emit
	emit(op,reg1,reg2,0,0,flags|FLAG_REG1|FLAG_REG2);
}
void shil_stream::emitRegRegImm(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2,u32 imm1,u16 flags)
{
	//emit
	emit(op,reg1,reg2,imm1,0,flags|FLAG_IMM1|FLAG_REG1|FLAG_REG2);
}
void shil_stream::mov(Sh4RegType to,Sh4RegType from)
{
	//emit32(shil_opcodes::mov,to,from);
}
void shil_stream::mov(Sh4RegType to,u32 from)
{
	//emit(shil_opcodes::mov,to,from);
}

/*** Mem reads ***/
u16 GetBaseFlags(Sh4RegType base)
{
	if (base!=r0 && base!=reg_gbr )
	{
		printf("SHIL ERROR\n");
	}

	if (base==r0)
		return FLAG_R0;
	else if (base==reg_gbr)
		return FLAG_GBR;
}
//readmem [const]
void shil_stream::readm8(Sh4RegType to,u32 from)
{
	emitRegImm(shil_opcodes::readm,to,from,FLAG_8|FLAG_SX);
}
void shil_stream::readm16(Sh4RegType to,u32 from)
{
	emitRegImm(shil_opcodes::readm,to,from,FLAG_16|FLAG_SX);
}
void shil_stream::readm32(Sh4RegType to,u32 from)
{
	emitRegImm(shil_opcodes::readm,to,from,FLAG_32);
}
void shil_stream::readm64(Sh4RegType to,u32 from)
{
	//emit(shil_opcodes::readm,to,from,FLAG_64);
}

//readmem [reg]
void shil_stream::readm8(Sh4RegType to,Sh4RegType from)
{
	readm8(to,from,0);
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType from)
{
	readm16(to,from,0);
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType from)
{
	readm32(to,from,0);
}
void shil_stream::readm64(Sh4RegType to,Sh4RegType from)
{
	//emit(shil_opcodes::readm,to,from,FLAG_64);
}

//readmem base[offset]
void shil_stream::readm8(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shil_opcodes::readm,to,offset,FLAG_8|FLAG_SX|GetBaseFlags(base));
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shil_opcodes::readm,to,offset,FLAG_16|FLAG_SX|GetBaseFlags(base));
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shil_opcodes::readm,to,offset,FLAG_32|GetBaseFlags(base));
}
void shil_stream::readm64(Sh4RegType to,Sh4RegType base,Sh4RegType offset)
{
//	emit(shil_opcodes::add,0,0,0);
}


//readmem base[const]
void shil_stream::readm8(Sh4RegType to,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shil_opcodes::readm,to,base,offset,FLAG_8|FLAG_SX);
}
void shil_stream::readm16(Sh4RegType to,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shil_opcodes::readm,to,base,offset,FLAG_16|FLAG_SX);
}
void shil_stream::readm32(Sh4RegType to,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shil_opcodes::readm,to,base,offset,FLAG_32);
}


/*** Mem writes ***/

//writemem [const]
void shil_stream::writem8(Sh4RegType from,u32 to)
{
	emitRegImm(shil_opcodes::writem,from,to,FLAG_8|FLAG_SX);
}
void shil_stream::writem16(Sh4RegType from,u32 to)
{
	emitRegImm(shil_opcodes::writem,from,to,FLAG_16|FLAG_SX);
}
void shil_stream::writem32(Sh4RegType from,u32 to)
{
	emitRegImm(shil_opcodes::writem,from,to,FLAG_32);
}
void shil_stream::writem64(Sh4RegType from,u32 to)
{
	//writem8(from,to,0,sx);
}

//writemem [reg]
void shil_stream::writem8(Sh4RegType from,Sh4RegType to)
{
//	emit(shil_opcodes::add,0,0,0);
	writem8(from,to,0);
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType to)
{
	writem16(from,to,0);
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType to)
{
	writem32(from,to,0);
}
void shil_stream::writem64(Sh4RegType from,Sh4RegType to)
{
	//writem8(from,to,0,sx);
}

//writemem reg[reg]
void shil_stream::writem8(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shil_opcodes::writem,from,offset,FLAG_8|GetBaseFlags(base));
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shil_opcodes::writem,from,offset,FLAG_16|GetBaseFlags(base));
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{
	emitRegReg(shil_opcodes::writem,from,offset,FLAG_32|GetBaseFlags(base));
}
void shil_stream::writem64(Sh4RegType from,Sh4RegType base,Sh4RegType offset)
{
//	emit(shil_opcodes::add,0,0,0);
}

//writemem reg[const]
void shil_stream::writem8(Sh4RegType from,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shil_opcodes::writem,from,base,offset,FLAG_8);
}
void shil_stream::writem16(Sh4RegType from,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shil_opcodes::writem,from,base,offset,FLAG_16);
}
void shil_stream::writem32(Sh4RegType from,Sh4RegType base,u32 offset)
{
	emitRegRegImm(shil_opcodes::writem,from,base,offset,FLAG_32);
}

//compares
void shil_stream::cmp(Sh4RegType to,Sh4RegType from)
{
	emit32(shil_opcodes::cmp,to,from);
}
void shil_stream::cmp(Sh4RegType to,s8 from)
{
	emit32(shil_opcodes::cmp,to,from);
}

void shil_stream::test(Sh4RegType to,Sh4RegType from)
{
	emit32(shil_opcodes::test,to,from);
}
void shil_stream::test(Sh4RegType to,u8 from)
{
	emit32(shil_opcodes::test,to,from);
}

void shil_stream::SaveT(cmd_cond cond)
{
	emit32(shil_opcodes::SaveT,(u32)cond);
}
void shil_stream::LoadT(x86_flags to)
{
	emit32(shil_opcodes::LoadT,(u32)to);
}


void shil_stream::dec(Sh4RegType to)
{
	sub(to,1);
}
void shil_stream::inc(Sh4RegType to)
{
	add(to,1);
}
void shil_stream::neg(Sh4RegType to)
{
	emit32(shil_opcodes::neg,to);

}
void shil_stream::not(Sh4RegType to)
{
	emit32(shil_opcodes::not,to);
}

//bitwise ops
void shil_stream::and(Sh4RegType to,Sh4RegType from)
{
	emit32(shil_opcodes::and,to,from);
}
void shil_stream::and(Sh4RegType to,u32 from)
{
	emit32(shil_opcodes::and,to,from);
}
void shil_stream::or(Sh4RegType to,Sh4RegType from)
{
	emit32(shil_opcodes::or,to,from);
}
void shil_stream::or(Sh4RegType to,u32 from)
{
	emit32(shil_opcodes::or,to,from);
}
void shil_stream::xor(Sh4RegType to,Sh4RegType from)
{
	emit32(shil_opcodes::xor,to,from);
}
void shil_stream::xor(Sh4RegType to,u32 from)
{
	emit32(shil_opcodes::xor,to,from);
}

//logical shifts
void shil_stream::shl(Sh4RegType to,u8 count)
{
	emit32(shil_opcodes::shl,to,count);
}
void shil_stream::shr(Sh4RegType to,u8 count)
{
	emit32(shil_opcodes::shr,to,count);
}

//arithmetic shifts
void shil_stream::sal(Sh4RegType to,u8 count)
{//<- is this used ?
	shl(to,count);
}
void shil_stream::sar(Sh4RegType to,u8 count)
{
	emit32(shil_opcodes::sar,to,count);
}

//rotate

void shil_stream::rcl(Sh4RegType to,u8 count)
{
	emit32(shil_opcodes::rcl,to,count);
}
void shil_stream::rcr(Sh4RegType to,u8 count)
{
	emit32(shil_opcodes::rcr,to,count);
}
void shil_stream::rol(Sh4RegType to,u8 count)
{
	emit32(shil_opcodes::rol,to,count);
}
void shil_stream::ror(Sh4RegType to,u8 count)
{
	emit32(shil_opcodes::ror,to,count);
}

//swaps
void shil_stream::bswap(Sh4RegType to)
{
	emitReg(shil_opcodes::swap,to,FLAG_8);
}
void shil_stream::wswap(Sh4RegType to)
{
	emitReg(shil_opcodes::swap,to,FLAG_16);
}

//extends
//signed
void shil_stream::movsxb(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::movex,to,from,0,0,FLAG_SX | FLAG_8);
}
void shil_stream::movsxw(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::movex,to,from,0,0,FLAG_SX | FLAG_16);
}
//unsigned
void shil_stream::movzxb(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::movex,to,from,0,0,FLAG_ZX | FLAG_8);
}
void shil_stream::movzxw(Sh4RegType to,Sh4RegType from)
{
	emit(shil_opcodes::movex,to,from,0,0,FLAG_ZX | FLAG_16);
}

//maths (integer)
void shil_stream::adc(Sh4RegType to,Sh4RegType from)
{
	emit32(shil_opcodes::adc,to,from);
}

void shil_stream::add(Sh4RegType to,Sh4RegType from)
{
	emit32(shil_opcodes::add,to,from);
}
void shil_stream::add(Sh4RegType to,u32 from)
{
	emit32(shil_opcodes::add,to,from);
}
void shil_stream::sub(Sh4RegType to,Sh4RegType from)
{
	emit32(shil_opcodes::sub,to,from);
}
void shil_stream::sub(Sh4RegType to,u32 from)
{
	emit32(shil_opcodes::sub,to,from);
}

//floating

bool IsReg64(Sh4RegType reg)
{
	if (reg>=dr_0 && reg<=dr_7)
		return true;

	if (reg>=xd_0 && reg<=xd_7)
		return true;
}

u16 shil_stream::GetFloatFlags(Sh4RegType reg1,Sh4RegType reg2)
{
	u32 rv=0;

	if (reg1==NoReg)
	{
		//error
	}
	//reg1 allways used
	rv|=FLAG_REG1;

	if (reg2!=NoReg)
		rv|=FLAG_REG2;

	if (IsReg64(reg1))
	{
		if (rv&FLAG_REG2)
		{
			if (!IsReg64(reg2))
			{
				//both operands need to be 64bit on float,float
				//?
			}
		}

		rv|=FLAG_64;
	}
	else
	{
		if (rv&FLAG_REG2)
		{
			if (IsReg64(reg2))
			{
				//both operands need to be 64bit on float,float
				//?
			}
		}
		rv|=FLAG_64;
	}

	return rv;
}
void shil_stream::fadd(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shil_opcodes::fadd,to,from,GetFloatFlags(to,from));
}
void shil_stream::fsub(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shil_opcodes::fsub,to,from,GetFloatFlags(to,from));
}
void shil_stream::fmul(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shil_opcodes::fmul,to,from,GetFloatFlags(to,from));
}
void shil_stream::fmac(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shil_opcodes::fmac,to,from,GetFloatFlags(to,from));
}
void shil_stream::fdiv(Sh4RegType to,Sh4RegType from)
{
	emitRegReg(shil_opcodes::fdiv,to,from,GetFloatFlags(to,from));
}

void shil_stream::fabs(Sh4RegType to)
{
	emitReg(shil_opcodes::fabs,to,GetFloatFlags(to,NoReg));
}
void shil_stream::fneg(Sh4RegType to)
{
	emitReg(shil_opcodes::fneg,to,GetFloatFlags(to,NoReg));
}

void shil_stream::shil_ifb(u32 opcode,u32 pc)
{
	emit(shil_opcodes::shil_ifb,NoReg,NoReg,opcode,pc,FLAG_IMM1|FLAG_IMM2);	//size flags ect are ignored on this opcode
}