#pragma once
#include "types.h"
#include "dc\sh4\sh4_if.h"
#include <Vector>

using namespace std;

#pragma pack (push,1)
//16 byte il representation
struct shil_opcode
{
	shil_opcode()
	{
		opcode=0;
		flags=0;
		flags_ex=0;
		reg1=0;
		reg2=0;  
		imm1=0;
		imm2=0;
	}
	//opcode info
	u16 opcode;
	u16 flags;//-> opcode related
	
	u16 flags_ex;//flags		-> opts related

	
	u8 reg1;	//reg 
	u8 reg2;	//reg

	u32 imm1;	//imm1
	u32 imm2;	//imm2

	bool ReadsReg(Sh4RegType reg);
	bool OverwritesReg(Sh4RegType reg);
	bool UpdatesReg(Sh4RegType reg);
	bool WritesReg(Sh4RegType reg);
};

#pragma pack (pop)

//flags
enum shil_opflags
{
	FLAG_8=0,
	FLAG_16=1,
	FLAG_32=2,
	FLAG_64=3,
	
	FLAG_SX=4,
	FLAG_ZX=0,//defualt

	FLAG_REG1=8,
	FLAG_REG2=16,

	FLAG_IMM1=32,
	FLAG_IMM2=64,

	FLAG_R0=128,
	FLAG_GBR=256,
	FLAG_MACH=512,
	FLAG_MACL=1024
};

//shil is a combination of sh4 and x86 opcodes , in a decoded form so that varius optimisations
//are possible (inlcuding cost removal , dead code elimination , flag elimination , more)
//
//regiser allocation is done on the next step (shil compiler)
//shil does not make any assumtions about x86 or x64 , it's easy to generate both x64 and x86 code from it
//
//
//Code compilation flow :
//Sh4 analyser (pc,ram,status) -> superblock  tree
//shil compiler (superblock  tree) -> shil tree
//shil optimisers(shil tree)->shil tree							[can be ommited]
//binary code compiler(shil tree)-> executable code (x86 or x64)

//
//Register allocation is done on the last step  , all other opts are done on shil to shil passes
//

enum shil_opcodes
{
	//mov reg,reg	[32|64]
	//mov reg,const	[32]
	mov,

	/*** Mem reads ***/
	//readmem reg,[base+reg]	[s]			[8|16|32|64]
	//readmem reg,[base+const]	[s]			[8|16|32|64]
	readm,

	/*** Mem writes ***/
	//writemem reg,[base+reg]	[]			[8|16|32|64]
	//writemem reg,[base+const]	[]			[8|16|32|64]
	writem,
	
	//cmp reg,reg
	//cmp reg,imm [s] [8]
	cmp,

	//cmp reg,reg
	//cmp reg,imm [] [8]
	test,

	//SaveT/LoadT cond
	SaveT,
	LoadT,

	//bit shits
	//neg reg
	neg,
	//not reg
	not,

	//bitwise ops

	//and reg,reg
	//and reg,const [32]
	and,

	//and reg,reg
	//and reg,const [32]
	or,

	//and reg,reg
	//and reg,const [32]
	xor,
	

	//logical shifts

	//shl reg,const [8]
	shl,
	//shr reg,const [8]
	shr,

	//arithmetic shifts

	//sal reg,const [8]
	//sal is same as shl
	//sar reg,const [8]
	sar,

	//rotate

	//rcl reg,const [8]
	rcl,
	
	//rcr reg,const [8]
	rcr,
	
	//rol reg,const [8]
	rol,
	
	//ror reg,const [8]
	ror,

	//swaps
	//swap [16|32] reg
	swap,

	//moves w/ extend
	//signed - unsigned
	//movex reg,reg		[s] [8|16]
	//movsxb reg,reg	 s	 8
	//movsxb reg,reg	 s	 16
	//movzxb reg,reg	 z	 8
	//movzxw reg,reg	 z	 16
	movex,

	//maths (integer)
	//adc reg,reg
	adc,

	//add reg,reg
	//add reg,const
	add,

	//sub reg,reg
	//sub reg,const
	sub,

	//floating
	//basic ops

	//fadd reg,reg [32|64]
	fadd,
	//fsub reg,reg [32|64]
	fsub,
	//fmul reg,reg [32|64]
	fmul,
	//fdiv reg,reg [32|64]
	fdiv,

	
	//fabs reg [32|64]
	fabs,
	//fneg reg [32|64]
	fneg,

	//pfftt
	//fmac r0,reg,reg [32|64]
	fmac,

	//shil_ifb const , const
	shil_ifb,

