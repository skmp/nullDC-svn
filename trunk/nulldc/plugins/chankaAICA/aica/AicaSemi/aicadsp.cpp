#include "aicadsp.h"
#include "assert.h"
#include "memory.h"
#include "stdio.h"
#include "malloc.h"

#pragma warning(disable:4311)

#define DYNBUF	0x10000

//#define USEFLOATPACK

//the PACK func in asm plus mov [esi],ax
unsigned char PackFunc[]={0x8B,0xD8,0xA9,0x00,0x00,0x80,0x00,0x75,0x02,0xF7,0xD3,0xF7,0xD3,0x0F,0xBD,0xCB,
						  0xF7,0xD9,0xC1,0xE0,0x08,0x83,0xC1,0x16,0xD3,0xE0,0xC1,0xF8,0x13,0xC1,0xE1,0x0B,
						  0x25,0xFF,0x87,0x00,0x00,0x0B,0xC1,0x66,0x89,0x06};
unsigned char UnpackFunc[]={0x8B,0xD8,0x8B,0xC8,0x81,0xE3,0x00,0x80,0x00,0x00,0x25,0xFF,0x07,
							0x00,0x00,0xC1,0xE9,0x0B,0xC1,0xE0,0x0B,0xC1,0xE3,0x08,0x83,0xE1,0x0F,0x0B,0xC3,
							0xD1,0xEB,0x81,0xF3,0x00,0x00,0x40,0x00,0x0B,0xC3,0x83,0xC1,0x08,0xC1,0xE0,0x08,
							0xD3,0xF8};

unsigned short inline PACK(signed int val)
{
/*	signed int v1=val;
	int n=0;
	while(((v1>>22)&1) == ((v1>>23)&1))
	{
		v1<<=1;
		++n;
	}
	v1<<=8;
	v1>>=11+8;
	v1=(v1&(~0x7800))|(n<<11);
	return v1;
*/
#ifdef USEFLOATPACK
	unsigned short f;
	__asm
	{
		mov eax,val
		mov ebx,eax
		test eax,0x00800000
		jne negval
		not ebx
negval:	not ebx
		bsr ecx,ebx
		neg ecx
		shl eax,8
		add ecx,22
		shl eax,cl
		sar eax,8+11
		shl ecx,11
		and eax,~0x7800
		or eax,ecx
		mov f,ax
	}
	return f;
#else

	//cut to 16 bits
	unsigned int f=((unsigned int ) val)>>8;
	return f;
#endif
}

signed int inline UNPACK(unsigned short val)
{
/*	if(val)
		int a=1;
	unsigned int mant=val&0x7ff;
	unsigned int exp=(val>>11)&0xf;
	unsigned int sign=(val>>15)&1;
	signed int r=0;
	r|=mant<<11;
	r|=sign<<23;
	r|=(sign^1)<<22;

	//signed int r=val<<8;
	//if(r&0x00800000)
	//	r|=0xFF000000;
	r<<=8;
	r>>=8+exp;
*/
#ifdef USEFLOATPACK
	signed int r;
	__asm
	{
		xor eax,eax
		mov ax,val
		mov ebx,eax
		mov ecx,eax
		and ebx,0x8000
		and eax,0x07ff
		shr ecx,11
		shl eax,11
		shl ebx,8
		and ecx,0xF
		or eax,ebx
		shr ebx,1
		xor ebx,0x00400000
		or eax,ebx
		add ecx,8
		shl eax,8
		sar eax,cl
		mov r,eax
	}
#else
	//unpack 16->24
	signed int r=val<<8;
	r<<=8;
	r>>=8;
#endif
	return r;
}

