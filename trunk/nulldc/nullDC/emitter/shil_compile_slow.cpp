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
x86_block* x86e;
shil_scs shil_compile_slow_settings=
{
	true,	//do Register allocation for x86
	4,		//on 4 regisers
	false,	//and on XMM
	true,	//Inline Const Mem reads
	true,	//Inline normal mem reads
	false	//Inline mem writes
};

x86_opcode_class SetCC[] =
{
	op_seto ,//r/m8 = 0F 90 /0
	op_setno ,//r/m8 = 0F 91 /0
	op_setb ,//r/m8 = 0F 92 /0
	op_setae ,//r/m8 = 0F 93 /0
	op_sete ,//r/m8 = 0F 94 /0
	op_setnz ,//r/m8 = 0F 95 /0
	op_setna ,//r/m8 = 0F 96 /0
	op_seta ,//r/m8 = 0F 97 /0
	op_sets ,//r/m8 = 0F 98 /0
	op_setns ,//r/m8 = 0F 99 /0
	op_setp ,//r/m8 = 0F 9A /0
	op_setnp ,//r/m8 = 0F 9B /0
	op_setl ,//r/m8 = 0F 9C /0
	op_setge ,//r/m8 = 0F 9D /0
	op_setle ,//r/m8 = 0F 9E /0
	op_setg ,//r/m8 = 0F 9F /0
	/*
	op_SETBE r/m8 = 0F 96 /0
	op_SETC r/m8 = 0F 92 /0

	op_SETNAE r/m8 = 0F 92 /0
	op_SETNB r/m8 = 0F 93 /0
	op_SETNBE r/m8 = 0F 97 /0
	op_SETNC r/m8 = 0F 93 /0
	op_SETNE r/m8 = 0F 95 /0
	op_SETNG r/m8 = 0F 9E /0
	op_SETNGE r/m8 = 0F 9C /0
	op_SETNL r/m8 = 0F 9D /0
	op_SETNLE r/m8 = 0F 9F /0
	op_SETPE r/m8 = 0F 9A /0
	op_SETPO r/m8 = 0F 9B /0
	op_SETZ r/m8 = 0F 94 /0
	*/
};
cDllHandler profiler_dll;

#define REG_ALLOC_COUNT			 (shil_compile_slow_settings.RegAllocCount)
#define REG_ALLOC_X86			 (shil_compile_slow_settings.RegAllocX86)
#define REG_ALLOC_XMM			 (shil_compile_slow_settings.RegAllocXMM)

#define INLINE_MEM_READ_CONST   (shil_compile_slow_settings.InlineMemRead_const)
#define INLINE_MEM_READ			(shil_compile_slow_settings.InlineMemRead)
#define INLINE_MEM_WRITE		(shil_compile_slow_settings.InlineMemWrite)

bool nullprof_enabled=false;

#define PROFILE_BLOCK_CYCLES (nullprof_enabled)
typedef void __fastcall shil_compileFP(shil_opcode* op,BasicBlock* block);

bool inited=false;

int fallbacks=0;
int native=0;
u32 T_jcond_value;

u32 reg_pc_temp_value;

int block_count=0;

//temporal register cache :)
//we cache contents of EAX,ECX,EDX , for as long as possible for registers that are not staticaly allocated
//to static reg alloc (ESI,EDI,EBX,EBP are staticaly allocated per block)

struct temporal_reg_cache_entry
{
	x86_gpr_reg reg;
	u32 am;		//0 == no alloc , 1 == spare reg , 2 == temporal cache , 3 == locked temporal cache

	u32 creation_time;	//relative position on x86 stream that the reg was alloated 
						//we replace less used/ old regs first :) (old regs prop got reused , 
						//usualy regs are reused within a few opcodes
	u32 sh4_reg;
};

temporal_reg_cache_entry trc [3]=
{
	{EAX,0,0,0},
	{ECX,0,0,0},
	{EDX,0,0,0}
};
/*
x86_gpr_reg GetTemporalRegCache(u32 sh4_reg)
{
	for (u32 i=0;i<3;i++)
	{
		if ((temporal_reg_cache_entry[i].am ==2) && (temporal_reg_cache_entry[i].sh4_reg==sh4_reg))
		{
			temporal_reg_cache_entry[i].am =3;//lock
			return temporal_reg_cache_entry[i].reg;
		}
	}

	return GPR_Error;
}

x86_gpr_reg GetSpareReg()
{
	for (u32 i=0;i<3;i++)
	{
		if ((temporal_reg_cache_entry[i].am ==0))
		{
			temporal_reg_cache_entry[i].am=1;
			return temporal_reg_cache_entry[i].reg;
		}
	}

	for (u32 i=0;i<3;i++)
	{
		if ((temporal_reg_cache_entry[i].valid ==2))
		{
			return temporal_reg_cache_entry[i].reg;
		}
	}
	printf("dynarec :: x86_gpr_reg GetSpareReg() -> cant find spare reg");
	__asm int 3;
	return GPR_Error;
}

*/
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
	x86e->Emit(op_call,x86_ptr_imm(c_Ensure32));
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
/*
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
*/
//REGISTER ALLOCATION
#define LoadReg(to,reg) ira->GetRegister(to,reg,RA_DEFAULT)
#define LoadReg_force(to,reg) ira->GetRegister(to,reg,RA_FORCE)
#define LoadReg_nodata(to,reg) ira->GetRegister(to,reg,RA_NODATA)
#define SaveReg(reg,from)	ira->SaveRegister(reg,from)

//intel sugest not to use the ItoM forms for some reason .. speed diference isnt big .. < 1%

void fastcall op_reg_to_reg(shil_opcode* op,x86_opcode_class op_cl)
{
	assert(FLAG_32==(op->flags & 3));
	assert(0==(op->flags & (FLAG_IMM2)));
	assert(op->flags & FLAG_REG1);
	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & FLAG_REG2));
		if (ira->IsRegAllocated(op->reg1))
		{
			x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
			assert(r1!=EAX);
			x86e-> Emit(op_cl,r1,op->imm1);
			SaveReg(op->reg1,r1);
		}
		else
		{
			/*x86e-> _ItM_ (GetRegPtr(op->reg1),op->imm1);*/
			x86e->Emit(op_mov32,EAX,op->imm1);
			x86e->Emit(op_cl,GetRegPtr(op->reg1),EAX);
		}
	}
	else
	{
		assert(op->flags & FLAG_REG2);
		if (ira->IsRegAllocated(op->reg1))\
		{
			x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
			assert(r1!=EAX);
			if (ira->IsRegAllocated(op->reg2))
			{
				x86_gpr_reg r2 = LoadReg(EAX,op->reg2);
				assert(r2!=EAX);
				x86e-> Emit(op_cl,r1,r2);
			}
			else
			{
				x86e-> Emit(op_cl,r1,GetRegPtr(op->reg2));
			}
			SaveReg(op->reg1,r1);
		}
		else
		{
			x86_gpr_reg r2 = LoadReg(EAX,op->reg2);
			x86e->Emit(op_cl,GetRegPtr(op->reg1),r2);
		}
	}
}

#define OP_RegToReg_simple(opcd) op_reg_to_reg(op,opcd);

void fastcall op_imm_to_reg(shil_opcode* op,x86_opcode_class op_cl)
{
	assert(FLAG_32==(op->flags & 3));
	assert(op->flags & FLAG_IMM1);
	assert(0==(op->flags & (FLAG_IMM2)));
	assert(op->flags & FLAG_REG1);
	assert(0==(op->flags & FLAG_REG2));
	if (ira->IsRegAllocated(op->reg1))
	{
		x86_gpr_reg r1=LoadReg(EAX,op->reg1);
		assert(r1!=EAX);
		x86e->Emit(op_cl,r1,op->imm1);
		SaveReg(op->reg1,r1);
	}
	else\
		x86e->Emit(op_cl,GetRegPtr(op->reg1),op->imm1);
}

	
void fastcall op_reg(shil_opcode* op,x86_opcode_class op_cl)
{
	assert(FLAG_32==(op->flags & 3));
	assert(0==(op->flags & FLAG_IMM1));
	assert(0==(op->flags & (FLAG_IMM2)));
	assert(op->flags & FLAG_REG1);
	assert(0==(op->flags & FLAG_REG2));
	if (ira->IsRegAllocated(op->reg1))
	{
		x86_gpr_reg r1=LoadReg(EAX,op->reg1);
		assert(r1!=EAX);
		x86e->Emit(op_cl,r1);
		SaveReg(op->reg1,r1);
	}
	else
		x86e->Emit(op_cl,x86_ptr(GetRegPtr(op->reg1)));
}

//shil compilation
void __fastcall shil_compile_nimp(shil_opcode* op,BasicBlock* block)
{
	printf("*********SHIL \"%s\" not recompiled*********\n",GetShilName((shil_opcodes)op->opcode));
}