	//JCond T==imm
	jcond,
	//Jmp
	jmp,
	//mul [s] [16|32|64] 16*16->32 , 32*32->32 , 32*32->64
	//mul mac[l|h|lh] [16|32|64] [s] , reg , reg
	mul,	

	shil_count,
};

enum cmd_cond
{
/* Condition codes */
CC_O			=0,				// overflow (OF=1)
 CC_NO			=1,				// not overflow (OF=0)
CC_B			=2,				// below (CF=1)
SaveCF			=CC_B,			//CF=1
CC_NAE			=CC_B,			// 
CC_NB			=3,				// above or equal (CF=0)
CC_NC			=3,				//
CC_AE			=CC_NB,			//
CC_E			=4,				// zero (ZF=1)
CC_Z			=CC_E,			//
SaveZF			=CC_Z,
CC_NE			=5,				// not zero (ZF=0)
CC_NZ			=CC_NE,			//
CC_BE			=6,				// below or equal (CF=1 or ZF=1)
CC_NA			=CC_BE,			//
CC_NBE			=7,				// above (CF=0 and ZF=0)
CC_A			=CC_NBE,			//
CC_S			=8,				// sign (SF=1)
CC_NS			=9,				// not sign (SF=0)
CC_P			=0xA,			// parity (PF=1)
CC_PE			=CC_P,			//
CC_NP			=0xB,			// not parity (PF=0)
CC_PO			=CC_NP,			//
CC_L			=0xC,			// less (SF<>OF)
CC_NGE			=CC_L,			//
CC_NL			=0xD,			// not less (SF=OF)
CC_GE			=CC_NL,			//
CC_LE			=0xE,			// less or equal (ZF=1 or SF<>OF)
CC_NG			=CC_LE,			//
CC_NLE			=0xF,			// greater (ZF=0 and SF=OF)
CC_G			=CC_NLE 		//
};

enum x86_flags
{
	CF=1,
	jcond_flag=128
};
class shil_stream
{
	u16 shil_stream::GetFloatFlags(Sh4RegType reg1,Sh4RegType reg2);
	void shil_stream::emit32(shil_opcodes op,u32 imm1);
	void shil_stream::emit32(shil_opcodes op,Sh4RegType reg1);
	void shil_stream::emit32(shil_opcodes op,Sh4RegType reg1,u32 param);
	void shil_stream::emit32(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2);
	void shil_stream::emit(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2,u32 imm1,u32 imm2,u32 flags);

	void shil_stream::emitReg(shil_opcodes op,Sh4RegType reg1,u32 flags);
	void shil_stream::emitRegImm(shil_opcodes op,Sh4RegType reg1,u32 imm1,u32 flags);
	void shil_stream::emitRegImmImm(shil_opcodes op,Sh4RegType reg1,u32 imm1,u32 imm2,u32 flags);
	void shil_stream::emitRegReg(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2,u32 flags);
	void shil_stream::emitRegRegImm(shil_opcodes op,Sh4RegType reg1,Sh4RegType  reg2,u32 imm1,u32 flags);

public :
	void shil_stream::jcond(u32 cond);
	void shil_stream::jmp();

	void shil_stream::muls_16_16_32(Sh4RegType reg1,Sh4RegType reg2);
	void shil_stream::mulu_16_16_32(Sh4RegType reg1,Sh4RegType reg2);
	void shil_stream::muls_32_32_32(Sh4RegType reg1,Sh4RegType reg2);
	void shil_stream::mulu_32_32_32(Sh4RegType reg1,Sh4RegType reg2);
	void shil_stream::muls_32_32_64(Sh4RegType reg1,Sh4RegType reg2);
	void shil_stream::mulu_32_32_64(Sh4RegType reg1,Sh4RegType reg2);

	vector<shil_opcode> opcodes;
	
	shil_stream()
	{
		op_count=0;
	}
	u32 op_count;
	void mov(Sh4RegType to,Sh4RegType from);
	void mov(Sh4RegType to,u32 from);


	/*** Mem reads ***/
	//readmem [const]
	void shil_stream::readm8(Sh4RegType to,u32 from);
	void shil_stream::readm16(Sh4RegType to,u32 from);
	void shil_stream::readm32(Sh4RegType to,u32 from);
	void shil_stream::readm64(Sh4RegType to,u32 from);

	//readmem [reg]
	void shil_stream::readm8(Sh4RegType to,Sh4RegType from);
	void shil_stream::readm16(Sh4RegType to,Sh4RegType from);
	void shil_stream::readm32(Sh4RegType to,Sh4RegType from);
	void shil_stream::readm64(Sh4RegType to,Sh4RegType from);

