#include "sh4_cpu_shil.h"
#include "emitter/emitter.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_opcode_list.h"
#include "dc/sh4/sh4_registers.h"
#include "dc/sh4/shil/shil.h"
#include <assert.h>

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

#define sh4op(str) void  __fastcall rec_shil_##str (u32 op,u32 pc,BasicBlock* bb)

#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
#define GetImm4(str) ((str>>0) & 0xf)
#define GetImm8(str) ((u8)(str>>0))
#define GetSImm8(str) ((s8)((str>>0) & 0xff))
#define GetImm12(str) ((str>>0) & 0xfff)
#define GetSImm12(str) (((s16)((GetImm12(str))<<4))>>3)

//#define tmu_underflow  0x0100
#define iNimp(info) rec_shil_iNimp(pc,op,info)

#define shil_interpret(str)  ilst->shil_ifb(str,pc);

Sh4RegType dyna_reg_id_r[16];
Sh4RegType dyna_reg_id_r_bank[8];

Sh4RegType dyna_reg_id_fr[16];
Sh4RegType dyna_reg_id_xf[16];

Sh4RegType dyna_reg_id_dr[8];
Sh4RegType dyna_reg_id_xd[8];

#define r dyna_reg_id_r
#define r_bank dyna_reg_id_r_bank

#define fr dyna_reg_id_fr
#define xf dyna_reg_id_xf

#define dr dyna_reg_id_dr
#define xd dyna_reg_id_xd

void rec_shil_iNimp(u32 pc,u32 op ,char * text)
{
}
//************************ TLB/Cache ************************
//ldtlb                         
sh4op(i0000_0000_0011_1000)
{
	//iNimp(op, "ldtlb");
	shil_interpret(op);
} 



//ocbi @<REG_N>                 
sh4op(i0000_nnnn_1001_0011)
{
	//iNimp("ocbi @<REG_N> ");
	shil_interpret(op);
} 


//ocbp @<REG_N>                 
sh4op(i0000_nnnn_1010_0011)
{
	//iNimp("ocbp @<REG_N> ");
	shil_interpret(op);
} 


//ocbwb @<REG_N>                
sh4op(i0000_nnnn_1011_0011)
{
	//iNimp("ocbwb @<REG_N> ");
	shil_interpret(op);
} 

//pref @<REG_N>                 
sh4op(i0000_nnnn_1000_0011)
{
	shil_interpret(op);
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
	shil_interpret(op);
	//ilst->mov(Sh4RegType::sr_S,1);
} 


//clrs                          
sh4op(i0000_0000_0100_1000)
{
	//iNimp(op, "clrs");
	shil_interpret(op);
	//ilst->mov(Sh4RegType::sr_S,0);
} 

//sett                          
sh4op(i0000_0000_0001_1000)
{
	//iNimp("sett");
	//sr.T = 1;
	shil_interpret(op);
	//ilst->mov(Sh4RegType::sr_T,1);
} 



//clrt                          
sh4op(i0000_0000_0000_1000)
{
	//iNimp("clrt");
	//sr.T = 0;
	shil_interpret(op);
	//ilst->mov(Sh4RegType::sr_T,0);
} 
//movt <REG_N>                  
sh4op(i0000_nnnn_0010_1001)
{
	//iNimp("movt <REG_N>");
	u32 n = GetN(op);
	//shil_interpret(op);
	ilst->mov(r[n],reg_sr);
	ilst->and(r[n],1);
	//ilst->mov(r[n],Sh4RegType::sr_T);
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

	ilst->cmp(r[n],(s8)0);			//singed compare
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

	ilst->cmp(r[n],(s8)0);			//singed compare
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
	ilst->cmp(r[0],GetSImm8(op));
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
	//shil_interpret(op);
	//return;
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
	shil_interpret(op);
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
	//shil_interpret(op);
	//iNimp("mulu.w <REG_M>,<REG_N>");//check  ++
	u32 n = GetN(op);
	u32 m = GetM(op);
	ilst->mulu_16_16_32(r[n],r[m]);
	/*
	macl=((u16)(s32)r[n])*
		((u16)(s32)r[m]);*/
}

//muls.w <REG_M>,<REG_N>          
sh4op(i0010_nnnn_mmmm_1111)
{
	//shil_interpret(op);
	//TODO : Check This [26/4/05]
	//iNimp("muls <REG_M>,<REG_N>");
	
	u32 n = GetN(op);
	u32 m = GetM(op);

	ilst->muls_16_16_32(r[n],r[m]);
	/*
	macl = (u32)(((s16)(s32)r[n]) * ((s16)(s32)r[m]));*/
}
//dmulu.l <REG_M>,<REG_N>       
sh4op(i0011_nnnn_mmmm_0101)
{
//	shil_interpret(op);

	//iNimp("dmulu.l <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);
	
	ilst->mulu_32_32_64(r[n],r[m]);

	/*
	u64 x;

	x = (u64)r[n] * (u64)r[m];

	macl = (u32)(x & 0xFFFFFFFF);
	mach = (u32)((x >> 32) & 0xFFFFFFFF);*/
}

//dmuls.l <REG_M>,<REG_N>       
sh4op(i0011_nnnn_mmmm_1101)
{
	//shil_interpret(op);
	//iNimp("dmuls.l <REG_M>,<REG_N>");//check ++
	u32 n = GetN(op);
	u32 m = GetM(op);
	
	ilst->muls_32_32_64(r[n],r[m]);

	/*s64 x;

	x = (s64)(s32)r[n] * (s64)(s32)r[m];

	macl = (u32)(x & 0xFFFFFFFF);
	mach = (u32)((x >> 32) & 0xFFFFFFFF);*/
}


//mac.w @<REG_M>+,@<REG_N>+     
sh4op(i0100_nnnn_mmmm_1111)
{
	shil_interpret(op);
	//iNimp(op, "mac.w @<REG_M>+,@<REG_N>+");
}		
//mac.l @<REG_M>+,@<REG_N>+     
sh4op(i0000_nnnn_mmmm_1111)
{//TODO : !Add this
	shil_interpret(op);/*
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
	//shil_interpret(op);


	
	u32 n = GetN(op);
	u32 m = GetM(op);
	ilst->muls_32_32_32(r[n],r[m]);
	/*macl = (u32)((((s32)r[n]) * ((s32)r[m])));*/
}
//************************ Div ! ************************ 
#define MASK_N_M 0xF00F
#define MASK_N   0xF0FF
#define MASK_NONE   0xFFFF

