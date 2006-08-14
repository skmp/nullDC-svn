#include "shil_ce.h"
#include "dc\mem\sh4_mem.h"



#define shilh(name) bool __fastcall shil_ce_##name(shil_opcode* op,BasicBlock* bb,shil_stream* il)

typedef shilh(FP);

shilh(nimp)
{
	//lol ?
	return false;
}

shil_ce_FP* shil_ce_lut[shil_count]=
{
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,
	shil_ce_nimp,shil_ce_nimp,shil_ce_nimp,shil_ce_nimp
};
void SetShilHanlder(shil_opcodes op,shil_ce_FP* ha)
{
	if (op>(shil_count-1))
	{
		printf("SHIL COMPILER ERROR\n");
	}
	if (shil_ce_lut[op]!=shil_ce_nimp)
	{
		printf("SHIL COMPILER ERROR [hash table overwrite]\n");
	}

	shil_ce_lut[op]=ha;
}
shilh(adc);
shilh(add);
shilh(and);
shilh(cmp);
shilh(fabs);
shilh(fadd);
shilh(fdiv);
shilh(fmac);

shilh(fmul);
shilh(fneg);
shilh(fsub);
shilh(LoadT);
shilh(mov);
shilh(movex);
shilh(neg);
shilh(not);

shilh(or);
shilh(rcl);
shilh(rcr);
shilh(readm);
shilh(rol);
shilh(ror);
shilh(sar);
shilh(SaveT);

shilh(shil_ifb);
shilh(shl);
shilh(shr);
shilh(sub);
shilh(swap);
shilh(test);
shilh(writem);
shilh(xor);
shilh(jcond);
shilh(jmp);
shilh(mul);

shilh(ftrv);
shilh(fsqrt);
shilh(fipr);
shilh(floatfpul);
shilh(ftrc);
shilh(fsca);
shilh(fsrra);
shilh(div32);
shilh(fcmp);

bool Inited_ce_pass=false;
struct RegData
{
	u32 RegValue;//Current reg value (if IsConstant is true)
	u32 RB_value;//last value writen back onto the register (exists olny of IsRB is true)

	bool IsConstant;
	bool IsRB;
};

RegData shil_ce_gpr[160];

void Init_ce()
{
	memset(shil_ce_gpr,0,sizeof(shil_ce_gpr));
	if (Inited_ce_pass)
		return;

	Inited_ce_pass=true;
	//
	SetShilHanlder(shil_opcodes::adc,shil_ce_adc);
	SetShilHanlder(shil_opcodes::add,shil_ce_add);
	SetShilHanlder(shil_opcodes::and,shil_ce_and);
	SetShilHanlder(shil_opcodes::cmp,shil_ce_cmp);
	SetShilHanlder(shil_opcodes::fabs,shil_ce_fabs);
	SetShilHanlder(shil_opcodes::fadd,shil_ce_fadd);
	SetShilHanlder(shil_opcodes::fdiv,shil_ce_fdiv);
	SetShilHanlder(shil_opcodes::fmac,shil_ce_fmac);

	SetShilHanlder(shil_opcodes::fmul,shil_ce_fmul);
	SetShilHanlder(shil_opcodes::fneg,shil_ce_fneg);
	SetShilHanlder(shil_opcodes::fsub,shil_ce_fsub);
	SetShilHanlder(shil_opcodes::LoadT,shil_ce_LoadT);
	SetShilHanlder(shil_opcodes::mov,shil_ce_mov);
	SetShilHanlder(shil_opcodes::movex,shil_ce_movex);
	SetShilHanlder(shil_opcodes::neg,shil_ce_neg);
	SetShilHanlder(shil_opcodes::not,shil_ce_not);

	SetShilHanlder(shil_opcodes::or,shil_ce_or);
	SetShilHanlder(shil_opcodes::rcl,shil_ce_rcl);
	SetShilHanlder(shil_opcodes::rcr,shil_ce_rcr);
	SetShilHanlder(shil_opcodes::readm,shil_ce_readm);
	SetShilHanlder(shil_opcodes::rol,shil_ce_rol);
	SetShilHanlder(shil_opcodes::ror,shil_ce_ror);
	SetShilHanlder(shil_opcodes::sar,shil_ce_sar);
	SetShilHanlder(shil_opcodes::SaveT,shil_ce_SaveT);

	SetShilHanlder(shil_opcodes::shil_ifb,shil_ce_shil_ifb);
	SetShilHanlder(shil_opcodes::shl,shil_ce_shl);
	SetShilHanlder(shil_opcodes::shr,shil_ce_shr);
	SetShilHanlder(shil_opcodes::sub,shil_ce_sub);
	SetShilHanlder(shil_opcodes::swap,shil_ce_swap);
	SetShilHanlder(shil_opcodes::test,shil_ce_test);
	SetShilHanlder(shil_opcodes::writem,shil_ce_writem);
	SetShilHanlder(shil_opcodes::xor,shil_ce_xor);
	SetShilHanlder(shil_opcodes::jcond,shil_ce_jcond);
	SetShilHanlder(shil_opcodes::jmp,shil_ce_jmp);
	SetShilHanlder(shil_opcodes::mul,shil_ce_mul);

	SetShilHanlder(shil_opcodes::ftrv,shil_ce_ftrv);
	SetShilHanlder(shil_opcodes::fsqrt,shil_ce_fsqrt);
	SetShilHanlder(shil_opcodes::fipr,shil_ce_fipr);
	SetShilHanlder(shil_opcodes::floatfpul,shil_ce_floatfpul);
	SetShilHanlder(shil_opcodes::ftrc,shil_ce_ftrc);
	SetShilHanlder(shil_opcodes::fsca,shil_ce_fsca);
	SetShilHanlder(shil_opcodes::fsrra,shil_ce_fsrra);
	SetShilHanlder(shil_opcodes::div32,shil_ce_div32);
	SetShilHanlder(shil_opcodes::fcmp,shil_ce_fcmp);
}

