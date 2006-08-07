#include "shil_compile_slow.h"
#include <assert.h>
#include "emitter.h"

#include "dc\sh4\shil\shil_ce.h"
#include "dc\sh4\sh4_registers.h"
#include "dc\sh4\rec_v1\blockmanager.h"
#include "dc\sh4\rec_v1\nullprof.h"
#include "dc\sh4\sh4_opcode_list.h"
#include "dc\mem\sh4_mem.h"
#include "regalloc\x86_sseregalloc.h"

FloatRegAllocator*		fra;
IntegerRegAllocator*	ira;
//TEMP!!!
emitter<>* x86e;
shil_scs shil_compile_slow_settings=
{
	true,	//do Register allocation for x86
	4,		//on 4 regisers
	false,	//and on XMM
	true,	//Inline Const Mem reads
	true,	//Inline normal mem reads
	false,	//Inline mem writes
	false,	//Do _not_ keep tbit seperate ;P , needs bug fixing
	true	//Predict returns (needs a bit debuggin)	
};

cDllHandler profiler_dll;

#define REG_ALLOC_COUNT			 (shil_compile_slow_settings.RegAllocCount)
#define REG_ALLOC_T_BIT_SEPERATE (shil_compile_slow_settings.TBitsperate)
#define REG_ALLOC_X86			 (shil_compile_slow_settings.RegAllocX86)
#define REG_ALLOC_XMM			 (shil_compile_slow_settings.RegAllocXMM)

#define INLINE_MEM_READ_CONST   (shil_compile_slow_settings.InlineMemRead_const)
#define INLINE_MEM_READ			(shil_compile_slow_settings.InlineMemRead)
#define INLINE_MEM_WRITE		(shil_compile_slow_settings.InlineMemWrite)

#define RET_PREDICTION			(shil_compile_slow_settings.RetPrediction)
#define BC_LINKING				(true)		//link conditional branches
#define BF_LINKING				(true)		//link fixed branches

bool nullprof_enabled=false;

#define PROFILE_BLOCK_CYCLES (nullprof_enabled)
typedef void __fastcall shil_compileFP(shil_opcode* op,BasicBlock* block);

bool inited=false;

int fallbacks=0;
int native=0;
u32 T_jcond_value;
u32 T_bit_value;
bool T_Edited;

u32 reg_pc_temp_value;

int block_count=0;

//profiling related things
u64 ifb_calls=0;
void profile_ifb_call()
{
	ifb_calls++;
}


//#define PROFILE_SLOW_BLOCK
//#ifdef PROFILE_SLOW_BLOCK
union _Cmp64
{
	struct
	{
		u32 l;
		u32 h;
	};
	u64 v;
};

_Cmp64 dyn_last_block;
_Cmp64 dyn_now_block;
void __fastcall dyna_profile_block_enter()
{
	__asm
	{
		rdtsc;
		mov dyn_last_block.l,eax;
		mov dyn_last_block.h,edx;
	}
}

void __fastcall dyna_profile_block_exit_BasicBlock(NullProfInfo* np)
{
	__asm
	{
		rdtsc;
		mov dyn_now_block.l,eax;
		mov dyn_now_block.h,edx;
	}

	u64 t=dyn_now_block.v-dyn_last_block.v;
	//rec_native_cycles+=t;
	np->time+=t;
	np->calls++;
}
//#endif
//

//find % of time used by dynarec code link
_Cmp64 dyn_ls;
_Cmp64 dyn_le;
u64 rec_native_cycles;

_Cmp64 old_tsmp;
_Cmp64 c_tsmp;
void DynaPrintCycles()
{
	__asm
	{
		rdtsc;
		mov c_tsmp.l,eax;
		mov c_tsmp.h,edx;
	}
	u64 hole=c_tsmp.v-old_tsmp.v;
	double percent=(double)rec_native_cycles/(double)hole;
	printf("Dynarec block execution path : %f%%\n",percent*100);
	rec_native_cycles=0;
	hole=0;
	__asm
	{
		rdtsc;
		mov old_tsmp.l,eax;
		mov old_tsmp.h,edx;
	}
}
void DynaPrintLinkStart()
{
	__asm
	{
		rdtsc;
		mov dyn_ls.l,eax;
		mov dyn_ls.h,edx;
	}
}
void DynaPrintLinkEnd()
{
	__asm
	{
		rdtsc;
		mov dyn_le.l,eax;
		mov dyn_le.h,edx;
	}
	rec_native_cycles+=dyn_le.v-dyn_ls.v;
}
//more helpers
//ensure mode is 32b (for floating point)
void c_Ensure32()
{
	assert(fpscr.PR==0);
}
//emit a call to c_Ensure32 
bool Ensure32()
{
	x86e->CALLFunc(c_Ensure32);
	return true;
}

//Register managment related
//Get a pointer to a reg
u32* GetRegPtr(u32 reg)
{
	if (reg==Sh4RegType::reg_pc_temp)
		return &reg_pc_temp_value;

	u32* rv=Sh4_int_GetRegisterPtr((Sh4RegType)reg);
	assert(rv!=0);
	return rv;
}
u32* GetRegPtr(Sh4RegType reg)
{
	return GetRegPtr((u8)reg);
}
bool IsSSEAllocReg(u32 reg)
{
	return (reg >=fr_0 && reg<=fr_15);
}
//write back float
void sse_WBF(u32 reg)
{
	if (IsSSEAllocReg(reg))
	{
		fra->WriteBackRegister(reg);
	}
}
//reaload float
void sse_RLF(u32 reg)
{
	if (IsSSEAllocReg(reg))
	{
		fra->ReloadRegister(reg);
	}
}
//REGISTER ALLOCATION
#define LoadReg(to,reg) ira->GetRegister(to,reg,RA_DEFAULT)
#define LoadReg_force(to,reg) ira->GetRegister(to,reg,RA_FORCE)
#define LoadReg_nodata(to,reg) ira->GetRegister(to,reg,RA_NODATA)
#define SaveReg(reg,from)	ira->SaveRegister(reg,from)
//#define MarkDirty_(reg) ira->MarkDirty(reg);
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

//intel sugest not to use the ItoM forms for some reason .. speed diference isnt big .. < 1%
//Macro :
#define OP_MoRtR_ItR(_MtR_,_RtR_,_ItR_,_ItM_,_RtM_)	assert(FLAG_32==(op->flags & 3));\
	assert(0==(op->flags & (FLAG_IMM2)));\
	assert(op->flags & FLAG_REG1);\
	if (op->flags & FLAG_IMM1)\
	{\
		assert(0==(op->flags & FLAG_REG2));\
		if (ira->IsRegAllocated(op->reg1))\
		{\
			x86IntRegType r1 = LoadReg(EAX,op->reg1);\
			assert(r1!=EAX);\
			x86e-> _ItR_ (r1,op->imm1);\
			SaveReg(op->reg1,r1);\
		}\
		else\
		{\
			/*x86e-> _ItM_ (GetRegPtr(op->reg1),op->imm1);*/\
			x86e->MOV32ItoR(EAX,op->imm1);\
			x86e->_RtM_(GetRegPtr(op->reg1),EAX);\
		}\
	}\
	else\
	{\
		assert(op->flags & FLAG_REG2);\
		if (ira->IsRegAllocated(op->reg1))\
		{\
			x86IntRegType r1 = LoadReg(EAX,op->reg1);\
			assert(r1!=EAX);\
			if (ira->IsRegAllocated(op->reg2))\
			{\
				x86IntRegType r2 = LoadReg(EAX,op->reg2);\
				assert(r2!=EAX);\
				x86e-> _RtR_ (r1,r2);\
			}\
			else\
			{\
				x86e-> _MtR_ (r1,GetRegPtr(op->reg2));\
			}\
			SaveReg(op->reg1,r1);\
		}\
		else\
		{\
			x86IntRegType r2 = LoadReg(EAX,op->reg2);\
			x86e-> _RtM_(GetRegPtr(op->reg1),r2);\
		}\
	}

#define OP_RegToReg_simple(opcd) OP_MoRtR_ItR(opcd##MtoR,opcd##RtoR,opcd##ItoR,opcd##ItoM,opcd##RtoM);

//original
/*
	assert(FLAG_32==(op->flags & 3));//32b olny	

	assert(op->flags & FLAG_IMM1);//reg1
	assert(0==(op->flags & (FLAG_IMM2)));//no imms2
	assert(op->flags & FLAG_REG1);//reg1 
	assert(0==(op->flags & FLAG_REG2));//no reg2

	x86e->SHR32ItoM(GetRegPtr(op->reg1),(u8)op->imm1);
*/
#define OP_ItMoR(_ItM_,_ItR_,_Imm_)	assert(FLAG_32==(op->flags & 3));\
	assert(op->flags & FLAG_IMM1);\
	assert(0==(op->flags & (FLAG_IMM2)));\
	assert(op->flags & FLAG_REG1);\
	assert(0==(op->flags & FLAG_REG2));\
	if (ira->IsRegAllocated(op->reg1))\
	{\
		x86IntRegType r1=LoadReg(EAX,op->reg1);\
		assert(r1!=EAX);\
		x86e->_ItR_(r1,_Imm_);\
		SaveReg(op->reg1,r1);\
	}\
	else\
		x86e->_ItM_(GetRegPtr(op->reg1),_Imm_);

#define OP_NtMoR_noimm(_ItM_,_ItR_)	assert(FLAG_32==(op->flags & 3));\
	assert(0==(op->flags & FLAG_IMM1));\
	assert(0==(op->flags & (FLAG_IMM2)));\
	assert(op->flags & FLAG_REG1);\
	assert(0==(op->flags & FLAG_REG2));\
	if (ira->IsRegAllocated(op->reg1))\
	{\
		x86IntRegType r1=LoadReg(EAX,op->reg1);\
		assert(r1!=EAX);\
		x86e->_ItR_(r1);\
		SaveReg(op->reg1,r1);\
	}\
	else\
		x86e->_ItM_(GetRegPtr(op->reg1));

#define OP_NtR_noimm(_ItR_)	assert(FLAG_32==(op->flags & 3));\
	assert(0==(op->flags & FLAG_IMM1));\
	assert(0==(op->flags & (FLAG_IMM2)));\
	assert(op->flags & FLAG_REG1);\
	assert(0==(op->flags & FLAG_REG2));\
	x86IntRegType r1= LoadReg(EAX,op->reg1);\
	x86e->_ItR_(r1);\
	SaveReg(op->reg1,r1);

//shil compilation
void __fastcall shil_compile_nimp(shil_opcode* op,BasicBlock* block)
{
	printf("*********SHIL \"%s\" not recompiled*********\n",GetShilName((shil_opcodes)op->opcode));
}

