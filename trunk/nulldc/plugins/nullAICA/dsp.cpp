#include "dsp.h"
#include "mem.h"
#include "emitter\x86_emitter.h"

const bool SUPPORT_NOFL=false;

__declspec(align(4096)) dsp_t dsp;
#define assert verify

#pragma warning(disable:4311)
#define DYNOPT 0
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

	unsigned int EWT;
	unsigned int EWA;
	unsigned int ADRL;
	unsigned int FRCL;
	unsigned int SHIFT;
	unsigned int YRL;
	unsigned int NEGB;
	unsigned int ZERO;
	unsigned int BSEL;

	unsigned int NOFL;	//MRQ set
	unsigned int TABLE;	//MRQ set
	unsigned int MWT;	//MRQ set
	unsigned int MRD;	//MRQ set	
	unsigned int MASA;	//MRQ set
	unsigned int ADREB;	//MRQ set
	unsigned int NXADR;	//MRQ set
};


#define DYNBUF	0x10000

//#define USEFLOATPACK

static UINT16 __fastcall PACK(INT32 val)
{
	UINT32 temp;
	int sign,exponent,k;

	sign = (val >> 23) & 0x1;
	temp = (val ^ (val << 1)) & 0xFFFFFF;
	exponent = 0;
	for (k=0; k<12; k++)
	{
		if (temp & 0x800000)
			break;
		temp <<= 1;
		exponent += 1;
	}
	if (exponent < 12)
		val = (val << exponent) & 0x3FFFFF;
	else
		val <<= 11;
	val >>= 11;
	val |= sign << 15;
	val |= exponent << 11;

	return (UINT16)val;
}

static INT32 __fastcall UNPACK(UINT16 val)
{
	int sign,exponent,mantissa;
	INT32 uval;

	sign = (val >> 15) & 0x1;
	exponent = (val >> 11) & 0xF;
	mantissa = val & 0x7FF;
	uval = mantissa << 11;
	if (exponent > 11)
		exponent = 11;
	else
		uval |= (sign ^ 1) << 22;
	uval |= sign << 23;
	uval <<= 8;
	uval >>= 8;
	uval >>= exponent;

	return uval;
}


void dsp_init()
{
	memset(&dsp,0,sizeof(dsp));
	memset(DSPData,0,sizeof(*DSPData));

	dsp.dyndirty=true;
	dsp.RBL=0x2000-1;
	dsp.RBP=0;
	dsp.regs.MDEC_CT=1;
}
void dsp_recompile();
void DecodeInst(u32 *IPtr,_INST *i)
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


void* dyna_realloc(void*ptr,u32 oldsize,u32 newsize)
{
	return dsp.DynCode;
}
void _dsp_debug_step_start()
{
	memset(&dsp.regs_init,0,sizeof(dsp.regs_init));
}
void _dsp_debug_step_end()
{
	verify(dsp.regs_init.MAD_OUT);
	verify(dsp.regs_init.MEM_ADDR);
	verify(dsp.regs_init.MEM_RD_DATA);
	verify(dsp.regs_init.MEM_WT_DATA);
	verify(dsp.regs_init.FRC_REG);
	verify(dsp.regs_init.ADRS_REG);
	verify(dsp.regs_init.Y_REG);

	//verify(dsp.regs_init.MDEC_CT); // -> its done on C
	verify(dsp.regs_init.MWT_1);
	verify(dsp.regs_init.MRD_1);
//	verify(dsp.regs_init.MADRS); //THAT WAS not real, MEM_ADDR is the deal ;p
	verify(dsp.regs_init.MEMS);
	verify(dsp.regs_init.NOFL_1);
	verify(dsp.regs_init.NOFL_2);
	verify(dsp.regs_init.TEMPS);
	verify(dsp.regs_init.EFREG);
}
#define nwtn(x) verify(!dsp.regs_init.##x)
#define wtn(x) nwtn(x);dsp.regs_init.##x=true;

//sign extend to 32 bits
void dsp_rec_se(x86_block& x86e,x86_gpr_reg reg,u32 src_sz,u32 dst_sz=0xFF)
{
	if (dst_sz==0xFF)
		dst_sz=src_sz;
	//24 -> 32 (pad to 32 bits)
	x86e.Emit(op_shl32,reg,32-src_sz);
	//32 -> 24 (MSB propagation)
	x86e.Emit(op_sar32,reg,32-dst_sz);
}
//Reads : MWT_1,MRD_1,MEM_ADDR
//Writes : Wire MEM_RD_DATA_NV
void dsp_rec_DRAM_CI(x86_block& x86e,_INST& prev_op,u32 step,x86_gpr_reg MEM_RD_DATA_NV)
{
	nwtn(MWT_1);
	nwtn(MRD_1);
	nwtn(MEM_ADDR);
	nwtn(MEM_WT_DATA);

	//Request : step x (odd step)
	//Operation : x+1   (even step)
	//Data avail : x+2   (odd step, can request again)
	if (!(step&1))	
	{
		//Get and mask ram address :)
		x86e.Emit(op_mov32,EAX,&dsp.regs.MEM_ADDR);
		x86e.Emit(op_and32,EAX,AICA_RAM_MASK);

		x86e.Emit(op_add32,EAX,(unat)aica_ram);

		//prev. opcode did a mem read request ?
		if (prev_op.MRD)
		{
			//Do the read [MEM_ADDRS] -> MEM_RD_DATA_NV
			x86e.Emit(op_movsx16to32,MEM_RD_DATA_NV,x86_mrm::create(EAX));
		}
		//prev. opcode did a mem write request ?
		if (prev_op.MWT)
		{
			//Do the write [MEM_ADDRS] <-MEM_WT_DATA
			x86e.Emit(op_mov32,EDX,&dsp.regs.MEM_WT_DATA);
			x86e.Emit(op_mov16,x86_mrm::create(EAX),EDX);
		}
	}
}
//Reads : ADRS_REG,MADRS,MDEC_CT
//Writes : MEM_ADDR
void dsp_rec_MEM_AGU(x86_block& x86e,_INST& op,u32 step)
{
	nwtn(ADRS_REG);
	nwtn(MEM_ADDR);
	
	//These opcode fields are valid on odd steps (mem req. is only allowed then)
	//MEM Request : step x
	//Mem operation : step x+1 (address is avaiable at this point)
	if (step&1)
	{
		//Addrs is 16:1
		x86e.Emit(op_mov32,EAX,&DSPData->MADRS[op.MASA]);

		//Added if ADREB
		if (op.ADREB)
			x86e.Emit(op_add32,EAX,&dsp.regs.ADRS_REG);
		
		//+1 if NXADR is set
		if (op.NXADR)
			x86e.Emit(op_add32,EAX,1);

		//RBL warp around is here, acording to docs, but that seems to cause _very_ bad results
	//	if (!op.TABLE)
	//		x86e.Emit(op_and32,EAX,dsp.RBL);

		//MDEC_CT is added if !TABLE
		if (!op.TABLE)
			x86e.Emit(op_add32,EAX,&dsp.regs.MDEC_CT);

		//RBL/RBP are constants for the pogram
		//Apply RBL if !TABLE
		//Else limit to 16 bit add
		//*update* allways limit to 16 bit add adter MDEC_CT ?
		if (!op.TABLE)
			x86e.Emit(op_and32,EAX,dsp.RBL);
		else
			x86e.Emit(op_and32,EAX,0xFFFF);

		//Calculate the value !
		//EAX*2 b/c it points to sample (16:1 of the address)
		x86e.Emit(op_lea32,EDX,x86_mrm::create(EAX,sib_scale_2,x86_ptr::create(dsp.RBP)));

		//Save the result to MEM_ADDR
		x86e.Emit(op_mov32,&dsp.regs.MEM_ADDR,EDX);
	}
	wtn(MEM_ADDR);
}
//Reads : MEMS,MIXS,EXTS
//Writes : INPUTS (Wire)
void dsp_rec_INPUTS(x86_block& x86e,_INST& op,x86_gpr_reg INPUTS)
{
	nwtn(MEMS);

	//nwtn(MIXS); -> these are read only :)
	//nwtn(EXTS);

	//INPUTS is 24 bit, we convert everything to that
	//Maby we dont need to convert, but just to sign extend ?
	if(op.IRA<0x20)
	{
		x86e.Emit(op_mov32,INPUTS,&dsp.MEMS[op.IRA]);
		dsp_rec_se(x86e,INPUTS,24);
	}
	else if(op.IRA<0x30)
	{
		x86e.Emit(op_mov32,INPUTS,&dsp.MIXS[op.IRA-0x20]);
		dsp_rec_se(x86e,INPUTS,20,24);
	}
	else if(op.IRA<0x32)
	{
		x86e.Emit(op_mov32,ESI,&DSPData->EXTS[op.IRA-0x30]);
		//x86e.Emit(op_shl32,INPUTS,8);
		dsp_rec_se(x86e,INPUTS,16,24);
	}

	//Sign extend to 32 bits
	//dsp_rec_se(x86e,INPUTS,24);
}
//Reads : MEM_RD_DATA,NO_FLT2
//Writes : MEMS
void dsp_rec_MEMS_WRITE(x86_block& x86e,_INST& op,u32 step,x86_gpr_reg INPUTS)
{
	nwtn(MEM_RD_DATA);
	nwtn(NOFL_2);

	//MEMS write reads from MEM_RD_DATA register (MEM_RD_DATA -> Converter -> MEMS).
	//The converter's nofl flag has 2 steps delay (so that it can be set with the MRQ).
	if (op.IWT)
	{
		x86e.Emit(op_movsx16to32,ECX,&dsp.regs.MEM_RD_DATA);
		x86e.Emit(op_mov32,EAX,ECX);

		//Pad and signed extend EAX
		//x86e.Emit(op_shl32,EAX,16);
		//x86e.Emit(op_sar32,EAX,8);
		x86e.Emit(op_shl32,EAX,8);

		if (SUPPORT_NOFL)
		{
			x86_Label* no_fl=x86e.CreateLabel(false,8);//no float convertions

			//Do we have to convert ?
			x86e.Emit(op_cmp32,&dsp.regs.NOFL_2,1);
			x86e.Emit(op_je,no_fl);
			{
				//Convert !
				x86e.Emit(op_call,x86_ptr_imm(UNPACK));
			}
			x86e.MarkLabel(no_fl);
		}
		x86e.Emit(op_mov32,&dsp.MEMS[op.IWA],EAX);
	}
	
	wtn(MEMS);
}
//Reads : MEM_RD_DATA_NV (Wire)
//Writes : MEM_RD_DATA
void dsp_rec_MEM_RD_DATA_WRITE(x86_block& x86e,_INST& op,u32 step,x86_gpr_reg MEM_RD_DATA_NV)
{
	//Request : step x (odd step)
	//Operation : x+1   (even step)
	//Data avail : x+2   (odd step, can request again)
	//The MEM_RD_DATA_NV wire exists only on even steps
	if (!(step&1))
	{
		x86e.Emit(op_mov32,&dsp.regs.MEM_RD_DATA,MEM_RD_DATA_NV);
	}

	wtn(MEM_RD_DATA);
}

