#pragma once
#include "emitter.h"

#define RALLOC_R  1
#define RALLOC_W  2
#define RALLOC_RW (RALLOC_R|RALLOC_W)

class IntegerRegAllocator
{
public :
	//methods needed
	//
	//DoAllocation		: do allocation on the block
	virtual void DoAllocation(rec_v1_BasicBlock* bb)
	{
		
	}
	//BeforeEmit		: generate any code needed before the main emittion begins (other register allocators may have emited code tho)
	virtual void BeforeEmit(emitter<>* x86e)
	{
		
	}
	//BeforeTrail		: generate any code needed after the main emittion has ended (other register allocators may emit code after that tho)
	virtual void BeforeTrail(emitter<>* x86e)
	{
		
	}
	//AfterTrail		: generate code after the native block end (after the ret) , can be used to emit helper functions (other register allocators may emit code after that tho)
	virtual void AfterTrail(emitter<>* x86e)
	{
		
	}

	//IsRegAllocated	: *couh* yea .. :P
	virtual bool IsRegAllocated(u8 sh4_reg)
	{
		return false;
	}

	//Carefull w/ register state , we may need to implement state push/pop
	//GetRegister		: Get the register , needs flag if it's read or write.
	virtual x86IntRegType GetRegister(u8 sh4_reg,u8 mode)
	{
		return GPR_Error;
	}

	//PushRegister		: push register to stack (if allocated)
	virtual void PushReg(x86IntRegType x86_reg)
	{
		
	}
	//PopRegister		: pop register from stack (if allocated)
	virtual void PopReg(x86IntRegType x86_reg)
	{
		
	}
	//FlushRegister		: write reg to reg location , and dealloc it
	virtual void FlushRegister(u8 sh4_reg)
	{
		
	}
	virtual void FlushRegCache()
	{
		
	}
	//WriteBackRegister	: write reg to reg location
	virtual void WriteBackRegister(u8 sh4_reg)
	{
		
	}
	//ReloadRegister	: read reg from reg location , discard old result
	virtual void ReloadRegister(u8 sh4_reg)
	{
		
	}
};


class FloatRegAllocator
{
public :
	//methods needed
	//
	//DoAllocation		: do allocation on the block
	virtual void DoAllocation(rec_v1_BasicBlock* bb)
	{
		
	}
	//BeforeEmit		: generate any code needed before the main emittion begins (other register allocators may have emited code tho)
	virtual void BeforeEmit(emitter<>* x86e)
	{
		
	}
	//BeforeTrail		: generate any code needed after the main emittion has ended (other register allocators may emit code after that tho)
	virtual void BeforeTrail(emitter<>* x86e)
	{
		
	}
	//AfterTrail		: generate code after the native block end (after the ret) , can be used to emit helper functions (other register allocators may emit code after that tho)
	virtual void AfterTrail(emitter<>* x86e)
	{
		
	}

	//IsRegAllocated	: *couh* yea .. :P
	virtual bool IsRegAllocated(u8 sh4_reg)
	{
		return false;
	}

	//Carefull w/ register state , we may need to implement state push/pop
	//GetRegister		: Get the register , needs flag if it's read or write.
	virtual x86SSERegType GetRegister(u8 sh4_reg,u8 mode)
	{
		return XMM_Error;
	}

	//PushRegister		: push register to stack (if allocated)
	virtual void PushReg(x86SSERegType sse_reg)
	{
		
	}
	//PopRegister		: pop register from stack (if allocated)
	virtual void PopReg(x86SSERegType sse_reg)
	{
		
	}
	//FlushRegister		: write reg to reg location , and dealloc it
	virtual void FlushRegister(u8 sh4_reg)
	{
		
	}
	virtual void FlushRegCache()
	{
		
	}
	//WriteBackRegister	: write reg to reg location
	virtual void WriteBackRegister(u8 sh4_reg)
	{
		
	}
	//ReloadRegister	: read reg from reg location , discard old result
	virtual void ReloadRegister(u8 sh4_reg)
	{
		
	}
};