void __fastcall shil_compile_mov(shil_opcode* op,BasicBlock* block)
{
	u32 size=op->flags&3;
	assert(op->flags & FLAG_REG1);//reg1 has to be used on mov :)
	
	if (op->reg1==op->reg2)
			return;

	if (size==FLAG_32)
	{
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

		if (IsInFReg(op->reg1))
			flags|=mov_flag_XMM_1;
		
		if (((op->flags & FLAG_IMM1)==0) && IsInFReg(op->reg2))
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
		
		x86_sse_reg sse1=ERROR_REG;
		x86_sse_reg sse2=ERROR_REG;
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

		x86_gpr_reg gpr1=ERROR_REG;
		x86_gpr_reg gpr2=ERROR_REG;

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
				x86e->Emit(op_movss,sse1,sse2);
				fra->SaveRegister(op->reg1,sse1);
			}
			break;
		case XMMtoGPR:
			{
				//write back to mem location
				x86e->Emit(op_movss,GetRegPtr(op->reg1),sse2);
				//mark that the register has to be reloaded from there
				ira->ReloadRegister(op->reg1);
			}
			break;
		case XMMtoM32:
			{
				//copy to mem location
				x86e->Emit(op_movss,GetRegPtr(op->reg1),sse2);
			}
			break;

		case GPRtoXMM:		
			{
				//write back to ram
				x86e->Emit(op_mov32,GetRegPtr(op->reg1),gpr2);
				//mark reload on next use
				fra->ReloadRegister(op->reg1);
			}
			break;
		case GPRtoGPR:
			{
				x86e->Emit(op_mov32,gpr1,gpr2);
				ira->SaveRegister(op->reg1,gpr1);
			}
			break;
		case GPRtoM32:
			{
				//copy to ram
				x86e->Emit(op_mov32,GetRegPtr(op->reg1),gpr2);
			}
			break;
		case M32toXMM:
			{
				x86e->Emit(op_movss,sse1,GetRegPtr(op->reg2));
				fra->SaveRegister(op->reg1,sse1);
			}
			break;
		case M32toGPR:
			{
				x86e->Emit(op_mov32,gpr1,GetRegPtr(op->reg2));
				ira->SaveRegister(op->reg1,gpr1);
			}
			break;
		case M32toM32:
			{
				x86e->Emit(op_mov32,EAX,GetRegPtr(op->reg2));
				x86e->Emit(op_mov32,GetRegPtr(op->reg1),EAX);
			}
			break;
		case IMMtoXMM:
			{
				//printf("impossible mov IMMtoXMM [%X]\n",flags);
				//__asm int 3;
				//write back to ram
				x86e->Emit(op_mov32,GetRegPtr(op->reg1),op->imm1);
				//mark reload on next use
				fra->ReloadRegister(op->reg1);
			}
			break;

		case IMMtoGPR:
			{
				x86e->Emit(op_mov32,gpr1,op->imm1);
				ira->SaveRegister(op->reg1,gpr1);
			}
			break;

		case IMMtoM32:
			{
				x86e->Emit(op_mov32,GetRegPtr(op->reg1),op->imm1);
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

		//x86e->Emit(op_mov32,EAX,GetRegPtr(source));
		//x86e->Emit(op_mov32,ECX,GetRegPtr(source+1));
		x86e->Emit(op_movlps,XMM0,GetRegPtr(source));

		//x86e->Emit(op_mov32,GetRegPtr(dest),EAX);
		//x86e->Emit(op_mov32,GetRegPtr(dest+1),ECX);
		x86e->Emit(op_movlps,GetRegPtr(dest),XMM0);
	}
}


