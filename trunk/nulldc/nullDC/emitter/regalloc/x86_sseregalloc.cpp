#include "x86_sseregalloc.h"
#include <assert.h>

#define REG_ALLOC_COUNT (7)
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


struct fprinfo
{
	x86SSERegType reg;
	bool Loaded;
	bool WritenBack;
};

bool SimpleSSERegAlloc_sse2=false;
bool SimpleSSERegAlloc_cft=false;
void SimpleSSERegAlloc_init()
{
	if (SimpleSSERegAlloc_cft)
		return;
	SimpleSSERegAlloc_cft=true;
	SimpleSSERegAlloc_sse2=true;
	try
	{
		__asm
		{
			movd XMM0,EAX;
		}
	}
	catch(...)
	{
		SimpleSSERegAlloc_sse2=false;
	}

	if (SimpleSSERegAlloc_sse2)
	{
		printf("SSE2 supported , using it\n");
	}
	else
	{
		printf("SSE2 not supported , using olny SSE1\n");
	}
}

class SimpleSSERegAlloc:public FloatRegAllocator
{
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



	emitter<>* x86e;
	fprinfo reginf[16];
	

	fprinfo* GetInfo(u32 reg)
	{
		reg-=fr_0;
		if (reg<16)
		{
			if (reginf[reg].reg!=XMM_Error)
			{
				//__asm int 3;
				return &reginf[reg];
			}
		}
		return 0;
	}
	void ensure_valid(u32 reg)
	{
		if (reg>=fr_0 && reg<=fr_15)
			__noop;
		else
			__asm int 3;
	}
	bool DoAlloc;
	//methods needed
	//
	//DoAllocation		: do allocation on the block
	virtual void DoAllocation(BasicBlock* block,emitter<>* x86e)
	{
		SimpleSSERegAlloc_init();

		this->x86e=x86e;
		DoAlloc=block->flags.FpuIsVector==0 && block->cpu_mode_tag==0;
		
		sort_temp used[16];
		for (int i=0;i<16;i++)
		{
			used[i].cnt=0;
			used[i].reg=r0+i;
			reginf[i].reg=XMM_Error;
			reginf[i].Loaded=false;
			reginf[i].WritenBack=false;
		}

		if (DoAlloc)
		{
			u32 op_count=block->ilst.op_count;
			shil_opcode* curop;
			
			for (u32 j=0;j<op_count;j++)
			{
				curop=&block->ilst.opcodes[j];
				for (int i = 0;i<16;i++)
				{
					//both reads and writes , give it one more ;P
					if ( curop->UpdatesReg((Sh4RegType) (fr_0+i)) )
						used[i].cnt+=12;

					if (curop->ReadsReg((Sh4RegType) (fr_0+i)))
						used[i].cnt+=6;

					if (curop->WritesReg((Sh4RegType) (fr_0+i)))
						used[i].cnt+=9;
				}
			}

			bubble_sort(used,16);
			u32 i;
			for (i=0;i<REG_ALLOC_COUNT;i++)
			{
				if (used[i].cnt<14)
					break;
				reginf[used[i].reg].reg=reg_to_alloc_xmm[i];
			}
			//printf("Allocaded %d xmm regs\n",i);
			memset(reginf,0xFF,sizeof(reginf));
		}
	}
	//BeforeEmit		: generate any code needed before the main emittion begins (other register allocators may have emited code tho)
	virtual void BeforeEmit()
	{
		for (int i=0;i<16;i++)
		{
			if (IsRegAllocated(i+fr_0))
			{
				//GetRegister(XMM0,i+fr_0,RA_DEFAULT);
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
		ensure_valid(sh4_reg);
		return GetInfo(sh4_reg)!=0;
	}
	//Carefull w/ register state , we may need to implement state push/pop
	//GetRegister		: Get the register , needs flag for mode
	virtual x86SSERegType GetRegister(x86SSERegType d_reg,u32 reg,u32 mode)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			fprinfo* r1=  GetInfo(reg);
			if (r1->Loaded==false && ((mode & RA_NODATA)==0) )
			{
				x86e->SSE_MOVSS_M32_to_XMM(r1->reg,GetRegPtr(reg));
				r1->WritenBack=true;
			}
			r1->Loaded=true;

			if (mode & RA_FORCE)
			{
				x86e->SSE_MOVSS_XMM_to_XMM(d_reg,r1->reg);
				return d_reg;
			}
			else
			{
				return r1->reg;
			}

		}
		else
		{
			if ((mode & RA_NODATA)==0)
				x86e->SSE_MOVSS_M32_to_XMM(d_reg,GetRegPtr(reg));
			return d_reg;
		}
		__asm int 3;
		return XMM_Error;
	}
	//Save registers
	virtual void SaveRegister(u32 reg,x86SSERegType from)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			fprinfo* r1=  GetInfo(reg);
			r1->Loaded=true;
			r1->WritenBack=false;
			
