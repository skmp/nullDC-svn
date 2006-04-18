#include "shil_compile_slow.h"

typedef void __fastcall shil_compileFP(shil_opcode* op);

void __fastcall shil_compile_nimp(shil_opcode* op)
{
	printf("SHIL \"%s\" not recompiled\n",GetShilName((shil_opcodes)op->opcode));
}

void __fastcall shil_compile_mov(shil_opcode* op)
{
	printf("SHIL \"%s\" not recompiled\n",GetShilName((shil_opcodes)op->opcode));
}

shil_compileFP* sclt[32]=
{
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp
};

void SetH(shil_opcodes op,shil_compileFP* ha)
{
	if (op>31)
	{
		printf("SHIL COMPILER ERROR\n");
	}
	if (sclt[op]!=shil_compile_nimp)
	{
		printf("SHIL COMPILER ERROR [hash table overwrite]\n");
	}

	sclt[op]=ha;
}
void Init()
{
	SetH(shil_opcodes::adc,shil_compile_nimp);
	SetH(shil_opcodes::add,shil_compile_nimp);
	SetH(shil_opcodes::and,shil_compile_nimp);
	SetH(shil_opcodes::cmp,shil_compile_nimp);
	SetH(shil_opcodes::fabs,shil_compile_nimp);
	SetH(shil_opcodes::fadd,shil_compile_nimp);
	SetH(shil_opcodes::fdiv,shil_compile_nimp);
	SetH(shil_opcodes::fmac,shil_compile_nimp);

	SetH(shil_opcodes::fmul,shil_compile_nimp);
	SetH(shil_opcodes::fneg,shil_compile_nimp);
	SetH(shil_opcodes::fsub,shil_compile_nimp);
	SetH(shil_opcodes::LoadT,shil_compile_nimp);
	SetH(shil_opcodes::mov,shil_compile_mov);
	SetH(shil_opcodes::movex,shil_compile_nimp);
	SetH(shil_opcodes::neg,shil_compile_nimp);
	SetH(shil_opcodes::not,shil_compile_nimp);

	SetH(shil_opcodes::or,shil_compile_nimp);
	SetH(shil_opcodes::rcl,shil_compile_nimp);
	SetH(shil_opcodes::rcr,shil_compile_nimp);
	SetH(shil_opcodes::readm,shil_compile_nimp);
	SetH(shil_opcodes::rol,shil_compile_nimp);
	SetH(shil_opcodes::ror,shil_compile_nimp);
	SetH(shil_opcodes::sar,shil_compile_nimp);
	SetH(shil_opcodes::SaveT,shil_compile_nimp);

	SetH(shil_opcodes::shil_ifb,shil_compile_nimp);
	SetH(shil_opcodes::shl,shil_compile_nimp);
	SetH(shil_opcodes::shr,shil_compile_nimp);
	SetH(shil_opcodes::sub,shil_compile_nimp);
	SetH(shil_opcodes::swap,shil_compile_nimp);
	SetH(shil_opcodes::test,shil_compile_nimp);
	SetH(shil_opcodes::writem,shil_compile_nimp);
	SetH(shil_opcodes::xor,shil_compile_nimp);

}
void CompileBasicBlock_slow(rec_v1_BasicBlock* block)
{
	Init();
	for (int i=0;i<block->ilst.opcodes.size();i++)
	{
		shil_opcode* op=&block->ilst.opcodes[i];
		if (op->opcode>31)
		{
			printf("SHIL COMPILER ERROR\n");
		}
		sclt[op->opcode](op);
	}
}