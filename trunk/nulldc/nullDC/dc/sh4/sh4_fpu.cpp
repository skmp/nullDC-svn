#include "types.h"
#include "sh4_interpreter.h"
#include "sh4_registers.h"

#include "dc/mem/sh4_mem.h"

#include <math.h>
#include <float.h>

#define sh4op(str) void __fastcall str (u32 op)
#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
#define GetImm4(str) ((str>>0) & 0xf)
#define GetImm8(str) ((str>>0) & 0xff)
#define GetImm12(str) ((str>>0) & 0xfff)

#define GetDN(opc)	((op&0x0F00)>>9)
#define GetDM(opc)	((op&0x00F0)>>5)

#define pi (3.14159265f)

void iNimp(char*str);

#define IS_DENORMAL(f) (((*(f))&0x7f800000) == 0)

#define ReadMemU64(to,addr) ReadMem64(addr,to)
#define ReadMemU32(to,addr) to=ReadMem32(addr)
#define ReadMemS32(to,addr) to=(s32)ReadMem32(addr)
#define ReadMemS16(to,addr) to=(u32)(s32)(s16)ReadMem16(addr)
#define ReadMemS8(to,addr) to=(u32)(s32)(s8)ReadMem8(addr)

//Base,offset format
#define ReadMemBOU32(to,addr,offset)	ReadMemU32(to,addr+offset)
#define ReadMemBOS16(to,addr,offset)	ReadMemS16(to,addr+offset)
#define ReadMemBOS8(to,addr,offset)		ReadMemS8(to,addr+offset)

//Write Mem Macros
#define WriteMemU64(addr,data)				WriteMem64(addr,data)
#define WriteMemU32(addr,data)				WriteMem32(addr,(u32)data)
#define WriteMemU16(addr,data)				WriteMem16(addr,(u16)data)
#define WriteMemU8(addr,data)				WriteMem8(addr,(u8)data)

//Base,offset format
#define WriteMemBOU32(addr,offset,data)		WriteMemU32(addr+offset,data)
#define WriteMemBOU16(addr,offset,data)		WriteMemU16(addr+offset,data)
#define WriteMemBOU8(addr,offset,data)		WriteMemU8(addr+offset,data)
#define exept_tet_now {if (sh4_exept_raised) return;}
#ifdef NO_MMU
#define EXEPT_EXIT()
#else
#define EXEPT_EXIT() if (sex) return;
#endif
INLINE void Denorm32(float &value)
{
	if (fpscr.DN)
	{
		u32* v=(u32*)&value;
		if (IS_DENORMAL(v) && (*v&0x7fFFFFFF)!=0)
		{
			*v&=0x80000000;
			//printf("Denromal ..\n");
		}
		if ((*v<=0x007FFFFF) && *v>0)
		{
			*v=0;
			printf("Fixed +denorm\n");
		}
		else if ((*v<=0x807FFFFF) && *v>0x80000000)
		{
			*v=0x80000000;
			printf("Fixed -denorm\n");
		}
	}
}


#define CHECK_FPU_32(v) Denorm32(v)
#define CHECK_FPU_64(v)

#define START64() _controlfp(_PC_53, MCW_PC)
#define END64()   _controlfp(_PC_24, MCW_PC)

#define STARTMODE64() _controlfp(_PC_53, MCW_PC)
#define ENDMODE64()	  _controlfp(_PC_24, MCW_PC)

//all fpu emulation ops :)

	// ZeZu[7/9/05] fpu needs some work ;)
	// need dr and a good way to handle fp stuff as int (union) : You can use fr_hex/xf_hex
	// also need to know about XD ie: sz=1 (DR) but unused N/M bit if 1 = XD (float[4])
	//fpscr is now easy to handle
	// Templates "could" be a very good way to handle DR/XD
	// BTW: you might want to check these, no sleep + heat != good code ;p
	// drkIIRaziel 11/7/05 [dd/mm] replaced fpu emulation with my fpu code :) (buggy most prob 
	// but at least kos demos should work now :) :) :) ) heh