void ce_die(char* reason)
{
	if (reason)
		printf("C.E. pass : die [%s]\n",reason);
	else
		printf("C.E. pass : die\n");

	__asm int 3;
}
bool ce_CanBeConst(u8 reg)
{
	return (reg<16) || (reg==reg_pc_temp);
}

bool ce_IsConst(u8 reg)
{
	if (ce_CanBeConst(reg))
		return shil_ce_gpr[reg].IsConstant;
	else
		return false;
}

u32 ce_GetConst(u8 reg)
{
	if (ce_IsConst(reg))
		return shil_ce_gpr[reg].RegValue;
	else
		ce_die("ce_GetConst : can't get const when reg is not const");
}
void ce_SetConst(u8 reg,u32 value)
{
	if (ce_IsConst(reg))
	{
		shil_ce_gpr[reg].RegValue=value;
	}
	else
		ce_die("ce_SetConst : can't set const when reg is not const");
}
void ce_MakeConst(u8 reg,u32 value)
{
	if (ce_CanBeConst(reg))
	{
		if (shil_ce_gpr[reg].IsConstant==false)
		{
			shil_ce_gpr[reg].IsConstant=true;
			shil_ce_gpr[reg].RegValue=value;
			shil_ce_gpr[reg].IsRB=false;
		}
		else
		{
			shil_ce_gpr[reg].RegValue=value;
		}
	}
	else
		ce_die("ce_MakeConst : can't create const when reg can't be const.tracked");
}
void ce_KillConst(u8 reg)
{
	if (ce_IsConst(reg))
	{
		shil_ce_gpr[reg].IsConstant=false;
		shil_ce_gpr[reg].IsRB=false;
	}
}
bool ce_FindExistingConst(u32 value,u8* reg_num)
{
	for (u8 i=0;i<160;i++)
	{
		if (ce_IsConst(i))
		{
			if ((shil_ce_gpr[i].IsRB==true) && shil_ce_gpr[i].RB_value==value)
			{
				*reg_num=i;
				return true;
			}
		}
	}
	return false;
}

void ce_WriteBack(u8 reg,shil_stream* il)
{
	if (ce_IsConst(reg))
	{
		if ((shil_ce_gpr[reg].IsRB==false) || (shil_ce_gpr[reg].RegValue!=shil_ce_gpr[reg].RB_value))
		{
			u8 aliased_reg;
			u32 rv=ce_GetConst(reg);
			if (ce_FindExistingConst(rv,&aliased_reg))
			{
				il->mov((Sh4RegType)reg,(Sh4RegType)aliased_reg);
			}
			else
			{
				il->mov((Sh4RegType)reg,ce_GetConst(reg));
			}
			shil_ce_gpr[reg].RB_value=ce_GetConst(reg);
			shil_ce_gpr[reg].IsRB=true;
		}
	}
}