	//readmem base[offset]
	void shil_stream::readm8(Sh4RegType to,Sh4RegType base,Sh4RegType offset);
	void shil_stream::readm16(Sh4RegType to,Sh4RegType base,Sh4RegType offset);
	void shil_stream::readm32(Sh4RegType to,Sh4RegType base,Sh4RegType offset);
	void shil_stream::readm64(Sh4RegType to,Sh4RegType base,Sh4RegType offset);

	//readmem base[const]
	void shil_stream::readm8(Sh4RegType to,Sh4RegType base,u32 offset);
	void shil_stream::readm16(Sh4RegType to,Sh4RegType base,u32 offset);
	void shil_stream::readm32(Sh4RegType to,Sh4RegType base,u32 offset);



	/*** Mem writes ***/
	//writemem [const]
	void shil_stream::writem8(Sh4RegType from,u32 to);
	void shil_stream::writem16(Sh4RegType from,u32 to);
	void shil_stream::writem32(Sh4RegType from,u32 to);
	void shil_stream::writem64(Sh4RegType from,u32 to);

	//writemem [reg]
	void shil_stream::writem8(Sh4RegType from,Sh4RegType to);
	void shil_stream::writem16(Sh4RegType from,Sh4RegType to);
	void shil_stream::writem32(Sh4RegType from,Sh4RegType to);
	void shil_stream::writem64(Sh4RegType from,Sh4RegType to);

	//writemem reg[reg]
	void shil_stream::writem8(Sh4RegType from,Sh4RegType base,Sh4RegType offset);
	void shil_stream::writem16(Sh4RegType from,Sh4RegType base,Sh4RegType offset);
	void shil_stream::writem32(Sh4RegType from,Sh4RegType base,Sh4RegType offset);
	void shil_stream::writem64(Sh4RegType from,Sh4RegType base,Sh4RegType offset);

	//writemem reg[const]
	void shil_stream::writem8(Sh4RegType from,Sh4RegType base,u32 offset);
	void shil_stream::writem16(Sh4RegType from,Sh4RegType base,u32 offset);
	void shil_stream::writem32(Sh4RegType from,Sh4RegType base,u32 offset);

	
	void cmp(Sh4RegType to,Sh4RegType from);
	void cmp(Sh4RegType to,s8 from);

	void test(Sh4RegType to,Sh4RegType from);
	void test(Sh4RegType to,u8 from);

	void SaveT(cmd_cond cond);
	void LoadT(x86_flags to);

	//bit shits
	void dec(Sh4RegType to);
	void inc(Sh4RegType to);
	void neg(Sh4RegType to);
	void not(Sh4RegType to);

	//bitwise ops
	void and(Sh4RegType to,Sh4RegType from);
	void and(Sh4RegType to,u32 from);
	void or(Sh4RegType to,Sh4RegType from);
	void or(Sh4RegType to,u32 from);
	void xor(Sh4RegType to,Sh4RegType from);
	void xor(Sh4RegType to,u32 from);

	//logical shifts
	void shl(Sh4RegType to,u8 count);
	void shr(Sh4RegType to,u8 count);

	//arithmetic shifts
	void sal(Sh4RegType to,u8 count);//<- is this used ?
	void sar(Sh4RegType to,u8 count);

	//rotate

	void rcl(Sh4RegType to);
	void rcr(Sh4RegType to);
	void rol(Sh4RegType to);
	void ror(Sh4RegType to);

	//swaps
	void bswap(Sh4RegType to);
	void wswap(Sh4RegType to);

	//extends
	//signed
	void movsxb(Sh4RegType to,Sh4RegType from);
	void movsxw(Sh4RegType to,Sh4RegType from);
	//unsigned
	void movzxb(Sh4RegType to,Sh4RegType from);
	void movzxw(Sh4RegType to,Sh4RegType from);

	//maths (integer)
	void adc(Sh4RegType to,Sh4RegType from);

	void add(Sh4RegType to,Sh4RegType from);
	void add(Sh4RegType to,u32 from);
	void sub(Sh4RegType to,Sh4RegType from);
	void sub(Sh4RegType to,u32 from);

	//floating
	void fadd(Sh4RegType to,Sh4RegType from);
	void fsub(Sh4RegType to,Sh4RegType from);
	void fmul(Sh4RegType to,Sh4RegType from);
	void fmac(Sh4RegType to,Sh4RegType from);
	void fdiv(Sh4RegType to,Sh4RegType from);

	void fabs(Sh4RegType to);
	void fneg(Sh4RegType to);

	//misc
	void shil_ifb(u32 opcode,u32 pc);
};

char* GetShilName(shil_opcodes ops);