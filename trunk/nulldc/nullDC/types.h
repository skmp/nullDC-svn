#pragma once

//this looks like the main header ? -Z
#define BUILD_DREAMCAST	(1)
//#define BUILD_DEV_UNIT	(1)
//#define BUILD_NAOMI	(1)


//SHUT UP M$ COMPILER !@#!@$#
#ifdef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#endif

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#ifdef _CRT_SECURE_NO_DEPRECATE
#undef _CRT_SECURE_NO_DEPRECATE
#endif

#define _CRT_SECURE_NO_DEPRECATE 

//Basic types :)
#include "basic_types.h"
#include <vector>
using namespace std;

#define No_gdb_stub
//#define DEBUG_DLL

//Do not complain when i use enum::member
#pragma warning( disable : 4482)

//unnamed struncts/unions
#pragma warning( disable : 4201)

//unused parameters
#pragma warning( disable : 4100)

//basic includes from runtime lib
#include <stdlib.h>
#include <stdio.h>

//used for asm-olny functions
#ifdef X86
#define naked   __declspec( naked )
#else
#define naked
#endif


/*
	Preprocecor fun !

	DEBUG is defined for debug version of the emu (meaning , for thr emu code to be debuged within the VS ide)
	DEBUG          -> project is compiled on debug mode , enable all breaks/debug olny checks

	TRACE/TRACE* is defined to trace errors , both on debuger and out of debuger (eg , when debuging an emulated app)
	So far:
	TRACE		   -> we are on general trace mode , look for and log all emulation wanrings/errors
	TRACE_DO_BREAK -> if we found an error and we are on debug mode then do a breakpoint
	
	MEM_*  is defined to  note a debug part of code using mem , mailny for buffer underruns
	So far :
	MEM_ALLOC_CHECK -> automagicaly checks mallocs and reallocs for success
	MEM_BOUND_CHECK -> automagicaly checks bounds on Array/Stack/*list types (stdclass.cpp)
	MEM_ERROR_BREAK -> breaks when an error is detected (int 3)
	MEM_DO_BREAK    -> does a break :P

*/

//On release we have no checks
#ifndef RELEASE
	#define MEM_ALLOC_CHECK
	#define MEM_BOUND_CHECK
	#define MEM_ERROR_BREAK
#endif

#ifdef MEM_ALLOC_CHECK

	extern  void * debug_malloc(size_t size);
	extern  void * debug_realloc(void* ptr,size_t size);
	extern  void debug_free(void* ptr);

	#define malloc debug_malloc
	#define realloc debug_realloc
	#define free debug_free

	//enable bound checks if on MEM_ALLOC_CHECK mode
	#ifndef MEM_BOUNDCHECK
	#define MEM_BOUNDCHECK
	#endif

#endif

#if DEBUG
//force
#define INLINE
//sugest
#define SINLINE
#else
//force
#define INLINE __forceinline
//sugest
#define SINLINE __inline
#endif
//no inline :)
#define NOINLINE __declspec(noinline)

#ifdef MEM_ERROR_BREAK
	#ifdef X86
		#define MEM_DO_BREAK {__asm { int 03}}
	#else
		#define MEM_DO_BREAK {printf("**Mem Error Break**\n");getc(stdin);}
	#endif
#else
	#define MEM_DO_BREAK
#endif

#ifdef TRACE
	#ifdef DEBUG
		#ifdef X86
			#define TRACE_DO_BREAK {__asm { int 03}}
		#else
			#define TRACE_DO_BREAK {printf("**Trace Break**\n");getc(stdin);}
		#endif
	#else
		#define TRACE_DO_BREAK
	#endif
#else
	#define TRACE_DO_BREAK
#endif

//basic includes
#include "stdclass.h"