void ce_WriteBack_aks(u8 reg,shil_stream* il)
{
	ce_WriteBack(reg,il);
	ce_KillConst(reg);
}
bool ce_re_run;
//Optimisation pass mainloop
u32 shil_optimise_pass_ce_main(BasicBlock* bb)
{
	bool rv=false;
	ce_re_run=false;
	u32 opt=0;

	Init_ce();

	shil_stream il;

	size_t old_Size=bb->ilst.opcodes.size();

	bool old_re_run=ce_re_run;
	for (size_t i=0;i<bb->ilst.opcodes.size();i++)
	{
		shil_opcode* op=&bb->ilst.opcodes[i];

		old_re_run=ce_re_run;
		if (shil_ce_lut[op->opcode](op,bb,&il)==false)
			il.opcodes.push_back(*op);//emit the old opcode
		else
		{
			opt++;
			rv=true;
		}
	}

	if (old_re_run!=ce_re_run)
		ce_re_run=false;

	if (rv)
	{
		bb->ilst.opcodes.clear();
		
		for (size_t i=0;i<il.opcodes.size();i++)
			bb->ilst.opcodes.push_back(il.opcodes[i]);
		
		//no need to write back reg_pc_temp , it not used after block [its olny a temp reg]:)
		for (u8 i=0;i<16;i++)
		{
			ce_WriteBack_aks(i,&bb->ilst);
		}
		bb->ilst.op_count=(u32)bb->ilst.opcodes.size();
	}

	if (old_Size!=bb->ilst.opcodes.size())
		ce_re_run = true;

	return opt;
}
u32 shil_optimise_pass_btp_main(BasicBlock* bb);
u64 total_ops_removed=0;
void shil_optimise_pass_ce_driver(BasicBlock* bb)
{
	ce_re_run=true;
	u32 rv=0;
	u32 pass=0;
	size_t old_Size=bb->ilst.opcodes.size();

	void CompileBasicBlock_slow_c(BasicBlock* block,u32 pass);
	//CompileBasicBlock_slow_c(bb,0);
	while(ce_re_run)
	{
		rv+=shil_optimise_pass_ce_main(bb);
		pass++;
		if (pass>10)
			break;
		//CompileBasicBlock_slow_c(bb,pass);
	}

	total_ops_removed+=old_Size-bb->ilst.opcodes.size();
	shil_optimise_pass_btp_main(bb);
	//if (rv)
	//	printf("Optimised block 0x%X , %d opts : %d passes ,delta=%d, total removed %d \n",bb->start,rv,pass,old_Size-bb->ilst.opcodes.size(),total_ops_removed);

}
//default thing to do :p
void DefHanlder(shil_opcode* op,BasicBlock* bb,shil_stream* il)
{
	for (u8 i=0;i<16;i++)
	{
		if (ce_IsConst(i))
		{
			if (op->ReadsReg((Sh4RegType)i))
			{
				ce_WriteBack(i,il);
			}
			if (op->WritesReg((Sh4RegType)i))
			{
				ce_KillConst(i);
			}
		}
	}
}
//optimisation hanlders ;)
#define NormBinaryOp(oppstrrr)\
if ((op->flags & FLAG_REG2) && (ce_IsConst(op->reg2)))\
	{\
		op->imm1=ce_GetConst(op->reg2);\
		op->flags|=FLAG_IMM1;\
		op->flags&=~FLAG_REG2;\
		ce_re_run=true;\
	}\
	\
	if (ce_IsConst(op->reg1))\
	{\
		if ((op->flags & FLAG_IMM1))\
		{\
			ce_SetConst(op->reg1,ce_GetConst(op->reg1) oppstrrr op->imm1);\
			ce_re_run=true;\
			return true;\
		}\
		else\
		{\
			ce_WriteBack_aks(op->reg1,il);\
		}\
	}\
	else\
	{\
	}\

	//ce_WriteBack_aks(op->reg1,il);
	//	ce_WriteBack(op->reg2,il);
		//reg1 has to be writen back , and its no more const 
		//-> its not const to start with :p not realy needed here
		//----*----\
		//reg2 has to be writen back , but it will remain constant ;)
		//-> not needed , if const , the reg1,imm1 form is used
shilh(adc)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(add)
{
	//DefHanlder(op,bb,il);
	NormBinaryOp(+);
	return false;
}
shilh(and)
{
	//DefHanlder(op,bb,il);
	NormBinaryOp(&);
	return false;
}
shilh(cmp)
{
	DefHanlder(op,bb,il);
	return false;
}