x86_mrm dsp_reg_GenerateTempsAddrs(x86_block& x86e,u32 TEMPS_NUM,x86_gpr_reg TEMPSaddrsreg)
{
	x86e.Emit(op_mov32,TEMPSaddrsreg,&dsp.regs.MDEC_CT);
	x86e.Emit(op_add32,TEMPSaddrsreg,TEMPS_NUM);
	x86e.Emit(op_and32,TEMPSaddrsreg,127);
	return x86_mrm::create(ECX,sib_scale_4,dsp.TEMP);
}
//Reads : INPUTS,TEMP,FRC_REG,COEF,Y_REG
//Writes : MAD_OUT_NV (Wire)
void dsp_rec_MAD(x86_block& x86e,_INST& op,u32 step,x86_gpr_reg INPUTS,x86_gpr_reg MAD_OUT_NV)
{
	bool use_TEMP=op.XSEL==0 || (op.BSEL==0 && op.ZERO==0);

	//TEMPS (if used) on ECX
	const x86_gpr_reg TEMPS_reg=ECX;
	if (use_TEMP)
	{
		//read temps
		x86e.Emit(op_mov32,TEMPS_reg,dsp_reg_GenerateTempsAddrs(x86e,op.TRA,TEMPS_reg));
		dsp_rec_se(x86e,TEMPS_reg,24);
	}

	x86_reg mul_x_input;
	//X : 24 bits
	if (op.XSEL==1)
	{
		//X=INPUTS
		mul_x_input=INPUTS;
		//x86e.Emit(op_mov32,EDX,INPUTS);
	}
	else
	{
		//X=TEMPS
		mul_x_input=TEMPS_reg;
		//x86e.Emit(op_mov32,EDX,TEMPS_reg);
	}

	//MUL Y in : EAX
	//Y : 13 bits
	switch(op.YSEL)
	{
	case 0:
		//Y=FRC_REG[13]
		x86e.Emit(op_mov32,EAX,&dsp.regs.FRC_REG);
		dsp_rec_se(x86e,EAX,13);
		break;

	case 1:
		//Y=COEF[13]
		x86e.Emit(op_mov32,EAX,&DSPData->COEF[step]);
		dsp_rec_se(x86e,EAX,16,13);
		break;

	case 2:
		//Y=Y_REG[23:11] (Y_REG is 19 bits, INPUTS[23:4], so that is realy 19:7)
		x86e.Emit(op_mov32,EAX,&dsp.regs.Y_REG);
		dsp_rec_se(x86e,EAX,19,13);
		break;

	case 3:
		//Y=0'Y_REG[15:4] (Y_REG is 19 bits, INPUTS[23:4], so that is realy 11:0)
		x86e.Emit(op_mov32,EAX,&dsp.regs.Y_REG);
		x86e.Emit(op_and32,0xFFF);//Clear bit 13+
		break;
	}

	//Do the mul -- maby it has overflow protection ?
	//24+13=37, -11 = 26
	//that can be >>1 or >>2 on the shifter after the mul 
	x86e.Emit(op_imul32,mul_x_input);
	//*NOTE* here, shrd is unsigned, but we have EDX signed, and we may only shift up to 11 bits from it
	//so it works just fine :)
	x86e.Emit(op_shrd32,EAX,EDX,10);

	//cut the upper bits so that it is 26 bits signed
	dsp_rec_se(x86e,EAX,26);

	//Adder, takes MUL_OUT at EAX
	//Adds B (EDX)
	//Outputs EAX

	if (!op.ZERO)	//if zero is set the adder has no effect
	{
		if (op.BSEL==1)
		{
			//B=MAD_OUT[??]
			//mad out is stored on s32 format, so no need for sign extention
			x86e.Emit(op_mov32,EDX,&dsp.regs.MAD_OUT);
		}
		else
		{
			//B=TEMP[??]
			//TEMPS is allready sign extended, so no need for it
			//Just converting 24 -> 26 bits using lea
			x86e.Emit(op_lea32,EDX,x86_mrm::create(TEMPS_reg,sib_scale_4,0));
		}
		//Gating is aplied here normaly (ZERO).
		//NEGB then inverts the value  (NOT) (or 0 , if gated) and the adder adds +1 if NEGB is set.
		//However, (~X)+1 = -X , and (~0)+1=0 so i skip the add
		if (op.NEGB)
		{
			x86e.Emit(op_neg32,EDX);
		}
		
		//Add hm, is there overflow protection here ?
		//The resultof mul is on EAX, we modify that
		x86e.Emit(op_add32,EAX,EDX);
	}

	//cut the upper bits so that it is 26 bits signed
	dsp_rec_se(x86e,EAX,26);

	//Write to MAD_OUT_NV wire :)
	x86e.Emit(op_mov32,MAD_OUT_NV,EAX);
}

