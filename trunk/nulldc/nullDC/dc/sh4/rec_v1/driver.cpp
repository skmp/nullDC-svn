#include "types.h"
#include <windows.h>

#include "dc/sh4/sh4_interpreter.h"
#include "dc/sh4/sh4_opcode_list.h"
#include "dc/sh4/sh4_registers.h"
#include "dc/sh4/sh4_if.h"
#include "dc/pvr/pvr_if.h"
#include "dc/aica/aica_if.h"
#include "dc/gdrom/gdrom_if.h"
#include "dc/sh4/intc.h"
#include "dc/sh4/tmu.h"
#include "dc/sh4/sh4_cst.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/shil/shil_ce.h"

#include "emitter/emitter.h"

#include "recompiler.h"
#include "blockmanager.h"
#include "analyser.h"
#include "superblock.h"
#include "nullprof.h"

#include <time.h>
#include <float.h>

#define CPU_TIMESLICE	(BLOCKLIST_MAX_CYCLES)

//uh uh 
volatile bool  rec_sh4_int_bCpuRun=false;
cThread* rec_sh4_int_thr_handle=0;

u32 rec_exec_cycles=0;
time_t rec_odtime=0;

u32 avg_rat=0;
u32 avg_rat_cnt=0;
u32 avg_bc=0;

u32 sb_count=0;
u32 nb_count=0;
u32 rec_cycles=0;

void* Dynarec_Mainloop_no_update;
void* Dynarec_Mainloop_do_update;

void DynaPrintLinkStart();
void DynaPrintLinkEnd();
void DumpBlockMappings();

CompiledBlockInfo*  __fastcall CompileCode(u32 pc)
{
	nb_count++;
	CompiledBlockInfo* cblock;
	BasicBlock* block=new BasicBlock();
	//scan code
	ScanCode(pc,block);
	//Fill block lock type info
	block->CalculateLockFlags();
	//analyse code [generate il/block type]
	AnalyseCode(block);
	//optimise
	shil_optimise_pass_ce_driver(block);
	//Compile code
	bool reset_blocks=!block->Compile();
	RegisterBlock(cblock=&block->cBB->cbi);
	delete block;
	//compile code
	//return pointer
	if (reset_blocks)
	{
		_SuspendAllBlocks();
		printf("Compile failed -- retrying\n");
		return CompileCode(pc);//retry
	}
	
	return cblock;
}
BasicBlockEP* __fastcall CompileCodePtr()
{
	return CompileCode(pc)->Code;
}
INLINE BasicBlockEP * __fastcall GetRecompiledCodePointer(u32 pc)
{
	return FindCode(pc);
}

CompiledBlockInfo* FindOrRecompileBlock(u32 pc)
{
	CompiledBlockInfo* cblock=FindBlock(pc);
	
	if (cblock)
		return cblock;
	else
		return CompileCode(pc);
}

void naked CompileAndRunCode()
{
	__asm 
	{
		call CompileCodePtr;
		jmp eax;
	}
}

void __fastcall rec_sh4_int_RaiseExeption(u32 ExeptionCode,u32 VectorAddress)
{
}
void rec_sh4_ResetCache()
{
	SuspendAllBlocks();
}
//asm version
BasicBlockEP* __fastcall FindCode_full(u32 address,CompiledBlockInfo* fastblock);

extern u32 fast_lookups;
u32 old_esp;
u32 Dynarec_Mainloop_no_update_fast;

