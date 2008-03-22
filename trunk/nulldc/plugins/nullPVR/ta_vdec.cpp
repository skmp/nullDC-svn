#include "ta.h"
#include "ta_vdec.h"

using namespace TASplitter;

/*
	Vertex Decoding code

	The TASplitter splits the dma's according to their type/function/data/command , and calls the callbacks
	of the vertex decoder.

	All of the functions are inlined , so they must be as small as possible , and as generic as possible 
	(no special cases) to keep speed up.

	Typicaly , on a game we have (per frame):
	
	~ 4 to 8 Lists
	~ 0.3k to 2k strips (united strips , each one around 20 verts)
	~	0.3k to 2k PolyParams
	~	0.4k to 7k 'raw' strips
	~ 5k-30k verts (the most iv seen is 1.4M verts/sec on doa2/SC , @60 fps) 
	
	-> Note to self : double check the numbers by profiling later

	The plan is to make the inner code (Vert handling) as fast as possible , and move the complex code as up 
	as possible.

	The Vertex handlers should ONLY care about converting data , and advancing the vertex pointer.Even a single
	opcode less is important here....

	The strip handlers ONLY have to unite the strips.That needs a bit of work , but not too much.

	The parameter handlers should take care of Vertex counting/params

	The List handlers will have to do some misc work (like setting up list pointers) and to close the last
	PParam.


	How all that stuff is stored :
	Atm we use static arrays , witch is fast & simple.However , we will need to handle memory more efficiently,
	in order to get multythreaded rendering and TA contexts working.

	A good idea i just had :D (27/3/2007)
	
	A list usualy involves lota work to get an item.Howerver , we can use a normal pointer and just write to it.
	The list will allocate (SIZE+1) pages , and it will mark the last one as no access.When a write is done to it ,
	we can automaticaly grow the list :)

	Current Implementation :

	StartList	-> Setups pointers and first , null , poly param
	EndList		-> Ends the last PolyParam (if none , then it ends the null one ;) ) , and does PParam counts

	AppendPolyParam -> 
		Ends the last PolyParam (the first time it ends the null one)
		Increases PP pointer
		Converts & writes PP

	StartString -> 
	EndStrip ->

	AppendVertex : Convert & write vertex ,increase vertex pointer
*/

u32 vertex_count=0;
extern u32 FrameCount;

Vertex verts[128*1024];
u32 ppsize_op;
PolyParam pplist_op[48*1024];
u32 ppsize_pt;
PolyParam pplist_pt[16*1024];
u32 ppsize_tr;
PolyParam pplist_tr[16*1024];
PolyParam null_pp;

PolyParam* current_pp;
//u32* current_pp_count;
Vertex* current_vert;
Vertex* strip_start;

#define _debug_only(code) code

#define EndPolyParam current_pp->len=vertex_count - current_pp->first; strip_start=0;

//Takes care of pcw/isp_tsp , poly param ending & count ;)
template<typename T>
__forceinline PolyParam* NextPolyParam(T* pparam_raw)
{
	EndPolyParam;

	current_pp++;
	/*
	u32	UV_16b		: 1;	//In TA they are replaced
	u32	Gouraud		: 1;	//by the ones on PCW
	u32	Offset		: 1;	//
	u32	Texture		: 1;	// -- up to here --
	*/

	current_pp->isp.full=pparam_raw->isp.full;

	current_pp->isp.UV_16bit= pparam_raw->pcw.UV_16bit;
	current_pp->isp.Gouraud	= pparam_raw->pcw.Gouraud;
	current_pp->isp.Offset	= pparam_raw->pcw.Offset;
	current_pp->isp.Texture = pparam_raw->pcw.Texture;

	return current_pp;
}

#define colfa(to,from)	to[0]=from[0];to[1]=from[1];to[2]=from[2];to[3]=from[3];
#define colf(to,from)	to[0]=from##A;to[1]=from##R;to[2]=from##G;to[3]=from##B;
#define coli(to,from)	to[0]=1;to[1]=1;to[2]=1;to[3]=1;

#define vcolfa(to,from) colfa(current_vert->to,from)
#define vcolf(to,from) colf(current_vert->to,vtx->from)
#define vcoli(to,from) coli(current_vert->to,vtx->from)

float BaseIntesity[4]={1,1,1,1};
float OffsIntesity[4]={1,1,1,1};

