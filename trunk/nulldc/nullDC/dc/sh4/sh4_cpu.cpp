//All non fpu opcodes :)
#include "types.h"

#include <windows.h>

#include "dc/pvr/pvr_if.h"
#include "sh4_interpreter.h"
#include "dc/mem/sh4_mem.h"
#include "dc/mem/sh4_internal_reg.h"
#include "sh4_registers.h"
#include "sh4_cst.h"
#include "sh4_interpreter.h"
#include "ccn.h"
#include "intc.h"
#include "tmu.h"
#include "dc/gdrom/gdrom_if.h"



#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
#define GetImm4(str) ((str>>0) & 0xf)
#define GetImm8(str) ((str>>0) & 0xff)
#define GetSImm8(str) ((s8)((str>>0) & 0xff))
#define GetImm12(str) ((str>>0) & 0xfff)
#define GetSImm12(str) (((s16)((GetImm12(str))<<4))>>3)

#define iNimp cpu_iNimp
#define iWarn cpu_iWarn

//Read Mem macros
#define ReadMemU32(to,addr) to=ReadMem32(addr)
#define ReadMemS16(to,addr) to=(u32)(s32)(s16)ReadMem16(addr)
#define ReadMemS8(to,addr) to=(u32)(s32)(s8)ReadMem8(addr)

//Base,offset format
#define ReadMemBOU32(to,addr,offset)	ReadMemU32(to,addr+offset)
#define ReadMemBOS16(to,addr,offset)	ReadMemS16(to,addr+offset)
#define ReadMemBOS8(to,addr,offset)		ReadMemS8(to,addr+offset)

//Write Mem Macros
#define WriteMemU32(addr,data)				WriteMem32(addr,(u32)data)
#define WriteMemU16(addr,data)				WriteMem16(addr,(u16)data)
#define WriteMemU8(addr,data)				WriteMem8(addr,(u8)data)

//Base,offset format
#define WriteMemBOU32(addr,offset,data)		WriteMemU32(addr+offset,data)
#define WriteMemBOU16(addr,offset,data)		WriteMemU16(addr+offset,data)
#define WriteMemBOU8(addr,offset,data)		WriteMemU8(addr+offset,data)


bool sh4_sleeping;

// 0xxx

void cpu_iNimp(u32 op, char* info)
{
	printf("not implemented opcode : %X : ", op);
	printf(info);
	printf(" @ %X\n", pc);
	sh4_cpu->Stop();
}

void cpu_iWarn(u32 op, char* info)
{
	printf("Check opcode : %X : ", op);
	printf(info);
	printf(" @ %X\n", pc);
}

#include "sh4_cpu_movs.h"
#include "sh4_cpu_branch.h"
#include "sh4_cpu_arith.h"
#include "sh4_cpu_logic.h"


//************************ TLB/Cache ************************
//ldtlb                         
sh4op(i0000_0000_0011_1000)
{
	iNimp(op, "ldtlb");
} 



//ocbi @<REG_N>                 
sh4op(i0000_nnnn_1001_0011)
{
	//iNimp("ocbi @<REG_N> ");
} 


//ocbp @<REG_N>                 
sh4op(i0000_nnnn_1010_0011)
{
	//iNimp("ocbp @<REG_N> ");
} 


//ocbwb @<REG_N>                
sh4op(i0000_nnnn_1011_0011)
{
	//iNimp("ocbwb @<REG_N> ");
} 

