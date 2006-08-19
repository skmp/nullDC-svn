#pragma once
#include "drkPvr.h"

//void TaWrite(u32* data);
void TADma(u32 address,u32* data,u32 size);

bool Ta_Init();
void Ta_Term();
void Ta_Reset(bool manual);

//hehe
//as it sems , bit 1,2 are type , bit 0 is mod volume :p
const u32 ListType_Opaque=0;			
const u32 ListType_Opaque_Modifier_Volume=1;
const u32 ListType_Translucent	=2;
const u32 ListType_Translucent_Modifier_Volume=3;
const u32 ListType_Punch_Through=4;
#define IsModVolList(list) (((list)&1)!=0)

//misc ones
const u32 ListType_None=-1;

//Control Parameter
const u32 ParamType_End_Of_List=0;
const u32 ParamType_User_Tile_Clip=1;
const u32 ParamType_Object_List_Set=2;

//Global Parameter
const u32 ParamType_Polygon_or_Modifier_Volume=4;
const u32 ParamType_Sprite=5;

//Vertex , Sprite or ModVolume Parameter
const u32 ParamType_Vertex_Parameter=7;

//Reserved
const u32 ParamType_Reserved_1=3;
const u32 ParamType_Reserved_2=6;

#pragma pack(push, 1)   // n = 1
//4B
struct PCW
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
	u32 ListType		: 3;
	u32 Res_1			: 1;
	u32 EndOfStrip		: 1;
	u32	ParaType		: 3;		
} ;

//32B
struct Ta_Dma
{
	//0
	//Parameter Control Word
	PCW pcw;
	//4
	union
	{
		u8  data_8[32-4];
		u32 data_32[8-1];
	};
};

//64B
struct TaVertex
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
struct TaVertexList
{
	u32 first;
	u32 sz;
};

//16B
struct TaGlobalParam
{
	u32 type;		//0 - poly , 1 - sprite , 2 - mod volume
	u32 first;		//entry index , holds vertex/pos data
	u32 vlc;
	u32 nil;		//yay padding
};
#pragma pack(pop)

typedef u32 fastcall TaListFP(Ta_Dma* data,u32 size);

void Ta_SoftReset();
void Ta_ListInit();
void Ta_ListCont();

extern List<TaVertex> verts_pv;
extern List<TaVertexList> vertlists_pv;
extern List<TaGlobalParam> global_param_op_pv[14];
extern List<TaGlobalParam> global_param_tr_pv[14];



#pragma pack(push, 1)   // n = 1

/*	Vertex Param Structs	*
*/
struct Vertex0	//	(Non-Textured, Packed Color)
{
	f32 xyz[3];
	u32 ignore_1,
		ignore_2;
	u32 BaseCol;
	u32 ignore_3;

};

struct Vertex1	//	(Non-Textured, Floating Color)
{
	f32 xyz[3];
	f32 BaseA, BaseR,
		BaseG, BaseB;

};

struct Vertex2	//	(Non-Textured, Intensity)
{
	f32 xyz[3];
	u32 ignore_1,
		ignore_2;
	f32 BaseInt;
	u32 ignore_3;

};

struct Vertex3	//	(Packed Color)
{
	f32 xyz[3];
	f32 u,v;
	u32 BaseCol;
	u32 OffsCol;

};

struct Vertex4	//	(Packed Color, 16bit UV)
{
	f32 xyz[3];
	u32 u : 16;
	u32 v : 16;
	u32 ignore_1;
	u32 BaseCol;
	u32 OffsCol;

};

struct Vertex5	//	(Floating Color)	
{
	f32 xyz[3];
	f32 u,v;
	u32 ignore_1;
	u32 ignore_2;
	f32 BaseA, BaseR,
		BaseG, BaseB;
	f32 OffsA, Offs,
		OffsG, OffsB;

};

struct Vertex6	//	(Floating Color, 16bit UV)
{
	f32 xyz[3];
	u32 u : 16;
	u32 v : 16;
	u32 ignore_1;
	u32 ignore_2;
	u32 ignore_3;
	f32 BaseA, BaseR,
		BaseG, BaseB;
	f32 OffsA, Offs,
		OffsG, OffsB;

};

