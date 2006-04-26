#include "shil_compile_slow.h"
#include <assert.h>
#include "emitter.h"

#include "dc\sh4\sh4_registers.h"
#include "dc\sh4\rec_v1\rec_v1_blockmanager.h"
#include "dc\sh4\sh4_opcode_list.h"
#include "dc\mem\sh4_mem.h"

//TEMP!!!
emitter<>* x86e;

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

int block_count=0;
//profiling related things
#ifdef PROFILE_DYNAREC
void profile_ifb_call()
{
	ifb_calls++;
}
#endif


//a few helpers
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

#ifdef REG_ALLOC_DISABLE
#define REG_ALLOC_COUNT 0
#else
#define REG_ALLOC_COUNT 4
#endif
x86IntRegType reg_to_alloc[REG_ALLOC_COUNT+10]=
{
	EBX,
	EBP,
	ESI,//-> reserved for cycle counts : no more
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
#ifndef REG_ALLOC_DISABLE
	return r_alloced[reg].x86reg!=GPR_Error;
#else
	return false;
#endif
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
	#ifndef REG_ALLOC_DISABLE
	for (int i=0;i<16;i++)
	{
		FlushRegCache_reg(i);
	}
#endif
}
void AllocateRegisters(rec_v1_BasicBlock* block)
{
	#ifndef REG_ALLOC_DISABLE
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
	
	for (int j=0;j<op_count;j++)
	{
		curop=&block->ilst.opcodes[j];
		for (int i = 0;i<16;i++)
		{
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
#endif
}
//more helpers
INLINE x86IntRegType LoadReg_force(x86IntRegType to,u8 reg)
{
	#ifndef REG_ALLOC_DISABLE
	if (reg>r15 || (!IsRegCached(reg)))
	{
#endif
		x86e->MOV32MtoR(to,GetRegPtr(reg));
#ifndef REG_ALLOC_DISABLE
	}
	else
	{
		x86IntRegType x86reg=LoadRegCache_reg(reg);
		if (!(to==x86reg))
			x86e->MOV32RtoR(to,x86reg);
	}
#endif

	return to;
}

INLINE x86IntRegType LoadReg(x86IntRegType to,u8 reg)
{
#ifndef REG_ALLOC_DISABLE
	if (reg>r15 || (!IsRegCached(reg)))
	{
#endif
		x86e->MOV32MtoR(to,GetRegPtr(reg));
		return to;
#ifndef REG_ALLOC_DISABLE
	}
	else
	{
		return LoadRegCache_reg(reg);
	}
#endif
}

INLINE x86IntRegType LoadReg_nodata(x86IntRegType to,u8 reg)
{
#ifndef REG_ALLOC_DISABLE
	if (reg>r15 || (!IsRegCached(reg)))
	{
#endif
		//x86e->MOV32MtoR(to,GetRegPtr(reg)); -> do nothin :P
		return to;
#ifndef REG_ALLOC_DISABLE
	}
	else
	{
		return LoadRegCache_reg_nodata(reg);
	}
#endif
}


INLINE void SaveReg(u8 reg,x86IntRegType from)
{
#ifndef REG_ALLOC_DISABLE
	if (reg>r15 || (!IsRegCached(reg)))
	{
#endif
		x86e->MOV32RtoM(GetRegPtr(reg),from);
#ifndef REG_ALLOC_DISABLE
	}
	else
	{
		x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
		if (x86reg!=from)
			x86e->MOV32RtoR(x86reg,from);
	}
#endif	
}

INLINE void SaveReg(u8 reg,u32 from)
{
#ifndef REG_ALLOC_DISABLE
	if (reg>r15 || (!IsRegCached(reg)))
	{
#endif
		x86e->MOV32ItoM(GetRegPtr(reg),from);
#ifndef REG_ALLOC_DISABLE
	}
	else
	{
		x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
		if (from==0)
			x86e->XOR32RtoR(x86reg,x86reg);
		else
			x86e->MOV32ItoR(x86reg,from);
	}
#endif
}

INLINE void SaveReg(u8 reg,u32* from)
{
#ifndef REG_ALLOC_DISABLE
	if (reg>r15 || (!IsRegCached(reg)))
	{
#endif
		x86e->MOV32MtoR(ECX,from);
		x86e->MOV32RtoM(GetRegPtr(reg),ECX);
#ifndef REG_ALLOC_DISABLE
	}
	else
	{
		x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
		x86e->MOV32MtoR(x86reg,from);
	}
#endif
}

INLINE void SaveReg(u8 reg,u16* from)
{
#ifndef REG_ALLOC_DISABLE
	if (reg>r15 || (!IsRegCached(reg)))
	{
#endif
		x86e->MOVSX32M16toR(ECX,from);
		x86e->MOV32RtoM(GetRegPtr(reg),ECX);
#ifndef REG_ALLOC_DISABLE
	}
	else
	{
		x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
		x86e->MOVSX32M16toR(x86reg,from);
	}
#endif
}

INLINE void SaveReg(u8 reg,u8* from)
{
#ifndef REG_ALLOC_DISABLE
	if (reg>r15 || (!IsRegCached(reg)))
	{
#endif
		x86e->MOVSX32M8toR(ECX,from);
		x86e->MOV32RtoM(GetRegPtr(reg),ECX);
#ifndef REG_ALLOC_DISABLE
	}
	else
	{
		x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
		x86e->MOVSX32M8toR(x86reg,from);
	}
#endif
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
#define OP_MoRtR_ItR(_MtR_,_RtR_,_ItR_)	assert(FLAG_32==(op->flags & 3));\
	assert(0==(op->flags & (FLAG_IMM2)));\
	assert(op->flags & FLAG_REG1);\
	if (op->flags & FLAG_IMM1)\
	{\
		assert(0==(op->flags & FLAG_REG2));\
		x86IntRegType r1 = LoadReg(EAX,op->reg1);\
		x86e-> _ItR_ (r1,op->imm1);\
		SaveReg(op->reg1,r1);\
	}\
	else\
	{\
		assert(op->flags & FLAG_REG2);\
		x86IntRegType r1 = LoadReg(EAX,op->reg1);\
		if (IsRegCached(op->reg2))\
		{\
			x86IntRegType r2 = LoadReg(EAX,op->reg2);\
			assert(r2!=EAX);\
			x86e-> _RtR_ (r1,r2);\
		}\
		else\
			x86e-> _MtR_ (r1,GetRegPtr(op->reg2));\
		SaveReg(op->reg1,r1);\
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
	OP_MoRtR_ItR(XOR32MtoR,XOR32RtoR,XOR32ItoR);
}
void __fastcall shil_compile_or(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_MoRtR_ItR(OR32MtoR,OR32RtoR,OR32ItoR);
}
void __fastcall shil_compile_and(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_MoRtR_ItR(AND32MtoR,AND32RtoR,AND32ItoR);
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
		LoadReg_force(EDX,op->reg1);
		x86e->CALLFunc(WriteMem8);
	}
	else if (size==FLAG_16)
	{	//maby zx ?
		LoadReg_force(EDX,op->reg1);
		x86e->CALLFunc(WriteMem16);
	}
	else if (size==FLAG_32)
	{
		LoadReg_force(EDX,op->reg1);
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
	OP_MoRtR_ItR(ADD32MtoR,ADD32RtoR,ADD32ItoR);
}
void __fastcall shil_compile_adc(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_MoRtR_ItR(ADC32MtoR,ADC32RtoR,ADC32ItoR);
}
void __fastcall shil_compile_sub(shil_opcode* op,rec_v1_BasicBlock* block)
{
	OP_MoRtR_ItR(SUB32MtoR,SUB32RtoR,SUB32ItoR);
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

		x86e->IMUL32RtoR(EAX,ECX);
		
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
	
	x86e->ADD32ItoM(&rec_cycles,block->cycles);
	//x86e->MOV32ItoM((u32*)&rec_v1_pCurrentBlock,(u32)block);

	AllocateRegisters(block);

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
				x86e->MOV32MtoR(EAX,pTF_f);		//assume it's this condition , unless CMOV overwrites
				//!=
				x86e->CMOVNE32MtoR(EAX,pTT_f);	//overwrite the "other" pointer if needed
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