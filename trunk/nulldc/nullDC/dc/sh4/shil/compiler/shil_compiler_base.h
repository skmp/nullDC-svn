#include "types.h"
#include "dc\sh4\shil\shil.h"
#include "emitter\emitter.h"
#include "emitter\regalloc\x86_sseregalloc.h"

extern bool nullprof_enabled;
extern u32 T_jcond_value;
extern u32 reg_pc_temp_value;

void shil_compiler_init(x86_block* block,IntegerRegAllocator* ira,FloatRegAllocator* fra);
void shil_compile(shil_opcode* op);