#pragma once
#include "recompiler.h"
#define COMPILED_BASIC_BLOCK 0
#define COMPILED_SUPER_BLOCK 1

#define COMPILED_BLOCK_TYPE_MASK 0xFF
#define COMPILED_BLOCK_NULLPROF 0x100
#define COMPILED_BLOCK_HOTSPOT 0x200

struct CodeSpan
{
public:
	//start pc
	u32 start;

	//end pc
	u32 end;

	//True if block is on ram :)
	bool OnRam();

	//Retuns the size of the code span (in bytes)
	u32 Size();
	//Returns the opcode count of the code span
	u32 OpcodeCount();
	//Returns the Page count of the code span [olny valid if in vram]
	u32 PageCount();
	//start page , olny valid if in ram
	u32 page_start();
	//end page , olny valid if in ram
	u32 page_end();

	//Checks if this CodeSpan contains an adress.Valid olny if on block is on ram and address is on ram :)
	bool Contains(u32 address,u32 sz);
	//checks if this CodeSpan contains another Codespan
	bool Contains(CodeSpan* to);
	//Checks if this CodeSpan contains an adress , counting in page units.
	bool ContainsPage(u32 address);
	//Checks if this CodeSpan contains another code span , counting in page units.
	bool ContainsPage(CodeSpan* to);

	//Creates a Union w/ the CodeSpan , assuming this CodeSpan is on ram.The to codespan contains decoded ram offsets
	void Union(CodeSpan* to);
	//Checks if this CodeSpan Intersects w/ another , assuming both are on ram
	bool Intersect(CodeSpan* to);
};

struct CodeRegion : public CodeSpan
{
public:
	//cycle count
	u32 cycles;
};

//YAY , compiled block fun ;)
//mmhhzztt

struct HotSpotInfo;
struct NullProfInfo;
struct CompiledBasicBlockInfo;
struct CompiledSuperBlockInfo;

//Generic block info
//All Compiled Block structs contain this first
struct CompiledBlockInfo:CodeSpan
{
public :
	BasicBlockEP* Code;				//compiled code ptr
	
	//needed for lookups
	u32 cpu_mode_tag;
	u32 lookups;	//count of lookups for this block

	//block type
	u32 block_type;

	//needed for free()/debug info
	u32 size;			//compiled code size (bytes)

	//can be avoided
	bool Discarded;

	//Called to Free :p yeshrly
	void Free();
	//Called when this block is suspended
	void Suspend();
	//Called when a block we reference is suspended
	void BlockWasSuspended(CompiledBlockInfo* block);
	//Called when a block adds reference to this
	void AddRef(CompiledBlockInfo* block);
	//remote pthis reference to block *warning* it was the oposite before
	void ClearBlock(CompiledBlockInfo* block);

	//Get Hotspot info (on Hotspot blocks)
	HotSpotInfo* GetHS();
	//Get NullProf info (on NullProf blocks)
	NullProfInfo* GetNP();

	//Get BasicBlock info (on BasicBlock blocks)
	CompiledBasicBlockInfo* GetBB();
	//Get SuperBlock info (on SuperBlock blocks)
	CompiledSuperBlockInfo* GetSP();
};

/////////////////////////////////////////////////
//Block TYPES:
//Block Type : BasicBlock
struct CompiledBasicBlockInfo
{	
	//Addresses to blocks
	u32 TF_next_addr;//tfalse or jmp
	u32 TT_next_addr;//ttrue  or rts guess

	//pointers to blocks
	CompiledBlockInfo* TF_block;
	CompiledBlockInfo* TT_block;

	//pointers to block entry points [isnt that the same as above ?-> not anymore]
	void* pTF_next_addr;//tfalse or jmp
	void* pTT_next_addr;//ttrue  or rts guess

	//Block link info
	vector<CompiledBlockInfo*> blocks_to_clear;
};
//Block Type : SuperBlock
struct CompiledSuperBlockInfo
{	
	void FillInfo(CompiledBlockInfo* to);
};

