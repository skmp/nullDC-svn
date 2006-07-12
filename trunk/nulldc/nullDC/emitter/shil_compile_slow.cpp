#include "shil_compile_slow.h"
#include <assert.h>
#include "emitter.h"

#include "dc\sh4\shil\shil_ce.h"
#include "dc\sh4\sh4_registers.h"
#include "dc\sh4\rec_v1\rec_v1_blockmanager.h"
#include "dc\sh4\rec_v1\nullprof.h"
#include "dc\sh4\sh4_opcode_list.h"
#include "dc\mem\sh4_mem.h"
#include "regalloc\x86_sseregalloc.h"

FloatRegAllocator* falloc;
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
typedef void __fastcall shil_compileFP(shil_opcode* op,rec_v1_BasicBlock* block);

bool inited=false;

int fallbacks=0;
int native=0;
u32 T_jcond_value;
u32 T_bit_value;
bool T_Edited;

u32 reg_pc_temp_value;
rec_v1_BasicBlock* rec_v1_pCurrentBlock;
u32 IsRamAddr[0x100];
int block_count=0;

#ifdef X86
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

void __fastcall dyna_profile_block_exit(rec_v1_BasicBlock* bb)
{
	__asm
	{
		rdtsc;
		mov dyn_now_block.l,eax;
		mov dyn_now_block.h,edx;
	}

	u64 t=dyn_now_block.v-dyn_last_block.v;
	//rec_native_cycles+=t;
	bb->profile_time+=t;
	bb->profile_calls++;
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
INLINE u32* GetRegPtr(u32 reg)
{
	if (reg==Sh4RegType::reg_pc_temp)
		return &reg_pc_temp_value;

	u32* rv=Sh4_int_GetRegisterPtr((Sh4RegType)reg);
	assert(rv!=0);
	return rv;
}
INLINE u32* GetRegPtr(Sh4RegType reg)
{
	return GetRegPtr((u8)reg);
}

//REGISTER ALLOCATION

struct RegAllocInfo
{
	x86IntRegType x86reg;
	bool InReg;
	bool Dirty;
};
 RegAllocInfo r_alloced[16];
//compile a basicblock

struct sort_temp
{
	int cnt;
	int reg;
};

//ebx, ebp, esi, and edi are preserved



x86IntRegType reg_to_alloc[4]=
{
	EBX,
	EBP,
	ESI,
	EDI
};

//xmm0 is reserved for math/temp
x86SSERegType reg_to_alloc_xmm[7]=
{
	XMM1,
	XMM2,
	XMM3,
	XMM4,
	XMM5,
	XMM6,
	XMM7,
};

//implement register allocators on a class , so we can swap em around?
//methods needed
//
//DoAllocation		: do allocation on the block
//BeforeEmit		: generate any code needed before the main emittion begins (other register allocators may have emited code tho)
//BeforeTrail		: generate any code needed after the main emittion has ended (other register allocators may emit code after that tho)
//AfterTrail		: generate code after the native block end (after the ret) , can be used to emit helper functions (other register allocators may emit code after that tho)
//IsRegAllocated	: *couh* yea .. :P
//GetRegister		: Get the register , needs flag if it's read or write. Carefull w/ register state , we may need to implement state push/pop
//PushRegister		: push register to stack (if allocated)
//PopRegister		: pop register from stack (if allocated)
//FlushRegister		: write reg to reg location , and dealloc it
//WriteBackRegister	: write reg to reg location
//ReloadRegister	: read reg from reg location , discard old result

void bubble_sort(sort_temp numbers[] , int array_size)
{
  int i, j;
  sort_temp temp;
  for (i = (array_size - 1); i >= 0; i--)
  {
    for (j = 1; j <= i; j++)
	{
		if (numbers[j-1].cnt < numbers[j].cnt)
		{
			temp = numbers[j-1];
			numbers[j-1] = numbers[j];
			numbers[j] = temp;
		}
	}
  }
}
INLINE bool IsRegCached(u32 reg)
{
	if (reg<=r15)
	{
		if (REG_ALLOC_X86)
		{
			return r_alloced[reg].x86reg!=GPR_Error;
		}
		else
		{
			return false;
		}
	}
	else
		return false;
}
INLINE void FlushRegCache_reg(u32 reg)
{
	if (IsRegCached(reg) && r_alloced[reg].InReg)
	{
		if (r_alloced[reg].Dirty)
			x86e->MOV32RtoM(GetRegPtr(reg),r_alloced[reg].x86reg);
		/*else
			printf("red r%d not dirty ;)\n",reg);*/
		r_alloced[reg].InReg=false;
		r_alloced[reg].Dirty=false;
	}
}

INLINE void MarkDirty(u32 reg)
{
	if (IsRegCached(reg))
	{
		r_alloced[reg].Dirty=true;
	}
}
INLINE x86IntRegType LoadRegCache_reg(u32 reg)
{
	if (IsRegCached(reg))
	{
		if (r_alloced[reg].InReg==false )
		{
			r_alloced[reg].InReg=true;
			x86e->MOV32MtoR(r_alloced[reg].x86reg,GetRegPtr(reg));
		}
		return r_alloced[reg].x86reg;
	}

	return GPR_Error;
}

INLINE x86IntRegType LoadRegCache_reg_nodata(u32 reg)
{
	if (IsRegCached(reg))
	{
		if (r_alloced[reg].InReg==false )
		{
			r_alloced[reg].InReg=true;
		}
		return r_alloced[reg].x86reg;
	}

	return GPR_Error;
}
void AllocateRegisters(rec_v1_BasicBlock* block)
{
	if(REG_ALLOC_X86)
	{
		sort_temp used[16];
		for (int i=0;i<16;i++)
		{
			used[i].cnt=0;
			used[i].reg=r0+i;
			r_alloced[i].x86reg=GPR_Error;
			r_alloced[i].InReg=false;
			r_alloced[i].Dirty=false;
		}

		u32 op_count=block->ilst.op_count;
		shil_opcode* curop;

		for (u32 j=0;j<op_count;j++)
		{
			curop=&block->ilst.opcodes[j];
			for (int i = 0;i<16;i++)
			{
				//both reads and writes , give it one more ;P
				if ( curop->UpdatesReg((Sh4RegType) (r0+i)) )
					used[i].cnt+=12;

				if (curop->ReadsReg((Sh4RegType) (r0+i)))
					used[i].cnt+=6;

				if (curop->WritesReg((Sh4RegType) (r0+i)))
					used[i].cnt+=9;
			}
		}

		bubble_sort(used,16);

		for (u32 i=0;i<REG_ALLOC_COUNT;i++)
		{
			if (used[i].cnt<5)
				break;
			r_alloced[used[i].reg].x86reg=reg_to_alloc[i];
		}
	}
}
void LoadRegisters()
{
	if(REG_ALLOC_X86)
	{
		for (int i=0;i<16;i++)
		{
			if (IsRegCached(i))
			{
				LoadRegCache_reg(i);
			}
		}
	}
}
//more helpers
INLINE x86IntRegType LoadReg_force(x86IntRegType to,u8 reg)
{
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOV32MtoR(to,GetRegPtr(reg));
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg(reg);
			if (!(to==x86reg))
				x86e->MOV32RtoR(to,x86reg);
		}
	}
	else
	{
		x86e->MOV32MtoR(to,GetRegPtr(reg));
	}

	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		if (reg==reg_sr)
		{
			x86e->AND32ItoR(to,(u32)~1);
			x86e->OR32MtoR(to,&T_bit_value);
			x86e->MOV32RtoM(GetRegPtr(reg_sr),to);//save it back to be sure :P
			T_Edited=false;
		}
	}
	return to;
}

