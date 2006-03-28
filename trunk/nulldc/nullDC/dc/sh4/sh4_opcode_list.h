#pragma once
#include "types.h"
#include "sh4_interpreter.h"

extern OpCallFP* OpPtr[0x10000];
extern RecOpCallFP* RecOpPtr[0x10000];
extern OpcodeType OpTyp[0x10000];

typedef void OpDissasmFP(char* out,char* FormatString,u32 pc,u16 opcode);

struct sh4_opcodelistentry
{
	RecOpCallFP* rec_oph;
	OpCallFP* oph;
	u32 mask;
	u32 rez;
	OpcodeType type;
	OpDissasmFP* dissasm;
	char disasm1[64];
};

#define ExecuteOpcode(op) {OpPtr[op](op);}

void BuildOpcodeTables();
void DissasembleOpcode(u16 opcode,u32 pc,char* Dissasm);