#define DIV0U_KEY 0x0019
#define DIV0S_KEY 0x2007
#define DIV1_KEY 0x3004
#define ROTCL_KEY 0x4024

Sh4RegType div_som_reg1;
Sh4RegType div_som_reg2;
Sh4RegType div_som_reg3;

u32 MatchDiv32(u32 pc , Sh4RegType &reg1,Sh4RegType &reg2 , Sh4RegType &reg3)
{
	u32 v_pc=pc;
	u32 match=1;
	for (int i=0;i<32;i++)
	{
		u16 opcode=ReadMem16(v_pc);
		v_pc+=2;
		if ((opcode&MASK_N)==ROTCL_KEY)
		{
			if (reg1==NoReg)
				reg1=(Sh4RegType)GetN(opcode);
			else if (reg1!=(Sh4RegType)GetN(opcode))
				break;
			match++;
		}
		else
			break;
		
		opcode=ReadMem16(v_pc);
		v_pc+=2;
		if ((opcode&MASK_N_M)==DIV1_KEY)
		{
			if (reg2==NoReg)
				reg2=(Sh4RegType)GetM(opcode);
			else if (reg2!=(Sh4RegType)GetM(opcode))
				break;
			
			if (reg2==reg1)
				break;

			if (reg3==NoReg)
				reg3=(Sh4RegType)GetN(opcode);
			else if (reg3!=(Sh4RegType)GetN(opcode))
				break;
			
			if (reg3==reg1)
				break;

			match++;
		}
		else
			break;
	}
	
	return match;
}
bool __fastcall MatchDiv32u(u32 op,u32 pc)
{
	div_som_reg1=Sh4RegType::NoReg;
	div_som_reg2=Sh4RegType::NoReg;
	div_som_reg3=Sh4RegType::NoReg;

	u32 match=MatchDiv32(pc+2,div_som_reg1,div_som_reg2,div_som_reg3);


	printf("DIV32U matched %d%%\n",match*100/65);
	if (match==65)
	{
		//DIV32U was perfectly matched :)
		return true;
	}
	else //no match ...
		return false;
}

bool __fastcall MatchDiv32s(u32 op,u32 pc)
{
	u32 n = GetN(op);
	u32 m = GetM(op);

	div_som_reg1=Sh4RegType::NoReg;
	div_som_reg2=(Sh4RegType)m;
	div_som_reg3=(Sh4RegType)n;

	u32 match=MatchDiv32(pc+2,div_som_reg1,div_som_reg2,div_som_reg3);
	printf("DIV32S matched %d%%\n",match*100/65);
	
	if (match==65)
	{
		//DIV32S was perfectly matched :)
		return true;
	}
	else //no match ...
		return false;
}
//div0u                         
sh4op(i0000_0000_0001_1001)
{	
	if (MatchDiv32u(op,pc))
	{
		//DIV32U was perfectly matched :)
		bb->flags.SynthOpcode=BLOCK_SOM_SIZE_128;
		ilst->div(div_som_reg1,div_som_reg2,div_som_reg3,FLAG_ZX|FLAG_32);
	}
	else //fallback to interpreter (16b div propably)
		shil_interpret(op);
}
//div0s <REG_M>,<REG_N>         
sh4op(i0010_nnnn_mmmm_0111)
{
	if (MatchDiv32s(op,pc))
	{
		//DIV32S was perfectly matched :)
		bb->flags.SynthOpcode=BLOCK_SOM_SIZE_128;
		ilst->div(div_som_reg1,div_som_reg2,div_som_reg3,FLAG_SX|FLAG_32);
	}
	else //fallback to interpreter (16b div propably)
		shil_interpret(op);

	return;
}

//div1 <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_0100)
{//ToDo : Check This [26/4/05]
	u32 n=GetN(op);
	u32 m=GetM(op);
	shil_interpret(op);
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
	shil_interpret(op);
	//iNimp(op, "addv <REG_M>,<REG_N>");
}

//subc <REG_M>,<REG_N>          
sh4op(i0011_nnnn_mmmm_1010)
{
	shil_interpret(op);
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
	shil_interpret(op);
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
	shil_interpret(op);
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
	ilst->mov(r[n],r[m]);
	ilst->neg(r[n]);
	//shil_interpret(op);
} 

//not <REG_M>,<REG_N>           
sh4op(i0110_nnnn_mmmm_0111)
{
	//iNimp("not <REG_M>,<REG_N>");
	u32 n = GetN(op);
	u32 m = GetM(op);

	//r[n] = ~r[m];
	ilst->mov(r[n],r[m]);
	ilst->not(r[n]);
} 


//************************ shifts/rotates ************************
//shll <REG_N>                  
sh4op(i0100_nnnn_0000_0000)
{
	u32 n = GetN(op);

	//sr.T = r[n] >> 31;
	//r[n] <<= 1;
	ilst->shl(r[n],1);
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

	ilst->shl(r[n],1);
	ilst->SaveT(SaveCF);
}


//shlr <REG_N>                  
sh4op(i0100_nnnn_0000_0001)
{
	u32 n = GetN(op);
	//sr.T = r[n] & 0x1;
	//r[n] >>= 1;
	
	ilst->shr(r[n],1);
	ilst->SaveT(SaveCF);
}

//shar <REG_N>                  
sh4op(i0100_nnnn_0010_0001)
{
	//iNimp("shar <REG_N>");
	u32 n = GetN(op);
//	u32 t;

	//sr.T=r[n] & 1;
	//r[n]=((s32)r[n])>>1;
	ilst->sar(r[n],1);
	ilst->SaveT(SaveCF);
}

//shad <REG_M>,<REG_N>          
sh4op(i0100_nnnn_mmmm_1100)
{
	shil_interpret(op);
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
	shil_interpret(op);
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
	ilst->rcl(r[n]);
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

	ilst->rol(r[n]);
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
	ilst->rcr(r[n]);
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
	ilst->ror(r[n]);
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

	shil_interpret(op);
	//r[n] =  wswap(r[n]&0xFFFF0000 | r[m]&0xFFFF)
	//r[n] = ((r[n] >> 16) & 0xFFFF) | ((r[m] << 16) & 0xFFFF0000);
}


