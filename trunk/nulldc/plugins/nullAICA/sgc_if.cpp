﻿#include "sgc_if.h"
#include "dsp.h"
#include "mem.h"
#include <math.h>

//Sound generation , mixin , and chanel regs emulation

SampleType mixl;
SampleType mixr;

//x.15
s32 volume_lut[16];
u32 SendLevel[16]={0xF000<<3,14<<3,13<<3,12<<3,11<<3,10<<3,9<<3,8<<3,7<<3,6<<3,5<<3,4<<3,3<<3,2<<3,1<<3,0<<3};
s32 tl_lut[256];	//xx.15 format :)

//in ms :)
float AEG_Attack_Time[]=
{
	-1,-1,8100.0,6900.0,6000.0,4800.0,4000.0,3400.0,3000.0,2400.0,2000.0,1700.0,1500.0,
	1200.0,1000.0,860.0,760.0,600.0,500.0,430.0,380.0,300.0,250.0,220.0,190.0,150.0,130.0,110.0,95.0,
	76.0,63.0,55.0,47.0,38.0,31.0,27.0,24.0,19.0,15.0,13.0,12.0,9.4,7.9,6.8,6.0,4.7,3.8,3.4,3.0,2.4,
	2.0,1.8,1.6,1.3,1.1,0.93,0.85,0.65,0.53,0.44,0.40,0.35,0.0,0.0
};
float AEG_DSR_Time[]=
{	-1,-1,118200.0,101300.0,88600.0,70900.0,59100.0,50700.0,44300.0,35500.0,29600.0,25300.0,22200.0,17700.0,
	14800.0,12700.0,11100.0,8900.0,7400.0,6300.0,5500.0,4400.0,3700.0,3200.0,2800.0,2200.0,1800.0,1600.0,1400.0,1100.0,
	920.0,790.0,690.0,550.0,460.0,390.0,340.0,270.0,230.0,200.0,170.0,140.0,110.0,98.0,85.0,68.0,57.0,49.0,43.0,34.0,
	28.0,25.0,22.0,18.0,14.0,12.0,11.0,8.5,7.1,6.1,5.4,4.3,3.6,3.1
};

#define AEG_STEP_BITS (16)
//Steps per sample
u32 AEG_ATT_SPS[64];
u32 AEG_DSR_SPS[64];

const char* stream_names[]=
{
	"0: 16-bit PCM (two's complement format)",
	"1: 8-bit PCM (two's complement format)",
	"2: 4-bit ADPCM (Yamaha format)",
	"3: 4-bit ADPCM long stream"
};

//x.8 format
const s32 adpcm_qs[8] = 
{
	0x0e6, 0x0e6, 0x0e6, 0x0e6, 0x133, 0x199, 0x200, 0x266,
};
//x.3 format
const s32 adpcm_scale[8] = 
{
	1,3,5,7,9,11,13,15,
};

void AICA_Sample();

//Remove the fractional part , with rounding ;) -- does not need an extra bit
#define well(a,bits) (((a) + ((1<<(bits-1))))>>bits)
//Remove the franctional part by chopping..
#define FPChop(a,bits) ((a)>>bits)

#define FPs FPChop
//Fixed point mul w/ rounding :)
#define FPMul(a,b,bits) (FPs(a*b,bits))

#define VOLPAN(value,vol,pan,outl,outr) {s32 temp;\
		temp=FPMul((value),volume_lut[(vol)],15);\
		u32 t_pan=(pan);\
		SampleType Sc=FPMul(temp,volume_lut[0xF-(t_pan&0xF)],15);\
	\
		if (t_pan& 0x10)\
		{\
			outl+=temp;\
			outr+=Sc ;\
		}\
		else\
		{\
			outl+=Sc;\
			outr+=temp;\
		}\
		}
s16 pl=0,pr=0;

struct DSP_OUT_VOL_REG
{
	//--	EFSDL[3:0]	--	EFPAN[4:0]
	
	u32 EFPAN:5;
	u32 res_1:3;

	u32 EFSDL:4;
	u32 res_2:4;

	u32 pad:16;
};
DSP_OUT_VOL_REG* dsp_out_vol;

