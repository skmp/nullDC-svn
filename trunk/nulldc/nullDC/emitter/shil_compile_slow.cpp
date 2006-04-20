#include "shil_compile_slow.h"
#include <assert.h>
#include "emitter.h"

#include "dc\sh4\sh4_registers.h"
#include "dc\sh4\sh4_opcode_list.h"
#include "dc\mem\sh4_mem.h"

//TEMP!!!
emitter<>* x86e;

typedef void __fastcall shil_compileFP(shil_opcode* op,rec_v1_BasicBlock* block);

//a few helpers
u32* GetRegPtr(u8 reg)
{
	u32* rv=Sh4_int_GetRegisterPtr((Sh4RegType)reg);
	assert(rv!=0);
	return rv;
}

void LoadReg(x86IntRegType to,u8 reg)
{
	x86e->MOV32MtoR(to,GetRegPtr(reg));
}

void SaveReg(u8 reg,x86IntRegType from)
{
	x86e->MOV32RtoM(GetRegPtr(reg),from);
}

void SaveReg(u8 reg,u32 from)
{
	x86e->MOV32ItoM(GetRegPtr(reg),from);
}

//Original :
	/*
	assert(FLAG_32==(op->flags & 3));//32b olny


	assert(0==(op->flags & (FLAG_IMM2)));//no imm2
	assert(op->flags & FLAG_REG1);//reg1

	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & FLAG_REG2));//no reg2
		LoadReg(EAX,op->reg1);
		x86e->SUB32ItoR(EAX,op->imm1);
		SaveReg(op->reg1,EAX);
	}
	else
	{
		assert(op->flags & FLAG_REG2);//reg2

		LoadReg(EAX,op->reg1);
		x86e->SUB32MtoR(EAX,GetRegPtr(op->reg2));
		SaveReg(op->reg1,EAX);
	}*/

//Macro :
#define OP_MtR_ItR(_MtR_,_ItR_)	assert(FLAG_32==(op->flags & 3));\
	assert(0==(op->flags & (FLAG_IMM2)));\
	assert(op->flags & FLAG_REG1);\
	if (op->flags & FLAG_IMM1)\
	{\
		assert(0==(op->flags & FLAG_REG2));\
		LoadReg(EAX,op->reg1);\
		x86e-> _ItR_ (EAX,op->imm1);\
		SaveReg(op->reg1,EAX);\
	}\
	else\
	{\
		assert(op->flags & FLAG_REG2);\
		LoadReg(EAX,op->reg1);\
		x86e-> _MtR_ (EAX,GetRegPtr(op->reg2));\
		SaveReg(op->reg1,EAX);\
	}

//original
/*
	assert(FLAG_32==(op->flags & 3));//32b olny	

	assert(op->flags & FLAG_IMM1);//reg1
	assert(0==(op->flags & (FLAG_IMM2)));//no imms2
	assert(op->flags & FLAG_REG1);//reg1
	assert(0==(op->flags & FLAG_REG2));//no reg2

	x86e->SHR32ItoM(GetRegPtr(op->reg1),(u8)op->imm1);
*/
#define OP_ItM(_ItM_,_Imm_)	assert(FLAG_32==(op->flags & 3));\
	assert(op->flags & FLAG_IMM1);\
	assert(0==(op->flags & (FLAG_IMM2)));\
	assert(op->flags & FLAG_REG1);\
	assert(0==(op->flags & FLAG_REG2));\
	x86e->_ItM_(GetRegPtr(op->reg1),_Imm_);

#define OP_NtM_noimm(_ItM_,_Imm_)	assert(FLAG_32==(op->flags & 3));\
	assert(0==(op->flags & FLAG_IMM1));\
	assert(0==(op->flags & (FLAG_IMM2)));\
	assert(op->flags & FLAG_REG1);\
	assert(0==(op->flags & FLAG_REG2));\
	x86e->_ItM_(GetRegPtr(op->reg1));

#define OP_NtR_noimm(_ItR_,_Imm_)	assert(FLAG_32==(op->flags & 3));\
	assert(0==(op->flags & FLAG_IMM1));\
	assert(0==(op->flags & (FLAG_IMM2)));\
	assert(op->flags & FLAG_REG1);\
	assert(0==(op->flags & FLAG_REG2));\
	LoadReg(EAX,op->reg1);\
	x86e->_ItR_(EAX);\
	SaveReg(op->reg1,EAX);

