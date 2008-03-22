/*	
	Dreamcast AICA Emulation
*/

#include "../stdafx.h"
#include "AICADC.h"
#include "stdio.h"
#include "memory.h"
#include "math.h"

//#define FASTAICA
#define INTERPOLATE

//#define REVERB
#define USEDSP

//FILE *test1;
//FILE *test2;

#include "aicadsp.cpp"

/*#define TIMER_LIMITSA  0x101
#define TIMER_LIMITSB  0x100
#define TIMER_LIMITSC  0xff
*/

#define TIMER_LIMITSA  0x100
#define TIMER_LIMITSB  0x100
#define TIMER_LIMITSC  0x100

static signed short *bufferl;		
static signed short *bufferr;
static int length;
static signed int *buffertmpl,*buffertmpr;

static unsigned int srate=44100;



#define REVERB_LEN	0x10000
#define REVERB_DIF	6000
#define REVERB_DIV	4


static signed short bufferrevr[REVERB_LEN];
static signed short bufferrevl[REVERB_LEN];
static unsigned int RevR,RevW;

//#define _DEBUG

#define ErrorLogMessage

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned int
#endif

static DWORD IrqTimA=1;
static DWORD IrqTimBC=2;
static DWORD IrqMidi=3;

static BYTE MidiOutStack[8];
static BYTE MidiOutW=0,MidiOutR=0;
static BYTE MidiStack[8];
static BYTE MidiOutFill;
static BYTE MidiInFill;
static BYTE MidiW=0,MidiR=0;
static BYTE HasSlaveAICA=0;

static DWORD FNS_Table[0x400];
/*static int TLTABLE[256];
static int LPANTABLE[16];
static int RPANTABLE[16];
*/

static float SDLT[8]={-1000000.0,-36.0,-30.0,-24.0,-18.0,-12.0,-6.0,0.0};
static int LPANTABLE[0x10000];
static int RPANTABLE[0x10000];

static int TimPris[3];
static int TimCnt[3];
static DWORD OnIRQ=0;

#define SHIFT	12
#define FIX(v)	((DWORD) ((float) (1<<SHIFT)*(v)))


#define EG_SHIFT	8

#include "AICAlfo.cpp"

/*
	AICA features 64 programmable slots
	that can generate FM and PCM (from ROM/RAM) sound
*/
//SLOT PARAMETERS
#define KEYONEX(slot)	((slot->data[0x0]>>0x0)&0x8000)
#define KEYONB(slot)	((slot->data[0x0]>>0x0)&0x4000)
#define SSCTL(slot)		((slot->data[0x0]>>0xA)&0x0001)
#define LPCTL(slot)		((slot->data[0x0]>>0x9)&0x0001)
#define PCMS(slot)		((slot->data[0x0]>>0x7)&0x0003)

#define SA(slot)		(((slot->data[0x0]&0x3F)<<16)|(slot->data[0x2]))

#define LSA(slot)		(slot->data[0x4])

#define LEA(slot)		(slot->data[0x6])

#define D2R(slot)		((slot->data[0x8]>>0xB)&0x001F)
#define D1R(slot)		((slot->data[0x8]>>0x6)&0x001F)
#define EGHOLD(slot)	((slot->data[0x8]>>0x0)&0x0020)
#define AR(slot)		((slot->data[0x8]>>0x0)&0x001F)

#define LPSLNK(slot)	((slot->data[0xa]>>0x0)&0x4000)
#define KRS(slot)		((slot->data[0xa]>>0xA)&0x000F)
#define DL(slot)		((slot->data[0xa]>>0x5)&0x001F)
#define RR(slot)		((slot->data[0xa]>>0x0)&0x001F)

//#define STWINH(slot)	((slot->data[0xc]>>0x0)&0x0200)
//#define SDIR(slot)		((slot->data[0xc]>>0x0)&0x0100)
//#define TL(slot)		((slot->data[0xc]>>0x0)&0x00FF)

#define OCT(slot)		((slot->data[0xc]>>0xB)&0x000F)
#define FNS(slot)		((slot->data[0xc]>>0x0)&0x03FF)

#define LFORE(slot)		((slot->data[0xe]>>0x0)&0x8000)
#define LFOF(slot)		((slot->data[0xe]>>0xA)&0x001F)
#define PLFOWS(slot)	((slot->data[0xe]>>0x8)&0x0003)
#define PLFOS(slot)		((slot->data[0xe]>>0x5)&0x0007)
#define ALFOWS(slot)	((slot->data[0xe]>>0x3)&0x0003)
#define ALFOS(slot)		((slot->data[0xe]>>0x0)&0x0007)

#define IMXL(slot)		((slot->data[0x10]>>0x4)&0x000F)
#define ISEL(slot)		((slot->data[0x10]>>0x0)&0x000F)

#define DISDL(slot)		((slot->data[0x12]>>0x9)&0x0007)	//me como 1 bit, no me cabe mas :(
#define DIPAN(slot)		((slot->data[0x12]>>0x0)&0x001F)

#define TL(slot)		((slot->data[0x14]>>0x8)&0x00FF)

#define EFSDL(slot)		(0)
#define EFPAN(slot)		(0)

//#define EFSDL(slot)		((slot->data[0x12]>>0x5)&0x0007)
//#define EFPAN(slot)		((slot->data[0x12]>>0x0)&0x001F)

int ARTABLE[64],DRTABLE[64];
double BaseTimes[64]={0,0,0,0,6222.95,4978.37,4148.66,3556.01,3111.47,2489.21,2074.33,1778.00,1555.74,1244.63,1037.19,889.02,
					 777.87,622.31,518.59,444.54,388.93,311.16,259.32,222.27,194.47,155.60,129.66,111.16,97.23,77.82,64.85,55.60,
					 48.62,38.91,32.43,27.80,24.31,19.46,16.24,13.92,12.15,9.75,8.12,6.98,6.08,4.90,4.08,3.49,
					 3.04,2.49,2.13,1.90,1.72,1.41,1.18,1.04,0.91,0.73,0.59,0.50,0.45,0.45,0.45,0.00};
double BaseTimes2[32]={7000,6500,6222.95,4978.37,4148.66,3556.01,3111.47,2489.21,2074.33,1778.00,1555.74,1244.63,1037.19,889.02,
					   777.87,622.31,518.59,444.54,388.93,311.16,259.32,222.27,194.47,155.60,129.66,111.16,97.23,77.82,64.85,55.60,41,20};
#define AR2DR	14.304187

