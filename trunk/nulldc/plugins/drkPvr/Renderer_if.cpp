#include "Renderer_if.h"
#include "nullRend.h"
#include "oglRend.h"
#include "d3dRend.h"

//current renderer (can't change after init)
rend_if* renderer;
rend_if  rend_data;

u32 VertexCount=0;
u32 FrameCount=0;

//List of interfaces to renderers
GetInterfaceFP* rend_list[4]=
{
	GetNullRenderer,	//Sw_Null
	GetNullRenderer,	//Sw_TileEmu
	GetOpenGLRenderer,	//Hw_OGL
	GetDirect3DRenderer		//Hw_D3d
};

bool SetRenderer(RendererType new_rend,void* Window)
{
	if (renderer!=0)
	{
		if (renderer->Inited)
		{
			//can't change renderer after init
			return false;
		}
	}

	renderer=&rend_data;

	rend_list[new_rend](renderer);

	//check if i/f is ok
	if (renderer->Init==0)
		return false;

	if (renderer->Term ==0)
		return false;

	if (renderer->Reset ==0)
		return false;

	if (renderer->VBlank ==0)
		return false;

	renderer->Window=Window;
	return true;
}

//Init the selected renderer, if none selected select null
bool rend_if_Init()
{
	if (renderer==0)//none selected
	{
		//warn and
		//pick up the null one
		if (!SetRenderer(RendererType::Sw_Null,0))
		{
			//warn for further failure
			//and fail init
			return false;
		}
	}

	if (!renderer->Init(renderer->Window))
	{
		//warn for failure
		//and fail init
		return false;
	}

	renderer->Inited=true;
	return true;
}

//Term the selected renderer (must be inited first)
void rend_if_Term()
{
	renderer->Term();
	renderer->Inited=false;
}

//Reset the selected renderer (must be inited first)
void rend_if_Reset(bool Manual)
{
	renderer->Reset(Manual);
}
//Thread init/term (for OpenGL mailny , needs to be inited on caller thread ...)
bool rend_if_ThreadInit()
{
	return renderer->ThreadStart(renderer->Window);
}

void rend_if_ThreadTerm()
{
	renderer->ThreadEnd();
}


