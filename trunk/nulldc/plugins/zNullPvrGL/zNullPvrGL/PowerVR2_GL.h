/*
**	PowerVR2_GL.h - David Miller 2006 - PowerVR2 Emulation Library
*/

#ifndef __POWERVR2_GL_H__
#define __POWERVR2_GL_H__

#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
//#include <cg/cggl.h>													// NEW: Cg OpenGL Specific Header

#include "zNullPvr.h"

#include "TA_Regs.h"
#include "TA_Param.h"
#include "TA_Texture.h"


//#define USE_VBOS
//#define USE_VERTEX_ARRAYS

enum {
	vbo_opq=0,
	vbo_trs,
	vbo_ptu
};



bool InitGL();
void TermGL();
void RenderGL();
u32 RenderToTexGL();
void CheckErrorsGL(char *szFunc);
void ResizeGL(bool bDefault=true);


S_INLINE void TexFilterGL( GLuint filter )
{
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
}

S_INLINE void DC_TexEnv_Modulate(void)
{
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );

	glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE );

	glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PRIMARY_COLOR_ARB );
	glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE );
	//	glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT_ARB );	// specular

	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
	//	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_COLOR);		// specular

	glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB,	GL_REPLACE );
	glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB,	GL_TEXTURE );
	glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB,	GL_SRC_ALPHA);
}

S_INLINE void DC_TexEnv_DecalAlpha(void)
{
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );

	glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB );

	glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE );
	glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB );
	glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_TEXTURE );

	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB,	GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB,	GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB,	GL_SRC_ALPHA);

	glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE );
	glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_PRIMARY_COLOR_ARB );
	glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
}

const static u32 TexEnvModeGL[] =
{
	GL_REPLACE,		// decal			| PIXrgb = TEXrgb + OFFSETrgb || PIXa = TEXa
	GL_MODULATE,	// modulate			| PIXrgb = COLrgb x TEXrgb + OFFSETrgb || PIXa = TEXa
	GL_DECAL,		// decal alpha		|
	GL_MODULATE		// modulate alpha	|
};


const static u32 DepthModeGL[] =
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

# define InvDepthModeGL(x) (DepthModeGL[7-x])

const static char *szDepthModeGL[] =
{
	"GL_NEVER",
	"GL_LESS",
	"GL_EQUAL",
	"GL_LEQUAL",
	"GL_GREATER",
	"GL_NOTEQUAL",
	"GL_GEQUAL",
	"GL_ALWAYS"
};

const static u32 SrcBlendGL[] =
{
	GL_ZERO,
	GL_ONE,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA
};

const static u32 DstBlendGL[] =
{
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA
};

# define InvSrcBlendGL(x) SrcBlendGL[7-x]
# define InvDstBlendGL(x) DstBlendGL[7-x]

const static char *szSrcBlendGL[] =
{
	"GL_ZERO",
	"GL_ONE",
	"GL_DST_COLOR",
	"GL_ONE_MINUS_DST_COLOR",
	"GL_SRC_ALPHA",
	"GL_ONE_MINUS_SRC_ALPHA",
	"GL_DST_ALPHA",
	"GL_ONE_MINUS_DST_ALPHA"
};

const static char *szDstBlendGL[] =
{
	"GL_ZERO",
	"GL_ONE",
	"GL_SRC_COLOR",
	"GL_ONE_MINUS_SRC_COLOR",
	"GL_SRC_ALPHA",
	"GL_ONE_MINUS_SRC_ALPHA",
	"GL_DST_ALPHA",
	"GL_ONE_MINUS_DST_ALPHA"
};


#endif //__POWERVR2_GL_H__