//Rreads : INPUTS,MAD_OUT
//Writes : EFREG,TEMP,FRC_REG,ADRS_REG,MEM_WT_DATA
void dsp_rec_EFO_FB(x86_block& x86e,_INST& op,u32 step,x86_gpr_reg INPUTS)
{
	nwtn(MAD_OUT);
	//MAD_OUT is s32, no sign extention needed
	x86e.Emit(op_mov32,EAX,&dsp.regs.MAD_OUT);
	//sh .. l ?
	switch(op.SHIFT)
	{
	case 0:
		x86e.Emit(op_sar32,EAX,2);
		//×1 Protected
		x86e.Emit(op_mov32,EDX,(u32)-524288);//8388608//32768//524288
		x86e.Emit(op_cmp32,EAX,EDX);
		x86e.Emit(op_cmovl32,EAX,EDX);
		x86e.Emit(op_neg32,EDX);
		x86e.Emit(op_cmp32,EAX,EDX);
		x86e.Emit(op_cmovg32,EAX,EDX);
		//protect !
		break;
	case 1:
		//×2 Protected
		x86e.Emit(op_sar32,EAX,1);

		x86e.Emit(op_mov32,EDX,(u32)-524288);//8388608//32768//524288
		x86e.Emit(op_cmp32,EAX,EDX);
		x86e.Emit(op_cmovl32,EAX,EDX);
		x86e.Emit(op_not32,EDX);
		x86e.Emit(op_cmp32,EAX,EDX);
		x86e.Emit(op_cmovg32,EAX,EDX);
		//protect !
		break;
	case 2:
		//×2 Not protected
		x86e.Emit(op_sar32,EAX,1);
		dsp_rec_se(x86e,EAX,24);
		break;
	case 3:
		//×1 Not protected
		x86e.Emit(op_sar32,EAX,1);
		x86e.Emit(op_shl32,EAX,2);
		dsp_rec_se(x86e,EAX,24);
		break;
	}

	//Write EFREG ?
	if (op.EWT)
	{
		x86e.Emit(op_mov32,EDX,EAX);
		//top 16 bits ? or lower 16 ? 
		//i use top 16, folowing the same rule as the input 
		x86e.Emit(op_sar32,EDX,4);

		//write :)
		x86e.Emit(op_mov16,&DSPData->EFREG[op.EWA],DX);
	}

	//Write TEMPS ?
	if (op.TWT)
	{	
		//Temps is 24 bit, stored as s32 ( no convertion required)

		//write it
		x86e.Emit(op_mov32,dsp_reg_GenerateTempsAddrs(x86e,op.TWA,ECX),EAX);
	}
 
	//COMMON TO FRC_REG and ADRS_REG
	//interpolation mode : shift1=1=shift0
	//non interpolation : shift1!=1 && shift0!=1 ? ( why && ?) -- i implement it as ||

	//Write to FRC_REG ?
	if (op.FRCL)
	{
		if (op.SHIFT==3)
		{
			//FRC_REG[12:0]=Shift[23:11]
			x86e.Emit(op_mov32,ECX,EAX);
			x86e.Emit(op_sar32,ECX,11);
		}
		else
		{
			//FRC_REG[12:0]=0'Shift[11:0]
			x86e.Emit(op_mov32,ECX,EAX);
			x86e.Emit(op_and32,ECX,(1<<12)-1);//bit 12 and up are 0'd
		}
		x86e.Emit(op_mov32,&dsp.regs.FRC_REG,ECX);
	}
	
	//Write to ADDRS_REG ?
	if (op.ADRL)
	{
		if (op.SHIFT==3)
		{
			//ADRS_REG[11:0]=Shift[23,23,23,23,23,22:16]
			x86e.Emit(op_mov32,ECX,EAX);
			x86e.Emit(op_shl32,ECX,8);	//bit31=bit 23
			x86e.Emit(op_sar32,ECX,24); //bit 0 = bit16 (16+8=24)
		}
		else
		{
			//ADRS_REG[11:0]=0'Shift[23:12]
			x86e.Emit(op_mov32,ECX,EAX);
			x86e.Emit(op_sar32,ECX,12);
			x86e.Emit(op_and32,ECX,(1<<12)-1);//bit 11 and up are 0'd
		}
		x86e.Emit(op_mov32,&dsp.regs.ADRS_REG,ECX);
	}

	//MEM_WT_DATA write
	//This kills off any non protected regs (EAX,EDX,ECX)
	{
		//pack ?
		if (!op.NOFL && SUPPORT_NOFL)
		{	//yes
			x86e.Emit(op_mov32,ECX,EAX);
			x86e.Emit(op_call,x86_ptr_imm(PACK));
		}
		else
		{	//shift (look @ EFREG write for more info)
			x86e.Emit(op_sar32,EAX,8);
		}
		//data in on EAX
		x86e.Emit(op_mov32,&dsp.regs.MEM_WT_DATA,EAX);
	}

	//more stuff here
	wtn(EFREG);
	wtn(TEMPS);
	wtn(FRC_REG);
	wtn(ADRS_REG);
	wtn(MEM_WT_DATA);
}
void dsp_recompile()
{
	dsp.dyndirty=false;

	x86_block x86e;
	x86e.Init(dyna_realloc,dyna_realloc);
	
	x86e.Emit(op_push32,EBX);
	x86e.Emit(op_push32,EBP);
	x86e.Emit(op_push32,ESI);
	x86e.Emit(op_push32,EDI);

	//OK.
	//Input comes from mems, mixs and exts, as well as possible memory reads and writes
	//mems is read/write (memory loads go there), mixs and exts are read only.
	//There are varius delays (registers) so i need to properly emulate (more on that later)

	//Registers that can be writen : MIXS,FRC_REG,ADRS_REG,EFREG,TEMP

	//MRD, MWT, NOFL, TABLE, NXADR, ADREB, and MASA[4:0]
	//Only alowed on odd steps, when counting from 1 (2,4,6, ...).That is even steps when counting from 0 (1,3,5, ...)
	for(int step=0;step<128;++step)
	{
		u32* mpro=DSPData->MPRO+step*4;
		u32 prev_step=(step-1)&127;
		u32* prev_mpro=DSPData->MPRO+prev_step*4;
		//if its a nop just go to the next opcode
		//No, dont realy do that, we need to propage opcode bits :p
		//if (mpro[0]==0 && mpro[1]==0 && mpro[2]== 0 && mpro[3]==0)
		//	continue;

		_INST op;
		_INST prev_op;
		DecodeInst(mpro,&op);
		DecodeInst(prev_mpro,&prev_op);

		//printf("[%d] "
		//	"TRA %d,TWT %d,TWA %d,XSEL %d,YSEL %d,IRA %d,IWT %d,IWA %d,TABLE %d,MWT %d,MRD %d,EWT %d,EWA %d,ADRL %d,FRCL %d,SHIFT %d,YRL %d,NEGB %d,ZERO %d,BSEL %d,NOFL %d,MASA %d,ADREB %d,NXADR %d\n"
		//	,step
		//	,op.TRA,op.TWT,op.TWA,op.XSEL,op.YSEL,op.IRA,op.IWT,op.IWA,op.TABLE,op.MWT,op.MRD,op.EWT,op.EWA,op.ADRL,op.FRCL,op.SHIFT,op.YRL,op.NEGB,op.ZERO,op.BSEL,op.NOFL,op.MASA,op.ADREB,op.NXADR);

		//Dynarec !
		_dsp_debug_step_start();
		//DSP regs are on memory
		//Wires stay on x86 regs, writen to memory as fast as possible
		
		//EDI=MEM_RD_DATA_NV
		dsp_rec_DRAM_CI(x86e,prev_op,step,EDI);
		
		//;)
		//Address Generation Unit ! nothing spectacular realy ...
		dsp_rec_MEM_AGU(x86e,op,step);
		
		//Calculate INPUTS wire
		//ESI : INPUTS
		dsp_rec_INPUTS(x86e,op,ESI);
		
		//:o ?
		//Write the MEMS register
		dsp_rec_MEMS_WRITE(x86e,op,step,ESI);
		
		//Write the MEM_RD_DATA regiter
		//Last use of MEM_RD_DATA_NV(EDI)
		dsp_rec_MEM_RD_DATA_WRITE(x86e,op,step,EDI);
		//EDI is now free :D
		
		//EDI is used for MAD_OUT_NV
		//Mul-add
		dsp_rec_MAD(x86e,op,step,ESI,EDI);
		
		//Effect output/ Feedback
		dsp_rec_EFO_FB(x86e,op,step,ESI);

		//Write MAD_OUT_NV
		{
			x86e.Emit(op_mov32,&dsp.regs.MAD_OUT,EDI);
			wtn(MAD_OUT);
		}
		//These are implemented here :p

		//Inputs -> Y reg
		//Last use of inputs (ESI) and its destructive at that ;p
		{
			if (op.YRL)
			{
				x86e.Emit(op_sar32,ESI,4);//[23:4]
				x86e.Emit(op_mov32,&dsp.regs.Y_REG,ESI);

			}
			wtn(Y_REG);
		}

		//NOFL delay propagation :)
		{
			//NOFL_2=NOFL_1
			x86e.Emit(op_mov32,EAX,&dsp.regs.NOFL_1);
			x86e.Emit(op_mov32,&dsp.regs.NOFL_2,EAX);
			//NOFL_1 = NOFL
			x86e.Emit(op_mov32,&dsp.regs.NOFL_1,op.NOFL);

			wtn(NOFL_2);
			wtn(NOFL_1);
		}

		//MWT_1/MRD_1 propagation
		{
			//MWT_1=MWT
			x86e.Emit(op_mov32,&dsp.regs.MWT_1,op.MWT);
			//MRD_1=MRD
			x86e.Emit(op_mov32,&dsp.regs.MRD_1,op.MRD);

			wtn(MWT_1);
			wtn(MRD_1);
		}

		_dsp_debug_step_end();
	}

	//Need to decrement MDEC_CT here :)
	x86e.Emit(op_pop32,EDI);
	x86e.Emit(op_pop32,ESI);
	x86e.Emit(op_pop32,EBP);
	x86e.Emit(op_pop32,EBX);
	x86e.Emit(op_ret);
	x86e.Generate();
}