//mnnn movex !! woohoo
void __fastcall shil_compile_movex(shil_opcode* op,BasicBlock* block)
{
	u32 size=op->flags&3;
	assert(op->flags & (FLAG_REG1|FLAG_REG2));	//reg1 , reg2 has to be used on movex :)
	assert((size!=FLAG_8)||(size!=FLAG_16));	//olny 8 or 16 bits can be extended

	if (size==FLAG_8)
	{//8 bit
		if (op->flags & FLAG_SX)
		{//SX 8
			x86_gpr_reg r2= LoadReg_force(EAX,op->reg2);
			x86_gpr_reg r1= LoadReg_nodata(ECX,op->reg1);//if same reg (so data is needed) that is done by the above op
			x86e->Emit(op_movsx8to32, r1,r2);
			SaveReg(op->reg1,r1);
		}
		else
		{//ZX 8
			x86_gpr_reg r2= LoadReg_force(EAX,op->reg2);
			x86_gpr_reg r1= LoadReg_nodata(ECX,op->reg1);//if same reg (so data is needed) that is done by the above op
			x86e->Emit(op_movzx8to32, r1,r2);
			SaveReg(op->reg1,r1);
		}
	}
	else
	{//16 bit
		if (op->flags & FLAG_SX)
		{//SX 16
			x86_gpr_reg r1;
			if (op->reg1!=op->reg2)
				r1= LoadReg_nodata(ECX,op->reg1);	//get a spare reg , or the allocated one. Data will be overwriten
			else
				r1= LoadReg(ECX,op->reg1);	//get or alocate reg 1 , load data b/c it's gona be used

			if (ira->IsRegAllocated(op->reg2))
			{
				x86_gpr_reg r2= LoadReg(EAX,op->reg2);
				assert(r2!=EAX);//reg 2 must be allocated
				x86e->Emit(op_movsx16to32, r1,r2);
			}
			else
			{
				x86e->Emit(op_movsx16to32, r1,(u16*)GetRegPtr(op->reg2));
			}
			SaveReg(op->reg1,r1);	//ensure it is saved
		}
		else
		{//ZX 16
			x86_gpr_reg r1;
			if (op->reg1!=op->reg2)
				r1= LoadReg_nodata(ECX,op->reg1);	//get a spare reg , or the allocated one. Data will be overwriten
			else
				r1= LoadReg(ECX,op->reg1);	//get or alocate reg 1 , load data b/c it's gona be used

			if (ira->IsRegAllocated(op->reg2))
			{
				x86_gpr_reg r2= LoadReg(EAX,op->reg2);
				assert(r2!=EAX);//reg 2 must be allocated
				x86e->Emit(op_movzx16to32, r1,r2);
			}
			else
			{
				x86e->Emit(op_movzx16to32, r1,(u16*)GetRegPtr(op->reg2));
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
	fra->FlushRegCache();
	//FlushRegCache();

	x86e->Emit(op_mov32,ECX,op->imm1);
	x86e->Emit(op_call,x86_ptr_imm(OpPtr[op->imm1]));

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
		x86_gpr_reg r1 = LoadReg_force(EAX,op->reg1);
		x86e->Emit(op_xchg8,AH,AL);//ror16 ?
		SaveReg(op->reg1,r1);
	}
	else
	{
		assert(size==FLAG_16);//has to be 16 bit
		//printf("Shil : wswap not implemented\n");
		
		//use rotate ?
		x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
		x86e->Emit(op_ror32,r1,16);
		SaveReg(op->reg1,r1);
	}
}

void __fastcall shil_compile_shl(shil_opcode* op,BasicBlock* block)
{
	op_imm_to_reg(op,op_shl32);
}

void __fastcall shil_compile_shr(shil_opcode* op,BasicBlock* block)
{
	op_imm_to_reg(op,op_shr32);
}

void __fastcall shil_compile_sar(shil_opcode* op,BasicBlock* block)
{
	op_imm_to_reg(op,op_sar32);
}

//rotates
void __fastcall shil_compile_rcl(shil_opcode* op,BasicBlock* block)
{
	op_reg(op,op_rcl32);
}
void __fastcall shil_compile_rcr(shil_opcode* op,BasicBlock* block)
{
	op_reg(op,op_rcr32);
}
void __fastcall shil_compile_ror(shil_opcode* op,BasicBlock* block)
{
	op_reg(op,op_ror32);
}
void __fastcall shil_compile_rol(shil_opcode* op,BasicBlock* block)
{
	op_reg(op,op_rol32);
}
//neg
void __fastcall shil_compile_neg(shil_opcode* op,BasicBlock* block)
{
	op_reg(op,op_neg32);
}
//not
void __fastcall shil_compile_not(shil_opcode* op,BasicBlock* block)
{
	op_reg(op,op_not32);
}
//or xor and
void __fastcall shil_compile_xor(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(op_xor32);
}
void __fastcall shil_compile_or(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(op_or32);
}
void __fastcall shil_compile_and(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(op_and32);
}

void readwrteparams1(u8 reg1,u32 imm)
{
	if (ira->IsRegAllocated(reg1))
	{
		//lea ecx,[reg1+imm]
		x86_reg reg=LoadReg(ECX,reg1);
		assert(reg!=ECX);
		x86e->Emit(op_lea32 ,ECX, x86_mrm::create(reg,x86_ptr::create(imm)));
	}
	else
	{
		//mov ecx,imm
		//add ecx,reg1
		x86e->Emit(op_mov32,ECX,imm);
		x86e->Emit(op_add32,ECX,GetRegPtr(reg1));
	}
}
void readwrteparams2(u8 reg1,u8 reg2)
{
	if (ira->IsRegAllocated(reg1))
	{
		x86_reg r1=LoadReg(ECX,reg1);
		assert(r1!=ECX);
		
		if (ira->IsRegAllocated(reg2))
		{
			//lea ecx,[reg1+reg2]
			x86_reg r2=LoadReg(ECX,reg2);
			assert(r2!=ECX);
			x86e->Emit(op_lea32,ECX,x86_mrm::create(r1,r2));
		}
		else
		{
			//mov ecx,reg1
			//add ecx,[reg2]
			x86e->Emit(op_mov32,ECX,r1);
			x86e->Emit(op_add32,ECX,GetRegPtr(reg2));
		}
	}
	else
	{
		if (ira->IsRegAllocated(reg2))
		{
			readwrteparams2(reg2,reg1);
		}
		else
		{
			//mov ecx,[reg1]
			//add ecx,[reg2]
			x86e->Emit(op_mov32,ECX,GetRegPtr(reg1));
			x86e->Emit(op_add32,ECX,GetRegPtr(reg2));
		}
	}
}
void readwrteparams3(u8 reg1,u8 reg2,u32 imm)
{
	if (ira->IsRegAllocated(reg1))
	{
		x86_reg r1=LoadReg(ECX,reg1);
		assert(r1!=ECX);
		
		if (ira->IsRegAllocated(reg2))
		{
			//lea ecx,[reg1+reg2]
			x86_reg r2=LoadReg(ECX,reg2);
			assert(r2!=ECX);
			x86e->Emit(op_lea32,ECX,x86_mrm::create(r1,r2,sib_scale_1,x86_ptr::create(imm)));
		}
		else
		{
			//lea ecx,[reg1+imm]
			//add ecx,[reg2]
			x86e->Emit(op_lea32,ECX,x86_mrm::create(r1,x86_ptr::create(imm)));
			x86e->Emit(op_add32,ECX,GetRegPtr(reg2));
		}
	}
	else
	{
		if (ira->IsRegAllocated(reg2))
		{
			readwrteparams3(reg2,reg1,imm);
		}
		else
		{
			//mov ecx,[reg1]
			//add ecx,[reg2]
			x86e->Emit(op_mov32,ECX,GetRegPtr(reg1));
			x86e->Emit(op_add32,ECX,imm);
			x86e->Emit(op_add32,ECX,GetRegPtr(reg2));
		}
	}
}
//read-write
x86_reg  readwrteparams(shil_opcode* op)
	{
	assert(0==(op->flags & FLAG_IMM2));
	assert(op->flags & FLAG_REG1);

	bool Loaded=false;

	//can use
	//mov ecx,imm
	//lea ecx[r[2]+imm]
	//lea ecx[r0/gbr+imm]
	//mov ecx,r0/gbr
	//mov ecx,r[2]
	//lea ecx[r0/gbr+r[2]*1]
	//lea ecx,[r0/gbr+r[2]*1+imm] ;)
	
	u32 flags=0;
	#define flag_imm 1
	#define flag_r2 2
	#define flag_r0 4
	#define flag_gbr 8

	if (op->flags & FLAG_IMM1)
	{
		if (op->imm1!=0)	//gota do that on const elimiation pass :D
			flags|=flag_imm;
		/*
		Loaded=true;
		x86e->Emit(op_mov32,ECX,op->imm1);
		*/
	}
	if (op->flags & FLAG_REG2)
	{
		flags|=flag_r2;
		/*
		x86_gpr_reg r1=LoadReg(EAX,op->reg2);
		if (Loaded)
			x86e->Emit(op_add32,ECX,r1);
		else
			x86e->Emit(op_mov32,ECX,r1);
		Loaded=true;
		*/
	}
	if (op->flags & FLAG_R0)
	{
		flags|=flag_r0;
		/*
		x86_gpr_reg r1=LoadReg(EAX,r0);
		if (Loaded)
			x86e->Emit(op_add32,ECX,r1);
		else
			x86e->Emit(op_mov32,ECX,r1);
		Loaded=true;
		*/
	}
	if (op->flags & FLAG_GBR)
	{
		flags|=flag_gbr;
		/*
		x86_gpr_reg r1=LoadReg(EAX,reg_gbr);
		if (Loaded)
			x86e->Emit(op_add32,ECX,r1);
		else
			x86e->Emit(op_mov32,ECX,r1);
		Loaded=true;
		*/
	}
	
	verify(flags!=0);
	x86_reg reg=ERROR_REG;

	switch(flags)
	{
		//1 olny
	case flag_imm:
		x86e->Emit(op_mov32,ECX,op->imm1);
		reg=ECX;
		break;

	case flag_r2:
		reg=LoadReg(ECX,op->reg2);
		break;

	case flag_r0:
		reg=LoadReg(ECX,r0);
		break;

	case flag_gbr:
		reg=LoadReg(ECX,reg_gbr);
		break;

		//2 olny
	case flag_imm | flag_r2:
		readwrteparams1(op->reg2,op->imm1);
		reg=ECX;
		break;

	case flag_imm | flag_r0:
		readwrteparams1((u8)r0,op->imm1);
		reg=ECX;
		break;

	case flag_imm | flag_gbr:
		readwrteparams1((u8)reg_gbr,op->imm1);
		reg=ECX;
		break;

	case flag_r2 | flag_r0:
		readwrteparams2(op->reg2,(u8)r0);
		reg=ECX;
		break;

	case flag_r2 | flag_gbr:
		readwrteparams2(op->reg2,(u8)reg_gbr);
		reg=ECX;
		break;

		//3 olny
	case flag_imm | flag_r2 | flag_gbr:
		readwrteparams3(op->reg2,(u8)reg_gbr,op->imm1);
		reg=ECX;
		break;

	case flag_imm | flag_r2 | flag_r0:
		readwrteparams3(op->reg2,(u8)r0,op->imm1);
		reg=ECX;
		break;

	default:
		die("Unable to compute readwrteparams");
		break;
	}

	verify(reg!=ERROR_REG);
	return reg;
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
			x86e->Emit(op_mov32,ECX,op->imm1);
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
				x86_gpr_reg r2=LoadReg(EAX,op->reg2);
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

void emit_vmem_op_compat_const(x86_block* x86e,u32 ra,
							   x86_gpr_reg ro,x86_sse_reg ro_sse,bool sse,
								u32 sz,u32 rw);
u32 m_unpack_sz[3]={1,2,4};
void emit_vmem_read(x86_reg reg_addr,u8 reg_out,u32 sz);
void emit_vmem_write(x86_reg reg_addr,u8 reg_data,u32 sz);
void __fastcall shil_compile_readm(shil_opcode* op,BasicBlock* block)
{
	u32 size=op->flags&3;

	//sse_RLF(op->reg1);//reload possibly readed reg
	if (INLINE_MEM_READ_CONST)
	{
		//if constant read , and on ram area , make it a direct mem access
		//_watch_ mmu
		if (!(op->flags & (FLAG_R0|FLAG_GBR|FLAG_REG2)))
		{//[reg2+imm] form
			assert(op->flags & FLAG_IMM1);
			//[imm1] form
			if (!IsInFReg(op->reg1))
			{
				x86_gpr_reg rall=LoadReg_nodata(EDX,op->reg1);
				emit_vmem_op_compat_const(x86e,op->imm1,rall,XMM0,false,m_unpack_sz[size],0);
				SaveReg(op->reg1,rall);
			}
			else
			{
				x86_sse_reg rall=fra->GetRegister(XMM0,op->reg1,RA_NODATA);
				emit_vmem_op_compat_const(x86e,op->imm1,EAX,rall,true,m_unpack_sz[size],0);
				fra->SaveRegister(op->reg1,rall);
			}
			return;
		}
	}

	x86_reg reg_addr = readwrteparams(op);

	emit_vmem_read(reg_addr,op->reg1,m_unpack_sz[size]);
	
	/*
	//movsx [if needed]
	if (size==0)
	{
		x86e->Emit(op_movsx8to32, EAX,EAX);	//se8
	}
	else if (size==1)
	{
		x86e->Emit(op_movsx16to32, EAX,EAX);	//se16
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
	*/
}
void __fastcall shil_compile_writem(shil_opcode* op,BasicBlock* block)
{
	//sse_WBF(op->reg1);//Write back possibly readed reg
	u32 size=op->flags&3;

	//if constant read , and on ram area , make it a direct mem access
	//_watch_ mmu
	if (!(op->flags & (FLAG_R0|FLAG_GBR|FLAG_REG2)))
	{//[reg2+imm] form
		assert(op->flags & FLAG_IMM1);
		//[imm1] form
		if (!IsInFReg(op->reg1))
		{
			x86_gpr_reg rall=LoadReg(EDX,op->reg1);
			emit_vmem_op_compat_const(x86e,op->imm1,rall,XMM0,false,m_unpack_sz[size],1);
		}
		else
		{
			x86_sse_reg rall=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			emit_vmem_op_compat_const(x86e,op->imm1,EAX,rall,true,m_unpack_sz[size],1);
		}
		return;
	}

	x86_reg reg_addr = readwrteparams(op);

	//ECX is address

	//so it's sure loaded (if from reg cache)
	/*x86_gpr_reg r1;
	if (IsSSEAllocReg(op->reg1))
	{
		r1=EDX;
		fra->LoadRegisterGPR(EDX,op->reg1);
	}
	else
		r1=LoadReg(EDX,op->reg1);
	
	emit_vmem_op_compat(x86e,reg_addr,r1,m_unpack_sz[size],1);*/
	emit_vmem_write(reg_addr,op->reg1,m_unpack_sz[size]);
}

//save-loadT
void __fastcall shil_compile_SaveT(shil_opcode* op,BasicBlock* block)
{
	assert(op->flags & FLAG_IMM1);//imm1
	assert(0==(op->flags & (FLAG_IMM2|FLAG_REG1|FLAG_REG2)));//no imm2/r1/r2

	//hmm guess 32b stores are faster or smth ????

	//x86e->SETcc8M(GetRegPtr(reg_sr_T),op->imm1);//imm1 :P
	
	x86e->Emit(SetCC[op->imm1],EAX);
	x86e->Emit(op_movzx8to32, EAX,EAX);				//zero out rest of eax
	x86e->Emit(op_mov32,GetRegPtr(reg_sr_T),EAX);
		
}
void __fastcall shil_compile_LoadT(shil_opcode* op,BasicBlock* block)
{
	assert(op->flags & FLAG_IMM1);//imm1
	assert(0==(op->flags & (FLAG_IMM2|FLAG_REG1|FLAG_REG2)));//no imm2/r1/r2
	

	assert( (op->imm1==x86_flags::CF) || (op->imm1==x86_flags::jcond_flag) );

	if (op->imm1==x86_flags::jcond_flag)
	{
		LoadReg_force(EAX,reg_sr_T);
		x86e->Emit(op_mov32,&T_jcond_value,EAX);//T_jcond_value;
	}
	else
	{
		LoadReg_force(EAX,reg_sr_T);
		x86e->Emit(op_shr32,EAX,1);//heh T bit is there now :P CF
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
			x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
			x86e->Emit(op_cmp32,r1,op->imm1);
		}
		else
		{
			x86e->Emit(op_cmp32,GetRegPtr(op->reg1),op->imm1);
		}
		//eflags is used w/ combination of SaveT
	}
	else
	{
		assert(0==(op->flags & FLAG_IMM2));
		assert(op->flags & FLAG_REG2);

		x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
		if (ira->IsRegAllocated(op->reg2))
		{
			x86_gpr_reg r2 = LoadReg(ECX,op->reg2);
			x86e->Emit(op_cmp32,r1,r2);//rm,rn
		}
		else
		{
			x86e->Emit(op_cmp32,r1,GetRegPtr(op->reg2));//rm,rn
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
			x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
			x86e->Emit(op_test32,r1,op->imm1);
		}
		else
		{
			x86e->Emit(op_test32,GetRegPtr(op->reg1),op->imm1);
		}
		//eflags is used w/ combination of SaveT
	}
	else
	{
		assert(0==(op->flags & FLAG_IMM2));
		assert(op->flags & FLAG_REG2);

		x86_gpr_reg r1 = LoadReg(EAX,op->reg1);
		if (ira->IsRegAllocated(op->reg2))
		{
			x86_gpr_reg r2 = LoadReg(ECX,op->reg2);
			x86e->Emit(op_test32,r1,r2);//rm,rn
		}
		else
		{
			x86e->Emit(op_test32,r1,GetRegPtr(op->reg2));//rm,rn
		}
		//eflags is used w/ combination of SaveT
	}
}

//add-sub
void __fastcall shil_compile_add(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(op_add32);
}
void __fastcall shil_compile_adc(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(op_adc32);
}
void __fastcall shil_compile_sub(shil_opcode* op,BasicBlock* block)
{
	OP_RegToReg_simple(op_sub32);
}

//**
void __fastcall shil_compile_jcond(shil_opcode* op,BasicBlock* block)
{
	printf("jcond ... heh not implemented\n");
	assert(false);
}
void __fastcall shil_compile_jmp(shil_opcode* op,BasicBlock* block)
{
	printf("jmp ... heh not implemented\n");
}

void load_with_se16(x86_gpr_reg to,u8 from)
{
	if (ira->IsRegAllocated(from))
	{
		x86_gpr_reg r1=LoadReg(EAX,from);
		x86e->Emit(op_movsx16to32, to,r1);
	}
	else
		x86e->Emit(op_movsx16to32, to,(u16*)GetRegPtr(from));
}

void load_with_ze16(x86_gpr_reg to,u8 from)
{
	if (ira->IsRegAllocated(from))
	{
		x86_gpr_reg r1=LoadReg(EAX,from);
		x86e->Emit(op_movzx16to32, to,r1);
	}
	else
		x86e->Emit(op_movzx16to32, to,(u16*)GetRegPtr(from));
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
		x86_gpr_reg r1,r2;
		if (sz==FLAG_16)
		{
			//FlushRegCache_reg(op->reg1);
			//FlushRegCache_reg(op->reg2);

			if (op->flags & FLAG_SX)
			{
				//x86e->Emit(op_movsx16to32, EAX,(u16*)GetRegPtr(op->reg1));
				load_with_se16(EAX,op->reg1);
				//x86e->Emit(op_movsx16to32, ECX,(u16*)GetRegPtr(op->reg2));
				load_with_se16(ECX,op->reg2);
			}
			else
			{
				//x86e->Emit(op_movzx16to32, EAX,(u16*)GetRegPtr(op->reg1));
				load_with_ze16(EAX,op->reg1);
				//x86e->Emit(op_movzx16to32, ECX,(u16*)GetRegPtr(op->reg2));
				load_with_ze16(ECX,op->reg2);
			}
		}
		else
		{
			//x86e->Emit(op_mov32,EAX,GetRegPtr(op->reg1));
			//x86e->Emit(op_mov32,ECX,GetRegPtr(op->reg2));
			r1=LoadReg_force(EAX,op->reg1);
			r2=LoadReg_force(ECX,op->reg2);
		}

		if (op->flags & FLAG_SX)
			x86e->Emit(op_imul32,EAX,ECX);
		else
			x86e->Emit(op_mul32,ECX);
		
		SaveReg((u8)reg_macl,EAX);
	}
	else
	{
		assert(sz==FLAG_64);

//		FlushRegCache_reg(op->reg1);
//		FlushRegCache_reg(op->reg2);
		
		ira->FlushRegister(op->reg1);
		ira->FlushRegister(op->reg2);

		x86e->Emit(op_mov32,EAX,GetRegPtr(op->reg1));

		if (op->flags & FLAG_SX)
			x86e->Emit(op_imul32,x86_ptr(GetRegPtr(op->reg2)));
		else
			x86e->Emit(op_mul32,x86_ptr(GetRegPtr(op->reg2)));

		SaveReg((u8)reg_macl,EAX);
		SaveReg((u8)reg_mach,EDX);
	}
}


void __fastcall shil_compile_div32(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM2)));

	//x86e->Emit(op_int3);
	u8 rQuotient=op->reg1;
	u8 rDivisor=op->reg2;
	u8 rDividend=(u8)op->imm1;


	x86_gpr_reg Quotient=LoadReg_force(EAX,rQuotient);



	x86_gpr_reg Dividend=LoadReg_force(EDX,rDividend);


	if (ira->IsRegAllocated(rDivisor))
	{
		x86_gpr_reg Divisor=LoadReg(EAX,rDivisor);
		if (op->flags & FLAG_SX)
		{
			x86e->Emit(op_idiv32,Divisor);
		}
		else
		{
			x86e->Emit(op_div32,Divisor);
		}
	}
	else
	{
		if (op->flags & FLAG_SX)
		{
			x86e->Emit(op_idiv32,x86_ptr(GetRegPtr(rDivisor)));
		}
		else
		{
			x86e->Emit(op_div32,x86_ptr(GetRegPtr(rDivisor)));
		}
	}

	if (op->flags & FLAG_SX)
	{
		x86e->Emit(op_sar32 ,EAX,1);
	}
	else
	{
		x86e->Emit(op_shr32 ,EAX,1);
	}

	//set T
	//WTF ? why doing both is faster?!?!?! WTFHH ?H?H?HH?H?H
	//TODO : Add an option for this :)
	x86e->Emit(op_setb,x86_ptr(GetRegPtr(reg_sr_T)));
	/*
	x86e->SETcc8R(ECX,CC_B);
	x86e->Emit(op_movzx8to32, ECX,ECX);		//clear rest of eax (to remove partial depency on 32:8)
	x86e->Emit(op_mov32,GetRegPtr(reg_sr_T),ECX);
	*/

	//x86e->AND32ItoM(GetRegPtr(reg_sr),~1);
	//x86e->Emit(op_movzx8to32, ECX,ECX);
	//x86e->OR32RtoM(GetRegPtr(reg_sr),ECX);


	SaveReg(rQuotient,Quotient);

	//WARNING--JUMP--
	//thanks to teh way i save sr , test has to be re-done here
	//->x86e->Emit(op_test32,ECX,ECX);

	x86_Label* exit =x86e->CreateLabel(false,8);
	x86_Label* no_fixup =x86e->CreateLabel(false,8);

	//u8* j1=x86e->JNZ8(0);
	x86e->Emit(op_jb ,no_fixup);

	if (ira->IsRegAllocated(rDivisor))
	{	//safe to do here b/c rDivisor was loaded to reg above (if reg cached)
		x86_gpr_reg t=LoadReg(EAX,rDivisor);
		x86e->Emit(op_sub32 ,EDX,t);
	}
	else
	{
		x86e->Emit(op_sub32 ,EDX,GetRegPtr(rDivisor));
	}

	SaveReg(rDividend,Dividend);

	//WARNING--JUMP--

	//u8* j2=x86e->JMP8(0);
	x86e->Emit(op_jmp ,exit);

	//x86e->x86SetJ8(j1);
	x86e->MarkLabel(no_fixup);

	SaveReg(rDividend,Dividend);

	//WARNING--JUMP--
	//x86e->x86SetJ8(j2);
	x86e->MarkLabel(exit);

	ira->MarkDirty(rDividend);
	ira->MarkDirty(rQuotient);
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