typedef enum {ATTACK=0,DECAY1,DECAY2,RELEASE} _STATE;
struct _EG
{
	int volume;	//
	_STATE state;
	int step;
	//step vals
	int AR;		//Attack
	int D1R;	//Decay1
	int D2R;	//Decay2
	int RR;		//Release

	int DL;		//Decay level
	BYTE EGHOLD;
	BYTE LPLINK;
};

struct _SLOT
{
	union
	{
		WORD *data;	//only 0x1a bytes used
		BYTE *datab;
	};
	BYTE active;	//this slot is currently playing
	BYTE *base;		//samples base address
	DWORD cur_addr;	//current play address (24.8)
	DWORD step;		//pitch step (24.8)
	_EG EG;			//Envelope
	_LFO PLFO;		//Phase LFO
	_LFO ALFO;		//Amplitude LFO
	int slot;
	signed short PPrev;	//Previous sample (for interpolation)
	signed short Prev;	
	float PrevQuant;
	unsigned int LastDecAddr;	//Last decoded address for ADPCM
	unsigned int ADStep;
};

#define MEM4B(AICA)		((AICA.data[0]>>0x0)&0x0200)
#define DAC18B(AICA)	((AICA.data[0]>>0x0)&0x0100)
#define MVOL(AICA)		((AICA.data[0]>>0x0)&0x000F)
#define RBL(AICA)		((AICA.data[1]>>0x7)&0x0003)
#define RBP(AICA)		((AICA.data[1]>>0x0)&0x003F)
#define MOFULL(AICA)	((AICA.data[2]>>0x0)&0x1000)
#define MOEMPTY(AICA)	((AICA.data[2]>>0x0)&0x0800)
#define MIOVF(AICA)		((AICA.data[2]>>0x0)&0x0400)
#define MIFULL(AICA)	((AICA.data[2]>>0x0)&0x0200)
#define MIEMPTY(AICA)	((AICA.data[2]>>0x0)&0x0100)

#define SCILV0(AICA)    ((AICA.data[0x24/2]>>0x0)&0xff)
#define SCILV1(AICA)    ((AICA.data[0x26/2]>>0x0)&0xff)
#define SCILV2(AICA)    ((AICA.data[0x28/2]>>0x0)&0xff)

#define SCIEX0	0
#define SCIEX1	1
#define SCIEX2	2
#define SCIMID	3
#define SCIDMA	4
#define SCIIRQ	5
#define SCITMA	6
#define SCITMB	7

static struct _AICA
{
	union
	{
		WORD *data;
		BYTE *datab;
	};
	_SLOT Slots[64];
	unsigned char BUFPTR;
	unsigned char *AICARAM;	
	char Master;
#ifdef USEDSP
	_AICADSP DSP;
#endif
} AICA;


unsigned char DecodeSCI(unsigned char irq)
{
	unsigned char SCI=0;
	unsigned char v;
	v=(SCILV0((AICA))&(1<<irq))?1:0;
	SCI|=v;
	v=(SCILV1((AICA))&(1<<irq))?1:0;
	SCI|=v<<1;
	v=(SCILV2((AICA))&(1<<irq))?1:0;
	SCI|=v<<2;
	return SCI;
}

void CheckPendingIRQ()
{
	DWORD pend=AICA.data[0x20/2];
	DWORD en=AICA.data[0x1e/2];
	/*if(pend&0x8)
		if(en&0x8)
		{
			Int68kCB(IrqMidi);
			return;
		}
	*/
	if(MidiW!=MidiR)
	{
//		Intf68k->Interrupt(IrqMidi,-1);
		return;
	}
	if(!pend)
		return;
	if(pend&0x40)
		if(en&0x40)
		{
//			Intf68k->Interrupt(IrqTimA,-1);
			return;
		}
	if(pend&0x80)
		if(en&0x80)
		{
//			Intf68k->Interrupt(IrqTimBC,-1);
			return;
		}
	if(pend&0x100)
		if(en&0x100)
		{
//			Intf68k->Interrupt(IrqTimBC,-1);
			return;
		}
//		Intf68k->Interrupt(0,-1);
}

int Get_AR(int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return ARTABLE[Rate];
}

int Get_DR(int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return DRTABLE[Rate];
}

int Get_RR(int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return ARTABLE[Rate];
}

void Compute_EG(_SLOT *slot)
{
	int octave=OCT(slot);
	int rate;
	/*if(octave&8) octave=octave-16;
	if(KRS(slot)!=0xf)
		rate=(octave+KRS(slot))*2+(FNS(slot)>>9);
	else*/
		rate=0;
	slot->EG.volume=0;
	slot->EG.AR=Get_AR(rate,AR(slot));
	slot->EG.D1R=Get_DR(rate,D1R(slot));
	slot->EG.D2R=Get_DR(rate,D2R(slot));
	slot->EG.RR=Get_RR(rate,RR(slot));
	slot->EG.DL=0x1f-DL(slot);
//	slot->EG.EGHOLD=EGHOLD(slot);
	if(LPSLNK(slot))
		int a=1;
}

void AICA_StopSlot(_SLOT *slot,int keyoff);

int EG_Update(_SLOT *slot)
{
	switch(slot->EG.state)
	{
		case ATTACK:
			slot->EG.volume+=slot->EG.AR;
			if(slot->EG.volume>=(0x3ff<<EG_SHIFT))
			{
				slot->EG.state=DECAY1;
				if(slot->EG.D1R>=(1024<<EG_SHIFT))	//Skip DECAY1, go directly to DECAY2
					slot->EG.state=DECAY2;
				slot->EG.volume=0x3ff<<EG_SHIFT;
			}
//			if(slot->EG.EGHOLD)
//				return 0x3ff<<(SHIFT-10);
			break;
		case DECAY1:
			slot->EG.volume-=slot->EG.D1R;
			if((slot->EG.volume>>(EG_SHIFT+5))<=slot->EG.DL)
				slot->EG.state=DECAY2;
			break;
		case DECAY2:
			if((slot->EG.volume>>(EG_SHIFT+5))<=0)	//fadeout decay (slow release)
			{
				slot->EG.volume=0;
				AICA_StopSlot(slot,0);
			}
			if(D2R(slot)==0)
				return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
			slot->EG.volume-=slot->EG.D2R;
			break;
		case RELEASE:
			slot->EG.volume-=slot->EG.RR;
			if(slot->EG.volume<=0)
			{
				AICA_StopSlot(slot,0);
				slot->EG.volume=0;
				slot->EG.state=ATTACK;
			}
			//slot->EG.volume=0;
			break;
		default:
			return 1<<SHIFT;
	}
	return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
}