#pragma pack (1)
//All regs are 16b , alligned to 32b (upper bits 0?)
struct ChannelCommonData
{
	//+00 [0]
	//SA is half at reg 0 and the rest at reg 1
	u32 SA_hi:7;
	u32 PCMS:2;
	u32 LPCTL:1;
	u32 SSCTL:1;
	u32 res_1:3;
	u32 KYONB:1;
	u32 KYONEX:1;

	u32 pad_2:16;

	//+04 [1]
	//SA (defined above)
	u32 SA_low:16;

	u32 pad_3:16;

	//+08 [2]
	u32 LSA:16;

	u32 pad_4:16;

	//+0C [3]
	u32 LEA:16;

	u32 pad_5:16;

	//+10 [4]
	u32 AR:5;
	u32 res_2:1;
	u32 D1R:5;
	u32 D2R:5;

	u32 pad_7:16;

	//+14 [5]
	u32 RR:5;
	u32 DL:5;
	u32 KRS:4;
	u32 LPSLNK:1;
	u32 res_3:1;

	u32 pad_8:16;

	//+18[6]
	u32 FNS:10;
	u32 rez_8_1:1;
	u32 OCT:4;
	u32 rez_8_2:1;

	u32 pad_9:16;

	//+1C	RE	LFOF[4:0]	PLFOWS	PLFOS[2:0]	ALFOWS	ALFOS[2:0]
	u32 ALFOS:3;
	u32 ALFOWS:2;

	u32 PLFOS:3;
	u32 PLFOWS:2;

	u32 LFOF:5;
	u32 LFORE:1;

	u32 pad_10:16;

	//+20	--	IMXL[3:0]	ISEL[3:0]
	u32 ISEL:4;
	u32 IMXL:4;
	u32 rez_20_0:8;

	u32 pad_11:16;

	//+24	--	DISDL[3:0]	--	DIPAN[4:0]
	u32 DIPAN:5;
	u32 rez_24_0:3;
	
	u32 DISDL:4;
	u32 rez_24_1:4;

	u32 pad_12:16;
	

	//+28	TL[7:0]	--	Q[4:0]
	u32 Q:5;
	u32 rez_28_0:3;

	u32 TL:8;

	u32 pad_13:16;

	//+2C	--	FLV0[12:0]
	u32 pad_14:16;

	//+30	--	FLV1[12:0]
	u32 pad_15:16;

	//+34	--	FLV2[12:0]
	u32 pad_16:16;

	//+38	--	FLV3[12:0]
	u32 pad_17:16;

	//+3C	--	FLV4[12:0]
	u32 pad_18:16;
	
	//+40	--	FAR[4:0]	--	FD1R[4:0]
	u32 pad_19:16;

	//+44	--	FD2R[4:0]	--	FRR[4:0]
	u32 pad_20:16;
};



enum _EG_state
{
	EG_Attack = 0,
	EG_Decay1 = 1,
	EG_Decay2 = 2,
	EG_Release = 3
};

/*
	KEY_OFF->KEY_ON : Resets everything, and starts playback (EG: A)
	KEY_ON->KEY_ON  : nothing
	KEY_ON->KEY_OFF : Switches to RELEASE state (does not disable channel)

*/
struct AicaChannel
{
	static AicaChannel Chans[64];

	union
	{
		ChannelCommonData* ChanData;
		u32* ChanData_raw;
	};
	u32 Channel;

	bool enabled;

	u32 CA;
	fp_22_10 scnt;

	u32 update_rate;	//22.10

	struct
	{
		s32 val;
		s32 GetValue() { return val>>AEG_STEP_BITS;}
		void SetValue(u32 aegb) { val=aegb<<AEG_STEP_BITS; }

		_EG_state state;
	} AEG;
	
	struct
	{
		s32 value;
		_EG_state state;
	} FEG;//how can this be emulated anyway ? -> using a digital low pass filter :p
	
	struct 
	{
		u32 counter;
		u32 start_value;
		u8	state;
		void Step() { counter--;if (counter==0) { state++; counter=start_value; } }
		void Reset() { state=0; counter=start_value; }
		void SetStartValue(u32 nv) { start_value=nv;counter=start_value; }
	} lfo;

