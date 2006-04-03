#include "shil.h"

//IL and related code

INLINE bool shil_opcode::ReadsReg(Sh4RegType reg)
{
	return true;
}
INLINE bool shil_opcode::OverwritesReg(Sh4RegType reg)
{
	return true;
}
INLINE bool shil_opcode::UpdatesReg(Sh4RegType reg)
{
	return true;
}

void shil_stream::emit(shil_opcodes op,u16 flags,u32 source,u32 dest)
{
}

void shil_stream::movRtoR(Sh4RegType from,Sh4RegType to)
{
}

void shil_stream::movRtoRM(Sh4RegType from,Sh4RegType to)
{
}

void shil_stream::movRMtoR(Sh4RegType from,Sh4RegType to)
{
}

void shil_stream::movItoR(u32 from,Sh4RegType to)
{
}



void shil_stream::cmpRtoR(Sh4RegType from,Sh4RegType to)
{
}


void shil_stream::cmpRtoI(Sh4RegType from,s8 to)
{
}


void shil_stream::cmpRtoI(Sh4RegType from,u8 to)
{
}

