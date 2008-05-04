#include <stdarg.h>
#include <vadefs.h>
#include <stdio.h>

void sysf(char* fmt,...)
{
	va_list	ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}
void dbgf(char* fmt,...)
{
	va_list	ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}