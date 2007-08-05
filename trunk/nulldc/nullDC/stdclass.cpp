#include "types.h"
#include <string.h>
#include <windows.h>
#include "dc/mem/_vmem.h"
#include "plugins/plugin_manager.h"

//comonly used classes across the project

//bah :P since it's template needs to be on .h pfftt
//anyhow , this is needed here ;P
u32 Array_T_id_count=0;

u32 fastrand_seed=0xDEADCAFE;

u32 fastrand()
{
	fastrand_seed=(fastrand_seed>>9)^(fastrand_seed<<11)^(fastrand_seed>>24);//^1 is there 
	return fastrand_seed++;//if it got 0 , take good care of it :)
}

//Misc function to get relative source directory for printf's
char temp[1000];
char* GetNullDCSoruceFileName(char* full)
{
	size_t len = strlen(full);
	while(len>18)
	{
		if (full[len]=='\\')
		{
			memcpy(&temp[0],&full[len-14],15*sizeof(char));
			temp[15*sizeof(char)]=0;
			if (strcmp(&temp[0],"\\nulldc\\nulldc\\")==0)
			{
				strcpy(temp,&full[len+1]);
				return temp;
			}
		}
		len--;
	}
	strcpy(temp,full);
	return &temp[0];
}

char* GetPathFromFileNameTemp(char* full)
{
	size_t len = strlen(full);
	while(len>2)
	{
		if (full[len]=='\\')
		{
			memcpy(&temp[0],&full[0],(len+1)*sizeof(char));
			temp[len+1]=0;
			return temp;	
		}
		len--;
	}
	strcpy(temp,full);
	return &temp[0];
}

void GetPathFromFileName(char* path)
{
	strcpy(path,GetPathFromFileNameTemp(path));
}

void GetFileNameFromPath(char* path,char* outp)
{
	
	size_t i=strlen(path);
	
	while (i>0)
	{
		i--;
		if (path[i]=='\\')
		{
			strcpy(outp,&path[i+1]);
			return;
		}
	}

	strcpy(outp,path);
}

char AppPath[1024];
void GetApplicationPath(char* path,u32 size)
{
	if (AppPath[0]==0)
	{
		GetModuleFileNameA(NULL,&AppPath[0],1024);
		GetPathFromFileName(AppPath);
	}

	strcpy(path,AppPath);
}

char* GetEmuPath(char* subpath)
{
	char* temp=(char*)malloc(1024);
	GetApplicationPath(temp,1024);
	strcat(temp,subpath);
	return temp;
}

//Windoze Code implementation of commong classes from here and after ..

//Thread class
cThread::cThread(ThreadEntryFP* function,void* prm)
{
	Entry=function;
	param=prm;
	hThread=CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)function,prm,CREATE_SUSPENDED,NULL);
}
cThread::~cThread()
{
	//gota think of something !
}
	
void cThread::Start()
{
	ResumeThread(hThread);
}
void cThread::Suspend()
{
	SuspendThread(hThread);
}
void cThread::WaitToEnd(u32 msec)
{
	WaitForSingleObject(hThread,msec);
}
//End thread class

//cResetEvent Calss
cResetEvent::cResetEvent(bool State,bool Auto)
{
		hEvent = CreateEvent( 
        NULL,             // default security attributes
		Auto?FALSE:TRUE,  // auto-reset event?
		State?TRUE:FALSE, // initial state is State
        NULL			  // unnamed object
        );
}
cResetEvent::~cResetEvent()
{
	//Destroy the event object ?
	 CloseHandle(hEvent);
}
void cResetEvent::Set()//Signal
{
	SetEvent(hEvent);
}
void cResetEvent::Reset()//reset
{
	ResetEvent(hEvent);
}
void cResetEvent::Wait(u32 msec)//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,msec);
}
void cResetEvent::Wait()//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,(u32)-1);
}
//End AutoResetEvent


//Dll loader/unloader
//.ctor
cDllHandler::cDllHandler()
{
	lib=0;
}

//.dtor
cDllHandler::~cDllHandler()
{
	if (lib)
	{
		#ifdef DEBUG_DLL
		EMUWARN("cDllHandler::~cDllHandler() -> dll still loaded , unloading it..");
		#endif
		Unload();
	}
}

bool cDllHandler::Load(char* dll)
{
	lib=LoadLibraryA(dll);
	if (lib==0)
	{
	#ifdef DEBUG_DLL
		EMUERROR2("void cDllHandler::Load(char* dll) -> dll %s could not be loaded",dll);
	#endif
	}

	return IsLoaded();
}

bool cDllHandler::IsLoaded()
{
	return lib!=0;
}

void cDllHandler::Unload()
{
	if (lib==0)
	{
		#ifdef DEBUG_DLL
		EMUWARN("void cDllHandler::Unload() -> dll is not loaded");
		#endif
	}
	else
	{
		u32 rv =FreeLibrary((HMODULE)lib);
		if (!rv)
		{
			printf("FreeLibrary -- failed %d\n",GetLastError());
		}
		lib=0;
	}
}

