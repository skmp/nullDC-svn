
#include "sh4_cpu_shil.h"
#include "emmiter/emmiter.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_opcode_list.h"
#include "dc/sh4/sh4_registers.h"
#include "dc/sh4/shil/shil.h"

#undef sh4op

//simple recompiler
//to test up some register allocation code , ect

// test recompiler stuff

//block manager - need to be writen
//emitters - need to be writen
//opcode handlers - need to be writen
//register structs/class - need to be writen

//this will be a very simple recompiler 
//no register allocation , no sse , no mmx , and generaly no other optimisations
//the idea is to test my idea to warp registers onto structs ect

#define SH4_REC


shil_stream* ilst;

#define sh4op(str) void  __fastcall rec_shil_##str (u32 op,u32 pc)

#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
#define GetImm4(str) ((str>>0) & 0xf)
#define GetImm8(str) ((str>>0) & 0xff)
#define GetSImm8(str) ((s8)((str>>0) & 0xff))
#define GetImm12(str) ((str>>0) & 0xfff)
#define GetSImm12(str) (((s16)((GetImm12(str))<<4))>>3)

//#define tmu_underflow  0x0100
#define iNimp(op,info) rec_iNimp(pc,op,info)

#define shil_intepret(str) 

//************************ TLB/Cache ************************
//ldtlb                         
sh4op(i0000_0000_0011_1000)
{
	//iNimp(op, "ldtlb");
	shil_intepret(op);
} 



//ocbi @<REG_N>                 
sh4op(i0000_nnnn_1001_0011)
{
	//iNimp("ocbi @<REG_N> ");
	shil_intepret(op);
} 


//ocbp @<REG_N>                 
sh4op(i0000_nnnn_1010_0011)
{
	//iNimp("ocbp @<REG_N> ");
	shil_intepret(op);
} 


//ocbwb @<REG_N>                
sh4op(i0000_nnnn_1011_0011)
{
	//iNimp("ocbwb @<REG_N> ");
	shil_intepret(op);
} 

//pref @<REG_N>                 
sh4op(i0000_nnnn_1000_0011)
{
	shil_intepret(op);
	/*
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
				libPvr->pvr_info.TADma(Address,sq,1);
			}
			catch(...){}
		}
		else
		{
			for (u32 i = 0; i < 8; i++)
				WriteMem32((Address + (i << 2)), sq[i]);
		}

	}*/
}







//************************ Set/Get T/S ************************
//sets                          
sh4op(i0000_0000_0101_1000)
{
	//iNimp("sets");
	//sr.S = 1;
	ilst->mov(Sh4RegType::sr_S,1);
} 


//clrs                          
sh4op(i0000_0000_0100_1000)
{
	//iNimp(op, "clrs");
	ilst->mov(Sh4RegType::sr_S,0);
} 

//sett                          
sh4op(i0000_0000_0001_1000)
{
	//iNimp("sett");
	//sr.T = 1;
	ilst->mov(Sh4RegType::sr_T,1);
} 



//clrt                          
sh4op(i0000_0000_0000_1000)
{
	//iNimp("clrt");
	//sr.T = 0;
	ilst->mov(Sh4RegType::sr_T,0);
} 
//movt <REG_N>                  
sh4op(i0000_nnnn_0010_1001)
{
	//iNimp("movt <REG_N>");
	u32 n = GetN(op);
	ilst->mov(r[n],Sh4RegType::sr_T);
} 
//************************ Reg Compares ************************
//cmp/pz <REG_N>                
sh4op(i0100_nnnn_0001_0001)
{//ToDo : Check This [26/4/05]
	//iNimp("cmp/pz <REG_N>");
	u32 n = GetN(op);

	//if (((s32)r[n]) >= 0)
	//	sr.T = 1;
	//else
	//	sr.T = 0;

	ilst->cmp(r[n],0);
	ilst->SaveT(cmd_cond::CC_NL);
}