#define EMUERROR(x)(printf("Error in %s:" __FUNCTION__":%d  -> " x"\n",GetNullDCSoruceFileName(__FILE__),__LINE__))
#define EMUERROR2(x,a)(printf("Error in %s:" __FUNCTION__":%d  -> " x"\n",GetNullDCSoruceFileName(__FILE__),__LINE__,a))
#define EMUERROR3(x,a,b)(printf("Error in %s:" __FUNCTION__":%d  -> " x"\n",GetNullDCSoruceFileName(__FILE__),__LINE__,a,b))
#define EMUERROR4(x,a,b,c)(printf("Error in %s:" __FUNCTION__":%d  -> " x"\n",GetNullDCSoruceFileName(__FILE__),__LINE__,a,b,c))

#define EMUWARN(x)(printf("Warning in %s:" __FUNCTION__":%d  -> " x "\n",GetNullDCSoruceFileName(__FILE__),__LINE__))
#define EMUWARN2(x,a)(printf("Warning in %s:"__FUNCTION__":%d  -> " x"\n",GetNullDCSoruceFileName(__FILE__),__LINE__,a))
#define EMUWARN3(x,a,b)(printf("Warning in %s:" __FUNCTION__":%d  -> " x"\n",GetNullDCSoruceFileName(__FILE__),__LINE__,a,b))
#define EMUWARN4(x,a,b,c)(printf("Warning in %s:" __FUNCTION__":%d  -> " x"\n",GetNullDCSoruceFileName(__FILE__),__LINE__,a,b,c))

#define BUILD_VER 0
#define MAJOR_VER 1
#define MINOR_VER 0

#define VER_EMUNAME		"nullDC"
#define VER_FULLNAME	VER_EMUNAME " v1.0.0 beta 2 (built " __DATE__ "@" __TIME__ ")"
#define VER_SHORTNAME	VER_EMUNAME " 1.0.0b2"

#define dbgbreak __asm {int 3}

#define fastcall __fastcall
#define FASTCALL __fastcall
#define verify(x) if((x)==false){ msgboxf("Verify Failed  : " #x "\n in %s -> %s : %d \n",MBX_ICONERROR,__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define die(reason) { msgboxf("Fatal error : %s\n in %s -> %s : %d \n",MBX_ICONERROR,reason,__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define fverify verify

//will be removed sometime soon
//This shit needs to be moved to proper headers
typedef u32  RegReadFP();
typedef void RegWriteFP(u32 data);
typedef u32  RegChangeFP();

enum RegStructFlags
{
	//Basic :
	REG_8BIT_READWRITE=1,	//used when doing direct reads from data for checks [not  on pvr , size is allways 32bits]
	REG_16BIT_READWRITE=2,	//used when doing direct reads from data for checks [not  on pvr , size is allways 32bits]
	REG_32BIT_READWRITE=4,	//used when doing direct reads from data for checks [not  on pvr , size is allways 32bits]
	REG_READ_DATA=8,		//we can read the data from the data member
	REG_WRITE_DATA=16,		//we can write the data to the data member
	REG_READ_PREDICT=32,	//we can call the predict function and know when the register will change
	//Extended :
	REG_WRITE_NOCHANGE=64,	//we can read and write to this register , but the write won't change the value readed 
	REG_CONST=128,			//register contains constant value
	REG_NOT_IMPL=256		//Register is not implemented/unkown
};

struct RegisterStruct
{
	union{
	u32* data32;					//stores data of reg variable [if used] 32b
	u16* data16;					//stores data of reg variable [if used] 16b
	u8* data8  ;					//stores data of reg variable [if used]	8b
	};
	RegReadFP* readFunction;	//stored pointer to reg read function
	RegWriteFP* writeFunction;	//stored pointer to reg write function
	RegChangeFP* NextCange;		//stored pointer to reg change calculation function
	u32 flags;					//flags for read/write
};

#define NO_MMU


struct __settings
{
	struct
	{
		bool Enable;
		bool CPpass;
		bool UnderclockFpu;
	} dynarec;
	
	struct
	{
		u32 cable;
	} dreamcast;
};
extern __settings settings;

void LoadSettings();
void SaveSettings();