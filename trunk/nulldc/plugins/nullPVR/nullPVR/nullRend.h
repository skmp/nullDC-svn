/*
**	nullRendGL.h,	nullDC::nullPVR	(2007) ZeZu & drk||Raziel
*/
#ifndef __NULLREND_H__
#define __NULLREND_H__

#ifndef __NULLPVR_H__
#error	"#include nullPvr.h before this file!"
#endif

typedef s32  FASTCALL InitFP(void * handle);
typedef void FASTCALL TermFP(void * handle);
typedef void FASTCALL ResizeFP(void * handle);
typedef void FASTCALL RenderFP(void * buffer);
typedef void FASTCALL SetStateFP(void * state);

struct RenderInterface {
	InitFP		* nrInit;
	TermFP		* nrTerm;
	ResizeFP	* nrResize;
	RenderFP	* nrRender;
	SetStateFP	* nrSetState;
	// TODO, Add Texture Load/Del
};


#include "nullRendGL.h"
#include "nullRendSoft.h"

#ifndef linux
#include "nullRendD3D.h"
#endif

#endif	//__NULLREND_H__
