/*
**
*/
#include <windows.h>
#include "zNullMaple.h"




ConfigLoadStrFP	* ConfigLoadStr;
ConfigSaveStrFP	* ConfigSaveStr;


HINSTANCE hInst=NULL;

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD   ul_reason_for_call,
					  LPVOID  lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


/*
**	NullDC specific Interface,
**	returns valid fucntion ptrs
*/

extern "C" __declspec(dllexport) 
void dcGetPluginInfo(ndcPluginIf* If)
{
	If->dwIfVersion	= 0x00020000;	// double check with nullgdr
	If->dwLibVersion= 0x01;			// 
	If->dwPluginType= 0x08;			// 8=Maple
	strcpy_s(If->szName, "zNullMaple, Maple bus Plugin By _ZeZu_ [" __DATE__ "]");

	If->Init		= mplInit;
	If->Term		= mplTerm;
	If->Reset		= mplReset;
	If->ThreadInit	= mplThreadInit;
	If->ThreadTerm	= mplThreadTerm;
	If->Config		= mplConfig;

	ConfigLoadStr	= If->ConfigLoadStr;
	ConfigSaveStr	= If->ConfigSaveStr;
}







void mplTerm(DWORD){}
void mplInit(void*,DWORD){}
void mplReset(bool,DWORD){}
void mplThreadInit(DWORD){}
void mplThreadTerm(DWORD){}
void mplConfig(DWORD,void*){}
