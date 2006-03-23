#include "rec_v0_recompiler.h"
#include "emmiter\emmiter.h"

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

void GenAll()
{
	//Emmiter te;
}

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

struct RecRegType
{
	u32 nil;
	bool IsConst;
	u32 ConstValue;

	void operator-=(const RecRegType &rhs)
    {
       	if (rhs.IsConst)//sub x , value
		{
			if (IsConst)//sub value,value
			{
				ConstValue-=rhs.ConstValue;
			}
			else//sub reg,value
			{
				/*if (!RegCache)
				{
					x86Reg =recAllocReg(ShReg,ReadWrite);
					RegCache=true;
				}
				sub32ItoR(x86Reg,rhs.ConstValue);*/
			}
		}
		else
		{
			if (IsConst)//sub const,reg
			{
				/*if (!RegCache)
				{
					x86Reg =recAllocReg(ShReg,ReadWrite);
					RegCache=true;
				}
				//lea ?
				MOV32ItoR(x86Reg,ConstValue);
				if (rhs.RegCache)
				{
					sub32RtoR(x86Reg,rhs.x86Reg);
				}
				else
				{
					sub32MtoR(x86Reg,(u32)rhs.RegLoc);
				}
				IsConst=false;//no more const ;(*/
			}
			else//sub reg,reg
			{
				/*if (rhs.RegCache)//sub reg,x86reg
				{
					if (!RegCache)
					{
						x86Reg =recAllocReg(ShReg,ReadWrite);
						RegCache=true;
					}
					sub32RtoR(x86Reg,rhs.x86Reg);
				}
				else			//sub reg,memreg
				{
					if (!RegCache)
					{
						x86Reg =recAllocReg(ShReg,ReadWrite);
						RegCache=true;
					}
					sub32MtoR(x86Reg,(u32)rhs.RegLoc);
				}*/
			}

		}
    }

	void operator+=(const RecRegType &rhs)
    {
       	if (rhs.IsConst)//add x , value
		{
			if (IsConst)//add value,value
			{
				ConstValue+=rhs.ConstValue;
			}
			else//add reg,value
			{
				/*if (!RegCache)
				{
					x86Reg =recAllocReg(ShReg,ReadWrite);
					RegCache=true;
				}
				ADD32ItoR(x86Reg,rhs.ConstValue);*/
			}
		}
		else
		{
			if (IsConst)//add const,reg
			{
				/*if (!RegCache)
				{
					x86Reg =recAllocReg(ShReg,ReadWrite);
					RegCache=true;
				}
				//lea ?
				MOV32ItoR(x86Reg,ConstValue);
				if (rhs.RegCache)
				{
					ADD32RtoR(x86Reg,rhs.x86Reg);
				}
				else
				{
					ADD32MtoR(x86Reg,(u32)rhs.RegLoc);
				}
				IsConst=false;//no more const ;(*/
			}
			else//add reg,reg
			{
				/*if (rhs.RegCache)//add reg,x86reg
				{
					if (!RegCache)
					{
						x86Reg =recAllocReg(ShReg,ReadWrite);
						RegCache=true;
					}
					ADD32RtoR(x86Reg,rhs.x86Reg);
				}
				else			//add reg,memreg
				{
					if (!RegCache)
					{
						x86Reg =recAllocReg(ShReg,ReadWrite);
						RegCache=true;
					}
					ADD32MtoR(x86Reg,(u32)rhs.RegLoc);
				}*/
			}

		}
    }

	void operator=(const RecRegType &rhs)
    {
		if (rhs.IsConst)
		{
			IsConst=true;
			ConstValue=rhs.ConstValue;
		}
		else
		{
			IsConst=false;
			if (rhs.RegCache)
			{
				/*if (!RegCache)
				{
					x86Reg =recAllocReg(ShReg,Write);
					RegCache=true;
				}
				MOV32RtoR(x86Reg,rhs.x86Reg);*/
			}
			else
			{
				/*if (!RegCache)
				{
					x86Reg =recAllocReg(ShReg,Write);
					RegCache=true;
				}
				MOV32MtoR(x86Reg,(u32)rhs.RegLoc);*/
			}

		}
    }
	void operator=(const u32 &constv)
    {
		IsConst=true;
		ConstValue=constv;	
    }
	void operator=(const s32 &constv)
    {
		IsConst=true;
		ConstValue=constv;	
    }
	void operator+=(const u32 constv)
	{
	};
	void operator-=(const s32 constv)
	{
	};
	void operator--(int wtf)
	{
	}
	void operator++(int wtf)
	{
	}
	
	bool IsInCache()
	{
		
	}

    //recSh4RegType ShReg;
	//recx86IntRegType x86Reg;
	u32* RegLoc;
	bool RegCache;
};
//hm?
struct RecSrType: public RecRegType
{
	u32 GetFull()
	{
		return 0;
	}
	void SetFull(RecRegType& reg)
	{
		
	}
};

struct RecFpscrType: public RecRegType
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



//Read Mem macros
#define ReadMemU32(to,addr) //to=ReadMem32(addr)
#define ReadMemS16(to,addr)// to=(u32)(s32)(s16)ReadMem16(addr)
#define ReadMemS8(to,addr) //to=(u32)(s32)(s8)ReadMem8(addr)

//Base,offset format
#define ReadMemBOU32(to,addr,offset)	//ReadMemU32(to,addr+offset)
#define ReadMemBOS16(to,addr,offset)	//ReadMemS16(to,addr+offset)
#define ReadMemBOS8(to,addr,offset)		//ReadMemS8(to,addr+offset)

//Write Mem Macros
#define WriteMemU32(addr,data)				//WriteMem32(addr,(u32)data)
#define WriteMemU16(addr,data)				//WriteMem16(addr,(u16)data)
#define WriteMemU8(addr,data)				//WriteMem8(addr,(u8)data)

//Base,offset format
#define WriteMemBOU32(addr,offset,data)		//WriteMemU32(addr+offset,data)
#define WriteMemBOU16(addr,offset,data)		//WriteMemU16(addr+offset,data)
#define WriteMemBOU8(addr,offset,data)		//WriteMemU8(addr+offset,data)

void ReadMemRec(RecRegType &to,u32 addr,u32 sz)
{
	//do some magic :P
}

void ReadMemRec(RecRegType &to,RecRegType& addr,u32 sz)
{
	//do some magic :P
	if (addr.IsConst)
	{
		ReadMemRec(to,addr.ConstValue,sz);
		return;
	}
}



//WriteMem(u32 addr,u32 data,u32 sz)
void WriteMemRec(u32 addr,u32 data,u32 sz)
{
}
void WriteMemRec(u32 addr,RecRegType &data,u32 sz)
{
	if (data.IsConst)
	{
		WriteMemRec(addr,data.ConstValue,sz);
		return;
	}
}

void WriteMemRec(RecRegType& addr,u32 data,u32 sz)
{
	if (addr.IsConst)
	{
		WriteMemRec(addr.ConstValue,data,sz);
		return;
	}
}
void WriteMemRec(RecRegType& addr,RecRegType &data,u32 sz)
{
	if (data.IsConst)
	{
		WriteMemRec(addr,data.ConstValue,sz);
		return;
	}
	else if (addr.IsConst)
	{
		WriteMemRec(addr.ConstValue,data,sz);
		return;
	}
}







#define UpdateFPSCR rec_UpdateFPSCR
#define UpdateSR rec_UpdateSR

void rec_UpdateFPSCR()
{
}

void rec_UpdateSR()
{
}

#include "dc\sh4\sh4_cpu_movs.h"