#pragma once
#include "ta.h"

using namespace TASplitter;
bool InitRenderer();
void TermRenderer();
void ResetRenderer(bool Manual);
void ListCont();
void ListInit();
void SoftReset();

struct Vertex
{
	f32 x,y,z;

	//we may want to switch to fp colors for speed
	u32 argb;
};

struct PolyParam
{
	u32 first;
	u32 len;

	ISP_TSP isp;
	TSP tsp;
	TCW tcw;

	//TCW/TSP/Stuff here
};

extern u32 vertex_count;
extern Vertex verts[128*1024];
extern u32 pplist_op_size;
extern PolyParam pplist_op[48*1024];
extern u32 pplist_pt_size;
extern PolyParam pplist_pt[16*1024];
extern u32 pplist_tr_size;
extern PolyParam pplist_tr[16*1024];