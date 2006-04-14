#include "rec_v0_recompiler.h"
#include "emmiter/emmiter.h"
#include "dc/mem/sh4_mem.h"
#include "dc/sh4/sh4_opcode_list.h"
#include "dc/sh4/sh4_registers.h"
#undef sh4op

//simple recompiler
//to test up some register allocation code , ect

// test recompiler stuff

//block manager - need to be writen
//emitters - need to be writen
//opcode handlers - need to be writen
//register structs/class - need to be writen

//this will be a very simple recompiler 
//no register allocation , no sse , no mmx , and generaly no other optimisations
//the idea is to test my idea to warp registers onto structs ect

#define SH4_REC

Emmiter<>* CodeGen;

#define sh4op(str) void  __fastcall rec_##str (u32 op,u32 pc)

#define GetN(str) ((str>>8) & 0xf)
#define GetM(str) ((str>>4) & 0xf)
#define GetImm4(str) ((str>>0) & 0xf)
#define GetImm8(str) ((str>>0) & 0xff)
#define GetSImm8(str) ((s8)((str>>0) & 0xff))
#define GetImm12(str) ((str>>0) & 0xfff)
#define GetSImm12(str) (((s16)((GetImm12(str))<<4))>>3)

//#define tmu_underflow  0x0100
#define iNimp(op,info) rec_iNimp(pc,op,info)

void rec_iNimp(u32 pc,u32 op, char* info)
{
	printf("not implemented opcode : %X : ", op);
	printf(info);
	printf(" @ %X\n", pc);
}



struct RecRegType
{
	bool IsConst;
	u32 ConstValue;
	u32* RegLoc;

	void FlushKill()
	{
		if (IsConst)
		{
			IsConst=false;
			CodeGen->MOV32ItoM(RegLoc,ConstValue);
		}
	}
	void LoadTo(x86IntRegType to) const
	{
		if (!IsConst)
			CodeGen->MOV32MtoR(to,RegLoc);
		else
			CodeGen->MOV32ItoR(to,ConstValue);
	}
	void SaveFrom(x86IntRegType from)
	{
		IsConst=false;
		CodeGen->MOV32RtoM(RegLoc,from);
	}
	void SaveFrom(u32 from)
	{
		IsConst=true;
		ConstValue=from;
	}

	//SUB
	void operator-=(const u32 constv)
	{
		if (IsConst)//const,const : result remains const
		{
			ConstValue-=constv;
		}
		else//reg,const : result remains non-const
		{
			CodeGen->SUB32ItoM(RegLoc,constv);
		}
	};
	void operator-=(const RecRegType& rhs)
    {
       	if (rhs.IsConst)//x , const
		{
			(*this)-=rhs.ConstValue;
		}
		else
		{
			//reg,reg	: result remains non-const
			//const,reg	: result remains non-const
			LoadTo(EAX);
			rhs.LoadTo(ECX);

			CodeGen->SUB32RtoR(EAX,ECX);

			SaveFrom(EAX);
		}
    }