//shil compilation
void __fastcall shil_compile_nimp(shil_opcode* op,rec_v1_BasicBlock* block)
{
	printf("*********SHIL \"%s\" not recompiled*********\n",GetShilName((shil_opcodes)op->opcode));
}

void __fastcall shil_compile_mov(shil_opcode* op,rec_v1_BasicBlock* block)
{
	u32 size=op->flags&3;
	assert(op->flags & FLAG_REG1);//reg1 has to be used on mov :)
	
	if (size==FLAG_32)
	{
		if (op->flags & FLAG_REG2)
		{
			assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));//no imm can be used
			//printf("mov [r]%d=[r]%d , sz==%d\n",op->reg1,op->reg2,size);

			LoadReg(EAX,op->reg2);
			SaveReg(op->reg1,EAX);
		}
		else
		{
			assert(0==(op->flags & (FLAG_IMM2)));//no imm2 can be used
			//printf("mov [r]%d=[imm]%d , sz==%d\n",op->reg1,op->imm1,size);

			x86e->MOV32ItoR(EAX,op->imm1);
			SaveReg(op->reg1,EAX);
		}
	}
	else
	{
		assert(size!=FLAG_64);//32 or 64 b
		assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));//no imm can be used
		printf("mov64 not supported\n");
	}
	
}


//mnnn movex !! wowhoo
void __fastcall shil_compile_movex(shil_opcode* op,rec_v1_BasicBlock* block)
{
	u32 size=op->flags&3;
	assert(op->flags & (FLAG_REG1|FLAG_REG2));	//reg1 , reg2 has to be used on movex :)
	assert((size!=FLAG_8)||(size!=FLAG_16));	//olny 8 or 16 bits can be extended

	if (size==FLAG_8)
	{//8 bit
		if (op->flags & FLAG_SX)
		{//SX 8
			LoadReg(EAX,op->reg2);
			x86e->MOVSX32R8toR(EAX,EAX);
			SaveReg(op->reg1,EAX);
		}
		else
		{//ZX 8
			LoadReg(EAX,op->reg2);
			x86e->MOVZX32R8toR(EAX,EAX);
			SaveReg(op->reg1,EAX);
		}
	}
	else
	{//16 bit
		if (op->flags & FLAG_SX)
		{//SX 16
			LoadReg(EAX,op->reg2);
			x86e->MOVSX32R16toR(EAX,EAX);
			SaveReg(op->reg1,EAX);
		}
		else
		{//ZX 16
			LoadReg(EAX,op->reg2);
			x86e->MOVZX32R16toR(EAX,EAX);
			SaveReg(op->reg1,EAX);
		}
	}
}

//ahh
void __fastcall shil_compile_shil_ifb(shil_opcode* op,rec_v1_BasicBlock* block)
{

	//if opcode needs pc , save it
	if (OpTyp[op->imm1] !=Normal)
		SaveReg(reg_pc,op->imm2);

	x86e->MOV32ItoR(ECX,op->imm1);
	x86e->CALLFunc(OpPtr[op->imm1]);
}

//shift
void __fastcall shil_compile_swap(shil_opcode* op,rec_v1_BasicBlock* block)
{
	u32 size=op->flags&3;
	
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));//no imms
	assert(op->flags & FLAG_REG1);//reg1
	assert(0==(op->flags & FLAG_REG2));//reg2

	if (size==FLAG_8)
	{
		LoadReg(EAX,op->reg1);
		x86e->XCHG8RtoR(Reg_AH,Reg_AL);
		SaveReg(op->reg1,EAX);
	}
	else
	{
		assert(size==FLAG_16);//has to be 16 bit
		//printf("Shil : wswap not implemented\n");
		
		//use rotate ?
		LoadReg(EAX,op->reg1);
		x86e->ROR32ItoR(EAX,16);
		SaveReg(op->reg1,EAX);
	}
}

void __fastcall shil_compile_shl(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_ItM(SHL32ItoM,(u8)op->imm1);
}