//cmp/pl <REG_N>                
sh4op(i0100_nnnn_0001_0101)
{//TODO : !Add this
	//iNimp("cmp/pl <REG_N>");
	u32 n = GetN(op);
	//if ((s32)r[n] > 0) 
	//	sr.T = 1;
	//else 
	//	sr.T = 0;

	ilst->cmp(r[n],0);
	ilst->SaveT(cmd_cond::CC_NLE);
}

//cmp/eq #<imm>,R0              
sh4op(i1000_1000_iiii_iiii)
{//TODO : Check This [26/4/05]
	//u32 imm = (u32)(s32)(GetSImm8(op));
	//if (r[0] == imm)
	//	sr.T =1;
	//else
	//	sr.T =0;
	ilst->cmp(r[n],GetSImm8(op));
	ilst->SaveT(cmd_cond::CC_E);
}

//cmp/eq <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0000)
{
	//iNimp("cmp/eq <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	//if (r[m] == r[n])
		//sr.T = 1;
	//else
		//sr.T = 0;
	ilst->cmp(r[n],r[m]);
	ilst->SaveT(cmd_cond::CC_E);
}

//cmp/hs <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0010)
{//ToDo : Check Me [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);
//	if (r[n] >= r[m])
//		sr.T=1;
//	else
//		sr.T=0;
	ilst->cmp(r[n],r[m]);
	ilst->SaveT(cmd_cond::CC_AE);
}

//cmp/ge <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0011)
{//TODO : Check This [26/4/05]
	//iNimp("cmp/ge <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
//	if ((s32)r[n] >= (s32)r[m])
//		sr.T = 1;
//	else 
//		sr.T = 0;
	ilst->cmp(r[n],r[m]);
	ilst->SaveT(cmd_cond::CC_GE);
}

//cmp/hi <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0110)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

//	if (r[n] > r[m])
//		sr.T=1;
	//else
	//	sr.T=0;
	ilst->cmp(r[n],r[m]);
	ilst->SaveT(cmd_cond::CC_A);
}

//cmp/gt <REG_M>,<REG_N>        
sh4op(i0011_nnnn_mmmm_0111)
{//TODO : Check This [26/4/05]
	//iNimp("cmp/gt <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	//if (((s32)r[n]) > ((s32)r[m]))
	//	sr.T = 1;
	//else 
	//	sr.T = 0;
	ilst->cmp(r[n],r[m]);
	ilst->SaveT(cmd_cond::CC_G);
}

//cmp/str <REG_M>,<REG_N>       
sh4op(i0010_nnnn_mmmm_1100)
{//TODO : Check This [26/4/05]
	shil_intepret(i0010_nnnn_mmmm_1100,op);
	//iNimp("cmp/str <REG_M>,<REG_N>");
	/*u32 n = GetN(op);
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
	else sr.T=0;*/
}

//tst #<imm>,R0                 
sh4op(i1100_1000_iiii_iiii)
{//TODO : Check This [26/4/05]
	//iNimp("tst #<imm>,R0");
	//u32 utmp1 = r[0] & GetImm8(op);
	//if (utmp1 == 0) 
	//	sr.T = 1;
	//else 
	//	sr.T = 0;
	ilst->test(r[0],GetImm8(op));
	ilst->SaveT(cmd_cond::CC_Z);
}
//tst <REG_M>,<REG_N>           
sh4op(i0010_nnnn_mmmm_1000)
{//ToDo : Check This [26/4/05]
	u32 n = GetN(op);
	u32 m = GetM(op);

	//if ((r[n] & r[m])!=0)
		//sr.T=0;
	//else
		//sr.T=1;

	ilst->test(r[n],r[m]);
	ilst->SaveT(cmd_cond::CC_Z);
}
//************************ mulls! ************************ 
//mulu.w <REG_M>,<REG_N>          
sh4op(i0010_nnnn_mmmm_1110)
{
	shil_intepret(op);
	//iNimp("mulu.w <REG_M>,<REG_N>");//check  ++
	/*u32 n = GetN(op);
	u32 m = GetM(op);
	macl=((u16)(s32)r[n])*
		((u16)(s32)r[m]);*/
}

