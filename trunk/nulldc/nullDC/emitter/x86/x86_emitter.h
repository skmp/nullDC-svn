#pragma once
#include "types.h"
#include "x86_op_classes.h"

using namespace std;
//Oh god , x86 is a sooo badly designed opcode arch -_-

const char* DissasmClass(x86_opcode_class opcode);

//Enum of all registers
enum x86_reg
{
	//8 bit
	AL=0,
	CL,
	DL,
	BL,
	AH,
	CH,
	DH,
	BH,

	//16 bit
	AX=0,
	CX,
	DX,
	BX,
	SP,
	BP,
	SI,
	DI,

	//32 bit
	EAX=0,
	ECX,
	EDX,
	EBX,
	ESP,
	EBP,
	ESI,
	EDI,
	
	

	//XMM (SSE)
	XMM0=0,
	XMM1,
	XMM2,
	XMM3,
	XMM4,
	XMM5,
	XMM6,
	XMM7,

	//mmx (? will it be suported by the emitter?) -> propably no , SSE2 mailny replaces em w/ integer XMM math
	MM0=0,
	MM1,
	MM2,
	MM3,
	MM4,
	MM5,
	MM6,
	MM7,

	//misc :p
	REG_NONE,
	REG_ERROR,
};
#define x86_sse_reg x86_reg
#define x86_gpr_reg x86_reg

//define it here cus we use it on label type ;)
class x86_block;
// a label
struct x86_Label
{
	u32 target_opcode;
	x86_block* owner;
	bool marked;
};
//An empty type that we will use as ptr type.This is ptr-reference
struct x86_ptr
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
struct x86_ptr_imm
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
	mod_SIB,		//[reg1*scale+reg2], reg2 can be reg_none , reg1 can't
	mod_SIB_disp	//[(reg1*scale+reg2)+disp], reg2 can be reg_none , reg1 can't
};
enum x86_sib_scale
{
	sib_scale_1,
	sib_scale_2,
	sib_scale_4,
	sib_scale_8
};
//shit
struct x86_mrm
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
class x86_block
{
private:
	vector<x86_Label*> labels;
	void ApplyPatches(u8* base);

public:
	vector<code_patch> patches;
	u8* x86_buff;
	u32 x86_indx;
	u32 x86_size;


	~x86_block();
	void x86_buffer_ensure(u32 size);

	//init things
	void Init();

	//Generates code.if user_data is non zero , user_data_size bytes are allocated after the executable code
	//and user_data is set to the first byte of em.Allways 16 byte alligned
	void* Generate(void** user_data,u32 user_data_size);

	//Will free any used resources exept generated code
	void Free();

	//Label related code
	//NOTE : Label position in mem must not chainge
	void CreateLabel(x86_Label* lbl,bool mark);
	//Allocate a label and create it :).Will be delete'd when calling free and/or dtor
	x86_Label* CreateLabel(bool mark);
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
