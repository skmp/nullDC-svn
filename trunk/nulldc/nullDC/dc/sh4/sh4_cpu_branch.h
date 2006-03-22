

//braf <REG_N>                  
 sh4op(i0000_nnnn_0010_0011)
{
	u32 n = GetN(op);
	u32 newpc = r[n] + pc + 2;//pc +2 is done after
	ExecuteDelayslot();
	pc = newpc;
} 


//bsrf <REG_N>                  
 sh4op(i0000_nnnn_0000_0011)
{
	u32 n = GetN(op);
	u32 newpc = r[n] + pc +2;//pc +2 is done after
	pr = pc + 4;		   //after delayslot
	ExecuteDelayslot();
	pc = newpc;

	AddCall(pr-4,pr,pc,0);
} 


 //rte                           
 sh4op(i0000_0000_0010_1011)
{
	//iNimp("rte");
	sr.SetFull(ssr);
	UpdateSR();
	u32 newpc = spc;//+2 is added after instruction
	ExecuteDelayslot();
	pc = newpc -2;
	RemoveCall(spc,1);
} 


//rts                           
 sh4op(i0000_0000_0000_1011)
{
	//TODO Check new delay slot code [28/1/06]
	u32 newpc=pr;//+2 is added after instruction
	ExecuteDelayslot();
	pc=newpc-2;
	RemoveCall(pr,0);
} 


 //
// 1xxx

//
//	2xxx

 //
// 3xxx



//
// 4xxx

 //
// 5xxx

 //
// 6xxx

 //
// 7xxx

 //
// 8xxx


// bf <bdisp8>                   
 sh4op(i1000_1011_iiii_iiii)
{//ToDo : Check Me [26/4/05]  | Check DELAY SLOT [28/1/06]
	if (sr.T==0)
	{
		//direct jump
		pc = (u32)((GetSImm8(op))*2 + 4 + pc );
		pc-=2;
	}
}


// bf.s <bdisp8>                 
 sh4op(i1000_1111_iiii_iiii)
{//TODO : Check This [26/4/05] | Check DELAY SLOT [28/1/06]
	if (sr.T==0)
	{
		//delay 1 instruction
		u32 newpc=(u32) ( (GetSImm8(op)<<1) + pc+2);//pc+2 is done after
		ExecuteDelayslot();
		pc = newpc;
	}
}


// bt <bdisp8>                   
 sh4op(i1000_1001_iiii_iiii)
{//TODO : Check This [26/4/05]  | Check DELAY SLOT [28/1/06]
	if (sr.T==1)
	{
		//direct jump
		pc = (u32) ( (GetSImm8(op)<<1) + pc+2);
	}
}


// bt.s <bdisp8>                 
 sh4op(i1000_1101_iiii_iiii)
{
	if (sr.T == 1)
	{//TODO : Check This [26/4/05]  | Check DELAY SLOT [28/1/06]
		//delay 1 instruction
		u32 newpc=(u32) ( (GetSImm8(op)<<1) + pc+2);//pc+2 is done after
		ExecuteDelayslot();
		pc = newpc;
	}
}





//
// Axxx
// bra <bdisp12>
sh4op(i1010_iiii_iiii_iiii)
{//ToDo : Check Me [26/4/05] | Check ExecuteDelayslot [28/1/06] 
	//delay 1 jump imm12
	u32 newpc =(u32) ((  ((s16)((GetImm12(op))<<4)) >>3)  + pc + 4);//(s16<<4,>>4(-1*2))
	ExecuteDelayslot();
	pc=newpc-2;
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
	AddCall(pc,pr,newpc,0);
	ExecuteDelayslot();
	pc=newpc-2;
	//	CallStackTrace.cstAddCall(pc, pr, delayslot, CallType.Normal);
	//pc_funct = 2;//jump delay 1
}

//
// Cxxx
// trapa #<imm>                  
sh4op(i1100_0011_iiii_iiii)
{
	CCN_TRA = (GetImm8(op) << 2);
	Do_Exeption(0,0x160,0x100);
}



#include "../../gui/emuWinUI.h"
sh4op(sh4_bpt_op)
{
	sh4_cpu->Stop();
	pc-=2;//hehe
	RefreshDebugger(NULL);//need fixup
}