//************************ xxx.b #<imm>,@(R0,GBR) ************************
//tst.b #<imm>,@(R0,GBR)        
sh4op(i1100_1100_iiii_iiii)
{
	shil_interpret(op);
	//iNimp(op, "tst.b #<imm>,@(R0,GBR)");
}


//and.b #<imm>,@(R0,GBR)        
sh4op(i1100_1101_iiii_iiii)
{
	shil_interpret(op);
	//iNimp(op, "and.b #<imm>,@(R0,GBR)");
}


//xor.b #<imm>,@(R0,GBR)        
sh4op(i1100_1110_iiii_iiii)
{
	shil_interpret(op);
	//iNimp(op, "xor.b #<imm>,@(R0,GBR)");
}


//or.b #<imm>,@(R0,GBR)         
sh4op(i1100_1111_iiii_iiii)
{
	shil_interpret(op);
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
	shil_interpret(op);
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
	shil_interpret(op);
	//cpu_iNimp(op,"Unkown opcode");
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



//all fpu emulation ops :)

//fadd <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0000)
{//TODO : CHECK THIS PR FP
	
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		ilst->fadd(fr[n],fr[m]);
		//fr[n] += fr[m];
		//CHECK_FPU_32(fr[n]);
	}
	else
	{
		shil_interpret(op);
		return;
		u32 n = (op >> 9) & 0x07;
		u32 m = (op >> 5) & 0x07;
		
		ilst->fadd(dr[n],dr[m]);

		//START64();
		//double drn=GetDR(n), drm=GetDR(m);
		//drn += drm;
		//CHECK_FPU_64(drn);
		//SetDR(n,drn);
		//END64();
	}
}

//fsub <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0001)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		ilst->fsub(fr[n],fr[m]);
		//fr[n] -= fr[m];
		//CHECK_FPU_32(fr[n]);
	}
	else
	{
		shil_interpret(op);
		return;
		//u32 n = (op >> 9) & 0x07;
		//u32 m = (op >> 5) & 0x07;
		//
		//ilst->fsub(dr[n],dr[m]);
		//START64();
		//double drn=GetDR(n), drm=GetDR(m);
		//drn-=drm;
		//dr[n] -= dr[m];
		//SetDR(n,drn);
		//END64();
	}
}																								
//fmul <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0010)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		ilst->fmul(fr[n],fr[m]);
	}
	else
	{
		shil_interpret(op);
		return;
		//u32 n = (op >> 9) & 0x07;
		//u32 m = (op >> 5) & 0x07;
		//START64();
		//double drn=GetDR(n), drm=GetDR(m);
		//drn*=drm;
		//dr[n] *= dr[m];
		//SetDR(n,drn);
		//END64();
		//ilst->fmul(dr[n],dr[m]);
	}
}
//fdiv <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0011)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		ilst->fdiv(fr[n],fr[m]);
	}
	else
	{
		shil_interpret(op);
		return;
		//u32 n = (op >> 9) & 0x07;
		//u32 m = (op >> 5) & 0x07;
		//START64();
		//double drn=GetDR(n), drm=GetDR(m);
		//drn/=drm;
		//SetDR(n,drn);
		//END64();
		//ilst->fdiv(dr[n],dr[m]);
	}
}
//fcmp/eq <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0100)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		//sr.T = (fr[m] == fr[n]) ? 1 : 0;
		ilst->fcmp(fr[n],fr[m]);
		ilst->SaveT(cmd_cond::CC_E);
	}
	else
	{
		shil_interpret(op);
		return;
		//u32 n = (op >> 9) & 0x07;
		//u32 m = (op >> 5) & 0x07;
		//ilst->cmp(dr[n],dr[m]);
		//START64();
		//sr.T = (GetDR(m) == GetDR(n)) ? 1 : 0;
		//END64();	
	}
}
//fcmp/gt <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0101)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		/*if (fr[n] > fr[m])
			sr.T = 1;
		else
			sr.T = 0;*/
		ilst->fcmp(fr[n],fr[m]);
		ilst->SaveT(cmd_cond::CC_NBE);
	}
	else
	{
		shil_interpret(op);
		return;
		//u32 n = (op >> 9) & 0x07;
		//u32 m = (op >> 5) & 0x07;
		
		/*START64();
		if (GetDR(n) > GetDR(m))
			sr.T = 1;
		else
			sr.T = 0;
		END64();*/

		//ilst->cmp(dr[n],dr[m]);
	}
}
//fmov.s @(R0,<REG_M>),<FREG_N>
sh4op(i1111_nnnn_mmmm_0110)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		//fr_hex[n] = ReadMem32(r[m] + r[0]);
		ilst->readm32(fr[n],r[0],r[m]);
	}
	else
	{
		shil_interpret(op);
		return;
		/*u32 n = (op >> 8) & 0x0E;
		u32 m = GetM(op);
		if (((op >> 8) & 0x1) == 0)
		{
			//fr_hex[n] = ReadMem32(r[m] + r[0]);
			//fr_hex[n + 1] = ReadMem32(r[m] + r[0] + 4);
			ilst->readm64(dr[n>>1],r[0],r[m]);
		}
		else
		{
			//xf_hex[n] = ReadMem32(r[m] + r[0]);
			//xf_hex[n + 1] = ReadMem32(r[m] + r[0] + 4);
			ilst->readm64(xd[n>>1],r[0],r[m]);
		}*/
	}
}


//fmov.s <FREG_M>,@(R0,<REG_N>)
sh4op(i1111_nnnn_mmmm_0111)
{//used
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		//WriteMem32(r[0] + r[n], fr_hex[m]);
		ilst->writem32(fr[m],r[0],r[n]);
	}
	else
	{
		shil_interpret(op);
		return;
		/*u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;
		if (((op >> 4) & 0x1) == 0)
		{
			//WriteMem32(r[n] + r[0], fr_hex[m]);
			//WriteMem32(r[n] + r[0] + 4, fr_hex[m + 1]);
			ilst->writem64(dr[m>>1],r[0],r[n]);
		}
		else
		{
			//WriteMem32(r[n] + r[0], xf_hex[m]);
			//WriteMem32(r[n] + r[0] + 4, xf_hex[m + 1]);
			ilst->writem64(xd[m>>1],r[0],r[n]);
		}*/
	}
}


