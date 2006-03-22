#pragma once
#include "..\..\types.h"

void AddCall(u32 pc_callstart,u32 pc_callretadr,u32 branchaddr,u32 calltype);
void RemoveCall(u32 retaddress,u32 rettype);
void GetCallStackText(char* buff);

struct cst_entry
{
	u32 pc_callstart;	//pc , when the call was made (eg , points to jsr)
	u32 pc_callretadr;	//return address , usualy pc_callstart+2/pc_callstart+4
	u32 branchaddr;		//address to jump
	u32 calltype;		//0=normal call , 1 = Interrupt
};