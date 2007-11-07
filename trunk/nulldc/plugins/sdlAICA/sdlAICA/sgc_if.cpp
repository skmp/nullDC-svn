#include "sgc_if.h"
#include "mem.h"
#include <math.h>

//Sound generation , mixin , and chanel regs emulation

#define SAMPLE_CYCLES ((float)DCclock/(float)44100)

float rem_sh4_cycles=0;
SampleType mixl;
SampleType mixr;

//x.15
s32 volume_lut[16];
s32 tl_lut[256];	//xx.15 format :)
const char* stream_names[]=
{
	"0: 16-bit PCM (two's complement format)",
	"1: 8-bit PCM (two's complement format)",
	"2: 4-bit ADPCM (Yamaha format)",
	"3: 4-bit ADPCM long stream"
};

//x.8 format
const s32 adpcm_qs[16] = 
{
	0x0e6, 0x0e6, 0x0e6, 0x0e6, 0x133, 0x199, 0x200, 0x266,
	0x0e6, 0x0e6, 0x0e6, 0x0e6, 0x133, 0x199, 0x200, 0x266
};
//x.3 format
const s32 adpcm_scale[16] = 
{
	1,3,5,7,9,11,13,15,
	-1,-3,-5,-7,-9,-11,-13,-15,
};

void AICA_Sample();

//Remove the fractional part , with rounding ;) -- does not need an extra bit
#define FPRound(a,bits) (((a) + ((1<<(bits-1))))>>bits)
//Remove the franctional part by chopping..
#define FPChop(a,bits) ((a)>>bits)

//Fixed point mul w/ rounding :)
#define FPMul(a,b,bits) (FPRound(a*b,bits))

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
u32 AICA_GenerateSamples(u32 sh4_cycles)
{
	rem_sh4_cycles+=sh4_cycles;

	u32 sc=0;
	while(rem_sh4_cycles>SAMPLE_CYCLES)
	{
		rem_sh4_cycles-=SAMPLE_CYCLES;
		//generate 1 sample
		mixl = 0;
		mixr = 0;
		AICA_Sample();

		if (((s16)mixl) != mixl)
			printf("Cliped mixl %d\n",mixl);
		if (((s16)mixr) != mixr)
			printf("Cliped mixr %d\n",mixr);

		clip16(mixl);
		clip16(mixr);

		pl=mixl;
		pr=mixr;

		WriteSample(mixr,mixl);
		sc++;
	}

	return sc;
}

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
struct _EG
{
	fp_s_22_10 value;
	_EG_state state;
};
enum _KEY_STATE
{
	KEY_OFF,
	KEY_ON
};
u32 fc=64;
class AicaChannel
{
public:
	ChannelCommonData* ChanData;
	u32* ChanData_raw;
	u32 Channel;

	_KEY_STATE key_state;
	fp_22_10 CA;
	u32 update_rate;	//22.10

	_EG AEG; 
	_EG FEG;	//how can this be emulated anyway ?

	//Sampling
	u32 last_sp;
	

	//used on adpcm decoding
	s32 last_quant;//is that realy 32.0 ? maby 24.8 ?

	//used on apdcm decoding & interpolation
	SampleType prev_sample;
	SampleType curr_sample;

	bool sound_enabled;
	u32 looped;
	void Init(u32 channel,u8* reg_base)
	{
		//Enabled=false;
		AEG.value.ip=0x3FF;
		AEG.value.fp=0;
		AEG.state=EG_Release;

		Channel=channel;
		ChanData=(ChannelCommonData*)&reg_base[channel*0x80];
		ChanData_raw=(u32*)ChanData;
		CalcUpdateRate();
		looped=0;
	}


