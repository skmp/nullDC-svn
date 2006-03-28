
#include "types.h"
#include <windows.h>

#include "sh4_interpreter.h"
#include "sh4_opcode_list.h"
#include "sh4_registers.h"
#include "sh4_if.h"
#include "dc/pvr/pvr_if.h"
#include "dc/aica/aica_if.h"
#include "dc/gdrom/gdrom_if.h"
#include "intc.h"
#include "tmu.h"
#include "sh4_cst.h"
#include "dc/mem/sh4_mem.h"
#include "gdb_stub/gdb_stub.h"

#include <time.h>
#include <float.h>

#define CPU_TIMESLICE	(448)
#define CPU_RATIO		(1)

//uh uh 
volatile bool  sh4_int_bCpuRun=false;
cThread* sh4_int_thr_handle=0;

u32 exec_cycles=0;
time_t odtime=0;

u32 opcode_fam_cycles[0x10]=
{
 CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,
 CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,1
};

u32 THREADCALL sh4_int_ThreadEntry(void* ptar)
{

	//just cast it
	ThreadCallbackFP* ptr=(ThreadCallbackFP*) ptar;

	ptr(true);//call the callback to init
	u32 i=0;
	while(sh4_int_bCpuRun)
	{
		if (fpscr.RM)
			_controlfp( _RC_DOWN, _MCW_RC );//round to 0
		else
			_controlfp( _RC_NEAR, _MCW_RC );//round to nearest

		//for ( int i=0;i<CPU_TIMESLICE;i++)
		while(i<CPU_TIMESLICE*CPU_RATIO)
		{
			GDB_BOOT_TEST();

			u32 op=ReadMem16(pc);
			i+=opcode_fam_cycles[op>>12];
			if ((pc&0xFFFFFFF)==0xC00B6BC-2)
				pc=pc;
			ExecuteOpcode(op);
			pc+=2;
			
		}

		UpdateSystem(i);
		i=0;
		/*
		cycles++;
		if (cycles>200*1000*1000)
		{
			time_t now=time(0);
			printf("Cycl:%d , time : %d , speed %f\n",cycles,now-odtime,(float)((double)cycles/(double)(now-odtime))/1000000.0);
			cycles=0;
			odtime=now;
		}*/
	}
	ptr(false);//call the callback

	return 0;
}


//interface
void Sh4_int_Run(ThreadCallbackFP* tcb)
{
	if (sh4_int_thr_handle)
	{
		printf("Sh4_int_Run: Cpu allready running\n");
	}
	else
	{
		sh4_int_bCpuRun=true;
		sh4_int_thr_handle=new cThread(sh4_int_ThreadEntry,tcb);

		if (sh4_int_thr_handle==0)
		{
			printf("Sh4_int_Run: Thread creation failed\n");
		}
		sh4_int_thr_handle->Start();
	}
}

void Sh4_int_Stop()
{
	if (sh4_int_bCpuRun)
	{
		sh4_int_bCpuRun=false;
		//wait for thread to exit
		sh4_int_thr_handle->WaitToEnd((u32)-1);
		delete sh4_int_thr_handle;
		sh4_int_thr_handle=0;
	}
	if (sh4_int_thr_handle)
	{
		delete sh4_int_thr_handle;
		sh4_int_thr_handle=0;
	}
}

void Sh4_int_Step() 
{
	if (sh4_int_bCpuRun)
	{
		printf("Sh4 Is running , can't step\n");
	}
	else
	{
		u32 op=ReadMem16(pc);
		ExecuteOpcode(op);
		pc+=2;
	}
}

void Sh4_int_Skip() 
{
	if (sh4_int_bCpuRun)
	{
		printf("Sh4 Is running , can't Skip\n");
	}
	else
	{
		pc+=2;
	}
}

void Sh4_int_Reset(bool Manual) 
{
	if (sh4_int_bCpuRun)
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
		printf("Sh4 Reset\n");
	}
}