INLINE x86IntRegType LoadReg(x86IntRegType to,u8 reg)
{
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOV32MtoR(to,GetRegPtr(reg));
			//return to;
		}
		else
		{
			to= LoadRegCache_reg(reg);
		}
	}
	else
	{
		x86e->MOV32MtoR(to,GetRegPtr(reg));
		//return to;
	}
	
	if(REG_ALLOC_T_BIT_SEPERATE)
	{
		if (reg==reg_sr)
		{
			x86e->AND32ItoR(to,(u32)~1);
			x86e->OR32MtoR(to,&T_bit_value);
			x86e->MOV32RtoM(GetRegPtr(reg_sr),to);//save it back to be sure :P
			T_Edited=false;
		}
	}
	return to;
}

INLINE x86IntRegType LoadReg_nodata(x86IntRegType to,u8 reg)
{
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			//x86e->MOV32MtoR(to,GetRegPtr(reg)); -> do nothin :P
			return to;
		}
		else
		{
			return LoadRegCache_reg_nodata(reg);
		}
	}
	else
	{
		return to;
	}
}


INLINE void SaveReg(u8 reg,x86IntRegType from)
{
	MarkDirty(reg);
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOV32RtoM(GetRegPtr(reg),from);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			if (x86reg!=from)
				x86e->MOV32RtoR(x86reg,from);
		}
	}
	else
	{
		x86e->MOV32RtoM(GetRegPtr(reg),from);
	}

	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		if (reg==reg_sr)
		{
			T_Edited=false;
		}
	}
}

INLINE void SaveReg(u8 reg,u32 from)
{
	MarkDirty(reg);
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOV32ItoM(GetRegPtr(reg),from);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			if (from==0)
				x86e->XOR32RtoR(x86reg,x86reg);
			else if (from ==0xFFFFFFFF)
				x86e->MOV32ItoR(x86reg,from);//xor , dec ?
			else
				x86e->MOV32ItoR(x86reg,from);
		}
	}
	else
	{
		x86e->MOV32ItoM(GetRegPtr(reg),from);
	}

	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		if (reg==reg_sr)
		{
			T_Edited=false;
		}
	}
}

