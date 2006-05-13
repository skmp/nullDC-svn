/*
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
bool IsRegCached(u32 reg)
{
	if (reg<=r15)
	{
		if (REG_ALLOC_X86)
		{
			return r_alloced[reg].x86reg!=GPR_Error;
		}
		else
		{
			return false;
		}
	}
	else
		return false;
}
void FlushRegCache_reg(u32 reg)
{
	if (IsRegCached(reg) && r_alloced[reg].InReg)
	{
		if (r_alloced[reg].Dirty)
			x86e->MOV32RtoM(GetRegPtr(reg),r_alloced[reg].x86reg);
		r_alloced[reg].InReg=false;
		r_alloced[reg].Dirty=false;
	}
}

void MarkDirty(u32 reg)
{
	if (IsRegCached(reg))
	{
		r_alloced[reg].Dirty=true;
	}
}
x86IntRegType LoadRegCache_reg(u32 reg)
{
	if (IsRegCached(reg))
	{
		if (r_alloced[reg].InReg==false )
		{
			r_alloced[reg].InReg=true;
			x86e->MOV32MtoR(r_alloced[reg].x86reg,GetRegPtr(reg));
		}
		return r_alloced[reg].x86reg;
	}

	return GPR_Error;
}

x86IntRegType LoadRegCache_reg_nodata(u32 reg)
{
	if (IsRegCached(reg))
	{
		if (r_alloced[reg].InReg==false )
		{
			r_alloced[reg].InReg=true;
		}
		return r_alloced[reg].x86reg;
	}

	return GPR_Error;
}
void AllocateRegisters(rec_v1_BasicBlock* block)
{
	if(REG_ALLOC_X86)
	{
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
					used[i].cnt+=1;

				if (curop->ReadsReg((Sh4RegType) (r0+i)))
					used[i].cnt+=1;

				if (curop->WritesReg((Sh4RegType) (r0+i)))
					used[i].cnt+=1;
			}
		}

		bubble_sort(used,16);

		for (u32 i=0;i<REG_ALLOC_COUNT;i++)
		{
			if (used[i].cnt==0)
				break;
			r_alloced[used[i].reg].x86reg=reg_to_alloc[i];
		}
	}
}
void LoadRegisters()
{
	if(REG_ALLOC_X86)
	{
		for (int i=0;i<16;i++)
		{
			if (IsRegCached(i))
			{
				LoadRegCache_reg(i);
			}
		}
	}
}
INLINE x86IntRegType LoadReg_force(x86IntRegType to,u8 reg)
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

	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		if (reg==reg_sr)
		{
			x86e->AND32ItoR(to,(u32)~1);
			x86e->OR32MtoR(to,&T_bit_value);
			x86e->MOV32RtoM(GetRegPtr(reg_sr),to);//save it back to be sure :P
			T_Edited=false;
		}
	}
	return to;
}

INLINE x86IntRegType LoadReg(x86IntRegType to,u8 reg)
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
	
	if(REG_ALLOC_T_BIT_SEPERATE)
	{
		if (reg==reg_sr)
		{
			x86e->AND32ItoR(to,(u32)~1);
			x86e->OR32MtoR(to,&T_bit_value);
			x86e->MOV32RtoM(GetRegPtr(reg_sr),to);//save it back to be sure :P
			T_Edited=false;
		}
	}
	return to;
}

INLINE x86IntRegType LoadReg_nodata(x86IntRegType to,u8 reg)
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


INLINE void SaveReg(u8 reg,x86IntRegType from)
{
	MarkDirty(reg);
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOV32RtoM(GetRegPtr(reg),from);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			if (x86reg!=from)
				x86e->MOV32RtoR(x86reg,from);
		}
	}
	else
	{
		x86e->MOV32RtoM(GetRegPtr(reg),from);
	}

	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		if (reg==reg_sr)
		{
			T_Edited=false;
		}
	}
}

INLINE void SaveReg(u8 reg,u32 from)
{
	MarkDirty(reg);
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOV32ItoM(GetRegPtr(reg),from);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			if (from==0)
				x86e->XOR32RtoR(x86reg,x86reg);
			else if (from ==0xFFFFFFFF)
				x86e->MOV32ItoR(x86reg,from);//xor , dec ?
			else
				x86e->MOV32ItoR(x86reg,from);
		}
	}
	else
	{
		x86e->MOV32ItoM(GetRegPtr(reg),from);
	}

	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		if (reg==reg_sr)
		{
			T_Edited=false;
		}
	}
}

INLINE void SaveReg(u8 reg,u32* from)
{
	MarkDirty(reg);
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOV32MtoR(ECX,from);
			x86e->MOV32RtoM(GetRegPtr(reg),ECX);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			x86e->MOV32MtoR(x86reg,from);
		}
	}
	else
	{
		x86e->MOV32MtoR(ECX,from);
		x86e->MOV32RtoM(GetRegPtr(reg),ECX);
	}

	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		if (reg==reg_sr)
		{
			T_Edited=false;
		}
	}
}

INLINE void SaveReg(u8 reg,u16* from)
{
	MarkDirty(reg);
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOVSX32M16toR(ECX,from);
			x86e->MOV32RtoM(GetRegPtr(reg),ECX);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			x86e->MOVSX32M16toR(x86reg,from);
		}
	}
	else
	{
		x86e->MOVSX32M16toR(ECX,from);
		x86e->MOV32RtoM(GetRegPtr(reg),ECX);
	}
}

INLINE void SaveReg(u8 reg,u8* from)
{
	MarkDirty(reg);
	if(REG_ALLOC_X86)
	{
		if (reg>r15 || (!IsRegCached(reg)))
		{
			x86e->MOVSX32M8toR(ECX,from);
			x86e->MOV32RtoM(GetRegPtr(reg),ECX);
		}
		else
		{
			x86IntRegType x86reg=LoadRegCache_reg_nodata(reg);
			x86e->MOVSX32M8toR(x86reg,from);
		}
	}
	else
	{
		x86e->MOVSX32M8toR(ECX,from);
		x86e->MOV32RtoM(GetRegPtr(reg),ECX);
	}
}

INLINE void FlushRegCache()
{
	if(REG_ALLOC_X86)
	{
		for (int i=0;i<16;i++)
		{
			FlushRegCache_reg(i);
		}
	}
	
	if (REG_ALLOC_T_BIT_SEPERATE)
	{
		if (T_Edited)
		{
			//save T
			LoadReg_force(EAX,reg_sr);			//ecx=sr(~1)|T
			x86e->AND32ItoR(EAX,(u32)~1);
			x86e->OR32MtoR(EAX,&T_bit_value);
			SaveReg(reg_sr,EAX);
			T_Edited=false;
		}
	}
}



//REGISTER ALLOCATION

struct RegAllocInfo
{
	x86IntRegType x86reg;
	bool InReg;
	bool Dirty;
};
 RegAllocInfo r_alloced[16];
//compile a basicblock

struct sort_temp
{
	int cnt;
	int reg;
};

//ebx, ebp, esi, and edi are preserved



x86IntRegType reg_to_alloc[4]=
{
	EBX,
	EBP,
	ESI,//-> reserved for cycle counts : no more :)
	EDI
};

//xmm0 is reserved for math/temp
x86SSERegType reg_to_alloc_xmm[7]=
{
	XMM1,
	XMM2,
	XMM3,
	XMM4,
	XMM5,
	XMM6,
	XMM7,
};

*/