void Sh4_int_Init() 
{
	BuildOpcodeTables();
	printf("Sh4 Init\n");
}

void Sh4_int_Term() 
{
	printf("Sh4 Term\n");
}

bool Sh4_int_IsCpuRunning() 
{
	return sh4_int_bCpuRun;
}

u32 Sh4_int_GetRegister(Sh4RegType reg)
{
	if ((reg>=r0) && (reg<=r15))
	{
		return r[reg-r0];
	}
	else if ((reg>=r0_Bank) && (reg<=r7_Bank))
	{
		return r_bank[reg-r0_Bank];
	}
	else if ((reg>=fr_0) && (reg<=fr_15))
	{
		return fr_hex[reg-fr_0];
	}
	else if ((reg>=xf_0) && (reg<=xf_15))
	{
		return xf_hex[reg-xf_0];
	}
	else
	{
		switch(reg)
		{
		case Sh4RegType::reg_gbr :
			return gbr;
			break;
		case Sh4RegType::reg_vbr :
			return vbr;
			break;

		case Sh4RegType::reg_ssr :
			return ssr;
			break;

		case Sh4RegType::reg_spc :
			return spc;
			break;

		case Sh4RegType::reg_sgr :
			return sgr;
			break;

		case Sh4RegType::reg_dbr :
			return dbr;
			break;

		case Sh4RegType::reg_mach :
			return mach;
			break;

		case Sh4RegType::reg_macl :
			return macl;
			break;

		case Sh4RegType::reg_pr :
			return pr;
			break;

		case Sh4RegType::reg_fpul :
			return fpul;
			break;


		case Sh4RegType::reg_pc :
			return pc;
			break;

		case Sh4RegType::reg_sr :
			return sr.GetFull();
			break;
		case Sh4RegType::reg_fpscr :
			return fpscr.full;
			break;


		default:
			EMUERROR2("Unkown register Id %d",reg);
			return 0;
			break;
		}
	}
}


void Sh4_int_SetRegister(Sh4RegType reg,u32 regdata)
{
	if (reg<=Sh4RegType::r15)
	{
		r[reg]=regdata;
	}
	else if (reg<=Sh4RegType::r7_Bank)
	{
		r_bank[reg-16]=regdata;
	}
	else
	{
		switch(reg)
		{
		case Sh4RegType::reg_gbr :
			gbr=regdata;
			break;

		case Sh4RegType::reg_ssr :
			ssr=regdata;
			break;

		case Sh4RegType::reg_spc :
			spc=regdata;
			break;

		case Sh4RegType::reg_sgr :
			sgr=regdata;
			break;

		case Sh4RegType::reg_dbr :
			dbr=regdata;
			break;

		case Sh4RegType::reg_mach :
			mach=regdata;
			break;

		case Sh4RegType::reg_macl :
			macl=regdata;
			break;

		case Sh4RegType::reg_pr :
			pr=regdata;
			break;

		case Sh4RegType::reg_fpul :
			fpul=regdata;
			break;


		case Sh4RegType::reg_pc :
			pc=regdata;
			break;
		case Sh4RegType::reg_sr :
			sr.SetFull(regdata);
			UpdateSR();
			break;
		case Sh4RegType::reg_fpscr :
			fpscr.full=regdata;
			UpdateFPSCR();
			break;


		default:
			EMUERROR2("Unkown register Id %d",reg);
			break;
		}
	}
}







//more coke .. err code
//TODO : Check for valid delayslot instruction
bool ExecuteDelayslot()
{
	exec_cycles++;

	pc+=2;
	u32 op=ReadMem16(pc);
	ExecuteOpcode(op);

	return true;
}


//General update
//u32 gdCnt=0;
int UpdateSystem(u32 Cycles)
{
	//TODO : Add Update System implementation

	UpdateAica(Cycles);
	UpdateTMU(Cycles);
	UpdatePvr(Cycles);
	return UpdateINTC();
}