void AICADSP_Init(_AICADSP *DSP)
{
	memset(DSP,0,sizeof(_AICADSP));
	DSP->RBL=0x8000;
	DSP->Stopped=true;
	DSP->DSPRAM=(unsigned short*) malloc(0x10000*2);
	memset(DSP->DSPRAM,0,0x10000*2);
}
#ifndef DYNDSP
void AICADSP_Step(_AICADSP *DSP)
{
	if(DSP->Stopped)
		return;
	signed int ACC=0;	//26 bit
	signed int SHIFTED=0;	//24 bit
	signed int X=0;	//24 bit
	signed int Y=0;	//13 bit
	signed int B=0;	//26 bit
	signed int INPUTS=0;	//24 bit
	signed int MEMVAL=0;
	signed int FRC_REG=0;	//13 bit
	signed int Y_REG=0;		//24 bit
	unsigned int ADDR=0;
	unsigned int ADRS_REG=0;	//13 bit

	memset(DSP->EFREG,0,2*16);
	int dump=0;
	FILE *f=NULL;
	if(dump)
		f=fopen("dsp.txt","wt");
	for(int step=0;step</*128*/DSP->LastStep;++step)
	{
		unsigned short *IPtr=DSP->MPRO+step*4;

//		if(IPtr[0]==0 && IPtr[1]==0 && IPtr[2]==0 && IPtr[3]==0)
//			break;

		unsigned int TRA=(IPtr[0]>>9)&0x7F;
		unsigned int TWT=(IPtr[0]>>8)&0x01;
		unsigned int TWA=(IPtr[0]>>1)&0x7F;
		
		unsigned int XSEL=(IPtr[1]>>15)&0x01;
		unsigned int YSEL=(IPtr[1]>>13)&0x03;
		unsigned int IRA=(IPtr[1]>>7)&0x3F;
		unsigned int IWT=(IPtr[1]>>6)&0x01;
		unsigned int IWA=(IPtr[1]>>1)&0x1F;
	
		unsigned int TABLE=(IPtr[2]>>15)&0x01;
		unsigned int MWT=(IPtr[2]>>14)&0x01;
		unsigned int MRD=(IPtr[2]>>13)&0x01;
		unsigned int EWT=(IPtr[2]>>12)&0x01;
		unsigned int EWA=(IPtr[2]>>8)&0x0F;
		unsigned int ADRL=(IPtr[2]>>7)&0x01;
		unsigned int FRCL=(IPtr[2]>>6)&0x01;
		unsigned int SHIFT=(IPtr[2]>>4)&0x03;
		unsigned int YRL=(IPtr[2]>>3)&0x01;
		unsigned int NEGB=(IPtr[2]>>2)&0x01;
		unsigned int ZERO=(IPtr[2]>>1)&0x01;
		unsigned int BSEL=(IPtr[2]>>0)&0x01;

		unsigned int NOFL=(IPtr[3]>>15)&1;		//????
		unsigned int MASA=(IPtr[3]>>9)&0x3f;	//???
		unsigned int ADREB=(IPtr[3]>>8)&0x1;
		unsigned int NXADR=(IPtr[3]>>7)&0x1;


		//operations are done at 24 bit precision

		if(MASA)
			int a=1;
		if(NOFL)
			int a=1;

		int dump=0;

		if(f)
		{
#define DUMP(v)	fprintf(f," " #v ": %04X",v);
			
			fprintf(f,"%d: ",step);
			DUMP(ACC);
			DUMP(SHIFTED);
			DUMP(X);
			DUMP(Y);
			DUMP(B);
			DUMP(INPUTS);
			DUMP(MEMVAL);
			DUMP(FRC_REG);
			DUMP(Y_REG);
			DUMP(ADDR);
			DUMP(ADRS_REG);
			fprintf(f,"\n");
		}
		//INPUTS RW
		assert(IRA<0x32);
		if(IRA<=0x1f)
			INPUTS=DSP->MEMS[IRA];	
		else if(IRA<=0x2F)
			INPUTS=DSP->MIXS[IRA-0x20];	//MIXS is 24 bit
		else if(IRA<=0x31)
			INPUTS=DSP->EXTS[IRA-0x30];
		else INPUTS=0;

		INPUTS<<=8;
		INPUTS>>=8;
		//if(INPUTS&0x00800000)
		//	INPUTS|=0xFF000000;

		if(IWT)
		{
			DSP->MEMS[IWA]=MEMVAL;	//MEMVAL was selected in previous MRD
			if(IRA==IWA)
				INPUTS=MEMVAL;
		}

		//Operand sel
		//B
		if(!ZERO)
		{
			if(BSEL)
				B=ACC;
			else
			{
				B=DSP->TEMP[(TRA+DSP->DEC)&0x7F];
				B<<=8;
				B>>=8;
				//if(B&0x00800000)
				//	B|=0xFF000000;	//Sign extend
			}
			if(NEGB)
				B=0-B;
		}
		else
			B=0;

		//X
		if(XSEL)
			X=INPUTS;
		else
		{
			X=DSP->TEMP[(TRA+DSP->DEC)&0x7F];
			X<<=8;
			X>>=8;
			//if(X&0x00800000)
			//	X|=0xFF000000;
		}

		//Y
		if(YSEL==0)
			Y=FRC_REG;
		else if(YSEL==1)
			Y=DSP->COEF[step]>>3;	//COEF is 16 bits
		else if(YSEL==2)
			Y=(Y_REG>>11)&0x1FFF;
		else if(YSEL==3)
			Y=(Y_REG>>4)&0x0FFF;

		if(YRL)
			Y_REG=INPUTS;

		//Shifter
		if(SHIFT==0)
		{
			SHIFTED=ACC;
			if(SHIFTED>0x007FFFFF)
				SHIFTED=0x007FFFFF;
			if(SHIFTED<(-0x00800000))
				SHIFTED=-0x00800000;
		}
		else if(SHIFT==1)
		{
			SHIFTED=ACC*2;
			if(SHIFTED>0x007FFFFF)
				SHIFTED=0x007FFFFF;
			if(SHIFTED<(-0x00800000))
				SHIFTED=-0x00800000;
		}
		else if(SHIFT==2)
		{
			SHIFTED=ACC*2;
			SHIFTED<<=8;
			SHIFTED>>=8;
			//SHIFTED&=0x00FFFFFF;
			//if(SHIFTED&0x00800000)
			//	SHIFTED|=0xFF000000;
		}
		else if(SHIFT==3)
		{
			SHIFTED=ACC;
			SHIFTED<<=8;
			SHIFTED>>=8;
			//SHIFTED&=0x00FFFFFF;
			//if(SHIFTED&0x00800000)
			//	SHIFTED|=0xFF000000;
		}

		//ACCUM
		Y<<=19;
		Y>>=19;
		//if(Y&0x1000)
		//	Y|=0xFFFFF000;

		__int64 v=(((__int64) X*(__int64) Y)>>12);
		ACC=(int) v+B;

		if(TWT)
			DSP->TEMP[(TWA+DSP->DEC)&0x7F]=SHIFTED;

		if(FRCL)
		{
			if(SHIFT==3)
				FRC_REG=SHIFTED&0x0FFF;
			else
				FRC_REG=(SHIFTED>>11)&0x1FFF;
		}

		if(MRD || MWT)
		//if(0)
		{
			ADDR=DSP->MADRS[MASA];
			if(!TABLE)
				ADDR+=DSP->DEC;
			if(ADREB)
				ADDR+=ADRS_REG&0x0FFF;
			if(NXADR)
				ADDR++;
			if(!TABLE)
				ADDR&=DSP->RBL-1;
			else
				ADDR&=0xFFFF;
			//ADDR<<=1;
			//ADDR+=DSP->RBP<<13;
			//MEMVAL=DSP->DSPRAM[ADDR>>1];
			ADDR+=DSP->RBP<<12;

			if(MWT && (step&1))
			{
				if(NOFL)
					DSP->DSPRAM[ADDR]=SHIFTED>>8;
				else
					DSP->DSPRAM[ADDR]=PACK(SHIFTED);
			}

			if(MRD && (step&1))	//memory only allowed on odd? DoA inserts NOPs on even
			{
				if(NOFL)
					MEMVAL=DSP->DSPRAM[ADDR]<<8;
				else
					MEMVAL=UNPACK(DSP->DSPRAM[ADDR]);
				if(MEMVAL)
					int a=1;

			}
			
		}

		if(ADRL)
		{
			if(SHIFT==3)
				ADRS_REG=(SHIFTED>>12)&0xFFF;
			else
				ADRS_REG=(INPUTS>>16);
		}

		if(EWT)
			DSP->EFREG[EWA]+=SHIFTED>>8;

	}
	--DSP->DEC;
	memset(DSP->MIXS,0,4*16);
	if(f)
		fclose(f);
}
#else

