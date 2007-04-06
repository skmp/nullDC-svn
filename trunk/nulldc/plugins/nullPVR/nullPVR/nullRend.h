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

struct RenderInterface
{
	InitFP		* nrInit;
	TermFP		* nrTerm;
	ResizeFP	* nrResize;
	RenderFP	* nrRender;
	// TODO, Add Texture Load/Del
};
extern RenderInterface * nRendIf;

/*
**	The texture interface is going to be a bit strange
**	the emulator can be optimized in many cases for things
**	that reload textures in real-time (effects or rend.to.tex)
**	The idea is that we can abstract much of the manual work,
**	and have input/output formats, input = dc formats,
**	output could be the same as input, just detwiddled,
**	and in a linear format.  nVidia cards are known to save
**	textures internally as 16b to save vram when there are
**	no hints to do otherwise.  OpenGL and D3D can be made to
**	load formats like 4444, 1555, and 565.
*/


enum TEX_INP_FMT
{
	TXI_1555,
	TXI_565,
	TXI_4444,
	TXI_32B,

}


typedef s32  FASTCALL UploadTexFP(TEX_INP_FMT in, TEX_OUTP_FMT out, u32 addr);

struct TexInterface
{

};

#include "nullRendGL.h"
#include "nullRendSoft.h"

#ifndef linux
#include "nullRendD3D.h"
#endif

#endif	//__NULLREND_H__