//fadd <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0000)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		fr[n] += fr[m];
		//CHECK_FPU_32(fr[n]);
	}
	else
	{
		u32 n = (op >> 9) & 0x07;
		u32 m = (op >> 5) & 0x07;
		
		START64();
		double drn=GetDR(n), drm=GetDR(m);
		drn += drm;
		CHECK_FPU_64(drn);
		SetDR(n,drn);
		END64();
	}
}

//fsub <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0001)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		fr[n] -= fr[m];
		CHECK_FPU_32(fr[n]);
	}
	else
	{
		u32 n = (op >> 9) & 0x07;
		u32 m = (op >> 5) & 0x07;

		START64();
		double drn=GetDR(n), drm=GetDR(m);
		drn-=drm;
		//dr[n] -= dr[m];
		SetDR(n,drn);
		END64();
	}
}																								
//fmul <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0010)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		fr[n] *= fr[m];
		CHECK_FPU_32(fr[n]);
	}
	else
	{
		u32 n = (op >> 9) & 0x07;
		u32 m = (op >> 5) & 0x07;
		START64();
		double drn=GetDR(n), drm=GetDR(m);
		drn*=drm;
		//dr[n] *= dr[m];
		SetDR(n,drn);
		END64();
	}
}
//fdiv <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_0011)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		
		fr[n] /= fr[m];
		
		CHECK_FPU_32(fr[n]);
	}
	else
	{
		u32 n = (op >> 9) & 0x07;
		u32 m = (op >> 5) & 0x07;
		START64();
		double drn=GetDR(n), drm=GetDR(m);
		drn/=drm;
		SetDR(n,drn);
		END64();
	}
}
//fcmp/eq <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0100)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		sr.T = (fr[m] == fr[n]) ? 1 : 0;
	}
	else
	{
		u32 n = (op >> 9) & 0x07;
		u32 m = (op >> 5) & 0x07;
		START64();
		sr.T = (GetDR(m) == GetDR(n)) ? 1 : 0;
		END64();	
	}
}
//fcmp/gt <FREG_M>,<FREG_N>
sh4op(i1111_nnnn_mmmm_0101)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		if (fr[n] > fr[m])
			sr.T = 1;
		else
			sr.T = 0;
	}
	else
	{
		u32 n = (op >> 9) & 0x07;
		u32 m = (op >> 5) & 0x07;
		
		START64();
		if (GetDR(n) > GetDR(m))
			sr.T = 1;
		else
			sr.T = 0;
		END64();
	}
}
//All memory opcodes are here
//fmov.s @(R0,<REG_M>),<FREG_N>
sh4op(i1111_nnnn_mmmm_0110)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		ReadMemU32(fr_hex[n],r[m] + r[0]);
	}
	else
	{
		u32 n = (op >> 8) & 0x0E;
		u32 m = GetM(op);
		if (((op >> 8) & 0x1) == 0)
		{
			//ReadMemU32(fr_hex[n],r[m] + r[0]);  exept_tet_now;
			//ReadMemU32(fr_hex[n + 1],r[m] + r[0] + 4);
			ReadMemU64(fr_hex[n],r[m] + r[0]);
		}
		else
		{
			//ReadMemU32(xf_hex[n],r[m] + r[0]);  exept_tet_now;
			//ReadMemU32(xf_hex[n + 1],r[m] + r[0] + 4);
			ReadMemU64(xf_hex[n],r[m] + r[0]);
		}
	}
}


//fmov.s <FREG_M>,@(R0,<REG_N>)
sh4op(i1111_nnnn_mmmm_0111)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		WriteMem32(r[0] + r[n], fr_hex[m]);
	}
	else
	{
		u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;
		if (((op >> 4) & 0x1) == 0)
		{
			//WriteMem32(r[n] + r[0], fr_hex[m]);  exept_tet_now;
			//WriteMem32(r[n] + r[0] + 4, fr_hex[m + 1]);
			WriteMemU64(r[n] + r[0],fr_hex[m]);
		}
		else
		{
			//WriteMem32(r[n] + r[0], xf_hex[m]);  exept_tet_now;
			//WriteMem32(r[n] + r[0] + 4, xf_hex[m + 1]);
			WriteMemU64(r[n] + r[0],xf_hex[m]);
		}
	}
}