	//KYONEX(-/w)
	//Writing "1" to this register executes KEY_ON, OFF for all slots.  Writing "0" is invalid.
	//Called on Write w/ value 1
	void KONEX()
	{
		if (key_state == KEY_ON)
		{
			//If enabled & KYONB=0
			if (ChanData->KYONB==0)
			{
				fc++;
				Disable();
				key_state=KEY_OFF;
			}
		}
		else
		{
			//if not enabled & KYONB=1 ->KEY_ON
			if (ChanData->KYONB)
			{
				fc--;
				Enable();
				key_state=KEY_ON;
			}
		}
	}
	void CalcUpdateRate()
	{
		//sample'=sample<<(oct+10) + sample*fns
		u32 oct=ChanData->OCT;
		/*if (oct&8)
			oct|=0xFFFFFFF8;
		if (ChanData->PCMS>2)
		{
			if ((s32)oct>1)
				oct=1;
		}
		oct+=10;
		
		update_rate=1<<(oct);
		update_rate+=(ChanData->FNS*update_rate)>>10;
		*/

		//same code as above realy , just faster =P
		update_rate =1024 + ChanData->FNS;
		if (oct& 8)
		{
			update_rate>>=(16-oct);
		}
		else
		{
			update_rate<<=oct;
		}
	}


	void Enable()
	{
		//called on key on
		//Enabled=true;
		AEG.state=EG_Attack;// reset AEG
		AEG.value.ip=0x3FF;//start from 0x3FF ? .. it seems so !
		AEG.value.fp=0;

		//Reset samplig state
		CA.full=0;
		looped=0;
		last_sp=0xFFFFFFFF;

		last_quant=127;
		prev_sample=0;
		curr_sample=0;
		
		CalcUpdateRate();
		sound_enabled=true;//mnn maby it is some internal bit or smth?
		//printf("%d chanel enabled ! [%d] , %s stream type @ %f hrz %d oct,%d fns\n",Channel,fc,stream_names[ChanData->PCMS],(44100.0*update_rate)/1024,ChanData->OCT,ChanData->FNS);
		//Decode first sample
		Decoder_Step();
	}

	//Disable is olny executed if KEY_OFF , otherwise its not =P
	void Disable()
	{
		//called on key off
		AEG.state=EG_Release; //AEG switches to release state !
		//printf("%d chanel disabled ! [%d]\n",Channel,fc);
		//ChanData->KYONB=0; -- KYONB data is not touched
		//CA.full=0;  //what about these ? -> not updated
		//looped=0;	//wiiii ? -> not updated
		//Enabled=false;
	}