	u32 GetALFO()
	{
		//return 0;//	for now :)
		u32 rv;
		switch(ChanData->ALFOWS)
		{
			case 0:	//sawtooth
				rv=lfo.state;
				break;

			case 1: //square
				rv=lfo.state&0x80?255:0;
				break;

			case 2: //triangle
				rv=(lfo.state&0x7f)^(lfo.state&0x80 ? 0x7F:0);
				rv<<=1;
				break;

			case 3://random ! .. not :p
				rv=(lfo.state>>3)^(lfo.state<<3)^(lfo.state&0xE3);
				break;
		}
		return rv>>(8-ChanData->ALFOS);
	}

	//used on adpcm decoding
	s32 last_quant;//32.0

	//used on apdcm decoding
	SampleType prev_sample;

	u32 looped;
	void Init(u32 channel,u8* reg_base)
	{
		enabled=false;
		AEG.SetValue(0x3FF);
		AEG.state=EG_Release;

		Channel=channel;
		ChanData=(ChannelCommonData*)&reg_base[channel*0x80];
		ChanData_raw=(u32*)ChanData;
		CalcUpdateRate();
		looped=0;
	}


	void KEY_ON()
	{
		verify(ChanData->KYONB);
		if (AEG.state==EG_Release)
		{
			//if it was off then turn it on !
			enabled=true;

			AEG.state=EG_Attack;// reset AEG
			AEG.SetValue(0x3FF);//start from 0x3FF ? .. it seems so !

			//Reset sampling state
			CA=0;
			scnt.full=0;
			looped=0;

			last_quant=127;
			prev_sample=0;

			CalcUpdateRate();
			printf("[%d] KEY_ON %s @ %f hrz, loop : %d\n",Channel,stream_names[ChanData->PCMS],(44100.0*update_rate)/1024,ChanData->LPCTL);
			//Decode first sample
			//Decoder_Step();
		}
		else
		{
			//ingore ?
		}

	}
	void KEY_OFF()
	{
		verify(!ChanData->KYONB);
		if (AEG.state!=EG_Release)
		{
			printf("[%d] KEY_OFF -> Release\n",Channel);
			AEG.state=EG_Release;
			//switch to release state
		}
		else
		{
			//ingore ?
		}
	}
	//enables the channel, resets state ;)
	void enable()
	{
		
	}
	void EnterReleaseState()
	{
		AEG.state=EG_Release;
		ChanData->KYONB=0;
	}
	//disables (freezes) the channel
	void disable()
	{
		ChanData->KYONB=0;
		printf("[%d] Channel disabled\n",Channel);
		AEG.SetValue(0x3ff);
		enabled=false;
	}
	
	void CalcUpdateRate()
	{
		u32 oct=ChanData->OCT;

		//same code as above realy , just faster =P
		update_rate =1024 | ChanData->FNS;
		if (oct& 8)
		{
			update_rate>>=(16-oct);
		}
		else
		{
			update_rate<<=oct;
		}
	}

