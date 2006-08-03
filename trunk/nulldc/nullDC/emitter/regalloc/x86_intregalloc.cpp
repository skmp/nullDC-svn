#include "x86_intregalloc.h"

/*
//implement register allocators on a class , so we can swap em around?
//methods needed
//
//DoAllocation		: do allocation on the block
//BeforeEmit		: generate any code needed before the main emittion begins (other register allocators may have emited code tho)
//BeforeTrail		: generate any code needed after the main emittion has ended (other register allocators may emit code after that tho)
//AfterTrail		: generate code after the native block end (after the ret) , can be used to emit helper functions (other register allocators may emit code after that tho)
//IsRegAllocated	: *couh* yea .. :P
//GetRegister		: Get the register , needs flag if it's read or write. Carefull w/ register state , we may need to implement state push/pop
//PushRegister		: push register to stack (if allocated)
//PopRegister		: pop register from stack (if allocated)
//FlushRegister		: write reg to reg location , and dealloc it
//WriteBackRegister	: write reg to reg location
//ReloadRegister	: read reg from reg location , discard old result

//more helpers
x86IntRegType LoadReg_force(x86IntRegType to,u8 reg)
{
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOV32MtoR(to,GetRegPtr(reg));
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg(reg);
			if (!(to==x86reg))
				x86e->MOV32RtoR(to,x86reg);
		}
	}
	else
	{
		x86e->MOV32MtoR(to,GetRegPtr(reg));
	}

	return to;
}

x86IntRegType LoadReg(x86IntRegType to,u8 reg)
{
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOV32MtoR(to,GetRegPtr(reg));
			//return to;
		}
		else
		{
			to= LoadRegCache_reg(reg);
		}
	}
	else
	{
		x86e->MOV32MtoR(to,GetRegPtr(reg));
		//return to;
	}

	return to;
}

x86IntRegType LoadReg_nodata(x86IntRegType to,u8 reg)
{
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			//x86e->MOV32MtoR(to,GetRegPtr(reg)); -> do nothin :P
			return to;
		}
		else
		{
			return LoadRegCache_reg_nodata(reg);
		}
	}
	else
	{
		return to;
	}
}
*/

u32 ssenoalloc=0;
u32 ssealloc=0;
x86IntRegType reg_to_alloc[4]=
{
	EBX,
	EBP,
	ESI,
	EDI
};

#define REG_ALLOC_COUNT 4

//////////////////////////////////////////////////
// new reg alloc class							//
//////////////////////////////////////////////////
class SimpleGPRAlloc : public IntegerRegAllocator
{

	emitter<>* x86e;
	//helpers & misc shit
	struct RegAllocInfo
	{
		x86IntRegType x86reg;
		bool InReg;
		bool Dirty;
	};
	RegAllocInfo r_alloced[16];
	struct sort_temp
	{
		int cnt;
		int reg;
	};

	//ebx, ebp, esi, and edi are preserved

	//Yay bubble sort
	void bubble_sort(sort_temp numbers[] , int array_size)
	{
		int i, j;
		sort_temp temp;
		for (i = (array_size - 1); i >= 0; i--)
		{
			for (j = 1; j <= i; j++)
			{
				if (numbers[j-1].cnt < numbers[j].cnt)
				{
					temp = numbers[j-1];
					numbers[j-1] = numbers[j];
					numbers[j] = temp;
				}
			}
		}
	}


