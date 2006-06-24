/*
**	PowerVR2_GL.cpp	- David Miller 2006 -
*/
#include "PowerVR2.h"

PowerVR2_GL PvrIfGl;

__inline 
void PowerVR2_GL::SetRenderMode(u32 ParamID, u32 TexID)
{
/*#ifndef DEBUG_LIB
	static u32 lParam = ~0;

	if(ParamID == lParam)
		return;
	else
		lParam = ParamID;
#endif*/

	GlobalParam * gp = &GlobalParams[ParamID];

	// PCW Settings
	glShadeModel(gp->pcw.Gouraud ? GL_SMOOTH : GL_FLAT);

	// ISP Settings
	glDepthFunc(DepthModeGL[gp->isp.DepthMode]);
	glDepthMask(gp->isp.ZWriteDis ? GL_FALSE : GL_TRUE);

//	glEnable(GL_DEPTH_TEST);


	ASSERT_T(gp->param0.tsp.DstSelect && gp->param0.tsp.SrcSelect, "Src/Dst Select Both Selected !");

	if(gp->param0.tsp.DstSelect)
	{
		glClear(GL_ACCUM_BUFFER_BIT);
		glAccum(GL_ACCUM, 1.f);
	}
	if(gp->param0.tsp.SrcSelect)
	{
		ASSERT_T((gp->isp.Texture || gp->pcw.Texture),"SrcSelect on Textured Poly!");
		glAccum(GL_RETURN, 1.f);
	}
	if((gp->isp.Texture || gp->pcw.Texture) && !gp->param0.tsp.SrcSelect)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, TexID);


#ifdef DEBUG_LIB
		int tw=0,th=0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &tw);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);
		ASSERT_F(glIsTexture(TexID),"Textures Enabled, and Texture is Invalid!");
		ASSERT_T(gp->param0.tsp.SrcSelect,"SrcSelect in Texture Settings!");
		ASSERT_T((0==tw), "OpenGL TexWidth  is Zero!");
		ASSERT_T((0==th), "OpenGL TexHeight is Zero!");
#endif

		// TSP Settings
		// these should be correct now except for offset color 
		switch( gp->param0.tsp.ShadInstr )
		{
		case 1:	DC_TexEnv_Modulate();	break;
		case 2:	DC_TexEnv_DecalAlpha();	break;
		case 0:	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);		break;
		case 3:	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	break;
		default: printf("~<PVR>ERROR: SetRenderMode->TSP.ShadInstr is INVALID !!"); return;
		}
		CheckErrorsGL("RenderSceneGL()->RenderLists()->SetRenderMode() TEX ENV");

		// GL_ARB_texture_mirrored_repeat 
		//	if( vParam[StripID].param0.tsp.FlipUV )
		//u32 Clamp = gp->param0.tsp.ClampUV;
		//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ((Clamp&1) ? GL_CLAMP : GL_REPEAT) );
		//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ((Clamp&2) ? GL_CLAMP : GL_REPEAT) );

		switch(gp->param0.tsp.FilterMode)
		{
		case 0:	TexFilterGL(GL_NEAREST);	break;
		case 1:	TexFilterGL(GL_LINEAR);		break;
		case 2:
		case 3:
		//	if( gp->param0.tcw.MipMapped ) {
		//		TexFilterGL( GL_LINEAR_MIPMAP_LINEAR );
		//	} else {
				TexFilterGL( GL_LINEAR );
		//	} 
		break;
		default: printf("~<PVR>ERROR: Unknown Tex Filter Type in SetRenderMode() !!"); break;
		}

	} else {
		glDisable(GL_TEXTURE_2D);
	}

	// ListType Specific

	CheckErrorsGL("RenderSceneGL()->RenderLists()->SetRenderMode() TEX");
	/*	list type dependant shit ...
	*	blending :
	*	Opaque: "1" must be SRC instruction and "0" must be DST instruction.
	*	PunchThrough: "4" (SRC Alpha) SRC and "5" (Inverse SRC Alpha)  DST
	*/
	switch( gp->pcw.ListType )
	{
	case LT_Opaque:
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);

		CheckErrorsGL("RenderSceneGL()->RenderLists()->SetRenderMode(LT_OPQ)");
		break;

	case LT_Translucent:
		// don't know if this is right .. supposed to enable/disable vtx alpha
		// could always use this at the vtx cmd level but thats a bitch
		if( gp->param0.tsp.UseAlpha )
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);

	//	glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);		// no zbuffering for transparencies

		if(!gp->param0.tsp.IgnoreTexA)
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.f);
		}
		else {
			glDisable(GL_ALPHA_TEST);
		}

		// this has got to be all fucked up ...
		//	if( gp->param0.tsp.SrcSelect ) glAccum( GL_RETURN, 0.f ) ;
		//	if( gp->param0.tsp.DstSelect ) glAccum( GL_LOAD, 0.f ) ;

		glBlendFunc(SrcBlendGL[gp->param0.tsp.SrcInstr], DstBlendGL[gp->param0.tsp.DstInstr]);
		CheckErrorsGL("RenderSceneGL()->RenderLists()->SetRenderMode(LT_TRS)");
		break;

		// PUNCH THRU is NOTHING BUT NON BLENDED / ALPHA TESTED TRIS
	case LT_PunchThrough:	
		if(!gp->param0.tsp.IgnoreTexA)
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GEQUAL, (float)(*pPT_ALPHA_REF &0xFF)/255.f);
		}
		else {
			glDisable(GL_ALPHA_TEST);
		}

		glDisable(GL_BLEND);

		// This should look nicer, could be trouble with somethings..
	//	glEnable(GL_BLEND);
	//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// ...

		CheckErrorsGL("RenderSceneGL()->RenderLists()->SetRenderMode(LT_PTHRU)");
		break;

	case LT_OpaqueMod:return;		// don't care yet
	case LT_TransMod: return;		// don't care yet
	case LT_Reserved:	ASSERT_T((1),"BOGUS LIST TYPE IN SetRenderMode()");	return;
	}
}


