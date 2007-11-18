#pragma once
#include "types.h"
#include "x86_op_classes.h"

using namespace std;
//Oh god , x86 is a sooo badly designed opcode arch -_-

const char* DissasmClass(x86_opcode_class opcode);

#define REG_CLASS(regv) (regv>>16)
#define REG_ID(regv) (regv&0xFFFF)

enum reg_class
{
	reg_GPR8=0<<16,
	reg_GPR16=0,//1<<16,
	reg_GPR32=0,//2<<16,
	reg_SSE=0,//3<<16,
#ifdef X64
	reg_GPR64=0,//4<<16,
#endif
};
//Enum of all registers
enum x86_reg
{
//8 bit

	AL=reg_GPR8,
	CL,
	DL,
	BL,

#ifndef X64
	AH,	//these are ONLY avaialbe on x86 mode.They will possibly added later  for x64...
	CH,
	DH,
	BH,
#endif

#ifdef X64
	R0b=reg_GPR8,
	R1b,
	R2b,
	R3b,
	R4b,
	R5b,
	R6b,
	R7b,
	R8b,
	R9b,
	R10b,
	R11b,
	R12b,
	R13b,
	R14b,
	R15b,
#endif

//16 bit

	AX=reg_GPR16,
	CX,
	DX,
	BX,
	SP,
	BP,
	SI,
	DI,
#ifdef X64
	R0w=reg_GPR16,	//these are the same as AX .. DI
	R1w,
	R2w,
	R3w,
	R4w,
	R5w,
	R6w,
	R7w,
	R8w,
	R9w,
	R10w,
	R11w,
	R12w,
	R13w,
	R14w,
	R15w,
#endif

//32 bit

	EAX=reg_GPR32,
	ECX,
	EDX,
	EBX,
	ESP,
	EBP,
	ESI,
	EDI,
#ifdef X64
	R0d=reg_GPR32,	//these are the same as EAX .. EDI
	R1d,
	R2d,
	R3d,
	R4d,
	R5d,
	R6d,
	R7d,
	R8d,
	R9d,
	R10d,
	R11d,
	R12d,
	R13d,
	R14d,
	R15d,
#endif

//64 bit

#ifdef X64
	R0q=reg_GPR64,
	R1q,
	R2q,
	R3q,
	R4q,
	R5q,
	R6q,
	R7q,
	R8q,
	R9q,
	R10q,
	R11q,
	R12q,
	R13q,
	R14q,
	R15q,
#endif
	
//XMM (SSE)

	XMM0=reg_SSE,
	XMM1,
	XMM2,
	XMM3,
	XMM4,
	XMM5,
	XMM6,
	XMM7,
#ifdef X64
	XMM8,
	XMM9,
	XMM10,
	XMM11,
	XMM12,
	XMM13,
	XMM14,
	XMM15,
#endif

#ifdef USE_MM
	//mmx (? will it be suported by the emitter?) -> propably no , SSE2 mailny replaces em w/ integer XMM math
	MM0=0,
	MM1,
	MM2,
	MM3,
	MM4,
	MM5,
	MM6,
	MM7,
#endif

	//misc :p
	NO_REG=-1,
	ERROR_REG=-2,
};
#define x86_sse_reg x86_reg
#define x86_gpr_reg x86_reg

//memory managment !
typedef void* dyna_reallocFP(void*ptr,u32 oldsize,u32 newsize);
typedef void* dyna_finalizeFP(void* ptr,u32 oldsize,u32 newsize);

//define it here cus we use it on label type ;)
class x86_block;
// a label
struct __declspec(dllexport) x86_Label
{
	u32 target_opcode;
	u8 patch_sz;
	x86_block* owner;
	bool marked;
	void* GetPtr();
};
//An empty type that we will use as ptr type.This is ptr-reference
struct __declspec(dllexport)  x86_ptr
{
	union
	{
		void* ptr;
		unat ptr_int;
	};
	static x86_ptr create(unat ptr);
	x86_ptr(void* ptr)
	{
		this->ptr=ptr;
	}
};
//This is ptr/imm (for call/jmp)
struct __declspec(dllexport)  x86_ptr_imm
{
	union
	{
		void* ptr;
		unat ptr_int;
	};
	static x86_ptr_imm create(unat ptr);
	x86_ptr_imm(void* ptr)
	{
		this->ptr=ptr;
	}
};

enum x86_mrm_mod
{
	mod_RI,			//[reg]
	mod_RI_disp,	//[reg+disp]
	mod_DISP,		//[disp]
	mod_REG,		//reg
	mod_SIB,		//[reg1*scale+reg2], reg2 can be NO_REG , reg1 can't
	mod_SIB_disp	//[(reg1*scale+reg2)+disp], reg2 can be NO_REG , reg1 can't
};
enum x86_sib_scale
{
	sib_scale_1,
	sib_scale_2,
	sib_scale_4,
	sib_scale_8
};
//shit
struct  __declspec(dllexport)  x86_mrm
{
	u8 flags;
	u8 modrm;
	u8 sib;
	u32 disp;