INLINE void SaveReg(u8 reg,u32* from)
{
	MarkDirty(reg);
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOV32MtoR(ECX,from);
			x86e->MOV32RtoM(GetRegPtr(reg),ECX);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			x86e->MOV32MtoR(x86reg,from);
		}
	}
	else
	{
		x86e->MOV32MtoR(ECX,from);
		x86e->MOV32RtoM(GetRegPtr(reg),ECX);
	}

	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		if (reg==reg_sr)
		{
			T_Edited=false;
		}
	}
}

INLINE void SaveReg(u8 reg,s16* from)
{
	MarkDirty(reg);
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOVSX32M16toR(ECX,(u16*)from);
			x86e->MOV32RtoM(GetRegPtr(reg),ECX);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			x86e->MOVSX32M16toR(x86reg,(u16*)from);
		}
	}
	else
	{
		x86e->MOVSX32M16toR(ECX,(u16*)from);
		x86e->MOV32RtoM(GetRegPtr(reg),ECX);
	}
}

INLINE void SaveReg(u8 reg,s8* from)
{
	MarkDirty(reg);
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOVSX32M8toR(ECX,(u8*)from);
			x86e->MOV32RtoM(GetRegPtr(reg),ECX);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			x86e->MOVSX32M8toR(x86reg,(u8*)from);
		}
	}
	else
	{
		x86e->MOVSX32M8toR(ECX,(u8*)from);
		x86e->MOV32RtoM(GetRegPtr(reg),ECX);
	}
}

INLINE void FlushRegCache()
{
	if(REG_ALLOC_X86)
	{
		for (int i=0;i<16;i++)
		{
			FlushRegCache_reg(i);
		}
	}
	
	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		if (T_Edited)
		{
			//save T
			LoadReg_force(EAX,reg_sr);			//ecx=sr(~1)|T
			x86e->AND32ItoR(EAX,(u32)~1);
			x86e->OR32MtoR(EAX,&T_bit_value);
			SaveReg(reg_sr,EAX);
			T_Edited=false;
		}
	}
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

