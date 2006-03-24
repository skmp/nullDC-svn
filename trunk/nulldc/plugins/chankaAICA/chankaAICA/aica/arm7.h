////////////////////////////////////////////////////////////////////////////////////////
/// @file  arm7.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef ARM7_H_
#define ARM7_H_

////////////////////////////////////////////////////////////////////////////////////////
/// @class  CArm7
/// @brief  
////////////////////////////////////////////////////////////////////////////////////////
class CArm7
{
public:
	CArm7   () : m_bInit(false)     {}
	~CArm7  ()                      { CArm7::End(); }

	TError  Init    ();
	bool    IsOk    () const                { return m_bInit; }
	void    End     ();


	enum
	{
		R13_IRQ=18,
		R14_IRQ=19,
		SPSR_IRQ =20,
		R13_USR=26,
		R14_USR=27,
		R13_SVC=28,
		R14_SVC=29,
		SPSR_SVC =30,
		R13_ABT=31,
		R14_ABT=32,
		SPSR_ABT =33,
		R13_UND=34,
		R14_UND=35,
		SPSR_UND =36,
		R8_FIQ= 37,
		R9_FIQ= 38,
		R10_FIQ=39,
		R11_FIQ=40,
		R12_FIQ=41,
		R13_FIQ=42,
		R14_FIQ=43,
		SPSR_FIQ =44,
	};

	enum EErrorCode
	{
		E_OK,
		E_ACCESS_MEM_NOT_VALID,
		E_BREAKPOINT,
		E_OPCODE_NOT_VALID,
	};

	EErrorCode ExecuteInstruction();



	typedef union {
		struct {
			BYTE B0;
			BYTE B1;
			BYTE B2;
			BYTE B3;
		} B;
		struct {
			WORD W0;
			WORD W1;
		} W;
		DWORD I;
	} reg_pair;

	EErrorCode		StepArm7() 
	{ 
		if (!m_bEnable)
			return E_OK;
		return _StepArm7();
	}

	EErrorCode		_StepArm7();

	EErrorCode		BlockStepArm7(DWORD uNumCycles)
	{
		if (!m_bEnable)
			return E_OK;

		return _BlockStepArm7(uNumCycles);
	}

	EErrorCode		_BlockStepArm7(DWORD uNumCycles);


	const reg_pair* GetRegisters() const { return m_aReg;}

	static reg_pair m_aReg[45];

	void SetPC(DWORD uData) { m_aReg[15].I = uData; m_uArmNextPC = uData;}
	

	DWORD GetPC() const { return m_uArmNextPC;	}
	bool IsArmState() const { return armState;}

	void SetEnable(bool bEnable) 
	{ 
		m_bEnable = bEnable;
		if (!bEnable)
			Reset();
	}

	__forceinline void SetErrorCode(EErrorCode uError) { m_eErrorCode = uError; }
	__forceinline EErrorCode GetErrorCode() const { return m_eErrorCode;}
	


	void CPUSwitchMode(int mode, bool saveState, bool breakLoop=true);
	void CPUUpdateCPSR();
	void CPUUpdateFlags(bool breakLoop = true);
	void CPUSoftwareInterrupt(int comment);
	void CPUUndefinedException();

	static int memoryWait[16];
	static int memoryWait32[16];
	static int memoryWaitSeq[16];
	static int memoryWaitSeq32[16];
	static int memoryWaitFetch[16];
	static int memoryWaitFetch32[16];
	static const int cpuMemoryWait[16];
	static const int cpuMemoryWait32[16];
	static const bool memory32[16];

	static bool N_FLAG;
	static bool Z_FLAG;
	static bool C_FLAG;
	static bool V_FLAG;
	static bool armIrqEnable;
	static bool armState;
	static int armMode;

	static BYTE cpuBitsSet[256];

	static const int thumbCycles[];

	static WORD IE;
	static WORD IF;
	static WORD IME;
	static int cpuSavedTicks;
	static int* extCpuLoopTicks;
	static int* extClockTicks;
	static int* extTicks;
	static bool intState;
	static bool stopState;
	static bool holdState;
	static bool armFiqEnable;

	void Reset();	
	void CPUInterrupt();
	void CPUFiq();

	void RaiseInterrupt();

	void SetStopBreakPoint(DWORD uPos) { m_uStop = uPos;}

	
private:	
	bool    m_bInit;
	bool m_bEnable;
	DWORD m_uArmNextPC;
	DWORD m_uClockTicks;
	DWORD m_uStop;
	EErrorCode m_eErrorCode;
	bool m_bFiqPending;
};

#endif