	void operator--(int wtf)
	{
		(*this)-=1;
	}
	//ADD
	void operator+=(const u32 constv)
	{
		if (IsConst)//const,const : result remains const
		{
			ConstValue+=constv;
		}
		else//reg,const : result remains non-const
		{
			CodeGen->ADD32ItoM(RegLoc,constv);
		}
	};
	void operator+=(const RecRegType& rhs)
        {
       	if (rhs.IsConst)//x , const
		{
			(*this)+=rhs.ConstValue;
		}
		else
		{
			//reg,reg	: result remains non-const
			//const,reg	: result remains non-const
			LoadTo(EAX);
			rhs.LoadTo(ECX);

			CodeGen->ADD32RtoR(EAX,ECX);

			SaveFrom(EAX);
		}
    }
	void operator++(int wtf)
	{
		(*this)+=1;
	}
	//MOVS
	void operator=(const u32 constv)
    {
		 SaveFrom(constv);
    }
	void operator=(const s32 constv)
    {
		(*this)=(u32)constv;
    }
	void operator=(const RecRegType& rhs)
    {
		if (rhs.IsConst)//becomes const
		{
			(*this)=rhs.ConstValue;
		}
		else			//becomes non-const
		{
			rhs.LoadTo(EAX);
			SaveFrom(EAX);
		}
    }
	//AND
	void operator&=(const u32 constv)
	{
		if (IsConst)//const calc ;P
		{
			ConstValue&=constv;
		}
		else		//on mem
		{
			CodeGen->AND32ItoM(RegLoc,constv);
		}
	};
	void operator&=(const RecRegType& reg)
	{
		if (reg.IsConst)
		{
			(*this)&=reg.ConstValue;
		}
		else
		{
			reg.LoadTo(EAX);
			if (IsConst)
			{	//To r and save , no longer const
				CodeGen->AND32ItoR(EAX,ConstValue);
				SaveFrom(EAX);
			}
			else
			{
				//to m
				CodeGen->AND32RtoM(RegLoc,EAX);
			}
		}
	};
	//OR
	void operator|=(const u32 constv)
	{
		if (IsConst)//const calc ;P
		{
			ConstValue|=constv;
		}
		else		//on mem
		{
			CodeGen->OR32ItoM(RegLoc,constv);
		}
	};
	void operator|=(const RecRegType& reg)
	{
		if (reg.IsConst)
		{
			(*this)|=reg.ConstValue;
		}
		else
		{
			reg.LoadTo(EAX);
			if (IsConst)
			{	//To r and save , no longer const
				CodeGen->OR32ItoR(EAX,ConstValue);
				SaveFrom(EAX);
			}
			else
			{
				//to m
				CodeGen->OR32RtoM(RegLoc,EAX);
			}
		}
	};
	//XOR
	void operator^=(const u32 constv)
	{
		if (IsConst)//const calc ;P
		{
			ConstValue^=constv;
		}
		else		//on mem
		{
			CodeGen->XOR32ItoM(RegLoc,constv);
		}
	};
	void operator^=(const RecRegType& reg)
	{
		if (reg.IsConst)
		{
			(*this)^=reg.ConstValue;
		}
		else
		{
			reg.LoadTo(EAX);
			if (IsConst)
			{	//To r and save , no longer const
				CodeGen->XOR32ItoR(EAX,ConstValue);
				SaveFrom(EAX);
			}
			else
			{
				//to m
				CodeGen->XOR32RtoM(RegLoc,EAX);
			}
		}
	};
	//SHIFT RIGHT
	void operator>>=(const u32 constv)
	{
		if (IsConst)
		{
			ConstValue>>=constv;
		}
		else
		{
			if (constv)
				CodeGen->SHR32ItoM(RegLoc,(u8)constv);
		}
	};
	void operator>>=(const RecRegType& reg)
	{
		if (reg.IsConst)
			(*this)>>=reg.ConstValue;
		else
		{
			//hmm?
		}
	};
	//SHIFT LEFT
	void operator<<=(const u32 constv)
	{
		if (IsConst)
		{
			ConstValue<<=constv;
		}
		else
		{
			if (constv)
				CodeGen->SHL32ItoM(RegLoc,(u8)constv);
		}
	};
	void operator<<=(const RecRegType& reg)
	{
		if (reg.IsConst)
			(*this)<<=reg.ConstValue;
		else
		{
			//hmm?
		}
	};
};
//hm?
struct RecSrType
{
	RecRegType full;
};

struct RecFpscrType
{
	RecRegType full;
};

//temp types that hold data
RecRegType rec_r[16];
RecRegType rec_r_bank[8];

RecRegType rec_gbr,rec_ssr,rec_spc,rec_sgr,rec_dbr,rec_vbr;
RecRegType rec_mach,rec_macl,rec_pr,rec_fpul;

RecSrType rec_sr;

RecFpscrType rec_fpscr;
//TODO : fixup readmem's on interpreter around macros so that i can use em on recompiler
//TODO : fixup sr type

