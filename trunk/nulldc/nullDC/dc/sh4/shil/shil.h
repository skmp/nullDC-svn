#pragma once
#include "types.h"
#include "dc\sh4\sh4_if.h"

#pragma pack (push,1)
//16 byte il representation
struct shil_opcode
{
	//opcode info
	u16 opcode;
	u16 flags;
	//flags
	u16 flags_ex;

	u32 source;
	u32 dest;

	u8 source_ex;
	u8 dest_ex;

	INLINE bool ReadsReg(Sh4RegType reg);
	INLINE bool OverwritesReg(Sh4RegType reg);
	INLINE bool UpdatesReg(Sh4RegType reg);
};

#pragma pack (pop)

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
	//reg2reg , reg2ram , ram2reg , reg2const , ram2const
	mov=0,

	add,
	sub,
	
	fadd,
	fsub,
	fmul,
	fdiv,
	fmac
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
	CF=1
};
class shil_stream
{
	void emit(shil_opcodes op,u16 flags,u32 source,u32 dest);
public :
	
	void mov(Sh4RegType to,Sh4RegType from);
	void mov(Sh4RegType to,u32 from);

	/*** Mem reads ***/
	//readmem [reg]
	void readm8(Sh4RegType to,Sh4RegType from,bool sx);
	void readm16(Sh4RegType to,Sh4RegType from,bool sx);
	void readm32(Sh4RegType to,Sh4RegType from);
	void readm64(Sh4RegType to,Sh4RegType from);

	//readmem [const]
	void readm8(Sh4RegType to,u32 from,bool sx);
	void readm16(Sh4RegType to,u32  from,bool sx);
	void readm32(Sh4RegType to,u32  from);

	//readmem base[offset]
	void readm8(Sh4RegType to,Sh4RegType base,Sh4RegType offset,bool sx);
	void readm16(Sh4RegType to,Sh4RegType base,Sh4RegType offset,bool sx);
	void readm32(Sh4RegType to,Sh4RegType base,Sh4RegType offset);
	void readm64(Sh4RegType to,Sh4RegType base,Sh4RegType offset);

	/*** Mem writes ***/
	//readmem base[const]
	void readm8(Sh4RegType to,Sh4RegType base,u32 offset,bool sx);
	void readm16(Sh4RegType to,Sh4RegType base,u32 offset,bool sx);
	void readm32(Sh4RegType to,Sh4RegType base,u32 offset);

	//writemem [reg]
	void writem8(Sh4RegType from,Sh4RegType to,bool sx);
	void writem16(Sh4RegType from,Sh4RegType to,bool sx);
	void writem32(Sh4RegType from,Sh4RegType to);
	void writem64(Sh4RegType from,Sh4RegType to);

	//writemem [const]
	void writem8(Sh4RegType from,u32 to,bool sx);
	void writem16(Sh4RegType from,u32  to,bool sx);
	void writem32(Sh4RegType from,u32  to);

	//writemem base[offset]
	void writem8(Sh4RegType from,Sh4RegType base,Sh4RegType offset,bool sx);
	void writem16(Sh4RegType from,Sh4RegType base,Sh4RegType offset,bool sx);
	void writem32(Sh4RegType from,Sh4RegType base,Sh4RegType offset);
	void writem64(Sh4RegType from,Sh4RegType base,Sh4RegType offset);

	//writemem base[const]
	void writem8(Sh4RegType from,Sh4RegType base,u32 offset,bool sx);
	void writem16(Sh4RegType from,Sh4RegType base,u32 offset,bool sx);
	void writem32(Sh4RegType from,Sh4RegType base,u32 offset);

	
	void cmp(Sh4RegType to,Sh4RegType from);
	void cmp(Sh4RegType to,s8 from);
	void cmp(Sh4RegType to,u8 from);

	void test(Sh4RegType to,Sh4RegType from);
	void test(Sh4RegType to,u8 from);

	void SaveT(cmd_cond cond);
	void LoadT(x86_flags to);


	void dec(Sh4RegType to);
	void inc(Sh4RegType to);
	void neg(Sh4RegType to);
	void not(Sh4RegType to);

	//logical shifts
	void shl(Sh4RegType to,u8 count);
	void shr(Sh4RegType to,u8 count);

	//arithmetic shifts
	void sal(Sh4RegType to,u8 count);//<- is this used ?
	void sar(Sh4RegType to,u8 count);

	//rotate

	void rcl(Sh4RegType to,u8 count);
	void rcr(Sh4RegType to,u8 count);
	void rol(Sh4RegType to,u8 count);
	void ror(Sh4RegType to,u8 count);

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
};