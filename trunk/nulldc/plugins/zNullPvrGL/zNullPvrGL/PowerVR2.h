/*
**	PowerVR2.h - David Miller 2006 - PowerVR2 Emulation Library
*/

#ifndef __POWERVR2_H__
#define __POWERVR2_H__	

//#include <cg\cg.h>	
#include "PowerVR2_GL.h"
#include "PowerVR2_D3D.h"


typedef void PvrCallFP();
typedef bool bPvrCallFP();

namespace PvrIf
{
	extern u32 FrameCount;
	extern TextureCache TCache;
	extern vector<GlobalParam> GlobalParams;

	S_INLINE void AppendStrip (VertexParam *vp)
	{
		DCache * pDList = NULL;

		switch(vp->pcw.ListType)	{
		case LT_Opaque:			pDList = &Opaque;		break;
		case LT_Translucent:	pDList = &Transparent;	break;
		case LT_PunchThrough:	pDList = &PunchThru;	break;
		default: ASSERT_T((1),"PvrIf::AppendStrip() LType Default");	return;
		}
		pDList->AppendVert(vp);
	}
//	u32 AppendSprite(VertexParam *vp);

	extern bPvrCallFP	* Init;
	extern PvrCallFP	* Term;
	extern PvrCallFP	* Render;
	extern PvrCallFP	* Resize;
};







#endif //__POWERVR2_H__





/*
	static CGcontext   cgContext;
	static CGprofile   cgVProfile;
	static CGprogram   cgVProgram;
	static CGprofile   cgFProfile;
	static CGprogram   cgFProgram;

	static CGparameter cgVertexParam_modelViewProj, cgFragmentParam_c;

	static void checkForCgError(const char * where)
	{
		CGerror error;
		const char *string = cgGetLastErrorString(&error);

		if (error != CG_NO_ERROR) {
			printf("ERROR in Cg while: %s : %s\n", where, string);
			if (error == CG_COMPILER_ERROR) {
				printf("%s\n", cgGetLastListing(cgContext));
			}
			exit(1);
		}
	}

	static void InitCg()
	{
		cgContext = cgCreateContext();
		checkForCgError("creating context");
	}
*/