void FlushKillAll()
{
	for (int i=0;i<8;i++)
	{
		rec_r[i].FlushKill();
		rec_r_bank[i].FlushKill();
	}

	for (int i=8;i<16;i++)
	{
		rec_r[i].FlushKill();
	}

	rec_gbr.FlushKill();
	rec_ssr.FlushKill();
	rec_spc.FlushKill();
	rec_sgr.FlushKill();
	rec_dbr.FlushKill();
	rec_vbr.FlushKill();

	rec_mach.FlushKill();
	rec_macl.FlushKill();
	rec_pr.FlushKill();
	rec_fpul.FlushKill();
	rec_sr.full.FlushKill();
	rec_fpscr.full.FlushKill();
}

//Read Mem macros
#define ReadMemU32(to,addr)					ReadMemRec(to,addr,0,4)//to=ReadMem32(addr)
#define ReadMemS16(to,addr)					ReadMemRecS(to,addr,0,2)// to=(u32)(s32)(s16)ReadMem16(addr)
#define ReadMemS8(to,addr)					ReadMemRecS(to,addr,0,1)//to=(u32)(s32)(s8)ReadMem8(addr)

//Base,offset format
#define ReadMemBOU32(to,addr,offset)		ReadMemRec(to,addr,offset,4)//ReadMemU32(to,addr+offset)
#define ReadMemBOS16(to,addr,offset)		ReadMemRecS(to,addr,offset,2)//ReadMemS16(to,addr+offset)
#define ReadMemBOS8(to,addr,offset)			ReadMemRecS(to,addr,offset,1)//ReadMemS8(to,addr+offset)

//Write Mem Macros
#define WriteMemU32(addr,data)				WriteMemRec(addr,0,data,4)//WriteMem32(addr,(u32)data)
#define WriteMemU16(addr,data)				WriteMemRec(addr,0,data,2)//WriteMem16(addr,(u16)data)
#define WriteMemU8(addr,data)				WriteMemRec(addr,0,data,1)//WriteMem8(addr,(u8)data)

//Base,offset format
#define WriteMemBOU32(addr,offset,data)		WriteMemRec(addr,offset,data,4)//WriteMemU32(addr+offset,data)
#define WriteMemBOU16(addr,offset,data)		WriteMemRec(addr,offset,data,2)//WriteMemU16(addr+offset,data)
#define WriteMemBOU8(addr,offset,data)		WriteMemRec(addr,offset,data,1)//WriteMemU8(addr+offset,data)
INLINE bool IsOnRam(u32 addr)
{
	if (((addr>>26)&0x7)==3)
	{
		if ((((addr>>29) &0x7)!=7))
		{
			return true;
		}
	}

	return false;
}
void ReadMemRec(RecRegType &to,u32 addr,u32 offset,u32 sz)
{
	//do some magic :P
	//TODO : optimise w/ static reads more
	u32 ReadAddr=addr+offset;
	if (IsOnRam(ReadAddr))
	{
		if(sz==4)
		{
			CodeGen->MOV32MtoR(EAX,(u32*)GetMemPtr(ReadAddr,sz));
		}
		else
			printf("ReadMemRec , wrong sz param\n");
	}
	else
	{
		CodeGen->MOV32ItoR(ECX,ReadAddr);

		if(sz==4)
			CodeGen->CALLFunc(ReadMem32);//bwhahaha
		else
			printf("ReadMemRec , wrong sz param\n");
	}

	to.SaveFrom(EAX);	//save the result
}

void ReadMemRec(RecRegType &to,RecRegType& addr,u32 offset,u32 sz)
{
	//do some magic :P
	if (addr.IsConst)
	{
		ReadMemRec(to,addr.ConstValue,offset,sz);
		return;
	}

	addr.LoadTo(ECX);

	//this could be done on ADD too ;)
	if (offset)
		CodeGen->ADD32ItoR(ECX,offset);

	if(sz==4)
		CodeGen->CALLFunc(ReadMem32);//bwhahaha
	else
		printf("ReadMemRec , wrong sz param\n");

	to.SaveFrom(EAX);	//save the result
}