void __fastcall shil_compile_mov(shil_opcode* op,BasicBlock* block)
{
	u32 size=op->flags&3;
	assert(op->flags & FLAG_REG1);//reg1 has to be used on mov :)
	
	if (size==FLAG_32)
	{
		if (op->reg1==op->reg2)
			return;
		//sse_WBF(op->reg2);//write back possibly readed reg
		//OP_RegToReg_simple(MOV32);
		//sse_RLF(op->reg1);//reload writen reg
		#define mov_flag_GRP_1 1
		#define mov_flag_GRP_2 2
		#define mov_flag_XMM_1 4
		#define mov_flag_XMM_2 8
		#define mov_flag_M32_1 0
		#define mov_flag_M32_2 0
		#define mov_flag_imm_2 16

		u32 flags = 0;

		if (op->flags & FLAG_IMM1)
			flags|=mov_flag_imm_2;

		if (IsSSEAllocReg(op->reg1) && fra->IsRegAllocated(op->reg1))
			flags|=mov_flag_XMM_1;
		
		if (((op->flags & FLAG_IMM1)==0) && IsSSEAllocReg(op->reg2) && fra->IsRegAllocated(op->reg2))
			flags|=mov_flag_XMM_2;

		if (IsSSEAllocReg(op->reg1)==false && ira->IsRegAllocated(op->reg1))
			flags|=mov_flag_GRP_1;

		if (((op->flags & FLAG_IMM1)==0) && IsSSEAllocReg(op->reg2)==false && ira->IsRegAllocated(op->reg2))
			flags|=mov_flag_GRP_2;

		#define XMMtoXMM (mov_flag_XMM_1 | mov_flag_XMM_2)
		#define XMMtoGPR (mov_flag_GRP_1 | mov_flag_XMM_2)
		#define XMMtoM32 (mov_flag_M32_1 | mov_flag_XMM_2)
		
		#define GPRtoXMM (mov_flag_XMM_1 | mov_flag_GRP_2)
		#define GPRtoGPR (mov_flag_GRP_1 | mov_flag_GRP_2)
		#define GPRtoM32 (mov_flag_M32_1 | mov_flag_GRP_2)

		#define M32toXMM (mov_flag_XMM_1 | mov_flag_M32_2)
		#define M32toGPR (mov_flag_GRP_1 | mov_flag_M32_2)
		#define M32toM32 (mov_flag_M32_1 | mov_flag_M32_2)

		#define IMMtoXMM (mov_flag_XMM_1 | mov_flag_imm_2)
		#define IMMtoGPR (mov_flag_GRP_1 | mov_flag_imm_2)
		#define IMMtoM32 (mov_flag_M32_1 | mov_flag_imm_2)
		
		x86SSERegType sse1=XMM_Error;
		x86SSERegType sse2=XMM_Error;
		if (flags & mov_flag_XMM_1)
		{
			sse1=fra->GetRegister(XMM0,op->reg1,RA_NODATA);
			assert(sse1!=XMM0);
		}

		if (flags & mov_flag_XMM_2)
		{
			sse2=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);
			assert(sse2!=XMM0);
		}

		x86IntRegType gpr1=GPR_Error;
		x86IntRegType gpr2=GPR_Error;

		if (flags & mov_flag_GRP_1)
		{
			gpr1=ira->GetRegister(EAX,op->reg1,RA_NODATA);
			assert(gpr1!=EAX);
		}

		if (flags & mov_flag_GRP_2)
		{
			gpr2=ira->GetRegister(EAX,op->reg2,RA_DEFAULT);
			assert(gpr2!=EAX);
		}


		switch(flags)
		{
		case XMMtoXMM:
			{
				x86e->SSE_MOVSS_XMM_to_XMM(sse1,sse2);
				fra->SaveRegister(op->reg1,sse1);
			}
			break;
		case XMMtoGPR:
			{
				//write back to mem location
				x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),sse2);
				//mark that the register has to be reloaded from there
				ira->ReloadRegister(op->reg1);
			}
			break;
		case XMMtoM32:
			{
				//copy to mem location
				x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),sse2);
			}
			break;

		case GPRtoXMM:		
			{
				//write back to ram
				x86e->MOV32RtoM(GetRegPtr(op->reg1),gpr2);
				//mark reload on next use
				fra->ReloadRegister(op->reg1);
			}
			break;
		case GPRtoGPR:
			{
				x86e->MOV32RtoR(gpr1,gpr2);
				ira->SaveRegister(op->reg1,gpr1);
			}
			break;
		case GPRtoM32:
			{
				//copy to ram
				x86e->MOV32RtoM(GetRegPtr(op->reg1),gpr2);
			}
			break;
		case M32toXMM:
			{
				x86e->SSE_MOVSS_M32_to_XMM(sse1,GetRegPtr(op->reg2));
				fra->SaveRegister(op->reg1,sse1);
			}
			break;
		case M32toGPR:
			{
				x86e->MOV32MtoR(gpr1,GetRegPtr(op->reg2));
				ira->SaveRegister(op->reg1,gpr1);
			}
			break;
		case M32toM32:
			{
				x86e->MOV32MtoR(EAX,GetRegPtr(op->reg2));
				x86e->MOV32RtoM(GetRegPtr(op->reg1),EAX);
			}
			break;
		case IMMtoXMM:
			{
				//printf("impossible mov IMMtoXMM [%X]\n",flags);
				//__asm int 3;
				//write back to ram
				x86e->MOV32ItoM(GetRegPtr(op->reg1),op->imm1);
				//mark reload on next use
				fra->ReloadRegister(op->reg1);
			}
			break;

		case IMMtoGPR:
			{
				x86e->MOV32ItoR(gpr1,op->imm1);
				ira->SaveRegister(op->reg1,gpr1);
			}
			break;

		case IMMtoM32:
			{
				x86e->MOV32ItoM(GetRegPtr(op->reg1),op->imm1);
			}
			break;

		default:
			printf("Unkown mov %X\n",flags);
			__asm int 3;
			break;
		}
	}
	else
	{
		assert(size==FLAG_64);//32 or 64 b
		assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));//no imm can be used
		//printf("mov64 not supported\n");
		u8 dest=GetSingleFromDouble(op->reg1);
		u8 source=GetSingleFromDouble(op->reg2);

		//x86e->MOV32MtoR(EAX,GetRegPtr(source));
		//x86e->MOV32MtoR(ECX,GetRegPtr(source+1));
		x86e->SSE_MOVLPS_M64_to_XMM(XMM0,GetRegPtr(source));

		//x86e->MOV32RtoM(GetRegPtr(dest),EAX);
		//x86e->MOV32RtoM(GetRegPtr(dest+1),ECX);
		x86e->SSE_MOVLPS_XMM_to_M64(GetRegPtr(dest),XMM0);
	}
}


//mnnn movex !! wowhoo
void __fastcall shil_compile_movex(shil_opcode* op,BasicBlock* block)
{
	u32 size=op->flags&3;
	assert(op->flags & (FLAG_REG1|FLAG_REG2));	//reg1 , reg2 has to be used on movex :)
	assert((size!=FLAG_8)||(size!=FLAG_16));	//olny 8 or 16 bits can be extended

	if (size==FLAG_8)
	{//8 bit
		if (op->flags & FLAG_SX)
		{//SX 8
			x86IntRegType r2= LoadReg_force(EAX,op->reg2);
			x86IntRegType r1= LoadReg_nodata(ECX,op->reg1);//if same reg (so data is needed) that is done by the above op
			x86e->MOVSX32R8toR(r1,r2);
			SaveReg(op->reg1,r1);
		}
		else
		{//ZX 8
			x86IntRegType r2= LoadReg_force(EAX,op->reg2);
			x86IntRegType r1= LoadReg_nodata(ECX,op->reg1);//if same reg (so data is needed) that is done by the above op
			x86e->MOVZX32R8toR(r1,r2);
			SaveReg(op->reg1,r1);
		}
	}
	else
	{//16 bit
		if (op->flags & FLAG_SX)
		{//SX 16
			x86IntRegType r1;
			if (op->reg1!=op->reg2)
				r1= LoadReg_nodata(ECX,op->reg1);	//get a spare reg , or the allocated one. Data will be overwriten
			else
				r1= LoadReg(ECX,op->reg1);	//get or alocate reg 1 , load data b/c it's gona be used

			if (ira->IsRegAllocated(op->reg2))
			{
				x86IntRegType r2= LoadReg(EAX,op->reg2);
				assert(r2!=EAX);//reg 2 must be allocated
				x86e->MOVSX32R16toR(r1,r2);
			}
			else
			{
				x86e->MOVSX32M16toR(r1,(u16*)GetRegPtr(op->reg2));
			}
			SaveReg(op->reg1,r1);	//ensure it is saved
		}
		else
		{//ZX 16
			x86IntRegType r1;
			if (op->reg1!=op->reg2)
				r1= LoadReg_nodata(ECX,op->reg1);	//get a spare reg , or the allocated one. Data will be overwriten
			else
				r1= LoadReg(ECX,op->reg1);	//get or alocate reg 1 , load data b/c it's gona be used

			if (ira->IsRegAllocated(op->reg2))
			{
				x86IntRegType r2= LoadReg(EAX,op->reg2);
				assert(r2!=EAX);//reg 2 must be allocated
				x86e->MOVZX32R16toR(r1,r2);
			}
			else
			{
				x86e->MOVZX32M16toR(r1,(u16*)GetRegPtr(op->reg2));
			}
			SaveReg(op->reg1,r1);	//ensure it is saved
		}
	}
}

//ahh .. just run interpreter :P
void __fastcall shil_compile_shil_ifb(shil_opcode* op,BasicBlock* block)
{

	//if opcode needs pc , save it
	if (OpTyp[op->imm1] !=Normal)
		SaveReg(reg_pc,op->imm2);
	
	ira->FlushRegCache();
	//FlushRegCache();

	x86e->MOV32ItoR(ECX,op->imm1);
	x86e->CALLFunc(OpPtr[op->imm1]);

	//x86e->CALLFunc(profile_ifb_call);

}

//shift
void __fastcall shil_compile_swap(shil_opcode* op,BasicBlock* block)
{
	u32 size=op->flags&3;
	
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));//no imms
	assert(op->flags & FLAG_REG1);//reg1
	assert(0==(op->flags & FLAG_REG2));//reg2

	if (size==FLAG_8)
	{
		x86IntRegType r1 = LoadReg_force(EAX,op->reg1);
		x86e->XCHG8RtoR(Reg_AH,Reg_AL);//ror16 ?
		SaveReg(op->reg1,r1);
	}
	else
	{
		assert(size==FLAG_16);//has to be 16 bit
		//printf("Shil : wswap not implemented\n");
		
		//use rotate ?
		x86IntRegType r1 = LoadReg(EAX,op->reg1);
		x86e->ROR32ItoR(r1,16);
		SaveReg(op->reg1,r1);
	}
	//MarkDirty(op->reg1);
}

