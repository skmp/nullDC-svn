#include "dc.h"
#include "dc\mem\mmap.h"

void dcInit()
{
	sh4MemInit();
}
void dcReset(bool phys)
{
	sh4MemReset(phys);
}
void dcResume()
{
}
void dcPause()
{
}
void dcTerm()
{
	sh4MemTerm();
}