/////////////////////////////////////////////////
//Block MODS
//Block Mod : HotSpot
struct HotSpotInfo
{
	//profile timer value on last reset
	u32 gcp_lasttimer;
	//count time timer , when it reaches 0 a reset on gcp_lasttimer is made
	u32 bpm_ticks;
};
//Block Mod : NullProf
struct NullProfInfo
{
	u64 time;
	u32 calls;
	u32 cycles;
};
/////////////////////////////////////////////////
//Block type structs ;)
#pragma warning( disable : 4003)
#define BLOCKSTRUCT(name,eBlockInfo) struct name {CompiledBlockInfo cbi;eBlockInfo ebi;HOTSPOTINFO;NULLPROFINFO;};

#define ALLBLOCKSTRUCTS(namemod) BLOCKSTRUCT(CompiledBasicBlock##namemod,CompiledBasicBlockInfo)\
								 BLOCKSTRUCT(CompiledSuperBlock##namemod,CompiledSuperBlockInfo)

//NON HS , NON NP
#define HOTSPOTINFO
#define NULLPROFINFO
ALLBLOCKSTRUCTS( );

//HS , NON NP
#undef HOTSPOTINFO
#define HOTSPOTINFO HotSpotInfo hs
ALLBLOCKSTRUCTS(HotSpot);

//HS , NP
#undef NULLPROFINFO
#define NULLPROFINFO NullProfInfo np
ALLBLOCKSTRUCTS(HotSpotNullProf);

//NON HS , NP
#undef HOTSPOTINFO
#define HOTSPOTINFO

ALLBLOCKSTRUCTS(NullProf);

#pragma warning( default : 4003)

//helpers
#define GET_CURRENT_FPU_MODE() (fpscr.PR_SZ)

#define BLOCKLIST_MAX_CYCLES (448)

class BasicBlock: public CodeRegion
{
	public :

	vector<u32> locked;
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
			#define BLOCK_EXITTYPE_COND				(2)		//T==0 -> TT , else TF
			#define BLOCK_EXITTYPE_FIXED_CALL		(3)		//Set rv to known for rts handling
			#define BLOCK_EXITTYPE_DYNAMIC_CALL		(4)		//Set rv to known for rts handling
			#define BLOCK_EXITTYPE_RET				(5)		//Ends w/ ret ;)
			#define BLOCK_EXITTYPE_FIXED_CSC		(6)		//Fixed , but we cant link due to cpu state change :)
			#define BLOCK_EXITTYPE_RES_2			(7)		//Reserved
			u32 ExitType:3;
			//flags

			#define BLOCK_FPUMODE_32_S32	(0)	//32 bit math , 32 bit reads/writes
			#define BLOCK_FPUMODE_64_S32	(1)	//64 bit math , 32 bit reads/writes 
			#define BLOCK_FPUMODE_32_S64	(2)	//32 bit math , 64 bit read/writes
			#define BLOCK_FPUMODE_INVALID	(3)	//this mode is invalid
			u32 FpuMode:2;

			u32 FpuIsVector:1;

			#define BLOCK_PROTECTIONTYPE_LOCK	(0)	//block checks are done by locking memory (no extra code needed)
			#define BLOCK_PROTECTIONTYPE_MANUAL	(1)	//block checks if it's valid itself
			u32 ProtectionType:1;

			u32 HasDelaySlot:1;

			//If set , this block wont be profiled and as a result it can't become a superblock 
			u32 DisableHS:1;

			u32 PerformModeLookup:1;

			//Analyse flags :)
			#define BLOCK_SOM_NONE		(0)			//NONE
			#define BLOCK_SOM_SIZE_128	(1)			//DIV32U[Q|R]/DIV32S[Q|R]
			#define BLOCK_SOM_RESERVED1	(2)			//RESERVED
			#define BLOCK_SOM_RESERVED2	(3)			//RESERVED
			u32 SynthOpcode:2;

			u32 EndAnalyse:1;
		};
	}flags;	//compiled block flags :)

	//u32 cycles;

	shil_stream ilst;

	void SetCompiledBlockInfo(CompiledBasicBlock* cBl);

	CompiledBasicBlock* cBB;

	u32 TF_next_addr;//tfalse or jmp
	u32 TT_next_addr;//ttrue  or rts guess
};


typedef void (__fastcall RecOpCallFP) (u32 op,u32 pc,BasicBlock* bb);
void DeleteBlock(CompiledBlockInfo* block);
CompiledBlockInfo* CreateBlock(u32 type);