void __fastcall shil_compile_shl(shil_opcode* op,BasicBlock* block)
{
	OP_ItMoR(SHL32ItoM,SHL32ItoR,(u8)op->imm1);
	//MarkDirty(op->reg1);
}

void __fastcall shil_compile_shr(shil_opcode* op,BasicBlock* block)
{
	OP_ItMoR(SHR32ItoM,SHR32ItoR,(u8)op->imm1);
	//MarkDirty(op->reg1);
}

void __fastcall shil_compile_sar(shil_opcode* op,BasicBlock* block)
{
	OP_ItMoR(SAR32ItoM,SAR32ItoR,(u8)op->imm1);
	//MarkDirty(op->reg1);
}

//rotates
void __fastcall shil_compile_rcl(shil_opcode* op,BasicBlock* block)
{
	OP_NtMoR_noimm(RCL321toM,RCL321toR);
	//MarkDirty(op->reg1);
}
void __fastcall shil_compile_rcr(shil_opcode* op,BasicBlock* block)
{
	OP_NtR_noimm(RCR321toR);
	//MarkDirty(op->reg1);
}
void __fastcall shil_compile_ror(shil_opcode* op,BasicBlock* block)
{
	OP_NtR_noimm(ROR321toR);
	//MarkDirty(op->reg1);
}
void __fastcall shil_compile_rol(shil_opcode* op,BasicBlock* block)
{
	OP_NtR_noimm(ROL321toR);
	//MarkDirty(op->reg1);
}
//neg
void __fastcall shil_compile_neg(shil_opcode* op,BasicBlock* block)
{
	OP_NtR_noimm(NEG32R);
	//MarkDirty(op->reg1);
}
//not
void __fastcall shil_compile_not(shil_opcode* op,BasicBlock* block)
{
	OP_NtR_noimm(NOT32R);
	//MarkDirty(op->reg1);
}
//or xor and
void __fastcall shil_compile_xor(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(XOR32);
	//MarkDirty(op->reg1);
}
void __fastcall shil_compile_or(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(OR32);
	//MarkDirty(op->reg1);
}
void __fastcall shil_compile_and(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(AND32);
	//MarkDirty(op->reg1);
}
//read-write
void readwrteparams(shil_opcode* op)
	{
	assert(0==(op->flags & FLAG_IMM2));
	assert(op->flags & FLAG_REG1);

	bool Loaded=false;

	//can use
	//mov ecx,imm
	//mov ecx,r0/gbr
	//mov ecx,r[2]
	//lea ecx[r[2]+imm]
	//lea ecx[r0/gbr+imm]
	//lea ecx[r0/gbr+r[2]*1]
	//lea ecx,[r0/gbr+r[2]*1+imm] ;)
	//
	if (op->flags & FLAG_IMM1)
	{
		Loaded=true;
		x86e->MOV32ItoR(ECX,op->imm1);
	}
	if (op->flags & FLAG_REG2)
	{
		x86IntRegType r1=LoadReg(EAX,op->reg2);
		if (Loaded)
			x86e->ADD32RtoR(ECX,r1);
		else
			x86e->MOV32RtoR(ECX,r1);
		Loaded=true;
	}
	if (op->flags & FLAG_R0)
	{
		x86IntRegType r1=LoadReg(EAX,r0);
		if (Loaded)
			x86e->ADD32RtoR(ECX,r1);
		else
			x86e->MOV32RtoR(ECX,r1);
		Loaded=true;
	}
	if (op->flags & FLAG_GBR)
	{
		x86IntRegType r1=LoadReg(EAX,reg_gbr);
		if (Loaded)
			x86e->ADD32RtoR(ECX,r1);
		else
			x86e->MOV32RtoR(ECX,r1);
		Loaded=true;
	}
	

	/*if (!(op->flags & (FLAG_R0|FLAG_GBR)))
	{//[reg2] form
		assert(op->flags & FLAG_IMM1);
		assert(0==(op->flags & FLAG_IMM2));

		if (op->flags & FLAG_REG2)
		{	//[reg2+imm1]
			LoadReg_force(ECX,op->reg2);
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
		//reg0/gbr[reg2+imm1] form
		assert(op->flags & (FLAG_R0|FLAG_GBR));
		//imm1 can be used now ;)
		assert(0==(op->flags & FLAG_IMM2));

		if (op->flags & FLAG_R0)
			LoadReg_force(ECX,r0);
		else
			LoadReg_force(ECX,reg_gbr);

		if (op->flags & FLAG_REG2)
		{
			if (ira->IsRegAllocated(op->reg2))
			{
				x86IntRegType r2=LoadReg(EAX,op->reg2);
				assert(r2!=EAX);
				x86e->ADD32RtoR(ECX,r2);
			}
			else
				x86e->ADD32MtoR(ECX,GetRegPtr(op->reg2));
		}
		if (op->flags & FLAG_IMM1)
		{
			x86e->ADD32ItoR(ECX,op->imm1);
		}
	}*/
}

u32 const_hit=0;
u32 non_const_hit=0;

void emit_vmem_op_compat(emitter<>* x86e,x86IntRegType ra,
					  x86IntRegType ro,
					  u32 sz,u32 rw);

void emit_vmem_op_compat_const(emitter<>* x86e,u32 ra,
							   x86IntRegType ro,x86SSERegType ro_sse,bool sse,
								u32 sz,u32 rw);
u32 m_unpack_sz[3]={1,2,4};
void __fastcall shil_compile_readm(shil_opcode* op,BasicBlock* block)
{
	u32 size=op->flags&3;

	sse_RLF(op->reg1);//reload possibly readed reg
	if (INLINE_MEM_READ_CONST)
	{
		//if constant read , and on ram area , make it a direct mem access
		//_watch_ mmu
		if (!(op->flags & (FLAG_R0|FLAG_GBR|FLAG_REG2)))
		{//[reg2+imm] form
			assert(op->flags & FLAG_IMM1);
			//[imm1] form
			if (!IsSSEAllocReg(op->reg1))
			{
				x86IntRegType rall=LoadReg_nodata(EDX,op->reg1);
				emit_vmem_op_compat_const(x86e,op->imm1,rall,XMM0,false,m_unpack_sz[size],0);
				SaveReg(op->reg1,rall);
			}
			else
			{
				x86SSERegType rall=fra->GetRegister(XMM0,op->reg1,RA_NODATA);
				emit_vmem_op_compat_const(x86e,op->imm1,EAX,rall,true,m_unpack_sz[size],0);
				fra->SaveRegister(op->reg1,rall);
			}
			//MarkDirty(op->reg1);
			return;
		}
	}

	readwrteparams(op);

	emit_vmem_op_compat(x86e,ECX,EAX,m_unpack_sz[size],0);
	
	//movsx [if needed]
	if (size==0)
	{
		x86e->MOVSX32R8toR(EAX,EAX);	//se8
	}
	else if (size==1)
	{
		x86e->MOVSX32R16toR(EAX,EAX);	//se16
	}
	else if (size==2)
	{
		//nothing needed
	}
	else
		printf("ReadMem error\n");

	if (IsSSEAllocReg(op->reg1))
	{
		fra->SaveRegisterGPR(op->reg1,EAX);
	}
	else
	{
		SaveReg(op->reg1,EAX);//save return value
	}
	//MarkDirty(op->reg1);
}
void __fastcall shil_compile_writem(shil_opcode* op,BasicBlock* block)
{
	sse_WBF(op->reg1);//Write back possibly readed reg
	u32 size=op->flags&3;

	//if constant read , and on ram area , make it a direct mem access
	//_watch_ mmu
	if (!(op->flags & (FLAG_R0|FLAG_GBR|FLAG_REG2)))
	{//[reg2+imm] form
		assert(op->flags & FLAG_IMM1);
		//[imm1] form
		if (!IsSSEAllocReg(op->reg1))
		{
			x86IntRegType rall=LoadReg(EDX,op->reg1);
			emit_vmem_op_compat_const(x86e,op->imm1,rall,XMM0,false,m_unpack_sz[size],1);
		}
		else
		{
			x86SSERegType rall=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			emit_vmem_op_compat_const(x86e,op->imm1,EAX,rall,true,m_unpack_sz[size],1);
		}
		return;
	}

	readwrteparams(op);

	//ECX is address

	//so it's sure loaded (if from reg cache)
	x86IntRegType r1;
	if (IsSSEAllocReg(op->reg1))
	{
		r1=EDX;
		fra->LoadRegisterGPR(EDX,op->reg1);
	}
	else
		r1=LoadReg(EDX,op->reg1);
	
	emit_vmem_op_compat(x86e,ECX,r1,m_unpack_sz[size],1);
}

//save-loadT
void __fastcall shil_compile_SaveT(shil_opcode* op,BasicBlock* block)
{
	assert(op->flags & FLAG_IMM1);//imm1
	assert(0==(op->flags & (FLAG_IMM2|FLAG_REG1|FLAG_REG2)));//no imm2/r1/r2

	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		T_Edited=true;

		//x86e->SETcc8M(&T_bit_value,op->imm1); //imm1 is cond ;)

		x86e->SETcc8R(EAX,op->imm1);//imm1 :P
		x86e->MOV8RtoM((u8*)&T_bit_value,EAX);

		x86e->INT3();
	}
	else
	{
		//x86e->XOR32RtoR(EAX,EAX);
		
		x86e->SETcc8R(EAX,op->imm1);//imm1 :P
		x86e->AND32ItoM(GetRegPtr(reg_sr),(u32)~1);
		x86e->MOVZX32R8toR(EAX,EAX);//clear rest of eax (to remove partial depency on 32:8)
		x86e->OR32RtoM(GetRegPtr(reg_sr),EAX);
		//LoadReg_force(ECX,reg_sr);			//ecx=sr(~1)|T
		//x86e->AND32ItoR(ECX,(u32)~1);
		//x86e->OR32RtoR(ECX,EAX);
		//SaveReg(reg_sr,ECX);
	}
}
void __fastcall shil_compile_LoadT(shil_opcode* op,BasicBlock* block)
{
	assert(op->flags & FLAG_IMM1);//imm1
	assert(0==(op->flags & (FLAG_IMM2|FLAG_REG1|FLAG_REG2)));//no imm2/r1/r2
	

	assert( (op->imm1==x86_flags::CF) || (op->imm1==x86_flags::jcond_flag) );

	if (op->imm1==x86_flags::jcond_flag)
	{
		if ( (!REG_ALLOC_T_BIT_SEPERATE) || (!T_Edited))
		{
			LoadReg_force(EAX,reg_sr);
			x86e->MOV32RtoM(&T_jcond_value,EAX);//T_jcond_value;
		}
		else
		{
			x86e->MOV32MtoR(EAX,&T_bit_value);
			x86e->MOV32RtoM(&T_jcond_value,EAX);//T_jcond_value;
		}
	}
	else
	{
		if ( (!REG_ALLOC_T_BIT_SEPERATE) || (!T_Edited))
		{
			LoadReg_force(EAX,reg_sr);
			x86e->SHR32ItoR(EAX,1);//heh T bit is there now :P CF
		}
		else
		{
			x86e->MOV32MtoR(EAX,&T_bit_value);
			x86e->SHR32ItoR(EAX,1);//heh T bit is there now :P CF
		}
	}
}
//cmp-test

