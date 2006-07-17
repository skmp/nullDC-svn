#include "x86_sseregalloc.h"


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
	bool IsAllocated;
};

class SimpleSSERegAlloc:public FloatRegAllocator
{
	emitter<>* x86e;
	fprinfo reginf[16];

	fprinfo* GetInfo(u32 reg)
	{
		return 0;
		if (reg<16)
		{
			if (reginf[reg].IsAllocated)
			{
				__asm int 3;
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
	virtual void DoAllocation(rec_v1_BasicBlock* bb,emitter<>* x86e)
	{
		this->x86e=x86e;
		DoAlloc=bb->flags.FpuIsVector==0;
		if (DoAlloc)
		{
			memset(reginf,0,sizeof(reginf));
			for (int i=0;i<16;i++)
			{
				reginf[i].IsAllocated=false;
			}
		}
	}
	//BeforeEmit		: generate any code needed before the main emittion begins (other register allocators may have emited code tho)
	virtual void BeforeEmit()
	{
		if (DoAlloc)
		{
		}
	}
	//BeforeTrail		: generate any code needed after the main emittion has ended (other register allocators may emit code after that tho)
	virtual void BeforeTrail()
	{
		if (DoAlloc)
		{
		}
	}
	//AfterTrail		: generate code after the native block end (after the ret) , can be used to emit helper functions (other register allocators may emit code after that tho)
	virtual void AfterTrail()
	{
		if (DoAlloc)
		{
		}
	}
	//IsRegAllocated	: *couh* yea .. :P
	virtual bool IsRegAllocated(u32 sh4_reg)
	{
		ensure_valid(sh4_reg);
		return GetInfo(sh4_reg-fr_0)!=0;
	}
	//Carefull w/ register state , we may need to implement state push/pop
	//GetRegister		: Get the register , needs flag for mode
	virtual x86SSERegType GetRegister(x86SSERegType d_reg,u32 sh4_reg,u32 mode)
	{
		if (DoAlloc)
		{
		}
		x86e->SSE_MOVSS_M32_to_XMM(d_reg,GetRegPtr(sh4_reg));
		return d_reg;
	}
	//Save registers
	virtual void SaveRegister(u32 reg,x86SSERegType from)
	{
		if (DoAlloc)
		{
		}
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(reg),from);
	}
	
	virtual void SaveRegister(u32 reg,float* from)
	{
		if (DoAlloc)
		{
		}
		x86e->SSE_MOVSS_M32_to_XMM(XMM0,(u32*)from);
		x86e->SSE_MOVSS_XMM_to_M32(GetRegPtr(reg),XMM0);
	}
	//FlushRegister		: write reg to reg location , and reload it on next use that needs reloading
	virtual void FlushRegister(u32 sh4_reg)
	{
		if (IsRegAllocated(sh4_reg))
		{
		}
	}
	virtual void FlushRegCache()
	{
		if (DoAlloc)
		{
			for (int i=0;i<16;i++)
				FlushRegister(fr_0+i);
		}
	}
	//WriteBackRegister	: write reg to reg location
	virtual void WriteBackRegister(u32 sh4_reg)
	{
		if (IsRegAllocated(sh4_reg))
		{
		}
	}
	//ReloadRegister	: read reg from reg location , discard old result
	virtual void ReloadRegister(u32 sh4_reg)
	{
		if (IsRegAllocated(sh4_reg))
		{
		}
	}
};

FloatRegAllocator * GetFloatAllocator()
{
	return new SimpleSSERegAlloc();
}