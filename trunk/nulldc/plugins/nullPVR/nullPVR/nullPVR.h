/*
**	nullPVR.h,	nullDC::nullPVR	(2007) ZeZu & drk||Raziel
*/
#ifndef __NULLPVR_H__
#define __NULLPVR_H__

#include <stdio.h>

#ifndef linux
#include <windows.h>
#else
#error "FIXME - EWW LINUZ EWW"
#endif


#include "plugins/plugin_header.h"




/*
**	Minor debugging facilities
*/

//#ifdef _DEBUG
#define DEBUG_LIB (1)
//#endif

#ifdef DEBUG_LIB
#define INLINE		inline
#define S_INLINE	static inline
#define ASSERT_T(cond,str) if((cond)) printf("#!T\tERROR: ASSERTION FAILED: %s !\n", str);
#define ASSERT_F(cond,str) if(!(cond)) printf("#!F\tERROR: ASSERTION FAILED: %s !\n", str);
#else
#define INLINE		__forceinline
#define S_INLINE	static __forceinline
#define ASSERT_T(cond,str)
#define ASSERT_F(cond,str)
#endif





/*
**	Prototypes for these ugly bastards
*/

s32  FASTCALL pvrInit(pvr_init_params*);
void FASTCALL pvrTerm();
void FASTCALL pvrReset(bool);
s32  FASTCALL pvrLoad(emu_info*,u32);
void FASTCALL pvrUnload();
void FASTCALL pvrConfig(void*);

void FASTCALL pvrWriteFifo(u32 address, u32* data, u32 size);
void FASTCALL pvrLockCB(vram_block *bl, u32 addr);



#define dbgbreak __asm {int 3}

#define verify(x) if((x)==false){ printf("Verify Failed  : " #x "\n in %s -> %s : %d \n",__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define die(reason) { printf("Fatal error : %s\n in %s -> %s : %d \n",reason,__FUNCTION__,__FILE__,__LINE__); dbgbreak;}
#define fverify verify
#define fastcall FASTCALL

#endif //__NULLPVR_H___