void __fastcall shil_compile_cmp(shil_opcode* op,BasicBlock* block)
{
	assert(FLAG_32==(op->flags & 3));
	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & (FLAG_REG2|FLAG_IMM2)));
		if (ira->IsRegAllocated(op->reg1))
		{
			x86IntRegType r1 = LoadReg(EAX,op->reg1);
			x86e->CMP32ItoR(r1,op->imm1);
		}
		else
		{
			x86e->CMP32ItoM(GetRegPtr(op->reg1),op->imm1);
		}
		//eflags is used w/ combination of SaveT
	}
	else
	{
		assert(0==(op->flags & FLAG_IMM2));
		assert(op->flags & FLAG_REG2);

		x86IntRegType r1 = LoadReg(EAX,op->reg1);
		if (ira->IsRegAllocated(op->reg2))
		{
			x86IntRegType r2 = LoadReg(ECX,op->reg2);
			x86e->CMP32RtoR(r1,r2);//rm,rn
		}
		else
		{
			x86e->CMP32MtoR(r1,GetRegPtr(op->reg2));//rm,rn
		}
		//eflags is used w/ combination of SaveT
	}
}
void __fastcall shil_compile_test(shil_opcode* op,BasicBlock* block)
{
	assert(FLAG_32==(op->flags & 3));
	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & (FLAG_REG2|FLAG_IMM2)));
		if (ira->IsRegAllocated(op->reg1))
		{
			x86IntRegType r1 = LoadReg(EAX,op->reg1);
			x86e->TEST32ItoR(r1,op->imm1);
		}
		else
		{
			x86e->TEST32ItoM(GetRegPtr(op->reg1),op->imm1);
		}
		//eflags is used w/ combination of SaveT
	}
	else
	{
		assert(0==(op->flags & FLAG_IMM2));
		assert(op->flags & FLAG_REG2);

		x86IntRegType r1 = LoadReg(EAX,op->reg1);
		if (ira->IsRegAllocated(op->reg2))
		{
			x86IntRegType r2 = LoadReg(ECX,op->reg2);
			x86e->TEST32RtoR(r1,r2);//rm,rn
		}
		else
		{
			x86e->TEST32MtoR(r1,GetRegPtr(op->reg2));//rm,rn
		}
		//eflags is used w/ combination of SaveT
	}
}

//add-sub
void __fastcall shil_compile_add(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(ADD32);
	//MarkDirty(op->reg1);
}
void __fastcall shil_compile_adc(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(ADC32);
	//MarkDirty(op->reg1);
}
void __fastcall shil_compile_sub(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(SUB32);
	//MarkDirty(op->reg1);
}

//**
void __fastcall shil_compile_jcond(shil_opcode* op,BasicBlock* block)
{
	printf("jcond ... heh not implemented\n");
	assert(false);
/*
	x86e->MOV32MtoR(EAX,&T_jcond_value);
	x86e->TEST32ItoR(EAX,1);//test for T
	emitter<>* x86e_b=x86e;

	u32 pre_cbup=pre_cycles_on_block;
	assert(block->TF_next);
	assert(block->TT_next);

	//if (block->TF_next->compiled==0)
		CompileBasicBlock_slow(block->TF_next,pre_cycles_on_block);
	//else
	//	printf("COMPILE HIT!\n");

	//if (block->TT_next->compiled==0)
		CompileBasicBlock_slow(block->TT_next,pre_cycles_on_block);
	//else
	//	printf("COMPILE HIT!\n");

	pre_cycles_on_block=pre_cbup;
	x86e=x86e_b;

	assert(op->flags & FLAG_IMM1);
	assert((op->imm1==1) || (op->imm1==0));

	if (op->imm1==0)
	{
		x86e->JNE32(block->TF_next->compiled->Code);//!=
		x86e->JMP(block->TT_next->compiled->Code);		 //==
	}
	else
	{
		x86e->JNE32(block->TT_next->compiled->Code);//!=
		x86e->JMP(block->TF_next->compiled->Code);		 //==
	}

	if (block->TT_next->flags & BLOCK_TEMP)
	{
		delete block->TT_next->compiled;
		delete block->TT_next;
		//printf("Deleted temp block \n");
	}
	if (block->TF_next->flags & BLOCK_TEMP)
	{
		delete block->TF_next->compiled;
		delete block->TF_next;
		//printf("Deleted temp block \n");
	}*/
}
void __fastcall shil_compile_jmp(shil_opcode* op,BasicBlock* block)
{
	printf("jmp ... heh not implemented\n");
}

void load_with_se16(x86IntRegType to,u8 from)
{
	if (ira->IsRegAllocated(from))
	{
		x86IntRegType r1=LoadReg(EAX,from);
		x86e->MOVSX32R16toR(to,r1);
	}
	else
		x86e->MOVSX32M16toR(to,(u16*)GetRegPtr(from));
}

void load_with_ze16(x86IntRegType to,u8 from)
{
	if (ira->IsRegAllocated(from))
	{
		x86IntRegType r1=LoadReg(EAX,from);
		x86e->MOVZX32R16toR(to,r1);
	}
	else
		x86e->MOVZX32M16toR(to,(u16*)GetRegPtr(from));
}

void __fastcall shil_compile_mul(shil_opcode* op,BasicBlock* block)
{
	u32 sz=op->flags&3;

	assert(sz!=FLAG_8);//nope , can't be 16 bit..

	if (sz==FLAG_64)//mach is olny used on 64b version
		assert(op->flags & FLAG_MACH);
	else
		assert(0==(op->flags & FLAG_MACH));


	if (sz!=FLAG_64)
	{
		x86IntRegType r1,r2;
		if (sz==FLAG_16)
		{
			//FlushRegCache_reg(op->reg1);
			//FlushRegCache_reg(op->reg2);

			if (op->flags & FLAG_SX)
			{
				//x86e->MOVSX32M16toR(EAX,(u16*)GetRegPtr(op->reg1));
				load_with_se16(EAX,op->reg1);
				//x86e->MOVSX32M16toR(ECX,(u16*)GetRegPtr(op->reg2));
				load_with_se16(ECX,op->reg2);
			}
			else
			{
				//x86e->MOVZX32M16toR(EAX,(u16*)GetRegPtr(op->reg1));
				load_with_ze16(EAX,op->reg1);
				//x86e->MOVZX32M16toR(ECX,(u16*)GetRegPtr(op->reg2));
				load_with_ze16(ECX,op->reg2);
			}
		}
		else
		{
			//x86e->MOV32MtoR(EAX,GetRegPtr(op->reg1));
			//x86e->MOV32MtoR(ECX,GetRegPtr(op->reg2));
			r1=LoadReg_force(EAX,op->reg1);
			r2=LoadReg_force(ECX,op->reg2);
		}

		if (op->flags & FLAG_SX)
			x86e->IMUL32RtoR(EAX,ECX);
		else
			x86e->MUL32R(ECX);
		
		SaveReg((u8)reg_macl,EAX);
		//MarkDirty(reg_macl);
	}
	else
	{
		assert(sz==FLAG_64);

//		FlushRegCache_reg(op->reg1);
//		FlushRegCache_reg(op->reg2);
		
		ira->FlushRegister(op->reg1);
		ira->FlushRegister(op->reg2);

		x86e->MOV32MtoR(EAX,GetRegPtr(op->reg1));

		if (op->flags & FLAG_SX)
			x86e->IMUL32M(GetRegPtr(op->reg2));
		else
			x86e->MUL32M(GetRegPtr(op->reg2));

		SaveReg((u8)reg_macl,EAX);
		SaveReg((u8)reg_mach,EDX);
		//MarkDirty(reg_macl);
		//MarkDirty(reg_mach);
	}
}


void __fastcall shil_compile_div32(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM2)));

	u8 rQuotient=op->reg1;
	u8 rDivisor=op->reg2;
	u8 rDividend=(u8)op->imm1;


	x86IntRegType Quotient=LoadReg_force(EAX,rQuotient);



	x86IntRegType Dividend=LoadReg_force(EDX,rDividend);


	if (ira->IsRegAllocated(rDivisor))
	{
		x86IntRegType Divisor=LoadReg(EAX,rDivisor);
		if (op->flags & FLAG_SX)
		{
			x86e->IDIV32R(Divisor);
		}
		else
		{
			x86e->DIV32R(Divisor);
		}
	}
	else
	{
		if (op->flags & FLAG_SX)
		{
			x86e->IDIV32M((s32*)GetRegPtr(rDivisor));
		}
		else
		{
			x86e->DIV32M(GetRegPtr(rDivisor));
		}
	}

	if (op->flags & FLAG_SX)
	{
		x86e->SAR32ItoR(EAX,1);
	}
	else
	{
		x86e->SHR32ItoR(EAX,1);
	}

	//set T
	x86e->SETcc8R(ECX,(u8)CC_B);
	x86e->AND32ItoM(GetRegPtr(reg_sr),~1);
	x86e->MOVZX32R8toR(ECX,ECX);
	x86e->OR32RtoM(GetRegPtr(reg_sr),ECX);


	SaveReg(rQuotient,Quotient);

	//WARNING--JUMP--
	//thanks to teh way i save sr , test has to be re-done here
	x86e->TEST32RtoR(ECX,ECX);
	u8* j1=x86e->JNZ8(0);

	if (ira->IsRegAllocated(rDivisor))
	{	//safe to do here b/c rDivisor was loaded to reg above (if reg cached)
		x86IntRegType t=LoadReg(EAX,rDivisor);
		x86e->SUB32RtoR(EDX,t);
	}
	else
	{
		x86e->SUB32MtoR(EDX,GetRegPtr(rDivisor));
	}

	SaveReg(rDividend,Dividend);

	//WARNING--JUMP--

	u8* j2=x86e->JMP8(0);

	x86e->x86SetJ8(j1);

	SaveReg(rDividend,Dividend);

	//WARNING--JUMP--
	x86e->x86SetJ8(j2);

	//MarkDirty(rDividend);
	//MarkDirty(rQuotient);
	ira->MarkDirty(rDividend);
	ira->MarkDirty(rQuotient);
}