	//must be done after GetSample ;)
	void Sample_step()
	{
		fp_22_10 na;
		na.full=CA.full+update_rate;

		u32 cur_addr;

		if (ChanData->PCMS==3)
		{
			cur_addr=na.ip & ~3; //adpcm "stream" mode -- why realy ?
		}
		else
		{
			cur_addr=na.ip;
		}

		if (ChanData->LPSLNK)
		{
			if ((AEG.state==EG_Attack) && (cur_addr>=ChanData->LSA))
			{
				printf("[%d]LPSLNK : Switching to EG_Decay1 %X\n",Channel,AEG.value.ip);
				AEG.state=EG_Decay1;
			}
		}

		
		if (cur_addr>=ChanData->LEA)
		{
			int dec_steps=ChanData->LEA-CA.ip-1;
			while(dec_steps-->0)
			{
				CA.ip++;
				Decoder_Step();
			}

			if (ChanData->LPCTL)
			{
				looped=1;
				CA.ip=ChanData->LSA;
				//CA.fp=0; //mnn that doesnt realy make sense so i have comented it out -)
				
				if (ChanData->PCMS==2) //if in adpcm non-stream mode, reset the decoder
				{	
					last_quant=127;
					prev_sample=0;
				}
				if (ChanData->LEA>ChanData->LSA)
				{
					dec_steps=na.ip-ChanData->LEA;
					//decode the first sample, allways has to be done...
					Decoder_Step();
					while(dec_steps-->0)
					{
						CA.ip++;
						Decoder_Step();
					}
				}
				//verify(CA.ip==na.ip); -> invalid, CA.ip is set to LSA
				CA.fp=na.fp;	//we still need to copy
			}
			else
			{
				//if (sound_enabled)///?????????
				{
					//looped=1;
					//Disable(); -> OLNY key_off seems to do that =P
					//				verify(AEG.state!=EG_Attack);//whee?
					AEG.value.ip=0x3FF;			 //mute chanel [end of loop]
					//hopefully thats what real h/w does
					AEG.value.fp=0;
					//AEG.state=EG_Release;
					sound_enabled=false; //?
					//ChanData->KYONB=0;
					//key_state=KEY_OFF;
					CA.ip=ChanData->LSA;
					key_state=KEY_OFF;
					ChanData->KYONB=0;
				}
			}
		}
		else
		{
			int dec_steps=na.ip-CA.ip;
			while(dec_steps-->0)
			{
				CA.ip++;
				Decoder_Step();
			}
			verify(CA.ip==na.ip);
			CA.full=na.full;
		}
	}
	void Decoder_Step()
	{
		verify(last_sp!=CA.ip);
		//save the old sample
		prev_sample=curr_sample;
		//decode the new one :)

		last_sp=CA.ip;
		if (ChanData->PCMS==0)
		{
			u32 addr=(ChanData->SA_hi<<16) | (ChanData->SA_low);
			s16* ptr=(s16*)&aica_ram[addr+CA.ip*2];
			curr_sample=*ptr;
			if(curr_sample==-32768 && !(prev_sample&0x8000))
			{
				//printf("//wtf is this ? why is it here ?"); // -> it actualy fixes SA2 music . why ?
				curr_sample=0x7fff;
			}
		}
		else if (ChanData->PCMS==1)
		{
			u32 addr=(ChanData->SA_hi<<16) | (ChanData->SA_low);
			s8* ptr=(s8*)&aica_ram[addr+CA.ip];
			curr_sample=*ptr<<8;
		}
		else
		{
			u32 addr=(ChanData->SA_hi<<16) | (ChanData->SA_low);
			u8* ptr=(u8*)&aica_ram[addr+(CA.ip>>1)];

			u32 sample=*ptr;

			if (CA.ip&1)
				sample>>=4; //2nd sample is HI nible ;)
			else
				sample&=0xF;//first sample is LOW nible !



			/*(1 - 2 * L4) * (L3 + L2/2 +L1/4 + 1/8) * quantized width (ƒΆn) + decode value (Xn - 1) */
			curr_sample = prev_sample + ((last_quant*adpcm_scale[sample])>>3);

			last_quant = (last_quant * adpcm_qs[sample])>>8;

			clip(last_quant,127,24576);
			clip16(curr_sample);
		}

	}
	SampleType GetSample()
	{
		//Gets a sample , and updates needed regs ! :)
		//verify(Enabled);
		//if (AEG.value.ip==0x3FF)
		//	return 0; //chanel will not produce *any* output :)

		SampleType rv;

		/*
		if (last_sample!=next_sample)
		{*/
		rv=FPMul(prev_sample,(s32)(1023-CA.fp),10);
		rv+=FPMul(curr_sample,(s32)(CA.fp),10);
		/*
		if (rv==-32769)
			rv=32768;
		else if (rv==32768)
			rv==32767;
		*/
		/*}
		else*/
		//rv=curr_sample;

		//make sure its still in range
		verify(((s16)rv)==rv);
		return rv;
	}
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