//fmov.s @<REG_M>,<FREG_N> 
sh4op(i1111_nnnn_mmmm_1000)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		ReadMemU32(fr_hex[n],r[m]);
	}
	else
	{
		u32 n = (op >> 8) & 0x0E;
		u32 m = GetM(op);
		if (((op >> 8) & 0x1) == 0)
		{
			//ReadMemU32(fr_hex[n],r[m]);  exept_tet_now;
			//ReadMemU32(fr_hex[n + 1],r[m] + 4);
			ReadMemU64(fr_hex[n],r[m]);
		}
		else
		{
			//ReadMemU32(xf_hex[n],r[m]);  exept_tet_now;
			//ReadMemU32(xf_hex[n + 1],r[m] + 4);
			ReadMemU64(xf_hex[n],r[m]);
		}
	}
}


//fmov.s @<REG_M>+,<FREG_N>
sh4op(i1111_nnnn_mmmm_1001)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);

		ReadMemU32(fr_hex[n],r[m]);
		r[m] += 4;
	}
	else 
	{
		u32 n = (op >> 8) & 0x0E;
		u32 m = GetM(op);
		if (((op >> 8) & 0x1) == 0)
		{
			//ReadMemU32(fr_hex[n],r[m]); exept_tet_now;
			//ReadMemU32(fr_hex[n + 1],r[m]+ 4);
			ReadMemU64(fr_hex[n],r[m]);
		}
		else
		{
			//ReadMemU32(xf_hex[n],r[m] ); exept_tet_now;
			//ReadMemU32(xf_hex[n + 1],r[m]+ 4);
			ReadMemU64(xf_hex[n],r[m] );
		}
		r[m] += 8;
	}
}


//fmov.s <FREG_M>,@<REG_N>
sh4op(i1111_nnnn_mmmm_1010)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		WriteMemU32(r[n], fr_hex[m]);
	}
	else
	{
		u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;

		if (((op >> 4) & 0x1) == 0)
		{
			//WriteMemU32(r[n], fr_hex[m]); exept_tet_now;
			//WriteMemU32(r[n] + 4, fr_hex[m + 1]);
			WriteMemU64(r[n], fr_hex[m]);
		}
		else
		{
			//WriteMemU32(r[n], xf_hex[m]); exept_tet_now;
			//WriteMemU32(r[n] + 4, xf_hex[m + 1]);
			WriteMemU64(r[n], xf_hex[m]);
		}
		
	}
}

//fmov.s <FREG_M>,@-<REG_N>
sh4op(i1111_nnnn_mmmm_1011)
{
	if (fpscr.SZ == 0)
	{
		//iNimp("fmov.s <FREG_M>,@-<REG_N>");
		u32 n = GetN(op);
		u32 m = GetM(op);

		r[n] -= 4;

		WriteMemU32(r[n], fr_hex[m]);
	}
	else
	{

		u32 n = GetN(op);
		u32 m = (op >> 4) & 0x0E;

		r[n] -= 8;
		if (((op >> 4) & 0x1) == 0)
		{
			//WriteMemU32(r[n] , fr_hex[m]); exept_tet_now;
			//WriteMemU32(r[n]+ 4, fr_hex[m + 1]);
			WriteMemU64(r[n] , fr_hex[m]);
		}
		else
		{
			//WriteMemU32(r[n] , xf_hex[m]); exept_tet_now;
			//WriteMemU32(r[n]+ 4, xf_hex[m + 1]);
			WriteMemU64(r[n] , xf_hex[m]);
		}
	}
}

//end of memory opcodes