DWORD AICA_Step(_SLOT *slot)
{
	int octave=OCT(slot);
	int Fn;

	/*
	int Fo=srate;
	if(octave&8)
		Fo>>=(16-octave);
	else
		Fo<<=octave;
	Fn=Fo*(((FNS(slot))<<(SHIFT-10))|(1<<SHIFT));
	*/
	Fn=(FNS_Table[FNS(slot)]);	//24.8
	if(octave&8)
		Fn>>=(16-octave);
	else
		Fn<<=octave;

	return Fn/srate;
}


void Compute_LFO(_SLOT *slot)
{
	if(PLFOS(slot)!=0)
		LFO_ComputeStep(&(slot->PLFO),LFOF(slot),PLFOWS(slot),PLFOS(slot),0);
	if(ALFOS(slot)!=0)
		LFO_ComputeStep(&(slot->ALFO),LFOF(slot),ALFOWS(slot),ALFOS(slot),1);
}

signed short DecodeADPCM(float m_fLastValue,unsigned char Delta,float &m_fQuant)
{
  BYTE cValue=Delta;

  float fNewValue = m_fQuant/8.f;
  if ( cValue&1 )
    fNewValue += m_fQuant/4.f;
  if ( cValue&2 )
    fNewValue += m_fQuant/2.f;
  if ( cValue&4 )
    fNewValue += m_fQuant;
  if ( cValue&8 )
    fNewValue = -fNewValue;

  m_fLastValue += fNewValue;
  if ( m_fLastValue > 32767.f )
    m_fLastValue = 32737.f;
  else if ( m_fLastValue < -32768.f )
    m_fLastValue = -32768.f;

  const float aTableQuant[8] = { 0.8984375f, 0.8984375f, 0.8984375f, 0.8984375f, 1.19921875f, 1.59765625f, 2.0f, 2.3984375f };
  m_fQuant = (float)((int)(aTableQuant[cValue&0x7] * m_fQuant));
  if ( m_fQuant < 127.f )
    m_fQuant = 127.f;
  if ( m_fQuant > 24576.f )
    m_fQuant = 24576.f;

  return m_fLastValue;
}


void AICA_StartSlot(_SLOT *slot)
{
	slot->active=1;
	slot->base=AICA.AICARAM+SA(slot);
	slot->cur_addr=0;
	slot->step=AICA_Step(slot);	
	Compute_EG(slot);
	slot->EG.state=ATTACK;
	slot->EG.volume=0;
	slot->Prev=slot->PPrev=0;
	Compute_LFO(slot);
	if(PCMS(slot)>=2)
	{
		unsigned char *p=(unsigned char *) (slot->base);
		slot->PrevQuant=127.f;
		slot->Prev=0;
		slot->Prev=DecodeADPCM(slot->Prev,p[0]&0xF,slot->PrevQuant);
		slot->PPrev=slot->Prev;
		slot->LastDecAddr=slot->cur_addr>>SHIFT;
		slot->ADStep=0;
	}
/*	if(!PCM8B(slot))
	{
		FILE *f=fopen("smp.raw","wb");
		fwrite(slot->base,LEA(slot)*2,1,f);
		fclose(f);
	}
*/
}

void AICA_StopSlot(_SLOT *slot,int keyoff)
{
	/*if(keyoff)
	{
		if(slot->EG.state!=RELEASE)
			slot->EG.state=RELEASE;
		return;
	}
	else*/
		slot->active=0;
	slot->data[0]&=~0x4000;
	slot->EG.state=RELEASE;
	slot->EG.volume=0;
	ErrorLogMessage("KEYOFF2 %d",slot->slot);
}

#define log2(n) (log((float) n)/log((float) 2))

void AICA_Init(int SampleRate)
{
	srate=SampleRate;
	memset(&AICA,0,sizeof(AICA));
	RevR=0;
	RevW=REVERB_DIF;
	memset(bufferrevl,0,sizeof(bufferrevl));
	memset(bufferrevr,0,sizeof(bufferrevr));
	MidiR=MidiW=0;
	MidiOutR=MidiOutW=0;
	MidiOutFill=0;
	MidiInFill=0;

#ifdef USEDSP
	AICADSP_Init(&AICA.DSP);
#endif


	//test1=fopen("d:\\test1.raw","wb");
	//test2=fopen("d:\\test2.raw","wb");
int i;
	for(i=0;i<0x400;++i)
	{
		double fcent=(double) 1200.0*log2((double)(((double) 1024.0+(double)i)/(double)1024.0));
		//float fcent=1.0+(float) i/1024.0;
		fcent=(double) 44100.0*pow(2.0,fcent/1200.0);
		FNS_Table[i]=(float) (1<<SHIFT) *fcent;
		//FNS_Table[i]=(i>>(10-SHIFT))|(1<<SHIFT);
		
	}
	for(i=0;i<0x10000;++i)
	{
		int iTL =(i>>0x8)&0xff;
		int iPAN=(i>>0x0)&0x1f;
		int iSDL=(i>>0x5)&0x07;

		float TL=1.0;
		float SegaDB=0;
		//2^(-(TL-2^4))
		/*if(iTL&0x01) TL*=0.95760;
		if(iTL&0x02) TL*=0.91700;
		if(iTL&0x04) TL*=0.84090;
		if(iTL&0x08) TL*=0.70711;
		if(iTL&0x10) TL*=0.50000;
		if(iTL&0x20) TL*=0.25000;
		if(iTL&0x40) TL*=0.06250;
		if(iTL&0x80) TL*=0.00391;*/
		if(iTL&0x01) SegaDB-=0.4;
		if(iTL&0x02) SegaDB-=0.8;
		if(iTL&0x04) SegaDB-=1.5;
		if(iTL&0x08) SegaDB-=3;
		if(iTL&0x10) SegaDB-=6;
		if(iTL&0x20) SegaDB-=12;
		if(iTL&0x40) SegaDB-=24;
		if(iTL&0x80) SegaDB-=48;

		TL=pow(10.0,SegaDB/20.0);

		float PAN=1.0;
		//2^(-2^(PAN-2))
		/*if(iPAN&0x1) PAN*=0.70711;
		if(iPAN&0x2) PAN*=0.50000;
		if(iPAN&0x4) PAN*=0.25000;
		if(iPAN&0x8) PAN*=0.06250;
		if(iPAN==0xf) PAN=0.0;*/

		SegaDB=0;
		if(iPAN&0x1) SegaDB-=3;
		if(iPAN&0x2) SegaDB-=6;
		if(iPAN&0x4) SegaDB-=12;
		if(iPAN&0x8) SegaDB-=24;

		if(iPAN==0xf) PAN=0.0;
		else PAN=pow(10.0,SegaDB/20.0);

		float LPAN,RPAN;

		if(iPAN<0x10)
		{
			LPAN=PAN;
			RPAN=1.0;
		}
		else
		{
			RPAN=PAN;
			LPAN=1.0;
		}

		float SDL=1.0;
		if(iSDL)
			SDL=pow(10.0,(SDLT[iSDL])/20.0);
		else
			SDL=0.0;

		TL/=10.0;

		LPANTABLE[i]=FIX((8.0*LPAN*TL*SDL));
		RPANTABLE[i]=FIX((8.0*RPAN*TL*SDL));


	}
	/*for(i=0;i<4;++i)	
		ARTABLE[i]=DRTABLE[i]=0;
	for(i=4;i<62;++i)*/
	for(i=0;i<62;++i)
	{
		//double t=BaseTimes[i];	//In ms
		double t=BaseTimes2[i/2]/AR2DR;	//In ms
		double step=(1023*1000.0)/((float) srate*t);
		double scale=(double) (1<<EG_SHIFT);
		ARTABLE[i]=(int) (step*scale);
		step/=AR2DR;
		DRTABLE[i]=(int) (step*scale);
	}
	ARTABLE[62]=DRTABLE[62]=1024<<EG_SHIFT;
	ARTABLE[63]=DRTABLE[63]=1024<<EG_SHIFT;
	
	for(i=0;i<64;++i)
		AICA.Slots[i].slot=i;

	LFO_Init();
	buffertmpl=(signed int*) malloc(44100*sizeof(signed int));
	buffertmpr=(signed int*) malloc(44100*sizeof(signed int));
	memset(buffertmpl,0,44100*sizeof(signed int));
	memset(buffertmpr,0,44100*sizeof(signed int));
}