//pref @<REG_N>                 
sh4op(i0000_nnnn_1000_0011)
{
	//iNimp("pref @<REG_N>");
	u32 n = GetN(op);
	u32 Dest = r[n];

	if ((Dest & 0xFC000000) == 0xE0000000) //Store Queue
	{
		//TODO : Check for enabled store queues
		u32* sq;
		u32 Address, QACR;
		if (((Dest >> 5) & 0x1) == 0)
		{
			sq = sq0_dw;
			QACR = CCN_QACR0.Area;
		}
		else
		{
			sq = sq1_dw;
			QACR = CCN_QACR1.Area;
		}

		Address = (Dest & 0x03FFFFE0) | (QACR << 26);//ie:(QACR&0x1c>>2)<<26



		if (((Address >> 26) & 0x7) == 4)//??
		{
			try
			{
				//printf("TA dlist SQ to Addr: %08X\n", Address);
				//TODO: Add pvr handling
				//				PvrLib.lib.Update(PVRU_TA_SQ,sq);
				//	PvrPlugin.PvrSQWrite(sq,1);
				libPvr->pvr_info.TADma(Address,sq,1);
			}
			catch(...){}
		}
		else
		{
			for (u32 i = 0; i < 8; i++)
				WriteMem32((Address + (i << 2)), sq[i]);
		}

	}


}







//************************ Set/Get T/S ************************
//sets                          
sh4op(i0000_0000_0101_1000)
{
	//iNimp("sets");
	sr.S = 1;
} 


//clrs                          
sh4op(i0000_0000_0100_1000)
{
	iNimp(op, "clrs");
} 

//sett                          
sh4op(i0000_0000_0001_1000)
{
	//iNimp("sett");
	sr.T = 1;
} 



//clrt                          
sh4op(i0000_0000_0000_1000)
{
	//iNimp("clrt");
	sr.T = 0;
} 
//movt <REG_N>                  
sh4op(i0000_nnnn_0010_1001)
{
	//iNimp("movt <REG_N>");
	u32 n = GetN(op);
	r[n] = sr.T;
} 
//************************ Reg Compares ************************
//cmp/pz <REG_N>                
sh4op(i0100_nnnn_0001_0001)
{//ToDo : Check This [26/4/05]
	//iNimp("cmp/pz <REG_N>");
	u32 n = GetN(op);

	if (((s32)r[n]) >= 0)
		sr.T = 1;
	else
		sr.T = 0;
}




//cmp/pl <REG_N>                
sh4op(i0100_nnnn_0001_0101)
{//TODO : !Add this
	//iNimp("cmp/pl <REG_N>");
	u32 n = GetN(op);
	if ((s32)r[n] > 0) 
		sr.T = 1;
	else 
		sr.T = 0;
}




//cmp/eq #<imm>,R0              
sh4op(i1000_1000_iiii_iiii)
{//TODO : Check This [26/4/05]
	u32 imm = (u32)(s32)(GetSImm8(op));
	if (r[0] == imm)
		sr.T =1;
	else
		sr.T =0;
}

//cmp/eq <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0000)
{
	//iNimp("cmp/eq <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	if (r[m] == r[n])
		sr.T = 1;
	else
		sr.T = 0;
}

//cmp/hs <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0010)
{//ToDo : Check Me [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	if (r[n] >= r[m])
		sr.T=1;
	else
		sr.T=0;
}

//cmp/ge <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0011)
{//TODO : Check This [26/4/05]
	//iNimp("cmp/ge <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	if ((s32)r[n] >= (s32)r[m])
		sr.T = 1;
	else 
		sr.T = 0;
}

//cmp/hi <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0110)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	if (r[n] > r[m])
		sr.T=1;
	else
		sr.T=0;
}

//cmp/gt <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0111)
{//TODO : Check This [26/4/05]
	//iNimp("cmp/gt <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	if (((s32)r[n]) > ((s32)r[m]))
		sr.T = 1;
	else 
		sr.T = 0;
}

//cmp/str <REG_M>,<REG_N>       
sh4op(i0010_nnnn_mmmm_1100)
{//TODO : Check This [26/4/05]
	//iNimp("cmp/str <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	u32 temp;
	u32 HH, HL, LH, LL;

	temp = r[n] ^ r[m];

	HH=(temp&0xFF000000)>>24;
	HL=(temp&0x00FF0000)>>16;
	LH=(temp&0x0000FF00)>>8;
	LL=temp&0x000000FF;
	HH=HH&&HL&&LH&&LL;
	if (HH==0) sr.T=1;
	else sr.T=0;
}