//fmov <FREG_M>,<FREG_N>   
sh4op(i1111_nnnn_mmmm_1100)
{
	if (fpscr.SZ == 0)
	{
		u32 n = GetN(op);
		u32 m = GetM(op);
		fr[n] = fr[m];
	}
	else
	{
		u32 n = (op >> 8) & 0xE;
		u32 m = (op >> 4) & 0xE;
		switch ((op >> 4) & 0x11)
		{
			case 0x00:
				//dr[n] = dr[m];
				fr_hex[n] = fr_hex[m];
				fr_hex[n + 1] = fr_hex[m + 1];
				break;
			case 0x01:
				//dr[n] = xf[m];
				fr_hex[n] = xf_hex[m];
				fr_hex[n + 1] = xf_hex[m + 1];
				break;
			case 0x10:
				//xf[n] = dr[m];
				xf_hex[n] = fr_hex[m];
				xf_hex[n + 1] = fr_hex[m + 1];
				break;
			case 0x11:
				//xf[n] = xf[m];
				xf_hex[n] = xf_hex[m];
				xf_hex[n + 1] = xf_hex[m + 1];
				break;
		}
	}
}


//fabs <FREG_N>            
sh4op(i1111_nnnn_0101_1101)
{
	int n=GetN(op);
	
	if (fpscr.PR ==0)
		fr_hex[n]&=0x7FFFFFFF;
	else
		fr_hex[(n&0xE)]&=0x7FFFFFFF;

}

//FSCA FPUL, DRn//F0FD//1111_nnn0_1111_1101
sh4op(i1111_nnn0_1111_1101)
{
//#define MY_PI2 6.283185307179586f
//#define MY_ANG_RAD(k)  ((k) * MY_PI2 / 65536.0f)
	 
	int n=GetN(op) & 0xE;
	 
	
	//cosine(x) = sine(pi/2 + x).
	if (fpscr.PR==0)
	{
		//float real_pi=(((float)(s32)fpul)/65536)*(2*pi);
		u32 pi_index=fpul&0xFFFF;
		
		fr[n | 0] = sin_table[pi_index];//sinf(real_pi);
		fr[n | 1] = sin_table[(16384 + pi_index) & 0xFFFF];//cosf(real_pi);

		CHECK_FPU_32(fr[n]);
		CHECK_FPU_32(fr[n+1]);
	}
	else
	iNimp("FSCA : Double precision mode");
}

//FSRRA //1111_nnnn_0111_1101
sh4op(i1111_nnnn_0111_1101)
{
	// What about double precision?
	u32 n = GetN(op);
	if (fpscr.PR==0)
	{
		fr[n] = (float)(1/sqrt((double)fr[n]));
		CHECK_FPU_32(fr[n]);
	}
	else
		iNimp("FSRRA : Double precision mode");		
}

//fcnvds <DR_N>,FPUL       
sh4op(i1111_nnnn_1011_1101)
{
	
	if (fpscr.PR == 1)
	{
		START64();
		//iNimp("fcnvds <DR_N>,FPUL");
		u32 n = (op >> 9) & 0x07;
		u32*p=&fpul;
		*((float*)p) = (float)GetDR(n);
		//fpul= (int)GetDR(n);
		END64();
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
		START64();
		u32 n = (op >> 9) & 0x07;
		u32* p = &fpul;
		SetDR(n,(double)*((float*)p));
		//SetDR(n,(double)fpul);
		END64();
	}
	else
	{
		iNimp("fcnvsd FPUL,<DR_N>,m=0");
	}
}
 