	//must be done after GetSample ;)
	void Sample_step()
	{
		scnt.full+=update_rate;
		while(scnt.ip>0)
		{
			scnt.ip--;
			
			//keep adpcm up to date
			if (ChanData->PCMS>=2)
				prev_sample=DecodeADPCM(CA,prev_sample,last_quant);

			CA++;

			if (ChanData->LPSLNK)
			{
				if ((AEG.state==EG_Attack) && (CA>=ChanData->LSA))
				{
					printf("[%d]LPSLNK : Switching to EG_Decay1 %X\n",Channel,AEG.GetValue());
					AEG.state=EG_Decay1;
				}
			}

			u32 ca_t=CA;
			if (ChanData->PCMS==3)
				ca_t&=~3;	//adpcm "stream" mode :)

			if (ca_t>=ChanData->LEA)
			{
				looped=1;
				CA=ChanData->LSA;
				if (ChanData->LPCTL)
				{
					if (ChanData->PCMS==2) //if in adpcm non-stream mode, reset the decoder
					{	
						last_quant=127;
						prev_sample=0;
					}
				}
				else
				{
					disable();
					//EnterReleaseState(); //what is realy correct ?
				}
			}
		}


	}
	SampleType DecodeADPCM(u32 pos,s32 prev,s32& quant)
	{
		u32 addr=(ChanData->SA_hi<<16) | (ChanData->SA_low);
		u8* ptr=(u8*)&aica_ram[addr+(pos>>1)];

		u32 sample=*ptr;

		if (pos&1)
			sample>>=4; //2nd sample is HI nible ;)
		else
			sample&=0xF;//first sample is LOW nible !

		u32 sign=1;
		if (sample&8)
			sign=-1;
		u32 data=sample&7;

		/*(1 - 2 * L4) * (L3 + L2/2 +L1/4 + 1/8) * quantized width (ƒΆn) + decode value (Xn - 1) */
		SampleType rv = prev + sign*((quant*adpcm_scale[data])>>3);

		//if (Channel==0x30)
		//	printf("%d -> %d\n",sample,rv);

		quant = (quant * adpcm_qs[data])>>8;

		clip(quant,127,24576);
		clip16(rv);
		return rv;
	}
	SampleType GetSample()
	{
		u32 addr=(ChanData->SA_hi<<16) | (ChanData->SA_low);
		SampleType s0,s1;
		switch(ChanData->PCMS)
		{
		case 0:
			{
				s16* ptr=(s16*)&aica_ram[addr+(CA<<1)];
				s0=ptr[0];
				s1=ptr[1];
				if (s1==-32768 && 0==(s0&0x8000))
					s1&=0x7FFF;
			}
			break;

		case 1:
			{
				s8* ptr=(s8*)&aica_ram[addr+(CA)];
				s0=ptr[0]<<8;
				s1=ptr[1]<<8;
			}
			break;

		case 2:
		case 3:
			{
				s32 q=last_quant;
				s0=DecodeADPCM(CA,prev_sample,q);
				s1=DecodeADPCM(CA+1,s0,q);
			}
			break;
		}
		SampleType rv;

		rv=FPMul(s0,(s32)(scnt.fp),10);
		rv+=FPMul(s1,(s32)(1023-scnt.fp),10);

		//make sure its still in range
		verify(((s16)rv)==rv);
		return rv;
	}