//tst #<imm>,R0                 
sh4op(i1100_1000_iiii_iiii)
{//TODO : Check This [26/4/05]
	//iNimp("tst #<imm>,R0");
	u32 utmp1 = r[0] & GetImm8(op);
	if (utmp1 == 0) 
		sr.T = 1;
	else 
		sr.T = 0;
}
//tst <REG_M>,<REG_N>           
sh4op(i0010_nnnn_mmmm_1000)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);

	if ((r[n] & r[m])!=0)
		sr.T=0;
	else
		sr.T=1;

}
//************************ mulls! ************************ 
//mulu.w <REG_M>,<REG_N>          
sh4op(i0010_nnnn_mmmm_1110)
{
	//iNimp("mulu.w <REG_M>,<REG_N>");//check  ++
	u32 n = GetN(op);
	u32 m = GetM(op);
	macl=((u16)(s32)r[n])*
		((u16)(s32)r[m]);
}

//muls.w <REG_M>,<REG_N>          
sh4op(i0010_nnnn_mmmm_1111)
{
	//TODO : Check This [26/4/05]
	//iNimp("muls <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	macl = (u32)(((s16)(s32)r[n]) * ((s16)(s32)r[m]));
}
//dmulu.l <REG_M>,<REG_N>       
sh4op(i0011_nnnn_mmmm_0101)
{
	//iNimp("dmulu.l <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	u64 x;

	x = (u64)r[n] * (u64)r[m];

	macl = (u32)(x & 0xFFFFFFFF);
	mach = (u32)((x >> 32) & 0xFFFFFFFF);
}

//dmuls.l <REG_M>,<REG_N>       
sh4op(i0011_nnnn_mmmm_1101)
{
	//iNimp("dmuls.l <REG_M>,<REG_N>");//check ++
	u32 n = GetN(op);
	u32 m = GetM(op);
	s64 x;

	x = (s64)(s32)r[n] * (s64)(s32)r[m];

	macl = (u32)(x & 0xFFFFFFFF);
	mach = (u32)((x >> 32) & 0xFFFFFFFF);
}


//mac.w @<REG_M>+,@<REG_N>+     
sh4op(i0100_nnnn_mmmm_1111)
{
	iNimp(op, "mac.w @<REG_M>+,@<REG_N>+");
}		
//mac.l @<REG_M>+,@<REG_N>+     
sh4op(i0000_nnnn_mmmm_1111)
{//TODO : !Add this
	//iNimp("mac.l @<REG_M>+,@<REG_N>+");
	u32 n = GetN(op);
	u32 m = GetM(op);
	s32 rm, rn;
	s64 mul, mac, result;


	rm = (s32)ReadMem32(r[m]);
	rn = (s32)ReadMem32(r[n]);


	r[m] += 4;
	r[n] += 4;

	mul = rm * rn;
	mac = (s64)(((u64)mach << 32) + macl);

	result = mac + mul;

	macl = (u32)(result & 0xFFFFFFFF);
	mach = (u32)((result >> 32) & 0xFFFFFFFF);
}

