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

#include "emitter/shil_compile_slow.h"

#include "recompiler.h"
#include "blockmanager.h"
#include "analyser.h"

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
void*  __fastcall CompileCode_SuperBlock(u32 pc)
{
	sb_count++;
	printf("Zuperblock @ pc=0x%X , %d superblocks , %d%% of all blocks\n",pc,sb_count,sb_count*100/nb_count);
	CompiledBasicBlock* cBB=FindBlock(pc);
	cBB->bpm_ticks=0xFFFFFFFF;
	return 0;
}

CompiledBasicBlock*  __fastcall CompileCode(u32 pc)
{
	nb_count++;
	CompiledBasicBlock* cblock;
	BasicBlock* block=new BasicBlock();
	//scan code
	ScanCode(pc,block);
	//Fill block lock type info
	FillBlockLockInfo(block);
	//analyse code [generate il/block type]
	AnalyseCode(block);
	//Compile code
	CompileBasicBlock_slow(block);
	RegisterBlock(cblock=block->cBB);
	delete block;
	//compile code
	//return pointer
	return cblock;
}
INLINE CompiledBasicBlock* __fastcall GetRecompiledCode(u32 pc)
{
	CompiledBasicBlock* cblock=FindBlock(pc);
	
	if (cblock)
		return cblock;
	else
		return CompileCode(pc);
}

CompiledBasicBlock* FindOrRecompileCode(u32 pc)
{
	
	return GetRecompiledCode(pc);
}

//#define PROFILE_DYNAREC
#ifdef PROFILE_DYNAREC
u64 calls=0;
u64 total_cycles=0;
extern u64 ifb_calls;
#endif

#ifdef PROFILE_DYNAREC_CALL
void naked __fastcall DoRunCode(void * code)
{
#ifdef X86
	__asm
	{
		push esi;
		push edi;
		push ebx;
		push ebp;

		call ecx;

		pop ebp;
		pop ebx;
		pop edi;
		pop esi;
		ret;
	}
#endif
}
#endif

void DynaPrintLinkStart();
void DynaPrintLinkEnd();
u32 rec_cycles=0;
u32 THREADCALL rec_sh4_int_ThreadEntry(void* ptar)
{
	//just cast it
	ThreadCallbackFP* ptr=(ThreadCallbackFP*) ptar;

	ptr(true);//call the callback to init
	
	
//	pExitBlock=0;
	rec_cycles=0;
	SetFloatStatusReg();
	while(true)
	{
		CompiledBasicBlock* currBlock=GetRecompiledCode(pc);
		//rec_cycles+=currBlock->cycles;
		BasicBlockEP* fp=currBlock->Code;
		
#ifdef X86
		//call block :)
#ifndef PROFILE_DYNAREC_CALL
		//DynaPrintLinkStart();
		__asm
		{
			push esi;
			push edi
			push ebx
			push ebp
		}

		fp();


		__asm 
		{
			pop ebp
			pop ebx
			pop edi
			pop esi;
		}

		//DynaPrintLinkEnd();
#else	
		//call it using a helper function
		//so we can profile :)
		__asm
		{
			mov ecx,fp;
			call DoRunCode;
		}
#endif
#endif

#ifdef PROFILE_DYNAREC
		calls++;
		if ((calls & (0x80000-1))==(0x80000-1))//more ?
		{
			//void PrintSortedBlocks(FILE* to,u32 count);
			printf("Dynarec Stats : average link cycles : %d \n",(u32)(total_cycles/calls));
			//PrintSortedBlocks(stdout,40);
			//printf("Dynarec Stats : average link cycles : %d , ifb opcodes : %d%% [%d]\n",(u32)(total_cycles/calls),(u32)(ifb_calls*100/total_cycles),ifb_calls);
			calls=total_cycles=0;//ifb_calls
			//printprofile();
		}
#endif

		if (rec_cycles>(CPU_TIMESLICE*9/10))
		{
			UpdateSystem(rec_cycles);
#ifdef PROFILE_DYNAREC
			total_cycles+=rec_cycles;
#endif
			if (rec_sh4_int_bCpuRun==false)
				break;
			rec_cycles=0;
		}
	}
	ptr(false);//call the callback

	return 0;
}


//setup the SEH handler here so it doesnt fuq us (vc realy likes not to optimise SEH enabled functions)
u32 THREADCALL rec_sh4_int_ThreadEntry_stub(void* ptar)
{
	__try
	{
		return rec_sh4_int_ThreadEntry(ptar);
	}
	__except( ExeptionHandler( GetExceptionCode(), (GetExceptionInformation())->ExceptionRecord ) )
	{

	}

	return 0;
}

//interface
void rec_Sh4_int_Run(ThreadCallbackFP* tcb)
{
	if (rec_sh4_int_thr_handle)
	{
		printf("recSh4_int_Run: Cpu allready running\n");
	}
	else
	{
		rec_sh4_int_bCpuRun=true;
		rec_sh4_int_thr_handle=new cThread(rec_sh4_int_ThreadEntry_stub,tcb);

		if (rec_sh4_int_thr_handle==0)
		{
			printf("recSh4_int_Run: Thread creation failed\n");
		}
		rec_sh4_int_thr_handle->Start();
	}
}

void rec_Sh4_int_Stop()
{
	if (rec_sh4_int_bCpuRun)
	{
		rec_sh4_int_bCpuRun=false;
		//wait for thread to exit
		rec_sh4_int_thr_handle->WaitToEnd((u32)-1);
		delete rec_sh4_int_thr_handle;
		rec_sh4_int_thr_handle=0;
	}
	if (rec_sh4_int_thr_handle)
	{
		delete rec_sh4_int_thr_handle;
		rec_sh4_int_thr_handle=0;
	}
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