//intel sugest not to use the ItoM forms for some reason .. speed diference isnt big .. < 1%
//Macro :
#define OP_MoRtR_ItR(_MtR_,_RtR_,_ItR_,_ItM_,_RtM_)	assert(FLAG_32==(op->flags & 3));\
	assert(0==(op->flags & (FLAG_IMM2)));\
	assert(op->flags & FLAG_REG1);\
	if (op->flags & FLAG_IMM1)\
	{\
		assert(0==(op->flags & FLAG_REG2));\
		if (IsRegCached(op->reg1))\
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
		if (IsRegCached(op->reg1))\
		{\
			x86IntRegType r1 = LoadReg(EAX,op->reg1);\
			assert(r1!=EAX);\
			if (IsRegCached(op->reg2))\
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
	if (IsRegCached(op->reg1))\
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
	if (IsRegCached(op->reg1))\
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
		OP_RegToReg_simple(MOV32);
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
void __fastcall shil_compile_movex(shil_opcode* op,rec_v1_BasicBlock* block)
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

			if (IsRegCached(op->reg2))
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

			if (IsRegCached(op->reg2))
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
void __fastcall shil_compile_shil_ifb(shil_opcode* op,rec_v1_BasicBlock* block)
{

	//if opcode needs pc , save it
	if (OpTyp[op->imm1] !=Normal)
		SaveReg(reg_pc,op->imm2);
	
	FlushRegCache();

	x86e->MOV32ItoR(ECX,op->imm1);
	x86e->CALLFunc(OpPtr[op->imm1]);

	//x86e->CALLFunc(profile_ifb_call);

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
	MarkDirty(op->reg1);
}

void __fastcall shil_compile_shl(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_ItMoR(SHL32ItoM,SHL32ItoR,(u8)op->imm1);
	MarkDirty(op->reg1);
}

void __fastcall shil_compile_shr(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_ItMoR(SHR32ItoM,SHR32ItoR,(u8)op->imm1);
	MarkDirty(op->reg1);
}

void __fastcall shil_compile_sar(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_ItMoR(SAR32ItoM,SAR32ItoR,(u8)op->imm1);
	MarkDirty(op->reg1);
}

//rotates
void __fastcall shil_compile_rcl(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtMoR_noimm(RCL321toM,RCL321toR);
	MarkDirty(op->reg1);
}
void __fastcall shil_compile_rcr(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtR_noimm(RCR321toR);
	MarkDirty(op->reg1);
}
void __fastcall shil_compile_ror(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtR_noimm(ROR321toR);
	MarkDirty(op->reg1);
}
void __fastcall shil_compile_rol(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtR_noimm(ROL321toR);
	MarkDirty(op->reg1);
}
//neg
void __fastcall shil_compile_neg(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtR_noimm(NEG32R);
	MarkDirty(op->reg1);
}
//not
void __fastcall shil_compile_not(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtR_noimm(NOT32R);
	MarkDirty(op->reg1);
}
//or xor and
void __fastcall shil_compile_xor(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_RegToReg_simple(XOR32);
	MarkDirty(op->reg1);
}
void __fastcall shil_compile_or(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_RegToReg_simple(OR32);
	MarkDirty(op->reg1);
}
void __fastcall shil_compile_and(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_RegToReg_simple(AND32);
	MarkDirty(op->reg1);
}
//read-write
void readwrteparams(shil_opcode* op)
{
	assert(0==(op->flags & FLAG_IMM2));
	assert(op->flags & FLAG_REG1);

	
	if (!(op->flags & (FLAG_R0|FLAG_GBR)))
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
			if (IsRegCached(op->reg2))
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
	}
}

u32 const_hit=0;
u32 non_const_hit=0;

void emit_vmem_op_compat(emitter<>* x86e,x86IntRegType ra,
					  x86IntRegType ro,
					  u32 sz,u32 rw);

void emit_vmem_op_compat_const(emitter<>* x86e,u32 ra,
					x86IntRegType ro,
					  u32 sz,u32 rw);
u32 m_unpack_sz[3]={1,2,4};
void __fastcall shil_compile_readm(shil_opcode* op,rec_v1_BasicBlock* block)
{
	u32 size=op->flags&3;

	
	if (INLINE_MEM_READ_CONST)
	{
		//if constant read , and on ram area , make it a direct mem access
		//_watch_ mmu
		if (!(op->flags & (FLAG_R0|FLAG_GBR)))
		{//[reg2+imm] form
			assert(op->flags & FLAG_IMM1);

			if (!(op->flags & FLAG_REG2))
			{	//[imm1] form
				x86IntRegType rall=LoadReg(EDX,op->reg1);
				emit_vmem_op_compat_const(x86e,op->imm1,rall,m_unpack_sz[size],0);
				SaveReg(op->reg1,rall);
				MarkDirty(op->reg1);
				return;
			}
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

	SaveReg(op->reg1,EAX);//save return value
	MarkDirty(op->reg1);
}
void __fastcall shil_compile_writem(shil_opcode* op,rec_v1_BasicBlock* block)
{
	
	u32 size=op->flags&3;

	//if constant read , and on ram area , make it a direct mem access
	//_watch_ mmu
	if (!(op->flags & (FLAG_R0|FLAG_GBR)))
	{//[reg2+imm] form
		assert(op->flags & FLAG_IMM1);

		if (!(op->flags & FLAG_REG2))
		{	//[imm1] form
			x86IntRegType rall=LoadReg(EDX,op->reg1);
			emit_vmem_op_compat_const(x86e,op->imm1,rall,m_unpack_sz[size],1);
			return;
		}
	}

	readwrteparams(op);

	//ECX is address

	//so it's sure loaded (if from reg cache)
	x86IntRegType r1=LoadReg(EDX,op->reg1);
	
	/*u8* inline_label=0;

	/*if (INLINE_MEM_WRITE)
	{	//try to inline all mem reads at runtime :P
		//eax = ((ecx>>24) & 0xFF)<<2

		//mov eax,ecx
		x86e->MOV32RtoR(EAX,ECX);
		//shr eax,24
		//shl eax,2
		x86e->SHR32ItoR(EAX,24-2);
		x86e->AND32ItoR(EAX,0xFF<<2);

		//add eax , imm	;//should realy use lea/mov eax,[eax+xx]
		x86e->ADD32ItoR(EAX,(u32)IsRamAddr);
		//mov eax,[eax]
		x86e->MOV32RmtoR(EAX,EAX);
		//test eax,eax
		x86e->TEST32RtoR(EAX,EAX);
		//jz inline;//if !0 , then do normal write
		inline_label = x86e->JZ8(0);
	}*/
	/*if (size==FLAG_8)
	{	//maby zx ?
		if (r1!=EDX)
			x86e->MOV32RtoR(EDX,r1);
		x86e->CALLFunc(WriteMem8);
	}
	else if (size==FLAG_16)
	{	//maby zx ?
		if (r1!=EDX)
			x86e->MOV32RtoR(EDX,r1);
		x86e->CALLFunc(WriteMem16);
	}
	else if (size==FLAG_32)
	{
		if (r1!=EDX)
			x86e->MOV32RtoR(EDX,r1);
		x86e->CALLFunc(WriteMem32);
	}
	else
		printf("WriteMem error\n");*/
	emit_vmem_op_compat(x86e,ECX,r1,m_unpack_sz[size],1);
/*
	
	if (INLINE_MEM_WRITE)
	{
		//jmp normal;
		u8* normal_label = x86e->JMP8(0);
		//
		//inline:  //inlined ram write
		x86e->x86SetJ8(inline_label);

		//and ecx , ram_mask
		x86e->AND32ItoR(ECX,RAM_MASK);


		//no more block tests
		
		//if needed
		//if (r1==EDX)
		//	x86e->PUSH32R(r1);


		//call rec_v1_BlockTest
		//
		//rec_v1_CompileBlockTest(x86e,ECX,EAX);

		//if needed
		//if (r1==EDX)
		//	x86e->POP32R(r1);

		//add ecx, ram_base
		x86e->ADD32ItoR(ECX,(u32)(&mem_b[0]));
		//mov/sx eax,[ecx]
		if (size==0)
		{
			if (r1!=EDX)
				x86e->MOV32RtoR(EDX,r1);
			x86e->MOV8RtoRm(ECX,EDX);
		}
		else if (size==1)
		{
			x86e->MOV16RtoRm(ECX,r1);
		}
		else if (size==2)
		{
			x86e->MOV32RtoRm(ECX,r1);
		}
		else
			printf("ReadMem error\n");
		//normal:
		x86e->x86SetJ8(normal_label);
	}*/

}

//save-loadT
void __fastcall shil_compile_SaveT(shil_opcode* op,rec_v1_BasicBlock* block)
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
void __fastcall shil_compile_LoadT(shil_opcode* op,rec_v1_BasicBlock* block)
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

void __fastcall shil_compile_cmp(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(FLAG_32==(op->flags & 3));
	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & (FLAG_REG2|FLAG_IMM2)));
		if (IsRegCached(op->reg1))
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
		if (IsRegCached(op->reg2))
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
void __fastcall shil_compile_test(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(FLAG_32==(op->flags & 3));
	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & (FLAG_REG2|FLAG_IMM2)));
		if (IsRegCached(op->reg1))
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
		if (IsRegCached(op->reg2))
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
void __fastcall shil_compile_add(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_RegToReg_simple(ADD32);
	MarkDirty(op->reg1);
}
void __fastcall shil_compile_adc(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_RegToReg_simple(ADC32);
	MarkDirty(op->reg1);
}
void __fastcall shil_compile_sub(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_RegToReg_simple(SUB32);
	MarkDirty(op->reg1);
}

//**
void __fastcall shil_compile_jcond(shil_opcode* op,rec_v1_BasicBlock* block)
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
void __fastcall shil_compile_jmp(shil_opcode* op,rec_v1_BasicBlock* block)
{
	printf("jmp ... heh not implemented\n");
}

void load_with_se16(x86IntRegType to,u8 from)
{
	if (IsRegCached(from))
	{
		x86IntRegType r1=LoadReg(EAX,from);
		x86e->MOVSX32R16toR(to,r1);
	}
	else
		x86e->MOVSX32M16toR(to,(u16*)GetRegPtr(from));
}

void load_with_ze16(x86IntRegType to,u8 from)
{
	if (IsRegCached(from))
	{
		x86IntRegType r1=LoadReg(EAX,from);
		x86e->MOVZX32R16toR(to,r1);
	}
	else
		x86e->MOVZX32M16toR(to,(u16*)GetRegPtr(from));
}

void __fastcall shil_compile_mul(shil_opcode* op,rec_v1_BasicBlock* block)
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
		MarkDirty(reg_macl);
	}
	else
	{
		assert(sz==FLAG_64);

		FlushRegCache_reg(op->reg1);
		FlushRegCache_reg(op->reg2);

		x86e->MOV32MtoR(EAX,GetRegPtr(op->reg1));

		if (op->flags & FLAG_SX)
			x86e->IMUL32M(GetRegPtr(op->reg2));
		else
			x86e->MUL32M(GetRegPtr(op->reg2));

		SaveReg((u8)reg_macl,EAX);
		SaveReg((u8)reg_mach,EDX);
		MarkDirty(reg_macl);
		MarkDirty(reg_mach);
	}
}

