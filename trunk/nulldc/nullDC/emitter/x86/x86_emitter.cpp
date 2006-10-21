//Emitting code ;)
#pragma warning(disable:4127)
#pragma warning(disable:4244)
#pragma warning(disable:4245)
#include "x86_emitter.h"
#include "x86_op_encoder.h"
#include "x86_matcher.h"
//x86_Label 
/*
//x86_ptr/x86_ptr_imm
x86_ptr x86_ptr::create(void* ptr)
{
	x86_ptr rv={ptr};
	return rv;
}*/
x86_ptr x86_ptr::create(unat ptr)
{
#pragma warning(disable:4312)
	x86_ptr rv(0);
	rv.ptr_int=ptr;
	return rv;
#pragma warning(default:4312) 
}
/*
x86_ptr_imm x86_ptr_imm::create(void* ptr)
{
	x86_ptr_imm rv={ptr};
	return rv;
}*/
x86_ptr_imm x86_ptr_imm::create(unat ptr)
{
#pragma warning(disable:4312) 
	x86_ptr_imm rv(0);
	rv.ptr_int=ptr;
	return rv;
#pragma warning(default:4312) 
}
//x86_block
//init things
/*
const char Names[op_count][64] =
{
	#include "generated_class_names_string.h"
};
*/

const char* DissasmClass(x86_opcode_class opcode)
{
	return "NoDissasm";//Names[opcode];
}
void x86_block::Init()
{
	x86_buff=0;
	x86_indx=0;
	x86_size=0;
}

//Generates code.if user_data is non zero , user_data_size bytes are allocated after the executable code
//and user_data is set to the first byte of em.Allways 16 byte alligned
void* x86_block::Generate(void** user_data,u32 user_data_size)
{
	if (user_data_size)
	{
		x86_indx+=32;
		x86_indx&=~31;
	}

	realloc(x86_buff,x86_indx+user_data_size);

	if (user_data_size)
		*user_data=(void*)&x86_buff[x86_indx];

	ApplyPatches(x86_buff);

	return &x86_buff[0];
}

//wut ?
void x86_block::ApplyPatches(u8* base)
{
	for (u32 i=0;i<patches.size();i++)
	{
		u8* dest=(u8*)patches[i].dest;

		u8* code_offset=base+patches[i].offset;
		u8* diff_offset=code_offset+(patches[i].type&0xF);

		if (patches[i].type&16)
			dest=patches[i].lbl->owner->x86_buff + patches[i].lbl->target_opcode;

		u32 diff=(u32)(dest-diff_offset);
		if ((patches[i].type&0xF)==1)
		{
			*code_offset=(u8)diff;
		}
		else if ((patches[i].type&0xF)==2)
		{
			*(u16*)code_offset=(u16)diff;
		}
		else if ((patches[i].type&0xF)==4)
		{
			*(u32*)code_offset=(u32)diff;
		}
	}

	patches.clear();
}
x86_block::~x86_block()
{
	//ensure everything is free'd :)
	Free();
}
//Will free any used resources exept generated code
void x86_block::Free()
{
	for (u32 i =0;i<labels.size();i++)
		delete labels[i];
	labels.clear();
}
void x86_block::x86_buffer_ensure(u32 size)
{
	if (this->x86_size<(size+x86_indx))
	{
		x86_size+=4096;
		x86_size*=2;
		x86_buff=(u8*)realloc(x86_buff,x86_size);
	}
}

//Label related code

//NOTE : Label position in mem must not chainge
void x86_block::CreateLabel(x86_Label* lbl,bool mark)
{
	memset(lbl,0xFFFFFFFF,sizeof(x86_Label));
	lbl->owner=this;
	lbl->marked=false;
	if (mark)
		MarkLabel(lbl);
}
//Allocate a label and create it :).Will be delete'd when calling free and/or dtor
x86_Label* x86_block::CreateLabel(bool mark)
{
	x86_Label* lbl = new x86_Label();
	CreateLabel(lbl,mark);
	labels.push_back(lbl);
	return lbl;
}
//Mark a label so that it points to next emitted opcode
void x86_block::MarkLabel(x86_Label* lbl)
{
	verify(lbl->marked==false);
	lbl->marked=true;
	lbl->target_opcode=x86_indx;
	//lbl->target_opcode=(u32)opcodes.size();
}
//opcode Emitters

x86_mrm c_mrm(x86_ptr mem)
{
	return x86_mrm::create(REG_NONE,mem);
}
//no param
void x86_block::Emit(x86_opcode_class op)
{
	ME_op_0(op);
}

