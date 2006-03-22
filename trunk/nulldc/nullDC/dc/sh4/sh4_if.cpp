#include "..\..\types.h"
#include "sh4_if.h"
#include "sh4_interpreter.h"

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
	return 0;
}

void Release_Sh4If(sh4_if* cpu)
{
	free(cpu);
}