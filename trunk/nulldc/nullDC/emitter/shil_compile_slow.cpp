#include "shil_compile_slow.h"
#include <assert.h>
#include "emitter.h"

#include "dc\sh4\sh4_registers.h"
#include "dc\sh4\rec_v1\rec_v1_blockmanager.h"
#include "dc\sh4\sh4_opcode_list.h"
#include "dc\mem\sh4_mem.h"

//TEMP!!!
emitter<>* x86e;
shil_scs shil_compile_slow_settings=
{
	false	//do Register allocation for x86
	,0		//on 4 regisers
	,false	//and on XMM
	,true	//Inline Const Mem reads
	,true	//Inline normal mem reads
	,true	//Inline mem writes
};


#define REG_ALLOC_COUNT			(shil_compile_slow_settings.RegAllocCount)
#define REG_ALLOC_X86			(shil_compile_slow_settings.RegAllocX86)
#define REG_ALLOC_XMM			(shil_compile_slow_settings.RegAllocXMM)
#define INLINE_MEM_READ_CONST   (shil_compile_slow_settings.InlineMemRead_const)
#define INLINE_MEM_READ			(shil_compile_slow_settings.InlineMemRead)
#define INLINE_MEM_WRITE		(shil_compile_slow_settings.InlineMemWrite)


typedef void __fastcall shil_compileFP(shil_opcode* op,rec_v1_BasicBlock* block);

#ifdef PROFILE_DYNAREC
u64 ifb_calls;
#endif

bool inited=false;

int fallbacks=0;
int native=0;
u32 T_jcond_value;
u32 reg_pc_temp_value;
rec_v1_BasicBlock* rec_v1_pCurrentBlock;
u32 IsRamAddr[0xFF];
int block_count=0;
//profiling related things
#ifdef PROFILE_DYNAREC
void profile_ifb_call()
{
	ifb_calls++;
}
#endif

//a few helpers

typedef void opMtoR_FP (x86IntRegType to,u32* from);
typedef void opItoR_FP (x86IntRegType to,u32 from);
typedef void opRtoR_FP (x86IntRegType to,x86IntRegType from);

typedef void opItoM_FP (u32* to, u32 from);
typedef void opRtoM_FP (u32* to, x86IntRegType from);


void c_Ensure32()
{
	assert(fpscr.PR==0);
}
bool Ensure32()
{
	x86e->CALLFunc(c_Ensure32);
	return true;
}

INLINE u32* GetRegPtr(u8 reg)
{
	if (reg==Sh4RegType::reg_pc_temp)
		return &reg_pc_temp_value;

	u32* rv=Sh4_int_GetRegisterPtr((Sh4RegType)reg);
	assert(rv!=0);
	return rv;
}

//REGISTER ALLOCATION

struct RegAllocInfo
{
	x86IntRegType x86reg;
	bool InReg;
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
	ESI,//-> reserved for cycle counts : no more :)
	EDI
};

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
INLINE void FlushRegCache_reg(u8 reg)
{
	if (r_alloced[reg].InReg)
	{
		r_alloced[reg].InReg=false;
		x86e->MOV32RtoM(GetRegPtr(reg),r_alloced[reg].x86reg);
	}
}
INLINE bool IsRegCached(u8 reg)
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
INLINE x86IntRegType LoadRegCache_reg(u8 reg)
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

INLINE x86IntRegType LoadRegCache_reg_nodata(u8 reg)
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
INLINE void FlushRegCache()
{
	if(REG_ALLOC_X86)
	{
		for (int i=0;i<16;i++)
		{
			FlushRegCache_reg(i);
		}
	}
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
					used[i].cnt+=1;

				if (curop->ReadsReg((Sh4RegType) (r0+i)))
					used[i].cnt+=1;

				if (curop->WritesReg((Sh4RegType) (r0+i)))
					used[i].cnt+=1;
			}
		}

		bubble_sort(used,16);

		for (int i=0;i<REG_ALLOC_COUNT;i++)
		{
			if (used[i].cnt==0)
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

	return to;
}

INLINE x86IntRegType LoadReg(x86IntRegType to,u8 reg)
{
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOV32MtoR(to,GetRegPtr(reg));
			return to;
		}
		else
		{
			return LoadRegCache_reg(reg);
		}
	}
	else
	{
		x86e->MOV32MtoR(to,GetRegPtr(reg));
		return to;
	}
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
}

