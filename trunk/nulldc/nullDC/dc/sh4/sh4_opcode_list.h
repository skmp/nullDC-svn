#pragma once
#include "types.h"
#include "sh4_interpreter.h"
#include "dc\sh4\rec_v1\rec_v1_basicblock.h"

extern OpCallFP* OpPtr[0x10000];
extern RecOpCallFP* RecOpPtr[0x10000];
extern OpcodeType OpTyp[0x10000];

typedef void OpDissasmFP(char* out,const char* const FormatString,u32 pc,u16 opcode);

struct sh4_opcodelistentry
{
	RecOpCallFP* rec_oph;
	OpCallFP* oph;
	u32 mask;
	u32 rez;
	OpcodeType type;
	OpDissasmFP* dissasm;
	char disasm1[64];

	void Dissasemble(char* strout,u32 pc , u16 params) const
	{
		dissasm(strout,&disasm1[0],pc,params);
	}

	INLINE bool SetPC() const
	{
		return (type & WritesPC)!=0;
	}

	INLINE bool NeedPC() const
	{
		return (type & ReadsPC)!=0;
	}

	INLINE bool SetSR() const
	{
		return (type & WritesSR)!=0;
	}

	INLINE bool SetFPSCR() const
	{
		return (type & WritesFPSCR)!=0;
	}
	
};

#define ExecuteOpcode(op) {OpPtr[op](op);}

void BuildOpcodeTables();
void DissasembleOpcode(u16 opcode,u32 pc,char* Dissasm);