FILE *f=NULL;

void AICADSP_Step(_AICADSP *DSP)
{
	if(DSP->Stopped)
		return;

	memset(DSP->EFREG,0,2*16);
	
	assert(DSP->DoSteps!=NULL);

	int dump=0;
	if(dump)
		f=fopen("dsp2.txt","wt");

	DSP->DoSteps();

	if(f)
	{
		fclose(f);
		f=NULL;
	}

	--DSP->DEC;
	memset(DSP->MIXS,0,4*16);

}

void __fastcall dumpreg(_AICADSP *DSP)
{
	static int n=0;
	//f=fopen("dsp2.txt","a+t");
		if(f)
		{
#define DUMP(v)	fprintf(f," " #v ": %04X",DSP->v);
			
			fprintf(f,"%d: ",n++);
			DUMP(ACC);
			DUMP(SHIFTED);
			DUMP(X);
			DUMP(Y);
			DUMP(B);
			DUMP(INPUTS);
			DUMP(MEMVAL);
			DUMP(FRC_REG);
			DUMP(Y_REG);
			DUMP(ADDR);
			DUMP(ADRS_REG);
			fprintf(f,"\n");
		}
}

#define EMIT8(x) *PtrInsts=x; ++PtrInsts;
#define EMIT16(x) *((unsigned short *) PtrInsts)=x; PtrInsts+=2;
#define EMIT32(x) *((unsigned int *) PtrInsts)=x; PtrInsts+=4;