void* cDllHandler::GetProcAddress(char* name)
{
	if (lib==0)
	{
		EMUERROR("void* cDllHandler::GetProcAddress(char* name) -> dll is not loaded");
		return 0;
	}
	else
	{
		void* rv = ::GetProcAddress((HMODULE)lib,name);

		if (rv==0)
		{
			//EMUERROR3("void* cDllHandler::GetProcAddress(char* name) :  Export named %s is not found on lib %p",name,lib);
		}

		return rv;
	}
}

//File Enumeration
void FindAllFiles(FileFoundCB* callback,char* dir,void* param)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char DirSpec[MAX_PATH + 1];  // directory specification
	DWORD dwError;

	strncpy (DirSpec, dir, strlen(dir)+1);
	//strncat (DirSpec, "\\*", 3);

	hFind = FindFirstFileA( DirSpec, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	{
		return;
	} 
	else 
	{

		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
		{
			callback(FindFileData.cFileName,param);
		}
u32 rv;
		while ( (rv=FindNextFileA(hFind, &FindFileData)) != 0) 
		{ 
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
			{
				callback(FindFileData.cFileName,param);
			}
		}
		dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES) 
		{
			return ;
		}
	}
	return ;
}



void VArray::Init(u32 sz)
{
	size=sz;
	data=(u8*)VirtualAlloc(0 , size,MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}
void VArray::Term()
{
	VirtualFree(data,0,MEM_RELEASE);
}
void VArray::LockRegion(u32 offset,u32 size)
{
	DWORD old;
	VirtualProtect(((u8*)data)+offset , size, PAGE_READONLY,&old);
}
void VArray::UnLockRegion(u32 offset,u32 size)
{
	DWORD old;
	VirtualProtect(((u8*)data)+offset , size, PAGE_READWRITE,&old);
}
/*
void VArray2::Init(void* dta,u32 sz)
{
	size=sz;
	data=(u8*)dta;
	void* rv=VirtualAlloc(data,sz,MEM_COMMIT,PAGE_READWRITE);
	verify(rv!=0);
}
void VArray2::Term()
{
	VirtualFree(data,size,MEM_DECOMMIT);
	size=0;
	data=0;
}*/
void VArray2::LockRegion(u32 offset,u32 size)
{
	DWORD old;
	VirtualProtect(((u8*)data)+offset , size, PAGE_READONLY,&old);
}
void VArray2::UnLockRegion(u32 offset,u32 size)
{
	DWORD old;
	VirtualProtect(((u8*)data)+offset , size, PAGE_READWRITE,&old);
}


bool VramLockedWrite(u8* address);
bool RamLockedWrite(u8* address,u32* sp);
extern u8* DynarecCache;
extern u32 DynarecCacheSize;
int ExeptionHandler(u32 dwCode, void* pExceptionPointers)
{
	EXCEPTION_POINTERS* ep=(EXCEPTION_POINTERS*)pExceptionPointers;
	
	EXCEPTION_RECORD* pExceptionRecord=ep->ExceptionRecord;

	if (dwCode != EXCEPTION_ACCESS_VIOLATION)
		return EXCEPTION_CONTINUE_SEARCH;
	
	u8* address=(u8*)pExceptionRecord->ExceptionInformation[1];

	if (VramLockedWrite(address))
		return EXCEPTION_CONTINUE_EXECUTION;
	else if (RamLockedWrite(address,(u32*)ep->ContextRecord->Esp))
		return EXCEPTION_CONTINUE_EXECUTION;
	else if (((u32)(address-sh4_reserved_mem))<(512*1024*1024))
	{
		//k
		//
		//cmp ecx,mask1
		//jae full_lookup
		//and ecx,mask2
		//the write 
		u32 pos=ep->ContextRecord->Eip; //<- the write
		if (((u32)(pos-(u32)DynarecCache))<DynarecCacheSize)
		{
			u32* ptr_jae_offset=(u32*)(pos-4-6);
			u8* offset_2=(u8*)(*ptr_jae_offset + pos -6-2);
			u8* ptr_cmp=(u8*)(pos-6-6-6);
			*ptr_cmp=0xE9;
			*(u32*) (ptr_cmp+1)=*ptr_jae_offset+7 + offset_2[0];
			ep->ContextRecord->Eip=(pos-6-6-6- offset_2[1]);
			//printf("Patched %08X,%08X<-%08X %d %d\n",ep->ContextRecord->Eip,ep->ContextRecord->Ecx,offset_2[0],offset_2[1]);
			//		ep->ContextRecord->Ecx=ep->ContextRecord->Eax;
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		else
		{
			printf("[GPF]Unhandled access to : 0x%X\n",address);
			return EXCEPTION_CONTINUE_SEARCH;
		}
	}
	else
	{
		printf("[GPF]Unhandled access to : 0x%X\n",address);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

int msgboxf(char* text,unsigned int type,...)
{
	va_list args;

	char temp[2048];
	va_start(args, type);
	vsprintf(temp, text, args);
	va_end(args);

	if (libgui.MsgBox!=0)
	{
		return libgui.MsgBox(temp,type);
	}
	else
		return MessageBox(NULL,temp,VER_SHORTNAME,type | MB_TASKMODAL);
}