//fipr <FV_M>,<FV_N>            
sh4op(i1111_nnmm_1110_1101)
{
//	iNimp("fipr <FV_M>,<FV_N>");
 

	int n=GetN(op)&0xC;
	int m=(GetN(op)&0x3)<<2;
	if(fpscr.PR ==0)
	{
		float idp;

		idp=fr[n+0]*fr[m+0];
		idp+=fr[n+1]*fr[m+1];
		idp+=fr[n+2]*fr[m+2];
		idp+=fr[n+3]*fr[m+3];

		CHECK_FPU_32(idp);
		fr[n+3]=idp;
	}
	else
		iNimp("FIPR Precision=1");

	/*
	u32 n = (op >> 8) & 0xC;
	u32 m = ((op >> 8) & 0x3)<<2;
 
	fr[n+3] =	fr[m+0] * fr[n+0] +
				fr[m+1] * fr[n+1] +
				fr[m+2] * fr[n+2] +
				fr[m+3] * fr[n+3];

 
	/*
	union {
		double d;
		int l[2];
	} mlt[4];
	float dstf;
	int i;
 
 
		for(i=0;i<4;i++) {
 
			mlt[i].d = fr[m+i];
			mlt[i].d *= fr[n+i];
			// The multiplication array emulation is necessary for obtaining the
			//same result as that of the FIPR hardware, because the hardware cut
			//lower 18 bits of the array output before carry propagate addition.
			//The following flow is different from the hardware algorism but simple. //
			mlt[i].l[1] &= 0xff000000;
			mlt[i].l[1] |= 0x00800000;
		}
		mlt[0].d += mlt[1].d + mlt[2].d + mlt[3].d;
		mlt[0].l[1] &= 0xff800000;
		dstf =(float) mlt[0].d;
	//	fp_set_I();*/
 
 
 
}


//fldi0 <FREG_N>           
sh4op(i1111_nnnn_1000_1101)
{
	if (fpscr.PR!=0)
		iNimp("fldi0 <Dreg_N>");

	//iNimp("fldi0 <FREG_N>");
	u32 n = GetN(op);

	fr[n] = 0.0f;

}


//fldi1 <FREG_N>           
sh4op(i1111_nnnn_1001_1101)
{
	if (fpscr.PR!=0)
		iNimp("fldi1 <Dreg_N>");

	//iNimp("fldi1 <FREG_N>");
	u32 n = GetN(op);

	fr[n] = 1.0f;
}


//flds <FREG_N>,FPUL       
sh4op(i1111_nnnn_0001_1101)
{
	//iNimp("flds <FREG_N>,FPUL");
	/*if (fpscr.PR != 0)
		iNimp("flds <DREG_N>,FPUL");*/

	u32 n = GetN(op);

	fpul = fr_hex[n];
}
//fsts FPUL,<FREG_N>       
sh4op(i1111_nnnn_0000_1101)
{
	//iNimp("fsts FPUL,<FREG_N>");
	/*if (fpscr.PR != 0)
		iNimp("fsts FPUL,<DREG_N>");*/

	u32 n = GetN(op);
	fr_hex[n] = fpul;
}

//float FPUL,<FREG_N>      
sh4op(i1111_nnnn_0010_1101)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		fr[n] = (float)(int)fpul;
	}
	else
	{
		START64();
		u32 n = (op >> 9) & 0x07;
		SetDR(n, (double)(int)fpul);
		//iNimp("float FPUL,<DREG_N>");
		END64();
	}
}


//fneg <FREG_N>            
sh4op(i1111_nnnn_0100_1101)
{
	u32 n = GetN(op);

	if (fpscr.PR ==0)
		fr_hex[n]^=0x80000000;
	else
		fr_hex[(n&0xE)]^=0x80000000;
}


//frchg                    
sh4op(i1111_1011_1111_1101)
{
	//iNimp("frchg");
 	fpscr.FR = 1 - fpscr.FR;

	UpdateFPSCR();
}

//fschg                    
sh4op(i1111_0011_1111_1101)
{
	//iNimp("fschg");
	fpscr.SZ = 1 - fpscr.SZ;
	//printf("SZ %d %08X\n",fpscr.SZ,id);
}

