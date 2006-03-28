
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

#include "rec_v0_recompiler.h"
#include "block_manager.h"

#include <time.h>
#include <float.h>

#define CPU_TIMESLICE	(152)
#define CPU_RATIO		(3)

//uh uh 
volatile bool  rec_sh4_int_bCpuRun=false;
cThread* rec_sh4_int_thr_handle=0;

u32 rec_exec_cycles=0;
time_t rec_odtime=0;

u32 rec_opcode_fam_cycles[0x10]=
{
 CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,
 CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,1
};

u32 avg_rat=0;
u32 avg_rat_cnt=0;
u32 THREADCALL rec_sh4_int_ThreadEntry(void* ptar)
{

	//just cast it
	ThreadCallbackFP* ptr=(ThreadCallbackFP*) ptar;

	ptr(true);//call the callback to init
	u32 i=0;
	u32 rec_cycles;
	u32 rec_pc;
	
	while(rec_sh4_int_bCpuRun)
	{
		if (fpscr.RM)
			_controlfp( _RC_DOWN, _MCW_RC );//round to 0
		else
			_controlfp( _RC_NEAR, _MCW_RC );//round to nearest

		recBlock* currBlock;
		currBlock=FindBlock(pc);
		if (!currBlock)
		{
			currBlock=AddBlock(pc);
			//currBlock->StartAddr=pc; // redutant , allready done on add block

			rec_pc=pc;
			recStartRecompile();
			int bc=0;

			//for ( int i=0;i<CPU_TIMESLICE;i++)
			while(i<CPU_TIMESLICE*CPU_RATIO)
			{

				u32 op=ReadMem16(rec_pc);
				i+=rec_opcode_fam_cycles[op>>12];
				bc+=2;
				if (!recRecompileOp(op,rec_pc))
				{
					break;
				}
				rec_pc+=2;
			}
			recEndRecompile();
			//save info to block struct
			currBlock->Code=recGetFunction();
			currBlock->Size=bc;
			currBlock->NativeSize=recGetCodeSize();
			currBlock->Cycles=i;
			avg_rat=(avg_rat*avg_rat_cnt+ (recGetCodeSize()*100/bc));
			avg_rat_cnt++;
			avg_rat=avg_rat/(avg_rat_cnt);

			printf("Generated code bytes ratio %d:%d = %d%% , Average %d%%\n",bc, recGetCodeSize(),recGetCodeSize()*100/bc,avg_rat);
			


			bc=0;
			i=0;
		}

		rec_cycles+=currBlock->Cycles;
		currBlock->Code();
		//pc+=2 is needed after call
		pc+=2;

		if (rec_cycles>CPU_TIMESLICE*CPU_RATIO)
		{
			UpdateSystem(rec_cycles);
			rec_cycles-=CPU_TIMESLICE*CPU_RATIO;
		}
	}
	ptr(false);//call the callback

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
		rec_sh4_int_thr_handle=new cThread(rec_sh4_int_ThreadEntry,tcb);

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