//fmov.s @<REG_M>,<FREG_N> 
sh4op(i1111_nnnn_mmmm_1000)
{//used
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		//fr_hex[n] = ReadMem32(r[m]);
		ilst->readm32(fr[n],r[m]);
	}
	else
	{
		shil_interpret(op);
		return;
		/*u32 n = (op >> 8) & 0x0E;
		u32 m = GetM(op);
		if (((op >> 8) & 0x1) == 0)
		{
			//fr_hex[n] = ReadMem32(r[m]);
			//fr_hex[n + 1] = ReadMem32(r[m] + 4);
			ilst->readm64(dr[n>>1],r[m]);
		}
		else
		{
			//xf_hex[n] = ReadMem32(r[m]);
			//xf_hex[n + 1] = ReadMem32(r[m] + 4);
			ilst->readm64(xd[n>>1],r[m]);
		}*/
	}
}


//fmov.s @<REG_M>+,<FREG_N>
sh4op(i1111_nnnn_mmmm_1001)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		//fr_hex[n] = ReadMem32(r[m]);
		ilst->readm32(fr[n],r[m]);
		ilst->add(r[m],4);
		//r[m] += 4;
	}
	else 
	{
		//shil_interpret(op);
		//return;
		u32 n = (op >> 8) & 0x0E;
		u32 m = GetM(op);
		if (((op >> 8) & 0x1) == 0)
		{
			//fr_hex[n] = ReadMem32(r[m]);
			//fr_hex[n + 1] = ReadMem32(r[m]+ 4);
			ilst->readm32(fr[n],r[m]);
			ilst->add(r[m],4);
			ilst->readm32(fr[n+1],r[m]);
			ilst->add(r[m],4);
		}
		else
		{
			//xf_hex[n] = ReadMem32(r[m] );
			//xf_hex[n + 1] = ReadMem32(r[m]+ 4);
			ilst->readm32(xf[n],r[m]);
			ilst->add(r[m],4);
			ilst->readm32(xf[n+1],r[m]);
			ilst->add(r[m],4);
		}
		//ilst->add(r[m],8);
		//r[m] += 8;
	}
}


//fmov.s <FREG_M>,@<REG_N>
sh4op(i1111_nnnn_mmmm_1010)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		//WriteMem32(r[n], fr_hex[m]);
		ilst->writem32(fr[m],r[n]);
	}
	else
	{
		shil_interpret(op);
		return;
		/*
		u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;

		if (((op >> 4) & 0x1) == 0)
		{
			//WriteMem32(r[n], fr_hex[m]);
			//WriteMem32(r[n] + 4, fr_hex[m + 1]);
			ilst->writem64(dr[m>>1],r[n]);
		}
		else
		{
			//WriteMem32(r[n], xf_hex[m]);
			//WriteMem32(r[n] + 4, xf_hex[m + 1]);
			ilst->writem64(xd[m>>1],r[n]);
		}*/
	}
}

//fmov.s <FREG_M>,@-<REG_N>
sh4op(i1111_nnnn_mmmm_1011)
{//used
	if (fpscr.SZ == 0)
	{
		//iNimp("fmov.s <FREG_M>,@-<REG_N>");
		u32 n = GetN(op);
		u32 m = GetM(op);

		//r[n] -= 4;
		ilst->sub(r[n],4);
		ilst->writem32(fr[m],r[n]);
		//WriteMem32(r[n], fr_hex[m]);
	}
	else
	{
		//shil_interpret(op);
		//return;
	
		u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;

		//r[n] -= 8;
		if (((op >> 4) & 0x1) == 0)
		{ 
			//WriteMem32(r[n] , fr_hex[m]);
			//WriteMem32(r[n]+ 4, fr_hex[m + 1]);
			ilst->sub(r[n],4);
			ilst->writem32(fr[m+1],r[n]);
			ilst->sub(r[n],4);
			ilst->writem32(fr[m],r[n]);
		}
		else
		{
			//WriteMem32(r[n] , xf_hex[m]);
			//WriteMem32(r[n]+ 4, xf_hex[m + 1]);
			ilst->sub(r[n],4);
			ilst->writem32(xf[m+1],r[n]);
			ilst->sub(r[n],4);
			ilst->writem32(xf[m],r[n]);
		}
	}
}


//fmov <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_1100)
{//TODO : checkthis
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		//fr[n] = fr[m];
		ilst->mov(fr[n],fr[m]);
	}
	else
	{
		u32 n = (op >> 9) & 0x7;
		u32 m = (op >> 5) & 0x7;

		switch ((op >> 4) & 0x11)
		{
			case 0x00:
				//dr[n] = dr[m];
				//fr_hex[n] = fr_hex[m];
				//fr_hex[n + 1] = fr_hex[m + 1];
				ilst->mov(dr[n],dr[m]);
				break;
			case 0x01:
				//dr[n] = xf[m];
				//fr_hex[n] = xf_hex[m];
				//fr_hex[n + 1] = xf_hex[m + 1];
				ilst->mov(dr[n],xd[m]);
				break;
			case 0x10:
				//xf[n] = dr[m];
				//xf_hex[n] = fr_hex[m];
				//xf_hex[n + 1] = fr_hex[m + 1];
				ilst->mov(xd[n],dr[m]);
				break;
			case 0x11:
				//xf[n] = xf[m];
				//xf_hex[n] = xf_hex[m];
				//xf_hex[n + 1] = xf_hex[m + 1];
				ilst->mov(xd[n],xd[m]);
				break;
		}
	}
}


//fabs <FREG_N>            
sh4op(i1111_nnnn_0101_1101)
{
	int n=GetN(op);
	
	if (fpscr.PR ==0)
	{
		//fr_hex[n]&=0x7FFFFFFF;
		ilst->fabs(fr[n]);
	}
	else
	{
		//fr_hex[(n&0xE)+1]&=0x7FFFFFFF;
		ilst->fabs(dr[n>>1]);
	}

}

//FSCA FPUL, DRn//F0FD//1111_nnn0_1111_1101
sh4op(i1111_nnn0_1111_1101)
{
	if (fpscr.PR==0)
	{
		int n=GetN(op) & 0xE;
		ilst->fsca(fr[n]);
		//shil_interpret(op);
	}
	else
		iNimp("FSCA : Double precision mode");

}