	//Set the AEG
	u32 AEG_EffRate(u32 re)
	{
		s32 rv=ChanData->KRS+(ChanData->FNS>>9) + re*2;
		if (ChanData->KRS==0xF)
			rv-=0xF;
		if (ChanData->OCT&8)
			rv-=(16-ChanData->OCT)*2;
		else
			rv+=ChanData->OCT*2;

		if (rv<0)
			rv=0;
		if (rv>0x3f)
			rv=0x3f;
		return rv;
	}
	void AEG_step()
	{
		switch(AEG.state)
		{
		case EG_Attack:
			{
				//wii
				AEG.val-=AEG_ATT_SPS[AEG_EffRate(ChanData->AR)];
				if (AEG.GetValue()<=0)
				{
					AEG.SetValue(0);
					if (!ChanData->LPSLNK)
					{
						printf("[%d]AEG_step : Switching to EG_Decay1 %d\n",Channel,AEG.GetValue());
						AEG.state=EG_Decay1;
					}
				}
			}
			break;
		case EG_Decay1:
			{
				//x2
				AEG.val+=AEG_DSR_SPS[AEG_EffRate(ChanData->D1R)];
				if (((u32)AEG.GetValue()>>5)>=ChanData->DL)
				{
					printf("[%d]AEG_step : Switching to EG_Decay2 @ %x\n",Channel,AEG.GetValue());
					AEG.state=EG_Decay2;
				}
			}
			break;
		case EG_Decay2:
			{
				//x3
				AEG.val+=AEG_DSR_SPS[AEG_EffRate(ChanData->D2R)];
				if (AEG.GetValue()>=0x3FF)
				{
					AEG.SetValue(0x3FF);
					EnterReleaseState();
					printf("[%d]AEG_step : Switching to EG_Release @ %x\n",Channel,AEG.GetValue());
				}
			}
			break;
		case EG_Release: //olny on key_off ?
			{
				AEG.val+=AEG_DSR_SPS[AEG_EffRate(ChanData->RR)];
				
				if (AEG.GetValue()>=0x3FF)
				{
					AEG.SetValue(0x3FF); //mnn , should we do anything about it running wild ?
					disable();
				}
			}
			break;
		}
	}
	void Generate()
	{
		if (!enabled)
			return;

		AEG_step(); //here ? or after sample ? duno...
					//lets say it is here for now .. otherwise the first sample whould allways be muted o.O
		
		//If first sample, decode it :)

		SampleType sample = GetSample();

		u32 const max_att=(16<<4)-1;

		u32 baseatt=ChanData->TL+GetALFO()+(AEG.GetValue()>>2);	//common to DISDL,IMXL

		u32 directatt=baseatt+SendLevel[ChanData->DISDL];
		u32 panatt=directatt+ SendLevel[(~ChanData->DIPAN)&0xF];
		u32 dspatt=baseatt+SendLevel[ChanData->IMXL];

		directatt=min(directatt,max_att);
		panatt=min(panatt,max_att);
		dspatt=min(dspatt,max_att);

		SampleType oFull=FPMul(sample,tl_lut[directatt],15);
		SampleType oPan=FPMul(sample,tl_lut[panatt],15);
		SampleType oDsp=FPMul(sample,tl_lut[dspatt],15);

		static bool channel_mute=false;

		if (!channel_mute)
		{
			dsp.MIXS[ChanData->ISEL]+=oDsp;

			if (ChanData->DIPAN&0x10)
			{	//0x1* -> R decreases
				mixl+=oFull;
				mixr+=oPan;
			}
			else
			{	//0x0* -> L decreases
				mixl+=oPan;
				mixr+=oFull;
			}
		}

		//GOOD ! now we finished w/ sound generation ...
		//Loop procecing / misc stuff(tm)
		Sample_step();
		lfo.Step();
	}
	void RegWrite(u32 offset)
	{
		switch(offset)
		{
		case 0x01:	//yay ?
			if (ChanData->KYONEX)
			{
				ChanData->KYONEX=0;
				for (int i=0;i<64;i++)
				{
					if (Chans[i].ChanData->KYONB)
						Chans[i].KEY_ON();
					else
						Chans[i].KEY_OFF();
				}
			}
			break;

		case 0x1C+1:	//just for LFORE && LFOF :)
			if (ChanData->LFORE)
			{
				lfo.Reset();
			}
			{
				int N=ChanData->LFOF;
				int S = N >> 2;
				int M = (~N) & 3;
				int G = 128>>S;
				int L = (G-1)<<2;
				int O = L + G * (M+1);
				lfo.SetStartValue(O);
			}
			
			break;

			//mhhrpppfff
		case 0x18:
		case 0x19:
			CalcUpdateRate();
			break;
		}
	} 
	//see , this had to moved out of this ohh-so-nice class b/c of C++ lameness .. cant beat it xD
	/*
	KYONB
	This register registers KEY_ON, OFF.
	(If KEY_ON is to be registered simultaneously, set this bit to "1" for all slots to be turned ON, and then write a "1" to KEYONEX for one of the slots.)
	SSCTL
	0: Use the data in external memory (SDRAM) as sound input data.
	1: Use noise as sound input data.
	LPCTL
	0:	Loop OFF (The LSA and LEA settings are required; once LEA is reached, processing ends.)
	1: Forward loop.

	PCMS[1:0]
	(Cannot be changed during ADPCM playback.)
	0: 16-bit PCM (two's complement format)
	1: 8-bit PCM (two's complement format)
	2: 4-bit ADPCM (Yamaha format)
	3: 4-bit ADPCM long stream

	SA[22:0]
	This register specifies the starting address for the sound data in terms of the byte address.  However, 
	when 	PCMS = 0, the LSB of SA must be "0."
	PCMS ="2" or "3", LSB two bits of SA must be "00".

	LSA[15:0]
	This register specifies the loop starting address for the sound data in terms of the number of samples from SA.
	The number of samples indicates the number of bytes in 8-bit PCM, the number of pairs of bytes (16 bits) in the case of 16-bit PCM, and the number of half-bytes in the case of ADPCM.  The minimum values that can be set are limited by the pitch and the loop mode.  Because the actual value is not approximated at values near SA due to the specifications for ADPCM, as large a value as possible must be used for LSA (LSA > 0x8).  (When in a loop)  When using long stream, the lowest two bits of LSA must be "00".

	LEA[15:0]
	This register specifies the loop ending address for the sound data in terms of the number of samples from SA. 
	The minimum value that can be set is limited by the pitch and the loop mode. 
	Specify so that SA≦LSA≦LEA.  When using long stream, the lowest two bits of LEA must be "00".
	Refer to section 8.1.1.1, "Loop Control."
	AR[4:0]
	This register specifies the rate of change in the EG in the attack state.  (The volume increases.)
	D1R[4:0]
	This register specifies the rate of change in the EG in the decay 1 state.  (The volume decreases.)
	D2R[4:0]
	This register specifies the rate of change in the EG in the decay 2 state.  (The volume decreases.)
	RR[4:0]
	This register specifies the rate of change in the EG in the release state.  (The volume decreases.)
	DL[4:0]
	This register specifies the EG level at which the transition is made from decay 1 to decay 2, making the specification through the upper 5 bits of the EG code.
	KRS[3:0]
	This register specifies the EG key rate scaling rate (as a positive number).
	0x0: 	Minimum scaling
	:
	0xE: 	Maximum scaling
	0xF: 	Scaling off
	LPSLNK
	Loop start link function: when the sound slot input data address that is read exceeds the loop start address, the EG makes the transition to decay 1.
	(When EG = 000, the transition is not made.)  In this case, the transition to decay 2 may not be made, depending on the DL setting.
	(Refer to section 8.1.1.3, "AEG.")
	OCT[3:0]
	This register specifies the octave in two's complement format.  The values that appear in parentheses in the table below could generate noise in the ADPCM, so they should be used with caution.  (A maximum of "2" (when FNS = 0) is valid.)

	OCT	0x8	0x9	0xA	0xB	0xC	0xD	0xE	0xF	0x0	0x1	0x2	0x3	0x4	0x5	0x6	0x7
	Interval	-8	-7	-6	-5	-4	-3	-2	-1	0	+1	(+2)	(+3)	(+4)	(+5)	(+6)	(+7)
	Table 8-22 Octave Specification
	FNS[9:0]
	The pitch is set along with OCT by setting the F number.
	Pitch: P[CENT] = 1200 ´ log2((2^10 + FNS)/2^10)
	When FNS = 0 (and OCT = 0), the interval matches the sampling source.  The pitch error (pitch precision) that is equivalent to the LSB of the FNS is 1.69.
	(Refer to section 8.1.1.4 “PG.")
	LFORE
	This register specifies whether or not to put the LFO into the initial state.  (If noise was selected, the setting is invalid.)
	0: Do not put the LFO in the reset state.
	1: Put the LFO in the reset state.
	LFOF[4:0]
	This register specifies the LFO oscillating frequency.  (If noise was selected, the setting is invalid.)
	ALFOWS[1:0]
	This register specifies the shape of the ALFO waveform.
	PLFOWS[1:0]
	This register specifies the shape of the PLFO waveform.
	ALFOS[2:0]
	This register specifies the degree of mixing of the LFO to the EG.
	PLFOS[2:0]
	This register specifies the degree of the LFO on the pitch.
	(Refer to section 8.1.1.5, "LFO.")
	ISEL[3:0]
	This register specifies the MIXS register address for each slot when inputting sound slot output data to the DSP's MIXS register.
	(Supplement)	MIXS determines the sum of the inputs for all slots and handles the result as the DSP input.  MIXS has an area for adding the input on each slot, and an area for storing the interval and value of one sample.  These areas are allocated in alternation.  As a result, reads on the DSP side are possible at any step.
	(Caution)	Make the settings so that the sum of the inputs to the MIXS does not exceed 0dB.  (There is no overflow protect function.)
	TL[7:0]
	Total level: This register specifies the actual attenuation, which is derived by multiplying the EG value by this value which indicates the attenuation.
	DIPAN[4:0]
	This register specifies the orientation for each slot when sending direct data.
	EFPAN[4:0]
	This register specifies the orientation for each slot of effect data and external input data.
	IMXL[3:0]
	This register specifies the level for each slot when inputting sound slot output data to the DSP MIXS register.  (Refer to Table 8-23 below.)
	DISDL[3:0]
	This register specifies the send level for each slot when outputting direct data to the DAC.  (Refer to the table below.)

	//NOT per generation channel , just per dsp channel ! =P
	EFSDL[3:0]
	This register specifies the send level for each slot when outputting of effect data and external input data to the DAC.
	Register value	Volume
	0	-MAXdB
	1	-42dB
	2	-39dB
	：	：
	0xD	-6dB
	0xE	-3dB
	0xF	0dB
	Table 8-23  Send Level
	(Refer to section 8.1.1.6, "MIXER.")
	Q[4:0]
	This register contains resonance data, and sets the Q value for the FEG filter.  A gain range from -3.00 to 20.25dB can be specified.  The relationship between the bit settings and the gain is illustrated in the following table.  (Q[dB] = 0.75 ´ register value - 3)
	DATA	GAIN[dB]		DATA	GAIN[dB]
	11111	20.25		00110	1.50
	11100	18.00		00100	0.00
	11000	15.00		00011	-0.75
	10000	9.00		00010	-1.50
	01100	6.00		00001	-2.25
	01000	3.00		00000	-3.00
	Table 8-24  Resonance Data Setting Values
	The definition of Q is illustrated in the following graph.


	Fig. 8-13	Definition of Q
	FLV0[12:0]
	This is the cutoff frequency at attack start.
	FLV1[12:0]
	This is the cutoff frequency at attack end (decay start).
	FLV2[12:0]
	This is the cutoff frequency at decay end (sustain start).
	FLV3[12:0]
	This is the cutoff frequency at KOFF.
	FLV4[12:0]
	This is the cutoff frequency after release.
	FAR[4:0]
	However, only values ranging from 0x0008 to 0x1FF8 can be used for FLV0 through 4.  Playback may not be possible if any other values are used.

	The following graph summarizes the function of each register.


	Fig. 8-14	Function of Each Register


	The following graph roughly shows the correspondence between the filter cutoff frequency and the registers.

	Fig. 8-15	Filter Cutoff Frequency
	* To set the filter to pass signals through, set Q to 4h and FLV to 0x1FF8.
	FAR[4:0]
	This register specifies the rate of change in the FEG in the attack state.
	FD1R[4:0]
	This register specifies the rate of change in the FEG in the decay 1 state.
	FD2R[4:0]
	This register specifies the rate of change in the FEG in the decay 2 state.
	FRR[4:0]
	This register specifies the rate of change in the FEG in the release state.

	*/
};