__inline 
void PowerVR2_GL::SetRenderModeSpr(u32 ParamID, u32 TexID)
{
	GlobalParam * gp = &GlobalParams[ParamID];

	// PCW Settings
	glShadeModel(GL_FLAT);	// sprites are flat shaded 


	// ISP Settings
	glDepthFunc(DepthModeGL[gp->isp.DepthMode]);
	//	glDepthMask(gp->isp.ZWriteDis ? GL_FALSE : GL_TRUE);

	if(LT_Opaque == gp->pcw.ListType)
	{
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	} else {
		//glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc( GL_GREATER, 0.f );
	}

	if(gp->isp.Texture || gp->pcw.Texture)
	{
		glEnable(GL_TEXTURE_2D);
	/*	if(gp->param0.tcw.ScanOrder && gp->param0.tcw.StrideSel)
		{
			glEnable(GL_TEXTURE_RECTANGLE_ARB);
			glBindTexture(GL_TEXTURE_RECTANGLE_ARB, TexID);
		} else {	*/
	//		glDisable(GL_TEXTURE_RECTANGLE_ARB);	// needed?
			glBindTexture(GL_TEXTURE_2D, TexID);
	//	}

		// TSP Settings - these should be correct now except for offset color 
		switch( gp->param0.tsp.ShadInstr ) {
		case 1:	DC_TexEnv_Modulate();	break;
		case 2:	DC_TexEnv_DecalAlpha();	break;
		case 0:	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );	break;
		case 3:	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );	break;
		default: printf("~<PVR>ERROR: SetRenderMode->TSP.ShadInstr is INVALID !!"); return;
		}
		CheckErrorsGL("SetRenderModeSpr() TEX ENV");

		if(gp->param0.tsp.FilterMode == 0)
			TexFilterGL( GL_NEAREST );
		else
			TexFilterGL( GL_LINEAR );

		// GL_ARB_texture_mirrored_repeat 
		//	if( vParam[StripID].param0.tsp.FlipUV )		//u32 Clamp = gp->param0.tsp.ClampUV;
		//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ((Clamp&1) ? GL_CLAMP : GL_REPEAT) );
		//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ((Clamp&2) ? GL_CLAMP : GL_REPEAT) );

	} else {
		glDisable(GL_TEXTURE_2D);
	}
}





__inline 
#ifndef USE_STD_VECTOR
void PowerVR2_GL::RenderStripList(zector<Vertex> &vl)
#else
void PowerVR2_GL::RenderStripList(vector<Vertex> &vl)
#endif
{
	for(u32 p=0; p<vl.size(); p++)
	{
		SetRenderMode(vl[p].ParamID, vl[p].TexID);

		glBegin(GL_TRIANGLE_STRIP);
		for(u32 v=0; v<vl[p].List.size(); v++)
		{
			glColor4ubv((u8*)&vl[p].List[v].col);
			glTexCoord4fv(vl[p].List[v].uv);
			glVertex3fv(vl[p].List[v].xyz);
		}
		glEnd();
	}
}
__inline 
#ifndef USE_STD_VECTOR
void PowerVR2_GL::RenderStripListRev(zector<Vertex> &vl)
#else
void PowerVR2_GL::RenderStripListRev(vector<Vertex> &vl)
#endif
{
	for(u32 p=0; p<vl.size(); p++)
	{
		SetRenderMode(vl[p].ParamID, vl[p].TexID);

		glBegin(GL_TRIANGLE_STRIP);
		for(s32 v=vl[p].List.size()-1; v>=0; v--)
		{
			glColor4ubv((u8*)&vl[p].List[v].col);
			glTexCoord4fv(vl[p].List[v].uv);
			glVertex3fv(vl[p].List[v].xyz);
		}
		glEnd();
	}
}