//FSRRA //1111_nnnn_0111_1101
sh4op(i1111_nnnn_0111_1101)
{

	// What about double precision?
	if (fpscr.PR==0)
	{
		u32 n = GetN(op);
		ilst->fsrra(fr[n]);
		//shil_interpret(op);
	}
	else
		iNimp("FSRRA : Double precision mode");		

}

//fcnvds <DR_N>,FPUL       
sh4op(i1111_nnnn_1011_1101)
{
	if (fpscr.PR == 1)
	{
		shil_interpret(op);
		return;
	}
	else
	{
		iNimp("fcnvds <DR_N>,FPUL,m=0");
	}
}


//fcnvsd FPUL,<DR_N>       
sh4op(i1111_nnnn_1010_1101)
{

	if (fpscr.PR == 1)
	{
		shil_interpret(op);
		return;
	}
	else
	{
		iNimp("fcnvsd FPUL,<DR_N>,m=0");
	}
}
 
//fipr <FV_M>,<FV_N>            
sh4op(i1111_nnmm_1110_1101)
{
	int n=GetN(op)&0xC;
	int m=(GetN(op)&0x3)<<2;
	ilst->fipr(fr[n],fr[m]);
	bb->flags.FpuIsVector=true;
}


//fldi0 <FREG_N>           
sh4op(i1111_nnnn_1000_1101)
{
	if (fpscr.PR==0)
	{
		//iNimp("fldi0 <FREG_N>");
		u32 n = GetN(op);

		//fr[n] = 0.0f;
		ilst->mov(fr[n],0);//0.0f is 0x0 , right ?
	}
	else
	{
		iNimp("fldi0 <Dreg_N>");
	}
}


//fldi1 <FREG_N>           
sh4op(i1111_nnnn_1001_1101)
{
	if (fpscr.PR==0)
	{
		//iNimp("fldi1 <FREG_N>");
		u32 n = GetN(op);

		//fr[n] = 1.0f;
		ilst->mov(fr[n],0x3f800000);//1.0f is 0x3f800000 , right ?
	}
	else
	{
		iNimp("fldi1 <Dreg_N>");
	}
}


//flds <FREG_N>,FPUL       
sh4op(i1111_nnnn_0001_1101)
{
	//iNimp("flds <FREG_N>,FPUL");
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		//fpul = fr_hex[n];
		ilst->mov(reg_fpul,fr[n]);
	}
	else
	{
		iNimp("flds <DREG_N>,FPUL");
	}
}


//float FPUL,<FREG_N>      
sh4op(i1111_nnnn_0010_1101)
{//TODO : CHECK THIS (FP)

	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		//fr[n] = (float)(int)fpul;
		ilst->floatfpul(fr[n]);
		//shil_interpret(op);
	}
	else
	{
		shil_interpret(op);
	}
}

//ftrc <FREG_N>, FPUL      
sh4op(i1111_nnnn_0011_1101)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		//fpul = (u32)(s32)fr[n];
		ilst->ftrc(fr[n]);
		//shil_interpret(op);
	}
	else
	{
		shil_interpret(op);
	}
}


//fneg <FREG_N>            
sh4op(i1111_nnnn_0100_1101)
{
	u32 n = GetN(op);

	if (fpscr.PR ==0)
	{
		ilst->fneg(fr[n]);
	}
	else
	{
		ilst->fneg(dr[n>>1]);
	}
}


//frchg                    
sh4op(i1111_1011_1111_1101)
{
	shil_interpret(op);
	return;

	//iNimp("frchg");
 	//fpscr.FR = 1 - fpscr.FR;

	//UpdateFPSCR();
}


//fschg                    
sh4op(i1111_0011_1111_1101)
{

	shil_interpret(op);
	return;

	//iNimp("fschg");
	//fpscr.SZ = 1 - fpscr.SZ;
	//UpdateFPSCR();//*FixME* prob not needed
}

//fsqrt <FREG_N>                
sh4op(i1111_nnnn_0110_1101)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);

		//fr[n] = (float)sqrt((double)fr[n]);
		//CHECK_FPU_32(fr[n]);
		ilst->fsqrt(fr[n]);
	}
	else
	{
		//Operation _can_ be done on sh4
		iNimp("fsqrt <DREG_N>");
	}
}




//fsts FPUL,<FREG_N>       
sh4op(i1111_nnnn_0000_1101)
{
	//iNimp("fsts FPUL,<FREG_N>");
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		//fr_hex[n] = fpul;
		ilst->mov(fr[n],reg_fpul);
	}
	else
	{
		iNimp("fsts FPUL,<DREG_N>");
	}
}

//fmac <FREG_0>,<FREG_M>,<FREG_N> 
sh4op(i1111_nnnn_mmmm_1110)
{
	//iNimp("fmac <FREG_0>,<FREG_M>,<FREG_N>");
	if (fpscr.PR==0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		//fr[n] += fr[0] * fr[m];
		ilst->fmac(fr[n],fr[m]);
		//CHECK_FPU_32(fr[n]);
	}
	else
	{
		iNimp("fmac <DREG_0>,<DREG_M>,<DREG_N>");
	}
}


//ftrv xmtrx,<FV_N>       
sh4op(i1111_nn01_1111_1101)
{
	u32 n=GetN(op)&0xC;
	ilst->ftrv(fr[n]);
	bb->flags.FpuIsVector=true;
}																				  


sh4op(icpu_nimp)
{
	shil_interpret(op);
}


//Branches

void DoDslot(u32 pc,BasicBlock* bb)
{
	u16 opcode=ReadMem16(pc+2);

	/*if ((opcode&0xF000)==0xF000)
		ilst->shil_ifb(opcode,pc+2);
	else*/
		RecOpPtr[opcode](opcode,pc+2,bb);
		bb->flags.HasDelaySlot=true;
}

