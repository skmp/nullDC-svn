#include "emitter.h"
#include "windows.h"

x86_features x86_caps;

#ifndef EXCEPTION_EXECUTE_HANDLER
#define EXCEPTION_EXECUTE_HANDLER       1
#endif

x86_features::x86_features()
{
	sse_1=true;
	sse_2=true;
	sse_3=true;
	ssse_3=true;
	mmx=true;

	__try
	{
		__asm addps xmm0,xmm0
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		sse_1=false;
	}

	__try
	{
		__asm addpd xmm0,xmm0
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		sse_2=false;
	}

	__try
	{
		__asm addsubpd xmm0,xmm0
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		sse_3=false;
	}

	__try
	{
		__asm phaddw xmm0,xmm0
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		ssse_3=false;
	}

	
	__try
	{
		__asm paddd mm0,mm1
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		mmx=false;
	}

	if (mmx)
		__asm emms;


	char* command_line = GetCommandLine();

	if (ssse_3 && strstr(command_line,"-nossse3"))
	{
		printf("sSSE3 detected but disabled[-nossse3]\n");
		ssse_3=false;
	}
	if (sse_3 && strstr(command_line,"-nosse3"))
	{
		printf("SSE3 detected but disabled[-nosse3]\n");
		sse_3=false;
	}

	if (sse_2 && strstr(command_line,"-nosse2"))
	{
		printf("SSE2 detected but disabled[-nosse2]\n");
		sse_2=false;
	}

	if (sse_1 && strstr(command_line,"-nosse1"))
	{
		printf("SSE1 detected but disabled[-nosse1]\n");
		sse_1=false;
	}

	if (mmx && strstr(command_line,"-nommx"))
	{
		printf("MMX detected but disabled[-nommx]\n");
		mmx=false;
	}
	

	printf("Detected cpu features : ");
	if (mmx)
		printf("MMX ");
	if (sse_1)
		printf("SSE1 ");
	if (sse_2)
		printf("SSE2 ");
	if (sse_3)
		printf("SSE3 ");
	if (ssse_3)
		printf("sSSE3[ohh god , is that a name?] ");

	printf("\n");
	printf("\n");

	
}