	//check
	void checkvr(u32 reg)
	{
		if (reg>=fr_0 && reg<=fr_15 )
			__asm int 3;
	//	if (reg>=xf_0 && reg<=xf_15 )
		//	__asm int 3; 
	}
	//Helper , olny internaly used now
	virtual void MarkDirty(u32 reg)
	{
		checkvr(reg);
		if (IsRegAllocated(reg))
		{
			r_alloced[reg].Dirty=true;
		}
	}
	//reg alloc interface :)
	//DoAllocation		: do allocation on the block
	virtual void DoAllocation(BasicBlock* block,emitter<>* x86e)
	{
		this->x86e=x86e;
		sort_temp used[16];
		for (int i=0;i<16;i++)
		{
			used[i].cnt=0;
			used[i].reg=r0+i;
			r_alloced[i].x86reg=GPR_Error;
			r_alloced[i].InReg=false;
			r_alloced[i].Dirty=false;
		}

		u32 op_count=block->ilst.op_count;
		shil_opcode* curop;

		for (u32 j=0;j<op_count;j++)
		{
			curop=&block->ilst.opcodes[j];
			for (int i = 0;i<16;i++)
			{
				//both reads and writes , give it one more ;P
				if ( curop->UpdatesReg((Sh4RegType) (r0+i)) )
					used[i].cnt+=12;

				if (curop->ReadsReg((Sh4RegType) (r0+i)))
					used[i].cnt+=6;

				if (curop->WritesReg((Sh4RegType) (r0+i)))
					used[i].cnt+=9;
			}
		}

		bubble_sort(used,16);

		for (u32 i=0;i<REG_ALLOC_COUNT;i++)
		{
			if (used[i].cnt<14)
				break;
			r_alloced[used[i].reg].x86reg=reg_to_alloc[i];
		}
	}
	//BeforeEmit		: generate any code needed before the main emittion begins (other register allocators may have emited code tho)
	virtual void BeforeEmit()
	{
		for (int i=0;i<16;i++)
		{
			if (IsRegAllocated(i))
			{
				GetRegister(EAX,i,RA_DEFAULT);
			}
		}
	}
	//BeforeTrail		: generate any code needed after the main emittion has ended (other register allocators may emit code after that tho)
	virtual void BeforeTrail()
	{
		FlushRegCache();
	}
	//AfterTrail		: generate code after the native block end (after the ret) , can be used to emit helper functions (other register allocators may emit code after that tho)
	virtual void AfterTrail()
	{
	}
	//IsRegAllocated	: *couh* yea .. :P
	virtual bool IsRegAllocated(u32 sh4_reg)
	{
		checkvr(sh4_reg);
		if (sh4_reg<=r15)
			return r_alloced[sh4_reg].x86reg!=GPR_Error;
		else 
			return false;
	}
	//Carefull w/ register state , we may need to implement state push/pop
	//GetRegister		: Get the register , needs flag for mode
	virtual x86IntRegType GetRegister(x86IntRegType d_reg,u32 sh4_reg,u32 mode)
	{
		checkvr(sh4_reg);
		//No move , or RtR move + reload
		if (IsRegAllocated(sh4_reg))
		{
			if ( ((mode & RA_NODATA)==0) && r_alloced[sh4_reg].InReg==false)
			{
				//if we must load , and its not loaded
				x86e->MOV32MtoR(r_alloced[sh4_reg].x86reg,GetRegPtr(sh4_reg));
				r_alloced[sh4_reg].Dirty=false;//not dirty .ffs, we just loaded it....
			}
			
			r_alloced[sh4_reg].InReg=true;

			//nada
			if (mode & RA_FORCE)
			{
				//move to forced reg , if needed
				if (r_alloced[sh4_reg].x86reg!=d_reg)
					x86e->MOV32RtoR(d_reg,r_alloced[sh4_reg].x86reg);
				return d_reg;
			}
			else
			{
				//return allocated reg
				return r_alloced[sh4_reg].x86reg;
			}
		}
		else
		{
			//MtoR move , nodata is invalid , force has no effect ;)
			if (0==(mode & RA_NODATA))
				x86e->MOV32MtoR(d_reg,GetRegPtr(sh4_reg));
			return d_reg;
		}

		//yes it realy can't come here
//		__asm int 3;
		//return EAX;
	}
	//Save registers
	virtual void SaveRegister(u32 reg,x86IntRegType from)
	{
		checkvr(reg);
		if (!IsRegAllocated(reg))
		{
			x86e->MOV32RtoM(GetRegPtr(reg),from);
		}
		else
		{
			x86IntRegType x86reg=GetRegister(EAX,reg,RA_NODATA);
			if (x86reg!=from)
				x86e->MOV32RtoR(x86reg,from);
		}
		MarkDirty(reg);
	}
	virtual void SaveRegister(u32 reg,u32 from)
	{
		checkvr(reg);
		if (!IsRegAllocated(reg))
		{
			x86e->MOV32ItoM(GetRegPtr(reg),from);
		}
		else
		{
			x86IntRegType x86reg=GetRegister(EAX,reg,RA_NODATA);
			x86e->MOV32ItoR(x86reg,from);
		}
		MarkDirty(reg);
	}
	virtual void SaveRegister(u32 reg,u32* from)
	{
		checkvr(reg);
		if (!IsRegAllocated(reg))
		{
			x86e->MOV32MtoR(ECX,from);
			x86e->MOV32RtoM(GetRegPtr(reg),ECX);
		}
		else
		{
			x86IntRegType x86reg=GetRegister(EAX,reg,RA_NODATA);
			x86e->MOV32MtoR(x86reg,from);
		}
		MarkDirty(reg);
	}
	virtual void SaveRegister(u32 reg,s16* from)
	{
		checkvr(reg);
		if (!IsRegAllocated(reg))
		{
			x86e->MOVSX32M16toR(ECX,(u16*)from);
			x86e->MOV32RtoM(GetRegPtr(reg),ECX);
		}
		else
		{
			x86IntRegType x86reg=GetRegister(EAX,reg,RA_NODATA);
			x86e->MOVSX32M16toR(x86reg,(u16*)from);
		}
		MarkDirty(reg);
	}
	virtual void SaveRegister(u32 reg,s8* from)
	{
		checkvr(reg);
		if (!IsRegAllocated(reg))
		{
			x86e->MOVSX32M8toR(ECX,(u8*)from);
			x86e->MOV32RtoM(GetRegPtr(reg),ECX);
		}
		else
		{
			x86IntRegType x86reg=GetRegister(EAX,reg,RA_NODATA);
			x86e->MOVSX32M8toR(x86reg,(u8*)from);
		}
		MarkDirty(reg);
	}

