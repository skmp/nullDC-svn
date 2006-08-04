#pragma once
#include "recompiler.h"
#include "recompiler.h"

class CodeRegion
{
public:
	//start pc
	u32 start;
	
	//end pc
	u32 end;

	//cycle count
	u32 cycles;

	bool OnRam();

	u32 Size()
	{
		return end-start+2;
	}

	u32 OpcodeCount()
	{
		return Size()>>1;
	}

	//start page , olny valid if in ram
	u32 page_start();
	//end page , olny valid if in ram
	u32 page_end();
};

class CompiledBasicBlock:public CodeRegion
{
public :
	BasicBlockEP* Code;				//compiled code ptr
	
	//needed for lookups
	u32 cpu_mode_tag;
	u32 lookups;	//count of lookups for this block

	//needed for free() // maby not ?
	u32 size;			//compiled code size (bytes)

	
	//Addresses to blocks
	u32 TF_next_addr;//tfalse or jmp
	u32 TT_next_addr;//ttrue  or rts guess

	//pointers to blocks
	CompiledBasicBlock* TF_block;
	CompiledBasicBlock* TT_block;

	//pointers to block entry points [isnt that the same as above ?]
	void* pTF_next_addr;//tfalse or jmp
	void* pTT_next_addr;//ttrue  or rts guess

	//can be avoided
	bool Discarded;

	//misc profile & debug variables
	u64 profile_time;
	u32 profile_calls;

	//Functions
	void Free();
	void Suspend();
	void BlockWasSuspended(CompiledBasicBlock* block);
	void AddRef(CompiledBasicBlock* block);
	void ClearBlock(CompiledBasicBlock* block);

private :
	//Block link info
	vector<CompiledBasicBlock*> blocks_to_clear;
};

//helpers
#define GET_CURRENT_FPU_MODE() (fpscr.PR_SZ)

#define BLOCKLIST_MAX_CYCLES (448)
class BasicBlock;



class BasicBlock: public CodeRegion
{
	public :

	BasicBlock(CodeRegion& cregion):CodeRegion(cregion)
	{
		flags.full=0;
		cBB=0;
		TF_next_addr=0xFFFFFFFF;
		TT_next_addr=0xFFFFFFFF;
	}

	BasicBlock()
	{
		start=0;
		end=0;
		flags.full=0;
		cycles=0;
		cBB=0;
		TF_next_addr=0xFFFFFFFF;
		TT_next_addr=0xFFFFFFFF;
	}

	bool IsMemLocked(u32 adr);

	
	//u32 start;
	//end pc
	//u32 end;

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
			u32 HasDelaySlot:1;
		};
	}flags;	//compiled block flags :)

	//u32 cycles;

	shil_stream ilst;

	void CreateCompiledBlock()
	{
		cBB= new CompiledBasicBlock();

		cBB->start=start;
		cBB->TF_next_addr=TF_next_addr;
		cBB->TT_next_addr=TT_next_addr;
		cBB->cycles=cycles;
		cBB->end=end;
		cBB->cpu_mode_tag=flags.FpuMode;
		cBB->lookups=0;

		cBB->TF_block=cBB->TT_block=0;
		cBB->profile_time=0;
		cBB->profile_calls=0;
		cBB->Discarded=false;
	}

	CompiledBasicBlock* cBB;

	bool Contains(u32 pc);

	u32 TF_next_addr;//tfalse or jmp
	u32 TT_next_addr;//ttrue  or rts guess
};


typedef void (__fastcall RecOpCallFP) (u32 op,u32 pc,BasicBlock* bb);