INLINE void SaveReg(u8 reg,u32 from)
{
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
}

INLINE void SaveReg(u8 reg,u32* from)
{
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
	}

INLINE void SaveReg(u8 reg,u16* from)
{
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOVSX32M16toR(ECX,from);
			x86e->MOV32RtoM(GetRegPtr(reg),ECX);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			x86e->MOVSX32M16toR(x86reg,from);
		}
	}
	else
	{
		x86e->MOVSX32M16toR(ECX,from);
		x86e->MOV32RtoM(GetRegPtr(reg),ECX);
	}
}

INLINE void SaveReg(u8 reg,u8* from)
{
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOVSX32M8toR(ECX,from);
			x86e->MOV32RtoM(GetRegPtr(reg),ECX);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			x86e->MOVSX32M8toR(x86reg,from);
		}
	}
	else
	{
		x86e->MOVSX32M8toR(ECX,from);
		x86e->MOV32RtoM(GetRegPtr(reg),ECX);
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
			x86e-> _ItR_ (r1,op->imm1);\
			SaveReg(op->reg1,r1);\
		}\
		else\
		{\
			x86e-> _ItM_ (GetRegPtr(op->reg1),op->imm1);\
		}\
	}\
	else\
	{\
		assert(op->flags & FLAG_REG2);\
		if (IsRegCached(op->reg1))\
		{\
			x86IntRegType r1 = LoadReg(EAX,op->reg1);\
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
		if (op->flags & FLAG_REG2)
		{
			assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));//no imm can be used
			//printf("mov [r]%d=[r]%d , sz==%d\n",op->reg1,op->reg2,size);

		 	//LoadReg(EAX,op->reg2);
			x86IntRegType r2=LoadReg(EAX,op->reg2);
			SaveReg(op->reg1,r2);
		}
		else
		{
			assert(0==(op->flags & (FLAG_IMM2)));//no imm2 can be used
			//printf("mov [r]%d=[imm]%d , sz==%d\n",op->reg1,op->imm1,size);

			//x86e->MOV32ItoR(EAX,);
			SaveReg(op->reg1,op->imm1);
		}
	}
	else
	{
		assert(size==FLAG_64);//32 or 64 b
		assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2)));//no imm can be used
		//printf("mov64 not supported\n");
		u8 dest=GetSingleFromDouble((Sh4RegType)op->reg1);
		u8 source=GetSingleFromDouble((Sh4RegType)op->reg2);

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
#ifdef PROFILE_DYNAREC
	x86e->CALLFunc(profile_ifb_call);
#endif
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
}

void __fastcall shil_compile_shl(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_ItMoR(SHL32ItoM,SHL32ItoR,(u8)op->imm1);
}

void __fastcall shil_compile_shr(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_ItMoR(SHR32ItoM,SHR32ItoR,(u8)op->imm1);
}

void __fastcall shil_compile_sar(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_ItMoR(SAR32ItoM,SAR32ItoR,(u8)op->imm1);
}

