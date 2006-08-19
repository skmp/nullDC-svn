#include "oglRend.h"
#include "windows.h"
#include "gl\gl.h"

struct TextureCacheData
{
	u32 Start;
	GLuint texID;
	u32 format;
	
	u32 w,h;
	u32 size;
	bool dirty;
	vram_block* lock_block;
};

TexCacheList<TextureCacheData> ogl_texture_cache;

GLuint GenText(u32 addr)
{
	return 0;
}
GLuint GetTexture(u32 addr)
{
	TextureCacheData * tf = ogl_texture_cache.Find(addr);
	if (tf)
	{
		return tf->texID;
	}
	else
		return GenText(addr);
}

#ifndef NO_OPENGL
//if OpenGL support is not included , fallback to d3d/soft/null rendering
HDC   hdc1;
HGLRC    hglrc1;
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)				// Resize And Initialize The GL Window
{
	if (height==0)								// Prevent A Divide By Zero By
	{
		height=1;							// Making Height Equal One
	}

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();							// Reset The Projection Matrix
}


void * win;
void Resize()
{
	RECT rClient;
	GetClientRect((HWND)win,&rClient);

	glViewport( 0,0, (u32)(rClient.right-rClient.left), (u32)(rClient.bottom-rClient.top) );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0,640,480,0, 1, -1 );
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void EnableOpenGL(HWND hWnd, HDC& hDC, HGLRC& hRC)
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

	if(!(hDC=GetDC(hWnd)))
	{
		MessageBox(hWnd,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		//Term();	return false;
	}
	if( !(PixelFormat=ChoosePixelFormat(hDC,&pfd)) ||
		!SetPixelFormat(hDC,PixelFormat,&pfd))
	{
		MessageBox(hWnd,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
//		Term();	return false;
	}
	if(!(hRC=wglCreateContext(hDC)) || !wglMakeCurrent(hDC,hRC))
	{
		MessageBox(hWnd,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
//		Term();	return false;
	}
	/*if (GLEW_OK != glewInit())
	{
		MessageBox(hWnd,"Couldn't Initialize GLEW.","ERROR",MB_OK|MB_ICONEXCLAMATION);
//		Term();	return false;
	}*/


	//	if( gfx_opts.wireframe == TRUE )
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//		else
	//{	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	}

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glShadeModel(GL_SMOOTH);
	glAlphaFunc(GL_GREATER, 0.f);

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);

	glClearDepth(0.f);	// 0 ? wtf .. thX F|RES
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable( GL_CULL_FACE );

	Resize();
}

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent( NULL, NULL );
    wglDeleteContext( hRC );
    ReleaseDC( hWnd, hDC );
}
//null renderer is here
void oglPresentFB()
{
}

void olgRendStrips_pv(TaGlobalParam* gp)
{
	u32 vlc=gp->vlc;
	TaVertexList* v=&vertlists_pv.data[gp->first];
	while(vlc--)
	{
		glDrawArrays(GL_TRIANGLE_STRIP, v->first, v->sz);
		v++;//next vertex list
	}
}

extern u32 VertexCount;
void FixZValues(List<TaVertex>& verts,f32& z_max,f32& z_min)
{
	f32 z_diff=z_max;
	z_max*=1.01f;
	z_diff=z_max-z_diff;
	f32 z_min2,z_max2;

	//fixup z_min
	if (z_min==0)
		z_min=0.0000001;
	if (z_min<=1)
	{
		z_min2=3.6f+(1/(z_min));
	}
	else
	{
		//z_max>=invW
		z_min2=z_max-z_min;	//smaller == closer to screen
		z_min2/=z_min;		//scale from 0 to 1
		z_min2=2.1+z_min;	//its in front of other Z :)
	}

	//fixup z_max
	if (z_max<=1)
	{
		z_max2=3.4f+(1/(z_max));
	}
	else
	{
		//z_max>=invW
		z_max2=z_diff;	//smaller == closer to screen
		z_max2/=z_max;		//scale from 0 to 1
		z_max2=1.9+z_max;	//its in front of other Z :)
	}

	f32 z_scale=(z_min2-z_max2); //yay ?

	TaVertex* cv = &verts.data[0];
	for (u32 i=0;i<verts.used;i++)
	{
		//cv is 0 for infinitevly away , <0 for closer
		f32 invW=cv->xyz[2];
		if (invW==0)
			invW=0.0000002;
		
		if (invW<=1)
		{
			invW=3.5f+(1/(invW));
		}
		else
		{
			//z_max>=invW
			invW=z_max-invW;	//smaller == closer to screen
			invW/=z_max;		//scale from 0 to 1
			invW=2+invW;		//its in front of other Z :)
		}

		cv->xyz[2]=(invW-z_max2)/z_min2;	//scale from 0 to 1 (?)

		//now , the larger z , the further away things are now
		cv++;
	}

	//constants were edited a bit , so z_min & z_max can be used as clipping planes :)

	//near cliping is z_max (closest point to screen) and far cliping is z_min
}
void oglStartRender()
{
	VertexCount+=verts_pv.used;


	//temp fix for Z values
	FixZValues(verts_pv,z_max,z_min);

	Resize();
	//glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT,GL_FASTEST);
	if (GetAsyncKeyState('Q'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (GetAsyncKeyState('W'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColorPointer(4, GL_FLOAT, sizeof(TaVertex),  verts_pv.data->col);
	glVertexPointer(3, GL_FLOAT, sizeof(TaVertex), verts_pv.data->xyz);

	glEnableClientState( GL_COLOR_ARRAY );
    glEnableClientState( GL_VERTEX_ARRAY );

	for (u32 cgp=0;cgp<14;cgp++)
	{
		for (u32 i=0;i<global_param_op_pv[cgp].used;i++)
		{
			olgRendStrips_pv(&global_param_op_pv[cgp].data[i]);
		}
	}
	for (u32 cgp=0;cgp<14;cgp++)
	{
		for (u32 i=0;i<global_param_tr_pv[cgp].used;i++)
		{
			olgRendStrips_pv(&global_param_tr_pv[cgp].data[i]);
		}
	}

	SwapBuffers(hdc1);
}
bool oglInitRenderer(void* window)
{
	return true;
}

void oglTermRenderer()
{
}

void oglResetRenderer(bool Manual)
{
}


bool oglThreadStart(void* window)
{
	win=window;
	EnableOpenGL((HWND)win,hdc1,hglrc1);
	return true;
}

void oglThreadEnd()
{
	DisableOpenGL((HWND)win,hdc1,hglrc1);
}


void GetOpenGLRenderer(rend_if* rif)
{
	//general init/term/reset
	rif->Init=oglInitRenderer;
	rif->Term=oglTermRenderer;
	rif->Reset=oglResetRenderer;
	
	//thread init/term
	rif->ThreadStart=oglThreadStart;
	rif->ThreadEnd=oglThreadEnd;

	//drawing related functions :)
	rif->PresentFB=oglPresentFB;
	rif->StartRender=oglStartRender;
}

#else
#include "nullRend.h"
void GetOpenGLRenderer(rend_if* rif)
{
	GetNullRenderer(rif);
}
#endif