void AICA_SetRAM(unsigned char *r,unsigned char *regs)
{
	AICA.AICARAM=r;
	AICA.datab=regs+0x2800;
	for(int i=0;i<64;++i)
	{
		AICA.Slots[i].datab=regs+0x0000+128*i;
	}
	//AICA.DSP.DSPRAM=AICARAM;
}

void AICA_UpdateSlotReg(int s,int r)
{
	_SLOT *slot=AICA.Slots+s;
	switch(r&0x3f)
	{
		case 0:
		case 1:
			if(KEYONEX(slot))
			{
				for(int sl=0;sl<64;++sl)
				{
					_SLOT *s2=AICA.Slots+sl;
					//if(s2->EG.state!=RELEASE)
					{
						if(KEYONB(s2) && !s2->active)
						{
							ErrorLogMessage("KEYON %d",sl);
							AICA_StartSlot(s2);
						}
						if(!KEYONB(s2) && s2->active)
						{
							//s2->active=0;
							AICA_StopSlot(s2,1);
							ErrorLogMessage("KEYOFF %d",sl);
						}
					}
				}
				slot->data[0]&=~0x8000;
			}
			break;
		case 0x18:
		case 0x19:
			
			//slot->step=AICA_Step(slot);	
			{
				int tmp=slot->step;
				slot->step=AICA_Step(slot);
				//if(slot->slot==0x28 && tmp!=slot->step)
				//	int a=1;
			}

			break;
		case 0x14:
		case 0x15:
			if(slot->active)
				int a=1;
//			if(RR(slot)==0x1f)
//				AICA_StopSlot(slot,0);
			slot->EG.RR=Get_RR(0,RR(slot));
			slot->EG.DL=0x1f-DL(slot);
			break;
		case 0x1C:
		case 0x1D:
			Compute_LFO(slot);
			break;
	}
}

void AICA_UpdateReg(int reg)
{
	switch(reg&0x3f)
	{
		case 0x6:
		case 0x7:
			break;
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			{
				int a=1;
			}
			break;
		case 0x18:
		case 0x19:
			if(AICA.Master)	
			{
				TimPris[0]=1<<((AICA.data[0x18/2]>>8)&0x7);
				TimCnt[0]=((AICA.data[0x18/2]&0xff)<<8)|(TimCnt[0]&0xff);
			}
			break;
		case 0x1a:
		case 0x1b:
			if(AICA.Master)	
			{
				TimPris[1]=1<<((AICA.data[0x1A/2]>>8)&0x7);
				TimCnt[1]=((AICA.data[0x1A/2]&0xff)<<8)|(TimCnt[1]&0xff);
			}
			break;
		case 0x1C:
		case 0x1D:
			if(AICA.Master)	
			{
				TimPris[2]=1<<((AICA.data[0x1C/2]>>8)&0x7);
				TimCnt[2]=((AICA.data[0x1C/2]&0xff)<<8)|(TimCnt[2]&0xff);
			}
			break;
		case 0x22:	//SCIRE
		case 0x23:
			if(AICA.Master)	
			{
				AICA.data[0x20/2]&=~AICA.data[0x22/2];
				CheckPendingIRQ();
			}
			break;
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
			if(AICA.Master)
			{
				IrqTimA=DecodeSCI(SCITMA);
				IrqTimBC=DecodeSCI(SCITMB);
				IrqMidi=DecodeSCI(SCIMID);
			}
			break;
	}
}

void AICA_UpdateSlotRegR(int slot,int reg)
{

}

void AICA_UpdateRegR(int reg)
{
	switch(reg&0x3f)
	{
		case 4:
		case 5:
			{
				unsigned short v=AICA.data[0x5/2];
				v&=0xff00;
				v|=MidiStack[MidiR];
				if(MidiR!=MidiW)
				{
					++MidiR;
					MidiR&=7;
					//Int68kCB(IrqMidi);
				}
				MidiInFill--;
				AICA.data[0x5/2]=v;
			}
			break;
		case 8:
		case 9:
			{
				unsigned char slot=AICA.data[0x8/2]>>11;	
				unsigned int CA=AICA.Slots[slot&0x3f].cur_addr>>(SHIFT+12);
				AICA.data[0x8/2]&=~(0x780);
				AICA.data[0x8/2]|=CA<<7;
			}
			break;
	}
}


void AICA_w8(unsigned int addr,unsigned char val)
{
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		ErrorLogMessage("Slot %02X Reg %02X write byte %04X",slot,addr,val);
		*((unsigned char *) (AICA.Slots[slot].datab+(addr^1))) = val;
		AICA_UpdateSlotReg(slot,addr&0x1f);
	}
	else if(addr<0x600)
	{
		*((unsigned char *) (AICA.datab+((addr&0xff)^1))) = val;
		AICA_UpdateReg(addr&0xff);
	}	
	else
		int a=1;
}