	//FlushRegister		: write reg to reg location , and dealloc it
	virtual void FlushRegister(u32 reg)
	{
		checkvr(reg);
		if (IsRegAllocated(reg) && r_alloced[reg].InReg)
		{
			if (r_alloced[reg].Dirty)
				x86e->MOV32RtoM(GetRegPtr(reg),r_alloced[reg].x86reg);
			r_alloced[reg].InReg=false;
			r_alloced[reg].Dirty=false;
		}
	}
	//Flush all regs
	virtual void FlushRegCache()
	{
		for (int i=0;i<16;i++)
		{
			FlushRegister(i);
		}
	}
	//WriteBackRegister	: write reg to reg location
	virtual void WriteBackRegister(u32 reg)
	{
		checkvr(reg);
		if (IsRegAllocated(reg) && r_alloced[reg].InReg && r_alloced[reg].Dirty)
		{
			r_alloced[reg].Dirty=false;
			x86e->MOV32RtoM(GetRegPtr(reg),r_alloced[reg].x86reg);
		}
	}
	//ReloadRegister	: read reg from reg location , discard old result
	virtual void ReloadRegister(u32 reg)
	{
		checkvr(reg);
		if (IsRegAllocated(reg) && r_alloced[reg].InReg)
		{
			r_alloced[reg].Dirty=false;
			x86e->MOV32MtoR(r_alloced[reg].x86reg,GetRegPtr(reg));
		}
	}
};

IntegerRegAllocator* GetGPRtAllocator()
{
	return new SimpleGPRAlloc();
}