//FPU !!! YESH
u32 IsInFReg(u32 reg)
{
	if (IsSSEAllocReg(reg))
	{
		if (fra->IsRegAllocated(reg))
			return 1;
	}
	return 0;
}
//Fpu alloc helpers
#define fa_r1r2 (1|2)
#define fa_r1m2 (1|0)
#define fa_m1r2 (0|2)
#define fa_m1m2 (0|0)

#define frs(op) (IsInFReg(op->reg1)|(IsInFReg(op->reg2)<<1))
#define pi (3.14159265f)

__declspec(align(32)) u32 ps_not_data[4]={0x80000000,0x80000000,0x80000000,0x80000000};
__declspec(align(32)) u32 ps_and_data[4]={0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF};

__declspec(align(32)) float mm_1[4]={1.0f,1.0f,1.0f,1.0f};
__declspec(align(32)) float fsca_fpul_adj[4]={((2*pi)/65536.0f),((2*pi)/65536.0f),((2*pi)/65536.0f),((2*pi)/65536.0f)};

#define SSE_SS_OP(op,RtR,RtM)\
	switch (frs(op))\
	{\
	case fa_r1r2:\
		{\
			x86SSERegType r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);\
			x86SSERegType r2=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);\
			assert(r1!=XMM0 && r2!=XMM0);\
			x86e->RtR(r1,r2);\
			fra->SaveRegister(op->reg1,r1);\
		}\
		break;\
	case fa_r1m2:\
		{\
			x86SSERegType r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);\
			assert(r1!=XMM0);\
			x86e->RtM(r1,GetRegPtr(op->reg2));\
			fra->SaveRegister(op->reg1,r1);\
		}\
		break;\
	case fa_m1r2:\
		{\
			x86SSERegType r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);\
			x86SSERegType r2=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);\
			assert(r1==XMM0);\
			assert(r2!=XMM0);\
			x86e->RtR(r1,r2);\
			fra->SaveRegister(op->reg1,r1);\
		}\
		break;\
	case fa_m1m2:\
		{\
			x86SSERegType r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);\
			assert(r1==XMM0);\
			x86e->RtM(r1,GetRegPtr(op->reg2));\
			fra->SaveRegister(op->reg1,r1);\
		}\
		break;\
	}

#define SSE_s(op,sseop) SSE_SS_OP(op,sseop##_XMM_to_XMM,sseop##_M32_to_XMM);
//simple opcodes
void __fastcall shil_compile_fadd(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		SSE_s(op,SSE_ADDSS);
	}
	else
	{
		assert(false);
	}
}
void __fastcall shil_compile_fsub(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());
		
		SSE_s(op,SSE_SUBSS);
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fmul(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		SSE_s(op,SSE_MULSS);
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fdiv(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		SSE_s(op,SSE_DIVSS);
	}
	else
	{
		assert(false);
	}
}

//binary opcodes
void __fastcall shil_compile_fneg(shil_opcode* op,BasicBlock* block)
{

	
//IsSSEAllocReg
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64 ((Sh4RegType)op->reg1));
		if (IsInFReg(op->reg1))
		{
			x86SSERegType r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(r1!=XMM0);
			x86e->SSE_XORPS_M128_to_XMM(r1,ps_not_data);
			fra->SaveRegister(op->reg1,r1);
		}
		else
		{
			x86e->XOR32ItoM(GetRegPtr(op->reg1),0x80000000);
		}
	}
	else
	{
		assert(sz==FLAG_64);
		assert(IsReg64((Sh4RegType)op->reg1));
		u32 reg=GetSingleFromDouble(op->reg1);
		x86e->XOR32ItoM(GetRegPtr(reg+1),0x80000000);
	}
}

void __fastcall shil_compile_fabs(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		if (IsInFReg(op->reg1))
		{
			x86SSERegType r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(r1!=XMM0);
			x86e->SSE_ANDPS_M128_to_XMM(r1,ps_and_data);
			fra->SaveRegister(op->reg1,r1);
		}
		else
		{
			x86e->AND32ItoM(GetRegPtr(op->reg1),0x7FFFFFFF);
		}
	}
	else
	{
		assert(sz==FLAG_64);
		assert(IsReg64((Sh4RegType)op->reg1));
		u32 reg=GetSingleFromDouble(op->reg1);
		x86e->AND32ItoM(GetRegPtr(reg+1),0x7FFFFFFF);
	}
}



//complex opcodes
void __fastcall shil_compile_fcmp(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		//sse_WBF(op->reg1);
		//sse_WBF(op->reg2);

		//x86e->SSE_MOVSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));
		x86SSERegType fr1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
		
		//x86e->SSE_UCOMISS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));
		if (fra->IsRegAllocated(op->reg2))
		{
			x86SSERegType fr2=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);
			assert(fr2!=XMM0);
			x86e->SSE_UCOMISS_XMM_to_XMM(fr1,fr2);
		}
		else
		{
			x86e->SSE_UCOMISS_M32_to_XMM(fr1,GetRegPtr(op->reg2));
		}
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fmac(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		//fr[n] += fr[0] * fr[m];
		assert(Ensure32());

		//sse_WBF(fr_0);
		//sse_WBF(op->reg2);
		//sse_WBF(op->reg1);

		//x86e->SSE_MOVSS_M32_to_XMM(XMM0,GetRegPtr(fr_0));		//xmm0=fr[0]
		x86SSERegType fr0=fra->GetRegister(XMM0,fr_0,RA_FORCE);
		assert(fr0==XMM0);
		
		//x86e->SSE_MULSS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));	//xmm0*=fr[m]
		if (fra->IsRegAllocated(op->reg2))
		{
			x86SSERegType frm=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);
			assert(frm!=XMM0);
			x86e->SSE_MULSS_XMM_to_XMM(fr0,frm);
		}
		else
		{
			x86e->SSE_MULSS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));
		}

		//x86e->SSE_ADDSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));	//xmm0+=fr[n] 
		if (fra->IsRegAllocated(op->reg1))
		{
			x86SSERegType frn=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(frn!=XMM0);
			x86e->SSE_ADDSS_XMM_to_XMM(fr0,frn);
		}
		else
		{
			x86e->SSE_ADDSS_M32_to_XMM(fr0,GetRegPtr(op->reg1));
		}
		
		
		//x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);	//fr[n]=xmm0
		fra->SaveRegister(op->reg1,fr0);

		//sse_RLF(op->reg1);
	}
	else
	{
		assert(false);
	}
}


void __fastcall shil_compile_fsqrt(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());

		assert(!IsReg64((Sh4RegType)op->reg1));
		//RSQRT vs SQRTSS -- why rsqrt no workie ? :P -> RSQRT = 1/SQRTSS
		if (fra->IsRegAllocated(op->reg1))
		{
			//sse_WBF(op->reg1);
			x86SSERegType fr1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(fr1!=XMM0);
			x86e->SSE_SQRTSS_XMM_to_XMM(fr1,fr1);
			fra->SaveRegister(op->reg1,fr1);
			//sse_RLF(op->reg1);
		}
		else
		{
			x86e->SSE_SQRTSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));
			x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);
		}
		
	}
	else
	{
		assert(false);
	}
}
void __fastcall shil_compile_fsrra(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());
		assert(!IsReg64((Sh4RegType)op->reg1));
		//maby need to calculate 1/sqrt manualy ? -> yes , it seems rcp is not as accurate as needed :)
		//-> no , it wasn that , rcp=1/x , RSQRT=1/srqt tho

		//x86e->SSE_SQRTSS_M32_to_XMM(XMM1,GetRegPtr(op->reg1));	//XMM1=sqrt
		//x86e->SSE_MOVSS_M32_to_XMM(XMM0,(u32*)mm_1);			//XMM0=1
		//x86e->SSE_DIVSS_XMM_to_XMM(XMM0,XMM1);					//XMM0=1/sqrt
		//or
		//x86e->SSE_RSQRTSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));//XMM0=APPR(1/sqrt(fr1))
		//-> im using Approximate version , since this is an aproximate opcode on sh4 too
		//i hope x86 isnt less accurate ..

		if (fra->IsRegAllocated(op->reg1))
		{
			//sse_WBF(op->reg1);
			x86SSERegType fr1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			verify(fr1!=XMM0);
#ifdef _FAST_fssra
			x86e->SSE_RSQRTSS_XMM_to_XMM(fr1,fr1);
#else
			//fra->FlushRegister_xmm(XMM7);
			x86e->SSE_SQRTSS_XMM_to_XMM(XMM0,fr1);				//XMM0=sqrt(fr1)
			x86e->SSE_MOVSS_M32_to_XMM(fr1,(u32*)mm_1);			//fr1=1
			x86e->SSE_DIVSS_XMM_to_XMM(fr1,XMM0);				//fr1=1/XMM0
#endif
			fra->SaveRegister(op->reg1,fr1);
			//sse_RLF(op->reg1);
		}
		else
		{
			#ifdef _FAST_fssra
			x86e->SSE_RSQRTSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));//XMM0=APPR(1/sqrt(fr1))
			#else
			fra->FlushRegister_xmm(XMM7);
			x86e->SSE_SQRTSS_M32_to_XMM(XMM7,GetRegPtr(op->reg1));	//XMM7=sqrt(fr1)
			x86e->SSE_MOVSS_M32_to_XMM(XMM0,(u32*)mm_1);			//XMM0=1
			x86e->SSE_DIVSS_XMM_to_XMM(XMM0,XMM7);					//XMM0=1/XMM7
			#endif
			x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);	//fr1=XMM0
		}
		
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_floatfpul(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());
		assert(!IsReg64((Sh4RegType)op->reg1));

		//TODO : This is not entietly correct , sh4 rounds too [need to set MXCSR]
		//GOTA UNFUCK THE x86 EMITTER
		x86SSERegType r1=fra->GetRegister(XMM0,op->reg1,RA_NODATA);
		x86e->SSE_CVTSI2SS_M32_To_XMM(r1,GetRegPtr(reg_fpul));
		//x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);
		//sse_RLF(op->reg1);
		fra->SaveRegister(op->reg1,r1);
		
	}
	else
	{
		assert(false);
	}
}
void __fastcall shil_compile_ftrc(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());
		assert(!IsReg64((Sh4RegType)op->reg1));

		//TODO : This is not entietly correct , sh4 saturates too
		//GOTA UNFUCK THE x86 EMITTER
		//EAX=(int)fr[n]
		//sse_WBF(op->reg1);
		if (fra->IsRegAllocated(op->reg1))
		{
			x86SSERegType r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(r1!=XMM0);
			//x86e->INT3();
			x86e->SSE_CVTTSS2SI_XMM_To_R32(EAX,r1);
		}
		else
		{
			x86e->SSE_CVTTSS2SI_M32_To_R32(EAX,GetRegPtr(op->reg1));
		}
		//fpul=EAX
		SaveReg(reg_fpul,EAX);
	}
	else
	{
		assert(false);
	}
}