void AICA_w16(unsigned int addr,unsigned short val)
{
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		ErrorLogMessage("Slot %02X Reg %02X write word %04X",slot,addr,val);
		*((unsigned short *) (AICA.Slots[slot].datab+(addr))) = val;
		AICA_UpdateSlotReg(slot,addr&0x1f);
	}
	else if(addr<0x600)
	{
		*((unsigned short *) (AICA.datab+((addr&0xff)))) = val;
		AICA_UpdateReg(addr&0xff);
	}	
	else
		int a=1;
}

void AICA_w32(unsigned int addr,unsigned int val)
{
	addr&=0xffff;
	
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		ErrorLogMessage("Slot %02X Reg %02X write dword %08X",slot,addr,val);
		_asm rol val,16
		*((unsigned int *) (AICA.Slots[slot].datab+(addr))) = val;
		AICA_UpdateSlotReg(slot,addr&0x1f);
		AICA_UpdateSlotReg(slot,(addr&0x1f)+2);
	}
	else if(addr<0x600)
	{
		_asm rol val,16
		*((unsigned int *) (AICA.datab+((addr&0xff)))) = val;
		AICA_UpdateReg(addr&0xff);
		AICA_UpdateReg((addr&0xff)+2);
	}	
	else if(addr<0x800)
	{

	}
	else
		int a=1;
}

unsigned char AICA_r8(unsigned int addr)
{
	unsigned char v=0;
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		AICA_UpdateSlotRegR(slot,addr&0x1f);
		
		v=*((unsigned char *) (AICA.Slots[slot].datab+(addr^1)));
		ErrorLogMessage("Slot %02X Reg %02X Read byte %02X",slot,addr,v);
	}
	else if(addr<0x600)
	{
		AICA_UpdateRegR(addr&0xff);
		v= *((unsigned char *) (AICA.datab+((addr&0xff)^1)));
		//ErrorLogMessage("AICA Reg %02X Read byte %02X",addr&0xff,v);		
	}	
	else if(addr<0x700)
		v=0;
	return v;
}

unsigned short AICA_r16(unsigned int addr)
{
	unsigned short v=0;
	addr&=0xffff;
	if(addr<0x400)
	{
		int slot=addr/0x20;
		addr&=0x1f;
		AICA_UpdateSlotRegR(slot,addr&0x1f);
		v=*((unsigned short *) (AICA.Slots[slot].datab+(addr)));
		ErrorLogMessage("Slot %02X Reg %02X Read word %04X",slot,addr,v);
	}
	else if(addr<0x600)
	{
		AICA_UpdateRegR(addr&0xff);
		v= *((unsigned short *) (AICA.datab+((addr&0xff))));
		//ErrorLogMessage("AICA Reg %02X Read word %04X",addr&0xff,v);
	}	
	return v;
}

unsigned int AICA_r32(unsigned int addr)
{
	return 0xffffffff;
}

#define REVSIGN(v) ((~v)+1)


void AICA_TimersAddTicks2(int ticks)
{
		//Update timers
		WORD cnt;
		WORD step;
		//Timer A
		
		if(!TimPris[0])
		{
			//cnt=AICA.data[0x18/2]&0xff;
			cnt=TimCnt[0];
			if(cnt==0xffff)
				goto noTA;
			++cnt;
			++TimCnt[0];
			if(cnt>=TIMER_LIMITSA)
			{
				/*if((AICA.data[0x20/2]&AICA.data[0x1e/2])&0x40)	//timer pending ack
					int a=1;*/
				AICA.data[0x20/2]|=0x40;
				/*if(AICA.data[0x1e/2]&0x40)
					Int68kCB(IrqTimA);*/
				cnt=0xff;
				TimCnt[0]=0xffff;
			}
			step=1<<((AICA.data[0x18/2]>>8)&0x7);
			TimPris[0]=step;
			AICA.data[0x18/2]&=0xff00;
			AICA.data[0x18/2]|=cnt;
		}
//		else
			TimPris[0]--;
noTA:
;
		//Timer B
		
		if(!TimPris[1])
		{
			//cnt=AICA.data[0x1a/2]&0xff;
			cnt=TimCnt[1];
			if(cnt==0xffff)
				goto noTB;
			++cnt;
			++TimCnt[1];
			if(cnt>=TIMER_LIMITSB)
			{
				/*if((AICA.data[0x20/2]&AICA.data[0x1e/2])&0x80)	//timer pending ack
					int a=1;*/
				AICA.data[0x20/2]|=0x80;
				/*if(AICA.data[0x1e/2]&0x80)
					Int68kCB(IrqTimBC);*/
				cnt=0xff;
				TimCnt[1]=0xffff;
			}
			step=1<<((AICA.data[0x1a/2]>>8)&0x7);
			TimPris[1]=step;
			AICA.data[0x1a/2]&=0xff00;
			AICA.data[0x1a/2]|=cnt;
		}
//		else
			TimPris[1]--;
noTB:
;
		//Timer C
		
		if(!TimPris[2])
		{
			//cnt=AICA.data[0x1c/2]&0xff;
			cnt=TimCnt[2];
			if(cnt==0xffff)
				goto noTC;
			++cnt;
			++TimCnt[2];
			if(cnt>=TIMER_LIMITSC)
			{
				/*if((AICA.data[0x20/2]&AICA.data[0x1e/2])&0x100)	//timer pending ack
					int a=1;*/
				AICA.data[0x20/2]|=0x100;
				/*if(AICA.data[0x1e/2]&0x100)
					Int68kCB(IrqTimBC);*/
				cnt=0xff;
				TimCnt[2]=0xffff;
			}
			step=1<<((AICA.data[0x1c/2]>>8)&0x7);
			TimPris[2]=step;
			AICA.data[0x1c/2]&=0xff00;
			AICA.data[0x1c/2]|=cnt;
		}
//		else
			TimPris[2]--;
noTC:
;
}