//muls.w <REG_M>,<REG_N>          
sh4op(i0010_nnnn_mmmm_1111)
{
	shil_intepret(op);
	//TODO : Check This [26/4/05]
	//iNimp("muls <REG_M>,<REG_N>");
	/*
	u32 n = GetN(op);
	u32 m = GetM(op);

	macl = (u32)(((s16)(s32)r[n]) * ((s16)(s32)r[m]));*/
}
//dmulu.l <REG_M>,<REG_N>       
sh4op(i0011_nnnn_mmmm_0101)
{
	shil_intepret(op);
	/*
	//iNimp("dmulu.l <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	u64 x;

	x = (u64)r[n] * (u64)r[m];

	macl = (u32)(x & 0xFFFFFFFF);
	mach = (u32)((x >> 32) & 0xFFFFFFFF);*/
}

//dmuls.l <REG_M>,<REG_N>       
sh4op(i0011_nnnn_mmmm_1101)
{
	shil_intepret(op);/*
	//iNimp("dmuls.l <REG_M>,<REG_N>");//check ++
	u32 n = GetN(op);
	u32 m = GetM(op);
	s64 x;

	x = (s64)(s32)r[n] * (s64)(s32)r[m];

	macl = (u32)(x & 0xFFFFFFFF);
	mach = (u32)((x >> 32) & 0xFFFFFFFF);*/
}


//mac.w @<REG_M>+,@<REG_N>+     
sh4op(i0100_nnnn_mmmm_1111)
{
	shil_intepret(op);
	//iNimp(op, "mac.w @<REG_M>+,@<REG_N>+");
}		
//mac.l @<REG_M>+,@<REG_N>+     
sh4op(i0000_nnnn_mmmm_1111)
{//TODO : !Add this
	shil_intepret(op);/*
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
	mach = (u32)((result >> 32) & 0xFFFFFFFF);*/
}

//mul.l <REG_M>,<REG_N>         
sh4op(i0000_nnnn_mmmm_0111)
{//TODO : CHECK THIS
	shil_intepret(op);
	/*
	u32 n = GetN(op);
	u32 m = GetM(op);
	macl = (u32)((((s32)r[n]) * ((s32)r[m])));*/
}
//************************ Div ! ************************ 
//div0u                         
sh4op(i0000_0000_0001_1001)
{//ToDo : Check This [26/4/05]
	//iNimp("div0u");
	//sr.Q = 0;
	//sr.M = 0;
	//sr.T = 0;
	ilst->mov(Sh4RegType::sr_Q,0);
	ilst->mov(Sh4RegType::sr_M,0);
	ilst->mov(Sh4RegType::sr_T,0);
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
}

//div1 <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_0100)
{//ToDo : Check This [26/4/05]
	u32 n=GetN(op);
	u32 m=GetM(op);
	shil_intepret(op);
/*
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
	sr.T = (sr.Q==sr.M);*/
}


//************************ Simple maths ************************ 
//addc <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1110)
{//ToDo : Check This [26/4/05]
	//iNimp("addc <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	//u32 tmp1 = r[n] + r[m];
	//u32 tmp0 = r[n];

	//r[n] = tmp1 + sr.T;

	//if (tmp0 > tmp1)
	//	sr.T=1;
	//else
	//	sr.T = 0;

	//if (tmp1 > r[n])
	//	sr.T = 1;
	
	

	ilst->LoadT(CF);			//load T to carry flag
	ilst->adc(r[n],r[m]);		//add w/ carry
	ilst->SaveT(SaveCF);//save CF to T

}

// addv <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1111)
{
	shil_intepret(op);
	//iNimp(op, "addv <REG_M>,<REG_N>");
}

//subc <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1010)
{
	shil_intepret(op);
	/*
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

	//no subc on x86 .. what a pain
	*/
}