void __fastcall shil_compile_shr(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_ItM(SHR32ItoM,(u8)op->imm1);
}

void __fastcall shil_compile_sar(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_ItM(SAR32ItoM,(u8)op->imm1);
}

//rotates
void __fastcall shil_compile_rcl(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtM_noimm(RCL321toM);
}
void __fastcall shil_compile_rcr(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtR_noimm(RCR321toR);
}
void __fastcall shil_compile_ror(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtR_noimm(ROR321toR);
}
void __fastcall shil_compile_rol(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtR_noimm(ROL321toR);
}
//neg
void __fastcall shil_compile_neg(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtR_noimm(NEG32R);
}
//not
void __fastcall shil_compile_not(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtR_noimm(NOT32R);
}
//or xor and
void __fastcall shil_compile_xor(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_MtR_ItR(XOR32MtoR,XOR32ItoR);
}
void __fastcall shil_compile_or(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_MtR_ItR(OR32MtoR,OR32ItoR);
}
void __fastcall shil_compile_and(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_MtR_ItR(AND32MtoR,AND32ItoR);
}
//read-write
void readwrteparams(shil_opcode* op)
{
	assert(0==(op->flags & FLAG_IMM2));
	assert(op->flags & FLAG_REG1);

	
	if (!(op->flags & (FLAG_R0|FLAG_GBR)))
	{//[reg2] form
		assert(op->flags & FLAG_IMM1);

		if (op->flags & FLAG_REG2)
		{	//[reg2+imm1]
			LoadReg(ECX,op->reg2);
			if (op->imm1)//no imm :P
				x86e->ADD32ItoR(ECX,op->imm1);
		}
		else
		{	//[imm1]
			assert(0==(op->flags & FLAG_REG2));
			x86e->MOV32ItoR(ECX,op->imm1);
		}
	}
	else
	{
		//reg0/gbr[reg2] form
		assert(op->flags & (FLAG_R0|FLAG_GBR));

		assert(0==(op->flags & FLAG_IMM1));

		if (op->flags & FLAG_R0)
			LoadReg(ECX,r0);
		else
			LoadReg(ECX,reg_gbr);

		x86e->ADD32MtoR(ECX,GetRegPtr(op->reg2));
	}
}
void __fastcall shil_compile_readm(shil_opcode* op,rec_v1_BasicBlock* block)
{
	u32 size=op->flags&3;

	readwrteparams(op);

	if (size==0)
	{
		x86e->CALLFunc(ReadMem8);
		x86e->MOVSX32R8toR(EAX,EAX);	//se8
	}
	else if (size==1)
	{
		x86e->CALLFunc(ReadMem16);
		x86e->MOVSX32R16toR(EAX,EAX);	//se16
	}
	else if (size==2)
	{
		x86e->CALLFunc(ReadMem32);
	}
	else
		printf("ReadMem error\n");

	SaveReg(op->reg1,EAX);//save return value
}
void __fastcall shil_compile_writem(shil_opcode* op,rec_v1_BasicBlock* block)
{
	u32 size=op->flags&3;

	readwrteparams(op);

	if (size==FLAG_8)
	{	//maby zx ?
		LoadReg(EDX,op->reg1);
		x86e->CALLFunc(WriteMem8);
	}
	else if (size==FLAG_16)
	{	//maby zx ?
		LoadReg(EDX,op->reg1);
		x86e->CALLFunc(WriteMem16);
	}
	else if (size==FLAG_32)
	{
		LoadReg(EDX,op->reg1);
		x86e->CALLFunc(WriteMem32);
	}
	else
		printf("WriteMem error\n");
}

