
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
#define CPU_RATIO		(8)

//uh uh 
volatile bool  sh4_int_bCpuRun=false;
cThread* sh4_int_thr_handle=0;

u32 exec_cycles=0;
time_t odtime=0;

u32 opcode_fam_cycles[0x10]=
{
 CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,
 CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO,CPU_RATIO
};

u32 sh4_ex_ExeptionCode,sh4_ex_VectorAddress;
Sh4RegContext sh4_ex_SRC;

void sh4_int_restore_reg_cnt()
{
	//restore reg context
	LoadSh4Regs(&sh4_ex_SRC);
	//raise exeption
	RaiseExeption(sh4_ex_ExeptionCode,sh4_ex_VectorAddress);
	sh4_exept_raised=false;
}
void naked sh4_int_exept_hook()
{
	__asm 
	{
		call sh4_int_restore_reg_cnt;
		jmp [sh4_exept_next];
	}
}

void __fastcall sh4_int_RaiseExeption(u32 ExeptionCode,u32 VectorAddress)
{
	//if (sh4_exept_raised)
	//	return;
	verify(sh4_exept_raised==false);
	sh4_exept_raised=true;
	*sh4_exept_ssp=(u32)sh4_int_exept_hook;

	sh4_ex_ExeptionCode=ExeptionCode;
	sh4_ex_VectorAddress=VectorAddress;

	//save reg context
	SaveSh4Regs(&sh4_ex_SRC);
}

u32 THREADCALL sh4_int_ThreadEntry_code(void* ptar)
{
	//just cast it
	ThreadCallbackFP* ptr=(ThreadCallbackFP*) ptar;

	ptr(true);//call the callback to init

	__asm
	{
		//save regs used
		push esi;

		//for exeption rollback
		mov sh4_exept_ssp,esp;		//esp wont chainge after that :)
		sub sh4_exept_ssp,4;		//point to next stack item :)
		mov sh4_exept_next,offset i_exept_rp;

		//init vars
		mov esi,CPU_TIMESLICE;  //cycle count = max

		//loop start
		i_mainloop:

		//run a single opcode -- doesnt use _any_ stack space :D
		{
i_run_opcode:
			mov ecx , pc;			//param #1 for readmem16
			call IReadMem16;		//ax has opcode to execute now
			movzx eax,ax;			//zero extend to 32b
			
			mov ecx,eax;			//ecx=opcode (param 1 to opcode handler)
			call OpPtr[eax*4];		//call opcode handler

			add pc,2;				//pc+=2 -> goto next opcode
			
			
			sub esi,CPU_RATIO;		//remove cycles from cycle count
			jns i_run_opcode;		//jump not (esi>0) , inner loop til timeslice is executed
		}
		
		//exeption rollback point
		//if an exeption happened , resume execution here
i_exept_rp:

		//update system  and run a new timeslice
		xor eax,eax;			//zero eax [used later]

		//esi is 0 or negative here
		//cc = CPU_TIMESLICE - esi
		//cc =  (-esi) + CPU_TIMESLICE
		//we want cc on ecx (to call update)

		mov ecx,esi;			//save leftover cycles
		mov esi,CPU_TIMESLICE;	//reset cycle count

		neg ecx;				//ecx=-ecx , now we have leftover as positive
		add ecx,esi;			//add ecx+=CPU_TIMESLICE , so we have total cycles on ecx
		
		//take in acount delay slots now
		add ecx,exec_cycles;	//add cycles to ecx
		mov exec_cycles,eax;	//zero out cycles [rember ? we zero'd out eax lots ago :p]

		//ecx : passed cycles
		//esi : new cycle count , inited
		call UpdateSystem;

		//if cpu still on go for one more brust of opcodes :)
		cmp  sh4_int_bCpuRun,0;
		jne i_mainloop;

i_exit_mainloop:
		//restore regs used
		pop esi;
	}


	ptr(false);//call the callback
	sh4_int_bCpuRun=false;
	return 0;
}
//setup the SEH handler here so it doesnt fuq us (vc realy likes not to optimise SEH enabled functions)
u32 THREADCALL sh4_int_ThreadEntry(void* ptar)
{
	__try
	{
		return sh4_int_ThreadEntry_code(ptar);
	}
	__except( ExeptionHandler( GetExceptionCode(), (GetExceptionInformation())->ExceptionRecord ) )
	{

	}

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

		memset(r,0,sizeof(r));
		memset(r_bank,0,sizeof(r_bank));

		gbr=ssr=spc=sgr=dbr=vbr=0;
		mach=macl=pr=fpul=0;

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
	Sh4_int_Stop();
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
	exec_cycles+=CPU_RATIO;

	pc+=2;
	u32 op=ReadMem16(pc);
	ExecuteOpcode(op);

	return true;
}

#include "ccn.h"

//General update
//u32 gdCnt=0;
u32 aica_cycl=0;
void FreeSuspendedBlocks();;
void DynaPrintCycles();
u32 shitaaa=0;

u32 gcp_timer=0;
u32 gpc_counter=0;
#define cpu_ratio 100
int __fastcall UpdateSystem(u32 Cycles)
{
	Cycles=Cycles*100/cpu_ratio;
	//TODO : Add Update System implementation
	aica_cycl+=Cycles;
	if (aica_cycl>(200*1000*1000/(44100*3)))
	{
		UpdateAica(aica_cycl);

		//~15k cycles
		gpc_counter++;
		if (gpc_counter>10)
		{
			gpc_counter=0;
			gcp_timer++;
			FreeSuspendedBlocks();
		}
		/*if (gpc_counter &1)
		{
			if (CCN_MMUCR.AT)
			{
				u32 rv=CCN_MMUCR.URB;
				rv=rv==0?63:rv;
				CCN_MMUCR.URC=fastrand() % rv;
			}
		}*/

		/*if (shitaaa==0x10000)
		{
			shitaaa=0;
			DynaPrintCycles();
		}
		shitaaa++;*/
		aica_cycl=0;
	}
	UpdateTMU(Cycles);
	UpdatePvr(Cycles);
	return UpdateINTC();
}