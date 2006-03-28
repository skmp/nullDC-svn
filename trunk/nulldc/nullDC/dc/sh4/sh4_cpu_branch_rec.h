//All branches have to be rewriten for the recompiler

//
// Axxx
// bra <bdisp12>
sh4op(i1010_iiii_iiii_iiii)
{//ToDo : Check Me [26/4/05] | Check ExecuteDelayslot [28/1/06] 
	//delay 1 jump imm12
	u32 newpc =(u32) ((  ((s16)((GetImm12(op))<<4)) >>3)  + pc + 4);//(s16<<4,>>4(-1*2))
	ExecuteDelayslot();
	SetRecPC(newpc-2);
}
//
// Bxxx
// bsr <bdisp12>
sh4op(i1011_iiii_iiii_iiii)
{//ToDo : Check Me [26/4/05] | Check new delay slot code [28/1/06]
	//iNimp("bsr <bdisp12>");
	u32 disp = GetImm12(op);
	pr = pc + 4;
	//delay 1 opcode
	u32 newpc = (u32)((((s16)(disp<<4)) >> 3) + pc + 4);
	//AddCall(pc,pr,newpc,0);
	ExecuteDelayslot();
	SetRecPC(newpc-2);
	//	CallStackTrace.cstAddCall(pc, pr, delayslot, CallType.Normal);
	//pc_funct = 2;//jump delay 1
}