#define SSE_ItoRoM(_ItR_,_ItM_,reg,flags,imm1,imm2) \
	if (falloc->IsRegAllocated(reg))\
{\
	x86SSERegType reg=falloc->GetRegister(reg,flags);\
	x86e->_ItR_(reg,imm1);\
}\
	else\
	{\
		x86e->_ItM_(GetRegPtr(reg),imm2);\
	}

//FPU !!! YESH
void __fastcall shil_compile_fneg(shil_opcode* op,rec_v1_BasicBlock* block)
{

	

	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64 ((Sh4RegType)op->reg1));
		//SSE_ItoRoM(SSE_XORPS_M128_to_XMM,XOR32ItoM,op->reg1,RALLOC_RW,ps_not_data,0x80000000);
		x86e->XOR32ItoM(GetRegPtr(op->reg1),0x80000000);
	}
	else
	{
		assert(sz==FLAG_64);
		assert(IsReg64((Sh4RegType)op->reg1));
		u32 reg=GetSingleFromDouble(op->reg1);
		x86e->XOR32ItoM(GetRegPtr(reg+1),0x80000000);
	}
}

void __fastcall shil_compile_fabs(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		x86e->AND32ItoM(GetRegPtr(op->reg1),0x7FFFFFFF);
		//SSE_ItoRoM(SSE_ANDPS_M128_to_XMM,AND32ItoM,op->reg1,RALLOC_RW,ps_and_data,0x7FFFFFFF);
	}
	else
	{
		assert(sz==FLAG_64);
		assert(IsReg64((Sh4RegType)op->reg1));
		u32 reg=GetSingleFromDouble(op->reg1);
		x86e->AND32ItoM(GetRegPtr(reg+1),0x7FFFFFFF);
	}
}