//rotates
void __fastcall shil_compile_rcl(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_NtMoR_noimm(RCL321toM,RCL321toR);
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
	OP_RegToReg_simple(XOR32);
}
void __fastcall shil_compile_or(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_RegToReg_simple(OR32);
}
void __fastcall shil_compile_and(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_RegToReg_simple(AND32);
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
		//reg0/gbr[reg2] form
		assert(op->flags & (FLAG_R0|FLAG_GBR));

		assert(0==(op->flags & FLAG_IMM1));

		if (op->flags & FLAG_R0)
			LoadReg_force(ECX,r0);
		else
			LoadReg_force(ECX,reg_gbr);

		if (IsRegCached(op->reg2))
		{
			x86IntRegType r2=LoadReg(EAX,op->reg2);
			assert(r2!=EAX);
			x86e->ADD32RtoR(ECX,r2);
		}
		else
			x86e->ADD32MtoR(ECX,GetRegPtr(op->reg2));
	}
}
INLINE bool IsOnRam(u32 addr)
{
	if (((addr>>26)&0x7)==3)
	{
		if ((((addr>>29) &0x7)!=7))
		{
			return true;
		}
	}

	return false;
}
void __fastcall shil_compile_readm(shil_opcode* op,rec_v1_BasicBlock* block)
{
	u32 size=op->flags&3;

	if (INLINE_MEM_READ_CONST)
	{
		//if constant read , and on ram area , make it a direct mem access
		//_watch_ mmu
		if (!(op->flags & (FLAG_R0|FLAG_GBR)))
		{//[reg2] form
			assert(op->flags & FLAG_IMM1);

			if (op->flags & FLAG_REG2)
			{	//[reg2+imm1]

			}
			else
			{	//[imm1]
				assert(0==(op->flags & FLAG_REG2));
				if (IsOnRam(op->imm1))
				{
					if (size==0)
					{
						SaveReg(op->reg1,(u8 *)GetMemPtr(op->imm1,1));
					}
					else 	if (size==1)
					{
						SaveReg(op->reg1,(u16*)GetMemPtr(op->imm1,2));
					}
					else 	if (size==2)
					{
						SaveReg(op->reg1,(u32*)GetMemPtr(op->imm1,4));
					}
					return;
				}
			}
		}
	}

	readwrteparams(op);

	u8* inline_label;

	if (INLINE_MEM_READ)
	{
		//try to inline all mem reads , by comparing values at runtime :P
		//ECX is address

		//eax = ((ecx>>24) & 0xFF)<<2

		//mov eax,ecx
		x86e->MOV32RtoR(EAX,ECX);
		//shr eax,24
		//shl eax,2
		x86e->SHR32ItoR(EAX,24);
		x86e->SHL32ItoR(EAX,2);

		//add eax , imm	;//should realy use lea/mov eax,[eax+xx]
		x86e->ADD32ItoR(EAX,(u32)IsRamAddr);
		//mov eax,[eax]
		x86e->MOV32RmtoR(EAX,EAX);
		//test eax,eax
		x86e->TEST32RtoR(EAX,EAX);
		//jz inline;//if !0 , then do normal read
		inline_label = x86e->JZ8(0);
	}
	//call ReadMem32
	//movsx [if needed]
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

	if (INLINE_MEM_READ)
	{
		//jmp normal;
		u8* normal_label = x86e->JMP8(0);
		//
		//inline:  //inlined ram read
		x86e->x86SetJ8(inline_label);
		//and ecx , ram_mask
		x86e->AND32ItoR(ECX,RAM_MASK);
		//add ecx, ram_base
		x86e->ADD32ItoR(ECX,(u32)(&mem_b[0]));
		//mov/sx eax,[ecx]
		if (size==0)
		{
			x86e->MOV32RmtoR(EAX,ECX);		//1 byte read ?
			x86e->MOVSX32R8toR(EAX,EAX);	//se8
		}
		else if (size==1)
		{
			x86e->MOV32RmtoR(EAX,ECX);		//2 bytes read ?
			x86e->MOVSX32R16toR(EAX,EAX);	//se16
		}
		else if (size==2)
		{
			x86e->MOV32RmtoR(EAX,ECX);
		}
		else
			printf("ReadMem error\n");
		//normal:
		x86e->x86SetJ8(normal_label);
	}
	//mov reg,eax
	SaveReg(op->reg1,EAX);//save return value
}
void __fastcall shil_compile_writem(shil_opcode* op,rec_v1_BasicBlock* block)
{
	
	u32 size=op->flags&3;

	readwrteparams(op);

	//ECX is address

	//so it's sure loaded (if from reg cache)
	x86IntRegType r1=LoadReg(EDX,op->reg1);
	
	u8* inline_label;

	if (INLINE_MEM_WRITE)
	{	//try to inline all mem reads at runtime :P
		//eax = ((ecx>>24) & 0xFF)<<2

		//mov eax,ecx
		x86e->MOV32RtoR(EAX,ECX);
		//shr eax,24
		//shl eax,2
		x86e->SHR32ItoR(EAX,24);
		x86e->SHL32ItoR(EAX,2);

		//add eax , imm	;//should realy use lea/mov eax,[eax+xx]
		x86e->ADD32ItoR(EAX,(u32)IsRamAddr);
		//mov eax,[eax]
		x86e->MOV32RmtoR(EAX,EAX);
		//test eax,eax
		x86e->TEST32RtoR(EAX,EAX);
		//jz inline;//if !0 , then do normal write
		inline_label = x86e->JZ8(0);
	}
	if (size==FLAG_8)
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
		printf("WriteMem error\n");

	
	if (INLINE_MEM_WRITE)
	{
		//jmp normal;
		u8* normal_label = x86e->JMP8(0);
		//
		//inline:  //inlined ram write
		x86e->x86SetJ8(inline_label);

		//and ecx , ram_mask
		x86e->AND32ItoR(ECX,RAM_MASK);


		//if needed
		if (r1==EDX)
			x86e->PUSH32R(r1);


		//call rec_v1_BlockTest
		//
		rec_v1_CompileBlockTest(x86e,ECX,EAX);

		//if needed
		if (r1==EDX)
			x86e->POP32R(r1);

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
	}

}

