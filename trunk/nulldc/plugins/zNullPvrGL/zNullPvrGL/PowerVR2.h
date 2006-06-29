/*
**	PowerVR2.h - David Miller 2006 - PowerVR2 Emulation Library
*/

#ifndef __POWERVR2_H__
#define __POWERVR2_H__

#include <cg\cg.h>														// NEW: Cg Header

#define USE_VERTEX_PROGRAMS
//#define USE_FRAGMENT_PROGRAMS


#include "zNullPvr.h"

#include "TA_Regs.h"
#include "TA_Param.h"
#include "TA_Texture.h"


extern 
class PowerVR2
	: public PrimConverter, public TextureCache
{
public:

	virtual bool Init() { ASSERT_T((1),"PowerVR2 Not Subclassed!"); return false; }
	virtual void Term() { ASSERT_T((1),"PowerVR2 Not Subclassed!"); }
	virtual void Render() { ASSERT_T((1),"PowerVR2 Not Subclassed!"); }
	virtual void Resize() { ASSERT_T((1),"PowerVR2 Not Subclassed!"); }
	u32 FrameCount;


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


} * PvrIf;

#include "PowerVR2_GL.h"
#include "PowerVR2_D3D.h"


#endif //__POWERVR2_H__