void AICA_TimersAddTicks(int ticks)
{
	if(TimCnt[0]<=0xff00)
	{
		TimCnt[0]+=ticks << (8-((AICA.data[0x18/2]>>8)&0x7));
		if (TimCnt[0] > 0xFE00)
		{
			TimCnt[0] = 0xFFFF;
			AICA.data[0x20/2]|=0x40;
		}
		AICA.data[0x18/2]&=0xff00;
		AICA.data[0x18/2]|=TimCnt[0]>>8;
	}
	if(TimCnt[1]<=0xff00)
	{
		TimCnt[1]+=ticks << (8-((AICA.data[0x1a/2]>>8)&0x7));
		if (TimCnt[1] > 0xFE00)
		{
			TimCnt[1] = 0xFFFF;
			AICA.data[0x20/2]|=0x80;
		}
		AICA.data[0x1a/2]&=0xff00;
		AICA.data[0x1a/2]|=TimCnt[1]>>8;

	}
	if(TimCnt[2]<=0xff00)
	{
		TimCnt[2]+=ticks << (8-((AICA.data[0x1c/2]>>8)&0x7));
		if (TimCnt[2] > 0xFE00)
		{
			TimCnt[2] = 0xFFFF;
			AICA.data[0x20/2]|=0x100;
		}
		AICA.data[0x1c/2]&=0xff00;
		AICA.data[0x1c/2]|=TimCnt[2]>>8;
	}

}

#ifdef INTERPOLATE
#define CHOOSE(interp,nointerp) interp
#else
#define CHOOSE(interp,nointerp) nointerp
#endif

#ifdef FASTAICA

signed int *bufl1,*bufr1;
#define AICANAME(loop,lfo,alfo,stype) \
void AICA_Update##stype##lfo##alfo##loop(_SLOT *slot,unsigned int Enc,unsigned int nsamples)



//TRUST ON THE COMPILER OPTIMIZATIONS
#define AICATMPL(loop,lfo,alfo,stype) \
AICANAME(loop,lfo,alfo,stype)\
{\
	signed int sample;\
	DWORD addr;\
	for(unsigned int s=0;s<nsamples;++s)\
	{\
		int step=slot->step;\
		if(!slot->active)\
			return;\
		if(lfo) \
		{\
			step=step*PLFO_Step(&(slot->PLFO));\
			step>>=SHIFT; \
		}\
		if(stype==1)\
			addr=slot->cur_addr>>SHIFT;\
		else if(stype==0)\
			addr=(slot->cur_addr>>(SHIFT-1))&(~1);\
		else\
			addr=slot->cur_addr>>(SHIFT+1);\
		if(stype==1) /*8bits*/ \
		{\
			signed char *p=(signed char *) (slot->base+addr);\
			int s;\
			signed int fpart=slot->cur_addr&((1<<SHIFT)-1);\
			s=(int) p[0]*((1<<SHIFT)-fpart)+(int) p[1]*fpart;\
			/*sample=(s>>SHIFT)<<8;*/\
			sample=CHOOSE((s>>SHIFT)<<8,p[0]<<8);\
		}\
		else if(stype==0) /*16bits*/ \
		{\
			signed short *p=(signed short *) (slot->base+addr);\
			int s;\
			signed int fpart=slot->cur_addr&((1<<SHIFT)-1);\
			s=(int) p[0]*((1<<SHIFT)-fpart)+(int) p[1]*fpart;\
			/*sample=(s>>SHIFT);*/\
			sample=CHOOSE((s>>SHIFT),p[0]);\
		}\
		else /*ADPCM*/ \
		{\
			if((slot->cur_addr>>SHIFT)!=slot->LastDecAddr)\
			{\
				int hl=(slot->cur_addr>>SHIFT)&1;\
				unsigned char *p=(unsigned char *) (slot->base+(addr));\
				int steps=step>>SHIFT;\
				slot->PPrev=slot->Prev;\
				if(!steps)\
					steps=1;\
				while(steps--)\
				{\
					slot->Prev=DecodeADPCM(slot->Prev,(p[0]>>(4*hl))&0xF,slot->PrevQuant);\
					hl^=1;\
					if(!hl)\
						++p;\
				}\
				slot->LastDecAddr=slot->cur_addr>>SHIFT;\
			}\
			int s;\
			signed int fpart=slot->cur_addr&((1<<SHIFT)-1);\
			s=(int) slot->PPrev*((1<<SHIFT)-fpart)+(int) slot->Prev*fpart;\
			/*sample=(s>>SHIFT);*/\
			sample=CHOOSE(s>>SHIFT,slot->Prev);\
		}\
		slot->cur_addr+=step;\
		addr=slot->cur_addr>>SHIFT;\
		if(loop==0)\
		{\
			if(addr>=LEA(slot))\
			{\
				AICA_StopSlot(slot,0);\
			}\
		}\
		if(loop==1)\
		{\
			if(addr>LEA(slot))\
				slot->cur_addr=(LSA(slot)+1)<<SHIFT;\
		}\
		if(alfo)\
		{\
			sample=sample*ALFO_Step(&(slot->ALFO));\
			sample>>=SHIFT;\
		}\
		\
		sample=(sample*EG_Update(slot))>>SHIFT;\
	\
		*bufl1=*bufl1 + ((sample*LPANTABLE[Enc])>>SHIFT);\
		*bufr1=*bufr1 + ((sample*RPANTABLE[Enc])>>SHIFT);\
		++bufl1;\
		++bufr1;\
	}\
}

AICATMPL(0,0,0,0) AICATMPL(0,0,0,1) AICATMPL(0,0,0,2) AICATMPL(0,0,0,3)
AICATMPL(0,0,1,0) AICATMPL(0,0,1,1) AICATMPL(0,0,1,2) AICATMPL(0,0,1,3)
AICATMPL(0,1,0,0) AICATMPL(0,1,0,1) AICATMPL(0,1,0,2) AICATMPL(0,1,0,3)
AICATMPL(0,1,1,0) AICATMPL(0,1,1,1) AICATMPL(0,1,1,2) AICATMPL(0,1,1,3)
AICATMPL(1,0,0,0) AICATMPL(1,0,0,1) AICATMPL(1,0,0,2) AICATMPL(1,0,0,3)
AICATMPL(1,0,1,0) AICATMPL(1,0,1,1) AICATMPL(1,0,1,2) AICATMPL(1,0,1,3)
AICATMPL(1,1,0,0) AICATMPL(1,1,0,1) AICATMPL(1,1,0,2) AICATMPL(1,1,0,3)
AICATMPL(1,1,1,0) AICATMPL(1,1,1,1) AICATMPL(1,1,1,2) AICATMPL(1,1,1,3)

#undef AICATMPL
#define AICATMPL(loop,lfo,alfo,stype) \
 AICA_Update##stype##lfo##alfo##loop ,


typedef void (*_AICAUpdateModes)(_SLOT *,unsigned int,unsigned int);