x86SSERegType LoadSSEReg(u8 reg,u8 mode)
{
	if (falloc->IsRegAllocated(reg))
	{
		return falloc->GetRegister(reg,mode);
	}
	else
	{
		x86e->SSE_MOVSS_M32_to_XMM(XMM0,GetRegPtr(reg));
		return XMM0;
	}
}

void SaveSSEReg(u8 reg,x86SSERegType from)
{
	if (falloc->IsRegAllocated(reg))
	{
		x86SSERegType ar= falloc->GetRegister(reg,RALLOC_W);
		if (ar!=from)
			x86e->SSE_MOVSS_XMM_to_XMM(ar,from);
	}
	else
	{
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(reg),from);
	}
}

void __fastcall shil_compile_fadd(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());
/*		
		x86SSERegType sse1=LoadSSEReg(reg1,mode);
		if(falloc->IsRegAllocated(reg2))
		{
			x86SSERegType sse2=LoadSSEReg(reg2,RALLOC_R);
			x86e->SSE_ADDSS_XMM_to_XMM(sse1,GetRegPtr(reg2));
		}
		else
		{
			x86e->SSE_ADDSS_M32_to_XMM(sse1,GetRegPtr(reg2));
		}
*/
		x86e->SSE_MOVSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));
		x86e->SSE_ADDSS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);
	}
	else
	{
		assert(false);
	}
}
void __fastcall shil_compile_fcmp(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());
		x86e->SSE_MOVSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));
		x86e->SSE_UCOMISS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fsub(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());
		
		x86e->SSE_MOVSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));
		x86e->SSE_SUBSS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fmul(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		x86e->SSE_MOVSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));
		x86e->SSE_MULSS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fdiv(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		assert(Ensure32());

		x86e->SSE_MOVSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));
		x86e->SSE_DIVSS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fmac(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64((Sh4RegType)op->reg1));
		//fr[n] += fr[0] * fr[m];
		assert(Ensure32());

		x86e->SSE_MOVSS_M32_to_XMM(XMM0,GetRegPtr(fr_0));		//xmm0=fr[0]
		x86e->SSE_MULSS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));	//xmm0*=fr[m]
		x86e->SSE_ADDSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));	//xmm0+=fr[n] 
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);	//fr[n]=xmm0
	}
	else
	{
		assert(false);
	}
}


void __fastcall shil_compile_ftrv(shil_opcode* op,rec_v1_BasicBlock* block)
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

void __fastcall shil_compile_fipr(shil_opcode* op,rec_v1_BasicBlock* block)
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
void __fastcall shil_compile_fsqrt(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());

		assert(!IsReg64((Sh4RegType)op->reg1));
		//RSQRT vs SQRTSS -- why rsqrt no workie ? :P
		x86e->SSE_SQRTSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);
	}
	else
	{
		assert(false);
	}
}
void __fastcall shil_compile_floatfpul(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());
		assert(!IsReg64((Sh4RegType)op->reg1));

		//TODO : This is not entietly correct , sh4 rounds too [need to set MXCSR]
		//GOTA UNFUCK THE x86 EMITTER
		x86e->SSE_CVTSI2SS_M32_To_XMM(XMM0,GetRegPtr(reg_fpul));
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);
		
	}
	else
	{
		assert(false);
	}
}
void __fastcall shil_compile_ftrc(shil_opcode* op,rec_v1_BasicBlock* block)
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
		x86e->SSE_CVTTSS2SI_M32_To_R32(EAX,GetRegPtr(op->reg1));
		//fpul=EAX
		SaveReg(reg_fpul,EAX);
	}
	else
	{
		assert(false);
	}
}

#define pi (3.14159265f)

__declspec(align(32)) float mm_1[4]={1.0f,1.0f,1.0f,1.0f};
__declspec(align(32)) float fsca_fpul_adj[4]={((2*pi)/65536.0f),((2*pi)/65536.0f),((2*pi)/65536.0f),((2*pi)/65536.0f)};

void __fastcall shil_compile_fsca(shil_opcode* op,rec_v1_BasicBlock* block)
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
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_fsrra(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(Ensure32());
		assert(!IsReg64((Sh4RegType)op->reg1));
		//maby need to calculate 1/sqrt manualy ? -> yes , it seems rcp is not as accurate as needed :)
		x86e->SSE_SQRTSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));	//XMM0=sqrt
		x86e->SSE_MOVSS_M32_to_XMM(XMM1,(u32*)mm_1);			//XMM1=1
		x86e->SSE_DIVSS_XMM_to_XMM(XMM1,XMM0);					//XMM1=1/sqrt
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM1);	//fr=XMM1
	}
	else
	{
		assert(false);
	}
}