			if (r1->reg!=from)
				x86e->SSE_MOVSS_XMM_to_XMM(r1->reg,from);
		}
		else
			x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(reg),from);
	}
	
	virtual void SaveRegister(u32 reg,float* from)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			fprinfo* r1=  GetInfo(reg);
			r1->Loaded=true;
			r1->WritenBack=false;
			x86e->SSE_MOVSS_M32_to_XMM(r1->reg,(u32*)from);
		}
		else
		{
			x86e->SSE_MOVSS_M32_to_XMM(XMM0,(u32*)from);
			x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(reg),XMM0);
		}
	}
	//FlushRegister		: write reg to reg location , and reload it on next use that needs reloading
	virtual void FlushRegister(u32 reg)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			WriteBackRegister(reg);
			ReloadRegister(reg);
		}
	}
	virtual void FlushRegister_xmm(x86SSERegType reg)
	{
		for (int i=0;i<16;i++)
		{
			fprinfo* r1=  GetInfo(fr_0+i);
			if (r1!=0 && r1->reg==reg)
			{
				FlushRegister(fr_0+i);
			}
		}
	}
	virtual void FlushRegCache()
	{
		for (int i=0;i<16;i++)
			FlushRegister(fr_0+i);
	}
	//WriteBackRegister	: write reg to reg location
	virtual void WriteBackRegister(u32 reg)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			fprinfo* r1=  GetInfo(reg);
			if (r1->Loaded)
			{
				if (r1->WritenBack==false)
				{
					x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(reg),r1->reg);
					r1->WritenBack=true;
				}
			}
		}
	}
	//ReloadRegister	: read reg from reg location , discard old result
	virtual void ReloadRegister(u32 reg)
	{
		ensure_valid(reg);
		if (IsRegAllocated(reg))
		{
			fprinfo* r1=  GetInfo(reg);
			r1->Loaded=false;
		}
	}
	virtual void SaveRegisterGPR(u32 to,x86IntRegType from)
	{
		if (IsRegAllocated(to))
		{
			if (SimpleSSERegAlloc_sse2)
			{
				x86SSERegType freg=GetRegister(XMM0,to,RA_NODATA);
				assert(freg!=XMM0);
				//
				x86e->SSE2_MOVD_32R_to_XMM(freg,from);
			}
			else
			{
				x86e->MOV32RtoM(GetRegPtr(to),from);
				ReloadRegister(to);
			}
		}
		else
		{
			x86e->MOV32RtoM(GetRegPtr(to),from);
		}
	}
	virtual void LoadRegisterGPR(x86IntRegType to,u32 from)
	{
		if (IsRegAllocated(from))
		{
			fprinfo* r1=  GetInfo(from);
			if ((SimpleSSERegAlloc_sse2==true) &&  (r1->Loaded==true) && (r1->WritenBack==false))
			{
				x86SSERegType freg=GetRegister(XMM0,to,RA_DEFAULT);
				assert(freg!=XMM0);
				x86e->INT3();
				x86e->SSE2_MOVD_XMM_to_32R(to,freg);
			}
			else
			{
				FlushRegister(from);
				x86e->MOV32MtoR(to,GetRegPtr(from));
			}
		}
		else
		{
			x86e->MOV32MtoR(to,GetRegPtr(from));
		}
	}
};

FloatRegAllocator * GetFloatAllocator()
{
	return new SimpleSSERegAlloc();
}