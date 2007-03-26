#include "ta.h"
#include "ta_vdec.h"

using namespace TASplitter;

u32 VertexCount=0;
extern u32 FrameCount;

Vertex verts[128*1024];
u32 pplist_op_size;
PolyParam pplist_op[48*1024];
u32 pplist_pt_size;
PolyParam pplist_pt[16*1024];
u32 pplist_tr_size;
PolyParam pplist_tr[16*1024];

PolyParam* current_pp;
u32* current_pp_count;
Vertex* current_vert;

//Fill that in w/ some vertex decoding :)
struct VertexDecoder
{
	//list handling
	__forceinline
		static void StartList(u32 ListType)
	{
		if (ListType==ListType_Opaque)
		{
			current_pp=&pplist_op[pplist_op_size];
			current_pp_count=&pplist_op_size;
		}
		else if (ListType==ListType_Punch_Through)
		{
			current_pp=&pplist_pt[pplist_pt_size];
			current_pp_count=&pplist_pt_size;
		}
		else if (ListType==ListType_Translucent)
		{
			current_pp=&pplist_tr[pplist_tr_size];
			current_pp_count=&pplist_tr_size;
		}
	}
	__forceinline
		static void EndList(u32 ListType)
	{
		current_pp=0;
	}

	//Polys
#define glob_param_bdc (*current_pp_count)++; \
		current_pp->first=VertexCount;

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
		glob_param_bdc;
	}
	__forceinline
		static void EndPolyStrip()
	{
		current_pp->len=VertexCount - current_pp->first +1;
		current_pp++;
	}

	//Poly Vertex handlers
#define vert_cvt_base VertexCount++; \
	current_vert->x=vtx->xyz[0];\
	current_vert->y=vtx->xyz[1];\
	current_vert->z=vtx->xyz[2];\
	


	//(Non-Textured, Packed Color)
	__forceinline
		static void AppendPolyVertex0(TA_Vertex0* vtx)
	{
		vert_cvt_base;
		
		current_vert->argb = vtx->BaseCol;

		current_vert++;
	}

	//(Non-Textured, Floating Color)
	__forceinline
		static void AppendPolyVertex1(TA_Vertex1* vtx)
	{
		vert_cvt_base;

		current_vert->argb = 0xFFFFFFFF;

		current_vert++;
	}

	//(Non-Textured, Intensity)
	__forceinline
		static void AppendPolyVertex2(TA_Vertex2* vtx)
	{
		vert_cvt_base;
		
		current_vert->argb = 0xFFFFFFFF;

		current_vert++;
	}

	//(Textured, Packed Color)
	__forceinline
		static void AppendPolyVertex3(TA_Vertex3* vtx)
	{
		vert_cvt_base;

		current_vert->argb = vtx->BaseCol;

		current_vert++;
	}

	//(Textured, Packed Color, 16bit UV)
	__forceinline
		static void AppendPolyVertex4(TA_Vertex4* vtx)
	{
		vert_cvt_base;

		current_vert->argb = vtx->BaseCol;

		current_vert++;
	}

	//(Textured, Floating Color)
	__forceinline
		static void AppendPolyVertex5A(TA_Vertex5A* vtx)
	{
		vert_cvt_base;

		current_vert->argb = 0xFFFFFFFF;

	}
	__forceinline
		static void AppendPolyVertex5B(TA_Vertex5B* vtx)
	{

		current_vert++;
	}

	//(Textured, Floating Color, 16bit UV)
	__forceinline
		static void AppendPolyVertex6A(TA_Vertex6A* vtx)
	{
		vert_cvt_base;

		current_vert->argb = 0xFFFFFFFF;
	}
	__forceinline
		static void AppendPolyVertex6B(TA_Vertex6B* vtx)
	{

		current_vert++;
	}

	//(Textured, Intensity)
	__forceinline
		static void AppendPolyVertex7(TA_Vertex7* vtx)
	{
		vert_cvt_base;

		current_vert->argb = 0xFFFFFFFF;

		current_vert++;
	}

	//(Textured, Intensity, 16bit UV)
	__forceinline
		static void AppendPolyVertex8(TA_Vertex8* vtx)
	{
		vert_cvt_base;

		current_vert->argb = 0xFFFFFFFF;

		current_vert++;
	}

	//(Non-Textured, Packed Color, with Two Volumes)
	__forceinline
		static void AppendPolyVertex9(TA_Vertex9* vtx)
	{
		vert_cvt_base;

		current_vert++;
	}

	//(Non-Textured, Intensity,	with Two Volumes)
	__forceinline
		static void AppendPolyVertex10(TA_Vertex10* vtx)
	{
		vert_cvt_base;

		current_vert->argb = 0xFFFFFFFF;

		current_vert++;
	}

	//(Textured, Packed Color,	with Two Volumes)	
	__forceinline
		static void AppendPolyVertex11A(TA_Vertex11A* vtx)
	{
		vert_cvt_base;

		current_vert->argb = vtx->BaseCol0;
	}
	__forceinline
		static void AppendPolyVertex11B(TA_Vertex11B* vtx)
	{

		current_vert++;
	}

	//(Textured, Packed Color, 16bit UV, with Two Volumes)
	__forceinline
		static void AppendPolyVertex12A(TA_Vertex12A* vtx)
	{
		vert_cvt_base;

		current_vert->argb = vtx->BaseCol0;
	}
	__forceinline
		static void AppendPolyVertex12B(TA_Vertex12B* vtx)
	{

		current_vert++;
	}

	//(Textured, Intensity,	with Two Volumes)
	__forceinline
		static void AppendPolyVertex13A(TA_Vertex13A* vtx)
	{
		vert_cvt_base;		

		current_vert->argb = 0xFFFFFFFF;
	}
	__forceinline
		static void AppendPolyVertex13B(TA_Vertex13B* vtx)
	{

		current_vert++;
	}

	//(Textured, Intensity, 16bit UV, with Two Volumes)
	__forceinline
		static void AppendPolyVertex14A(TA_Vertex14A* vtx)
	{
		vert_cvt_base;

		current_vert->argb = 0xFFFFFFFF;
	}
	__forceinline
		static void AppendPolyVertex14B(TA_Vertex14B* vtx)
	{

		current_vert++;
	}

	//Sprites
	__forceinline
		static void AppendSpriteParam(TA_SpriteParam* spr)
	{

	}

	//Sprite Vertex Handlers
	__forceinline
		static void AppendSpriteVertexA(TA_Sprite1A* sv)
	{

	}
	__forceinline
		static void AppendSpriteVertexB(TA_Sprite1B* sv)
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
		pplist_op_size=0;
		pplist_tr_size=0;
		pplist_pt_size=0;
		VertexCount=0;
		current_vert = &verts[0];

	}
	__forceinline
		static void SoftReset()
	{
		pplist_op_size=0;
		pplist_tr_size=0;
		pplist_pt_size=0;
		VertexCount=0;
		current_vert = &verts[0];

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


void ListCont()
{
	TileAccel.ListCont();
}

void ListInit()
{
	TileAccel.ListInit();
}
void SoftReset()
{
	TileAccel.SoftReset();
}