shilh(LoadT)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(mov)
{
	bool rv=false;

	if ((op->flags & FLAG_REG2) &&  op->reg1==op->reg2)
	{
		ce_re_run=true;
		return true;
	}

	if ((op->flags & FLAG_REG2) &&  ce_IsConst(op->reg2))
	{
		op->flags&=~FLAG_REG2;
		op->flags|=FLAG_IMM1;
		op->imm1 = ce_GetConst(op->reg2);
		ce_re_run=true;
	}

	if (op->flags & FLAG_IMM1)
	{	//reg1=imm1
		//reg1 gets a known value
		if (ce_CanBeConst(op->reg1))
		{
			ce_MakeConst(op->reg1,op->imm1);
			rv=true;
		}
		else
		{
			ce_KillConst(op->reg1);
		}
	}
	else
	{
		//reg1 gets an unkown value
		ce_WriteBack(op->reg2,il);
		ce_KillConst(op->reg1);
	}

	return rv;
}
shilh(movex)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(neg)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(not)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(or)
{
	//DefHanlder(op,bb,il);
	NormBinaryOp(|);
	return false;
}
shilh(rcl)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(rcr)
{
	DefHanlder(op,bb,il);
	return false;
}
//ReadMem 
u32 GetRamReadAdr(shil_opcode* op)
{
	if ((op->flags & (FLAG_R0|FLAG_GBR|FLAG_REG2))==0)
	{//[imm] form
		if (IsOnRam(op->imm1))
		{
			return op->imm1;
		}
	}
	return 0xFFFFFFFF;
}
bool ce_ReadWriteParams(shil_opcode* op)
{
	//return false;
	bool rv=false;

	if (op->flags & FLAG_REG2)
	{
		if (ce_IsConst(op->reg2))
		{
			u32 vl=ce_GetConst(op->reg2);
			if (op->flags & FLAG_IMM1)
				op->imm1+=vl;
			else
				op->imm1=vl;

			rv=true;
			op->flags |= FLAG_IMM1;
			op->flags&=~FLAG_REG2;
		}
	}
	if (op->flags & FLAG_R0)
	{
		if (ce_IsConst(r0))
		{
			u32 vl=ce_GetConst(r0);
			if (op->flags & FLAG_IMM1)
				op->imm1+=vl;
			else
				op->imm1=vl;

			rv=true;
			op->flags |= FLAG_IMM1;
			op->flags&=~FLAG_R0;
		}
	}
	if (op->flags & FLAG_GBR)
	{
		if (ce_IsConst(reg_gbr))
		{
			u32 vl=ce_GetConst(reg_gbr);
			if (op->flags & FLAG_IMM1)
				op->imm1+=vl;
			else
				op->imm1=vl;

			rv=true;
			op->flags |= FLAG_IMM1;
			op->flags&=~FLAG_GBR;
		}
	}
	
	return rv;
}
shilh(readm)
{
	bool rv=ce_ReadWriteParams(op);

	u32 addr=GetRamReadAdr(op);
	if (addr!=0xFFFFFFFF && bb->IsMemLocked(addr))
	{
		u32 size=op->flags&3;
		u32 data=0;
		if (size==FLAG_64)
			__asm int 3;
		if (size==0)
		{
			data=(u32)(s32)(s8)ReadMem8(addr);
		}
		else if (size==1)
		{
			data=(u32)(s32)(s16)ReadMem16(addr);
		}
		else
		{
			data=ReadMem32(addr);
		}

		if (ce_CanBeConst(op->reg1))
		{
			ce_MakeConst(op->reg1,data);
		}
		else
			il->mov((Sh4RegType)op->reg1,data);

		ce_re_run=true;
		return true;
	}

	//even if we did optimise smth , a readback may be needed
	//since it uses opcode flags to decode what to write back , it should't cause any non needed write backs ;)
	DefHanlder(op,bb,il);

	//we did chainge smth :0
	if (rv)//we optimised something , re run the ce pass once more
		ce_re_run=true;

	return false;
}
shilh(writem)
{
	bool rv=ce_ReadWriteParams(op);
	//even if we did optimise smth , a readback may be needed
	//since it uses opcode flags to decode what to write back , it should't cause any non needed write backs ;)
	DefHanlder(op,bb,il);

	//we did chainge smth :0
	if (rv)//we optimised something , re run the ce pass once more
		ce_re_run=true;

	return false;
}
shilh(rol)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(ror)
{
	DefHanlder(op,bb,il);
	return false;
}
//uses flags from previus opcode
shilh(SaveT)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(shil_ifb)
{
	for (u8 i=0;i<16;i++)
		ce_WriteBack_aks(i,il);
	return false;
}
//sets T if imm ==1
shilh(sar)
{
	if (ce_IsConst(op->reg1) && (op->imm1>1))
	{
		ce_SetConst(op->reg1,(u32)(((s32)ce_GetConst(op->reg1))>>(op->imm1)));
		return true;
	}
	else
		DefHanlder(op,bb,il);
	return false;
}
//sets T if imm ==1
shilh(shl)
{
	if (ce_IsConst(op->reg1) && (op->imm1>1))
	{
		ce_SetConst(op->reg1,ce_GetConst(op->reg1)<<(op->imm1));
		return true;
	}
	else
		DefHanlder(op,bb,il);
	return false;
}
//sets T if imm ==1
shilh(shr)
{
	if (ce_IsConst(op->reg1) && (op->imm1>1))
	{
		ce_SetConst(op->reg1,ce_GetConst(op->reg1)>>(op->imm1));
		return true;
	}
	else
		DefHanlder(op,bb,il);
	return false;
}
//sets T if used as DT
shilh(sub)
{
	//NormBinaryOp(-);
	//that's all we can do for now
	//DT uses sub , and it needs flags to be ok after it
	if ((op->flags & FLAG_REG2) && (ce_IsConst(op->reg2)))
	{
		op->imm1=ce_GetConst(op->reg2);
		op->flags|=FLAG_IMM1;
		op->flags&=~FLAG_REG2;
	}
	
	DefHanlder(op,bb,il);
	return false;
}
shilh(swap)
{
	if (ce_IsConst(op->reg1))
	{
		u32 size=op->flags&3;
		if (size==FLAG_8)
		{
			u32 ov=ce_GetConst(op->reg1);
			ov=(ov & 0xFFFF0000) | ((ov&0xFF)<<8) | ((ov>>8)&0xFF);
			ce_SetConst(op->reg1,ov);
		}
		else
		{
			u32 ov=ce_GetConst(op->reg1);
			ov=(ov>>16)|(ov<<16);
			ce_SetConst(op->reg1,ov);
		}
		
		return true;
	}
	else
		DefHanlder(op,bb,il);
	return false;
}
//sets T
shilh(test)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(xor)
{
	NormBinaryOp(^);
	return false;
}
shilh(jcond)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(jmp)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(mul)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(div32)
{
	/*if (ce_IsConst((u8)op->imm1))
	{
		il->mov((Sh4RegType)(u8)op->imm1,ce_GetConst((u8)op->imm1));
	}*/
	ce_WriteBack_aks(op->reg1,il);
	ce_WriteBack_aks(op->reg2,il);
	ce_WriteBack_aks((u8)op->imm1,il);
	DefHanlder(op,bb,il);
	return false;
}
shilh(fcmp)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fabs)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fadd)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fdiv)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fmac)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fmul)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fneg)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fsub)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(ftrv)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fsqrt)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fipr)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(floatfpul)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(ftrc)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fsca)
{
	DefHanlder(op,bb,il);
	return false;
}
shilh(fsrra)
{
	DefHanlder(op,bb,il);
	return false;
}

