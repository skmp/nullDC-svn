#pragma once
#include "drkPvr.h"
#include "ta.h"
#include "TexCache.h"

//basic i/f
typedef bool InitRendererFP(void* window);
typedef void TermRendererFP();
typedef void ResetRendererFP(bool Manual);

//more shit
//Present FB/Rendered data
//~~Watch~~ for rendered data/fb writes/RenderToTexture ...
typedef void VBlankFP();

typedef void StartRenderFP();
typedef void voidFunctRetvoid();
typedef void VramLockedWriteFP(vram_block* bl);
//Renderer interface
struct rend_if
{
	//interface
	InitRendererFP* Init;
	TermRendererFP* Term;
	ResetRendererFP*Reset;
	
	InitRendererFP* ThreadStart;
	TermRendererFP* ThreadEnd;

	VBlankFP*	VBlank;
	StartRenderFP*	StartRender;

	voidFunctRetvoid* Ta_ListInit;
	voidFunctRetvoid* Ta_SoftReset;
	voidFunctRetvoid* Ta_ListCont;
	VramLockedWriteFP* VramLockedWrite;
	
	//misc data
	bool Inited;
	void* Window;
	/*u32 VertexCount;
	u32 FrameCount;*/
};

extern u32 VertexCount;
extern u32 FrameCount;

//GetInterface
typedef void GetInterfaceFP(rend_if* rif);
//Current renderer
extern rend_if* renderer;
//hmm?
enum RendererType
{
	Sw_Null=0,			//no rendering ; pvr lle/none mix
	Sw_TileEmu=1,		//s/w emulation of tile renderer (pvr lle)
	Hw_OGL=2,			//h/w emulation of pvr (hle/lle mix) ; open GL
	Hw_D3d=3			//h/w emulation of pvr (hle/lle mix) ; Direct3D
};



bool SetRenderer(RendererType new_rend,void* Window);
bool rend_if_Init();
void rend_if_Term();
void rend_if_Reset(bool Manual);

bool rend_if_ThreadInit();
void rend_if_ThreadTerm();