//Mixed opcodes (sse & x87)
void __fastcall shil_compile_fsca(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());
		//bah()
		assert(!IsReg64((Sh4RegType)op->reg1));
		//float real_pi=(((float)(s32)fpul)/65536)*(2*pi);
		//real_pi=(s32)fpul * ((2*pi)/65536.0f);
		x86e->FILD32(GetRegPtr(reg_fpul));		//st(0)=(s32)fpul
		x86e->FMUL32((u32*)fsca_fpul_adj);			//st(0)=(s32)fpul * ((2*pi)/65536.0f)
		x86e->FSINCOS();						//st(0)=sin , st(1)=cos
		
		x86e->FSTP32(GetRegPtr(op->reg1 +1));	//Store cos to reg+1
		x86e->FSTP32(GetRegPtr(op->reg1));		//store sin to reg

		fra->ReloadRegister(op->reg1+1);
		fra->ReloadRegister(op->reg1);
	}
	else
	{
		assert(false);
	}
}

//Vector opcodes ;)
void __fastcall shil_compile_ftrv(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		x86e->SSE_MOVAPS_M128_to_XMM(XMM0,GetRegPtr(op->reg1));
		x86e->SSE_MOVAPS_XMM_to_XMM(XMM3,XMM0);
		x86e->SSE_SHUFPS_XMM_to_XMM(XMM0,XMM0,0);
		x86e->SSE_MOVAPS_XMM_to_XMM(XMM1,XMM3);
		x86e->SSE_MOVAPS_XMM_to_XMM(XMM2,XMM3);
		x86e->SSE_SHUFPS_XMM_to_XMM(XMM1,XMM1,0x55);
		x86e->SSE_SHUFPS_XMM_to_XMM(XMM2,XMM2,0xaa);
		x86e->SSE_SHUFPS_XMM_to_XMM(XMM3,XMM3,0xff);

		x86e->SSE_MULPS_M128_to_XMM(XMM0,GetRegPtr(xf_0));
		x86e->SSE_MULPS_M128_to_XMM(XMM1,GetRegPtr(xf_4));
		x86e->SSE_MULPS_M128_to_XMM(XMM2,GetRegPtr(xf_8));
		x86e->SSE_MULPS_M128_to_XMM(XMM3,GetRegPtr(xf_12));

		x86e->SSE_ADDPS_XMM_to_XMM(XMM0,XMM1);
		x86e->SSE_ADDPS_XMM_to_XMM(XMM2,XMM3);
		x86e->SSE_ADDPS_XMM_to_XMM(XMM0,XMM2);

		x86e->SSE_MOVAPS_XMM_to_M128(GetRegPtr(op->reg1),XMM0);
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fipr(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());

		x86e->SSE_MOVAPS_M128_to_XMM(XMM0,GetRegPtr(op->reg1));
		x86e->SSE_MULPS_M128_to_XMM(XMM0,GetRegPtr(op->reg2));
		x86e->SSE_MOVHLPS_XMM_to_XMM(XMM1,XMM0);
		x86e->SSE_ADDPS_XMM_to_XMM(XMM0,XMM1);
		x86e->SSE_MOVAPS_XMM_to_XMM(XMM1,XMM0);
		x86e->SSE_SHUFPS_XMM_to_XMM(XMM1,XMM1,1);
		x86e->SSE_ADDSS_XMM_to_XMM(XMM0,XMM1);
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1+3),XMM0);
	}
	else
	{
		assert(false);
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
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,
	shil_compile_nimp,shil_compile_nimp,shil_compile_nimp,shil_compile_nimp
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
	SetH(shil_opcodes::fabs,shil_compile_fabs);
	SetH(shil_opcodes::fadd,shil_compile_fadd);
	SetH(shil_opcodes::fdiv,shil_compile_fdiv);
	SetH(shil_opcodes::fmac,shil_compile_fmac);

	SetH(shil_opcodes::fmul,shil_compile_fmul);
	SetH(shil_opcodes::fneg,shil_compile_fneg);
	SetH(shil_opcodes::fsub,shil_compile_fsub);
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

	SetH(shil_opcodes::ftrv,shil_compile_ftrv);
	SetH(shil_opcodes::fsqrt,shil_compile_fsqrt);
	SetH(shil_opcodes::fipr,shil_compile_fipr);
	SetH(shil_opcodes::floatfpul,shil_compile_floatfpul);
	SetH(shil_opcodes::ftrc,shil_compile_ftrc);
	SetH(shil_opcodes::fsca,shil_compile_fsca);
	SetH(shil_opcodes::fsrra,shil_compile_fsrra);
	SetH(shil_opcodes::div32,shil_compile_div32);
	SetH(shil_opcodes::fcmp,shil_compile_fcmp);
	

	
	u32 shil_nimp=shil_opcodes::shil_count;
	for (int i=0;i<shil_opcodes::shil_count;i++)
	{
		if (sclt[i]==shil_compile_nimp)
			shil_nimp--;
	}

	printf("lazy shil compiler stats : %d%% opcodes done\n",shil_nimp*100/shil_opcodes::shil_count);
	if(profiler_dll.Load("nullprof_server.dll"))
	{
		void* temp=profiler_dll.GetProcAddress("InitProfiller");
		if (temp)
		{
			nullprof_enabled=true;
			printf("nullprof_server.dll found , enabling profiling\n"); 
			((InitProfillerFP*)temp)(&null_prof_pointers);
		}
	}

}
//Compile block and return pointer to it's code
void* __fastcall bb_link_compile_inject_TF(CompiledBlockInfo* ptr)
{
	CompiledBlockInfo* target= FindOrRecompileCode(ptr->GetBB()->TF_next_addr);
	
	
	//if current block is Discared , we must not add any chain info , just jump to the new one :)
	if (ptr->Discarded==false)
	{
		//Add reference so we can undo the chain later
		target->AddRef(ptr);
		ptr->GetBB()->TF_block=target;
		ptr->GetBB()->pTF_next_addr=target->Code;
	}
	return target->Code;
}

void* __fastcall bb_link_compile_inject_TT(CompiledBlockInfo* ptr)
{
	CompiledBlockInfo* target= FindOrRecompileCode(ptr->GetBB()->TT_next_addr);

	//if current block is Discared , we must not add any chain info , just jump to the new one :)
	if (ptr->Discarded==false)
	{
		//Add reference so we can undo the chain later
		target->AddRef(ptr);
		ptr->GetBB()->TT_block=target;
		ptr->GetBB()->pTT_next_addr=target->Code;
	}
	return target->Code;
} 


//call link_compile_inject_TF , and jump to code
void naked bb_link_compile_inject_TF_stub(CompiledBlockInfo* ptr)
{
	__asm
	{
		call bb_link_compile_inject_TF;
		jmp eax;
	}
}


void naked bb_link_compile_inject_TT_stub(CompiledBlockInfo* ptr)
{
	__asm
	{
		call bb_link_compile_inject_TT;
		jmp eax;
	}
}


extern u32 rec_cycles;

u32 call_ret_address=0xFFFFFFFF;//holds teh return address of the previus call ;)
CompiledBlockInfo* pcall_ret_address=0;//holds teh return address of the previus call ;)