void dsp_print_mame();
void dsp_step_mame();
void dsp_emu_grandia();
void dsp_step()
{
	//clear output reg
	memset(DSPData->EFREG,0,sizeof(DSPData->EFREG));

	if (dsp.dyndirty)
	{
		dsp.dyndirty=false;
		//dsp_print_mame();
		dsp_recompile();
	}
	//dsp_step_mame();
	//dsp_emu_grandia();
	
	//run the code :p
	((void (*)())&dsp.DynCode)();

	dsp.regs.MDEC_CT--;
	if (dsp.regs.MDEC_CT==0)
		dsp.regs.MDEC_CT=dsp.RBL;
	//here ? or before ?
	//memset(DSP->MIXS,0,4*16);
}

void dsp_writenmem(u32 addr)
{
	addr-=0x3000;
	//COEF : native
	//MEMS : native
	//MPRO : native
	if (addr>=0x400 && addr<0xC00)
	{
		dsp.dyndirty=true;
	}

	/*
	//buffered DSP state
	//24 bit wide
	u32 TEMP[128];
	//24 bit wide
	u32 MEMS[32];
	//20 bit wide
	s32 MIXS[16];
	*/
}

void dsp_readmem(u32 addr)
{
	//nothing ? :p
}


//MAME - Elsemi :)

