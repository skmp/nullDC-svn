#include "nullRend.h"

#include "d3dRend.h"
#include "windows.h"
#include "gl\gl.h"
#include "regs.h"
#include <d3d9.h>

namespace Direct3DRenderer
{
	IDirect3D9* d3d9;
	IDirect3DDevice9* dev;
	//use that someday
	void PresentFB()
	{
	}

	u32 VertexCount;
	u32 FrameCount;

	void StartRender()
	{
		render_end_pending_cycles=100000;
		if (FB_W_SOF1 & 0x1000000)
			return;
		FrameCount++;
	}

	using namespace TASplitter;


	struct VertexDecoder
	{
		//list handling
		__forceinline
		static void StartList(u32 ListType)
		{
			
		}
		__forceinline
		static void EndList(u32 ListType)
		{
			
		}

		//Polys
#define glob_param_bdc

		__forceinline
		static void AppendPolyParam32(TA_PolyParamA* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void AppendPolyParam64A(TA_PolyParamA* pp)
		{
			glob_param_bdc;
		}
		__forceinline
		static void AppendPolyParam64B(TA_PolyParamB* pp)
		{

		}

		//Poly Strip handling
		__forceinline
		static void StartPolyStrip()
		{
			
		}
		__forceinline
		static void EndPolyStrip()
		{
			
		}

			//Poly Vertex handlers

#define vert_cvt_base VertexCount++;


		//(Non-Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex0(TA_Vertex0* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex1(TA_Vertex1* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Intensity)
		__forceinline
		static void AppendPolyVertex2(TA_Vertex2* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Packed Color)
		__forceinline
		static void AppendPolyVertex3(TA_Vertex3* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Packed Color, 16bit UV)
		__forceinline
		static void AppendPolyVertex4(TA_Vertex4* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Floating Color)
		__forceinline
		static void AppendPolyVertex5A(TA_Vertex5A* vtx)
		{
			vert_cvt_base;
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
		}

		//(Textured, Intensity, 16bit UV)
		__forceinline
		static void AppendPolyVertex8(TA_Vertex8* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Packed Color, with Two Volumes)
		__forceinline
		static void AppendPolyVertex9(TA_Vertex9* vtx)
		{
			vert_cvt_base;
		}

		//(Non-Textured, Intensity,	with Two Volumes)
		__forceinline
		static void AppendPolyVertex10(TA_Vertex10* vtx)
		{
			vert_cvt_base;
		}

		//(Textured, Packed Color,	with Two Volumes)	
		__forceinline
		static void AppendPolyVertex11A(TA_Vertex11A* vtx)
		{
			vert_cvt_base;
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

		}
		__forceinline
		static void SoftReset()
		{
		}
	};

	FifoSplitter<VertexDecoder> TileAccel;

	bool InitRenderer(void* window)
	{
		d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

		D3DPRESENT_PARAMETERS ppar;
		memset(&ppar,0,sizeof(ppar));
		
		ppar.MultiSampleType = D3DMULTISAMPLE_NONE;
		ppar.BackBufferCount=3;
		ppar.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
		ppar.AutoDepthStencilFormat = D3DFMT_D24S8;
		ppar.BackBufferFormat = D3DFMT_R8G8B8;
		ppar.EnableAutoDepthStencil=true;
		ppar.hDeviceWindow=(HWND)Hwnd;

		
		verifyc(d3d9->CreateDevice(0,D3DDEVTYPE_HAL,(HWND)Hwnd,D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE,&ppar,&dev));

		return TileAccel.Init();
	}

	void TermRenderer()
	{
		TileAccel.Term();
	}

	void ResetRenderer(bool Manual)
	{
		TileAccel.Reset(Manual);
		VertexCount=0;
		FrameCount=0;
	}

	bool ThreadStart(void* window)
	{
		return true;
	}

	void ThreadEnd()
	{

	}
}

//Get null i/f
void GetDirect3DRenderer(rend_if* rif)
{
	//general init/term/reset
	rif->Init=Direct3DRenderer::InitRenderer;
	rif->Term=Direct3DRenderer::TermRenderer;
	rif->Reset=Direct3DRenderer::ResetRenderer;
	
	//thread init/term
	rif->ThreadStart=Direct3DRenderer::ThreadStart;
	rif->ThreadEnd=Direct3DRenderer::ThreadEnd;

	//drawing related functions :)
	rif->PresentFB=Direct3DRenderer::PresentFB;
	rif->StartRender=Direct3DRenderer::StartRender;
	
	//TA splitter i/f
	rif->Ta_ListCont=Direct3DRenderer::TileAccel.ListCont;
	rif->Ta_ListInit=Direct3DRenderer::TileAccel.ListInit;
	rif->Ta_SoftReset=Direct3DRenderer::TileAccel.SoftReset;

	rif->VertexCount=&Direct3DRenderer::VertexCount;
	rif->FrameCount=&Direct3DRenderer::FrameCount;
}