#define SSE_SS_OP(op,op_cl)\
	switch (frs(op))\
	{\
	case fa_r1r2:\
		{\
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);\
			x86_sse_reg r2=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);\
			assert(r1!=XMM0 && r2!=XMM0);\
			x86e->Emit(op_cl,r1,r2);\
			fra->SaveRegister(op->reg1,r1);\
		}\
		break;\
	case fa_r1m2:\
		{\
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);\
			assert(r1!=XMM0);\
			x86e->Emit(op_cl,r1,x86_ptr(GetRegPtr(op->reg2)));\
			fra->SaveRegister(op->reg1,r1);\
		}\
		break;\
	case fa_m1r2:\
		{\
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);\
			x86_sse_reg r2=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);\
			assert(r1==XMM0);\
			assert(r2!=XMM0);\
			x86e->Emit(op_cl,r1,r2);\
			fra->SaveRegister(op->reg1,r1);\
		}\
		break;\
	case fa_m1m2:\
		{\
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);\
			assert(r1==XMM0);\
			x86e->Emit(op_cl,r1,x86_ptr(GetRegPtr(op->reg2)));\
			fra->SaveRegister(op->reg1,r1);\
		}\
		break;\
	}

#define SSE_s(op,sseop) SSE_SS_OP(op,sseop);
//simple opcodes
void __fastcall shil_compile_fadd(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		SSE_s(op,op_addss);
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
		
		SSE_s(op,op_subss);
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

		SSE_s(op,op_mulss);
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

		SSE_s(op,op_divss);
	}
	else
	{
		assert(false);
	}
}

