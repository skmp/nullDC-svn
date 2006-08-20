#include "oglRend.h"
#include <windows.h>
#include <gl\gl.h>
#include <gl\glaux.h>
#include "regs.h"


using namespace TASplitter;

namespace OpenGLRenderer
{
	//Texture Cache :)
	struct TextureCacheData
	{
		u32 Start;
		GLuint texID;
		TCW format;

		u32 w,h;
		u32 size;
		bool dirty;
		vram_block* lock_block;
	};

	TexCacheList<TextureCacheData> TexCache;

	TextureCacheData* __fastcall GenText(TSP tsp,TCW tcw)
	{
		//generate texture
		TextureCacheData* tf = &TexCache.Add(0)->data;
		tf->Start=(tcw.NO_PAL.TexAddr<<3) & 0x7FFFFF;
		
		glGenTextures(1,&tf->texID);
		glBindTexture(GL_TEXTURE_2D, tf->texID);
		tf->w=1<<tsp.TexU;
		tf->h=1<<tsp.TexV;
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tf->w, tf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &vram_64[tf->Start]);
		
		printf("Texture @ 0x%X\n",tcw.NO_PAL.TexAddr<<3);
		return tf;
	}
	
	GLuint __fastcall GetTexture(TSP tsp,TCW tcw)
	{	
		TextureCacheData* tf = TexCache.Find(tcw.NO_PAL.TexAddr<<3);
		if (tf)
		{
			return tf->texID;
		}
		else
		{
			tf = GenText(tsp,tcw);
			return tf->texID;
		}
		return 0;
	}
	
	//OpenGl Init/Term
	HDC   hdc1;
	HGLRC    hglrc1;
	void * win;
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
	//use that someday
	void PresentFB()
	{
	}

	u32 VertexCount;
	u32 FrameCount;
	f32 z_min;
	f32 z_max;

	//Vertex storage types
	//64B
	struct Vertex
	{
		//0
		float xyz[3];

		//12
		u32 nil;//padding

		//16
		float col[4];

		//32
		//tex cords if texture
		float uv[4];

		//48
		//offset color
		float col_offset[4];

		//64
	};

	//8B
	struct VertexList
	{
		u32 first;
		u32 sz;
	};

	//8B
	struct PolyParam
	{
		u32 first;		//entry index , holds vertex/pos data
		u32 vlc;

		//lets see what more :)
		GLuint texID;	//0xFFFFFFFF if no texture

		void SetRenderMode_Op()
		{
			if (texID!=0xFFFFFFFF)
			{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,texID);
				glEnableClientState( GL_TEXTURE_COORD_ARRAY );
			}
			else
			{
				glDisableClientState( GL_TEXTURE_COORD_ARRAY );
				glDisable(GL_TEXTURE_2D);
			}
		}
		void SetRenderMode_Tr()
		{
			if (texID!=0xFFFFFFFF)
			{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,texID);
				glEnableClientState( GL_TEXTURE_COORD_ARRAY );
			}
			else
			{
				glDisableClientState( GL_TEXTURE_COORD_ARRAY );
				glDisable(GL_TEXTURE_2D);
			}
		}
		void SetRenderMode_Pt()
		{
			if (texID!=0xFFFFFFFF)
			{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,texID);
				glEnableClientState( GL_TEXTURE_COORD_ARRAY );
			}
			else
			{
				glDisableClientState( GL_TEXTURE_COORD_ARRAY );
				glDisable(GL_TEXTURE_2D);
			}
		}
	};


	//vertex lists
	List<Vertex> verts;
	List<VertexList> vertlists;
	List<PolyParam> global_param_op;
	List<PolyParam> global_param_pt;
	List<PolyParam> global_param_tr;

	template <u32 Type>
	__forceinline
	void RendStrips(PolyParam* gp)
	{
		u32 vlc=gp->vlc;
		VertexList* v=&vertlists.data[gp->first];
		
		if (Type==0)
			gp->SetRenderMode_Op();
		else if (Type==1)
			gp->SetRenderMode_Pt();
		else
			gp->SetRenderMode_Tr();

		while(vlc--)
		{
			glDrawArrays(GL_TRIANGLE_STRIP, v->first, v->sz);
			v++;//next vertex list
		}
	}

	template <u32 Type>
	void RendPolyParamList(List<PolyParam>& gpl)
	{
		for (u32 i=0;i<gpl.used;i++)
		{		
			RendStrips<Type>(&gpl.data[i]);
		}
	}
	void FixZValues(List<Vertex>& verts,f32& z_max,f32& z_min)
	{
		f32 z_diff=z_max;
		z_max*=1.01f;
		z_diff=z_max-z_diff;
		f32 z_min2,z_max2;

		//fixup z_min
		if (z_min==0)
			z_min=0.0000001f;
		if (z_min<=1)
		{
			z_min2=3.6f+(1/(z_min));
		}
		else
		{
			//z_max>=invW
			z_min2=z_max-z_min;	//smaller == closer to screen
			z_min2/=z_min;		//scale from 0 to 1
			z_min2=2.1f+z_min;	//its in front of other Z :)
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
			z_max2=1.9f+z_max;	//its in front of other Z :)
		}

		f32 z_scale=(z_min2-z_max2); //yay ?

		Vertex* cv = &verts.data[0];
		for (u32 i=0;i<verts.used;i++)
		{
			//cv is 0 for infinitevly away , <0 for closer
			f32 invW=cv->xyz[2];
			if (invW==0)
				invW=0.0000002f;

			if (invW<=1)
			{
				invW=3.5f+(1/(invW));
			}
			else
			{
				//z_max is >=invW
				invW=z_max-invW;	//smaller == closer to screen
				invW/=z_max;		//scale from 0 to 1
				invW=2+invW;		//its in front of other Z :)
			}

			cv->xyz[2]=(invW-z_max2)/z_min2;	//scale from 0 to 1

			//now , the larger z , the further away things are now
			cv++;
		}

		//constants were edited a bit , so z_min & z_max can be used as clipping planes :)

		//near cliping is z_max (closest point to screen) and far cliping is z_min
	}
	void StartRender()
	{
		VertexCount+=verts.used;

		if (FB_W_SOF1 & 0x1000000)
			return;

		FrameCount++;

		//temp fix for Z values
		FixZValues(verts,z_max,z_min);

		Resize();
		
		if (GetAsyncKeyState('Q'))
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (GetAsyncKeyState('W'))
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glColorPointer(4, GL_FLOAT, sizeof(Vertex),  verts.data->col);
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex), verts.data->xyz);
		glTexCoordPointer(4, GL_FLOAT, sizeof(Vertex), verts.data->uv);

		glEnableClientState( GL_COLOR_ARRAY );
		glEnableClientState( GL_VERTEX_ARRAY );
		
		RendPolyParamList<0>(global_param_op);
		RendPolyParamList<1>(global_param_pt);
		RendPolyParamList<2>(global_param_tr);
		
		SwapBuffers(hdc1);
	}

	using namespace TASplitter;
	VertexList* CurrentVL=0;
	PolyParam* CurrentPP=0;

	f32 f16(u16 v)
	{
		u32 z=v<<16;
		return *(f32*)&z;
	}
	struct VertexDecoder
	{
		//Polys
#define glob_param_bdc \
		PolyParam* d_pp ; \
		if (CurrentList==ListType_Opaque) \
			d_pp=global_param_op.Append(); \
		else if (CurrentList==ListType_Punch_Through) \
			d_pp=global_param_pt.Append(); \
		else \
			d_pp=global_param_tr.Append(); \
		\
		CurrentPP=d_pp;\
		\
		d_pp->first=vertlists.used;\
		d_pp->vlc=0;

		__forceinline
		static void AppendPolyParam32(TA_PolyParamA* pp)
		{
			glob_param_bdc;

			if (pp->pcw.Texture)
				d_pp->texID=GetTexture(pp->tsp,pp->tcw);
			else
				d_pp->texID=0xFFFFFFFF;
		}
		__forceinline
		static void AppendPolyParam64A(TA_PolyParamA* pp)
		{
			glob_param_bdc;

			if (pp->pcw.Texture)
				d_pp->texID=GetTexture(pp->tsp,pp->tcw);
			else
				d_pp->texID=0xFFFFFFFF;
		}
		__forceinline
		static void AppendPolyParam64B(TA_PolyParamB* pp)
		{

		}

		//Poly Strip handling
		__forceinline
		static void PolyStripStart()
		{
			CurrentVL= vertlists.Append();
			CurrentVL->first=verts.used;
			CurrentVL->sz=0;
		}
		__forceinline
		static void PolyStripEnd()
		{
			CurrentPP->vlc++;
			CurrentVL = 0;
		}

			//Poly Vertex handlers

#define vert_cvt_base \
	Vertex* cv=verts.Append();\
	CurrentVL->sz++;\
	cv->xyz[0]=vtx->xyz[0];\
	cv->xyz[1]=vtx->xyz[1];\
	f32 invW=vtx->xyz[2];\
	cv->xyz[2]=invW;\
	if (z_min>invW)\
		z_min=invW;\
	if (z_max<invW)\
		z_max=invW;

#define vert_uv_32(u_name,v_name) \
		cv->uv[0]	=	vtx->u_name;\
		cv->uv[1]	=	vtx->v_name;\
		cv->uv[2]	=	1; \
		cv->uv[3]	=	invW; 

#define vert_uv_16(u_name,v_name) \
		cv->uv[0]	=	f16(vtx->u_name);\
		cv->uv[1]	=	f16(vtx->v_name);\
		cv->uv[2]	=	1; \
		cv->uv[3]	=	invW; 

		//(Non-Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex0(TA_Vertex0* vtx)
		{
			vert_cvt_base;
			cv->col[0]	= (255 & (vtx->BaseCol >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol >> 24)) / 255.f;
		}

		//(Non-Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex1(TA_Vertex1* vtx)
		{
			vert_cvt_base;
			cv->col[0]	= vtx->BaseR;
			cv->col[1]	= vtx->BaseG;
			cv->col[2]	= vtx->BaseB;
			cv->col[3]	= vtx->BaseA;
		}

		//(Non-Textured, Intensity)
		__forceinline
		static void AppendPolyVertex2(TA_Vertex2* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= vtx->BaseInt;
			cv->col[1]	= vtx->BaseInt;
			cv->col[2]	= vtx->BaseInt;
			cv->col[3]	= vtx->BaseInt;
		}

		//(Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex3(TA_Vertex3* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= (255 & (vtx->BaseCol >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol >> 24)) / 255.f;

			vert_uv_32(u,v);
		}

		//(Textured, Packed Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex4(TA_Vertex4* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= (255 & (vtx->BaseCol >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol >> 24)) / 255.f;

			vert_uv_16(u,v);
		}

		//(Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex5A(TA_Vertex5A* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= 1;//vtx->BaseR;
			cv->col[1]	= 1;//vtx->BaseG;
			cv->col[2]	= 1;//vtx->BaseB;
			cv->col[3]	= 1;//vtx->BaseA;

			vert_uv_32(u,v);
		}
		__forceinline
		static void AppendPolyVertex5B(TA_Vertex5B* vtx)
		{

		}

		//(Textured, Floating Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex6A(TA_Vertex6A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= 1;//vtx->BaseR;
			cv->col[1]	= 1;//vtx->BaseG;
			cv->col[2]	= 1;//vtx->BaseB;
			cv->col[3]	= 1;//vtx->BaseA;

			vert_uv_16(u,v);
		}
		__forceinline
		static void AppendPolyVertex6B(TA_Vertex6B* vtx)
		{

		}

		//(Textured, Intensity)
		__forceinline
		static void AppendPolyVertex7(TA_Vertex7* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= vtx->BaseInt;
			cv->col[1]	= vtx->BaseInt;
			cv->col[2]	= vtx->BaseInt;
			cv->col[3]	= vtx->BaseInt;

			vert_uv_32(u,v);
		}

		//(Textured, Intensity, 16bit UV)
		__forceinline
		static void AppendPolyVertex8(TA_Vertex8* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= vtx->BaseInt;
			cv->col[1]	= vtx->BaseInt;
			cv->col[2]	= vtx->BaseInt;
			cv->col[3]	= vtx->BaseInt;

			vert_uv_16(u,v);
		}

		//(Non-Textured, Packed Color, with Two Volumes)
		__forceinline
		static void AppendPolyVertex9(TA_Vertex9* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= (255 & (vtx->BaseCol0 >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol0 >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol0 >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol0 >> 24)) / 255.f;
		}

		//(Non-Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex10(TA_Vertex10* vtx)
		{
			vert_cvt_base;
			
			cv->col[0]	= vtx->BaseInt0;
			cv->col[1]	= vtx->BaseInt0;
			cv->col[2]	= vtx->BaseInt0;
			cv->col[3]	= vtx->BaseInt0;
		}

		//(Textured, Packed Color,	with Two Volumes)	
		__forceinline
		static void AppendPolyVertex11A(TA_Vertex11A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= (255 & (vtx->BaseCol0 >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol0 >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol0 >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol0 >> 24)) / 255.f;

			vert_uv_32(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex11B(TA_Vertex11B* vtx)
		{
			
		}

		//(Textured, Packed Color, 16bit UV, with Two Volumes)
		__forceinline
		static void AppendPolyVertex12A(TA_Vertex12A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= (255 & (vtx->BaseCol0 >> 16)) / 255.f;
			cv->col[1]	= (255 & (vtx->BaseCol0 >> 8))  / 255.f;
			cv->col[2]	= (255 & (vtx->BaseCol0 >> 0))  / 255.f;
			cv->col[3]	= (255 & (vtx->BaseCol0 >> 24)) / 255.f;

			vert_uv_16(u0,v0);
		}
		__forceinline
		static void AppendPolyVertex12B(TA_Vertex12B* vtx)
		{

		}

		//(Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex13A(TA_Vertex13A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= vtx->BaseInt0;
			cv->col[1]	= vtx->BaseInt0;
			cv->col[2]	= vtx->BaseInt0;
			cv->col[3]	= vtx->BaseInt0;

			vert_uv_32(u0,v0);			
		}
		__forceinline
		static void AppendPolyVertex13B(TA_Vertex13B* vtx)
		{

		}

		//(Textured, Intensity, 16bit UV, with Two Volumes)
		__forceinline
		static void AppendPolyVertex14A(TA_Vertex14A* vtx)
		{
			vert_cvt_base;

			cv->col[0]	= vtx->BaseInt0;
			cv->col[1]	= vtx->BaseInt0;
			cv->col[2]	= vtx->BaseInt0;
			cv->col[3]	= vtx->BaseInt0;

			vert_uv_16(u0,v0);	
		}
		__forceinline
		static void AppendPolyVertex14B(TA_Vertex14B* vtx)
		{

		}

		//Sprites
		__forceinline
		static void AppendSpriteParam(TA_SpriteParam* spr)
		{

		}

		//Sprite Vertex Handlers
		__forceinline
		static void AppendSpriteVertex0A(TA_Sprite0A* sv)
		{

		}
		__forceinline
		static void AppendSpriteVertex0B(TA_Sprite0B* sv)
		{

		}
		__forceinline
		static void AppendSpriteVertex1A(TA_Sprite1A* sv)
		{

		}
		__forceinline
		static void AppendSpriteVertex1B(TA_Sprite1B* sv)
		{

		}

		//ModVolumes
		__forceinline
		static void AppendModVolParam(TA_ModVolParam* modv)
		{

		}

		//ModVol Strip handling
		__forceinline
		static void ModVolStripStart()
		{

		}
		__forceinline
		static void ModVolStripEnd()
		{

		}

		//Mod Volume Vertex handlers
		__forceinline
		static void AppendModVolVertexA(TA_ModVolA* mvv)
		{

		}
		__forceinline
		static void AppendModVolVertexB(TA_ModVolB* mvv)
		{

		}

		//Misc
		__forceinline
		static void ListCont()
		{
		}
		__forceinline
		static void ListInit()
		{
			verts.Clear();
			vertlists.Clear();
			global_param_op.Clear();
			global_param_pt.Clear();
			global_param_tr.Clear();
			z_min= 1000000.0f;
			z_max=-1000000.0f;
		}
		__forceinline
		static void SoftReset()
		{
		}
	};

	FifoSplitter<VertexDecoder> TileAccel;

	bool InitRenderer(void* window)
	{
		return TileAccel.Init();
	}

	void TermRenderer()
	{
		TileAccel.Term();
	}

	void ResetRenderer(bool Manual)
	{
		TileAccel.Reset(Manual);
		FrameCount=0;
		VertexCount=0;
	}

	bool ThreadStart(void* window)
	{
		win=window;
		EnableOpenGL((HWND)win,hdc1,hglrc1);
		return true;
	}

	void ThreadEnd()
	{
		DisableOpenGL((HWND)win,hdc1,hglrc1);
	}
}


void GetOpenGLRenderer(rend_if* rif)
{
	//general init/term/reset
	rif->Init=OpenGLRenderer::InitRenderer;
	rif->Term=OpenGLRenderer::TermRenderer;
	rif->Reset=OpenGLRenderer::ResetRenderer;
	
	//thread init/term
	rif->ThreadStart=OpenGLRenderer::ThreadStart;
	rif->ThreadEnd=OpenGLRenderer::ThreadEnd;

	//drawing related functions :)
	rif->PresentFB=OpenGLRenderer::PresentFB;
	rif->StartRender=OpenGLRenderer::StartRender;
	
	//TA splitter i/f
	rif->Ta_ListCont=OpenGLRenderer::TileAccel.ListCont;
	rif->Ta_ListInit=OpenGLRenderer::TileAccel.ListInit;
	rif->Ta_SoftReset=OpenGLRenderer::TileAccel.SoftReset;

	rif->VertexCount=&OpenGLRenderer::VertexCount;
	rif->FrameCount=&OpenGLRenderer::FrameCount;
}