void __fastcall shil_compile_div32(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM2)));

	u8 rQuotient=op->reg1;
	u8 rDivisor=op->reg2;
	u8 rDividend=(u8)op->imm1;


	x86IntRegType Quotient=LoadReg_force(EAX,rQuotient);



	x86IntRegType Dividend=LoadReg_force(EDX,rDividend);


	if (IsRegCached(rDivisor))
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

	if (IsRegCached(rDivisor))
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

	MarkDirty(rDividend);
	MarkDirty(rQuotient);
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

	for (int i=0;i<0x100;i++)
	{
		IsRamAddr[i]=IsOnRam(i<<24)?0:0xFFFFFFFF;
	}

	printf("lazy shil compiler stats : %d%% opcodes done\n",shil_nimp*100/shil_opcodes::shil_count);
	if(profiler_dll.Load("nullprof_server.dll"))
	{
		void* temp;
		if (temp=profiler_dll.GetProcAddress("InitProfiller"))
		{
			nullprof_enabled=true;
			printf("nullprof_server.dll found , enabling profiling\n"); 
			((InitProfillerFP*)temp)(&null_prof_pointers);
		}
	}

}
//Compile block and return pointer to it's code
void* __fastcall link_compile_inject_TF(rec_v1_BasicBlock* ptr)
{
	rec_v1_BasicBlock* target= rec_v1_FindOrRecompileCode(ptr->TF_next_addr);
	
	
	//if current block is Discared , we must not add any chain info , just jump to the new one :)
	if (ptr->Discarded==false)
	{
		//Add reference so we can undo the chain later
		target->AddRef(ptr);
		ptr->TF_block=target;
		ptr->pTF_next_addr=target->compiled->Code;
	}
	return target->compiled->Code;
}

void* __fastcall link_compile_inject_TT(rec_v1_BasicBlock* ptr)
{
	rec_v1_BasicBlock* target= rec_v1_FindOrRecompileCode(ptr->TT_next_addr);

	//if current block is Discared , we must not add any chain info , just jump to the new one :)
	if (ptr->Discarded==false)
	{
		//Add reference so we can undo the chain later
		target->AddRef(ptr);
		ptr->TT_block=target;
		ptr->pTT_next_addr=target->compiled->Code;
	}
	return target->compiled->Code;
} 


//call link_compile_inject_TF , and jump to code
void naked link_compile_inject_TF_stub(rec_v1_BasicBlock* ptr)
{
	__asm
	{
		call link_compile_inject_TF;
		jmp eax;
	}
}


void naked link_compile_inject_TT_stub(rec_v1_BasicBlock* ptr)
{
	__asm
	{
		call link_compile_inject_TT;
		jmp eax;
	}
}


extern u32 rec_cycles;

u32 call_ret_address=0xFFFFFFFF;//holds teh return address of the previus call ;)
rec_v1_BasicBlock* pcall_ret_address=0;//holds teh return address of the previus call ;)