void CBBs_BlockSuspended(CompiledBlockInfo* block)
{
	if (pcall_ret_address == block)
	{
		call_ret_address=0xFFFFFFFF;
		pcall_ret_address=0;
	}
}
void __fastcall CheckBlock(CompiledBlockInfo* block)
{
	if (block->Discarded)
	{
		printf("Called a discarded block\n");
		__asm int 3;
	}
}
void CompileBasicBlock_slow(BasicBlock* block)
{
	//CompileBasicBlock_slow_c(block);
	if (!inited)
	{
		Init();
		inited=true;
	}

	//perform constan elimination on this block :)
	shil_optimise_pass_ce_driver(block);


	x86e=new emitter<>();

	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		T_Edited=false;
		T_bit_value=0;//just to be sure
	}

	CompiledBasicBlock* cBB;

	bool do_hs=(block->flags.ProtectionType!=BLOCK_PROTECTIONTYPE_MANUAL) && (block->flags.DisableHS==0) &&
		(block->flags.ExitType!=BLOCK_EXITTYPE_DYNAMIC) && (block->flags.ExitType!=BLOCK_EXITTYPE_DYNAMIC_CALL);

	u32 b_type=0;
	
	if (PROFILE_BLOCK_CYCLES)
		b_type|=COMPILED_BLOCK_NULLPROF;
	
	if (do_hs)
		b_type|=COMPILED_BLOCK_HOTSPOT;
	
	b_type|=COMPILED_BASIC_BLOCK;

	cBB=(CompiledBasicBlock*)CreateBlock(b_type);

	block->SetCompiledBlockInfo(cBB);
	

	if (block->flags.ProtectionType==BLOCK_PROTECTIONTYPE_MANUAL)
	{
		int sz=block->end-block->start;
		//check at least 4 bytes
		if (sz<4)
			sz=1;
		else
			sz/=4;
		//sz++;
		int i=0;
		//that can be optimised a lota :p
		for (i=0;i<sz;i++)
		{
			u32* pmem=(u32*)GetMemPtr(block->start+i*4,4);
			x86e->CMP32ItoM((u32*)GetMemPtr(block->start+i*4,4),*pmem);
			u8* patch=x86e->JE8(0);
			x86e->MOV32ItoR(ECX,(u32)cBB);
			x86e->MOV32ItoM(GetRegPtr(reg_pc),block->start);
			x86e->JMP(SuspendBlock);
			x86e->x86SetJ8(patch);
		}
	}
	
	if (do_hs)
	{
		//check for block promotion to superblock ;)
		x86e->DEC32M(&cBB->cbi.GetHS()->bpm_ticks);
		//u8* not_zero2=0;
		u8* not_zero=x86e->JNZ8(0);
		{
			//yay , 0 , see if it needs promotion kkthxdie
			x86e->MOV32MtoR(EAX,&gcp_timer);//now
			x86e->SUB32MtoR(EAX,&cBB->cbi.GetHS()->gcp_lasttimer);//now-last
			x86e->CMP32ItoR(EAX,16);
			//if it took more that 16 ticks , then its less that 10% , no promotion
			u8*no_promote= x86e->JBE8(0);
			{
				//suspend block
				x86e->MOV32ItoR(ECX,(u32)cBB);
				x86e->CALLFunc(SuspendBlock);
				void*  __fastcall CompileCode_SuperBlock(u32 pc);
				x86e->MOV32ItoR(ECX,(u32)cBB->cbi.start);
				x86e->CALLFunc(CompileCode_SuperBlock);
				x86e->JMP32R(EAX);
				//not_zero2=x86e->JMP8(0);
			}
			x86e->x86SetJ8(no_promote);
			x86e->ADD32RtoM(&cBB->cbi.GetHS()->gcp_lasttimer,EAX);//last+now-last=now ;)
			x86e->MOV32ItoM(&cBB->cbi.GetHS()->bpm_ticks,3022);
		}
		x86e->x86SetJ8(not_zero);
		//x86e->x86SetJ8(not_zero2);

		//16 ticks or more to convert to zuper block
		//16 ticks -> 241760hrz /8 ~=30220 blocks
		//we promote to superblock if more that 20% of the time is spent on this block , 3022 ticks
		cBB->cbi.GetHS()->gcp_lasttimer=gcp_timer;
		cBB->cbi.GetHS()->bpm_ticks=3022*2;
	}

	

	//x86e->MOV32ItoR(ECX,(u32)block);
	//x86e->CALLFunc(CheckBlock);
	
	s8* start_ptr;

	if (PROFILE_BLOCK_CYCLES){
		start_ptr=x86e->x86Ptr;
		x86e->CALLFunc(dyna_profile_block_enter);
	}

	fra=GetFloatAllocator();
	ira=GetGPRtAllocator();
	
	ira->DoAllocation(block,x86e);
	fra->DoAllocation(block,x86e);
	//AllocateRegisters(block);
	
	
	//LoadRegisters();
	ira->BeforeEmit();
	fra->BeforeEmit();

	if (PROFILE_BLOCK_CYCLES==false){
		start_ptr=x86e->x86Ptr;
	}

	x86e->ADD32ItoM(&rec_cycles,block->cycles);
	//x86e->MOV32ItoM((u32*)&pCurrentBlock,(u32)block);

	u32 list_sz=(u32)block->ilst.opcodes.size();
	for (u32 i=0;i<list_sz;i++)
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

	
	//FlushRegCache();//flush reg cache
	ira->BeforeTrail();
	fra->BeforeTrail();

	if (PROFILE_BLOCK_CYCLES)
	{
		x86e->MOV32ItoR(ECX,(u32)(cBB->cbi.GetNP()));
		x86e->CALLFunc(dyna_profile_block_exit_BasicBlock);
	}

	//end block acording to block type :)
	switch(block->flags.ExitType)
	{
	
	case BLOCK_EXITTYPE_DYNAMIC_CALL:
		if (RET_PREDICTION)
		{
			//mov guess,pr
			x86e->MOV32ItoM(&call_ret_address,cBB->ebi.TT_next_addr);
			//mov pguess,this
			x86e->MOV32ItoM((u32*)&pcall_ret_address,(u32)(cBB));
		}
	case BLOCK_EXITTYPE_DYNAMIC:
		{
//			x86e->MOV32ItoM((u32*)&pExitBlock,(u32)cBB);
			x86e->RET();
			break;
		}

	case BLOCK_EXITTYPE_RET:
		{
			if (RET_PREDICTION)
			{
				//cmp pr,guess
				x86e->MOV32MtoR(EAX,GetRegPtr(reg_pc));
				x86e->CMP32MtoR(EAX,&call_ret_address);
				//je ok
				u8* ok=x86e->JE8(0);
				//save exit block 
//				x86e->MOV32ItoM((u32*)&pExitBlock,(u32)block);
				//ret
				x86e->RET();
				//ok:
				x86e->x86SetJ8(ok);
				//mov ecx , pcall_ret_address
				x86e->MOV32MtoR(ECX,(u32*)&pcall_ret_address);
				//mov eax,[pcall_ret_address+codeoffset]
				x86e->MOV32RtoR(EAX,ECX);
				x86e->ADD32ItoR(EAX,offsetof(CompiledBasicBlock,ebi.pTT_next_addr));
				x86e->MOV32RmtoR(EAX,EAX);//get ptr to compiled block/link stub
				//jmp eax
				x86e->JMP32R(EAX);	//jump to it
				
			}
			else
			{
				//save exit block 
//				x86e->MOV32ItoM((u32*)&pExitBlock,(u32)cBB);
				x86e->RET();
			}
			break;
		}

	case BLOCK_EXITTYPE_COND_0:
	case BLOCK_EXITTYPE_COND_1:
		{
			//ok , handle COND_0/COND_1 here :)
			//mem address
			u32* TT_a=&cBB->ebi.TT_next_addr;
			u32* TF_a=&cBB->ebi.TF_next_addr;
			//functions
			u32* pTF_f=(u32*)&(cBB->ebi.pTF_next_addr);
			u32* pTT_f=(u32*)&(cBB->ebi.pTT_next_addr);
			
			if (block->flags.ExitType==BLOCK_EXITTYPE_COND_0)
			{
				TT_a=&cBB->ebi.TF_next_addr;
				TF_a=&cBB->ebi.TT_next_addr;
				pTF_f=(u32*)&(cBB->ebi.pTT_next_addr);
				pTT_f=(u32*)&(cBB->ebi.pTF_next_addr);
			}


			x86e->CMP32ItoM(&rec_cycles,BLOCKLIST_MAX_CYCLES);
			
			u8* Link;

			if (BC_LINKING)
			{
				Link=x86e->JB8(0);
			}

			{
				//If our cycle count is expired
				//save the dest address to pc

				x86e->MOV32MtoR(EAX,&T_jcond_value);
				x86e->TEST32ItoR(EAX,1);//test for T
				//see witch pc to set

				x86e->MOV32ItoR(EAX,*TF_a);//==
				//!=
				x86e->CMOVNE32MtoR(EAX,TT_a);//!=
				x86e->MOV32RtoM(GetRegPtr(reg_pc),EAX);

				x86e->RET();//return to caller to check for interrupts
			}

			if (BC_LINKING)
			{
				//Link:
				//if we can execute more blocks
				x86e->x86SetJ8(Link);
				{
					//for dynamic link!
					x86e->MOV32ItoR(ECX,(u32)cBB);					//mov ecx , block
					x86e->MOV32MtoR(EAX,&T_jcond_value);
					x86e->TEST32ItoR(EAX,1);//test for T


					/*
					//link to next block :
					if (*TF_a==cBB->start)
					{
						//fast link (direct jmp to block start)
						if (x86e->CanJ8(start_ptr))
							x86e->JE8(start_ptr);
						else
							x86e->JE32(start_ptr);
					}
					else
					{*/
						x86e->MOV32MtoR(EAX,pTF_f);		//assume it's this condition , unless CMOV overwrites
					/*}
					//!=

					if (*TT_a==cBB->start)
					{
						//fast link (direct jmp to block start)
						if (x86e->CanJ8(start_ptr))
							x86e->JNE8(start_ptr);
						else
							x86e->JNE32(start_ptr);
					}
					else
					{
						if (*TF_a==cBB->start)
						{
							x86e->MOV32MtoR(EAX,pTT_f);// ;)
						}
						else*/
							x86e->CMOVNE32MtoR(EAX,pTT_f);	//overwrite the "other" pointer if needed
					//}
					x86e->JMP32R(EAX);		 //!=
				}
			}
		} 
		break;

	case BLOCK_EXITTYPE_FIXED_CALL:
		//mov guess,pr
		if (RET_PREDICTION)
		{
			x86e->MOV32ItoM(&call_ret_address,cBB->ebi.TT_next_addr);
			//mov pguess,this
			x86e->MOV32ItoM((u32*)&pcall_ret_address,(u32)(cBB));
		}
	case BLOCK_EXITTYPE_FIXED:
		{
			x86e->CMP32ItoM(&rec_cycles,BLOCKLIST_MAX_CYCLES);

			u8* Link;

			if (BF_LINKING)
			{
				Link=x86e->JB8(0);
			}

			//If our cycle count is expired
			x86e->MOV32ItoM(GetRegPtr(reg_pc),cBB->ebi.TF_next_addr);
			x86e->RET();//return to caller to check for interrupts


			if (BF_LINKING)
			{
				//Link:
				//if we can execute more blocks
				x86e->x86SetJ8(Link);
				if (cBB->ebi.TF_next_addr==cBB->cbi.start)
				{
					//__asm int 03;
					printf("Fast Link possible\n");
				}

				//link to next block :
				x86e->MOV32ItoR(ECX,(u32)cBB);					//mov ecx , cBB
				x86e->MOV32MtoR(EAX,(u32*)&(cBB->ebi.pTF_next_addr));	//mov eax , [pTF_next_addr]
				x86e->JMP32R(EAX);									//jmp eax
			}
			break;
		}
	}

	
	ira->AfterTrail();
	fra->AfterTrail();

	x86e->GenCode();//heh


	//block->compiled->Code=(BasicBlockEP*)x86e->GetCode();
	//block->compiled->count=x86e->UsedBytes()/5;
	//block->compiled->parent=block;
	//block->compiled->size=x86e->UsedBytes();
	cBB->cbi.Code=(BasicBlockEP*)x86e->GetCode();
	cBB->cbi.size=x86e->UsedBytes();

	//make it call the stubs , unless otherwise needed
	cBB->ebi.pTF_next_addr=bb_link_compile_inject_TF_stub;
	cBB->ebi.pTT_next_addr=bb_link_compile_inject_TT_stub;

	block_count++;
	
	if ((block_count%512)==128)
	{
		printf("Recompiled %d blocks\n",block_count);
		u32 rat=native>fallbacks?fallbacks:native;
		if (rat!=0)
			printf("Native/Fallback ratio : %d:%d [%d:%d]\n",native,fallbacks,native/rat,fallbacks/rat);
		else
			printf("Native/Fallback ratio : %d:%d [%d:%d]\n",native,fallbacks,native,fallbacks);
		printf("Average block size : %d opcodes ; ",(fallbacks+native)/block_count);
		printf("%d const hits and %d const misses\n",const_hit,non_const_hit);
	}
	
	delete fra;
	delete ira;
	delete x86e;
}

//non x86 , no dynarec


//_vmem for dynarec ;)
//included @ shil_compile_slow.cpp
//Original vmem code
/*
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
	mov eax , [_vmem_WF8+eax];
	jmp eax;
direct:
	and ecx,0xFFFF;//lower 16b of address
	//or eax,edx;	//get ptr to the value we want
	//mov eax,[eax]
	mov [eax+ecx],dl;
	ret;
	}
*/