AicaChannel AicaChannel::Chans[64];

#define Chans AicaChannel::Chans 
double dbToval(double db)
{
	return pow(10,db/20.0);
}
u32 CalcAegSteps(float t)
{
	const double aeg_allsteps=1024*(1<<AEG_STEP_BITS)-1;

	if (t<0)
		return 0;
	if (t==0)
		return (u32)aeg_allsteps;

	//44.1*ms = samples
	double scnt=44.1*t;
	double steps=aeg_allsteps/scnt;
	return (u32)(steps+0.5);
}
void sgc_Init()
{
	for (u32 i=0;i<16;i++)
	{
		volume_lut[i]=(1<<15)/pow(2,(15-i)/2.0);
		if (i==0)
			volume_lut[i]=0;
	}
	for (u32 i=0;i<256;i++)
	{
		tl_lut[i]=(1<<15)/pow(2,i/16.0);
	}

	for (int i=0;i<64;i++)
	{
		AEG_ATT_SPS[i]=CalcAegSteps(AEG_Attack_Time[i]);
		AEG_DSR_SPS[i]=CalcAegSteps(AEG_DSR_Time[i]);
	}
	for (int i=0;i<64;i++)
		Chans[i].Init(i,aica_reg);
	dsp_out_vol=(DSP_OUT_VOL_REG*)&aica_reg[0x2000];

	dsp_init();
}

