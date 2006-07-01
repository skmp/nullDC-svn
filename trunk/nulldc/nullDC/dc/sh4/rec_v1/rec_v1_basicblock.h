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

//helpers
#define GET_CURRENT_FPU_MODE() (fpscr.PR_SZ)

#define BLOCKLIST_MAX_CYCLES (448)
class rec_v1_BasicBlock;

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
		flags.full=0;
		cycles=0;
		compiled=0;
		TF_next_addr=0xFFFFFFFF;
		TT_next_addr=0xFFFFFFFF;
		TF_block=TT_block=0;
		profile_time=0;
		profile_calls=0;
		Discarded=false;
	}	

	void rec_v1_BasicBlock::RemoveCaller(rec_v1_BasicBlock* bb);
	void rec_v1_BasicBlock::RemoveCallee(rec_v1_BasicBlock* bb);

	void Free();
	void Suspend();
	bool Contains(u32 pc);
	//we get called by bb
	void AddCaller(rec_v1_BasicBlock* bb);
	//we call bb
	void AddCallee(rec_v1_BasicBlock* bb);
	//Find a calle
	rec_v1_BasicBlock* FindCallee(u32 address);

	//start pc
	u32 start;
	//end pc
	u32 end;
	u16 cpu_mode_tag;

	//flags
	union
	{
		u32 full;
		struct 
		{
			#define BLOCK_EXITTYPE_DYNAMIC			(0)		//link end
			#define BLOCK_EXITTYPE_FIXED			(1)		//call TF_next_addr
			#define BLOCK_EXITTYPE_COND_0			(2)		//T==0
			#define BLOCK_EXITTYPE_COND_1			(3)		//T==1
			#define BLOCK_EXITTYPE_FIXED_CALL		(4)		//Set rv to known for rts handling
			#define BLOCK_EXITTYPE_DYNAMIC_CALL		(5)		//Set rv to known for rts handling
			#define BLOCK_EXITTYPE_RET				(6)		//Ends w/ ret ;)
			#define BLOCK_EXITTYPE_RES_2			(7)		//Reserved
			u32 ExitType:3;
			//flags
			//#define BLOCK_TYPE_FPU_MASK		(3<<10)

			#define BLOCK_FPUMODE_32_S32	(0)	//32 bit math , 32 bit reads/writes
			#define BLOCK_FPUMODE_64_S32	(1)	//64 bit math , 32 bit reads/writes 
			#define BLOCK_FPUMODE_32_S64	(2)	//32 bit math , 64 bit read/writes
			#define BLOCK_FPUMODE_INVALID	(3)	//this mode is invalid
			u32 FpuMode:2;

			u32 FpuIsVector:1;


			#define BLOCK_SOM_NONE		(0)			//NONE
			#define BLOCK_SOM_SIZE_128	(1)			//DIV32U[Q|R]/DIV32S[Q|R]
			#define BLOCK_SOM_RESERVED1	(2)			//RESERVED
			#define BLOCK_SOM_RESERVED2	(3)			//RESERVED
			u32 SynthOpcode:2;

			u32 EndAnalyse:1;


			#define BLOCK_PROTECTIONTYPE_LOCK	(0)	//block checks are done my locking memory (no extra code needed)
			#define BLOCK_PROTECTIONTYPE_MANUAL	(1)	//block checks if it's valid itself
			u32 ProtectionType:1;
		};
	}flags;	//compiled block flags :)

	u32 cycles;
	u32 lookups;	//count of lookups for this block

	shil_stream ilst;

	rec_v1_CompiledBlock* compiled;

	u32 TF_next_addr;//tfalse or jmp
	u32 TT_next_addr;//ttrue  or rts guess

	//pointers to blocks
	void* pTF_next_addr;//tfalse or jmp
	void* pTT_next_addr;//ttrue  or rts guess

	vector<rec_v1_BasicBlock*> callees;
	bool Discarded;
	u64 profile_time;
	u32 profile_calls;
};


typedef void (__fastcall RecOpCallFP) (u32 op,u32 pc,rec_v1_BasicBlock* bb);