//1 param
//reg
void x86_block::Emit(x86_opcode_class op,x86_reg reg)
{
	ME_op_1_nrm(op,reg);
}
//smrm
void x86_block::Emit(x86_opcode_class op,x86_ptr mem)
{
	Emit(op,c_mrm(mem));
}
//mrm
void x86_block::Emit(x86_opcode_class op,x86_mrm mrm)
{
	ME_op_1_nrm(op,mrm);
}
//imm
void x86_block::Emit(x86_opcode_class op,u32 imm)
{
	ME_op_1_imm(op,imm);
}
//ptr_imm
void x86_block::Emit(x86_opcode_class op,x86_ptr_imm disp)
{
	ME_op_1_nrm(op,disp);
}
//lbl
void x86_block::Emit(x86_opcode_class op,x86_Label* lbl)
{
	
	ME_op_1_nrm(op,lbl);
}

//2 param
//reg,reg	,reg1 is writen
void x86_block::Emit(x86_opcode_class op,x86_reg reg1,x86_reg reg2)
{
	ME_op_2_nrm(op,reg1,reg2);
}
//reg,smrm	,reg is writen
void x86_block::Emit(x86_opcode_class op,x86_reg reg,x86_ptr mem)
{
	Emit(op,reg,c_mrm(mem));
}
//reg,mrm	,reg is writen
void x86_block::Emit(x86_opcode_class op,x86_reg reg,x86_mrm mrm)
{
	ME_op_2_nrm(op,reg,mrm);
}
//reg,imm	,reg is writen
void x86_block::Emit(x86_opcode_class op,x86_reg reg,u32 imm)
{
	ME_op_2_imm(op,reg,imm);
}

//smrm,reg	,mem is writen
void x86_block::Emit(x86_opcode_class op,x86_ptr mem,x86_reg reg)
{
	Emit(op,c_mrm(mem),reg);
}
//smrm,imm	,mem is writen
void x86_block::Emit(x86_opcode_class op,x86_ptr mem,u32 imm)
{
	Emit(op,c_mrm(mem),imm);
}

//mrm,reg	,mrm is writen
void x86_block::Emit(x86_opcode_class op,x86_mrm mrm,x86_reg reg)
{
	ME_op_2_nrm(op,mrm,reg);
}
//mrm,imm	,mrm is writen
void x86_block::Emit(x86_opcode_class op,x86_mrm mrm,u32 imm)
{
	ME_op_2_imm(op,mrm,imm);
}

//3 param
//reg,reg,imm	,reg1 is writen
void x86_block::Emit(x86_opcode_class op,x86_reg reg1,x86_reg reg2,u32 imm)
{
	ME_op_3_imm(op,reg1,reg2,imm);
}

//reg,mrm,imm	,reg1 is writen
void x86_block::Emit(x86_opcode_class op,x86_reg reg,x86_ptr mem,u32 imm)
{
	ME_op_3_imm(op,reg,c_mrm(mem),imm);
}

//reg,mrm,imm	,reg1 is writen
void x86_block::Emit(x86_opcode_class op,x86_reg reg,x86_mrm mrm,u32 imm)
{
	ME_op_3_imm(op,reg,mrm,imm);
}

#define make_modrm(mod,rm) ( ((mod)<<6) | ((0)<<3) | (rm) )
#define make_sib(scale,index,base) ( ((scale)<<6) | ((index)<<3) | (base) )

u8 EncodeDisp(u32 disp,x86_mrm* to,u8 flags)
{
	//[reg+sdisp8] or [reg+sdisp32]
	//sdisp32 support olny for now , sdisp8 for later
	if (flags&2)
	{
		to->flags|=4;
		to->disp=disp;
		if (flags&4)
			return 0;
		else
			return make_modrm(2,0);
	}
	verify(false);
}
x86_mrm x86_mrm::create(x86_reg base)
{
	return x86_mrm::create(base,REG_NONE,sib_scale_1,0);
}
x86_mrm x86_mrm::create(x86_reg base,x86_ptr disp)
{
	return x86_mrm::create(base,REG_NONE,sib_scale_1,disp);
}
x86_mrm x86_mrm::create(x86_reg index,x86_sib_scale scale,x86_ptr disp)
{
	return x86_mrm::create(REG_NONE,index,scale,disp);
}
x86_mrm x86_mrm::create(x86_reg base,x86_reg index)
{
	return x86_mrm::create(base,index,sib_scale_1,0);
}

