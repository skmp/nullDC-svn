#pragma once
#include "..\..\types.h"
#include "sh4_if.h"

extern u32 r[16];
extern u32 r_bank[8];

extern u32 gbr,ssr,spc,sgr,dbr,vbr;
extern u32 mach,macl,pr,fpul;
extern u32 pc;

extern StatusReg sr;

extern fpscr_type fpscr;

extern float xf[16],fr[16];

extern u32*  xf_hex,*fr_hex;

void UpdateFPSCR();
void UpdateSR();

#ifndef DEBUG
INLINE f64 GetDR(u32 n)
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

INLINE f64 GetXD(u32 n)
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

INLINE void SetDR(u32 n,f64 val)
{
#ifdef TRACE
	if (n>7)
		printf("DR_w INDEX OVERRUN %d >7",n);
#endif
	fr_hex[(n<<1) | 1]=((u32*)(&val))[0];
	fr_hex[(n<<1) | 0]=((u32*)(&val))[1];
}

INLINE void SetXD(u32 n,f64 val)
{
#ifdef TRACE
	if (n>7)
		printf("XD_w INDEX OVERRUN %d >7",n);
#endif

	xf_hex[(n<<1) | 1]=((u32*)(&val))[0];
	xf_hex[(n<<1) | 0]=((u32*)(&val))[1];
}
#else
f64 GetDR(u32 n);
f64 GetXD(u32 n);
void SetDR(u32 n,f64 val);
void SetXD(u32 n,f64 val);
#endif

extern StatusReg old_sr;
extern fpscr_type old_fpscr;