//works olny of moves
//a very limited form of dce pre scan ;)
bool backscan_const(BasicBlock* bb,u8 reg,u32* rv)
{
	u32 lop=bb->ilst.op_count;
	shil_stream* ilst=&bb->ilst;

	//look if reg takes a const value
	while (lop)
	{
		lop--;
		shil_opcode* op=&ilst->opcodes[lop];
		//if move (we can take const value olny from here for now)
		if (op->opcode==shil_opcodes::mov && op->reg1==reg)
		{
			//if the reg we want became a const , were finished :D
			if (op->flags & FLAG_IMM1)
			{
				*rv=op->imm1;
				return true;
			}
			else
			{
				//if its a reg 2 reg move , we alias the old reg w/ the one that replaced it ;)
				verify(op->flags & FLAG_REG2);
				reg=op->reg2;
			}
		}
		else
		{
			//we get writen an unkown value , unable to fix it up :p
			if (op->WritesReg((Sh4RegType)reg))
				return false;
		}
	}

	//we failed to find a const on the entire block
	return false;
}

u32 shil_optimise_pass_btp_main(BasicBlock* bb)
{
	bb->flags.DisableHS=1;
	if (bb->flags.PerformModeLookup)
		return 0;

	if ((bb->flags.ExitType==BLOCK_EXITTYPE_DYNAMIC) ||
		(bb->flags.ExitType==BLOCK_EXITTYPE_DYNAMIC_CALL))
	{
		u32 new_cv=0;
		if (backscan_const(bb,reg_pc,&new_cv))
		{
			//printf("Block promote 0x%X , from DYNAMIC to FIXED exit 0x%X\n",bb->start,new_cv);
			bb->TF_next_addr=new_cv;
			if (bb->flags.ExitType=BLOCK_EXITTYPE_DYNAMIC)
				bb->flags.ExitType=BLOCK_EXITTYPE_FIXED;
			else
				bb->flags.ExitType=BLOCK_EXITTYPE_FIXED_CALL;
		}
	}

	return 1;
}