//Fill that in w/ some vertex decoding :)
struct VertexDecoder
{
	//list handling
	__forceinline
		static void StartList(u32 ListType)
	{
		if (ListType==ListType_Opaque)
		{
			current_pp=&pplist_op[ppsize_op];
		}
		else if (ListType==ListType_Punch_Through)
		{
			current_pp=&pplist_pt[ppsize_pt];
		}
		else if (ListType==ListType_Translucent)
		{
			current_pp=&pplist_tr[ppsize_tr];
		}
		else
		{
			current_pp=&null_pp;
		}
	}
	__forceinline
		static void EndList(u32 ListType)
	{
		//end last PParam
		EndPolyParam;

		//Caclulate list size ..
		if (ListType==ListType_Opaque)
		{
			ppsize_op = (u32)(current_pp - &pplist_op[0]) + 1;
		}
		else if (ListType==ListType_Punch_Through)
		{
			ppsize_pt = (u32)(current_pp - &pplist_pt[0]) + 1;
		}
		else if (ListType==ListType_Translucent)
		{
			ppsize_tr = (u32)(current_pp - &pplist_tr[0]) + 1;
		}

		_debug_only(current_pp=0;)
	}

	//Polys
#define glob_param_bdc PolyParam* cpp=NextPolyParam(pp); \
		current_pp->first=vertex_count;

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
/*
	yeah , this needs some fixing ...
		if (strip_start==0)
		{
			strip_start=current_vert;// first strip
		}
		else
		{
			//more strips
		}
*/
		strip_start=current_vert;
	}
	__forceinline
		static void EndPolyStrip()
	{
		vertex_count += (u32)(current_vert-strip_start);
	}

	//Poly Vertex handlers
#define vert_cvt_base \
	current_vert->x=vtx->xyz[0];\
	current_vert->y=vtx->xyz[1];\
	current_vert->z=vtx->xyz[2];\
	


	//(Non-Textured, Packed Color)
	__forceinline
		static void AppendPolyVertex0(TA_Vertex0* vtx)
	{
		vert_cvt_base;
		
		vcoli(base,BaseCol);

		current_vert++;
	}

	//(Non-Textured, Floating Color)
	__forceinline
		static void AppendPolyVertex1(TA_Vertex1* vtx)
	{
		vert_cvt_base;

		vcolf(base,Base);

		current_vert++;
	}

	//(Non-Textured, Intensity)
	__forceinline
		static void AppendPolyVertex2(TA_Vertex2* vtx)
	{
		vert_cvt_base;
		
		vcolfa(base,BaseIntesity);

		current_vert++;
	}

	//(Textured, Packed Color)
	__forceinline
		static void AppendPolyVertex3(TA_Vertex3* vtx)
	{
		vert_cvt_base;

		vcoli(base,vtx->BaseCol);

		current_vert++;
	}

	//(Textured, Packed Color, 16bit UV)
	__forceinline
		static void AppendPolyVertex4(TA_Vertex4* vtx)
	{
		vert_cvt_base;

		vcoli(base,vtx->BaseCol);

		current_vert++;
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

		vcolf(base,Base);
		vcolf(offs,Offs);
		current_vert++;
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

		vcolf(base,Base);
		vcolf(offs,Offs);

		current_vert++;
	}

	//(Textured, Intensity)
	__forceinline
		static void AppendPolyVertex7(TA_Vertex7* vtx)
	{
		vert_cvt_base;

		vcolfa(base,BaseIntesity);

		current_vert++;
	}

	//(Textured, Intensity, 16bit UV)
	__forceinline
		static void AppendPolyVertex8(TA_Vertex8* vtx)
	{
		vert_cvt_base;

		vcolfa(base,BaseIntesity);

		current_vert++;
	}

	//(Non-Textured, Packed Color, with Two Volumes)
	__forceinline
		static void AppendPolyVertex9(TA_Vertex9* vtx)
	{
		vert_cvt_base;

		vcoli(base,Base);

		current_vert++;
	}

	//(Non-Textured, Intensity,	with Two Volumes)
	__forceinline
		static void AppendPolyVertex10(TA_Vertex10* vtx)
	{
		vert_cvt_base;

		vcolfa(base,BaseIntesity);

		current_vert++;
	}

	//(Textured, Packed Color,	with Two Volumes)	
	__forceinline
		static void AppendPolyVertex11A(TA_Vertex11A* vtx)
	{
		vert_cvt_base;

		vcoli(base,BaseCol0);
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

		vcoli(base,vtx->BaseCol0);
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

		vcolfa(base,BaseIntesity);
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

		vcolfa(base,BaseIntesity);
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
		ppsize_op=0;
		ppsize_tr=0;
		ppsize_pt=0;
		vertex_count=0;
		current_vert = &verts[0];

	}
	__forceinline
		static void SoftReset()
	{
		ppsize_op=0;
		ppsize_tr=0;
		ppsize_pt=0;
		vertex_count=0;
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
	vertex_count=0;
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