#include "types.h"

struct RegInfo
{
	u32 id;
};

struct Label
{
	u32 nil;
};

enum ConditionCode
{
	nz,
	z
};

struct CpuInfo
{
	//
	u32 nil;
};

class Emmiter
{
public :
	void GetCpuInfo(CpuInfo& cpuinfo);
	void DoLabelPatches();

	//bah moves
	void SetRegister(RegInfo to,u32 value);
	void SetRegister(RegInfo to,u8* ptr);
	void SetRegister(RegInfo to,u16* ptr);
	void SetRegister(RegInfo to,u32* ptr);
	void SetRegister(RegInfo to,RegInfo from);
	
	//basic maths
	//base+=value
	void Add(RegInfo base,RegInfo value);
	void Add(RegInfo base,u32 value);

	//base-=value
	void Sub(RegInfo base,RegInfo value);
	void Sub(RegInfo base,u32 value);

	//base*=vale ; signed
	void sMul(RegInfo base,RegInfo value);
	void sMul(RegInfo base,s32 value);

	//base*=vale ; unsigned
	void uMul(RegInfo base,RegInfo value);
	void uMul(RegInfo base,u32 value);

	//base&=value
	//Bitwise operations
	void And(RegInfo base,RegInfo value);
	void And(RegInfo base,u32 value);

	//base|=value
	void Or(RegInfo base,RegInfo value);
	void Or(RegInfo base,u32 value);

	//base^=value
	void Xor(RegInfo base,RegInfo value);
	void Xor(RegInfo base,u32 value);

	//base =!base
	void Not(RegInfo base);
	


	//basic logical structures
	void StartIf(RegInfo r1,RegInfo r2,ConditionCode cc);
	void StartIf(RegInfo r1,ConditionCode cc);
	void StartIf(ConditionCode cc);

	void ElseIf(RegInfo r1,RegInfo r2,ConditionCode cc);
	void ElseIf(RegInfo r1,ConditionCode cc);
	void ElseIf(ConditionCode cc);

	void Else(RegInfo r1,RegInfo r2,ConditionCode cc);
	void Else(RegInfo r1,ConditionCode cc);
	void Else(ConditionCode cc);

	void EndIf(RegInfo r1,RegInfo r2,ConditionCode cc);
	void EndIf(RegInfo r1,ConditionCode cc);
	void EndIf(ConditionCode cc);

	//Labels
	Label GenLabel();
	void MarkLabel(Label& label);

	//jumps
	void Jump(Label& label);
	void JumpCond(RegInfo r1,RegInfo r2,ConditionCode cc,Label& label);
	void JumpCond(RegInfo r1,ConditionCode cc,Label& label);
	void JumpCond(ConditionCode cc,Label& label);
	
};