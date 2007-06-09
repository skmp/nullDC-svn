//"ver"    -> get ndc version string
//"blocks [all|pcall|ptime|pslow] {##}" : get block list , all gets all blocks (## is ignored)
// pcall/time/slow -> get most called/most time consuming/slower (sh4/x86 cycles)
//"pclear" -> clear profiler data
//"blockinfo [id|addr] ##" -> get block info for block id=## or addr=##

//basic types
typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

#pragma pack(push,1)
struct nullprof_block_info
{
	u32 addr;
	void* sh4_code;
	void* x86_code;
	u32 sh4_bytes;
	u32 x86_bytes;
	u32 sh4_cycles;
	u64 time;
	u32 calls;
};

struct nullprof_blocklist
{
	nullprof_block_info* blocks;
	u32 count;
};
#pragma pack(pop)

typedef void nullprof_GetBlockFP(nullprof_block_info* to,u32 type,u32 address);
#define ALL_BLOCKS 0
#define PCALL_BLOCKS 1
#define PTIME_BLOCKS 2
#define PSLOW_BLOCKS 3

typedef void nullprof_GetBlocksFP(nullprof_blocklist* to, u32 type,u32 count);
typedef void nullprof_ClearBlockPdataFP();

struct nullprof_prof_pointers
{
	char ndc_ver[512];
	nullprof_GetBlockFP* GetBlockInfo;
	nullprof_GetBlocksFP* GetBlocks;
	nullprof_ClearBlockPdataFP* ClearPdata;
};


typedef void InitProfillerFP(nullprof_prof_pointers* pif);
extern bool nullprof_enabled;

extern nullprof_prof_pointers null_prof_pointers;

void InitnullProf();
void __fastcall dyna_profile_block_enter();
void __fastcall dyna_profile_block_exit_BasicBlock(NullProfInfo* np);