//braf <REG_N>                  
sh4op(i0000_nnnn_0010_0011)
{
	u32 n = GetN(op);
	/*
	u32 newpc = r[n] + pc + 2;//pc +2 is done after
	ExecuteDelayslot();	//WARN : r[n] can change here
	pc = newpc;
	*/

	//shil_interpret(op);
	//ilst->add(reg_pc,2);
	//return;
	bb->flags.ExitType = BLOCK_EXITTYPE_DYNAMIC;
	bb->flags.EndAnalyse=true;
	ilst->mov(reg_pc_temp,r[n]);
	ilst->add(reg_pc_temp,pc+4);
	DoDslot(pc,bb);
	ilst->mov(reg_pc,reg_pc_temp);
} 
//bsrf <REG_N>                  
 sh4op(i0000_nnnn_0000_0011)
{
	u32 n = GetN(op);
	/*
	u32 newpc = r[n] + pc +2;//pc +2 is done after
	pr = pc + 4;		   //after delayslot
	ExecuteDelayslot();	//WARN : pr and r[n] can change here
	pc = newpc;

	AddCall(pr-4,pr,pc,0);
	*/

	//shil_interpret(op);
	//ilst->add(reg_pc,2);
	//return;

	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_DYNAMIC_CALL;
	ilst->mov(reg_pr,pc+4);
	ilst->mov(reg_pc_temp,r[n]);
	ilst->add(reg_pc_temp,pc+4);
	DoDslot(pc,bb);
	ilst->mov(reg_pc,reg_pc_temp);
	bb->TT_next_addr=pc+4;
} 


 //rte                           
 sh4op(i0000_0000_0010_1011)
{
	/*
	//iNimp("rte");
	sr.SetFull(ssr);
	u32 newpc = spc;//+2 is added after instruction
	ExecuteDelayslot();
	pc = newpc -2;
	RemoveCall(spc,1);
	UpdateSR();
	*/
	shil_interpret(op);
	ilst->add(reg_pc,2);
	return;
} 


//rts                           
 sh4op(i0000_0000_0000_1011)
{
	/*
	//TODO Check new delay slot code [28/1/06]
	u32 newpc=pr;//+2 is added after instruction
	ExecuteDelayslot();	//WARN : pr can change here
	pc=newpc-2;
	RemoveCall(pr,0);
	*/
	//shil_interpret(op);
	//ilst->add(reg_pc,2);
	//return;
	bb->flags.EndAnalyse = true;
	bb->flags.ExitType= BLOCK_EXITTYPE_RET;
	ilst->mov(reg_pc_temp,reg_pr);
	DoDslot(pc,bb);
	ilst->mov(reg_pc,reg_pc_temp);
} 


// bf <bdisp8>                   
 sh4op(i1000_1011_iiii_iiii)
{//ToDo : Check Me [26/4/05]  | Check DELAY SLOT [28/1/06]
	/*
	if (sr.T==0)
	{
		//direct jump
		pc = (u32)((GetSImm8(op))*2 + 4 + pc );
		pc-=2;
	}
	*/
	
	bb->TF_next_addr=pc+2;
	bb->TT_next_addr=(u32)((GetSImm8(op))*2 + 4 + pc );

	ilst->LoadT(jcond_flag);
	
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_COND_0;
}


// bf.s <bdisp8>                 
 sh4op(i1000_1111_iiii_iiii)
{//TODO : Check This [26/4/05] | Check DELAY SLOT [28/1/06]
	/*
	if (sr.T==0)
	{
		//delay 1 instruction
		u32 newpc=(u32) ( (GetSImm8(op)<<1) + pc+2);//pc+2 is done after
		ExecuteDelayslot();
		pc = newpc;
	}
	*/
	//shil_interpret(op);
	//return;
	/*shil_interpret(op);
	ilst->add(reg_pc,2);
	return;*/

	bb->TF_next_addr=pc+4;
	bb->TT_next_addr=(u32)((GetSImm8(op))*2 + 4 + pc );

	ilst->LoadT(jcond_flag);
	DoDslot(pc,bb);
	bb->flags.EndAnalyse = true;
	bb->flags.ExitType = BLOCK_EXITTYPE_COND_0;
}


// bt <bdisp8>                   
 sh4op(i1000_1001_iiii_iiii)
{//TODO : Check This [26/4/05]  | Check DELAY SLOT [28/1/06]
	/*
	if (sr.T==1)
	{
		//direct jump
		pc = (u32) ( (GetSImm8(op)<<1) + pc+2);
	}
	*/


	bb->TF_next_addr=pc+2;
	bb->TT_next_addr=(u32)((GetSImm8(op))*2 + 4 + pc );

	ilst->LoadT(jcond_flag);
	
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_COND_1;
}


// bt.s <bdisp8>                 
 sh4op(i1000_1101_iiii_iiii)
{
	/*
	if (sr.T == 1)
	{//TODO : Check This [26/4/05]  | Check DELAY SLOT [28/1/06]
		//delay 1 instruction
		u32 newpc=(u32) ( (GetSImm8(op)<<1) + pc+2);//pc+2 is done after
		ExecuteDelayslot();
		pc = newpc;
	}
	*/
	/*shil_interpret(op);	
	ilst->add(reg_pc,2);
	return;*/

	bb->TF_next_addr=pc+4;
	bb->TT_next_addr=(u32)((GetSImm8(op))*2 + 4 + pc );

	ilst->LoadT(jcond_flag);
	DoDslot(pc,bb);
	
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_COND_1;
}



// bra <bdisp12>
sh4op(i1010_iiii_iiii_iiii)
{//ToDo : Check Me [26/4/05] | Check ExecuteDelayslot [28/1/06] 
	//delay 1 jump imm12
	/*
	u32 newpc =(u32) ((  ((s16)((GetImm12(op))<<4)) >>3)  + pc + 4);//(s16<<4,>>4(-1*2))
	ExecuteDelayslot();
	pc=newpc-2;
	*/

	bb->TF_next_addr=(u32) ((  ((s16)((GetImm12(op))<<4)) >>3)  + pc + 4);
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_FIXED;
	DoDslot(pc,bb);
}
// bsr <bdisp12>
sh4op(i1011_iiii_iiii_iiii)
{//ToDo : Check Me [26/4/05] | Check new delay slot code [28/1/06]
	/*
	//iNimp("bsr <bdisp12>");
	u32 disp = GetImm12(op);
	pr = pc + 4;
	//delay 1 opcode
	u32 newpc = (u32)((((s16)(disp<<4)) >> 3) + pc + 4);
	AddCall(pc,pr,newpc,0);	//WARN : pr can change here
	ExecuteDelayslot();
	pc=newpc-2;*/

	bb->TF_next_addr=(u32) ((  ((s16)((GetImm12(op))<<4)) >>3)  + pc + 4);
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_FIXED_CALL;

	ilst->mov(reg_pr,pc+4);
	DoDslot(pc,bb);
	bb->TT_next_addr=pc+4;
}