void ReadMemRec(RecRegType &to,RecRegType& addr,RecRegType& offset,u32 sz)
{
	if (offset.IsConst)
	{
		ReadMemRec(to,addr,offset.ConstValue,sz);
		return;
	}

	offset.LoadTo(ECX);
	if (addr.IsConst)
	{
		//this could be done on ADD too ;)
		if (addr.ConstValue)
			CodeGen->ADD32ItoR(ECX,addr.ConstValue);
	}
	else
	{
		CodeGen->ADD32MtoR(ECX,addr.RegLoc);
	}

	if(sz==4)
		CodeGen->CALLFunc(ReadMem32);//bwhahaha
	else
		printf("ReadMemRec , wrong sz param\n");

	to.SaveFrom(EAX);	//save the result
}

//signed
void ReadMemRecS(RecRegType &to,u32 addr,u32 offset,u32 sz)
{
	//do some magic :P
	//TODO : optimise w/ static reads more
	u32 ReadAddr=addr+offset;
	if (IsOnRam(ReadAddr))
	{
		if (sz==1)
		{
			CodeGen->MOVSX32M8toR(EAX,GetMemPtr(ReadAddr,sz));
		}
		else if (sz==2)
		{
			CodeGen->MOVSX32M16toR(EAX,(u16*)GetMemPtr(ReadAddr,sz));
		}
		else
			printf("ReadMemRecS , wrong sz param\n");
	}
	else
	{
		CodeGen->MOV32ItoR(ECX,ReadAddr);

		if (sz==1)
		{
			CodeGen->CALLFunc(ReadMem8);//bwhahaha
			CodeGen->MOVSX32R8toR(EAX,EAX);
		}
		else if (sz==2)
		{
			CodeGen->CALLFunc(ReadMem16);//bwhahaha
			CodeGen->MOVSX32R16toR(EAX,EAX);
		}
		else
			printf("ReadMemRecS , wrong sz param\n");
	}

	to.SaveFrom(EAX);	//save the result
}

void ReadMemRecS(RecRegType &to,RecRegType& addr,u32 offset,u32 sz)
{
	//do some magic :P
	if (addr.IsConst)
	{
		ReadMemRecS(to,addr.ConstValue,offset,sz);
		return;
	}

	addr.LoadTo(ECX);

	//this could be done on ADD too ;)
	if (offset)
		CodeGen->ADD32ItoR(ECX,offset);

	if (sz==1)
	{
		CodeGen->CALLFunc(ReadMem8);//bwhahaha
		CodeGen->MOVSX32R8toR(EAX,EAX);
	}
	else if (sz==2)
	{
		CodeGen->CALLFunc(ReadMem16);//bwhahaha
		CodeGen->MOVSX32R16toR(EAX,EAX);
	}
	else
		printf("ReadMemRecS , wrong sz param\n");

	to.SaveFrom(EAX);	//save the result
}



