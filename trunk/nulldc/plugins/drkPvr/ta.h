#pragma once
#include "drkPvr.h"

void TaWrite(u32* data);
void TADma(u32 address,u32* data,u32 size);

bool Ta_Init();
void Ta_Term();
void Ta_Reset(bool manual);

enum ListTypes
{
	Opaque=0,
	Opaque_Modifier_Volume=1,
	Translucent	=2,
	Translucent_Modifier_Volume=3,
	Punch_Through=4,
	//misc ones
	None=-1
};

enum ParamType
{
	End_Of_List=0,
	User_Tile_Clip=1,
	Object_List_Set=2,
	Reserved_1=3,
	Polygon_or_Modifier_Volume=4,
	Sprite=5,
	Reserved_2=6,
	Vertex_Parameter=7
};

struct Ta_Dma
{
	//0
	//Parameter Control Word
	struct
	{
		//Obj Control
		u32 UV_16bit		: 1;
		u32 Gouraud			: 1;
		u32 Offset			: 1;
		u32 Texture			: 1;
		u32 Col_Type		: 2;
		u32 Volume			: 1;
		u32 Shadow			: 1;
		u32	Reserved		: 8;
		
		// Group Control
		u32 User_Clip		: 2;
		u32 Strip_Len		: 2;
		u32 Res_2			: 3;
		u32 Group_En		: 1;
		
		// Para Control
		ListTypes ListType	: 3;
		u32 Res_1			: 1;
		u32 EndOfStrip		: 1;
		u32	ParaType: 3;		//Visual C++ 2005 bug : won't let me use the enum here :(:(:(
	} pcw;
	//4
	union
	{
		u8  data_8[32-4];
		u32 data_32[8-1];
	};
};


typedef void TaListFP(Ta_Dma* data);

void Ta_SoftReset();
void Ta_ListInit();
void Ta_ListCont();

struct TaVertex
{
	float x,y,z;
};

struct TaSprite
{
		u32 GlobalClip;
		u32 UserClip;
		u32 Flags;

		TaVertex* Vertex;//linked list of vertexes , 4 of em
};

struct TaPoly
{
		u32 GlobalClip;
		u32 UserClip;
		u32 Flags;

		TaVertex* Vertex;//linked list of vertexes
		u32 VertexCount;
};