//mul.l <REG_M>,<REG_N>         
sh4op(i0000_nnnn_mmmm_0111)
{//TODO : CHECK THIS
	u32 n = GetN(op);
	u32 m = GetM(op);
	macl = (u32)((((s32)r[n]) * ((s32)r[m])));
}
//************************ Div ! ************************ 
//div0u                         
sh4op(i0000_0000_0001_1001)
{//ToDo : Check This [26/4/05]
	//iNimp("div0u");
	sr.Q = 0;
	sr.M = 0;
	sr.T = 0;
}
//div0s <REG_M>,<REG_N>         
sh4op(i0010_nnnn_mmmm_0111)
{//ToDo : Check This [26/4/05]
	//iNimp("div0s <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	//new implementation
	sr.Q=r[n]>>31;
	sr.M=r[m]>>31;
	sr.T=sr.M^sr.Q;
	return;
	/*
	if ((r[n] & 0x80000000)!=0)
	//SET_BIT(SR, SR_Q);
	sr.Q = 1;
	else
	sr.Q = 0;
	//REMOVE_BIT(SR, SR_Q);

	if ((r[m] & 0x80000000)!=0)
	//SET_BIT(SR, SR_M);
	sr.M = 1;
	else
	sr.M = 0;
	//REMOVE_BIT(SR, SR_M);

	//f ((IS_SR_Q() && IS_SR_M()) || (!IS_SR_Q() && !IS_SR_M()))
	if (sr.Q == sr.M)
	//REMOVE_BIT(SR, SR_T);
	sr.T = 0;
	else		sr.T = 1;
	//SET_BIT(SR, SR_T);*/
}

//div1 <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_0100)
{//ToDo : Check This [26/4/05]
	u32 n=GetN(op);
	u32 m=GetM(op);

	unsigned long tmp0, tmp2;
	unsigned char old_q, tmp1;

	old_q = sr.Q;
	sr.Q = (u8)((0x80000000 & r[n]) !=0);

	r[n] <<= 1;
	r[n] |= (unsigned long)sr.T;

	tmp0 = r[n];	// this need only be done once here ..
	tmp2 = r[m];

	if( 0 == old_q )
	{	if( 0 == sr.M )
	{
		r[n] -= tmp2;
		tmp1	= (r[n]>tmp0);
		sr.Q	= (sr.Q==0) ? tmp1 : (u8)(tmp1==0) ;
	}	else	{
		r[n] += tmp2;
		tmp1	=(r[n]<tmp0);
		sr.Q	= (sr.Q==0) ? (u8)(tmp1==0) : tmp1 ;
	}
	}	else
	{	if( 0 == sr.M )
	{
		r[n] += tmp2;
		tmp1	=(r[n]<tmp0);
		sr.Q	= (sr.Q==0) ? tmp1 : (u8)(tmp1==0) ;
	}	else	{
		r[n] -= tmp2;
		tmp1	=(r[n]>tmp0);
		sr.Q	= (sr.Q==0) ? (u8)(tmp1==0) : tmp1 ;
	}
	}
	sr.T = (sr.Q==sr.M);
}









//************************ Simple maths ************************ 
//addc <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1110)
{//ToDo : Check This [26/4/05]
	//iNimp("addc <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	u32 tmp1 = r[n] + r[m];
	u32 tmp0 = r[n];

	r[n] = tmp1 + sr.T;

	if (tmp0 > tmp1)
		sr.T=1;
	//SET_BIT(SR, SR_T);
	else
		sr.T = 0;
	//REMOVE_BIT(SR, SR_T);

	if (tmp1 > r[n])
		sr.T = 1;
	//SET_BIT(SR, SR_T);

}

// addv <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1111)
{
	iNimp(op, "addv <REG_M>,<REG_N>");
}

//subc <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1010)
{//ToDo : Check This [26/4/05]
	//iNimp("subc <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	u32 tmp1 = (u32)(((s32)r[n]) - ((s32)r[m]));
	u32 tmp0 = r[n];
	r[n] = tmp1 - sr.T;
	if (tmp0 < tmp1)
	{
		sr.T=1;
	}
	else
	{
		sr.T=0;
	}
	if (tmp1 < r[n])
	{
		sr.T=1;
	}
}

//subv <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1011)
{
	iNimp(op, "subv <REG_M>,<REG_N>");
}
//dt <REG_N>                    
sh4op(i0100_nnnn_0001_0000)
{
	u32 n = GetN(op);
	r[n]-=1;
	if (r[n] == 0)
		sr.T=1;
	else
		sr.T=0;
}

//negc <REG_M>,<REG_N>          
sh4op(i0110_nnnn_mmmm_1010)
{
	//iNimp("negc <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n]=-r[m]-sr.T;
	sr.T=r[n]>>31;
}