void ReadMemRecS(RecRegType &to,RecRegType& addr,RecRegType& offset,u32 sz)
{
	if (offset.IsConst)
	{
		ReadMemRecS(to,addr,offset.ConstValue,sz);
		return;
	}

	offset.LoadTo(ECX);
	if (addr.IsConst)
	{
		//this could be done on ADD too ;)
		if (addr.ConstValue)
			CodeGen->ADD32ItoR(ECX,addr.ConstValue);
	}
	else
	{
		CodeGen->ADD32MtoR(ECX,addr.RegLoc);
	}

	if (sz==1)
	{
		CodeGen->CALLFunc(ReadMem8);//bwhahaha
		CodeGen->MOVSX32R8toR(EAX,EAX);
	}
	else if (sz==2)
	{
		CodeGen->CALLFunc(ReadMem16);//bwhahaha
		CodeGen->MOVSX32R16toR(EAX,EAX);
	}
	else
		printf("ReadMemRecS , wrong sz param\n");

	
	to.SaveFrom(EAX);	//save the result
}
//WriteMem(u32 addr,u32 data,u32 sz)
void WriteMemRec(u32 addr,u32 offset,RecRegType &data,u32 sz)
{
	u32 ReadAddr=addr+offset;
	if (IsOnRam(ReadAddr))
	{
		if (sz==1)
		{
			if (data.IsConst)
			{
				CodeGen->MOV8ItoM(GetMemPtr(ReadAddr,sz),(u8)data.ConstValue);
			}
			else
			{
				CodeGen->MOV8MtoR(EAX,(u8*)data.RegLoc);
				CodeGen->MOV8RtoM(GetMemPtr(ReadAddr,sz),EAX);
			}
		}
		else if (sz==2)
		{
			if (data.IsConst)
			{
				CodeGen->MOV16ItoM((u16*)GetMemPtr(ReadAddr,sz),(u16)data.ConstValue);
			}
			else
			{
				CodeGen->MOV16MtoR(EAX,(u16*)data.RegLoc);
				CodeGen->MOV16RtoM((u16*)GetMemPtr(ReadAddr,sz),EAX);
			}
		}
		else if(sz==4)
		{
			if (data.IsConst)
			{
				CodeGen->MOV32ItoM((u32*)GetMemPtr(ReadAddr,sz),data.ConstValue);
			}
			else
			{
				CodeGen->MOV32MtoR(EAX,data.RegLoc);
				CodeGen->MOV32RtoM((u32*)GetMemPtr(ReadAddr,sz),EAX);
			}
		}
		else
			printf("WriteMemRec , wrong sz param\n");
	}
	else
	{
		CodeGen->MOV32ItoR(ECX,addr+offset);

		if (data.IsConst)	//load imm
			CodeGen->MOV32ItoR(EDX,data.ConstValue);
		else				//load mem
			CodeGen->MOV32MtoR(EDX,data.RegLoc);

		if (sz==1)
			CodeGen->CALLFunc(WriteMem8);//bwhahaha
		else if (sz==2)
			CodeGen->CALLFunc(WriteMem16);//bwhahaha
		else if(sz==4)
			CodeGen->CALLFunc(WriteMem32);//bwhahaha
		else
			printf("WriteMemRec , wrong sz param\n");
	}
}
void WriteMemRec(RecRegType& addr,u32 offset,RecRegType &data,u32 sz)
{
	if (addr.IsConst)
	{
		WriteMemRec(addr.ConstValue,offset,data,sz);
		return;
	}

	//address is not const , we need to calculate it
	if (offset!=0)
	{
		CodeGen->MOV32ItoR(ECX,offset);		//offset
		CodeGen->ADD32MtoR(ECX,addr.RegLoc);//+base
	}
	else
		CodeGen->MOV32MtoR(ECX,addr.RegLoc);//0+base

	
	if (data.IsConst)	//load imm
		CodeGen->MOV32ItoR(EDX,data.ConstValue);
	else				//load mem
		CodeGen->MOV32MtoR(EDX,data.RegLoc);

	if (sz==1)
		CodeGen->CALLFunc(WriteMem8);//bwhahaha
	else if (sz==2)
		CodeGen->CALLFunc(WriteMem16);//bwhahaha
	else if(sz==4)
		CodeGen->CALLFunc(WriteMem32);//bwhahaha
	else
		printf("WriteMemRec , wrong sz param\n");
}
void WriteMemRec(RecRegType& addr,RecRegType& offset,RecRegType &data,u32 sz)
{
	if (offset.IsConst)
	{	//call the "offset is const" variant
		WriteMemRec(addr,offset.ConstValue,data,sz);
		return;
	}

	if (addr.IsConst)
	{	//call the "offset is const" variant , swap addr w/ offset ;)
		WriteMemRec(offset,addr.ConstValue,data,sz);
		return ;
	}

	//ohh well , address needs to be calculated at runtime :P

	CodeGen->MOV32MtoR(ECX,addr.RegLoc);	//base
	CodeGen->ADD32MtoR(ECX,offset.RegLoc);	//+=offset
	//k ready ;P

	if (data.IsConst)	//load imm
		CodeGen->MOV32ItoR(EDX,data.ConstValue);
	else				//load mem
		CodeGen->MOV32MtoR(EDX,data.RegLoc);

	if (sz==1)
		CodeGen->CALLFunc(WriteMem8);//bwhahaha
	else if (sz==2)
		CodeGen->CALLFunc(WriteMem16);//bwhahaha
	else if(sz==4)
		CodeGen->CALLFunc(WriteMem32);//bwhahaha
	else
		printf("WriteMemRec , wrong sz param\n");
}


