/*
**	nullRendGL.cpp,	nullDC::nullPVR	(2007) ZeZu & drk||Raziel
*/

#include "nullPvr.h"
#include "nullRend.h"
#include "pvrMemory.h"

HDC hDC;
HGLRC hRC;



s32  FASTCALL InitGL(void * handle)
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

	if(!(hDC=GetDC((HWND)handle)))
	{
		MessageBox((HWND)handle,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		TermGL(handle);	return false;
	}
	if( !(PixelFormat=ChoosePixelFormat(hDC,&pfd)) ||
		!SetPixelFormat(hDC,PixelFormat,&pfd))
	{
		MessageBox((HWND)handle,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		TermGL(handle);	return false;
	}
	if(!(hRC=wglCreateContext(hDC)) || !wglMakeCurrent(hDC,hRC))
	{
		MessageBox((HWND)handle,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		TermGL(handle);	return false;
	}
	if (GLEW_OK != glewInit())
	{
		MessageBox((HWND)handle,"Couldn't Initialize GLEW.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		TermGL(handle);	return false;
	}

	ASSERT_F(GLEW_ARB_vertex_program, "No Vertex Shaders!");
	ASSERT_F(GLEW_ARB_fragment_program, "No Fragment Shaders!");

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
	glClearColor(1.f, 1.f, 1.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	/*	InitCg();
	LoadVProgram("VProgram.cg","PVR_VertexInput");
	LoadFProgram("FProgram.cg","PVR_FragmentInput");	*/\

#ifdef USE_VBOS

	glGenBuffers(3, vbo_ptr);
	if(0==vbo_ptr[0] || 0==vbo_ptr[1] || 0==vbo_ptr[2])
		return false;

	glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo_ptr[vbo_opq]);
	glBufferData(GL_ARRAY_BUFFER_ARB, DCACHE_SIZE, NULL, GL_DYNAMIC_DRAW_ARB);
	glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo_ptr[vbo_trs]);
	glBufferData(GL_ARRAY_BUFFER_ARB, DCACHE_SIZE, NULL, GL_DYNAMIC_DRAW_ARB);
	glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo_ptr[vbo_ptu]);
	glBufferData(GL_ARRAY_BUFFER_ARB, DCACHE_SIZE, NULL, GL_DYNAMIC_DRAW_ARB);
	CheckErrorsGL("InitGL()->Creating VBOs");

#endif

	ResizeGL(handle);
	RenderGL(NULL);

	return true;
}

void FASTCALL TermGL(void * handle)
{
	// *FIXME* re-enable these
//	ClearDCache();
//	TCache.ClearTCache();		// Textures

	if (hRC && (!wglMakeCurrent(NULL,NULL)) || (!wglDeleteContext(hRC)) ) {
		MessageBox((HWND)handle, "Release Rendering Context Failed.","SHUTDOWN ERROR",MB_ICONERROR);
		hRC=NULL;
	}
	if (hDC && !ReleaseDC((HWND)handle,hDC)) {
		MessageBox((HWND)handle, "Release Device Context Failed.","SHUTDOWN ERROR",MB_ICONERROR);
		hDC=NULL;	
	}
}

static RECT rClient;
void FASTCALL ResizeGL(void * handle)
{
	GetClientRect((HWND)handle,&rClient);

//	if(bDefault)
		glViewport( 0,0, (u32)(rClient.right-rClient.left), (u32)(rClient.bottom-rClient.top) );
/*	else
		glViewport( 0,0, 640,480 );*/

	//lprintf("SizeGL() viewport: %i, %i\n",(rClient.right-rClient.left), (rClient.bottom-rClient.top));

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0,640,480,0, 1.f, -1.f );

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void FASTCALL RenderGL(void * buffer)
{
	u32 dwValue = *pVO_BORDER_COL;
	f32	R=((dwValue>>0x10)&0xFF)/255.f,
		G=((dwValue>>0x08)&0xFF)/255.f,
		B=((dwValue>>0x00)&0xFF)/255.f;

	glClearColor( R,G,B, 1.f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // | GL_ACCUM_BUFFER_BIT);


	printf("-------RENDER GL_-----------------\n");


//	ClearDCache();
//	TCache.ClearTInvalids();

	
	SwapBuffers(hDC);

}
void FASTCALL SetStateGL(void * state)
{
}