//subv <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1011)
{
	shil_intepret(op);
	//iNimp(op, "subv <REG_M>,<REG_N>");
}
//dt <REG_N>                    
sh4op(i0100_nnnn_0001_0000)
{
	u32 n = GetN(op);
//	r[n]-=1;
//	if (r[n] == 0)
//		sr.T=1;
//	else
//		sr.T=0;
	ilst->dec(r[n]);
	ilst->SaveT(cmd_cond::CC_Z);
}

//negc <REG_M>,<REG_N>          
sh4op(i0110_nnnn_mmmm_1010)
{
	//iNimp("negc <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	shil_intepret(op);
	/*u32 temp= (u32)(0 - ((s32)r[m]));

	r[n] = temp - sr.T;

	if (0 < temp)
	sr.T = 1;
	else
	sr.T = 0;

	if (temp < r[n])
	sr.T = 1;*/
	/*
	r[n]=-r[m]-sr.T;
	sr.T=r[n]>>31;
	*/
}


//neg <REG_M>,<REG_N>           
sh4op(i0110_nnnn_mmmm_1011)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n] = -r[m];
	ilst->neg(r[n]);
} 

//not <REG_M>,<REG_N>           
sh4op(i0110_nnnn_mmmm_0111)
{
	//iNimp("not <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	//r[n] = ~r[m];
	ilst->not(r[n]);
} 


//************************ shifts/rotates ************************
//shll <REG_N>                  
sh4op(i0100_nnnn_0000_0000)
{
	u32 n = GetN(op);

	//sr.T = r[n] >> 31;
	//r[n] <<= 1;
	ilst->shl(r[n]);
	ilst->SaveT(SaveCF);
}
//shal <REG_N>                  
sh4op(i0100_nnnn_0010_0000)
{
	u32 n=GetN(op);
	//sr.T=r[n]>>31;
	//((s32)r[n])<<=1;
	printf("shal is used , WTF\n");
	printf("shal is used , WTF\n");
	printf("shal is used , WTF\n");

	ilst->shl(r[n]);
	ilst->SaveT(SaveCF);
}


//shlr <REG_N>                  
sh4op(i0100_nnnn_0000_0001)
{
	u32 n = GetN(op);
	//sr.T = r[n] & 0x1;
	//r[n] >>= 1;
	
	ilst->shr(r[n]);
	ilst->SaveT(SaveCF);
}

//shar <REG_N>                  
sh4op(i0100_nnnn_0010_0001)
{
	//iNimp("shar <REG_N>");
	u32 n = GetN(op);
	u32 t;

	//sr.T=r[n] & 1;
	//r[n]=((s32)r[n])>>1;
	ilst->sar(r[n]);
	ilst->SaveT(SaveCF);
}

//shad <REG_M>,<REG_N>          
sh4op(i0100_nnnn_mmmm_1100)
{
	shil_intepret(op);
	/*
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
		r[n] = ((s32)r[n]) >> (s32)((~r[m] & 0x1F) + 1);*/
}


//shld <REG_M>,<REG_N>          
sh4op(i0100_nnnn_mmmm_1101)
{
	shil_intepret(op);
	/*
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
		r[n] = (u32)r[n] >> (32-r[m]);*/
}



//rotcl <REG_N>                 
sh4op(i0100_nnnn_0010_0100)
{//ToDo : Check This [26/4/05]
	//iNimp("rotcl <REG_N>");
	u32 n = GetN(op);
	//u32 t;
	//t = sr.T;
	//sr.T = r[n] >> 31;
	//r[n] <<= 1;
	//r[n]|=t;

	ilst->LoadT(CF);
	ilst->rcl(r[n],1);
	ilst->SaveT(SaveCF);
}


//rotl <REG_N>                  
sh4op(i0100_nnnn_0000_0100)
{
	//iNimp("rotl <REG_N>");
	u32 n = GetN(op);

	//sr.T=r[n]>>31;
	//r[n] <<= 1;
	//r[n]|=sr.T;

	ilst->rol(r[n],1);
	ilst->SaveT(SaveCF);
}