struct Vertex7	//	(Intensity)
{
	f32 xyz[3];
	f32 u,v;
	f32 BaseInt;
	f32 OffsInt;

};

struct Vertex8	//	(Intensity, 16bit UV)
{
	f32 xyz[3];
	u32 u : 16;
	u32 v : 16;
	u32 ignore_1;
	f32 BaseInt;
	f32 OffsInt;

};

struct Vertex9	//	(Non-Textured, Packed Color, with Two Volumes)
{
	f32 xyz[3];
	u32 BaseCol0;
	u32 BaseCol1;
	u32 ignore_1;
	u32 ignore_2;

};

struct Vertex10	//	(Non-Textured, Intensity, with Two Volumes)
{
	f32 xyz[3];
	f32 BaseInt0;
	f32 BaseInt1;
	u32 ignore_1;
	u32 ignore_2;

};

struct Vertex11	//	(Textured, Packed Color, with Two Volumes)
{
	f32 xyz[3];
	f32 u0,v0;
	u32 BaseCol0, OffsCol0;
	f32 u1,v1;
	u32 BaseCol1, OffsCol1;
	u32 ignore_1, ignore_2;
	u32 ignore_3, ignore_4;

};

struct Vertex12	//	(Textured, Packed Color, 16bit UV, with Two Volumes)
{
	f32 xyz[3];
	u32 u0 : 16;
	u32 v0 : 16;
	u32 ignore_1;
	u32 BaseCol0, OffsCol0;
	u32 u1 : 16;
	u32 v1 : 16;
	u32 ignore_2;
	u32 BaseCol1, OffsCol1;
	u32 ignore_3, ignore_4;
	u32 ignore_5, ignore_6;

};

struct Vertex13	//	(Textured, Intensity, with Two Volumes)
{
	f32 xyz[3];
	f32 u0,v0;
	f32 BaseInt0, OffsInt0;
	f32 u1,v1;
	f32 BaseInt1, OffsInt1;
	u32 ignore_1, ignore_2;
	u32 ignore_3, ignore_4;

};

struct Vertex14	//	(Textured, Intensity, 16bit UV, with Two Volumes)
{
	f32 xyz[3];
	u32 u0 : 16;
	u32 v0 : 16;
	u32 ignore_1;
	f32 BaseInt0, OffsInt0;
	u32 u1 : 16;
	u32 v1 : 16;
	u32 ignore_2;
	f32 BaseInt1, OffsInt1;
	u32 ignore_3, ignore_4;
	u32 ignore_5, ignore_6;

};

struct Sprite0	//	Line ?
{
	f32 x0,y0,z0;
	f32 x1,y1,z1;
	f32 x2,y2,z2;
	f32 x3,y3;
	u32 ignore_1, ignore_2;
	u32 ignore_3, ignore_4;

};

struct Sprite1
{
	f32 x0,y0,z0;
	f32 x1,y1,z1;
	f32 x2,y2,z2;
	f32 x3,y3;
	u32 ignore_1;

	u16 v0; u16 u0;	
	u16 v1; u16 u1;	
	u16 v2; u16 u2;	

};

struct ModifierVolume
{
	f32 x0,y0,z0;
	f32 x1,y1,z1;
	f32 x2,y2,z2;
	u32 ignore[6];

};


typedef struct VertexParam
{
	PCW pcw;

	union
	{
		u8 Raw[64-4];

		Vertex0		vtx0;
		Vertex1		vtx1;
		Vertex2		vtx2;
		Vertex3		vtx3;
		Vertex4		vtx4;
		Vertex5		vtx5;
		Vertex6		vtx6;
		Vertex7		vtx7;
		Vertex8		vtx8;
		Vertex9		vtx9;
		Vertex10	vtx10;
		Vertex11	vtx11;
		Vertex12	vtx12;
		Vertex13	vtx13;
		Vertex14	vtx14;

		Sprite0		spr0;
		Sprite1		spr1;

		ModifierVolume		mvol;
	};

} VertexParam, Vtx;

#pragma pack(pop)
extern f32 z_min;
extern f32 z_max;