//save-loadT
void __fastcall shil_compile_SaveT(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(op->flags & FLAG_IMM1);//imm1
	assert(0==(op->flags & (FLAG_IMM2|FLAG_REG1|FLAG_REG2)));//no imm2/r1/r2
	x86e->SETcc8R(EAX,op->imm1);//imm1 :P
	x86e->MOVZX32R8toR(EAX,EAX);//clear rest of eax (to remove partial depency on 32:8)
	
	//x86e->AND32ItoR(EAX,1);		//and 1

	LoadReg(ECX,reg_sr);			//ecx=sr(~1)|T
	x86e->AND32ItoR(ECX,(u32)~1);
	x86e->OR32RtoR(ECX,EAX);
	SaveReg(reg_sr,ECX);
}
void __fastcall shil_compile_LoadT(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(op->flags & FLAG_IMM1);//imm1
	assert(0==(op->flags & (FLAG_IMM2|FLAG_REG1|FLAG_REG2)));//no imm2/r1/r2
	
	assert(op->imm1==x86_flags::CF);
	
	//SHR_RegByImm(1,Reg_EAX,1);

	LoadReg(EAX,reg_sr);
	x86e->SHR32ItoR(EAX,1);//heh T bit is there now :P CF
}
//cmp-test

void __fastcall shil_compile_cmp(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(FLAG_32==(op->flags & 3));
	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & (FLAG_REG2|FLAG_IMM2)));
		LoadReg(EAX,op->reg1);
		x86e->CMP32ItoR(EAX,op->imm1);
		//eflags is used w/ combination of SaveT
	}
	else
	{
		assert(0==(op->flags & FLAG_IMM2));
		assert(op->flags & FLAG_REG2);
		LoadReg(EAX,op->reg1);
		LoadReg(ECX,op->reg2);
		x86e->CMP32RtoR(EAX,ECX);//rm,rn
		//eflags is used w/ combination of SaveT
	}
}
void __fastcall shil_compile_test(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(FLAG_32==(op->flags & 3));

	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & (FLAG_REG2|FLAG_IMM2)));
		LoadReg(EAX,op->reg1);
		x86e->TEST32ItoR(EAX,op->imm1);
		//eflags is used w/ combination of SaveT
	}
	else
	{
		assert(0==(op->flags & FLAG_IMM2));
		assert(op->flags & FLAG_REG2);
		LoadReg(EAX,op->reg1);
		LoadReg(ECX,op->reg2);
		x86e->TEST32RtoR(EAX,ECX);
		//eflags is used w/ combination of SaveT
	}
}

//add-sub
void __fastcall shil_compile_add(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_MtR_ItR(ADD32MtoR,ADD32ItoR);
}
void __fastcall shil_compile_adc(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_MtR_ItR(ADC32MtoR,ADC32ItoR);
}
void __fastcall shil_compile_sub(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_MtR_ItR(SUB32MtoR,SUB32ItoR);
}

//**
void __fastcall shil_compile_jcond(shil_opcode* op,rec_v1_BasicBlock* block)
{
	printf("jcond ... heh not implemented\n");
}
void __fastcall shil_compile_jmp(shil_opcode* op,rec_v1_BasicBlock* block)
{
	printf("jmp ... heh not implemented\n");
}

void __fastcall shil_compile_mul(shil_opcode* op,rec_v1_BasicBlock* block)
{
	u32 sz=op->flags&3;

	assert(sz==FLAG_8);//nope , can't be 16 bit..

	if (sz==FLAG_64)//mach is olny used on 64b version
		assert(op->flags & FLAG_MACH);
	else
		assert(0==(op->flags & FLAG_MACH));
	

	if (sz!=FLAG_64)
	{
		if (sz==FLAG_16)
		{
			if (op->flags & FLAG_SX)
			{
				x86e->MOVSX32M16toR(EAX,(u16*)GetRegPtr(op->reg1));
				x86e->MOVSX32M16toR(ECX,(u16*)GetRegPtr(op->reg2));
			}
			else
			{
				x86e->MOVZX32M16toR(EAX,(u16*)GetRegPtr(op->reg1));
				x86e->MOVZX32M16toR(ECX,(u16*)GetRegPtr(op->reg2));
			}
		}
		else
		{
			x86e->MOV32MtoR(EAX,GetRegPtr(op->reg1));
			x86e->MOV32MtoR(ECX,GetRegPtr(op->reg2));
		}

		x86e->IMUL32RtoR(EAX,ECX);
		
		SaveReg((u8)reg_macl,EAX);
	}
	else
	{
		assert(sz==FLAG_64);

		x86e->MOV32MtoR(EAX,GetRegPtr(op->reg1));

		if (op->flags & FLAG_SX)
			x86e->IMUL32M(GetRegPtr(op->reg2));
		else
			x86e->MUL32M(GetRegPtr(op->reg2));

		SaveReg((u8)reg_macl,EAX);
		SaveReg((u8)reg_mach,EDX);
	}
}

