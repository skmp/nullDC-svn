// nDisasm.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	return TRUE;
}
extern "C"
{
	long disasm(unsigned char *data, char *output, int outbufsize, int segsize,
		long offset, int autosync, unsigned long prefer);

	__declspec(dllexport)  
		long __stdcall Disasm(unsigned char *data, char *output, int outbufsize, int segsize,
		long offset, int autosync, unsigned long prefer)
	{
		return disasm(data,output,outbufsize,segsize,offset,autosync,prefer);
	}

}