//neg <REG_M>,<REG_N>           
sh4op(i0110_nnnn_mmmm_1011)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] = -r[m];
} 

//not <REG_M>,<REG_N>           
sh4op(i0110_nnnn_mmmm_0111)
{
	//iNimp("not <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n] = ~r[m];
} 


//************************ shifts/rotates ************************
//shll <REG_N>                  
sh4op(i0100_nnnn_0000_0000)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);

	sr.T = r[n] >> 31;
	r[n] <<= 1;
}
//shal <REG_N>                  
sh4op(i0100_nnnn_0010_0000)
{
	u32 n=GetN(op);
	sr.T=r[n]>>31;
	r[n]=((s32)r[n])<<1;
	//for (;;)
	printf("shal is used , WTF\n");
	printf("shal is used , WTF\n");
	printf("shal is used , WTF\n");
	printf("shal is used , WTF\n");
}


//shlr <REG_N>                  
sh4op(i0100_nnnn_0000_0001)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);
	sr.T = r[n] & 0x1;
	r[n] >>= 1;
}

//shar <REG_N>                  
sh4op(i0100_nnnn_0010_0001)
{//ToDo : Check This [26/4/05] x2
	//iNimp("shar <REG_N>");
	u32 n = GetN(op);
//	u32 t;

	sr.T=r[n] & 1;
	r[n]=((s32)r[n])>>1;
}

//shad <REG_M>,<REG_N>          
sh4op(i0100_nnnn_mmmm_1100)
{
	//iNimp("shad <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	u32 sgn = r[m] & 0x80000000;
	if (sgn == 0)
		r[n] <<= (s32)(r[m] & 0x1F);
	else if ((r[m] & 0x1F) == 0)
	{
		if ((r[n] & 0x80000000) == 0)
			r[n] = 0;
		else
			r[n] = 0xFFFFFFFF;
	}
	else
		r[n] = ((s32)r[n]) >> (s32)((~r[m] & 0x1F) + 1);
}


//shld <REG_M>,<REG_N>          
sh4op(i0100_nnnn_mmmm_1101)
{//ToDo : Check This [26/4/05] x2
	//iNimp("shld <REG_M>,<REG_N>");
	//HACK : CHECKME
	u32 n = GetN(op);
	u32 m = GetM(op);
	s32 s;

	s =  (s32)(r[m] & 0x80000000);
	if (s == 0)
		r[n] <<= (r[m]);
	else if ((r[m] & 0x1f) == 0)
		r[n] = 0;
	else
		r[n] = (u32)r[n] >> (32-r[m]);
}



//rotcl <REG_N>                 
sh4op(i0100_nnnn_0010_0100)
{//ToDo : Check This [26/4/05]
	//iNimp("rotcl <REG_N>");
	u32 n = GetN(op);
	u32 t;
	//return;
	t = sr.T;

	sr.T = r[n] >> 31;

	r[n] <<= 1;

	/*
	if (t==1)
	r[n] |= 0x1;
	else
	r[n] &= 0xFFFFFFFE;*/
	r[n]|=t;
}


//rotl <REG_N>                  
sh4op(i0100_nnnn_0000_0100)
{
	//iNimp("rotl <REG_N>");
	u32 n = GetN(op);
	//return;
	/*
	if ((r[n] & 0x80000000)!=0)
	sr.T=1;
	else
	sr.T = 0;*/

	sr.T=r[n]>>31;

	r[n] <<= 1;

	/*if (sr.T!=0)
	r[n] |= 0x00000001;
	else
	r[n] &= 0xFFFFFFFE;*/
	r[n]|=sr.T;
}