void sgc_Term()
{
	
}

void WriteChannelReg8(u32 channel,u32 reg)
{
	Chans[channel].RegWrite(reg);
}

void ReadCommonReg(u32 reg,bool byte)
{
	switch(reg)
	{
	case 0x2808:
	case 0x2809:
		CommonData->MIEMP=1;
		CommonData->MOEMP=1;
		break;
	case 0x2810: //LP & misc
	case 0x2811: //LP & misc
		{
			u32 chan=CommonData->MSLC;
			
			CommonData->LP=Chans[chan].looped;
			verify(CommonData->AFSET==0);
		
			CommonData->EG=Chans[chan].AEG.GetValue();	//AEG is only 10 bits, FEG is 13 bits
			CommonData->SGC=Chans[chan].AEG.state;

			if (! (byte && reg==0x2810))
				Chans[chan].looped=0;
		}
		break;
	case 0x2814:	//CA
	case 0x2815:	//CA
		{
			u32 chan=CommonData->MSLC;
			CommonData->CA = Chans[chan].CA /*& (~1023)*/; //mmnn??
			//printf("[%d] CA read %d\n",chan,Chans[chan].CA);
		}
		break;
	}
}

void WriteCommonReg8(u32 reg,u32 data)
{
	WriteMemArr(aica_reg,reg,data,1);
	if (reg==0x2804 || reg==0x2805)
	{
		dsp.RBL=(8192<<CommonData->RBL)-1;
		dsp.RBP=( CommonData->RBP*2048&AICA_RAM_MASK);
		dsp.dyndirty=true;
	}
}

