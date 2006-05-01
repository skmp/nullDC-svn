/*
**	TA_Param.h - David Miller 2006 - PowerVR2 Emulation Library
*/

#ifndef __POWERVR2_H__
#define __POWERVR2_H__

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "zNullPvrGL.h"
#include "TA_Regs.h"
#include "TA_Param.h"
#include "TA_Texture.h"


// Todo, add resize, switch (fs), config, init/term to use the rest

extern
class PowerVR2
	: public PrimConverter, public TextureCache
{
public:

	bool Init();
	void Term();
	void Render();
	void Resize();
	u32 FrameCount;

private:

	void RenderSprites(vector<Vertex> &vl);
	void RenderStripList(vector<Vertex> &vl);
	void RenderStripListRev(vector<Vertex> &vl);
	void RenderStripListArray(vector<Vertex> &vl);

	void SetRenderMode(u32 ParamID, u32 TexID);
	void SetRenderModeSpr(u32 ParamID, u32 TexID);

//#ifdef USE_OPENGL
	HDC hDC;
	HGLRC hRC;
	void CheckErrorsGL(char *szFunc);
//#endif

} PvrIf;






//#ifdef USE_OPENGL


__inline static void TexFilterGL( GLuint filter )
{
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
}

__inline static void DC_TexEnv_Modulate(void)
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

__inline static void DC_TexEnv_DecalAlpha(void)
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



//#endif

#endif //__POWERVR2_H__