shil_compileFP* sclt[shil_count]=
{
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp
};

void SetH(shil_opcodes op,shil_compileFP* ha)
{
	if (op>(shil_count-1))
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
	//
	SetH(shil_opcodes::adc,shil_compile_adc);
	SetH(shil_opcodes::add,shil_compile_add);
	SetH(shil_opcodes::and,shil_compile_and);
	SetH(shil_opcodes::cmp,shil_compile_cmp);
	SetH(shil_opcodes::fabs,shil_compile_nimp);
	SetH(shil_opcodes::fadd,shil_compile_nimp);
	SetH(shil_opcodes::fdiv,shil_compile_nimp);
	SetH(shil_opcodes::fmac,shil_compile_nimp);

	SetH(shil_opcodes::fmul,shil_compile_nimp);
	SetH(shil_opcodes::fneg,shil_compile_nimp);
	SetH(shil_opcodes::fsub,shil_compile_nimp);
	SetH(shil_opcodes::LoadT,shil_compile_LoadT);
	SetH(shil_opcodes::mov,shil_compile_mov);
	SetH(shil_opcodes::movex,shil_compile_movex);
	SetH(shil_opcodes::neg,shil_compile_neg);
	SetH(shil_opcodes::not,shil_compile_not);

	SetH(shil_opcodes::or,shil_compile_or);
	SetH(shil_opcodes::rcl,shil_compile_rcl);
	SetH(shil_opcodes::rcr,shil_compile_rcr);
	SetH(shil_opcodes::readm,shil_compile_readm);
	SetH(shil_opcodes::rol,shil_compile_rol);
	SetH(shil_opcodes::ror,shil_compile_ror);
	SetH(shil_opcodes::sar,shil_compile_sar);
	SetH(shil_opcodes::SaveT,shil_compile_SaveT);

	SetH(shil_opcodes::shil_ifb,shil_compile_shil_ifb);
	SetH(shil_opcodes::shl,shil_compile_shl);
	SetH(shil_opcodes::shr,shil_compile_shr);
	SetH(shil_opcodes::sub,shil_compile_sub);
	SetH(shil_opcodes::swap,shil_compile_swap);
	SetH(shil_opcodes::test,shil_compile_test);
	SetH(shil_opcodes::writem,shil_compile_writem);
	SetH(shil_opcodes::xor,shil_compile_xor);
	SetH(shil_opcodes::jcond,shil_compile_jcond);
	SetH(shil_opcodes::jmp,shil_compile_jmp);
	SetH(shil_opcodes::mul,shil_compile_mul);

	
	u32 shil_nimp=shil_opcodes::shil_count;
	for (int i=0;i<shil_opcodes::shil_count;i++)
	{
		if (sclt[i]==shil_compile_nimp)
			shil_nimp--;
	}

	printf("lazy shil compiler stats : %d%% opcodes done\n",shil_nimp*100/shil_opcodes::shil_count);
}
bool inited=false;
int fallbacks=0;
int native=0;

void CompileBasicBlock_slow(rec_v1_BasicBlock* block)
{
	if (!inited)
	{
		Init();
		inited=true;
	}

	x86e=new emitter<>();
	block->compiled=new rec_v1_CompiledBlock();

	for (u32 i=0;i<block->ilst.opcodes.size();i++)
	{
		shil_opcode* op=&block->ilst.opcodes[i];
		if (op->opcode==shil_ifb)
			fallbacks++;
		else
			native++;

		if (op->opcode>(shil_opcodes::shil_count-1))
		{
			printf("SHIL COMPILER ERROR\n");
		}
		sclt[op->opcode](op,block);
	}

	x86e->RET();

	block->compiled->Code=(rec_v1_BasicBlockEP*)x86e->GetCode();
	block->compiled->count=10;
	block->compiled->parent=block;
	block->compiled->size=10;

	u32 rat=native>fallbacks?fallbacks:native;
	printf("Native/Fallback ratio : %d:%d [%d:%d]\n",native,fallbacks,native/rat,fallbacks/rat);
	delete x86e;
}