void CBBs_BlockSuspended(rec_v1_BasicBlock* block)
{
	if (pcall_ret_address == block)
	{
		call_ret_address=0xFFFFFFFF;
		pcall_ret_address=0;
	}
}
void __fastcall CheckBlock(rec_v1_BasicBlock* block)
{
	if (block->Discarded)
	{
		printf("Called a discarded block\n");
		__asm int 3;
	}
}
void CompileBasicBlock_slow(rec_v1_BasicBlock* block)
{
	//CompileBasicBlock_slow_c(block);
	if (!inited)
	{
		Init();
		inited=true;
	}


	x86e=new emitter<>();

	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		T_Edited=false;
		T_bit_value=0;//just to be sure
	}

	block->compiled=new rec_v1_CompiledBlock();
	

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
			x86e->MOV32ItoR(ECX,(u32)block);
			x86e->MOV32ItoM(GetRegPtr(reg_pc),block->start);
			x86e->JMP(SuspendBlock);
			x86e->x86SetJ8(patch);
		}
	}

	//perform constan elimination as many times as needed :)
	//u32 num_itt=0;
	//while(shil_optimise_pass_ce(block) && num_itt<100)
	//	num_itt++;
	shil_optimise_pass_ce_driver(block);

	//x86e->MOV32ItoR(ECX,(u32)block);
	//x86e->CALLFunc(CheckBlock);
	
	s8* start_ptr;

	if (PROFILE_BLOCK_CYCLES){
		start_ptr=x86e->x86Ptr;
		x86e->CALLFunc(dyna_profile_block_enter);
	}

	AllocateRegisters(block);
	LoadRegisters();
	if (PROFILE_BLOCK_CYCLES==false){
		start_ptr=x86e->x86Ptr;
	}

	x86e->ADD32ItoM(&rec_cycles,block->cycles);
	//x86e->MOV32ItoM((u32*)&rec_v1_pCurrentBlock,(u32)block);

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

	FlushRegCache();//flush reg cache

	if (PROFILE_BLOCK_CYCLES){
			x86e->MOV32ItoR(ECX,(u32)(block));
			x86e->CALLFunc(dyna_profile_block_exit);
	}

	//end block acording to block type :)
	switch(block->flags.ExitType)
	{
	
	case BLOCK_EXITTYPE_DYNAMIC_CALL:
		if (RET_PREDICTION)
		{
			//mov guess,pr
			x86e->MOV32ItoM(&call_ret_address,block->TT_next_addr);
			//mov pguess,this
			x86e->MOV32ItoM((u32*)&pcall_ret_address,(u32)(block));
		}
	case BLOCK_EXITTYPE_DYNAMIC:
		{
//			x86e->MOV32ItoM((u32*)&pExitBlock,(u32)block);
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
				x86e->ADD32ItoR(EAX,offsetof(rec_v1_BasicBlock,pTT_next_addr));
				x86e->MOV32RmtoR(EAX,EAX);//get ptr to compiled block/link stub
				//jmp eax
				x86e->JMP32R(EAX);	//jump to it
				
			}
			else
			{
				//save exit block 
//				x86e->MOV32ItoM((u32*)&pExitBlock,(u32)block);
				x86e->RET();
			}
			break;
		}

	case BLOCK_EXITTYPE_COND_0:
	case BLOCK_EXITTYPE_COND_1:
		{
			//ok , handle COND_0/COND_1 here :)
			//mem address
			u32* TT_a=&block->TT_next_addr;
			u32* TF_a=&block->TF_next_addr;
			//functions
			u32* pTF_f=(u32*)&(block->pTF_next_addr);
			u32* pTT_f=(u32*)&(block->pTT_next_addr);
			
			if (block->flags.ExitType==BLOCK_EXITTYPE_COND_0)
			{
				TT_a=&block->TF_next_addr;
				TF_a=&block->TT_next_addr;
				pTF_f=(u32*)&(block->pTT_next_addr);
				pTT_f=(u32*)&(block->pTF_next_addr);
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
					x86e->MOV32ItoR(ECX,(u32)block);					//mov ecx , block
					x86e->MOV32MtoR(EAX,&T_jcond_value);
					x86e->TEST32ItoR(EAX,1);//test for T


					/*
					//link to next block :
					if (*TF_a==block->start)
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

					if (*TT_a==block->start)
					{
						//fast link (direct jmp to block start)
						if (x86e->CanJ8(start_ptr))
							x86e->JNE8(start_ptr);
						else
							x86e->JNE32(start_ptr);
					}
					else
					{
						if (*TF_a==block->start)
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
			x86e->MOV32ItoM(&call_ret_address,block->TT_next_addr);
			//mov pguess,this
			x86e->MOV32ItoM((u32*)&pcall_ret_address,(u32)(block));
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
			x86e->MOV32ItoM(GetRegPtr(reg_pc),block->TF_next_addr);
			x86e->RET();//return to caller to check for interrupts


			if (BF_LINKING)
			{
				//Link:
				//if we can execute more blocks
				x86e->x86SetJ8(Link);
				if (block->TF_next_addr==block->start)
				{
					//__asm int 03;
					printf("Fast Link possible\n");
				}

				//link to next block :
				x86e->MOV32ItoR(ECX,(u32)block);					//mov ecx , block
				x86e->MOV32MtoR(EAX,(u32*)&(block->pTF_next_addr));	//mov eax , [pTF_next_addr]
				x86e->JMP32R(EAX);									//jmp eax
			}
			break;
		}
	}

	x86e->GenCode();//heh


	block->compiled->Code=(rec_v1_BasicBlockEP*)x86e->GetCode();
	block->compiled->count=x86e->UsedBytes()/5;
	block->compiled->parent=block;
	block->compiled->size=x86e->UsedBytes();

	//make it call the stubs , unless otherwise needed
	block->pTF_next_addr=link_compile_inject_TF_stub;
	block->pTT_next_addr=link_compile_inject_TT_stub;


	//block->ilst.opcodes.clear();


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
	delete x86e;
}

//non x86 , no dynarec
#else
void CompileBasicBlock_slow(rec_v1_BasicBlock* block)
{
}
void link_compile_inject_TF_stub(rec_v1_BasicBlock* ptr)
{
}
void link_compile_inject_TT_stub(rec_v1_BasicBlock* ptr)
{
}
#endif



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
		if (ra!=ECX);
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
					x86IntRegType ro,
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
			
			if (ro!=EDX)
				x86e->MOV32RtoR(EDX,ro);
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
				x86e->MOV32MtoR(ro,(u32*)paddr);
			}
		}
	}
}