//save-loadT
void __fastcall shil_compile_SaveT(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(op->flags & FLAG_IMM1);//imm1
	assert(0==(op->flags & (FLAG_IMM2|FLAG_REG1|FLAG_REG2)));//no imm2/r1/r2
	x86e->SETcc8R(EAX,op->imm1);//imm1 :P
	x86e->MOVZX32R8toR(EAX,EAX);//clear rest of eax (to remove partial depency on 32:8)
	
	//x86e->AND32ItoR(EAX,1);		//and 1

	LoadReg_force(ECX,reg_sr);			//ecx=sr(~1)|T
	x86e->AND32ItoR(ECX,(u32)~1);
	x86e->OR32RtoR(ECX,EAX);
	SaveReg(reg_sr,ECX);
}
void __fastcall shil_compile_LoadT(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(op->flags & FLAG_IMM1);//imm1
	assert(0==(op->flags & (FLAG_IMM2|FLAG_REG1|FLAG_REG2)));//no imm2/r1/r2
	

	assert( (op->imm1==x86_flags::CF) || (op->imm1==x86_flags::jcond_flag) );

	if (op->imm1==x86_flags::jcond_flag)
	{
		LoadReg_force(EAX,reg_sr);
		x86e->MOV32RtoM(&T_jcond_value,EAX);//T_jcond_value;
	}
	else
	{
		LoadReg_force(EAX,reg_sr);
		x86e->SHR32ItoR(EAX,1);//heh T bit is there now :P CF
	}
}
//cmp-test

void __fastcall shil_compile_cmp(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(FLAG_32==(op->flags & 3));
	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & (FLAG_REG2|FLAG_IMM2)));
		x86IntRegType r1 = LoadReg(EAX,op->reg1);
		x86e->CMP32ItoR(r1,op->imm1);
		//eflags is used w/ combination of SaveT
	}
	else
	{
		assert(0==(op->flags & FLAG_IMM2));
		assert(op->flags & FLAG_REG2);
		x86IntRegType r1 = LoadReg(EAX,op->reg1);
		x86IntRegType r2 = LoadReg(ECX,op->reg2);
		x86e->CMP32RtoR(r1,r2);//rm,rn
		//eflags is used w/ combination of SaveT
	}
}
void __fastcall shil_compile_test(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(FLAG_32==(op->flags & 3));

	if (op->flags & FLAG_IMM1)
	{
		assert(0==(op->flags & (FLAG_REG2|FLAG_IMM2)));
		x86IntRegType r1 = LoadReg(EAX,op->reg1);
		x86e->TEST32ItoR(r1,op->imm1);
		//eflags is used w/ combination of SaveT
	}
	else
	{
		assert(0==(op->flags & FLAG_IMM2));
		assert(op->flags & FLAG_REG2);
		x86IntRegType r1 = LoadReg(EAX,op->reg1);
		x86IntRegType r2 = LoadReg(ECX,op->reg2);
		x86e->TEST32RtoR(r1,r2);
		//eflags is used w/ combination of SaveT
	}
}

//add-sub
void __fastcall shil_compile_add(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_RegToReg_simple(ADD32);
}
void __fastcall shil_compile_adc(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_RegToReg_simple(ADC32);
}
void __fastcall shil_compile_sub(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_RegToReg_simple(SUB32);
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
	}
}