void naked DynaMainLoop()
{
	__asm
	{
		//save corrupted regs
		push esi;
		push edi;
		push ebx;
		push ebp;

		mov old_esp,esp;

		//Allocate the ret cache table on stack
		//format :
		
		//[table] <- 'on'=1 , 'off' = 1 , 'index'=0
		//[waste] <- 'on'=1 , 'off' = 0 , 'index'=0

		sub esp,RET_CACHE_SZ*4-1;	//allign to RET_CACHE_SZ*4 
		and esp,~(RET_CACHE_SZ*4-1);	//(1kb for 32 entry cache) <- now , we are '00'
		sub esp,RET_CACHE_SZ*2;		//<- now we are '10'

		//now store it !
		mov ret_cache_base,esp;
		add ret_cache_base,RET_CACHE_SZ;	//pointer to the table base ;)

		//Reset it !
		call ret_cache_reset;

		//misc pointers needed
		mov block_stack_pointer,esp;
		mov Dynarec_Mainloop_no_update,offset no_update;
		mov Dynarec_Mainloop_no_update_fast,offset no_update;
		mov Dynarec_Mainloop_do_update,offset do_update;
		//Max cycle count :)
		mov rec_cycles,(CPU_TIMESLICE*9/10);

		jmp no_update;

		//some utility code :)

		//partial , faster lookup. When we know the mode hasnt changed :)
fast_lookup:

		align 16
no_update:
		/*
		//called if no update is needed
		
		call GetRecompiledCodePointer;
		*/
		
		/*
		#define LOOKUP_HASH_SIZE	0x4000
		#define LOOKUP_HASH_MASK	(LOOKUP_HASH_SIZE-1)
		#define GetLookupHash(addr) ((addr>>2)&LOOKUP_HASH_MASK)

		*/
		/*
		CompiledBlockInfo* fastblock;
		fastblock=BlockLookupGuess[GetLookupHash(address)];
		*/
		mov ecx,pc;
		mov edx,ecx;
		and edx,(LOOKUP_HASH_MASK<<2);

		mov edx,[BlockLookupGuess + edx]
		

		/*
		if ((fastblock->start==address) && 
			(fastblock->cpu_mode_tag ==fpscr.PR_SZ)
			)
		{
			fastblock->lookups++;
			return fastblock->Code;
		}*/
		
		mov eax,fpscr;
		cmp [edx],ecx;
		jne full_lookup;
		
		shr eax,19;
		and eax,0x3;
		cmp [edx+12],eax;
		jne full_lookup;
		add dword ptr[edx+16],1;
#ifdef _BM_CACHE_STATS
		add fast_lookups,1;
#endif
		jmp dword ptr[edx+8];
		/*
		else
		{
			return FindCode_full(address,fastblock);
		}*/
full_lookup:
		call FindCode_full
		jmp eax;

do_update:
		//called if update is needed
		mov ecx,CPU_TIMESLICE;
		add rec_cycles,ecx;
		//ecx=cycles
		call UpdateSystem;

		//check for exit
		cmp rec_sh4_int_bCpuRun,0;
		jne no_update;

		//exit from function
		mov esp,old_esp;
		pop ebp;
		pop ebx;
		pop edi;
		pop esi;
		ret;
	}
}
/*
u32 THREADCALL rec_sh4_int_ThreadEntry(void* ptar)
{
	//just cast it
	//ThreadCallbackFP* ptr=(ThreadCallbackFP*) ptar;

	//ptr(true);//call the callback to init
	

	//ptr(false);//call the callback

	return 0;
}
//setup the SEH handler here so it doesnt fuq us (vc realy likes not to optimise SEH enabled functions)
u32 THREADCALL rec_sh4_int_ThreadEntry_stub(void* ptar)
{
	//GenerateMainLoop();
	__try
	{
		return rec_sh4_int_ThreadEntry(ptar);
	}
	__except( ExeptionHandler( GetExceptionCode(), (GetExceptionInformation()) ) )
	{

	}

	return 0;
}
*/
//interface
void rec_Sh4_int_Run()
{
	rec_sh4_int_bCpuRun=true;
	rec_cycles=0;
	SetFloatStatusReg();
	DynaMainLoop();
}

void rec_Sh4_int_Stop()
{
	if (rec_sh4_int_bCpuRun)
	{
		rec_sh4_int_bCpuRun=false;
		//wait for thread to exit
		//rec_sh4_int_thr_handle->WaitToEnd((u32)-1);
		//delete rec_sh4_int_thr_handle;
		//rec_sh4_int_thr_handle=0;
	}
	//if (rec_sh4_int_thr_handle)
	//{
	//	delete rec_sh4_int_thr_handle;
	//	rec_sh4_int_thr_handle=0;
	//}
}

void rec_Sh4_int_Step() 
{
	if (rec_sh4_int_bCpuRun)
	{
		printf("recSh4 Is running , can't step\n");
	}
	else
	{
		u32 op=ReadMem16(pc);
		ExecuteOpcode(op);
		pc+=2;
	}
}

void rec_Sh4_int_Skip() 
{
	if (rec_sh4_int_bCpuRun)
	{
		printf("recSh4 Is running , can't Skip\n");
	}
	else
	{
		pc+=2;
	}
}

void rec_Sh4_int_Reset(bool Manual) 
{
	if (rec_sh4_int_bCpuRun)
	{
		printf("Sh4 Is running , can't Reset\n");
	}
	else
	{
		Sh4_int_Reset(Manual);

		ResetAnalyser();
		ResetBlockManager();
		
		//Any more registers have default value ?
		printf("recSh4 Reset\n");
	}
}

void rec_Sh4_int_Init() 
{
	//InitHash();
	Sh4_int_Init();
	InitAnalyser();
	InitBlockManager();

	ResetAnalyser();
	ResetBlockManager();

	InitnullProf();
	printf("recSh4 Init\n");
}

void rec_Sh4_int_Term() 
{
	TermBlockManager();
	TermAnalyser();
	Sh4_int_Term();
	printf("recSh4 Term\n");
}

bool rec_Sh4_int_IsCpuRunning() 
{
	return rec_sh4_int_bCpuRun;
}