void rec_UpdateFPSCR()
{
	FlushKillAll();
	
	CodeGen->CALLFunc(UpdateFPSCR);
}

bool rec_UpdateSR()
{
	FlushKillAll();
	CodeGen->CALLFunc(UpdateSR);
	return true;
}


void recStartRecompile()
{
	for (int i=0;i<8;i++)
	{
		rec_r[i].RegLoc=&r[i];
		rec_r_bank[i].RegLoc=&r_bank[i];
	}

	for (int i=8;i<16;i++)
	{
		rec_r[i].RegLoc=&r[i];
	}

	rec_gbr.RegLoc=&gbr;
	rec_ssr.RegLoc=&ssr;
	rec_spc.RegLoc=&spc;
	rec_sgr.RegLoc=&sgr;
	rec_dbr.RegLoc=&dbr;
	rec_vbr.RegLoc=&vbr;

	rec_mach.RegLoc=&mach;
	rec_macl.RegLoc=&macl;
	rec_pr.RegLoc=&pr;
	rec_fpul.RegLoc=&fpul;
	rec_sr.full.RegLoc=&sr.full;
	rec_fpscr.full.RegLoc=&fpscr.full;

	if (CodeGen)
		delete CodeGen;
	CodeGen=new Emmiter<>();
}
void recEndRecompile(bool bNoGen,u32 npc)
{
	FlushKillAll();
	if (!bNoGen)
		CodeGen->MOV32ItoM(&pc,npc);
	CodeGen->RET();
}
u32 recGetCodeSize()
{
	return CodeGen->UsedBytes();
}
u32* rec_pc_ptr;
void __fastcall rec_cpu_opcode_nimp(u32 op,u32 pc_v);
bool recRecompileOp(u32 op,u32 &rec_pc)
{
	rec_pc_ptr=&rec_pc;
//	FlushKillAll();
	RecOpPtr[op](op,rec_pc);
//	FlushKillAll();
	if (OpTyp[op] & (Branch | BranchDelay | SystemSt))
	{
		if (RecOpPtr[op]==rec_cpu_opcode_nimp)
			return false;
		else
			return true;
	}
	else
		return true;
}
RecCodeCall* recGetFunction()
{
	return (RecCodeCall*)CodeGen->GetCode();
}

void SetRecPC(u32 new_pc)
{
	*rec_pc_ptr=new_pc;
}

void recExecDelayslot()
{
	u16 op = ReadMem16(*rec_pc_ptr+2);
	RecOpPtr[op](op,*rec_pc_ptr+2);
}
#define ExecuteDelayslot recExecDelayslot

#define UpdateFPSCR rec_UpdateFPSCR
#define UpdateSR rec_UpdateSR

#define r rec_r
#define r_bank	rec_r_bank
#define gbr		rec_gbr
#define ssr		rec_ssr
#define spc		rec_spc
#define sgr		rec_sgr
#define dbr		rec_dbr
#define vbr		rec_vbr
#define mach	rec_mach
#define macl	rec_macl

#define pr		rec_pr
#define fpul	rec_fpul

#define sr		rec_sr
#define fpscr	rec_fpscr
//>:D
#include "dc\sh4\sh4_cpu_arith.h"
#include "dc\sh4\sh4_cpu_branch.h"
#include "dc\sh4\sh4_cpu_logic.h"
#include "dc\sh4\sh4_cpu_movs.h"

//callback to interpreter ;P
//we must kill all regcached things/consts
void __fastcall rec_cpu_opcode_nimp(u32 op,u32 pc_v)
{
	FlushKillAll();

	if ( (OpTyp[op] !=Arithm_FPU))
	{
		CodeGen->MOV32ItoM(&pc,pc_v);
	}
	CodeGen->MOV32ItoR(ECX,op);
	CodeGen->CALLFunc(OpPtr[op]);
}

void __fastcall rec_fpu_opcode(u32 op,u32 pc_v)
{
	rec_cpu_opcode_nimp(op,pc_v);
}