	//modr/m encoding can be improved , to add support for sib/ebp encodings ;)
	//its good for now tho
	static x86_mrm create(x86_reg base);
	static x86_mrm create(x86_reg base,x86_ptr disp);
	static x86_mrm create(x86_reg base,x86_reg index);
	static x86_mrm create(x86_reg index,x86_sib_scale scale,x86_ptr disp);
	static x86_mrm create(x86_reg base,x86_reg index,x86_sib_scale scale,x86_ptr disp);
};


struct code_patch
{
	u8 type;//0 = 8 bit , 2 = 16 bit  , 4 = 32 bit , 16[flag] is label
	union
	{
		void* dest;		//ptr for patch
		x86_Label* lbl;	//lbl for patch
	};
	u32 offset;			//offset in opcode stream :)
};
//A block of x86 code :p
class __declspec(dllexport) x86_block
{
private:
	vector<x86_Label*> labels;
	void ApplyPatches(u8* base);
	dyna_reallocFP* ralloc;
	dyna_finalizeFP* allocfin;
public:
	vector<code_patch> patches;
	u8* x86_buff;
	u32 x86_indx;
	u32 x86_size;
	bool do_realloc;

	~x86_block();
	void x86_buffer_ensure(u32 size);

	void  x86_block::write8(u32 value);
	void  x86_block::write16(u32 value);
	void  x86_block::write32(u32 value);

	//init things
	void Init(dyna_reallocFP* ral,dyna_finalizeFP* alf);

	//Generates code.
	void* Generate();
	//void CopyTo(void* to);

	//Will free any used resources exept generated code
	void Free();

	//Label related code
	//NOTE : Label position in mem must not chainge
	void CreateLabel(x86_Label* lbl,bool mark,u32 sz);
	//Allocate a label and create it :).Will be delete'd when calling free and/or dtor
	x86_Label* CreateLabel(bool mark,u32 sz);
	void MarkLabel(x86_Label* lbl);

	//When we want to keep info to mark opcodes dead , there is no need to create labels :p
	//Get an index to next emitted opcode
	u32 GetOpcodeIndex();

	//opcode Emitters

	//no param
	void x86_block::Emit(x86_opcode_class op);
	//1 param
	//reg
	void x86_block::Emit(x86_opcode_class op,x86_reg reg);
	//smrm
	void x86_block::Emit(x86_opcode_class op,x86_ptr mem);
	//mrm
	void x86_block::Emit(x86_opcode_class op,x86_mrm mrm);
	//imm
	void x86_block::Emit(x86_opcode_class op,u32 imm);
	//ptr_imm
	void x86_block::Emit(x86_opcode_class op,x86_ptr_imm disp);
	//lbl
	void x86_block::Emit(x86_opcode_class op,x86_Label* lbl);

	//2 param
	//reg,reg	,reg1 is writen
	void x86_block::Emit(x86_opcode_class op,x86_reg reg1,x86_reg reg2);
	//reg,smrm	,reg is writen
	void x86_block::Emit(x86_opcode_class op,x86_reg reg,x86_ptr mem);
	//reg,mrm	,reg is writen
	void x86_block::Emit(x86_opcode_class op,x86_reg reg1,x86_mrm mrm);
	//reg,imm	,reg is writen
	void x86_block::Emit(x86_opcode_class op,x86_reg reg,u32 imm);
	//smrm,reg	,mem is writen
	void x86_block::Emit(x86_opcode_class op,x86_ptr mem,x86_reg reg);
	//smrm,imm	,mem is writen
	void x86_block::Emit(x86_opcode_class op,x86_ptr mem,u32 imm);

	//mrm,reg	,mrm is writen
	void x86_block::Emit(x86_opcode_class op,x86_mrm mrm,x86_reg reg);
	//mrm,imm	,mrm is writen
	void x86_block::Emit(x86_opcode_class op,x86_mrm mrm,u32 imm);

	//3 param
	//reg,reg,imm	,reg1 is writen
	void x86_block::Emit(x86_opcode_class op,x86_reg reg1,x86_reg reg2,u32 imm);
	//reg,mrm,imm	,reg1 is writen
	void x86_block::Emit(x86_opcode_class op,x86_reg reg,x86_ptr mem,u32 imm);
	//reg,mrm,imm	,reg1 is writen
	void x86_block::Emit(x86_opcode_class op,x86_reg reg,x86_mrm mrm,u32 imm);
};