#define CDDA_SIZE  (2352/2)
s16 cdda_sector[CDDA_SIZE]={0};
u32 cdda_index=CDDA_SIZE<<1;
void AICA_Sample()
{
	mixl = 0;
	mixr = 0;
	memset(dsp.MIXS,0,sizeof(dsp.MIXS));

	for (int i=0;i<64;i++)
	{
		Chans[i].Generate();
	}

	//OK , generated all Chanels  , now DSP/ect + final mix ;p
	//CDDA EXTS input
	
	if (cdda_index>=CDDA_SIZE)
	{
		cdda_index=0;
		aica_params.CDDA_Sector(cdda_sector);
	}
	cdda_index+=2;
	//No dsp tho ;p

	//Final MIX ..
	//Add CDDA / DSP effect(s)

	//CDDA
	if (settings.CDDAMute==0) 
	{
		s32 EXTS0L=cdda_sector[cdda_index];
		s32 EXTS0R=cdda_sector[cdda_index+1];

		VOLPAN(EXTS0L,dsp_out_vol[16].EFSDL,dsp_out_vol[16].EFPAN,mixl,mixr);
		VOLPAN(EXTS0R,dsp_out_vol[17].EFSDL,dsp_out_vol[17].EFPAN,mixl,mixr);
	}
	
	dsp_step();

	for (int i=0;i<16;i++)
	{
		VOLPAN( (*(s16*)&DSPData->EFREG[i]) ,dsp_out_vol[i].EFSDL,dsp_out_vol[i].EFPAN,mixl,mixr);
	}

	//Mono !
	if (CommonData->Mono)
	{
		//Yay for mono =P
		mixl+=mixr;
		mixr=mixl;
	}
	
	//MVOL !
	//we want to make sure mix* is *At least* 23 bits wide here, so 64 bit mul !
	u32 mvol=CommonData->MVOL;
	s32 val=volume_lut[mvol];
	mixl=(s32)FPMul((s64)mixl,val,15);					
	mixr=(s32)FPMul((s64)mixr,val,15);					


	if (CommonData->DAC18B)
	{
		//If 18 bit output , make it 16b :p
		mixl=FPs(mixl,2);
		mixr=FPs(mixr,2);
	}

	//Sample is ready ! clip/saturate and store :}
	if (((s16)mixl) != mixl)
		printf("Cliped mixl %d\n",mixl);
	if (((s16)mixr) != mixr)
		printf("Cliped mixr %d\n",mixr);

	clip16(mixl);
	clip16(mixr);

	pl=mixl;
	pr=mixr;

	WriteSample(mixr,mixl);
}
//Decoder : decode a sample, FORWARD direction only, may need substeping
struct Decoder
{
	const int substeps;	//1 if steping is needed
	const int bps;		//bits per sample
	//Data is 
	SampleType Decode(u32 data);

	void OnStart();//Called when the buffer has warped, right before the first Decode is done
};
//Channel : handles looping, calls the Decoder where appropriate
/*
struct AicaChannelEx
{
	fp_22_10 samples_to_decode;
	_EG AEG;
	fp_22_10 CA;
	
	void Decode()
	{
		if (samples_to_decode.ip==0)
			return;
		//decode samples
		samples_to_decode.ip=0;
	}

	//range : s16 
	//When AEG state is release & vol =0 then no mem reads are done.WTF happens if adpcm ? counters still count? what about looping?
	SampleType GetSample()
	{
		if (AEG.state == EG_Release && AEG.value.ip=0x3FF)
		{
		}
		
	}

};
*/