// trapa #<imm>                  
sh4op(i1100_0011_iiii_iiii)
{
	/*
	CCN_TRA = (GetImm8(op) << 2);
	Do_Exeption(0,0x160,0x100);
	*/
	shil_interpret(op);	
	ilst->add(reg_pc,2);
	return;
}
//jmp @<REG_N>                  
 sh4op(i0100_nnnn_0010_1011)
{   //ToDo : Check Me [26/4/05] | Check new delay slot code [28/1/06]
	u32 n = GetN(op);
	/*
	//delay 1 instruction
	u32 newpc=r[n];
	ExecuteDelayslot();
	pc=newpc-2;//+2 is done after

	*/
	//shil_interpret(op);	
	//ilst->add(reg_pc,2);
	//return;
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_DYNAMIC;
	ilst->mov(reg_pc_temp,r[n]);
	DoDslot(pc,bb);
	ilst->mov(reg_pc,reg_pc_temp);
}


//jsr @<REG_N>                  
 sh4op(i0100_nnnn_0000_1011)
{//ToDo : Check This [26/4/05] | Check new delay slot code [28/1/06]
	u32 n = GetN(op);
	
	/*
	pr = pc + 4;
	//delay one
	u32 newpc= r[n];
	ExecuteDelayslot();	//WARN : pr can change here
	AddCall(pc-2,pr,newpc,0);
	pc=newpc-2;
	*/
	//shil_interpret(op);	
	//ilst->add(reg_pc,2);
	//return;
	bb->flags.EndAnalyse=true;
	bb->flags.ExitType=BLOCK_EXITTYPE_DYNAMIC_CALL;
	ilst->mov(reg_pr,pc+4);
	ilst->mov(reg_pc_temp,r[n]);
	DoDslot(pc,bb);
	ilst->mov(reg_pc,reg_pc_temp);
	bb->TT_next_addr=pc+4;
}



//sh4_bpt_op => breakpoint opcode
sh4op(sh4_bpt_op)
{
	shil_interpret(op);
}
//sleep                         
 sh4op(i0000_0000_0001_1011)
{
	/*
	//iNimp("Sleep");
	//just wait for an Interrupt
	//while on sleep the precition of Interrupt timing is not the same as when cpu is running :)
	sh4_sleeping=true;
	int i=0,s=1;

	pc+=2;//so that Interrupt return is on next opcode
	while (!UpdateSystem(2500))
	{
		if (i++>100)
		{
			s=0;
			break;
		}
	}
	//if not Interrupted , we must rexecute the sleep
	if (s==0)
		pc-=2;// re execute sleep
	
	pc-=2;//+2 is applied after opcode


	sh4_sleeping=false;
	*/
	shil_interpret(op);	
	ilst->add(reg_pc,2);
	return;
} 

bool __fastcall Scanner_FindSOM(u32 opcode,u32 pc,u32* SOM)
{ 
	if (opcode ==0x0019)
	{
		//possible div0u
		//i0000_0000_0001_1001  
		*SOM=128;
		return MatchDiv32u(opcode,pc);
	}
	else if ((opcode & 0xF00F)==0x2007)
	{
		//possible div0s
		//i0010_nnnn_mmmm_0111  
		*SOM=128;
		return MatchDiv32s(opcode,pc);
	}

	return false;
}
#define notshit
#ifdef notshit
//ok , all the opcodes to here are hand writen for the rec
//time for the compiler fun 
//>:D

#undef r
#undef r_bank

struct shil_RecRegType
{
	Sh4RegType regid;
	//SUB
	void operator-=(const u32 constv)
	{
		ilst->sub(regid,constv);
	};
	void operator-=(const shil_RecRegType& reg)
    {
		ilst->sub(regid,reg.regid);
    }

	void operator--(int wtf)
	{
		(*this)-=1;
	}
	//ADD
	void operator+=(const u32 constv)
	{
		ilst->add(regid,constv);
	};
	void operator+=(const shil_RecRegType& reg)
    {
       ilst->add(regid,reg.regid);
    }
	void operator++(int wtf)
	{
		(*this)+=1;
	}
	//MOVS
	void operator=(const u32 constv)
    {
		 ilst->mov(regid,constv);
    }
	void operator=(const s32 constv)
    {
		(*this)=(u32)constv;
    }
	void operator=(const shil_RecRegType& reg)
    {
		ilst->mov(regid,reg.regid);	
    }
	//AND
	void operator&=(const u32 constv)
	{
		ilst->and(regid,constv);
	};
	void operator&=(const shil_RecRegType& reg)
	{
		ilst->and(regid,reg.regid);
	};
	//OR
	void operator|=(const u32 constv)
	{
		ilst->or(regid,constv);
	};
	void operator|=(const shil_RecRegType& reg)
	{
		ilst->or(regid,reg.regid);
	};
	//XOR
	void operator^=(const u32 constv)
	{
		ilst->xor(regid,constv);
	};
	void operator^=(const shil_RecRegType& reg)
	{
		ilst->xor(regid,reg.regid);
	};
	//SHIFT RIGHT
	void operator>>=(const u32 constv)
	{
		ilst->shr(regid,(u8)constv);
	};

	//SHIFT LEFT
	void operator<<=(const u32 constv)
	{
		ilst->shl(regid,(u8)constv);
	};
};


//Read Mem macros
#define ReadMemU32(to,addr)					ReadMemRec(to,addr,0,4)//to=ReadMem32(addr)
#define ReadMemS16(to,addr)					ReadMemRecS(to,addr,0,2)// to=(u32)(s32)(s16)ReadMem16(addr)
#define ReadMemS8(to,addr)					ReadMemRecS(to,addr,0,1)//to=(u32)(s32)(s8)ReadMem8(addr)

//Base,offset format
#define ReadMemBOU32(to,addr,offset)		ReadMemRec(to,addr,offset,4)//ReadMemU32(to,addr+offset)
#define ReadMemBOS16(to,addr,offset)		ReadMemRecS(to,addr,offset,2)//ReadMemS16(to,addr+offset)
#define ReadMemBOS8(to,addr,offset)			ReadMemRecS(to,addr,offset,1)//ReadMemS8(to,addr+offset)

