#include "ta.h"
using namespace TASplitter;

u32 VertexCount=0;
extern u32 FrameCount;
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
		static void fastcall AppendPolyParam0(TA_PolyParam0* pp)
	{
		glob_param_bdc;
	}
	__forceinline
		static void fastcall AppendPolyParam1(TA_PolyParam1* pp)
	{
		glob_param_bdc;
	}
	__forceinline
		static void fastcall AppendPolyParam2A(TA_PolyParam2A* pp)
	{
		glob_param_bdc;
	}
	__forceinline
		static void fastcall AppendPolyParam2B(TA_PolyParam2B* pp)
	{

	}
	__forceinline
		static void fastcall AppendPolyParam3(TA_PolyParam3* pp)
	{
		glob_param_bdc;
	}
	__forceinline
		static void fastcall AppendPolyParam4A(TA_PolyParam4A* pp)
	{
		glob_param_bdc;
	}
	__forceinline
		static void fastcall AppendPolyParam4B(TA_PolyParam4B* pp)
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

bool InitRenderer()
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
	VertexCount=0;
	FrameCount=0;
}