//NEEDS WORK
x86_mrm x86_mrm::create(x86_reg base,x86_reg index,x86_sib_scale scale,x86_ptr disp)
{
	x86_mrm rv;
	rv.flags=0;

	verify(index!=ESP);//cant be used

	if(index==REG_NONE)
	{
		//no index , ingore scale
		if (base==ESP)
		{
			//special encoding
			verify(false);
		}
		else if (base == EBP)
		{
			//special encoding
			//verify(false);
			rv.modrm = make_modrm(0,base);

			rv.modrm |= EncodeDisp(disp.ptr_int,&rv,3);	//32 or 16 bit disp
		}
		else if (base == REG_NONE)
		{
			//[disp32]
			//special encoding
			rv.modrm=make_modrm(0,EBP);			//EBP means disp
			EncodeDisp(disp.ptr_int,&rv,2);		//olny 32b disp alowed
		}
		else
		{
			//[reg] , [reg+disp8/32]
			rv.modrm = make_modrm(0,base);
			if (disp.ptr_int!=0)
			{
				rv.modrm |= EncodeDisp(disp.ptr_int,&rv,3);	//32 or 16 bit disp
			}
		}
	}
	else
	{
		//index ,scale [sib]
		rv.modrm = make_modrm(0,ESP);//ESP means sib
		rv.flags|=1;

		bool force_disp=0;
		u8 disp_sz=3;

		
		if (base==EBP)
			force_disp=1;
		
		if (base==REG_NONE)
		{
			rv.sib=make_sib(scale,index,EBP);
			disp_sz=2|4;//olny 32b disp , return 0 on mrm type
		}
		else
			rv.sib=make_sib(scale,index,base);

		if ( force_disp  || (disp.ptr_int!=0) )
		{
			rv.modrm |= EncodeDisp(disp.ptr_int,&rv,disp_sz);
		}
	}
	/*
	switch(mod)
	{
	case mod_RI:
		{
			//[reg]:reg{EAX,ECX,EDX,EBX,EBP,ESP,EDI,ESI}
			//EBP and ESP have special handling

			//reg gpr
			rv.flags=0;
		}
		break;
	case mod_RI_disp:
		{
			//[reg+disp]:reg{EAX,ECX,EDX,EBX,EBP,ESP,EDI,ESI}
			//ESP has special handling

			//disp!=0
			//reg gpr
			rv.flags=4;
		}
		break;
	case mod_DISP:
		{
			//[disp]
			//mod : 0
			//r/m : 5
			rv.flags=4;
			rv.modrm=(0<<6) | (5) | (0<<3);
			rv.disp=disp.ptr_int;
		}
		break;
	case mod_REG:
		{
			//reg:reg{EAX,ECX,EDX,EBX,EBP,ESP,EDI,ESI}
			rv.flags=0;
			rv.modrm=(3<<6) | (reg1) | (0<<3);
		}
		break;
	case mod_SIB:
		{
			//[reg1*scale+reg2]:
			//reg1{EAX,ECX,EDX,EBX,EBP,EDI,ESI}
			//scale{scale_1,scale_2,scale_4,scale_8} , if reg2==REG_NONE then scale_1 is invalid
			//reg2{EAX,ECX,EDX,EBX,ESP,EDI,ESI,REG_NONE}

			//reg1 gpr,!ESP
			verify(reg1!=ESP);
			//reg2 gpr,!EBP
			verify(reg2!=EBP);

			if (scale==sib_scale_1)
				verify(reg2!=REG_NONE);


			//We want simple SIB
			rv.modrm=(0<<6) | (4) | (0<<3);
			rv.sib = ( scale <<6) | (reg1<<3) | (reg2<<0);
			rv.flags=1;
		}
		break;
	case mod_SIB_disp:
		{
			//[reg1*scale+reg2+disp]:
			//reg1{EAX,ECX,EDX,EBX,EBP,EDI,ESI}
			//scale{scale_1,scale_2,scale_4,scale_8} , if reg2==REG_NONE then scale_1 is invalid
			//reg2{EAX,ECX,EDX,EBX,ESP,EDI,ESI,REG_NONE}

			//disp !=0
			//reg1 gpr,!ESP
			//reg2 gpr,!EBP
			if (scale==sib_scale_1)
				verify(reg2!=REG_NONE);
			//We want Complex SIB
			rv.modrm=(2<<6) | (4) | (0<<3);
			rv.sib = ( scale <<6) | (reg1<<3) | (reg2<<0);
			rv.disp=disp.ptr_int;
			rv.flags=1|4;
		}
		break;
	};
	*/
	/*
	rv.modrm=modrm;
	rv.sib=sib;
	rv.disp=disp;
	*/
	return rv;
}