//binary opcodes
void __fastcall shil_compile_fneg(shil_opcode* op,BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64 ((Sh4RegType)op->reg1));
		if (IsInFReg(op->reg1))
		{
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(r1!=XMM0);
			x86e->Emit(op_xorps,r1,ps_not_data);
			fra->SaveRegister(op->reg1,r1);
		}
		else
		{
			x86e->Emit(op_xor32,GetRegPtr(op->reg1),0x80000000);
		}
	}
	else
	{
		assert(sz==FLAG_64);
		assert(IsReg64((Sh4RegType)op->reg1));
		u32 reg=GetSingleFromDouble(op->reg1);
		x86e->Emit(op_xor32,GetRegPtr(reg+1),0x80000000);
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
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(r1!=XMM0);
			x86e->Emit(op_andps,r1,ps_and_data);
			fra->SaveRegister(op->reg1,r1);
		}
		else
		{
			x86e->Emit(op_and32,GetRegPtr(op->reg1),0x7FFFFFFF);
		}
	}
	else
	{
		assert(sz==FLAG_64);
		assert(IsReg64((Sh4RegType)op->reg1));
		u32 reg=GetSingleFromDouble(op->reg1);
		x86e->Emit(op_and32,GetRegPtr(reg+1),0x7FFFFFFF);
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

		//x86e->Emit(op_movss,XMM0,GetRegPtr(op->reg1));
		x86_sse_reg fr1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
		
		//x86e->SSE_UCOMISS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));
		if (fra->IsRegAllocated(op->reg2))
		{
			x86_sse_reg fr2=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);
			assert(fr2!=XMM0);
			x86e->Emit(op_ucomiss, fr1,fr2);
		}
		else
		{
			x86e->Emit(op_ucomiss ,fr1,GetRegPtr(op->reg2));
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

		//x86e->Emit(op_movss,XMM0,GetRegPtr(fr_0));		//xmm0=fr[0]
		x86_sse_reg fr0=fra->GetRegister(XMM0,fr_0,RA_FORCE);
		assert(fr0==XMM0);
		
		//x86e->SSE_MULSS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));	//xmm0*=fr[m]
		if (fra->IsRegAllocated(op->reg2))
		{
			x86_sse_reg frm=fra->GetRegister(XMM0,op->reg2,RA_DEFAULT);
			assert(frm!=XMM0);
			x86e->Emit(op_mulss ,fr0,frm);
		}
		else
		{
			x86e->Emit(op_mulss ,XMM0,GetRegPtr(op->reg2));
		}

		//x86e->SSE_ADDSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));	//xmm0+=fr[n] 
		if (fra->IsRegAllocated(op->reg1))
		{
			x86_sse_reg frn=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(frn!=XMM0);
			x86e->Emit(op_addss ,fr0,frn);
		}
		else
		{
			x86e->Emit(op_addss ,fr0,GetRegPtr(op->reg1));
		}
		
		
		//x86e->Emit(op_movss,GetRegPtr(op->reg1),XMM0);	//fr[n]=xmm0
		fra->SaveRegister(op->reg1,fr0);
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
			x86_sse_reg fr1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(fr1!=XMM0);
			x86e->Emit(op_sqrtss ,fr1,fr1);
			fra->SaveRegister(op->reg1,fr1);
		}
		else
		{
			x86e->Emit(op_sqrtss ,XMM0,GetRegPtr(op->reg1));
			x86e->Emit(op_movss,GetRegPtr(op->reg1),XMM0);
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
		//x86e->Emit(op_movss,XMM0,(u32*)mm_1);			//XMM0=1
		//x86e->SSE_DIVSS_XMM_to_XMM(XMM0,XMM1);					//XMM0=1/sqrt
		//or
		//x86e->SSE_RSQRTSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));//XMM0=APPR(1/sqrt(fr1))
		//-> im using Approximate version , since this is an aproximate opcode on sh4 too
		//i hope x86 isnt less accurate ..

		if (fra->IsRegAllocated(op->reg1))
		{
			x86_sse_reg fr1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			verify(fr1!=XMM0);
#ifdef _FAST_fssra
			x86e->SSE_RSQRTSS_XMM_to_XMM(fr1,fr1);
#else
			//fra->FlushRegister_xmm(XMM7);
			x86e->Emit(op_sqrtss ,XMM0,fr1);				//XMM0=sqrt(fr1)
			x86e->Emit(op_movss,fr1,(u32*)mm_1);			//fr1=1
			x86e->Emit(op_divss,fr1,XMM0);				//fr1=1/XMM0
#endif
			fra->SaveRegister(op->reg1,fr1);
		}
		else
		{
			#ifdef _FAST_fssra
			x86e->SSE_RSQRTSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));//XMM0=APPR(1/sqrt(fr1))
			#else
			fra->FlushRegister_xmm(XMM7);
			x86e->Emit(op_sqrtss ,XMM7,GetRegPtr(op->reg1));	//XMM7=sqrt(fr1)
			x86e->Emit(op_movss ,XMM0,(u32*)mm_1);			//XMM0=1
			x86e->Emit(op_divss ,XMM0,XMM7);					//XMM0=1/XMM7
			#endif
			x86e->Emit(op_movss ,GetRegPtr(op->reg1),XMM0);	//fr1=XMM0
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
		x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_NODATA);
		x86e->Emit(op_cvtsi2ss ,r1,GetRegPtr(reg_fpul));
		//x86e->Emit(op_movss,GetRegPtr(op->reg1),XMM0);
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
		if (fra->IsRegAllocated(op->reg1))
		{
			x86_sse_reg r1=fra->GetRegister(XMM0,op->reg1,RA_DEFAULT);
			assert(r1!=XMM0);
			x86e->Emit(op_cvttss2si, EAX,r1);
		}
		else
		{
			x86e->Emit(op_cvttss2si, EAX,GetRegPtr(op->reg1));
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
		x86e->Emit(op_fild32i,x86_ptr(GetRegPtr(reg_fpul)));		//st(0)=(s32)fpul
		x86e->Emit(op_fmul32f,x86_ptr(fsca_fpul_adj));			//st(0)=(s32)fpul * ((2*pi)/65536.0f)
		x86e->Emit(op_fsincos);						//st(0)=sin , st(1)=cos
		
		x86e->Emit(op_fstp32f,x86_ptr(GetRegPtr(op->reg1 +1)));	//Store cos to reg+1
		x86e->Emit(op_fstp32f,x86_ptr(GetRegPtr(op->reg1)));		//store sin to reg

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

		x86e->Emit(op_movaps ,XMM0,GetRegPtr(op->reg1));
		x86e->Emit(op_movaps ,XMM3,XMM0);
		x86e->Emit(op_shufps ,XMM0,XMM0,0);
		x86e->Emit(op_movaps ,XMM1,XMM3);
		x86e->Emit(op_movaps ,XMM2,XMM3);
		x86e->Emit(op_shufps ,XMM1,XMM1,0x55);
		x86e->Emit(op_shufps ,XMM2,XMM2,0xaa);
		x86e->Emit(op_shufps ,XMM3,XMM3,0xff);

		x86e->Emit(op_mulps ,XMM0,GetRegPtr(xf_0));
		x86e->Emit(op_mulps ,XMM1,GetRegPtr(xf_4));
		x86e->Emit(op_mulps ,XMM2,GetRegPtr(xf_8));
		x86e->Emit(op_mulps ,XMM3,GetRegPtr(xf_12));

		x86e->Emit(op_addps ,XMM0,XMM1);
		x86e->Emit(op_addps ,XMM2,XMM3);
		x86e->Emit(op_addps ,XMM0,XMM2);

		x86e->Emit(op_movaps ,GetRegPtr(op->reg1),XMM0);
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

		x86e->Emit(op_movaps ,XMM0,GetRegPtr(op->reg1));
		x86e->Emit(op_mulps ,XMM0,GetRegPtr(op->reg2));
		x86e->Emit(op_movhlps ,XMM1,XMM0);
		x86e->Emit(op_addps ,XMM0,XMM1);
		x86e->Emit(op_movaps ,XMM1,XMM0);
		x86e->Emit(op_shufps ,XMM1,XMM1,1);
		x86e->Emit(op_addss ,XMM0,XMM1);
		x86e->Emit(op_movss,GetRegPtr(op->reg1+3),XMM0);
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
CompiledBasicBlock* Curr_block;

u32* block_stack_pointer;
//sp is 0 if manual discard
void CBBs_BlockSuspended(CompiledBlockInfo* block,u32* sp)
{
	u32* sp_inblock=block_stack_pointer-1;

	if(sp_inblock==sp)
	{
		//printf("Exeption within the same block !\n");
	}
	else
	{
		if (sp!=0)
		{
			//printf("Exeption possibly within the same block ; 0x%X\n",sp_inblock[-1]);
			//printf("Block EP : 0x%X , sz : 0x%X\n",block->Code,block->size);
		}
	}
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
void PatchDynamicLinkGeneric(void* ptr)
{
	/*
	x86_block* x86e = new x86_block();
	x86e->x86Ptr=(s8*)ptr;*/

}
void CompileBasicBlock_slow(BasicBlock* block)
{
	if (!inited)
	{
		Init();
		inited=true;
	}

	x86e=new x86_block();
	
	x86e->Init();

	CompiledBasicBlock* cBB;
	
	block->flags.DisableHS=1;

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

	/*
	//that is a realy nice debug helper :)
	x86e->Emit(op_mov32,&Curr_block,(u32)cBB);
	*/
	if (block->flags.ProtectionType==BLOCK_PROTECTIONTYPE_MANUAL)
	{
		int sz=block->end-block->start;
		//check at least 4 bytes
		sz=(sz +3) & (~3);

		if (sz<4)
			sz=1;
		else
			sz/=4;
		//sz++;
		int i=0;
		//that can be optimised a lota :p
		
		x86_Label* exit_discard_block= x86e->CreateLabel(false,0);
		x86_Label* execute_block= x86e->CreateLabel(false,0);

		for (i=0;i<sz;i++)
		{
			u32* pmem=(u32*)GetMemPtr(block->start+i*4,4);
			x86e->Emit(op_cmp32 ,GetMemPtr(block->start+i*4,4),*pmem);
			//u8* patch=x86e->JE8(0);
			//x86e->x86SetJ8(patch);
			if (i!=(sz-1))
			{
				x86e->Emit(op_jne ,exit_discard_block);
			}
			else
			{
				x86e->Emit(op_je ,execute_block);
			}
		}

		x86e->MarkLabel(exit_discard_block);
		x86e->Emit(op_mov32,ECX,(u32)cBB);
		x86e->Emit(op_mov32,GetRegPtr(reg_pc),block->start);
		x86e->Emit(op_call,x86_ptr_imm(SuspendBlock));
		x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_no_update));
		x86e->MarkLabel(execute_block);
	}

	verify(do_hs==false);
	/*
	if (do_hs)
	{
		//check for block promotion to superblock ;)
		x86e->Emit(op_dec32 ,&cBB->cbi.GetHS()->bpm_ticks);
		
		u8* not_zero=x86e->JNZ8(0);
		{
			//yay , 0 , see if it needs promotion kkthxdie
			x86e->Emit(op_mov32,EAX,&gcp_timer);//now
			x86e->SUB32MtoR(EAX,&cBB->cbi.GetHS()->gcp_lasttimer);//now-last
			x86e->CMP32ItoR(EAX,16);
			//if it took more that 16 ticks , then its less that 10% , no promotion
			u8*no_promote= x86e->JBE8(0);
			{
				//suspend block
				x86e->Emit(op_mov32,ECX,(u32)cBB);
				x86e->Emit(op_call,x86_ptr_imm(SuspendBlock));
				void*  __fastcall CompileCode_SuperBlock(u32 pc);
				x86e->Emit(op_mov32,ECX,(u32)cBB->cbi.start);
				x86e->Emit(op_call,x86_ptr_imm(CompileCode_SuperBlock));
				x86e->JMP32R(EAX);
			}
			x86e->x86SetJ8(no_promote);
			x86e->ADD32RtoM(&cBB->cbi.GetHS()->gcp_lasttimer,EAX);//last+now-last=now ;)
			x86e->Emit(op_mov32,&cBB->cbi.GetHS()->bpm_ticks,3022);
		}
		x86e->x86SetJ8(not_zero);

		//16 ticks or more to convert to zuper block
		//16 ticks -> 241760hrz /8 ~=30220 blocks
		//we promote to superblock if more that 20% of the time is spent on this block , 3022 ticks
		cBB->cbi.GetHS()->gcp_lasttimer=gcp_timer;
		cBB->cbi.GetHS()->bpm_ticks=3022*2;
	}
	*/
	
	//s8* start_ptr;
	x86_Label* block_start = x86e->CreateLabel(false,0);

	if (PROFILE_BLOCK_CYCLES)
	{
		//start_ptr=x86e->x86Ptr;
		x86e->MarkLabel(block_start);
		x86e->Emit(op_call,x86_ptr_imm(dyna_profile_block_enter));
	}

	fra=GetFloatAllocator();
	ira=GetGPRtAllocator();
	
	ira->DoAllocation(block,x86e);
	fra->DoAllocation(block,x86e);

	ira->BeforeEmit();
	fra->BeforeEmit();

	if (PROFILE_BLOCK_CYCLES==false)
	{
		//start_ptr=x86e->x86Ptr;
		x86e->MarkLabel(block_start);
	}

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

	ira->BeforeTrail();
	fra->BeforeTrail();

	if (PROFILE_BLOCK_CYCLES)
	{
		x86e->Emit(op_mov32,ECX,(u32)(cBB->cbi.GetNP()));
		x86e->Emit(op_call,x86_ptr_imm(dyna_profile_block_exit_BasicBlock));
	}

	//end block acording to block type :)
	switch(block->flags.ExitType)
	{
	
	case BLOCK_EXITTYPE_DYNAMIC_CALL:
		{
			//mov guess,pr
			x86e->Emit(op_mov32,&call_ret_address,cBB->ebi.TT_next_addr);
			//mov pguess,this
			x86e->Emit(op_mov32,(u32*)&pcall_ret_address,(u32)(cBB));
		}
	case BLOCK_EXITTYPE_DYNAMIC:
		{
			x86e->Emit(op_sub32 ,&rec_cycles,block->cycles);
			x86e->Emit(op_jns,x86_ptr_imm(Dynarec_Mainloop_no_update));
			x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_do_update));
			break;
		}

	case BLOCK_EXITTYPE_RET:
		{
			//x86_Label* not_ok=x86e->CreateLabel(false);

			//link end ?
			x86e->Emit(op_sub32 ,&rec_cycles,block->cycles);
			x86e->Emit(op_js,x86_ptr_imm(Dynarec_Mainloop_do_update));

			//cmp pr,guess
			x86e->Emit(op_mov32 ,EAX,GetRegPtr(reg_pc));
			x86e->Emit(op_cmp32 ,EAX,&call_ret_address);
			//je ok
			x86e->Emit(op_jne ,x86_ptr_imm(Dynarec_Mainloop_no_update));
			//ok:
			//mov ecx , pcall_ret_address
			x86e->Emit(op_mov32 ,ECX,(u32*)&pcall_ret_address);
			//mov eax,[pcall_ret_address+codeoffset]
			x86e->Emit(op_jmp32,x86_mrm::create(ECX,x86_ptr::create(offsetof(CompiledBasicBlock,ebi.pTT_next_addr))));
			/*
			x86e->Emit(op_mov32 ,EAX,ECX);
			x86e->Emit(op_add32 ,EAX,);
			x86e->MOV32RmtoR(EAX,EAX);//get ptr to compiled block/link stub
			//jmp eax
			x86e->JMP32R(EAX);	//jump to it
			*/

			//never gets here :D
			//x86e->x86SetJ8(not_ok);
			//x86e->MarkLabel(not_ok);
			//not_ok

			//ret
			//x86e->Emit(op_ret);

			break;
		}

	case BLOCK_EXITTYPE_COND:
		{
			//ok , handle COND here :)
			//mem address
			u32* TF_a=&cBB->ebi.TT_next_addr;
			u32* TT_a=&cBB->ebi.TF_next_addr;
			
			//functions
			u32* pTF_f=(u32*)&(cBB->ebi.pTT_next_addr);
			u32* pTT_f=(u32*)&(cBB->ebi.pTF_next_addr);

			x86e->Emit(op_sub32 ,&rec_cycles,block->cycles);
			
			x86_Label* Exit_Link = x86e->CreateLabel(false,8);

			
			x86e->Emit(op_js ,Exit_Link);

			//Link:
			//if we can execute more blocks
			{
				//for dynamic link!
				x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , block
				x86e->Emit(op_test32,&T_jcond_value,1);
				//x86e->TEST32ItoR(EAX,1);//test for T
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
				x86e->Emit(op_mov32,EAX,pTF_f);		//assume it's this condition , unless CMOV overwrites
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
				x86e->Emit(op_mov32,EAX,pTT_f);// ;)
				}
				else*/
				x86e->Emit(op_cmovne32 ,EAX,pTT_f);	//overwrite the "other" pointer if needed
				//}
				x86e->Emit(op_jmp32,EAX);		 //!=
			}
			
			x86e->MarkLabel(Exit_Link);
			{
				//If our cycle count is expired
				//save the dest address to pc
				/*
				x86e->Emit(op_mov32,EAX,&T_jcond_value);
				x86e->TEST32ItoR(EAX,1);//test for T
				*/
				x86e->Emit(op_test32,&T_jcond_value,1);
				//see witch pc to set

				x86e->Emit(op_mov32,EAX,*TF_a);//==
				//!=
				x86e->Emit(op_cmovne32 ,EAX,TT_a);//!=
				x86e->Emit(op_mov32,GetRegPtr(reg_pc),EAX);

				//return to caller to check for interrupts
				x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_do_update));
				//x86e->Emit(op_ret);
			}
		} 
		break;

	case BLOCK_EXITTYPE_FIXED_CALL:
		//mov guess,pr
		{
			x86e->Emit(op_mov32,&call_ret_address,cBB->ebi.TT_next_addr);
			//mov pguess,this
			x86e->Emit(op_mov32,(u32*)&pcall_ret_address,(u32)(cBB));
		}
	case BLOCK_EXITTYPE_FIXED:
		{
			//
			//x86e->Emit(op_cmp32 ,&rec_cycles,BLOCKLIST_MAX_CYCLES);

			x86e->Emit(op_sub32 ,&rec_cycles,block->cycles);
			x86_Label* No_Link = x86e->CreateLabel(false,8);

			x86e->Emit(op_js ,No_Link);

			//Link:
			//if we can execute more blocks
			if (cBB->ebi.TF_next_addr==cBB->cbi.start)
			{
				//__asm int 03;
				printf("Fast Link possible\n");
			}

			//link to next block :
			x86e->Emit(op_mov32,ECX,(u32)cBB);					//mov ecx , cBB
			x86e->Emit(op_jmp32,x86_ptr((u32*)&(cBB->ebi.pTF_next_addr)));	//mov eax , [pTF_next_addr]
			//x86e->Emit(op_jmp32 ,EAX);									//jmp eax

			//If our cycle count is expired
			x86e->MarkLabel(No_Link);
			//save pc
			x86e->Emit(op_mov32,GetRegPtr(reg_pc),cBB->ebi.TF_next_addr);
			//and return to caller to check for interrupts
			x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_do_update));
			break;
		}
	case BLOCK_EXITTYPE_FIXED_CSC:
		{
			//We have to exit , as we gota do mode lookup :)
			//We also have to reset return cache to ensure its ok

			x86e->Emit(op_sub32 ,&rec_cycles,block->cycles);
			//call_ret_address=0xFFFFFFFF;
			x86e->Emit(op_mov32,&call_ret_address,0xFFFFFFFF);
			//pcall_ret_address=0;
			x86e->Emit(op_mov32,(u32*)&pcall_ret_address,0);
			//Good , now return to caller :)
			//x86e->Emit(op_ret);
			x86e->Emit(op_jns,x86_ptr_imm(Dynarec_Mainloop_no_update));
			x86e->Emit(op_jmp,x86_ptr_imm(Dynarec_Mainloop_do_update));
			break;
		}
	}

	ira->AfterTrail();
	fra->AfterTrail();

	x86e->Emit(op_int3);
	void* codeptr=x86e->Generate(0,0);//heh

	cBB->cbi.Code=(BasicBlockEP*)codeptr;
	cBB->cbi.size=x86e->x86_indx;

	//make it call the stubs , unless otherwise needed
	cBB->ebi.pTF_next_addr=bb_link_compile_inject_TF_stub;
	cBB->ebi.pTT_next_addr=bb_link_compile_inject_TT_stub;

	block_count++;
	
	/*
	if ((block_count%512)==128)
	{
		printf("Recompiled %d blocks\n",block_count);
		u32 rat=native>fallbacks?fallbacks:native;
		if (rat!=0)
			printf("Native/Fallback ratio : %d:%d [%d:%d]\n",native,fallbacks,native/rat,fallbacks/rat);
		else
			printf("Native/Fallback ratio : %d:%d [%d:%d]\n",native,fallbacks,native,fallbacks);
		printf("Average block size : %d opcodes ; ",(fallbacks+native)/block_count);
	}*/
	
	delete fra;
	delete ira;
	x86e->Free();
	delete x86e;
}

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
#ifdef OLD_VMEM
void emit_vmem_op(x86_block* x86e,
				  u32 ma,x86_gpr_reg ra,u32* pa,u32 amode,
				  x86_gpr_reg ro,x86_sse_reg xo,u32 omode,
				  u32 sz,u32 rw)
{
	u8* direct=0;
	u8* op_end=0;
	u32 index=0;
	u32 rb=0;

	if (amode==2)//pointer to ram to read
	{
		x86e->Emit(op_mov32,ECX,pa);
		amode=1;
		ra=ECX;
	}

	if (amode==1)
	{
		
		x86e->Emit(op_mov32,EAX,ra);
		x86e->Emit(op_shr32,EAX,16);
		if (ra!=ECX)
			x86e->Emit(op_mov32,ECX,ra);
		if (rw==0)//olny on read
		{
		x86e->Emit(op_mov32,EDX,ECX);
		x86e->Emit(op_and32,EDX,0xFFFF);
		}
		//_vmem_MemInfo
		//mov eax,[_vmem_MemInfo+eax*4];
		//8B 04 85 base_address
		x86e->write8(0x8b);
		x86e->write8(0x04);
		x86e->write8(0x85);
		x86e->write32((u32)&_vmem_MemInfo[0]);

		//test eax,0xFFFF0000;
		x86e->Emit(op_test32 ,EAX,0xFFFF0000);
	}
	else if (amode == 0)
	{
		index=ma>>16;
		rb=ma&0xFFFF;
		//x86e->Emit(op_mov32,EAX,0/*_vmem_MemInfo*/+index*4);
	}
	direct=x86e->JNZ8(0);
	
	//x86e->Emit(op_mov32,EAX,
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

#endif
//this is P4 optimised (its faster olny when shr takes a bit :) )
//on AMD we can move the shr just before the read , and do the edx work after the read
//so we cover the read stall :)
//corrupts ecx,eax,edx; preserves all else (fastcall + xmm safe + xmm fpstatus safe)
void emit_vmem_op_compat(x86_block* x86e,x86_gpr_reg ra,
					  x86_gpr_reg ro,
					  u32 sz,u32 rw)
{
	if (rw==0)
	{
		if (ra!=ECX)
			x86e->Emit(op_mov32,ECX,ra);
	
		if (sz==1)
			x86e->Emit(op_call,x86_ptr_imm(ReadMem8));
		else if (sz==2)
			x86e->Emit(op_call,x86_ptr_imm(ReadMem16));
		else if (sz==4)
			x86e->Emit(op_call,x86_ptr_imm(ReadMem32));

		if (ro!=EAX)
			x86e->Emit(op_mov32,ro,EAX);
	}
	else
	{
		if (ra!=ECX)
			x86e->Emit(op_mov32,ECX,ra);
		if (ro!=EDX)
			x86e->Emit(op_mov32,EDX,ro);

		if (sz==1)
			x86e->Emit(op_call,x86_ptr_imm(WriteMem8));
		else if (sz==2)
			x86e->Emit(op_call,x86_ptr_imm(WriteMem16));
		else if (sz==4)
			x86e->Emit(op_call,x86_ptr_imm(WriteMem32));
	}
#if NOT_DEFINED
	u32 p_RWF_table=0;
	x86_Label* direct=x86e->CreateLabel(false);
	x86_Label* rw_end=x86e->CreateLabel(false);

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
	x86e->Emit(op_mov32,EAX,ra);
	////shr 14 + and vs shr16 + mov eax,[_vmem_MemInfo+eax*4];
	////after testing , shr16+mov complex is faster , both on amd (a64 x2) and intel (northwood)

	////get upper 16 bits
	//shr eax,16;
	x86e->Emit(op_shr32 ,EAX,16);
	
	//read olny
	if (rw==0)
	{
		//mov edx,ecx;//this is done here , among w/ the and , it should be possible to fully execute it on paraler (no depency)
		x86e->Emit(op_mov32,EDX,ra);
	}

	//read mem info
	//mov eax,[_vmem_MemInfo+eax*4];
	//8B 04 85 base_address
	x86e->Emit(op_mov32, EAX,x86_mrm::create(EAX,sib_scale_4,_vmem_MemInfo));
	/*
	x86e->write8(0x8B);
	x86e->write8(0x04);
	x86e->write8(0x85);
	x86e->write32((u32)&_vmem_MemInfo[0]);
	*/

	//olny on read
	if (rw==0)
	{
		//read is gona stall , even if 1 cycle ;
		//and edx,0xFFFF;//lower 16b of address
		x86e->Emit(op_and32 ,EDX,0xFFFF);
	}

	//ra may be another reg :)
	if (ra!=ECX)
		x86e->Emit(op_mov32,ECX,ra);

	//test eax,0xFFFF0000;
	x86e->Emit(op_test32 ,EAX,0xFFFF0000);

	if (rw==1)
	{
		if (ro!=EDX)
			x86e->Emit(op_mov32,EDX,ro);
	}

	//jnz direct;
	x86e->Emit(op_jnz , direct);
	
	////get function pointer
	//mov eax , [_vmem_RF8+eax];
	//0040FF23 8B 80 18 28 5A 02 mov         eax,dword ptr _vmem_WF32 (25A2818h)[eax] 
	/*x86e->write8(0x8B);
	x86e->write8(0x80);
	x86e->write32(p_RWF_table);*/

	// -> x86e->Emit(op_mov32,EAX,x86_mrm::create(mod_RI_disp,EAX,sib_scale_1,NO_REG,_vmem_RF8));
	//jmp eax;
	//x86e->CALL32R(EAX);
	x86e->Emit(op_call32,x86_mrm::create(EAX,_vmem_RF8));

	if (rw==0)
	{
		if (ro!=EAX)
			x86e->Emit(op_mov32,ro,EAX);
	}
	x86e->Emit(op_jmp,rw_end);

//direct:
	x86e->MarkLabel(direct);
	//write olny
	if (rw==1)
	{
		//and edx,0xFFFF;//lower 16b of address
		x86e->Emit(op_and32 ,ECX,0xFFFF);
		//write to [eax+ecx]
		if (sz==1)
		{	//, dl
			x86e->Emit(op_mov8,x86_mrm::create(EAX,ECX),DL);
			/*
			x86e->write8(0x88);
			x86e->write8(0x14);
			x86e->write8(0x08);
			*/
		}
		else if (sz==2)
		{	//,dx
			x86e->Emit(op_mov16,x86_mrm::create(EAX,ECX),DX);
			/*
			x86e->write8(0x66);
			x86e->write8(0x89);
			x86e->write8(0x14);
			x86e->write8(0x08);
			*/
		}
		else if (sz==4)
		{	//,edx
			x86e->Emit(op_mov32,x86_mrm::create(EAX,ECX),EDX);
			/*
			x86e->write8(0x89);
			x86e->write8(0x14);
			x86e->write8(0x08);
			*/
		}
	}
	else
	{
		//mov eax,[eax+edx];	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
		//0040FE43 8B 04 10         mov         eax,dword ptr [eax+edx] 
		/*
		x86e->write8(0x8B);
		x86e->write8(0x04);
		x86e->write8(0x10);
		*/
		x86e->Emit(op_mov32,ro,x86_mrm::create(EAX,EDX));
		//if (ro!=EAX)
		//	x86e->Emit(op_mov32,ro,EAX);
	}
	//ret;
	x86e->MarkLabel(rw_end);
#endif
}

void emit_vmem_op_compat_const(x86_block* x86e,u32 ra,
							   x86_gpr_reg ro,x86_sse_reg ro_sse,bool sse,
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
					x86e->Emit(op_mov32,EDX,ro);
			}
		}

		x86e->Emit(op_mov32,ECX,ra);

		u32 entry=((u32)t)>>2;

		x86e->Emit(op_call , x86_ptr_imm(((u32**)p_RWF_table)[entry]));
		if (rw==0)
		{
			if (sz==1)
			{
				x86e->Emit(op_movsx8to32, ro,EAX);
			}
			else if (sz==2)
			{
				x86e->Emit(op_movsx16to32, ro,EAX);
			}
			else if (sz==4)
			{
				if (ro!=EAX)
					x86e->Emit(op_mov32,ro,EAX);
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
					x86e->Emit(op_mov32,EDX,ro);
				x86e->Emit(op_mov8 ,(u8*)paddr,EDX);
			}
			else if (sz==2)
			{	//,dx
				x86e->Emit(op_mov16 ,(u16*)paddr,ro);
			}
			else if (sz==4)
			{	//,edx
				if (sse)
				{
					x86e->Emit(op_movss,(u32*)paddr,ro_sse);
				}
				else
					x86e->Emit(op_mov32,(u32*)paddr,ro);
			}
		}
		else
		{
			void* paddr=&((u8*)t)[lower];
			if (sz==1)
			{
				x86e->Emit(op_movsx8to32, ro,(u8*)paddr);
			}
			else if (sz==2)
			{
				x86e->Emit(op_movsx16to32, ro,(u16*)paddr);
			}
			else if (sz==4)
			{
				if (sse)
				{
					x86e->Emit(op_movss,ro_sse,(u32*)paddr);
				}
				else
					x86e->Emit(op_mov32,ro,(u32*)paddr);
			}
		}
	}
}