	//Step the AEG ;p
	void AEG_step()
	{
		return;//no aeg =P
		switch(AEG.state)
		{
		case EG_Attack:
			{
				//wii
				AEG.value.full-=200<<10;
				AEG.value.full=0;
				if (AEG.value.full<=0)
				{
					AEG.value.full=0;
					if (!ChanData->LPSLNK)
					{
						printf("[%d]AEG_step : Switching to EG_Decay1 %d\n",Channel,AEG.value);
						AEG.state=EG_Decay1;
					}
				}
			}
			break;
		case EG_Decay1:
			{
				//x2
				AEG.value.full+=10;
				if (((u32)AEG.value.ip>>5)>=ChanData->DL)
				{
					printf("[%d]AEG_step : Switching to EG_Decay2 @ %x\n",Channel,AEG.value.ip);
					AEG.state=EG_Decay2;
				}
			}
			break;
		case EG_Decay2:
			{
				//x3
				AEG.value.full+=10;
				if (AEG.value.ip>0x3FF)
				{
					AEG.value.ip=0x3FF; //mnn , should we do anything about it running wild ?
					AEG.value.fp=0;
					AEG.state=EG_Release;
					printf("[%d]AEG_step : Switching to EG_Release @ %x\n",Channel,AEG.value.ip);
				}
			}
			break;
		case EG_Release: //olny on key_off ?
			{
				AEG.value.full+=1000;
				
				if (AEG.value.ip>0x3FF)
				{
					AEG.value.ip=0x3FF; //mnn , should we do anything about it running wild ?
					sound_enabled=false;
				}
			}
			break;
		}
	}
	void Generate()
	{
		if (/*sound_enabled==false &&*/ key_state==KEY_OFF)
			return;

		AEG_step(); //here ? or after sample ? duno...
					//lets say it is here for now .. otherwise the first sample whould allways be muted o.O
		
		//If first sample, decode it :)
		if (last_sp!=CA.ip)
			Decoder_Step();

		SampleType sample = GetSample();

		//need to include AEG value in the future here ..
		sample=FPMul(sample,tl_lut[ChanData->TL],15);
		
		//pan~~
		VOLPAN(sample,ChanData->DISDL,ChanData->DIPAN,mixl,mixr);

		//GOOD ! now we finished w/ sound generation ...
		//Loop procecing / misc stuff(tm)
		Sample_step();
	}
	void RegWrite(u32 offset); //see , this had to moved out of this ohh-so-nice class b/c of C++ lameness .. cant beat it xD
};
AicaChannel Chans[64];
void AicaChannel::RegWrite(u32 offset)
{
	switch(offset)
	{
	case 0x01:	//yay ?
		if (ChanData->KYONEX)
		{
			ChanData->KYONEX=0;
			for (int i=0;i<64;i++)
			{
				Chans[i].KONEX();
			}
		}
		break;

		//mhhrpppfff
	case 0x18:
	case 0x19:
		CalcUpdateRate();
		break;
	}
}
double dbToval(double db)
{
	return pow(10,db/20.0);
}
void sgc_Init()
{
	for (u32 i=0;i<16;i++)
	{
		//volume_lut[i]=(pow(10,-3.0*(0xF-(double)i)/20.0))*pow(2.0,12.0);
		volume_lut[i]=(s32)(dbToval(-3.0*(0xF-(double)i))*(1<<15));
		//volume_lut[i]=i;
		if (i==0)
			volume_lut[i]=0;
	}
	for (u32 i=0;i<256;i++)
	{
		double db=0;
		
		if (i & (1<<0))
		{
			db-=0.4;
		}
		if (i & (1<<1))
		{
			db-=0.8;
		}
		if (i & (1<<2))
		{
			db-=1.5;
		}
		if (i & (1<<3))
		{
			db-=3;
		}
		if (i & (1<<4))
		{
			db-=6;
		}
		if (i & (1<<5))
		{
			db-=12;
		}
		if (i & (1<<6))
		{
			db-=24;
		}
		if (i & (1<<7))
		{
			db-=48;
		}
		
		tl_lut[i]=(s32)(dbToval(db)*(1<<15));
	}

	for (int i=0;i<64;i++)
		Chans[i].Init(i,aica_reg);
	dsp_out_vol=(DSP_OUT_VOL_REG*)&aica_reg[0x2000];
}

void sgc_Term()
{
	//for (int i=0;i<64;i++)
	//	delete Chans[i];
}
void WriteChannelReg8(u32 channel,u32 reg)
{
	Chans[channel].RegWrite(reg);
}


void ReadCommonReg8(u32 reg)
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
			CommonData->EG=Chans[chan].AEG.value.ip;
			CommonData->SGC=Chans[chan].AEG.state;

			Chans[chan].looped=0;
		}
		break;
	case 0x2814:	//CA
	case 0x2815:	//CA
		{
			u32 chan=CommonData->MSLC;
			CommonData->CA = Chans[chan].CA.ip /*& (~1023)*/; //mmnn??
		}
		break;
	}
}

void WriteCommonReg8(u32 reg,u32 data)
{
	WriteMemArrRet(aica_reg,reg,data,1);
}

#define CDDA_SIZE  (2352/2)
s16 cdda_sector[CDDA_SIZE]={0};
u32 cdda_index=CDDA_SIZE<<1;
void AICA_Sample()
{
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
	//DSP is missing ...
	
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


	if (!CommonData->DAC18B)
	{
		//If 18 bit output , make it 16b :p
		mixl=FPRound(mixl,2);
		mixr=FPRound(mixr,2);
	}
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