//rotcr <REG_N>                 
sh4op(i0100_nnnn_0010_0101)
{
	//iNimp("rotcr <REG_N>");
	u32 n = GetN(op);
	u32 temp;

	/*if ((R[n] & 0x00000001) == 0) 
	temp = 0;
	else 
	temp = 1;*/

	temp = r[n] & 0x1;

	r[n] >>= 1;


	/*
	if (sr.T == 1) 
	r[n] |= 0x80000000;
	else 
	r[n] &= 0x7FFFFFFF;*/
	r[n] |=sr.T<<31;

	sr.T = temp;
	/*if (temp == 1) 
	T = 1;
	else 
	T = 0;*/
}


//rotr <REG_N>                  
sh4op(i0100_nnnn_0000_0101)
{
	//iNimp("rotr <REG_N>");//check ++
	u32 n = GetN(op);
	sr.T = r[n] & 0x1;
	r[n] >>= 1;
	r[n] |= (sr.T << 31);
}					
//************************ byte reorder/sign ************************
//swap.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1000)
{
	//iNimp("swap.b <REG_M>,<REG_N>");
	u32 m = GetM(op);
	u32 n = GetN(op);
	r[n] = (r[m] & 0xFFFF0000) | ((r[m]&0xFF)<<8) | ((r[m]>>8)&0xFF);
} 


//swap.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1001)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);

	u16 t = (u16)(r[m]>>16);
	r[n] = (r[m] << 16) | t;
} 



//extu.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1100)
{//TODO : CHECK THIS
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] = r[m] & 0xFF;
} 


//extu.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1101)
{//TODO : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
	r[n] = r[m] & 0x0000FFFF;
} 


//exts.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1110)
{//TODO : Check This [26/4/05]
	//iNimp("exts.b <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n] = (u32)(s8)(u8)(r[m] & 0xFF);

} 


//exts.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1111)
{//ToDo : Check This [26/4/05]
	//iNimp("exts.w <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n] = (u32)(s16)(u16)(r[m] & 0xFFFF);
} 


//xtrct <REG_M>,<REG_N>         
sh4op(i0010_nnnn_mmmm_1101)
{//TODO: unsure of proper emulation [26/4/05]
	//iNimp("xtrct <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n] = ((r[n] >> 16) & 0xFFFF) | ((r[m] << 16) & 0xFFFF0000);
}


//************************ xxx.b #<imm>,@(R0,GBR) ************************
//tst.b #<imm>,@(R0,GBR)        
sh4op(i1100_1100_iiii_iiii)
{
	iNimp(op, "tst.b #<imm>,@(R0,GBR)");
}


//and.b #<imm>,@(R0,GBR)        
sh4op(i1100_1101_iiii_iiii)
{
	iNimp(op, "and.b #<imm>,@(R0,GBR)");
}


//xor.b #<imm>,@(R0,GBR)        
sh4op(i1100_1110_iiii_iiii)
{
	iNimp(op, "xor.b #<imm>,@(R0,GBR)");
}


//or.b #<imm>,@(R0,GBR)         
sh4op(i1100_1111_iiii_iiii)
{
	//iNimp(op, "or.b #<imm>,@(R0,GBR)");
	u8 temp = (u8)ReadMem8(gbr +r[0]);
	temp |= GetImm8(op);
	WriteMem8(gbr +r[0], temp);
}
//tas.b @<REG_N>                
sh4op(i0100_nnnn_0001_1011)
{
	//iNimp("tas.b @<REG_N>");
	u32 n = GetN(op);
	u8 val;

	val=(u8)ReadMem8(r[n]);

	if (val == 0)
		sr.T = 1;
	else
		sr.T = 0;

	val |= 0x80;

	WriteMem8(r[n], val);
}




//bah
//Not implt
sh4op(iNotImplemented)
{
	cpu_iNimp(op,"Unkown opcode");
	//em.status = EMU_HALTED;
	//	StopThread();
	//recSh4Stop();
	//Sh4Stop();
	//Error
}

sh4op(gdrom_hle_op)
{
#ifdef ZGDROM
	gdop(op);
#else
	EMUERROR("GDROM HLE NOT SUPPORTED");
#endif
}