_AICAUpdateModes AICAUpdateModes[]=
{
	AICATMPL(0,0,0,0) AICATMPL(0,0,0,1) AICATMPL(0,0,0,2) AICATMPL(0,0,0,3)
	AICATMPL(0,0,1,0) AICATMPL(0,0,1,1) AICATMPL(0,0,1,2) AICATMPL(0,0,1,3)
	AICATMPL(0,1,0,0) AICATMPL(0,1,0,1) AICATMPL(0,1,0,2) AICATMPL(0,1,0,3)
	AICATMPL(0,1,1,0) AICATMPL(0,1,1,1) AICATMPL(0,1,1,2) AICATMPL(0,1,1,3)
	AICATMPL(1,0,0,0) AICATMPL(1,0,0,1) AICATMPL(1,0,0,2) AICATMPL(1,0,0,3)
	AICATMPL(1,0,1,0) AICATMPL(1,0,1,1) AICATMPL(1,0,1,2) AICATMPL(1,0,1,3)
	AICATMPL(1,1,0,0) AICATMPL(1,1,0,1) AICATMPL(1,1,0,2) AICATMPL(1,1,0,3)
	AICATMPL(1,1,1,0) AICATMPL(1,1,1,1) AICATMPL(1,1,1,2) AICATMPL(1,1,1,3)

};
/*
#define SCANLINES	210

void AICA_CpuRunScanline()
{
	int slice=12000000/(44100);
	static unsigned int smp=0;
	smp+=(unsigned int) ((256.0*44100.0)/((float) SCANLINES*60));
	int lastdiff=0;
	for(;smp&0xffffff00;)
	{
		lastdiff=Intf68k->Run(slice-lastdiff);
		AICA_TimersAddTicks(1);
		CheckPendingIRQ();
		smp-=0x100;
	}
}
*/
void AICA_DoSamples(int nsamples)
{

	static int lastdiff=0;
	signed short *bufr,*bufl;
	
	
	for(int sl=0;sl<64;++sl)
	{
		bufr1=buffertmpr;
		bufl1=buffertmpl;
//		if(sl==0x15)
//			continue;

		if(AICA.Slots[sl].active)
		{
			_SLOT *slot=AICA.Slots+sl;
			unsigned int disdl=DISDL(slot);
			unsigned int efsdl=EFSDL(slot);
			unsigned int tl=TL(slot);
			unsigned short Enc=((TL(slot))<<0x8)|((DIPAN(slot))<<0x0)|((DISDL(slot))<<0x5);
			//unsigned short Enc=(0x00)|((DIPAN(slot))<<0x8)|((0x7)<<0xd);
			//unsigned int mode=LPCTL(slot);
			unsigned int mode=PCMS(slot);

			if(PLFOS(slot))
				mode|=8;
			if(ALFOS(slot))
				mode|=4;
			if(LPCTL(slot))
				mode|=0x10;

			AICAUpdateModes[mode](slot,Enc,nsamples);
//norender:
//;
		}
	}
	bufr=bufferr;
	bufl=bufferl;
	bufr1=buffertmpr;
	bufl1=buffertmpl;
	for(int s=0;s<nsamples;++s)
	{
#define ICLIP16(x) (x<-32768)?-32768:((x>32767)?32767:x)
		signed int smpl=*bufl1;
		signed int smpr=*bufr1;

#ifdef REVERB
		smpl+=bufferrevl[RevR];
		smpr+=bufferrevr[RevR];
		bufferrevl[RevW]=((smpl<<0)/REVERB_DIV)>>0;
		bufferrevr[RevW]=((smpr<<0)/REVERB_DIV)>>0;
		++RevW;
		if(RevW==REVERB_LEN)
			RevW=0;
		++RevR;
		if(RevR==REVERB_LEN)
			RevR=0;
#endif
		*bufl=ICLIP16(smpl);
		*bufr=ICLIP16(smpr);
		*bufl1=0;
		*bufr1=0;
		++bufl;
		++bufr;
		++bufl1;
		++bufr1;

	}
}


#else


signed int inline AICA_UpdateSlot(_SLOT *slot)
{
	signed int sample;
	int step=slot->step;
	DWORD addr;

	
	if(PLFOS(slot)!=0)
	{
		step=step*PLFO_Step(&(slot->PLFO));
		step>>=SHIFT;
	}

	if(PCMS(slot)==1)
		addr=slot->cur_addr>>SHIFT;
	else if(PCMS(slot)==0)
		addr=(slot->cur_addr>>(SHIFT-1))&(~1);
	else
		addr=slot->cur_addr>>(SHIFT+1);

	if(PCMS(slot)==1)	//8 bit signed
	{	
		signed char *p=(signed char *) (slot->base+(addr));
		int s;
		signed int fpart=slot->cur_addr&((1<<SHIFT)-1);
		signed int prev=slot->Prev;
		slot->Prev=p[0];
		s=(int) prev*(((1<<SHIFT)-1)-fpart)+(int) p[0]*fpart;
		/*sample=(s>>SHIFT)<<8;*/
		sample=CHOOSE((s>>SHIFT)<<8,p[0]<<8);
		//sample=0;
	}
	else if(PCMS(slot)==0)	//16 bit signed (endianness?)
	{
		signed short *p=(signed short *) (slot->base+addr);
		int s;
		signed int fpart=slot->cur_addr&((1<<SHIFT)-1);
		signed int prev=slot->Prev;
		signed int smpl=p[0];
		if(smpl==-32768 && !(prev&0x8000))
		{
			//printf("//wtf is this ? why is it here ?"); // -> it actualy fixes SA2 music . why ?
			smpl=0x7fff;
		}
		slot->Prev=smpl;
		s=(int) prev*(((1<<SHIFT)-1)-fpart)+(int) smpl*fpart;
		/*sample=(s>>SHIFT);*/
		sample=CHOOSE(s>>SHIFT,p[0]);
		//sample=0;
	}
	else	//ADPCM
	{
		slot->ADStep+=step;
		if(slot->ADStep>>SHIFT)
		{
			int hl=(slot->cur_addr>>SHIFT)&1;
			unsigned char *p=(unsigned char *) (slot->base+(addr));
			int ca=slot->cur_addr>>SHIFT;
			int steps=slot->ADStep>>SHIFT;
			
			slot->PPrev=slot->Prev;
			if(steps>1)
				int a=1;
			if(!steps)
				steps=1;
			slot->ADStep&=(1<<SHIFT)-1;
			while(steps--)
			{
				/*if(hl==0)
					fwrite(p,1,1,test2);*/
				slot->Prev=DecodeADPCM(slot->Prev,(p[0]>>(4*hl))&0xF,slot->PrevQuant);
				hl^=1;
				if(!hl)	//UFF cuidado con el fin de bucle
				{
					++ca;
					if(ca>=LEA(slot))
					{
						ca=LSA(slot);
						hl=ca&1;
						p=(unsigned char *) (slot->base+(ca>>1));
					}
					else
						++p;
				}
			}
			slot->LastDecAddr=slot->cur_addr>>SHIFT;
		}
		int s;
		signed int fpart=slot->cur_addr&((1<<SHIFT)-1);
		s=(int) slot->PPrev*((1<<SHIFT)-fpart)+(int) slot->Prev*fpart;
		//sample=(s>>SHIFT);
		sample=CHOOSE(s>>SHIFT,slot->Prev);
		
		
	}

	slot->cur_addr+=step;
	addr=slot->cur_addr>>SHIFT;
	switch(LPCTL(slot))
	{
	case 0:	//no loop
		if(addr>=LEA(slot))
		{
			//slot->active=0;
			AICA_StopSlot(slot,0);
		}
		break;
	case 1: //normal loop
		if(addr>=LEA(slot))
		{
			slot->cur_addr=(LSA(slot))<<SHIFT;
			slot->ADStep=0;
		}
		break;
	}

	if(ALFOS(slot)!=0)
	{
		sample=sample*ALFO_Step(&(slot->ALFO));
		sample>>=SHIFT;
	}
	
	sample=(sample*EG_Update(slot))>>SHIFT;

	return sample;
}

