#include "types.h"

#ifdef MSCPP
#define fix 1
#else
#define fix 0
#endif
void _typesystem_check()
{
	//make sure the types are >= their suposed sizes
	u8 tu8[1-sizeof(u8)+fix];
	u8 tu16[2-sizeof(u16)+fix];
	u8 tu32[4-sizeof(u32)+fix];
	u8 tuptr[sizeof(void*)-sizeof(uptr)+fix];

	u8 ts8[1-sizeof(s8)+fix];
	u8 ts16[2-sizeof(s16)+fix];
	u8 ts32[4-sizeof(s32)+fix];
	u8 tsptr[sizeof(void*)-sizeof(sptr)+fix];

	//make sure the types are <= their suposed sizes
	u8 tmu8[sizeof(u8)-1+fix];
	u8 tmu16[sizeof(u16)-2+fix];
	u8 tmu32[sizeof(u32)-4+fix];
	u8 tmuptr[sizeof(uptr)-sizeof(void*)+fix];

	u8 tms8[sizeof(s8)-1+fix];
	u8 tms16[sizeof(s16)-2+fix];
	u8 tms32[sizeof(s32)-4+fix];
	u8 tmsptr[sizeof(sptr)-sizeof(void*)+fix];
}