#define MOV_EAXTOMEM(addr)	EMIT8(0xA3); EMIT32((unsigned int) addr);
#define MOV_MEMTOEAX(addr)	EMIT8(0xA1); EMIT32((unsigned int) addr);
#define MOV_MEMTOAX(addr)	EMIT8(0x66); EMIT8(0xA1); EMIT32((unsigned int) addr);
#define MOV_MEMTOEBX(addr)	EMIT8(0x8B); EMIT8(0x1D); EMIT32((unsigned int) addr);
#define ADD_MEMTOEAX(addr)	EMIT8(0x03); EMIT8(0x05); EMIT32((unsigned int) addr);
#define ADD_EAXTOMEM(addr)	EMIT8(0x01); EMIT8(0x05); EMIT32((unsigned int) addr);
#define ADD_AXTOMEM(addr)	EMIT8(0x66); EMIT8(0x01); EMIT8(0x05); EMIT32((unsigned int) addr);
#define MOV_IMMTOEAX(imm)	EMIT8(0xB8); EMIT32((unsigned int) imm);
#define MOV_IMMTOECX(imm)	EMIT8(0xB9); EMIT32((unsigned int) imm);
#define ADD_IMMTOEAX(imm)	EMIT8(0x05); EMIT32((unsigned int) imm);
#define ADD_EBXTOEAX()		EMIT8(0x03); EMIT8(0xC3);
#define CMP_IMMTOEAX(imm)	EMIT8(0x3D); EMIT32((unsigned int) imm);
#define MOV_0TOEAX()		EMIT8(0x33); EMIT8(0xC0);
#define MOV_EAXTOEBX()		EMIT8(0x8B); EMIT8(0xD8);
#define MOV_EAXTOECX()		EMIT8(0x8B); EMIT8(0xC8);
#define NEG_EAX()			EMIT8(0xF7); EMIT8(0xD8);
#define INC_EAX()			EMIT8(0x40);
#define MOV_MEMEAXTOEAX()	EMIT8(0x8b); EMIT8(0x00);
#define MOV_MEMEAXTOAX()	EMIT8(0x66); EMIT8(0x8b); EMIT8(0x00);
#define MOV_EBXTOMEMEAX()	EMIT8(0x89); EMIT8(0x18);
#define MOV_EAXTOMEMEBX()	EMIT8(0x89); EMIT8(0x03);
#define MOV_AXTOMEMEBX()	EMIT8(0x66); EMIT8(0x89); EMIT8(0x03);
#define SHL_EAX(count)		EMIT8(0xC1); EMIT8(0xE0); EMIT8(count);
#define SHL_EBX(count)		EMIT8(0xC1); EMIT8(0xE3); EMIT8(count);
#define SHL_EDX(count)		EMIT8(0xC1); EMIT8(0xE2); EMIT8(count);
#define SHRD_EAX_EDX(count)	EMIT8(0x0F); EMIT8(0xAC); EMIT8(0xD0); EMIT8(count);
#define SAR_EAX(count)		EMIT8(0xC1); EMIT8(0xF8); EMIT8(count);
#define SHR_EAX(count)		EMIT8(0xC1); EMIT8(0xE8); EMIT8(count);
#define AND_EAX(mask)		EMIT8(0x25); EMIT32(mask);
#define AND_EBX(mask)		EMIT8(0x81); EMIT8(0xE3); EMIT32(mask);
#define AND_EAX_EBX()		EMIT8(0x23); EMIT8(0xC3);
#define DEC_EBX()			EMIT8(0x4B);
#define OR_EAX_EDX()		EMIT8(0x0B); EMIT8(0xC2);
#define ADD_EAX_ECX()		EMIT8(0x03); EMIT8(0xC1);
#define IMUL_EAX_EBX()		EMIT8(0xF7); EMIT8(0xEB);
//#define IMUL_EAX_EBX()		EMIT8(0xF7); EMIT8(0xE3);
#define RET()				EMIT8(0xC3);
#define PUSHA()				EMIT8(0x51); EMIT8(0x52); EMIT8(0x53); EMIT8(0x56); //ecx edx ebx esi
#define POPA()				EMIT8(0x5E); EMIT8(0x5B); EMIT8(0x5A); EMIT8(0x59);	//esi ebx edx ecx

struct _INST
{
	unsigned int TRA;
	unsigned int TWT;
	unsigned int TWA;
	
	unsigned int XSEL;
	unsigned int YSEL;
	unsigned int IRA;
	unsigned int IWT;
	unsigned int IWA;

	unsigned int TABLE;
	unsigned int MWT;
	unsigned int MRD;
	unsigned int EWT;
	unsigned int EWA;
	unsigned int ADRL;
	unsigned int FRCL;
	unsigned int SHIFT;
	unsigned int YRL;
	unsigned int NEGB;
	unsigned int ZERO;
	unsigned int BSEL;