__inline 
#ifndef USE_STD_VECTOR
void PowerVR2_GL::RenderStripListArray(zector<Vertex> &vl)
#else
void PowerVR2_GL::RenderStripListArray(vector<Vertex> &vl)
#endif
{
/*	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);*/

	for(u32 p=0; p<vl.size(); p++)
	{
		SetRenderMode(vl[p].ParamID, vl[p].TexID);

		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vert), &vl[p].List[0].col);
		glTexCoordPointer(4, GL_FLOAT, sizeof(Vert), vl[p].List[0].uv);
		glVertexPointer(3, GL_FLOAT, sizeof(Vert), vl[p].List[0].xyz);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)vl[p].List.size());
	}
/*	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);*/
}

__inline 
#ifndef USE_STD_VECTOR
void PowerVR2_GL::RenderSprites(zector<Vertex> &vl)
#else
void PowerVR2_GL::RenderSprites(vector<Vertex> &vl)
#endif
{
	for(u32 p=0; p<vl.size(); p++)
	{
		SetRenderModeSpr(vl[p].ParamID, vl[p].TexID);

		glBegin(GL_QUADS);
		for(u32 v=0; v<vl[p].List.size(); v++)
		{
			glColor4ubv((u8*)&vl[p].List[v].col);
			glTexCoord4fv(vl[p].List[v].uv);
			glVertex3fv(vl[p].List[v].xyz);
		}
		glEnd();
	}
}


void PowerVR2_GL::Render()
{
	FrameCount++;
	glFlush();
	Resize();

	u32 dwValue = *pVO_BORDER_COL;
	f32	R=((dwValue>>0x10)&0xFF)/255.f,
		G=((dwValue>>0x08)&0xFF)/255.f,
		B=((dwValue>>0x00)&0xFF)/255.f;

	glClearColor( R,G,B, 1.f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // | GL_ACCUM_BUFFER_BIT);
	////////////////////////////////////////////////////

#define USE_VERTEX_ARRAYS
#ifndef USE_VERTEX_ARRAYS
	RenderStripList(OpaqueVerts);
	RenderStripListRev(TranspVerts);	//Sprites
	RenderStripList(PunchtVerts);
#else
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	RenderStripListArray(OpaqueVerts);
	RenderStripListArray(TranspVerts);	//Sprites
	RenderStripListArray(PunchtVerts);

	glPopClientAttrib();
#endif

	RenderSprites(Sprites);

	ClearDCache();
	ClearTInvalids();
	SwapBuffers(hDC);
	CheckErrorsGL("RenderSceneGL()");

	//printf(" ----- End Render -------\n");
}




void PowerVR2_GL::Resize()
{
	RECT rClient;
	GetClientRect((HWND)emuIf.handle,&rClient);

	glViewport( 0,0, (u32)(rClient.right-rClient.left), (u32)(rClient.bottom-rClient.top) );
	//lprintf("SizeGL() viewport: %i, %i\n",(rClient.right-rClient.left), (rClient.bottom-rClient.top));

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0,640,480,0, 1.f, -1.f );

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


bool PowerVR2_GL::Init()
{
	GLuint	PixelFormat;

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		32,											// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		24,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if(!(hDC=GetDC((HWND)emuIf.handle)))
	{
		MessageBox((HWND)emuIf.handle,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		Term();	return false;
	}
	if( !(PixelFormat=ChoosePixelFormat(hDC,&pfd)) ||
		!SetPixelFormat(hDC,PixelFormat,&pfd))
	{
		MessageBox((HWND)emuIf.handle,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		Term();	return false;
	}
	if(!(hRC=wglCreateContext(hDC)) || !wglMakeCurrent(hDC,hRC))
	{
		MessageBox((HWND)emuIf.handle,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		Term();	return false;
	}

	//	if( gfx_opts.wireframe == TRUE )
	//	{	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	}
	//		else
	{	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	}

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glShadeModel(GL_SMOOTH);
	glAlphaFunc(GL_GREATER, 0.f);

	glClearDepth(0.f);	// 0 ? wtf .. thX F|RES
	glClearColor(1.f, 1.f, 256.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	Resize();
	return true;
}

void PowerVR2_GL::Term()
{
	ClearTCache();		// Textures
	ClearDCache();

	if (hRC && (!wglMakeCurrent(NULL,NULL)) || (!wglDeleteContext(hRC)) ) {
		MessageBox((HWND)emuIf.handle, "Release Rendering Context Failed.","SHUTDOWN ERROR",MB_ICONERROR);
		hRC=NULL;
	}
	if (hDC && !ReleaseDC((HWND)emuIf.handle,hDC)) {
		MessageBox((HWND)emuIf.handle, "Release Device Context Failed.","SHUTDOWN ERROR",MB_ICONERROR);
		hDC=NULL;	
	}
}



GLvoid PowerVR2_GL::CheckErrorsGL( char *szFunc )
{
	GLenum err;
	const GLubyte *pszErrStr;

	if((err = glGetError()) != GL_NO_ERROR)
	{
		pszErrStr = gluErrorString(err);
		printf("OpenGL Error in %s\n\t %s\n", szFunc, pszErrStr );
	}
}