//FPU !!! YESH
void __fastcall shil_compile_fneg(shil_opcode* op,rec_v1_BasicBlock* block)
{
	assert(0==(op->flags & (FLAG_IMM1|FLAG_IMM2|FLAG_REG2)));
	u32 sz=op->flags & 3;
	if (sz==FLAG_32)
	{
		assert(!IsReg64 ((Sh4RegType)op->reg1));
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
	}
	else
	{
		assert(sz==FLAG_64);
		assert(IsReg64((Sh4RegType)op->reg1));
		u32 reg=GetSingleFromDouble(op->reg1);
		x86e->AND32ItoM(GetRegPtr(reg+1),0x7FFFFFFF);
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
		//x86e->AND32ItoM(GetRegPtr(op->reg1),0x7FFFFFFF);
		x86e->SSE_MOVSS_M32_to_XMM(XMM0,GetRegPtr(op->reg1));
		x86e->SSE_ADDSS_M32_to_XMM(XMM0,GetRegPtr(op->reg2));
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(op->reg1),XMM0);
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
	assert(false);
}

#ifdef PROFILE_DYNAREC

u64  dyn_profile_cycles[shil_count];
u64  dyn_profile_calls[shil_count];

u64  dyn_profile_vc[shil_count];

int prints_count=0;
void printprofile()
{
	prints_count++;
	if (prints_count==1)
	{

		u64 max=0;
		double av=0;
		u32 im=0;
		for (int i=0;i<shil_count;i++)
		{

			if (dyn_profile_calls[i]>0)
			{
				double cr=(double)dyn_profile_cycles[i]/(double)dyn_profile_calls[i];
				if (dyn_profile_cycles[i]>max)
				{
					max=dyn_profile_cycles[i];
					im=i;
				}
			//	printf("opcode %s , %Lf cycles/call [%Lf calls , %Lf cycles]\n",GetShilName((shil_opcodes)i)
			//		,cr
			//		,(double)dyn_profile_calls[i]
			//	,(double)dyn_profile_cycles[i]
			//	);
			}
		}

		double cr=(double)dyn_profile_cycles[im]/(double)dyn_profile_calls[im];
		printf("slower opcode %s , %Lf cycles/call [%Lf calls , %Lf cycles]\n",GetShilName((shil_opcodes)im)
			,cr
			,(double)dyn_profile_calls[im]
		,(double)dyn_profile_cycles[im]
		);


		//print a sorted time ?
		//clear em
		for (int i=0;i<shil_count;i++)
		{
			dyn_profile_cycles[im]=0;
			dyn_profile_calls[im]=0;
		}
		prints_count=0;
	}
}
union Cmp64
{
	struct
	{
		u32 l;
		u32 h;
	};
	u64 v;
};

Cmp64 dyn_last;
Cmp64 dyn_now;
void __fastcall dyna_profile_cookie_start()
{
	__asm
	{
		rdtsc;
		mov dyn_last.l,eax;
		mov dyn_last.h,edx;
	}
}

void __fastcall dyna_profile_cookie(u32 op)
{
	__asm
	{
		rdtsc;
		mov dyn_now.l,eax;
		mov dyn_now.h,edx;
	}

	dyn_profile_calls[op]++;
	dyn_profile_cycles[op]+=dyn_now.v-dyn_last.v;
	dyn_last.v=dyn_now.v;
}
#endif

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
}
//Compile block and return pointer to it's code
void* __fastcall link_compile_inject_TF(rec_v1_BasicBlock* ptr)
{
	rec_v1_BasicBlock* target= rec_v1_FindOrRecompileCode(ptr->TF_next_addr);
	
	//Add reference so we can undo the chain later
	target->AddRef(ptr);
	ptr->TF_block=target;
	return ptr->pTF_next_addr=target->compiled->Code;
}