//sz : 1,2 -> sign extended , 4 fully loaded.SSE valid olny for sz=4
//reg_addr : either ECX , either allocated
void emit_vmem_read(x86_reg reg_addr,u8 reg_out,u32 sz)
{
	bool sse=IsInFReg(reg_out);
	if (sse)
		verify(sz==4);

	x86_ptr p_RF_table(0);
	x86_Label* direct=x86e->CreateLabel(false,8);
	x86_Label* end=x86e->CreateLabel(false,8);

	if (sz==1)
		p_RF_table=&_vmem_RF8[0];
	else if (sz==2)
		p_RF_table=&_vmem_RF16[0];
	else if (sz==4)
		p_RF_table=&_vmem_RF32[0];

	//x86e->Emit(op_int3);
	//copy address
	//this is done here , among w/ the and , it should be possible to fully execute it on paraler (no depency)
	x86e->Emit(op_mov32,EDX,reg_addr);
	x86e->Emit(op_mov32,EAX,reg_addr);
	//lower 16b of address
	x86e->Emit(op_and32,EDX,0xFFFF);
	//x86e->Emit(op_movzx16to32,EDX,EDX);
	//get upper 16 bits
	x86e->Emit(op_shr32,EAX,16);
	//read mem info
	//mov eax,[_vmem_MemInfo+eax*4];
	x86e->Emit(op_mov32,EAX,x86_mrm::create(EAX,sib_scale_4,_vmem_MemInfo));

	//test eax,0xFFFF0000;
	x86e->Emit(op_test32,EAX,0xFFFF0000);
	//jnz direct;
	x86e->Emit(op_jnz,direct);
	//--other read---
	if (reg_addr!=ECX)
		x86e->Emit(op_mov32,ECX,reg_addr);
	//Get function pointer and call it
	x86e->Emit(op_call32,x86_mrm::create(EAX,p_RF_table));

	//save reg
	if (!sse)
	{
		x86_reg writereg= LoadReg_nodata(EAX,reg_out);
		if (sz==1)
		{
			x86e->Emit(op_movsx8to32, writereg,EAX);
		}
		else if (sz==2)
		{
			x86e->Emit(op_movsx16to32, writereg,EAX);
		}
		else
		{
			x86e->Emit(op_mov32, writereg,EAX);
		}
		SaveReg(reg_out,writereg);
	}
	else
	{	
		fra->SaveRegisterGPR(reg_out,EAX);
	}

	x86e->Emit(op_jmp,end);
//direct:
	x86e->MarkLabel(direct);
//	mov eax,[eax+edx];	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
	if (!sse)
	{
		x86_reg writereg= LoadReg_nodata(EAX,reg_out);
		if (sz==1)
		{
			x86e->Emit(op_movsx8to32, writereg,x86_mrm::create(EAX,EDX));
		}
		else if (sz==2)
		{
			x86e->Emit(op_movsx16to32, writereg,x86_mrm::create(EAX,EDX));
		}
		else
		{
			x86e->Emit(op_mov32, writereg,x86_mrm::create(EAX,EDX));
		}
		SaveReg(reg_out,writereg);
	}
	else
	{
		x86_reg writereg= fra->GetRegister(XMM0,reg_out,RA_NODATA);
		
		x86e->Emit(op_movss, writereg,x86_mrm::create(EAX,EDX));
		
		fra->SaveRegister(reg_out,writereg);
	}
	x86e->MarkLabel(end);
}
//SSE valid olny for sz=4
//reg_addr : either ECX , either allocated
void emit_vmem_write(x86_reg reg_addr,u8 reg_data,u32 sz)
{
	bool sse=IsInFReg(reg_data);
	if (sse)
	{
		verify(sz==4);
		if (fra->IsRegAllocated(reg_data))
			fra->GetRegister(XMM0,reg_data,RA_DEFAULT);
	}
	else
	{
		if (ira->IsRegAllocated(reg_data))
			ira->GetRegister(EAX,reg_data,RA_DEFAULT);
	}

	x86_ptr p_WF_table(0);
	x86_Label* direct=x86e->CreateLabel(false,8);
	x86_Label* end=x86e->CreateLabel(false,8);

	if (sz==1)
		p_WF_table=&_vmem_WF8[0];
	else if (sz==2)
		p_WF_table=&_vmem_WF16[0];
	else if (sz==4)
		p_WF_table=&_vmem_WF32[0];

	//x86e->Emit(op_int3);
	//copy address
	//this is done here , among w/ the and , it should be possible to fully execute it on paraler (no depency)
	x86e->Emit(op_mov32,EDX,reg_addr);
	x86e->Emit(op_mov32,EAX,reg_addr);
	//lower 16b of address
	x86e->Emit(op_and32,EDX,0xFFFF);
	//x86e->Emit(op_movzx16to32,EDX,EDX);
	//get upper 16 bits
	x86e->Emit(op_shr32,EAX,16);
	//read mem info
	//mov eax,[_vmem_MemInfo+eax*4];
	x86e->Emit(op_mov32,EAX,x86_mrm::create(EAX,sib_scale_4,_vmem_MemInfo));

	//test eax,0xFFFF0000;
	x86e->Emit(op_test32,EAX,0xFFFF0000);
	//jnz direct;
	x86e->Emit(op_jnz,direct);
	//--other read---
	if (reg_addr!=ECX)
		x86e->Emit(op_mov32,ECX,reg_addr);

	//load reg
	if (!sse)
	{
		LoadReg_force(EDX,reg_data);
	}
	else
	{	
		fra->LoadRegisterGPR(EDX,reg_data);
	}

	//Get function pointer and call it
	x86e->Emit(op_call32,x86_mrm::create(EAX,p_WF_table));


	x86e->Emit(op_jmp,end);
//direct:
	x86e->MarkLabel(direct);
//	mov [eax+edx],reg;	//note : upper bits dont matter , so i do 32b read here ;) (to get read of partial register stalls)
	//x86e->Emit(op_int3);
	if (!sse)
	{
		
		if (sz==1)
		{
			x86_reg readreg= LoadReg_force(ECX,reg_data);
			x86e->Emit(op_mov8, x86_mrm::create(EAX,EDX),readreg);
		}
		else if (sz==2)
		{
			x86_reg readreg= LoadReg(ECX,reg_data);
			x86e->Emit(op_mov16, x86_mrm::create(EAX,EDX),readreg);
		}
		else
		{
			x86_reg readreg= LoadReg(ECX,reg_data);
			x86e->Emit(op_mov32, x86_mrm::create(EAX,EDX),readreg);
		}
	}
	else
	{
		x86_reg readreg= fra->GetRegister(XMM0,reg_data,RA_DEFAULT);
		
		x86e->Emit(op_movss, x86_mrm::create(EAX,EDX),readreg);
	}
	x86e->MarkLabel(end);
}