//on dynarec we have 3 cases for input , mem , reg , cost
//and 3 cases of output , mem , reg , xmm [fpu reg]
//this will also support 64b mem/xmm reads

//handler tables
extern _vmem_ReadMem8FP*		_vmem_RF8[0x1000];
extern _vmem_WriteMem8FP*		_vmem_WF8[0x1000];

extern _vmem_ReadMem16FP*		_vmem_RF16[0x1000];
extern _vmem_WriteMem16FP*		_vmem_WF16[0x1000];

extern _vmem_ReadMem32FP*		_vmem_RF32[0x1000];
extern _vmem_WriteMem32FP*		_vmem_WF32[0x1000];

extern void* _vmem_MemInfo[0x10000];

//sz is 1,2,4,8
//8 reads are assumed to be on same map (8 byte allign should ensure this , i dunoo -> it does)
void emit_vmem_op(emitter<>* x86e,
				  u32 ma,x86IntRegType ra,u32* pa,u32 amode,
				  x86IntRegType ro,x86SSERegType xo,u32 omode,
				  u32 sz,u32 rw)
{
	u8* direct=0;
	u8* op_end=0;
	u32 index=0;
	u32 rb=0;
	if (amode==2)//pointer to ram to read
	{
		x86e->MOV32MtoR(ECX,pa);
		amode=1;
		ra=ECX;
	}

	if (amode==1)
	{
		
		x86e->MOV32RtoR(EAX,ra);
		x86e->SHR32ItoR(EAX,16);
		if (ra!=ECX)
			x86e->MOV32RtoR(ECX,ra);
		if (rw==0)//olny on read
		{
		x86e->MOV32RtoR(EDX,ECX);
		x86e->AND32ItoR(EDX,0xFFFF);
		}
		//_vmem_MemInfo
		//mov eax,[_vmem_MemInfo+eax*4];
		//8B 04 85 base_address
		x86e->write8(0x8b);
		x86e->write8(0x04);
		x86e->write8(0x85);
		x86e->write32((u32)&_vmem_MemInfo[0]);

		//test eax,0xFFFF0000;
		x86e->TEST32ItoR(EAX,0xFFFF0000);
	}
	else if (amode == 0)
	{
		index=ma>>16;
		rb=ma&0xFFFF;
		//x86e->MOV32MtoR(EAX,0/*_vmem_MemInfo*/+index*4);
	}
	direct=x86e->JNZ8(0);
	
	//x86e->MOV32MtoR(EAX,
	//	mov eax , [_vmem_WF8+eax];
	//jmp eax;
	x86e->CALL32R(EAX);
	//mov rdest,EAX
	op_end=x86e->JMP8(0);

	//direct:
	x86e->x86SetJ8(direct);
	if (amode==1)
	{
		if (rw==1)
		{
			//and ecx,0xFFFF;//lower 16b of address
			x86e->AND32ItoR(ECX,0xFFFF);
			if (sz==1)
			{
				//mov [eax+ecx],dl;
			}
			else if (sz==2)
			{
				//mov [eax+ecx],dh;
			}
			else if (sz==4)
			{
				//mov [eax+ecx],edx;
			}
			else if (sz==8)
			{
			}
		}
		else
		{
			//x86e->SSE_MOVSS_M32_to_XMM
			//mov edx,[eax+ecx];
		}
	}
	else
	{
		if (rw==1)
		{
			//mov [eax+rb],dl;
		}
		else
		{
			if (sz<=4)
				;//mov edx,[eax+rb];
			else	//8 bytes read , olny possible to mem/ a pair of xmm registers
				;
		}
	}
	x86e->x86SetJ8(op_end);

}

//this is P4 optimised (its faster olny when shr takes a bit :) )
//on AMD we can move the shr just before the read , and do the edx work after the read
//so we cover the read stall :)
//corrupts ecx,eax,edx; preserves all else (fastcall + xmm safe + xmm fpstatus safe)
void emit_vmem_op_compat(emitter<>* x86e,x86IntRegType ra,
					  x86IntRegType ro,
					  u32 sz,u32 rw)
{
	u32 p_RWF_table=0;
	if (rw==0)
	{
		if (sz==1)
			p_RWF_table=(u32)&_vmem_RF8[0];
		else if (sz==2)
			p_RWF_table=(u32)&_vmem_RF16[0];
		else if (sz==4)
			p_RWF_table=(u32)&_vmem_RF32[0];
	}
	else
	{
		if (sz==1)
			p_RWF_table=(u32)&_vmem_WF8[0];
		else if (sz==2)
			p_RWF_table=(u32)&_vmem_WF16[0];
		else if (sz==4)
			p_RWF_table=(u32)&_vmem_WF32[0];
	}
	////copy address
	//mov eax,ecx;
	x86e->MOV32RtoR(EAX,ra);
	////shr 14 + and vs shr16 + mov eax,[_vmem_MemInfo+eax*4];
	////after testing , shr16+mov complex is faster , both on amd (a64 x2) and intel (northwood)

	////get upper 16 bits
	//shr eax,16;
	x86e->SHR32ItoR(EAX,16);
	
	//read olny
	if (rw==0)
	{
		//mov edx,ecx;//this is done here , among w/ the and , it should be possible to fully execute it on paraler (no depency)
		x86e->MOV32RtoR(EDX,ra);
	}

	//read mem info
	//mov eax,[_vmem_MemInfo+eax*4];
	//8B 04 85 base_address
	x86e->write8(0x8B);
	x86e->write8(0x04);
	x86e->write8(0x85);
	x86e->write32((u32)&_vmem_MemInfo[0]);

	//olny on read
	if (rw==0)
	{
		//read is gona stall , even if 1 cycle ;
		//and edx,0xFFFF;//lower 16b of address
		x86e->AND32ItoR(EDX,0xFFFF);
	}

	//ra may be another reg :)
	if (ra!=ECX)
		x86e->MOV32RtoR(ECX,ra);

	//test eax,0xFFFF0000;
	x86e->TEST32ItoR(EAX,0xFFFF0000);

	if (rw==1)
	{
		if (ro!=EDX)
			x86e->MOV32RtoR(EDX,ro);
	}

	//jnz direct;
	u8* direct=x86e->JNZ8(0);
	
	////get function pointer
	//mov eax , [_vmem_RF8+eax];
	//0040FF23 8B 80 18 28 5A 02 mov         eax,dword ptr _vmem_WF32 (25A2818h)[eax] 
	x86e->write8(0x8B);
	x86e->write8(0x80);
	x86e->write32(p_RWF_table);


	//jmp eax;
	x86e->CALL32R(EAX);
	if (rw==0)
	{
		if (ro!=EAX)
			x86e->MOV32RtoR(ro,EAX);
	}
	u8* rw_end=x86e->JMP8(0);

//direct:
	x86e->x86SetJ8(direct);
	//write olny
	if (rw==1)
	{
		//and edx,0xFFFF;//lower 16b of address
		x86e->AND32ItoR(ECX,0xFFFF);
		//write to [eax+ecx]
		if (sz==1)
		{	//, dl
			x86e->write8(0x88);
			x86e->write8(0x14);
			x86e->write8(0x08);
		}
		else if (sz==2)
		{	//,dx
			x86e->write8(0x66);
			x86e->write8(0x89);
			x86e->write8(0x14);
			x86e->write8(0x08);
		}
		else if (sz==4)
		{	//,edx
			x86e->write8(0x89);
			x86e->write8(0x14);
			x86e->write8(0x08);
		}
	}
	else
	{
		//mov eax,[eax+edx];	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
		//0040FE43 8B 04 10         mov         eax,dword ptr [eax+edx] 
		x86e->write8(0x8B);
		x86e->write8(0x04);
		x86e->write8(0x10);
		if (ro!=EAX)
			x86e->MOV32RtoR(ro,EAX);
	}
	//ret;
	x86e->x86SetJ8(rw_end);
}

void emit_vmem_op_compat_const(emitter<>* x86e,u32 ra,
							   x86IntRegType ro,x86SSERegType ro_sse,bool sse,
								u32 sz,u32 rw)
{
	void* p_RWF_table=0;
	if (rw==0)
	{
		if (sz==1)
			p_RWF_table=&_vmem_RF8[0];
		else if (sz==2)
			p_RWF_table=&_vmem_RF16[0];
		else if (sz==4)
			p_RWF_table=&_vmem_RF32[0];
	}
	else
	{
		if (sz==1)
			p_RWF_table=&_vmem_WF8[0];
		else if (sz==2)
			p_RWF_table=&_vmem_WF16[0];
		else if (sz==4)
			p_RWF_table=&_vmem_WF32[0];
	}

	u32 upper=ra>>16;

	void * t =_vmem_MemInfo[upper];


	u32 lower=ra& 0xFFFF;

	if ((((u32)t) & 0xFFFF0000)==0)
	{

		if (rw==1)
		{
			if (sse)
			{
				__asm int 3;
			}
			else
			{
				if (ro!=EDX)
					x86e->MOV32RtoR(EDX,ro);
			}
		}

		x86e->MOV32ItoR(ECX,ra);

		u32 entry=((u32)t)>>2;

		x86e->CALLFunc(((u32**)p_RWF_table)[entry]);
		if (rw==0)
		{
			if (sz==1)
			{
				x86e->MOVSX32R8toR(ro,EAX);
			}
			else if (sz==2)
			{
				x86e->MOVSX32R16toR(ro,EAX);
			}
			else if (sz==4)
			{
				if (ro!=EAX)
					x86e->MOV32RtoR(ro,EAX);
			}
		}
	}
	else
	{
		if (rw==1)
		{
			void* paddr=&((u8*)t)[lower];
			
			if (sz==1)
			{	//copy to eax :p
				if (ro!=EDX)
					x86e->MOV32RtoR(EDX,ro);
				x86e->MOV8RtoM((u8*)paddr,EDX);
			}
			else if (sz==2)
			{	//,dx
				x86e->MOV16RtoM((u16*)paddr,ro);
			}
			else if (sz==4)
			{	//,edx
				if (sse)
				{
					x86e->SSE_MOVSS_XMM_to_M32((u32*)paddr,ro_sse);
				}
				else
					x86e->MOV32RtoM((u32*)paddr,ro);
			}
		}
		else
		{
			void* paddr=&((u8*)t)[lower];
			if (sz==1)
			{
				x86e->MOVSX32M8toR(ro,(u8*)paddr);
			}
			else if (sz==2)
			{
				x86e->MOVSX32M16toR(ro,(u16*)paddr);
			}
			else if (sz==4)
			{
				if (sse)
				{
					x86e->SSE_MOVSS_M32_to_XMM(ro_sse,(u32*)paddr);
				}
				else
					x86e->MOV32MtoR(ro,(u32*)paddr);
			}
		}
	}
}