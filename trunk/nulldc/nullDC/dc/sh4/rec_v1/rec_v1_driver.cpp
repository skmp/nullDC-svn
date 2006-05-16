
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

#include "rec_v1_recompiler.h"
#include "rec_v1_blockmanager.h"
#include "rec_v1_analyser.h"

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
//recBlock* lastBlock;


INLINE rec_v1_BasicBlock* __fastcall GetRecompiledCode(u32 pc)
{
	rec_v1_BasicBlock* block=rec_v1_FindBlock(pc);
	
	if (block)
		return block;
	else
	{
		block=rec_v1_NewBlock(pc);
		//analyse code
		rec_v1_AnalyseCode(pc,block);
		rec_v1_RegisterBlock(block);
		CompileBasicBlock_slow(block);
		//compile code
		//return pointer
		return block;
	}
}
/*
rec_v1_BasicBlock* rec_v1_FindOrAnalyse(u32 pc)
{
	rec_v1_BasicBlock* block=rec_v1_FindBlock(pc);
	
	if (block)
		return block;
	else
	{
		block=rec_v1_AddBlock(pc);
		//analyse code
		rec_v1_AnalyseCode(pc,block);
		//compile code
		//return pointer
		return block;
	}
}*/
rec_v1_BasicBlock* rec_v1_FindOrRecompileCode(u32 pc)
{
	return GetRecompiledCode(pc);
}

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


u32 rec_cycles=0;
u32 THREADCALL rec_sh4_int_ThreadEntry(void* ptar)
{
	//just cast it
	ThreadCallbackFP* ptr=(ThreadCallbackFP*) ptar;

	ptr(true);//call the callback to init
	
	
//	pExitBlock=0;
	rec_cycles=0;
	SetFloatStatusReg();
	while(rec_sh4_int_bCpuRun)
	{
		rec_v1_BasicBlock* currBlock=GetRecompiledCode(pc);
		//rec_cycles+=currBlock->cycles;
		rec_v1_BasicBlockEP* fp=currBlock->compiled->Code;

#ifdef X86
		//call block :)
#ifndef PROFILE_DYNAREC_CALL
		__asm
		{
			push esi;
			push edi
			push ebx
			push ebp

			call fp;

			pop ebp
			pop ebx
			pop edi
			pop esi;
		}
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
		total_cycles+=rec_cycles;
		if ((calls & (0x8000-1))==(0x8000-1))//more ?
		{
			printf("Dynarec Stats : average link cycles : %d , ifb opcodes : %d%% [%d]\n",(u32)(total_cycles/calls),(u32)(ifb_calls*100/total_cycles),ifb_calls);
			calls=total_cycles=ifb_calls=0;
			printprofile();
		}
#endif

		if (rec_cycles>(CPU_TIMESLICE*0.9f))
		{
			if (rec_cycles>CPU_TIMESLICE*2)
			{
				printf("rec_cycles>CPU_TIMESLICE*2 !!!\n");
				if (rec_cycles>CPU_TIMESLICE*3)
					printf("rec_cycles>CPU_TIMESLICE*3 !!!\n");
				printf("rec_cycles=%d\n",rec_cycles);
			}

//			if (pExitBlock->Discarded)
			//	pExitBlock=0;
			if (UpdateSystem(rec_cycles))
			{
				//pExitBlock=0;
			}
			rec_cycles=0;
			//SetFloatStatusReg();
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
		pc = 0xA0000000;

		sr.SetFull(0x700000F0);
		old_sr=sr;
		UpdateSR();

		fpscr.full = 0x0004001;
		old_fpscr=fpscr;
		UpdateFPSCR();
		
		//Any more registers have default value ?
		printf("recSh4 Reset\n");
	}
}

void rec_Sh4_int_Init() 
{
	//InitHash();
	Sh4_int_Init();
	printf("recSh4 Init\n");
}

void rec_Sh4_int_Term() 
{
	printf("recSh4 Term\n");
}

bool rec_Sh4_int_IsCpuRunning() 
{
	return rec_sh4_int_bCpuRun;
}
