/*
**	TA_Param.h - David Miller 2006 - PowerVR2 Emulation Library
*/

#ifndef __POWERVR2_H__
#define __POWERVR2_H__

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

} * PvrIf;

#include "PowerVR2_GL.h"
#include "PowerVR2_D3D.h"


#endif //__POWERVR2_H__