//fsqrt <FREG_N>                
sh4op(i1111_nnnn_0110_1101)
{
	if (fpscr.PR == 0)
	{
		//iNimp("fsqrt <FREG_N>");
		u32 n = GetN(op);

		fr[n] = sqrt(fr[n]);
		CHECK_FPU_32(fr[n]);
	}
	else
	{
		//Operation _can_ be done on sh4
		u32 n = GetN(op)>>1;

		START64();
		SetDR(n,sqrt(GetDR(n)));
		//CHECK_FPU_32(fr[n]);
		//iNimp("fsqrt <DREG_N>");
		END64();
	}
}


//ftrc <FREG_N>, FPUL      
sh4op(i1111_nnnn_0011_1101)
{
	if (fpscr.PR == 0)
	{
		u32 n = GetN(op);
		fpul = (u32)(s32)fr[n];

		if (fpul==0x80000000)
		{
			if (fr[n]>0)
				fpul--;
		}
	}
	else
	{
		START64();
		u32 n = (op >> 9) & 0x07;
		fpul = (u32)(s32)GetDR(n);
		if (fpul==0x80000000)
		{
			if (GetDR(n)>0)
				fpul--;
		}
		END64();
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

		fr[n] += fr[0] * fr[m];
		CHECK_FPU_32(fr[n]);
	}
	else
	{
		iNimp("fmac <DREG_0>,<DREG_M>,<DREG_N>");
	}
}

INLINE float ftrv_dot_prod_1 (u8 val,u8 n)
{

	float frrez = 0.0f; //d = 0.0f;

	for (int i = 0; i < 4; i++) 
	{
		frrez+=xf[val+(i<<2)]*fr[n+i];
	}

	//fp_set_I();
	return frrez;
}

//ftrv xmtrx,<FV_N>       
sh4op(i1111_nn01_1111_1101)
{
	//iNimp("ftrv xmtrx,<FV_N>");



	/*
	XF[0] XF[4] XF[8] XF[12]	FR[n]			FR[n]
	XF[1] XF[5] XF[9] XF[13]  *	FR[n+1]		->	FR[n+1]
	XF[2] XF[6] XF[10] XF[14]	FR[n+2]			FR[n+2]
	XF[3] XF[7] XF[11] XF[15]	FR[n+3]			FR[n+3]
	fucking *nih* maths ....*/

	u32 n=GetN(op)&0xC;
	
	if (fpscr.PR==0)
	{
		float tmp[4];
		//double tmp[4];
		for (u8 i=0;i<4;i++)
			tmp[i]=ftrv_dot_prod_1(i,(u8)n);
		for (u8 i=0;i<4;i++)
			fr[n+i]=(float)tmp[i];
	}
	else
		iNimp("FTRV in dp mode");



	/* matrix:
											XF0		XF4		XF8		XF12
											XF1		XF5		XF9		XF13
											XF2		XF6		XF10	XF14
											XF3		XF7		XF11	XF15 */
	/*
	float v1, v2, v3, v4;
	
	v1 = xf[0] * fr[ n + 0] +
		xf[4] * fr[n + 1] +
		xf[8] * fr[n + 2] +
		xf[12] * fr [n + 3];
	
	v2 = xf[1] * fr[ n + 0] +
		xf[5] * fr[ n + 1] +
		xf[9] * fr[ n + 2] +
		xf[13] * fr[ n + 3];
	
	v3 = xf[2] * fr[ n + 0] +
		xf[6] * fr[n + 1] +
		xf[10] * fr[ n + 2] +
		xf[14] * fr[ n + 3];

	v4 = xf[3] * fr[ n + 0] +
		xf[7] * fr[n + 1] +
		xf[11] * fr[ n + 2]+
		xf[15] *
		fr[ n + 3];

	CHECK_FPU_32(v1);
	CHECK_FPU_32(v2);
	CHECK_FPU_32(v3);
	CHECK_FPU_32(v4);

	fr[n + 0] = v1;
	fr[n + 1] = v2;
	fr[n + 2] = v3;
	fr[n + 3] = v4;*/
}																				  


void iNimp(char*str)
{
	printf("Unimplemented sh4 fpu instruction: ");
	printf(str);
	printf("\n");
	
	//Sh4_int_Stop();
}