void* __fastcall link_compile_inject_TT(rec_v1_BasicBlock* ptr)
{
	rec_v1_BasicBlock* target= rec_v1_FindOrRecompileCode(ptr->TT_next_addr);

	//Add reference
	target->AddRef(ptr);
	ptr->TT_block=target;
	return ptr->pTT_next_addr=target->compiled->Code;
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
void CompileBasicBlock_slow(rec_v1_BasicBlock* block)
{
	if (!inited)
	{
		Init();
		inited=true;
	}


	x86e=new emitter<>();
	block->compiled=new rec_v1_CompiledBlock();
	

	AllocateRegisters(block);
	LoadRegisters();
	s8* start_ptr=x86e->x86Ptr;

	x86e->ADD32ItoM(&rec_cycles,block->cycles);
	//x86e->MOV32ItoM((u32*)&rec_v1_pCurrentBlock,(u32)block);

#ifdef PROFILE_DYNAREC
	x86e->CALLFunc(dyna_profile_cookie_start);
#endif
	u32 list_sz=block->ilst.opcodes.size();
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
#ifdef PROFILE_DYNAREC
			x86e->PUSHFD();
			x86e->PUSH32R(EAX);
			x86e->PUSH32R(ECX);
			x86e->PUSH32R(EDX);
			x86e->MOV32ItoR(ECX,op->opcode);
			x86e->CALLFunc(dyna_profile_cookie);
			x86e->POP32R(EDX);
			x86e->POP32R(ECX);
			x86e->POP32R(EAX);
			x86e->POPFD();
#endif
	}

	FlushRegCache();//flush reg cache

	//end block acording to block type :)
	switch(block->flags & BLOCK_TYPE_MASK)
	{
		case BLOCK_TYPE_DYNAMIC:
		{
			x86e->RET();
			break;
		}

	case BLOCK_TYPE_COND_0:
	case BLOCK_TYPE_COND_1:
		{
			//ok , handle COND_0/COND_1 here :)
			//mem address
			u32* TT_a=&block->TT_next_addr;
			u32* TF_a=&block->TF_next_addr;
			//functions
			u32* pTF_f=(u32*)&(block->pTF_next_addr);
			u32* pTT_f=(u32*)&(block->pTT_next_addr);
			
			if ((block->flags & BLOCK_TYPE_MASK)==BLOCK_TYPE_COND_0)
			{
				TT_a=&block->TF_next_addr;
				TF_a=&block->TT_next_addr;
				pTF_f=(u32*)&(block->pTT_next_addr);
				pTT_f=(u32*)&(block->pTF_next_addr);
			}


			x86e->CMP32ItoM(&rec_cycles,BLOCKLIST_MAX_CYCLES);
			
			u8* Link=x86e->JB8(0);
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
			//Link:
			//if we can execute more blocks
			x86e->x86SetJ8(Link);
			{
				//for dynamic link!
				x86e->MOV32ItoR(ECX,(u32)block);					//mov ecx , block
				x86e->MOV32MtoR(EAX,&T_jcond_value);
				x86e->TEST32ItoR(EAX,1);//test for T


				//link to next block :
				if (*TF_a==block->start)
				{
					//fast link (direct jmp to block start)
					//x86e->MOV32ItoR(EAX,(u32)start_ptr);		//assume it's this condition , unless CMOV overwrites
					if (x86e->CanJ8(start_ptr))
						x86e->JE8(start_ptr);
					else
						x86e->JE32(start_ptr);
				}
				else
				{
					x86e->MOV32MtoR(EAX,pTF_f);		//assume it's this condition , unless CMOV overwrites
				}
				//!=

				if (*TT_a==block->start)
				{
					//fast link (direct jmp to block start)
					//x86e->MOV32ItoR(EDX,(u32)start_ptr);
					//x86e->CMOVNE32RtoR(EAX,EDX);	//overwrite the "other" pointer if needed
					if (x86e->CanJ8(start_ptr))
						x86e->JNE8(start_ptr);
					else
						x86e->JNE32(start_ptr);
				}
				else
				{
					x86e->CMOVNE32MtoR(EAX,pTT_f);	//overwrite the "other" pointer if needed
				}
				x86e->JMP32R(EAX);		 //!=
			}
		}
		break;
	case BLOCK_TYPE_FIXED:
		{
			x86e->CMP32ItoM(&rec_cycles,BLOCKLIST_MAX_CYCLES);
			u8* Link=x86e->JB8(0);

			//If our cycle count is expired
			x86e->MOV32ItoM(GetRegPtr(reg_pc),block->TF_next_addr);
			x86e->RET();//return to caller to check for interrupts


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


	block->ilst.opcodes.clear();


	block_count++;
	if ((block_count%512)==128)
	{
		printf("Recompiled %d blocks\n",block_count);
		u32 rat=native>fallbacks?fallbacks:native;
		if (rat!=0)
			printf("Native/Fallback ratio : %d:%d [%d:%d]\n",native,fallbacks,native/rat,fallbacks/rat);
		else
			printf("Native/Fallback ratio : %d:%d [%d:%d]\n",native,fallbacks,native,fallbacks);
		printf("Average block size : %d opcodes\n",(fallbacks+native)/block_count);
	}
	delete x86e;
}