void dsp_emu_grandia()
{
	static bool stuff=true;
	if (stuff)
		return;
	//DSP PROGRAM DUMP START
	INT32 ACC=0;    //26 bit
	INT32 SHIFTED=0;    //24 bit
	INT32 X=0;  //24 bit
	INT32 Y=0;  //13 bit
	INT32 B=0;  //26 bit
	INT32 INPUTS=0; //24 bit
	INT32 MEMVAL=0;
	INT32 FRC_REG=0;    //13 bit
	INT32 Y_REG=0;      //24 bit
	UINT32 ADDR=0;
	UINT32 ADRS_REG=0;  //13 bit
	memset(DSPData->EFREG,0,sizeof(DSPData->EFREG));
	INT64 v;
	INPUTS=dsp.MIXS[32-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[0]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[33-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[1]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	ADDR=DSPData->MADRS[1<<1];
	ADDR+=dsp.DEC;
	ADDR&=dsp.RBL;
	MEMVAL=UNPACK(((u16*)(aica_ram+dsp.RBP))[ADDR]);
	INPUTS=dsp.MIXS[34-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[2]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[35-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	dsp.MEMS[8]=MEMVAL;  //MEMVAL was selected in previous MRD
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[3]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	ADDR=DSPData->MADRS[3<<1];
	ADDR+=dsp.DEC;
	ADDR&=dsp.RBL;
	MEMVAL=UNPACK(((u16*)(aica_ram+dsp.RBP))[ADDR]);
	INPUTS=dsp.MIXS[36-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[4]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[37-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	dsp.MEMS[9]=MEMVAL;  //MEMVAL was selected in previous MRD
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[5]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	ADDR=DSPData->MADRS[5<<1];
	ADDR+=dsp.DEC;
	ADDR&=dsp.RBL;
	MEMVAL=UNPACK(((u16*)(aica_ram+dsp.RBP))[ADDR]);
	INPUTS=dsp.MIXS[38-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[6]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[39-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	dsp.MEMS[0]=MEMVAL;  //MEMVAL was selected in previous MRD
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[7]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	ADDR=DSPData->MADRS[7<<1];
	ADDR+=dsp.DEC;
	ADDR&=dsp.RBL;
	MEMVAL=UNPACK(((u16*)(aica_ram+dsp.RBP))[ADDR]);
	INPUTS=dsp.MIXS[32-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[8]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[0]+=SHIFTED>>8;
	INPUTS=dsp.MIXS[33-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	dsp.MEMS[2]=MEMVAL;  //MEMVAL was selected in previous MRD
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[9]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	ADDR=DSPData->MADRS[9<<1];
	ADDR+=dsp.DEC;
	ADDR&=dsp.RBL;
	MEMVAL=UNPACK(((u16*)(aica_ram+dsp.RBP))[ADDR]);
	INPUTS=dsp.MIXS[34-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[10]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[35-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	dsp.MEMS[5]=MEMVAL;  //MEMVAL was selected in previous MRD
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[11]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[36-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[12]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[37-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[13]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[38-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[14]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[39-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[15]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(16+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[16]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[1]+=SHIFTED>>8;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(9+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[17]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(18+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[18]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(27+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[19]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(36+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[20]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(45+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[21]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(54+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[22]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(63+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[23]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(72+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[24]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(9+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[25]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	ADDR=DSPData->MADRS[0<<1];
	ADDR+=dsp.DEC;
	ADDR&=dsp.RBL;
	((u16*)(aica_ram+dsp.RBP))[ADDR]=PACK(SHIFTED);
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(18+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[26]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(27+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[27]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(36+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[28]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(45+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[29]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(54+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[30]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(63+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[31]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(72+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[32]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[8];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[33]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	ADDR=DSPData->MADRS[2<<1];
	ADDR+=dsp.DEC;
	ADDR&=dsp.RBL;
	((u16*)(aica_ram+dsp.RBP))[ADDR]=PACK(SHIFTED);
	INPUTS=dsp.MEMS[9];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[34]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[2]+=SHIFTED>>8;
	INPUTS=dsp.MIXS[32-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[35]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[3]+=SHIFTED>>8;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(8+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[36]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(0+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(6+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[37]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[32-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[38]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[33-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[39]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(7+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(17+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[40]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(9+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(15+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[41]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[33-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[42]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[34-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[43]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(16+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(26+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[44]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(18+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(24+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[45]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[34-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[46]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[35-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[47]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(25+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(35+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[48]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(27+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(33+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[49]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[35-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[50]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[36-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[51]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(34+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(44+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[52]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(36+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(42+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[53]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[36-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[54]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[37-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[55]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(43+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(53+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[56]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(45+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(51+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[57]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[37-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[58]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[38-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[59]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(52+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(62+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[60]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(54+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(60+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[61]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[38-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[62]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[39-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[63]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(61+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(71+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[64]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(63+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(69+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[65]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[39-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[66]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[40-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[67]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(70+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MIXS[41-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[68]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[8]+=SHIFTED>>8;
	INPUTS=dsp.MIXS[42-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[69]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[9]+=SHIFTED>>8;
	INPUTS=dsp.MIXS[43-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[70]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[10]+=SHIFTED>>8;
	INPUTS=dsp.MIXS[44-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[71]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[11]+=SHIFTED>>8;
	INPUTS=dsp.MIXS[45-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[72]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[12]+=SHIFTED>>8;
	INPUTS=dsp.MIXS[46-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[73]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[13]+=SHIFTED>>8;
	INPUTS=dsp.MIXS[47-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[74]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[14]+=SHIFTED>>8;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(63+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[75]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	DSPData->EFREG[15]+=SHIFTED>>8;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(54+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[76]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(45+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[77]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(36+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[78]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(27+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[79]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(18+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[80]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(9+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[81]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(0+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[82]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(83+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[83]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(82+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MIXS[47-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=dsp.TEMP[(82+dsp.DEC)&0x7F];
	B<<=8;
	B>>=8;
	X=INPUTS;
	Y=DSPData->COEF[84]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[46-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[85]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[45-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[86]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[44-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[87]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[43-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[88]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[42-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[89]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[41-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[90]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MIXS[40-0x20]<<4;  //MIXS is 20 bit
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[91]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(84+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[92]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(82+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(82+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[93]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(102+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[94]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(83+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(94+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[95]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(92+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[96]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(83+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[97]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(104+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[98]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(93+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[99]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(100+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[100]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(103+dsp.DEC)&0x7F]=SHIFTED;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(93+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[101]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(103+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[102]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(106+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[103]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	ADDR=DSPData->MADRS[4<<1];
	ADDR+=dsp.DEC;
	ADDR&=dsp.RBL;
	((u16*)(aica_ram+dsp.RBP))[ADDR]=PACK(SHIFTED);
	INPUTS=dsp.MEMS[2];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[104]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=dsp.TEMP[(108+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[105]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(105+dsp.DEC)&0x7F]=SHIFTED;
	DSPData->EFREG[4]+=SHIFTED>>8;
	INPUTS=dsp.MEMS[5];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=INPUTS;
	Y=DSPData->COEF[106]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(107+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[107]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	dsp.TEMP[(107+dsp.DEC)&0x7F]=SHIFTED;
	DSPData->EFREG[5]+=SHIFTED>>8;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[108]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(93+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[109]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(107+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[110]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(111+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[111]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	ADDR=DSPData->MADRS[6<<1];
	ADDR+=dsp.DEC;
	ADDR&=dsp.RBL;
	((u16*)(aica_ram+dsp.RBP))[ADDR]=PACK(SHIFTED);
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=0;
	X=INPUTS;
	Y=DSPData->COEF[112]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(93+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[113]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(105+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[114]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(115+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[115]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	ADDR=DSPData->MADRS[8<<1];
	ADDR+=dsp.DEC;
	ADDR&=dsp.RBL;
	((u16*)(aica_ram+dsp.RBP))[ADDR]=PACK(SHIFTED);
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(116+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[116]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(117+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[117]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(118+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[118]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(119+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[119]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(120+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[120]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(121+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[121]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(122+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[122]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(123+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[123]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(124+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[124]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(125+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[125]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(126+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[126]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	INPUTS=dsp.MEMS[0];
	INPUTS<<=8;
	INPUTS>>=8;
	B=ACC;
	X=dsp.TEMP[(127+dsp.DEC)&0x7F];
	X<<=8;
	X>>=8;
	Y=DSPData->COEF[127]>>3;    //COEF is 16 bits
	SHIFTED=ACC;
	if(SHIFTED>0x007FFFFF)
		SHIFTED=0x007FFFFF;
	if(SHIFTED<(-0x00800000))
		SHIFTED=-0x00800000;
	Y<<=19;
	Y>>=19;
	v=(((INT64) X*(INT64) Y)>>12);
	ACC=(int) v+B;
	if (dsp.DEC==0)
		dsp.DEC=dsp.RBL+1;
	--dsp.DEC;
	//DSP PROGRAM DUMP END
}
void dsp_print_mame()
{
	printf("DSP PROGRAM DUMP START\n");
	printf("INT32 ACC=0;    //26 bit\n");
	printf("INT32 SHIFTED=0;    //24 bit\n");
	printf("INT32 X=0;  //24 bit\n");
	printf("INT32 Y=0;  //13 bit\n");
	printf("INT32 B=0;  //26 bit\n");
	printf("INT32 INPUTS=0; //24 bit\n");
	printf("INT32 MEMVAL=0;\n");
	printf("INT32 FRC_REG=0;    //13 bit\n");
	printf("INT32 Y_REG=0;      //24 bit\n");
	printf("UINT32 ADDR=0;\n");
	printf("UINT32 ADRS_REG=0;  //13 bit\n");
	int step;


	printf("memset(DSPData->EFREG,0,sizeof(DSPData->EFREG));\n");
	printf("INT64 v;\n");

	for(step=0;step<128;++step)
	{
		UINT32 *IPtr=DSPData->MPRO+step*4;

		//      if(IPtr[0]==0 && IPtr[1]==0 && IPtr[2]==0 && IPtr[3]==0)
		//          break;

		UINT32 TRA=(IPtr[0]>>9)&0x7F;
		UINT32 TWT=(IPtr[0]>>8)&0x01;
		UINT32 TWA=(IPtr[0]>>1)&0x7F;

		UINT32 XSEL=(IPtr[1]>>15)&0x01;
		UINT32 YSEL=(IPtr[1]>>13)&0x03;
		UINT32 IRA=(IPtr[1]>>7)&0x3F;
		UINT32 IWT=(IPtr[1]>>6)&0x01;
		UINT32 IWA=(IPtr[1]>>1)&0x1F;

		UINT32 TABLE=(IPtr[2]>>15)&0x01;
		UINT32 MWT=(IPtr[2]>>14)&0x01;
		UINT32 MRD=(IPtr[2]>>13)&0x01;
		UINT32 EWT=(IPtr[2]>>12)&0x01;
		UINT32 EWA=(IPtr[2]>>8)&0x0F;
		UINT32 ADRL=(IPtr[2]>>7)&0x01;
		UINT32 FRCL=(IPtr[2]>>6)&0x01;
		UINT32 SHIFT=(IPtr[2]>>4)&0x03;
		UINT32 YRL=(IPtr[2]>>3)&0x01;
		UINT32 NEGB=(IPtr[2]>>2)&0x01;
		UINT32 ZERO=(IPtr[2]>>1)&0x01;
		UINT32 BSEL=(IPtr[2]>>0)&0x01;

		UINT32 NOFL=(IPtr[3]>>15)&1;        //????
		//UINT32 COEF=step;

		UINT32 MASA=(IPtr[3]>>9)&0x1f;  //???
		UINT32 ADREB=(IPtr[3]>>8)&0x1;
		UINT32 NXADR=(IPtr[3]>>7)&0x1;

		INT64 v;

		//operations are done at 24 bit precision

		//INPUTS RW
		assert(IRA<0x32);
		if(IRA<=0x1f)
			printf("INPUTS=dsp.MEMS[%d];\n",IRA);
		else if(IRA<=0x2F)
			printf("INPUTS=dsp.MIXS[%d-0x20]<<4;  //MIXS is 20 bit\n",IRA);
		else if(IRA<=0x31)
			printf("INPUTS=0;\n");

		printf("INPUTS<<=8;\n");
		printf("INPUTS>>=8;\n");
		//if(INPUTS&0x00800000)
		//  INPUTS|=0xFF000000;

		if(IWT)
		{
			printf("dsp.MEMS[%d]=MEMVAL;  //MEMVAL was selected in previous MRD\n",IWA);
			if(IRA==IWA)
				printf("INPUTS=MEMVAL;\n");
		}

		//Operand sel
		//B
		if(!ZERO)
		{
			if(BSEL)
				printf("B=ACC;\n");
			else
			{
				printf("B=dsp.TEMP[(%d+dsp.DEC)&0x7F];\n",TRA);
				printf("B<<=8;\n");
				printf("B>>=8;\n");
				//if(B&0x00800000)
				//  B|=0xFF000000;  //Sign extend
			}
			if(NEGB)
				printf("B=0-B;\n");
		}
		else
			printf("B=0;\n");

		//X
		if(XSEL)
			printf("X=INPUTS;\n");
		else
		{
			printf("X=dsp.TEMP[(%d+dsp.DEC)&0x7F];\n",TRA);
			printf("X<<=8;\n");
			printf("X>>=8;\n");
			//if(X&0x00800000)
			//  X|=0xFF000000;
		}

		//Y
		if(YSEL==0)
			printf("Y=FRC_REG;\n");
		else if(YSEL==1)
			printf("Y=DSPData->COEF[%d]>>3;    //COEF is 16 bits\n",step);
		else if(YSEL==2)
			printf("Y=(Y_REG>>11)&0x1FFF;\n");
		else if(YSEL==3)
			printf("Y=(Y_REG>>4)&0x0FFF;\n");

		if(YRL)
			printf("Y_REG=INPUTS;\n");

		//Shifter
		if(SHIFT==0)
		{
			printf("SHIFTED=ACC;\n");
			printf("if(SHIFTED>0x007FFFFF)\n");
			printf("	SHIFTED=0x007FFFFF;\n");
			printf("if(SHIFTED<(-0x00800000))\n");
			printf("	SHIFTED=-0x00800000;\n");
		}
		else if(SHIFT==1)
		{
			printf("SHIFTED=ACC*2;\n");
			printf("if(SHIFTED>0x007FFFFF)\n");
			printf("	SHIFTED=0x007FFFFF;\n");
			printf("if(SHIFTED<(-0x00800000))\n");
			printf("	SHIFTED=-0x00800000;\n");
		}
		else if(SHIFT==2)
		{
			printf("SHIFTED=ACC*2;\n");
			printf("SHIFTED<<=8;\n");
			printf("SHIFTED>>=8;\n");
			//SHIFTED&=0x00FFFFFF;
			//if(SHIFTED&0x00800000)
			//  SHIFTED|=0xFF000000;
		}
		else if(SHIFT==3)
		{
			printf("SHIFTED=ACC;\n");
			printf("SHIFTED<<=8;\n");
			printf("SHIFTED>>=8;\n");
			//SHIFTED&=0x00FFFFFF;
			//if(SHIFTED&0x00800000)
			//  SHIFTED|=0xFF000000;
		}

		//ACCUM
		printf("Y<<=19;\n");
		printf("Y>>=19;\n");
		//if(Y&0x1000)
		//  Y|=0xFFFFF000;

		printf("v=(((INT64) X*(INT64) Y)>>12);\n");
		printf("ACC=(int) v+B;\n");

		if(TWT)
			printf("dsp.TEMP[(%d+dsp.DEC)&0x7F]=SHIFTED;\n",TWA);

		if(FRCL)
		{
			if(SHIFT==3)
				printf("FRC_REG=SHIFTED&0x0FFF;\n");
			else
				printf("FRC_REG=(SHIFTED>>11)&0x1FFF;\n");
		}

		if(MRD || MWT)
			//if(0)
		{
			printf("ADDR=DSPData->MADRS[%d<<1];\n",MASA);
			if(!TABLE)
				printf("ADDR+=dsp.DEC;\n");
			if(ADREB)
				printf("ADDR+=ADRS_REG&0x0FFF;\n");
			if(NXADR)
				printf("ADDR++;\n");
			if(!TABLE)
				printf("ADDR&=dsp.RBL;\n");
			else
				printf("ADDR&=0xFFFF;\n");
			//ADDR<<=1;
			//ADDR+=DSP->RBP<<13;
			//MEMVAL=DSP->AICARAM[ADDR>>1];
			//ADDR+=DSP->RBP<<10;
			if(MRD && (step&1)) //memory only allowed on odd? DoA inserts NOPs on even
			{
				if(NOFL)
					printf("MEMVAL=((u16*)(aica_ram+dsp.RBP))[ADDR]<<8;\n");
				else
					printf("MEMVAL=UNPACK(((u16*)(aica_ram+dsp.RBP))[ADDR]);\n");
			}
			if(MWT && (step&1))
			{
				if(NOFL)
					printf("((u16*)(aica_ram+dsp.RBP))[ADDR]=SHIFTED>>8;\n");
				else
					printf("((u16*)(aica_ram+dsp.RBP))[ADDR]=PACK(SHIFTED);\n");
			}
		}

		if(ADRL)
		{
			if(SHIFT==3)
				printf("ADRS_REG=(SHIFTED>>12)&0xFFF;\n");
			else
				printf("ADRS_REG=(INPUTS>>16);\n");
		}

		if(EWT)
			printf("DSPData->EFREG[%d]+=SHIFTED>>8;\n",EWA);

	}
	printf("if (dsp.DEC==0)\n");
	printf("	dsp.DEC=dsp.RBL+1;\n");
	printf("--dsp.DEC;\n");
	//  memset(DSP->MIXS,0,4*16);
	//  if(f)
	//      fclose(f);

	printf("DSP PROGRAM DUMP END\n");
}
void dsp_step_mame()
{
	INT32 ACC=0;    //26 bit
	INT32 SHIFTED=0;    //24 bit
	INT32 X=0;  //24 bit
	INT32 Y=0;  //13 bit
	INT32 B=0;  //26 bit
	INT32 INPUTS=0; //24 bit
	INT32 MEMVAL=0;
	INT32 FRC_REG=0;    //13 bit
	INT32 Y_REG=0;      //24 bit
	UINT32 ADDR=0;
	UINT32 ADRS_REG=0;  //13 bit
	int step;


	memset(DSPData->EFREG,0,sizeof(DSPData->EFREG));

	for(step=0;step<128;++step)
	{
		UINT32 *IPtr=DSPData->MPRO+step*4;

		//      if(IPtr[0]==0 && IPtr[1]==0 && IPtr[2]==0 && IPtr[3]==0)
		//          break;

		UINT32 TRA=(IPtr[0]>>9)&0x7F;
		UINT32 TWT=(IPtr[0]>>8)&0x01;
		UINT32 TWA=(IPtr[0]>>1)&0x7F;

		UINT32 XSEL=(IPtr[1]>>15)&0x01;
		UINT32 YSEL=(IPtr[1]>>13)&0x03;
		UINT32 IRA=(IPtr[1]>>7)&0x3F;
		UINT32 IWT=(IPtr[1]>>6)&0x01;
		UINT32 IWA=(IPtr[1]>>1)&0x1F;

		UINT32 TABLE=(IPtr[2]>>15)&0x01;
		UINT32 MWT=(IPtr[2]>>14)&0x01;
		UINT32 MRD=(IPtr[2]>>13)&0x01;
		UINT32 EWT=(IPtr[2]>>12)&0x01;
		UINT32 EWA=(IPtr[2]>>8)&0x0F;
		UINT32 ADRL=(IPtr[2]>>7)&0x01;
		UINT32 FRCL=(IPtr[2]>>6)&0x01;
		UINT32 SHIFT=(IPtr[2]>>4)&0x03;
		UINT32 YRL=(IPtr[2]>>3)&0x01;
		UINT32 NEGB=(IPtr[2]>>2)&0x01;
		UINT32 ZERO=(IPtr[2]>>1)&0x01;
		UINT32 BSEL=(IPtr[2]>>0)&0x01;

		UINT32 NOFL=(IPtr[3]>>15)&1;        //????
		//UINT32 COEF=step;

		UINT32 MASA=(IPtr[3]>>9)&0x1f;  //???
		UINT32 ADREB=(IPtr[3]>>8)&0x1;
		UINT32 NXADR=(IPtr[3]>>7)&0x1;

		INT64 v;

		//operations are done at 24 bit precision

		//INPUTS RW
		assert(IRA<0x32);
		if(IRA<=0x1f)
			INPUTS=dsp.MEMS[IRA];
		else if(IRA<=0x2F)
			INPUTS=dsp.MIXS[IRA-0x20]<<4;  //MIXS is 20 bit
		else if(IRA<=0x31)
			INPUTS=0;

		INPUTS<<=8;
		INPUTS>>=8;
		//if(INPUTS&0x00800000)
		//  INPUTS|=0xFF000000;

		if(IWT)
		{
			dsp.MEMS[IWA]=MEMVAL;  //MEMVAL was selected in previous MRD
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
				B=dsp.TEMP[(TRA+dsp.DEC)&0x7F];
				B<<=8;
				B>>=8;
				//if(B&0x00800000)
				//  B|=0xFF000000;  //Sign extend
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
			X=dsp.TEMP[(TRA+dsp.DEC)&0x7F];
			X<<=8;
			X>>=8;
			//if(X&0x00800000)
			//  X|=0xFF000000;
		}

		//Y
		if(YSEL==0)
			Y=FRC_REG;
		else if(YSEL==1)
			Y=DSPData->COEF[step]>>3;    //COEF is 16 bits
		else if(YSEL==2)
			Y=(Y_REG>>11)&0x1FFF;
		else if(YSEL==3)
			Y=(Y_REG>>4)&0x0FFF;

		if(YRL)
			Y_REG=INPUTS;

		//Shifter
		if(SHIFT==0)
		{ //*1
			SHIFTED=ACC>>2;
			if(SHIFTED>0x007FFF)
				SHIFTED=0x007FFF;
			if(SHIFTED<(-0x008000))
				SHIFTED=-0x008000;
		}
		else if(SHIFT==1)
		{//*2
			SHIFTED=ACC>>1;
			if(SHIFTED>0x007FFF)
				SHIFTED=0x007FFF;
			if(SHIFTED<(-0x008000))
				SHIFTED=-0x008000;
		}
		else if(SHIFT==2)
		{//*1
			SHIFTED=ACC>>2;
			SHIFTED<<=8;
			SHIFTED>>=8;
			//SHIFTED&=0x00FFFFFF;
			//if(SHIFTED&0x00800000)
			//  SHIFTED|=0xFF000000;
		}
		else if(SHIFT==3)
		{//*2
			SHIFTED=ACC>>1;
			SHIFTED<<=8;
			SHIFTED>>=8;
			//SHIFTED&=0x00FFFFFF;
			//if(SHIFTED&0x00800000)
			//  SHIFTED|=0xFF000000;
		}

		//ACCUM
		Y<<=19;
		Y>>=19;
		//if(Y&0x1000)
		//  Y|=0xFFFFF000;

		v=(((INT64) X*(INT64) Y)>>12);
		ACC=(int) v+B;

		if(TWT)
			dsp.TEMP[(TWA+dsp.DEC)&0x7F]=SHIFTED;

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
			ADDR=DSPData->MADRS[MASA<<1];
			if(!TABLE)
				ADDR+=dsp.DEC;
			if(ADREB)
				ADDR+=ADRS_REG&0x0FFF;
			if(NXADR)
				ADDR++;
			if(!TABLE)
				ADDR&=dsp.RBL;
			else
				ADDR&=0xFFFF;
			//ADDR<<=1;
			//ADDR+=DSP->RBP<<13;
			//MEMVAL=DSP->AICARAM[ADDR>>1];
			//ADDR+=DSP->RBP<<10;
			if(MRD && (step&1)) //memory only allowed on odd? DoA inserts NOPs on even
			{
				if(NOFL)
					MEMVAL=((u16*)(aica_ram+dsp.RBP))[ADDR]<<8;
				else
					MEMVAL=UNPACK(((u16*)(aica_ram+dsp.RBP))[ADDR]);
			}
			if(MWT && (step&1))
			{
				if(NOFL)
					((u16*)(aica_ram+dsp.RBP))[ADDR]=SHIFTED>>8;
				else
					((u16*)(aica_ram+dsp.RBP))[ADDR]=PACK(SHIFTED);
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
			DSPData->EFREG[EWA]=SHIFTED;

	}
	if (dsp.DEC==0)
		dsp.DEC=dsp.RBL+1;
	--dsp.DEC;
	//  memset(DSP->MIXS,0,4*16);
	//  if(f)
	//      fclose(f);
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

void dsp_recompile_elsemi()
{	
	dsp.dyndirty=false;
	int i;
	for(i=127;i>=0;--i)
	{
		u32 *IPtr=DSPData->MPRO+i*4;

		if(IPtr[0]!=0 || IPtr[1]!=0 || IPtr[2]!=0 || IPtr[3]!=0)
			break;
	}
	int LastStep=i+1;

	unsigned char *PtrInsts=(unsigned char *)dsp.DynCode;

	PUSHA();

	#define USES_SHIFTED(inst)	(inst.TWT || inst.FRCL || inst.MWT || inst.ADRL || inst.EWT)

	for(int step=0;step</*128*/LastStep;++step)
	{
		u32 *IPtr=DSPData->MPRO+step*4;
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
			{//24 -> 32
				//INPUTS=dsp.MEMS[IRA];	
				MOV_MEMTOEAX(&(dsp.MEMS[ThisInst.IRA]));
				SHL_EAX(8);
			}
			else if(ThisInst.IRA<=0x2F)
			{//20 -> 32
				//INPUTS=DSP->MIXS[IRA-0x20]<<8;	//MIXS is 16 bit
				MOV_MEMTOEAX(&(dsp.MIXS[ThisInst.IRA-0x20]));
				SHL_EAX(12);
			}
			else if(ThisInst.IRA<=0x31)
			{//16 -> 32
				MOV_MEMTOEAX(&(DSPData->EXTS[ThisInst.IRA-0x30]));
				SHL_EAX(16);				
			}
			else
			{
				MOV_0TOEAX();
			}
			SAR_EAX(8);
			
			MOV_EAXTOMEM(&(dsp.INPUTS));
		}

		if(ThisInst.IWT)
		{
			MOV_MEMTOEAX(&dsp.MEMVAL);
			MOV_EAXTOMEM(&dsp.MEMS[ThisInst.IWA]);
			//dsp.MEMS[IWA]=MEMVAL;	//MEMVAL was selected in previous MRD
			//SRAM should delay here
			//if(ThisInst.IRA==ThisInst.IWA)
			{
				//INPUTS=MEMVAL;
			//	MOV_EAXTOMEM(&dsp.INPUTS);
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
					MOV_MEMTOEAX(&dsp.ACC);
	//				MOV_EAXTOMEM(&DSP->B);	//
				}
				else
				{
					MOV_IMMTOEAX(ThisInst.TRA);
					ADD_MEMTOEAX(&(dsp.DEC));
					AND_EAX(0x7F);
					SHL_EAX(2);
					ADD_IMMTOEAX(&(dsp.TEMP));
					MOV_MEMEAXTOEAX();
					SHL_EAX(8);
					SAR_EAX(8);
	//				MOV_EAXTOMEM(&DSP->B);	//


					//B=DSP->TEMP[(TRA+dsp.DEC)&0x7F];
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
				MOV_MEMTOEAX(&dsp.INPUTS);
	//			MOV_EAXTOMEM(&DSP->X);	//
			}
			else
			{
				//X=DSP->TEMP[(TRA+dsp.DEC)&0x7F];
				//X<<=8;
				//X>>=8;
				MOV_IMMTOEAX(ThisInst.TRA);
				ADD_MEMTOEAX(&(dsp.DEC));
				AND_EAX(0x7F);
				SHL_EAX(2);
				ADD_IMMTOEAX(&(dsp.TEMP));
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
				MOV_MEMTOEAX(&dsp.ACC);
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
				MOV_MEMTOEAX(&dsp.ACC);
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
				MOV_MEMTOEAX(&dsp.ACC);
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
				MOV_MEMTOEAX(&dsp.ACC);
				SHL_EAX(8);
				SAR_EAX(8);
				//SHIFTED&=0x00FFFFFF;
				//if(SHIFTED&0x00800000)
				//	SHIFTED|=0xFF000000;
			}
			MOV_EAXTOMEM(&dsp.SHIFTED);
		}

		if((USES_SHIFTED(NextInst) || NextInst.BSEL) || !DYNOPT)
		{
			//Y
			if(ThisInst.YSEL==0)
			{
				//Y=FRC_REG;
				MOV_MEMTOEAX(&dsp.FRC_REG);
			}
			else if(ThisInst.YSEL==1)
			{
				//MOV_0TOEAX();
				MOV_MEMTOAX(&DSPData->COEF[step]);
				SAR_EAX(3);
				//Y=DSP->COEF[COEF]>>3;	//COEF is 16 bits
			}
			else if(ThisInst.YSEL==2)
			{
				//Y=(Y_REG>>11)&0x1FFF;
				MOV_MEMTOEAX(&dsp.Y_REG);
				SAR_EAX(11);
				AND_EAX(0x1FFF);
			}
			else if(ThisInst.YSEL==3)
			{
				//Y=(Y_REG>>4)&0x0FFF;
				MOV_MEMTOEAX(&dsp.Y_REG);
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
			SHRD_EAX_EDX(11);
			ADD_EAX_ECX();

			MOV_EAXTOMEM(&dsp.ACC);
		}

		if(ThisInst.YRL)
		{
			MOV_MEMTOEAX(&dsp.INPUTS);
			MOV_EAXTOMEM(&dsp.Y_REG);
			//Y_REG=INPUTS;
		}

		if(ThisInst.TWT)
		{
			MOV_MEMTOEAX(&dsp.SHIFTED);
			MOV_EAXTOEBX();
			//DSP->TEMP[(TWA+dsp.DEC)&0x7F]=SHIFTED;
			MOV_IMMTOEAX(ThisInst.TWA);
			ADD_MEMTOEAX(&(dsp.DEC));
			AND_EAX(0x7F);
			SHL_EAX(2);
			ADD_IMMTOEAX(&(dsp.TEMP));
			MOV_EBXTOMEMEAX();
		}

		if(ThisInst.FRCL)
		{
			if(ThisInst.SHIFT==3)
			{
				//FRC_REG=SHIFTED&0x0FFF;
				MOV_MEMTOEAX(&dsp.SHIFTED);
				AND_EAX(0x0FFF);
				MOV_EAXTOMEM(&dsp.FRC_REG);
			}
			else
			{
				//FRC_REG=(SHIFTED>>11)&0x1FFF;
				MOV_MEMTOEAX(&dsp.SHIFTED);
				SHR_EAX(11);
				AND_EAX(0x1FFF);
				MOV_EAXTOMEM(&dsp.FRC_REG);
			}
		}

		//MEM
		if(ThisInst.MRD || ThisInst.MWT)
		//if(0)
		{
			MOV_0TOEAX();
			MOV_MEMTOAX(&DSPData->MADRS[ThisInst.MASA]);
			//ADDR=DSP->MADRS[MASA];
			if(!ThisInst.TABLE)
			{
				//ADDR+=dsp.DEC;
				//ADD_MEMTOEAX(&dsp.DEC);
				MOV_MEMTOEBX(&dsp.DEC);
				ADD_EBXTOEAX();
			}
			if(ThisInst.ADREB)
			{
				MOV_MEMTOEBX(&(dsp.ADRS_REG));
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
				MOV_MEMTOEBX(&(dsp.RBL));
				//DEC_EBX(); // -> RBL IS -1 allready :p
				//ADDR&=dsp.RBL-1;
				AND_EAX_EBX();

				//AND_EAX((dsp.RBL-1));
			}
			else
			{
				//ADDR&=0xFFFF;
				AND_EAX(0xFFFF);
			}

			//ADDR+=DSP->RBP<<12;
			/*
			MOV_MEMTOEBX(&(DSP->RBP));
			SHL_EBX(12);
			ADD_EBXTOEAX();

			SHL_EAX(1);
			ADD_IMMTOEAX(DSP->DSPRAM);
			*/
			MOV_MEMTOEBX(&(dsp.RBP));
			SHL_EAX(1);
			ADD_EBXTOEAX();

			assert(!(ThisInst.MRD && ThisInst.MWT));	//this shouldn't happen, read & write in the same cycle

			if(ThisInst.MWT && (step&1))
			{
				if(ThisInst.NOFL)
				{
					//add DSPRAM
					MOV_EAXTOEBX();
					MOV_MEMTOEAX(&dsp.SHIFTED);
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
					MOV_MEMTOEAX(&dsp.SHIFTED);
					SHR_EAX(8);
					memcpy(PtrInsts,PackFunc,sizeof(PackFunc));
					PtrInsts+=sizeof(PackFunc);
#else
					//add DSPRAM
					MOV_EAXTOEBX();
					MOV_MEMTOEAX(&dsp.SHIFTED);
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
					//add DSPRAM
					MOV_MEMEAXTOAX();
					SHL_EAX(8);
					//MOV_EAXTOMEM(&dsp.MEMVAL);
				}
				else
				{
					//MEMVAL=UNPACK(DSP->DSPRAM[ADDR]);
					//add DSPRAM
					MOV_MEMEAXTOAX();
#ifdef USEFLOATPACK
					memcpy(PtrInsts,UnpackFunc,sizeof(UnpackFunc));
					PtrInsts+=sizeof(UnpackFunc);
#else
					SHL_EAX(16);
					SAR_EAX(8);
#endif
					MOV_EAXTOMEM(&dsp.MEMVAL);
				}
			}
			
			
			
		}

		if(ThisInst.ADRL)
		{
			if(ThisInst.SHIFT==3)
			{
				MOV_MEMTOEAX(&dsp.SHIFTED);
				SAR_EAX(12);
				AND_EAX(0xFFF);
				MOV_EAXTOMEM(&dsp.ADRS_REG);
				//ADRS_REG=(SHIFTED>>12)&0xFFF;
			}
			else
			{
				MOV_MEMTOEAX(&dsp.INPUTS);
				SAR_EAX(16);
				MOV_EAXTOMEM(&dsp.ADRS_REG);
				//ADRS_REG=(INPUTS>>16);
			}
		}

		if(ThisInst.EWT)
		{
			MOV_MEMTOEAX(&dsp.SHIFTED);
			SAR_EAX(8);
			ADD_AXTOMEM(&DSPData->EFREG[ThisInst.EWA]);
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