//Write Mem Macros
#define WriteMemU32(addr,data)				WriteMemRec(addr,0,data,4)//WriteMem32(addr,(u32)data)
#define WriteMemU16(addr,data)				WriteMemRec(addr,0,data,2)//WriteMem16(addr,(u16)data)
#define WriteMemU8(addr,data)				WriteMemRec(addr,0,data,1)//WriteMem8(addr,(u8)data)

//Base,offset format
#define WriteMemBOU32(addr,offset,data)		WriteMemRec(addr,offset,data,4)//WriteMemU32(addr+offset,data)
#define WriteMemBOU16(addr,offset,data)		WriteMemRec(addr,offset,data,2)//WriteMemU16(addr+offset,data)
#define WriteMemBOU8(addr,offset,data)		WriteMemRec(addr,offset,data,1)//WriteMemU8(addr+offset,data)


void ReadMemRec(shil_RecRegType &to,u32 addr,u32 offset,u32 sz)
{
	assert(sz==4);
	ilst->readm32(to.regid,addr+offset);
}

void ReadMemRec(shil_RecRegType &to,shil_RecRegType& addr,u32 offset,u32 sz)
{
	assert(sz==4);
	ilst->readm32(to.regid,addr.regid,offset);
}


void ReadMemRec(shil_RecRegType &to,shil_RecRegType& addr,shil_RecRegType& offset,u32 sz)
{
	assert(sz==4);
	ilst->readm32(to.regid,addr.regid,offset.regid);
}

//signed
void ReadMemRecS(shil_RecRegType &to,u32 addr,u32 offset,u32 sz)
{
	assert(sz!=4);
	if (sz==1)
		ilst->readm8(to.regid,addr+offset);
	else
		ilst->readm16(to.regid,addr+offset);

}
void ReadMemRecS(shil_RecRegType &to,shil_RecRegType& addr,u32 offset,u32 sz)
{
	assert(sz!=4);
	if (sz==1)
		ilst->readm8(to.regid,addr.regid,offset);
	else
		ilst->readm16(to.regid,addr.regid,offset);
}


void ReadMemRecS(shil_RecRegType &to,shil_RecRegType& addr,shil_RecRegType& offset,u32 sz)
{
	assert(sz!=4);
	if (sz==1)
		ilst->readm8(to.regid,addr.regid,offset.regid);
	else
		ilst->readm16(to.regid,addr.regid,offset.regid);
}
//WriteMem(u32 addr,u32 data,u32 sz)
void WriteMemRec(shil_RecRegType& addr,u32 offset,shil_RecRegType &data,u32 sz)
{
	if (sz==1)
		ilst->writem8(data.regid,addr.regid,offset);
	else if (sz==2)
		ilst->writem16(data.regid,addr.regid,offset);
	else
		ilst->writem32(data.regid,addr.regid,offset);
}
void WriteMemRec(shil_RecRegType& addr,shil_RecRegType& offset,shil_RecRegType &data,u32 sz)
{
	if (sz==1)
		ilst->writem8(data.regid,addr.regid,offset.regid);
	else if (sz==2)
		ilst->writem16(data.regid,addr.regid,offset.regid);
	else
		ilst->writem32(data.regid,addr.regid,offset.regid);
}




shil_RecRegType shil_rec_r[16];
shil_RecRegType shil_rec_r_bank[8];

shil_RecRegType shil_rec_gbr,shil_rec_ssr,shil_rec_spc,shil_rec_sgr,shil_rec_dbr,shil_rec_vbr;
shil_RecRegType shil_rec_mach,shil_rec_macl,shil_rec_pr,shil_rec_fpul;


void shil_DynarecInit()
{
	for (int i=0;i<8;i++)
	{
		dyna_reg_id_fr[i]=(Sh4RegType)(fr_0+i);
		dyna_reg_id_xf[i]=(Sh4RegType)(xf_0+i);
		dyna_reg_id_dr[i]=(Sh4RegType)(dr_0+i);
		dyna_reg_id_xd[i]=(Sh4RegType)(xd_0+i);

		shil_rec_r[i].regid=dyna_reg_id_r[i]=(Sh4RegType)(r0+i);
		shil_rec_r_bank[i].regid=dyna_reg_id_r_bank[i]=(Sh4RegType)(r0_Bank+i);
	}

	for (int i=8;i<16;i++)
	{
		dyna_reg_id_fr[i]=(Sh4RegType)(fr_0+i);
		dyna_reg_id_xf[i]=(Sh4RegType)(xf_0+i);

		shil_rec_r[i].regid=dyna_reg_id_r[i]=(Sh4RegType)(r0+i);
	}

	shil_rec_gbr.regid=reg_gbr;
	shil_rec_ssr.regid=reg_ssr;
	shil_rec_spc.regid=reg_spc;
	shil_rec_sgr.regid=reg_sgr;
	shil_rec_dbr.regid=reg_dbr;
	shil_rec_vbr.regid=reg_vbr;

	shil_rec_mach.regid=reg_mach;
	shil_rec_macl.regid=reg_macl;
	shil_rec_pr.regid=reg_pr;
	shil_rec_fpul.regid=reg_fpul;
}
//rename shit

#define UpdateFPSCR rec_UpdateFPSCR
#define UpdateSR rec_UpdateSR

#define r		shil_rec_r
#define r_bank	shil_rec_r_bank
#define gbr		shil_rec_gbr
#define ssr		shil_rec_ssr
#define spc		shil_rec_spc
#define sgr		shil_rec_sgr
#define dbr		shil_rec_dbr
#define vbr		shil_rec_vbr
#define mach	shil_rec_mach
#define macl	shil_rec_macl

#define pr		shil_rec_pr
#define fpul	shil_rec_fpul

//nice names ehh ? make us sure we don't use em :)
#define sr		a_not_defined_name_has_to_be_rec_sr
#define fpscr	a_not_defined_name_has_to_be_rec_fpscr

#undef iNimp
#define iNimp(op,info) rec_shil_iNimp(pc,op,info)

#include "dc\sh4\sh4_cpu_arith.h"
#include "dc\sh4\sh4_cpu_branch.h"
#include "dc\sh4\sh4_cpu_logic.h"
#include "dc\sh4\sh4_cpu_movs.h"
#endif