#pragma once
#include "types.h"
#include "codespan.h"
#include "recompiler.h"

#define COMPILED_BASIC_BLOCK 0
#define COMPILED_SUPER_BLOCK 1

#define COMPILED_BLOCK_TYPE_MASK 0xFF
#define COMPILED_BLOCK_NULLPROF 0x100
#define COMPILED_BLOCK_HOTSPOT 0x200

//YAY , compiled block fun ;)

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
	union
	{
		struct
		{
			u32 type:8;
			u32 nullProf:1;
			u32 HotSpot:1;
			u32 ProtectionType:1;
			u32 exit_type:8;
		};
		u32 full;
	} block_type;

	//needed for free()/debug info
	u32 size;			//compiled code size (bytes)

	//can be avoided
	bool Discarded;

	//For TBP :)
	u32 tbp_ticks;

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
	u32 TF_next_addr;//tfalse or jmp or jmp guess
	u32 TT_next_addr;//ttrue  or rts guess

	//pointers to blocks
	CompiledBlockInfo* TF_block;
	CompiledBlockInfo* TT_block;

	//pointers to block entry points [isnt that the same as above ?-> not anymore]
	void* pTF_next_addr;//tfalse or jmp or jmp guess
	void* pTT_next_addr;//ttrue  or rts guess

	u32 RewriteOffset;
	u8 RewriteType;
	u8 LastRewrite;
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

//
void DeleteBlock(CompiledBlockInfo* block);
CompiledBlockInfo* CreateBlock(u32 type);