	unsigned int NOFL;
	unsigned int COEF;
	
	unsigned int MASA;
	unsigned int ADREB;
	unsigned int NXADR;
};

void DecodeInst(unsigned short *IPtr,_INST *i)
{
	i->TRA=(IPtr[0]>>9)&0x7F;
	i->TWT=(IPtr[0]>>8)&0x01;
	i->TWA=(IPtr[0]>>1)&0x7F;
	
	i->XSEL=(IPtr[1]>>15)&0x01;
	i->YSEL=(IPtr[1]>>13)&0x03;
	i->IRA=(IPtr[1]>>7)&0x3F;
	i->IWT=(IPtr[1]>>6)&0x01;
	i->IWA=(IPtr[1]>>1)&0x1F;

	i->TABLE=(IPtr[2]>>15)&0x01;
	i->MWT=(IPtr[2]>>14)&0x01;
	i->MRD=(IPtr[2]>>13)&0x01;
	i->EWT=(IPtr[2]>>12)&0x01;
	i->EWA=(IPtr[2]>>8)&0x0F;
	i->ADRL=(IPtr[2]>>7)&0x01;
	i->FRCL=(IPtr[2]>>6)&0x01;
	i->SHIFT=(IPtr[2]>>4)&0x03;
	i->YRL=(IPtr[2]>>3)&0x01;
	i->NEGB=(IPtr[2]>>2)&0x01;
	i->ZERO=(IPtr[2]>>1)&0x01;
	i->BSEL=(IPtr[2]>>0)&0x01;

	i->NOFL=(IPtr[3]>>15)&1;		//????
	//i->COEF=(IPtr[3]>>9)&0x3f;
	
	i->MASA=(IPtr[3]>>9)&0x3f;	//???
	i->ADREB=(IPtr[3]>>8)&0x1;
	i->NXADR=(IPtr[3]>>7)&0x1;
}

#define USES_SHIFTED(inst)	(inst.TWT || inst.FRCL || inst.MWT || inst.ADRL || inst.EWT)

