#include "Types.h"
#include "sh4_registers.h"

u32 r[16];
//u32* ro=r;

u32 r_bank[8];
//u32* rb=r_bank;

u32 gbr,ssr,spc,sgr,dbr,vbr;
u32 mach,macl,pr,fpul;
u32 pc;
//u16* pc_ptr;

StatusReg sr;

fpscr_type fpscr;

f32 xf[16],fr[16];

u32*  xf_hex=(u32*)xf,*fr_hex=(u32*)fr;

StatusReg old_sr;
fpscr_type old_fpscr;
INLINE void ChangeGPR()
{
	u32 temp[8];
	for (int i=0;i<8;i++)
	{
		temp[i]=r[i];
		r[i]=r_bank[i];
		r_bank[i]=temp[i];
	}
}

INLINE void ChangeFP()
{
	u32 temp[16];
	for (int i=0;i<16;i++)
	{
		temp[i]=fr_hex[i];
		fr_hex[i]=xf_hex[i];
		xf_hex[i]=temp[i];
	}
}

#include "sh4_cst.h"
//called when sr is changed and we must check for rom banks ect.. , returns true if interrupts got 
bool UpdateSR()
{
	if (sr.MD)
	{
		if (old_sr.RB !=sr.RB)
			ChangeGPR();//bank change
	}
	else
	{
		if (sr.RB)
		{
			printf("UpdateSR MD=0;RB=1 , this must not happen\n");
			sr.RB =0;//error - must allways be 0
			if (old_sr.RB)
				ChangeGPR();//switch
		}
		else
		{
			if (old_sr.RB)
				ChangeGPR();//switch
		}
	}
	if ((old_sr.IMASK!=0xF) && (sr.IMASK==0xF))
	{
		//printf("Interrupts disabled  , pc=0x%X\n",pc);
	}

	if ((old_sr.IMASK==0xF) && (sr.IMASK!=0xF))
	{
		//printf("Interrupts enabled  , pc=0x%X\n",pc);
	}
	bool rv=old_sr.IMASK > sr.IMASK;
	old_sr.full=sr.full;

	return rv;
}

//called when fpscr is changed and we must check for rom banks ect..
void UpdateFPSCR()
{
	if (fpscr.FR !=old_fpscr.FR)
		ChangeFP();//fpu bank change
	old_fpscr=fpscr;
}

#ifdef DEBUG
f64 GetDR(u32 n)
{
#ifdef TRACE
	if (n>7)
		printf("DR_r INDEX OVERRUN %d >7",n);
#endif
	double t;
	((u32*)(&t))[1]=fr_hex[(n<<1) | 0];
	((u32*)(&t))[0]=fr_hex[(n<<1) | 1];
	return t;
}

f64 GetXD(u32 n)
{
#ifdef TRACE
	if (n>7)
		printf("XD_r INDEX OVERRUN %d >7",n);
#endif
	double t;
	((u32*)(&t))[1]=xf_hex[(n<<1) | 0];
	((u32*)(&t))[0]=xf_hex[(n<<1) | 1];
	return t;
}

void SetDR(u32 n,f64 val)
{
#ifdef TRACE
	if (n>7)
		printf("DR_w INDEX OVERRUN %d >7",n);
#endif
	fr_hex[(n<<1) | 1]=((u32*)(&val))[0];
	fr_hex[(n<<1) | 0]=((u32*)(&val))[1];
}

void SetXD(u32 n,f64 val)
{
#ifdef TRACE
	if (n>7)
		printf("XD_w INDEX OVERRUN %d >7",n);
#endif

	xf_hex[(n<<1) | 1]=((u32*)(&val))[0];
	xf_hex[(n<<1) | 0]=((u32*)(&val))[1];
}
#endif

u32* Sh4_int_GetRegisterPtr(Sh4RegType reg)
{
	if ((reg>=r0) && (reg<=r15))
	{
		return &r[reg-r0];
	}
	else if ((reg>=r0_Bank) && (reg<=r7_Bank))
	{
		return &r_bank[reg-r0_Bank];
	}
	else if ((reg>=fr_0) && (reg<=fr_15))
	{
		return &fr_hex[reg-fr_0];
	}
	else if ((reg>=xf_0) && (reg<=xf_15))
	{
		return &xf_hex[reg-xf_0];
	}
	else
	{
		switch(reg)
		{
		case Sh4RegType::reg_gbr :
			return &gbr;
			break;
		case Sh4RegType::reg_vbr :
			return &vbr;
			break;

		case Sh4RegType::reg_ssr :
			return &ssr;
			break;

		case Sh4RegType::reg_spc :
			return &spc;
			break;

		case Sh4RegType::reg_sgr :
			return &sgr;
			break;

		case Sh4RegType::reg_dbr :
			return &dbr;
			break;

		case Sh4RegType::reg_mach :
			return &mach;
			break;

		case Sh4RegType::reg_macl :
			return &macl;
			break;

		case Sh4RegType::reg_pr :
			return &pr;
			break;

		case Sh4RegType::reg_fpul :
			return &fpul;
			break;


		case Sh4RegType::reg_pc :
			return &pc;
			break;

		case Sh4RegType::reg_sr :
			return &sr.full;
			break;
		case Sh4RegType::reg_fpscr :
			return &fpscr.full;
			break;


		default:
			EMUERROR2("Unkown register Id %d",reg);
			return 0;
			break;
		}
	}
}
