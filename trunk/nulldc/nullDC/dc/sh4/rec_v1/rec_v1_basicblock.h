#pragma once
#include "rec_v1_recompiler.h"

class rec_v1_BasicBlock;

class rec_v1_CompiledBlock
{
public :
	rec_v1_BasicBlockEP* Code;						//compiled code ptr
	rec_v1_BasicBlock* parent;		//basic block that the compiled code is for
	u32 size;						//compiled code size (bytes)
	u32 count;						//compiled code opcode count
};

//flags
#define BLOCK_TYPE_MASK				(7<<0)
#define BLOCK_TYPE_DYNAMIC			(0<<0)		//link end
#define BLOCK_TYPE_FIXED			(1<<0)		//call TF_next_addr
#define BLOCK_TYPE_COND_0			(2<<0)		//T==0
#define BLOCK_TYPE_COND_1			(3<<0)		//T==1
#define BLOCK_TYPE_FIXED_CALL		(4<<0)		//Set rv to known for rts handling
#define BLOCK_TYPE_DYNAMIC_CALL		(5<<0)		//Set rv to known for rts handling
#define BLOCK_TYPE_RET				(6<<0)		//Ends w/ ret ;)
#define BLOCK_TYPE_RES_2			(7<<0)		//Reserved

#define GET_CURRENT_FPU_MODE() (fpscr.PR_SZ<<10)
#define BLOCK_TYPE_FPU_MASK		(3<<10)
#define BLOCK_TYPE_FPU32_S32	(0<<10)	//32 bit math , 32 bit reads/writes
#define BLOCK_TYPE_FPU64_S32	(1<<10)	//64 bit math , 32 bit reads/writes 
#define BLOCK_TYPE_FPU32_S64	(2<<10)	//32 bit math , 64 bit read/writes
#define BLOCK_TYPE_FPU_INVALID	(3<<10)	//this mode is invalid

#define BLOCK_TYPE_VECTOR		(1<<12)	//this block contains vector opcodes (fipr/ftrv)

#define BLOCK_SOM_MASK		(3<<13)			//synthesyssed opcode mask
#define BLOCK_SOM_NONE		(0<<13)			//NONE
#define BLOCK_SOM_SIZE_128	(1<<13)			//DIV32U[Q|R]/DIV32S[Q|R]
#define BLOCK_SOM_RESERVED1	(2<<13)			//RESERVED
#define BLOCK_SOM_RESERVED2	(3<<13)			//RESERVED

#define BLOCK_ATSC_END		(1<<15)			//end  analyse [analyse olny flag]


#define BLOCKLIST_MAX_CYCLES (448)
class rec_v1_BasicBlock
{
	vector<rec_v1_BasicBlock*> callers;
	public :

	rec_v1_BasicBlock* TF_block;
	rec_v1_BasicBlock* TT_block;
	rec_v1_BasicBlock()
	{
		start=0;
		end=0;
		flags=0;
		cycles=0;
		compiled=0;
		TF_next_addr=0xFFFFFFFF;
		TT_next_addr=0xFFFFFFFF;
		TF_block=TT_block=0;
	}	

	void Discard();
	bool Contains(u32 pc);
	//we get called by bb
	void AddRef(rec_v1_BasicBlock* bb);

	//start pc
	u32 start;
	//end pc
	u32 end;
	u16 cpu_mode_tag;

	//flags
	u16 flags;	//compiled block flags :)
	u32 cycles;
	u32 lookups;	//count of lookups for this block

	shil_stream ilst;

	rec_v1_CompiledBlock* compiled;

	u32 TF_next_addr;//tfalse or jmp
	u32 TT_next_addr;//ttrue  or rts guess

	//pointers to blocks
	void* pTF_next_addr;//tfalse or jmp
	void* pTT_next_addr;//ttrue  or rts guess
};

typedef void (__fastcall RecOpCallFP) (u32 op,u32 pc,rec_v1_BasicBlock* bb);