void AICADSP_Recompile(_AICADSP *DSP)
{
	if(DSP->DoSteps)
		free(DSP->DoSteps);
	DSP->DoSteps=(void (*)()) malloc(DYNBUF);
	unsigned char *PtrInsts=(unsigned char *)DSP->DoSteps;

	PUSHA();

	for(int step=0;step</*128*/DSP->LastStep;++step)
	{
		unsigned short *IPtr=DSP->MPRO+step*4;
		_INST ThisInst,NextInst;
		DecodeInst(IPtr,&ThisInst);
		DecodeInst(IPtr+4,&NextInst);
		


/*		EMIT8(0x90);

		MOV_IMMTOECX(DSP);
		MOV_IMMTOEAX(dumpreg);
		EMIT8(0xFF); EMIT8(0xD0);
*/
		//INPUTS RW
		assert(ThisInst.IRA<0x32);
		if((ThisInst.XSEL || ThisInst.YRL || ThisInst.ADRL) || !DYNOPT)
		{
			if(ThisInst.IRA<=0x1f)
			{
				//INPUTS=DSP->MEMS[IRA];	
				MOV_MEMTOEAX(&(DSP->MEMS[ThisInst.IRA]));
				SHL_EAX(8);
			}
			else if(ThisInst.IRA<=0x2F)
			{
				//INPUTS=DSP->MIXS[IRA-0x20]<<8;	//MIXS is 16 bit
				MOV_MEMTOEAX(&(DSP->MIXS[ThisInst.IRA-0x20]));
				SHL_EAX(8);
			}
			else if(ThisInst.IRA<=0x31)
			{
				MOV_MEMTOEAX(&(DSP->EXTS[ThisInst.IRA-0x30]));
				SHL_EAX(8);				
			}
			else
			{
				MOV_0TOEAX();
			}
			SAR_EAX(8);
			
			MOV_EAXTOMEM(&(DSP->INPUTS));
		}

		if(ThisInst.IWT)
		{
			MOV_MEMTOEAX(&DSP->MEMVAL);
			MOV_EAXTOMEM(&DSP->MEMS[ThisInst.IWA]);
			//DSP->MEMS[IWA]=MEMVAL;	//MEMVAL was selected in previous MRD
			if(ThisInst.IRA==ThisInst.IWA)
			{
				//INPUTS=MEMVAL;
				MOV_EAXTOMEM(&DSP->INPUTS);
			}
		}
		
		if((USES_SHIFTED(NextInst) || NextInst.BSEL) || !DYNOPT)
		{
			//Operand sel
			//B
			if(!ThisInst.ZERO)
			{
				if(ThisInst.BSEL)
				{
					//B=ACC;
					MOV_MEMTOEAX(&DSP->ACC);
	//				MOV_EAXTOMEM(&DSP->B);	//
				}
				else
				{
					MOV_IMMTOEAX(ThisInst.TRA);
					ADD_MEMTOEAX(&(DSP->DEC));
					AND_EAX(0x7F);
					SHL_EAX(2);
					ADD_IMMTOEAX(&(DSP->TEMP));
					MOV_MEMEAXTOEAX();
					SHL_EAX(8);
					SAR_EAX(8);
	//				MOV_EAXTOMEM(&DSP->B);	//


					//B=DSP->TEMP[(TRA+DSP->DEC)&0x7F];
					//B<<=8;
					//B>>=8;
					//if(B&0x00800000)
					//	B|=0xFF000000;	//Sign extend
				}
				if(ThisInst.NEGB)
				{
					//B=0-B;
					NEG_EAX();
				}
			}
			else
			{
				MOV_0TOEAX();
			}
			//MOV_EAXTOMEM(&DSP->B);
			MOV_EAXTOECX();

			//X
			if(ThisInst.XSEL)
			{
				//X=INPUTS;
				MOV_MEMTOEAX(&DSP->INPUTS);
	//			MOV_EAXTOMEM(&DSP->X);	//
			}
			else
			{
				//X=DSP->TEMP[(TRA+DSP->DEC)&0x7F];
				//X<<=8;
				//X>>=8;
				MOV_IMMTOEAX(ThisInst.TRA);
				ADD_MEMTOEAX(&(DSP->DEC));
				AND_EAX(0x7F);
				SHL_EAX(2);
				ADD_IMMTOEAX(&(DSP->TEMP));
				MOV_MEMEAXTOEAX();
				SHL_EAX(8);
				SAR_EAX(8);
				//if(X&0x00800000)
				//	X|=0xFF000000;
	//			MOV_EAXTOMEM(&DSP->X);	//
			}
			MOV_EAXTOEBX();
		}


		//if(TWT || /*MRD ||*/ MWT || EWT || ADRL || FRCL)
		if(USES_SHIFTED(ThisInst) || !DYNOPT)
		{
			if(ThisInst.SHIFT==0)
			{
				MOV_MEMTOEAX(&DSP->ACC);
				CMP_IMMTOEAX(0x007FFFFF);
				EMIT8(0x7E); EMIT8(0x05);	//JLE
				MOV_IMMTOEAX(0x007FFFFF);
				CMP_IMMTOEAX(0xFF800000);
				EMIT8(0x7D); EMIT8(0x05);	//JGE
				MOV_IMMTOEAX(0xFF800000);


				//SHIFTED=ACC;
				//if(SHIFTED>0x007FFFFF)
				//	SHIFTED=0x007FFFFF;
				//if(SHIFTED<(-0x00800000))
				//	SHIFTED=-0x00800000;
			}
			else if(ThisInst.SHIFT==1)
			{
				//SHIFTED=ACC*2;
				MOV_MEMTOEAX(&DSP->ACC);
				SHL_EAX(1);
				CMP_IMMTOEAX(0x007FFFFF);
				EMIT8(0x7E); EMIT8(0x05);	//JLE
				MOV_IMMTOEAX(0x007FFFFF);
				CMP_IMMTOEAX(0xFF800000);
				EMIT8(0x7D); EMIT8(0x05);	//JGE
				MOV_IMMTOEAX(0xFF800000);
				//if(SHIFTED>0x007FFFFF)
				//	SHIFTED=0x007FFFFF;
				//if(SHIFTED<(-0x00800000))
				//	SHIFTED=-0x00800000;
			}
			else if(ThisInst.SHIFT==2)
			{
				//SHIFTED=ACC*2;
				//SHIFTED<<=8;
				//SHIFTED>>=8;
				MOV_MEMTOEAX(&DSP->ACC);
				SHL_EAX(9);
				SAR_EAX(8);
				//SHIFTED&=0x00FFFFFF;
				//if(SHIFTED&0x00800000)
				//	SHIFTED|=0xFF000000;
			}
			else if(ThisInst.SHIFT==3)
			{
				//SHIFTED=ACC;
				//SHIFTED<<=8;
				//SHIFTED>>=8;
				MOV_MEMTOEAX(&DSP->ACC);
				SHL_EAX(8);
				SAR_EAX(8);
				//SHIFTED&=0x00FFFFFF;
				//if(SHIFTED&0x00800000)
				//	SHIFTED|=0xFF000000;
			}
			MOV_EAXTOMEM(&DSP->SHIFTED);
		}

		if((USES_SHIFTED(NextInst) || NextInst.BSEL) || !DYNOPT)
		{
			//Y
			if(ThisInst.YSEL==0)
			{
				//Y=FRC_REG;
				MOV_MEMTOEAX(&DSP->FRC_REG);
			}
			else if(ThisInst.YSEL==1)
			{
				//MOV_0TOEAX();
				MOV_MEMTOAX(&DSP->COEF[step]);
				SAR_EAX(3);
				//Y=DSP->COEF[COEF]>>3;	//COEF is 16 bits
			}
			else if(ThisInst.YSEL==2)
			{
				//Y=(Y_REG>>11)&0x1FFF;
				MOV_MEMTOEAX(&DSP->Y_REG);
				SAR_EAX(11);
				AND_EAX(0x1FFF);
			}
			else if(ThisInst.YSEL==3)
			{
				//Y=(Y_REG>>4)&0x0FFF;
				MOV_MEMTOEAX(&DSP->Y_REG);
				SAR_EAX(4);
				AND_EAX(0x0FFF);
			}

			SHL_EAX(19);
			SAR_EAX(19);
	//		MOV_EAXTOMEM(&DSP->Y);	//



			//X:EBX
			//B:ECX
			//Y:EAX
			IMUL_EAX_EBX();
	//		SHR_EAX(12);
	//		SHL_EDX((32-12));
			SHRD_EAX_EDX(12);
			ADD_EAX_ECX();

			MOV_EAXTOMEM(&DSP->ACC);
		}

		if(ThisInst.YRL)
		{
			MOV_MEMTOEAX(&DSP->INPUTS);
			MOV_EAXTOMEM(&DSP->Y_REG);
			//Y_REG=INPUTS;
		}

		if(ThisInst.TWT)
		{
			MOV_MEMTOEAX(&DSP->SHIFTED);
			MOV_EAXTOEBX();
			//DSP->TEMP[(TWA+DSP->DEC)&0x7F]=SHIFTED;
			MOV_IMMTOEAX(ThisInst.TWA);
			ADD_MEMTOEAX(&(DSP->DEC));
			AND_EAX(0x7F);
			SHL_EAX(2);
			ADD_IMMTOEAX(&(DSP->TEMP));
			MOV_EBXTOMEMEAX();
		}

		if(ThisInst.FRCL)
		{
			if(ThisInst.SHIFT==3)
			{
				//FRC_REG=SHIFTED&0x0FFF;
				MOV_MEMTOEAX(&DSP->SHIFTED);
				AND_EAX(0x0FFF);
				MOV_EAXTOMEM(&DSP->FRC_REG);
			}
			else
			{
				//FRC_REG=(SHIFTED>>11)&0x1FFF;
				MOV_MEMTOEAX(&DSP->SHIFTED);
				SHR_EAX(11);
				AND_EAX(0x1FFF);
				MOV_EAXTOMEM(&DSP->FRC_REG);
			}
		}

		//MEM
		if(ThisInst.MRD || ThisInst.MWT)
		//if(0)
		{
			MOV_0TOEAX();
			MOV_MEMTOAX(&DSP->MADRS[ThisInst.MASA]);
			//ADDR=DSP->MADRS[MASA];
			if(!ThisInst.TABLE)
			{
				//ADDR+=DSP->DEC;
				//ADD_MEMTOEAX(&DSP->DEC);
				MOV_MEMTOEBX(&DSP->DEC);
				ADD_EBXTOEAX();
			}
			if(ThisInst.ADREB)
			{
				MOV_MEMTOEBX(&(DSP->ADRS_REG));
				AND_EBX(0x0FFF);
				ADD_EBXTOEAX();
				//ADDR+=ADRS_REG&0x0FFF;
			}
			if(ThisInst.NXADR)
			{
				//ADDR++;
				INC_EAX();
			}
			if(!ThisInst.TABLE)
			{
				MOV_MEMTOEBX(&(DSP->RBL));
				DEC_EBX();
				//ADDR&=DSP->RBL-1;
				AND_EAX_EBX();

				//AND_EAX((DSP->RBL-1));
			}
			else
			{
				//ADDR&=0xFFFF;
				AND_EAX(0xFFFF);
			}

			//ADDR+=DSP->RBP<<12;
			MOV_MEMTOEBX(&(DSP->RBP));
			SHL_EBX(12);
			ADD_EBXTOEAX();

			assert(!(ThisInst.MRD && ThisInst.MWT));	//this shouldn't happen, read & write in the same cycle

			if(ThisInst.MWT && (step&1))
			{
				if(ThisInst.NOFL)
				{
					SHL_EAX(1);
					ADD_IMMTOEAX(DSP->DSPRAM);
					MOV_EAXTOEBX();
					MOV_MEMTOEAX(&DSP->SHIFTED);
					SHR_EAX(8);
					MOV_AXTOMEMEBX();
					//DSP->DSPRAM[ADDR]=SHIFTED>>8;
				}
				else
				{
#ifdef USEFLOATPACK
					SHL_EAX(1);
					ADD_IMMTOEAX(DSP->DSPRAM);
					EMIT8(0x8B); EMIT8(0xF0);	//mov esi,eax
					MOV_MEMTOEAX(&DSP->SHIFTED);
					SHR_EAX(8);
					memcpy(PtrInsts,PackFunc,sizeof(PackFunc));
					PtrInsts+=sizeof(PackFunc);
#else
					SHL_EAX(1);
					ADD_IMMTOEAX(DSP->DSPRAM);
					MOV_EAXTOEBX();
					MOV_MEMTOEAX(&DSP->SHIFTED);
					SHR_EAX(8);
					MOV_AXTOMEMEBX();
#endif
					//DSP->DSPRAM[ADDR]=PACK(SHIFTED);
				}
			}

			if(ThisInst.MRD && (step&1))	//memory only allowed on odd? DoA inserts NOPs on even
			{
				if(ThisInst.NOFL)
				{
					//MEMVAL=DSP->DSPRAM[ADDR]<<8;
					SHL_EAX(1);
					ADD_IMMTOEAX(DSP->DSPRAM);
					MOV_MEMEAXTOAX();
					SHL_EAX(8);
					//MOV_EAXTOMEM(&DSP->MEMVAL);
				}
				else
				{
					//MEMVAL=UNPACK(DSP->DSPRAM[ADDR]);
					SHL_EAX(1);
					ADD_IMMTOEAX(DSP->DSPRAM);
					MOV_MEMEAXTOAX();
#ifdef USEFLOATPACK
					memcpy(PtrInsts,UnpackFunc,sizeof(UnpackFunc));
					PtrInsts+=sizeof(UnpackFunc);
#else
					SHL_EAX(16);
					SAR_EAX(8);
#endif
					MOV_EAXTOMEM(&DSP->MEMVAL);
				}
			}
			
			
			
		}

		if(ThisInst.ADRL)
		{
			if(ThisInst.SHIFT==3)
			{
				MOV_MEMTOEAX(&DSP->SHIFTED);
				SAR_EAX(12);
				AND_EAX(0xFFF);
				MOV_EAXTOMEM(&DSP->ADRS_REG);
				//ADRS_REG=(SHIFTED>>12)&0xFFF;
			}
			else
			{
				MOV_MEMTOEAX(&DSP->INPUTS);
				SAR_EAX(16);
				MOV_EAXTOMEM(&DSP->ADRS_REG);
				//ADRS_REG=(INPUTS>>16);
			}
		}

		if(ThisInst.EWT)
		{
			MOV_MEMTOEAX(&DSP->SHIFTED);
			SAR_EAX(8);
			ADD_AXTOMEM(&DSP->EFREG[ThisInst.EWA]);
			//DSP->EFREG[EWA]+=SHIFTED>>8;
		}
//		EMIT8(0x90);
//		EMIT8(0xCC);
		
	}

	POPA();

	RET();

	//FILE *f=fopen("dsp.rec","wb");
	//fwrite(DSP->DoSteps,1,PtrInsts-(unsigned char *)DSP->DoSteps,f);
	//fclose(f);
}

#endif
void AICADSP_SetSample(_AICADSP *DSP,signed int sample,int SEL,int MXL)
{
//	if(MXL!=6)
//		return;
	//16 to 24
	DSP->MIXS[SEL]+=sample<<((MXL>>1))/*7*/;
//	DSP->MIXS[SEL]+=sample<<7;
	if(MXL)
	{
		int a=1;
		if(MXL!=6)
			int a=1;
	}

}

void AICADSP_Start(_AICADSP *DSP)
{
	int i;
	DSP->Stopped=false;
	for(i=127;i>=0;--i)
	{
		unsigned short *IPtr=DSP->MPRO+i*4;

		if(IPtr[0]!=0 || IPtr[1]!=0 || IPtr[2]!=0 || IPtr[3]!=0)
			break;
	}
	DSP->LastStep=i+1;

	int test=0;
	if(test)
	{
		//test
		FILE *f1;
		f1=fopen("MPRO","wb");
		fwrite(DSP->MPRO,128*4*2,1,f1);
		fwrite(DSP->COEF,64*2,1,f1);
		fwrite(DSP->MADRS,32*2,1,f1);
		fclose(f1);
	}

#ifdef DYNDSP
	AICADSP_Recompile(DSP);
#endif
}