void AICA_CpuRunScanline()
{

}

void AICA_DoSamples(int nsamples)
{
	

	for(int s=0;s<nsamples;++s)
	{
		signed int smpl=0;
		signed int smpr=0;

		for(int sl=0;sl<64;++sl)
		{
			if(AICA.Slots[sl].active)
			{
				_SLOT *slot=AICA.Slots+sl;
				unsigned short Enc=((TL(slot))<<0x8)|((DIPAN(slot))<<0x0)|((DISDL(slot))<<0x5);
				signed int sample=0;
				//if(sl==0x28)
					sample=AICA_UpdateSlot(slot);
				/*unsigned char ef=EFSDL(slot);
				ef+=DISDL(slot);
				if(ef>0xf) ef=0xf;
				unsigned short Enc=((TL(slot))<<0x0)|((DIPAN(slot))<<0x8)|((ef)<<0xd);
				*/
				if(LPSLNK(slot))
					int a=1;

#ifdef USEDSP
				AICADSP_SetSample(&AICA.DSP,/*sample*/(sample*LPANTABLE[(Enc|0xE0)&0xFFE0])>>(SHIFT-1),ISEL(slot),IMXL(slot));
#endif
				
				{
					smpl+=(sample*LPANTABLE[Enc])>>SHIFT;
					smpr+=(sample*RPANTABLE[Enc])>>SHIFT;
				}
				
			}
		}
#ifdef USEDSP
		AICADSP_Step(&AICA.DSP);
//		smpl=0;
//		smpr=0;
		for(int i=0;i<16;++i)
		{
			unsigned int ef=(AICA.datab[0x2000-0x2800+4*i+1]>>1)&0x7;
			if(ef)
			{
				unsigned int epan=AICA.datab[0x2000-0x2800+4*i];
				unsigned short Enc=0|((epan)<<0x0)|((ef)<<0x5);
				smpl+=(AICA.DSP.EFREG[i]*LPANTABLE[Enc])>>SHIFT;
				smpr+=(AICA.DSP.EFREG[i]*RPANTABLE[Enc])>>SHIFT;
			}
			
		}
		
#endif
#define ICLIP16(x) (x<-32768)?-32768:((x>32767)?32767:x)
#ifdef REVERB
		smpl+=bufferrevl[RevR];
		smpr+=bufferrevr[RevR];
		bufferrevl[RevW]=((smpl<<0)/REVERB_DIV)>>0;
		bufferrevr[RevW]=((smpr<<0)/REVERB_DIV)>>0;
		++RevW;
		if(RevW==REVERB_LEN)
			RevW=0;
		++RevR;
		if(RevR==REVERB_LEN)
			RevR=0;
#endif
		bufferl[s]=ICLIP16(smpl);
		bufferr[s]=ICLIP16(smpr);
		//fwrite(&bufferr[s],2,1,test1);
	}
}
#endif


void AICA_MidiIn(BYTE val)
{
	ErrorLogMessage("Midi Buffer push %02X",val);

	MidiStack[MidiW++]=val;
	MidiW&=7;
	MidiInFill++;
	//Int68kCB(IrqMidi);
//	AICA.data[0x20/2]|=0x8;
}

void AICA_MidiOutW(BYTE val)
{
	ErrorLogMessage("Midi Out Buffer push %02X",val);
	MidiStack[MidiOutW++]=val;
	MidiOutW&=7;
	++MidiOutFill;
}


unsigned char AICA_MidiOutR()
{
	unsigned char val;
	if(MidiOutR==MidiOutW)
		return 0xff;

	val=MidiStack[MidiOutR++];
	ErrorLogMessage("Midi Out Buffer pop %02X",val);
	MidiOutR&=7;
	--MidiOutFill;
	return val;
}

unsigned char AICA_MidiOutFill()
{
	return MidiOutFill;
}

unsigned char AICA_MidiInFill()
{
	return MidiInFill;
}


int AICA_IRQCB(int)
{
	CheckPendingIRQ();
	return -1;
}


void AICA_SetBuffers(signed short *l,signed short *r)
{
	bufferl=l;
	bufferr=r;
}

unsigned int AICA_GetPlayPos(int slot)
{
	return AICA.Slots[slot&0x3f].cur_addr>>SHIFT;
}

unsigned int AICA_GetEnvState(int slot)
{
	_SLOT *s=&AICA.Slots[slot&0x3f];
	return (s->EG.state<<13)|(0x3ff-(s->EG.volume>>EG_SHIFT));
}


void AICA_UpdateDSP(unsigned int iAddress)
{
#ifdef USEDSP
		iAddress&=~1;
		unsigned short val=*((unsigned short *) (AICA.datab+iAddress-0x2800));
		unsigned int addr=iAddress-0x3000;
		//DSP
		if(addr<0x200)	//COEF
			*(((unsigned short *) AICA.DSP.COEF) + ((addr/4)))=val;
		else if(addr<0x400)
			*(((unsigned short *) AICA.DSP.MADRS) + (((addr-0x200)/4)))=val;
		else if(addr>=0x400 && addr<0xC00)
			*(((unsigned short *) AICA.DSP.MPRO) + ((addr-0x400)/4))=val;
		else
			int a=1;
		if(addr==0xBFC)
		{
			AICADSP_Start(&AICA.DSP);
		}
		int a=1;
#endif
}