//rotcr <REG_N>                 
sh4op(i0100_nnnn_0010_0101)
{
	//iNimp("rotcr <REG_N>");
	u32 n = GetN(op);
	//u32 temp;
	//temp = r[n] & 0x1;
	//r[n] >>= 1;
	//r[n] |=sr.T<<31;
	//sr.T = temp;
	ilst->LoadT(CF);
	ilst->rcr(r[n],1);
	ilst->SaveT(SaveCF);

}


//rotr <REG_N>                  
sh4op(i0100_nnnn_0000_0101)
{
	//iNimp("rotr <REG_N>");//check ++
	u32 n = GetN(op);
	//sr.T = r[n] & 0x1;
	//r[n] >>= 1;
	//r[n] |= (sr.T << 31);
	ilst->ror(r[n],1);
	ilst->SaveT(SaveCF);
}					
//************************ byte reorder/sign ************************
//swap.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1000)
{
	//iNimp("swap.b <REG_M>,<REG_N>");
	u32 m = GetM(op);
	u32 n = GetN(op);
	//r[n] = (r[m] & 0xFFFF0000) | ((r[m]&0xFF)<<8) | ((r[m]>>8)&0xFF);
	ilst->mov(r[n],r[m]);
	ilst->bswap(r[n]);
} 


//swap.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1001)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	//u16 t = (u16)(r[m]>>16);
	//r[n] = (r[m] << 16) | t;
	ilst->mov(r[n],r[m]);
	ilst->wswap(r[n]);
} 

//extu.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1100)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n] = r[m] & 0xFF;
	ilst->movzxb(r[n],r[m]);
} 


//extu.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1101)
{
	u32 n = GetN(op);
	u32 m = GetM(op);
	//r[n] = r[m] & 0x0000FFFF;
	ilst->movzxw(r[n],r[m]);
} 


//exts.b <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1110)
{
	//iNimp("exts.b <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	//r[n] = (u32)(s8)(u8)(r[m] & 0xFF);
	ilst->movsxb(r[n],r[m]);

} 


//exts.w <REG_M>,<REG_N>        
sh4op(i0110_nnnn_mmmm_1111)
{
	//iNimp("exts.w <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	//r[n] = (u32)(s16)(u16)(r[m] & 0xFFFF);
	ilst->movsxw(r[n],r[m]);
} 


//xtrct <REG_M>,<REG_N>         
sh4op(i0010_nnnn_mmmm_1101)
{
	//iNimp("xtrct <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	r[n] = ((r[n] >> 16) & 0xFFFF) | ((r[m] << 16) & 0xFFFF0000);
}


//************************ xxx.b #<imm>,@(R0,GBR) ************************
//tst.b #<imm>,@(R0,GBR)        
sh4op(i1100_1100_iiii_iiii)
{
	shil_intepret(op);
	//iNimp(op, "tst.b #<imm>,@(R0,GBR)");
}


//and.b #<imm>,@(R0,GBR)        
sh4op(i1100_1101_iiii_iiii)
{
	shil_intepret(op);
	//iNimp(op, "and.b #<imm>,@(R0,GBR)");
}


//xor.b #<imm>,@(R0,GBR)        
sh4op(i1100_1110_iiii_iiii)
{
	shil_intepret(op);
	//iNimp(op, "xor.b #<imm>,@(R0,GBR)");
}


//or.b #<imm>,@(R0,GBR)         
sh4op(i1100_1111_iiii_iiii)
{
	shil_intepret(op);
	/*
	//iNimp(op, "or.b #<imm>,@(R0,GBR)");
	u8 temp = (u8)ReadMem8(gbr +r[0]);
	temp |= GetImm8(op);
	WriteMem8(gbr +r[0], temp);
	*/
}
//tas.b @<REG_N>                
sh4op(i0100_nnnn_0001_1011)
{
	shil_intepret(op);
	/*
	//iNimp("tas.b @<REG_N>");
	u32 n = GetN(op);
	u8 val;

	val=(u8)ReadMem8(r[n]);

	if (val == 0)
		sr.T = 1;
	else
		sr.T = 0;

	val |= 0x80;

	WriteMem8(r[n], val);*/
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
