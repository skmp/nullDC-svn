#include "types.h"
#include "sh4_if.h"
#include "sh4_interpreter.h"
#include "rec_v1\driver.h"

extern u8 GetSingleFromDouble(u8 dbl)
{
	if (dbl>=Sh4RegType::dr_0 && dbl<=Sh4RegType::dr_7)
	{
		u8 res=dbl-Sh4RegType::dr_0;
		return (Sh4RegType::fr_0+(res<<1));
	}
	else if (dbl>=Sh4RegType::xd_0 && dbl<=Sh4RegType::xd_7)
	{
		u8 res=dbl-Sh4RegType::xd_0;
		return (Sh4RegType::xf_0+(res<<1));
	}

	printf("GetSingleFromDouble : WRONG ID %X\n",dbl);

	return Sh4RegType::reg_xmtrx;//error :P
}

bool IsReg64(Sh4RegType reg)
{
	if (reg>=dr_0 && reg<=dr_7)
		return true;

	if (reg>=xd_0 && reg<=xd_7)
		return true;

	return false;
}
//what to put here ?
//Ahh i know i know :P

//Get an interface to sh4 interpreter
sh4_if* Get_Sh4Interpreter()
{
	sh4_if* rv=(sh4_if*)malloc(sizeof(sh4_if));
	
	rv->Run=Sh4_int_Run;
	rv->Stop=Sh4_int_Stop;
	rv->Step=Sh4_int_Step;
	rv->Skip=Sh4_int_Skip;
	rv->Reset=Sh4_int_Reset;
	rv->Init=Sh4_int_Init;
	rv->Term=Sh4_int_Term;
	rv->IsCpuRunning=Sh4_int_IsCpuRunning;
	rv->GetRegister=Sh4_int_GetRegister;
	rv->SetRegister=Sh4_int_SetRegister;
	rv->RaiseInterrupt=RaiseInterrupt;
	return rv;
}

//Get an interface to sh4 dynarec
sh4_if* Get_Sh4Recompiler()
{
	sh4_if* rv=(sh4_if*)malloc(sizeof(sh4_if));
	
	rv->Run=rec_Sh4_int_Run;
	rv->Stop=rec_Sh4_int_Stop;
	rv->Step=rec_Sh4_int_Step;
	rv->Skip=rec_Sh4_int_Skip;
	rv->Reset=rec_Sh4_int_Reset;
	rv->Init=rec_Sh4_int_Init;
	rv->Term=rec_Sh4_int_Term;
	rv->IsCpuRunning=rec_Sh4_int_IsCpuRunning;
	rv->GetRegister=Sh4_int_GetRegister;
	rv->SetRegister=Sh4_int_SetRegister;
	rv->RaiseInterrupt=RaiseInterrupt;
	return rv;
}

void Release_Sh4If(sh4_if* cpu)
{
	free(cpu);
}