/*
**	TA_Param.h - David Miller 2006 - PowerVR2 Emulation Library
*/

#ifndef __TAPARAM_H__
#define __TAPARAM_H__

#include <vector>
using namespace std;

typedef struct ParameterControlWord
{
	u32 UVFormat		: 1;
	u32 Gouraud			: 1;
	u32 Offset			: 1;
	u32 Texture			: 1;
	u32 Col_Type		: 2;
	u32 Volume			: 1;
	u32 Shadow			: 1;
	u32	Reserved		: 8;
	// Obj Control

	u32 User_Clip		: 2;
	u32 Strip_Len		: 2;
	u32 Reserved_02		: 3;
	u32 Group_En		: 1;
	// Group Control

	u32 ListType		: 3;
	u32 Reserved_01		: 1;
	u32 EndOfStrip		: 1;
	u32	ParaType		: 3;
	// Para Control

} PCW, ParamCtrl;

typedef enum
{
	PT_EndOfList=0,		// Control: End Object List
	PT_UserTileClip,	// Control: User Tile Clip
	PT_ObjectListSet,	// Control: Object List Set
	PT_Reserved3,		// Control: Reserved Param

	PT_Polygon=4,		// Global: Polygon 
	PT_Modifier=4,		// Global: Modfifier Volume

	PT_Sprite,			// Global: Sprite
	PT_Reserved6,		// Global: Reserved Param

	PT_Vertex,			// Vertex Parameter

} TA_Param;

typedef enum
{
	LT_Opaque=0,		// Polygon or Sprite
	LT_OpaqueMod,		// Modifier Volume
	LT_Translucent,		// Polygon or Sprite
	LT_TransMod,		// Modifier Volume
	LT_PunchThrough,	// Polygon or Sprite
	LT_Reserved,		// Reserved List Type

} TA_ListType, LType;


typedef enum
{
	CT_Packed=0,		// 8b ARGB
	CT_Float,			// 32b Floating Point
	CT_Intensity1,		// The Face Color is specified by the immediately preceding Global Parameters.
	CT_Intensity2,		// Previous Face Color specified by Global Parameters in Intensity Mode 1 

} TA_ColType;



/////////


// For OpenGL //
#define RGBA(c) (((c)&0xFF00FF00) | (((c)&0xFF)<<16) | (((c)>>16)&0xFF))


//// ISP/TSP Instruction Word

typedef struct ISP_TSP_IWORD
{
	u32	Reserved	: 20;
	u32	DCalcCtrl	: 1;
	u32	CacheBypass	: 1;
	u32	UV_16b		: 1;	// redundant in many places
	u32	Gouraud		: 1;
	u32	Offset		: 1;
	u32	Texture		: 1;
	u32	ZWriteDis	: 1;
	u32	CullMode	: 2;
	u32	DepthMode	: 3;

} ISP_TSP, ISP ;


//// END ISP/TSP Instruction Word


//// TSP Instruction Word

typedef struct TSP_IWORD
{
	u32 TexV		: 3;
	u32 TexU		: 3;
	u32 ShadInstr	: 2;
	u32 MipMapD		: 4;
	u32 SupSample	: 1;
	u32 FilterMode	: 2;
	u32 ClampUV		: 2;
	u32 FlipUV		: 2;
	u32 IgnoreTexA	: 1;
	u32 UseAlpha	: 1;
	u32 ColorClamp	: 1;
	u32 FogCtrl		: 2;
	u32 DstSelect	: 1;	// Secondary Accum
	u32 SrcSelect	: 1;	// Primary Accum
	u32 DstInstr	: 3;
	u32 SrcInstr	: 3;

} TSP ;


//// END TSP Instruction Word


/// Texture Control Word

typedef struct TextureControlWord
{
	u32	TexAddr		:21;
	u32	Reserved	: 4;
	u32 StrideSel	: 1;
	u32 ScanOrder	: 1;
	u32	PixelFmt	: 3;
	u32 VQ_Comp		: 1;
	u32	MipMapped	: 1;

} TCW;

typedef struct PalTexControlWord
{
	u32 TexAddr		:21;
	u32 PalSelect	: 6;
	u32 PixelFmt	: 3;
	u32 VQ_Comp		: 1;
	u32 MipMapped	: 1;

} TCW_PAL;

/// END Texture Control Word



/*	Global Params Structs	*
*/

//// TA POLYGON STRUCTURE

typedef struct GlobalParam0	// (Packed/Floating Color)
{
	TSP tsp;
	TCW tcw;

	u32 Rsvd1;
	u32 Rsvd2;

	u32 SDMA_SIZE;
	u32 SDMA_ADDR;

} Param0;

typedef struct GlobalParam1 // (Intensity, no Offset Color)
{
	TSP tsp;
	TCW tcw;

	f32 FaceA, FaceR, FaceG, FaceB;

} Param1;

typedef struct GlobalParam2 // (Intensity, use Offset Color)
{
	TSP tsp;
	TCW tcw;

	u32 Rsvd1;
	u32 Rsvd2;

	u32 SDMA_SIZE;
	u32 SDMA_ADDR;

	f32 FaceA, FaceR, FaceG, FaceB;
	f32 OffsA, OffsR, OffsG, OffsB;

} Param2;

typedef struct GlobalParam3 // (Packed Color, with Two Volumes)
{
	TSP tsp0;
	TCW tcw0;

	TSP tsp1;
	TCW tcw1;

	u32 SDMA_SIZE;
	u32 SDMA_ADDR;

} Param3;

typedef struct GlobalParam4 // (Intensity, with Two Volumes)
{
	TSP tsp0;
	TCW tcw0;

	TSP tsp1;
	TCW tcw1;

	u32 SDMA_SIZE;
	u32 SDMA_ADDR;

	f32 Face0A, Face0R, Face0G, Face0B;
	f32 Face1A, Face1R, Face1G, Face1B;

} Param4;

typedef struct GlobalSprite
{
	TSP tsp;
	TCW tcw;

	u32 BaseCol;
	u32 OffsCol;

	u32 DataSize;
	u32 NextAddr;

} GP_Sprite;

// unneeded
typedef struct GlobalModifier
{
	u32 ignored[6];

} GP_Modifier, GP_Mod;

typedef struct GlobalParam
{
	PCW pcw;
	ISP isp;

	union
	{
		u8 Raw[64-8];

		Param0 param0;
		Param1 param1;
		Param2 param2;
		Param3 param3;
		Param4 param4;

		GP_Sprite sprite;
		GP_Modifier modifier;
	};

} GParam, PolyParam;


//// END GLOBAL PARAM STRUCTS


/*	Vertex Param Structs	*
*/

typedef struct Vertex0	//	(Non-Textured, Packed Color)
{
	f32 xyz[3];
	u32 ignore_1,
		ignore_2;
	u32 BaseCol;
	u32 ignore_3;

} Vtx0;

typedef struct Vertex1	//	(Non-Textured, Floating Color)
{
	f32 xyz[3];
	f32 BaseA, BaseR,
		BaseG, BaseB;

} Vtx1;

typedef struct Vertex2	//	(Non-Textured, Intensity)
{
	f32 xyz[3];
	u32 ignore_1,
		ignore_2;
	f32 BaseInt;
	u32 ignore_3;

} Vtx2;

typedef struct Vertex3	//	(Packed Color)
{
	f32 xyz[3];
	f32 u,v;
	u32 BaseCol;
	u32 OffsCol;

} Vtx3;

typedef struct Vertex4	//	(Packed Color, 16bit UV)
{
	f32 xyz[3];
	u32 u : 16;
	u32 v : 16;
	u32 ignore_1;
	u32 BaseCol;
	u32 OffsCol;

} Vtx4;

typedef struct Vertex5	//	(Floating Color)	
{
	f32 xyz[3];
	f32 u,v;
	u32 ignore_1;
	u32 ignore_2;
	f32 BaseA, BaseR,
		BaseG, BaseB;
	f32 OffsA, Offs,
		OffsG, OffsB;

} Vtx5;

typedef struct Vertex6	//	(Floating Color, 16bit UV)
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

} Vtx6;

typedef struct Vertex7	//	(Intensity)
{
	f32 xyz[3];
	f32 u,v;
	f32 BaseInt;
	f32 OffsInt;

} Vtx7;

typedef struct Vertex8	//	(Intensity, 16bit UV)
{
	f32 xyz[3];
	u32 u : 16;
	u32 v : 16;
	u32 ignore_1;
	f32 BaseInt;
	f32 OffsInt;

} Vtx8;

typedef struct Vertex9	//	(Non-Textured, Packed Color, with Two Volumes)
{
	f32 xyz[3];
	u32 BaseCol0;
	u32 BaseCol1;
	u32 ignore_1;
	u32 ignore_2;

} Vtx9;

typedef struct Vertex10	//	(Non-Textured, Intensity, with Two Volumes)
{
	f32 xyz[3];
	f32 BaseInt0;
	f32 BaseInt1;
	u32 ignore_1;
	u32 ignore_2;

} Vtx10;

typedef struct Vertex11	//	(Textured, Packed Color, with Two Volumes)
{
	f32 xyz[3];
	f32 u0,v0;
	u32 BaseCol0, OffsCol0;
	f32 u1,v1;
	u32 BaseCol1, OffsCol1;
	u32 ignore_1, ignore_2;
	u32 ignore_3, ignore_4;

} Vtx11;

typedef struct Vertex12	//	(Textured, Packed Color, 16bit UV, with Two Volumes)
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

} Vtx12;

typedef struct Vertex13	//	(Textured, Intensity, with Two Volumes)
{
	f32 xyz[3];
	f32 u0,v0;
	f32 BaseInt0, OffsInt0;
	f32 u1,v1;
	f32 BaseInt1, OffsInt1;
	u32 ignore_1, ignore_2;
	u32 ignore_3, ignore_4;

} Vtx13;

typedef struct Vertex14	//	(Textured, Intensity, 16bit UV, with Two Volumes)
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

} Vtx14;

typedef struct Sprite0	//	Line ?
{
	f32 x0,y0,z0;
	f32 x1,y1,z1;
	f32 x2,y2,z2;
	f32 x3,y3;
	u32 ignore_1, ignore_2;
	u32 ignore_3, ignore_4;

} Spr0;

typedef struct Sprite1
{
	f32 x0,y0,z0;
	f32 x1,y1,z1;
	f32 x2,y2,z2;
	f32 x3,y3;
	u32 ignore_1;

	u16 v0; u16 u0;	
	u16 v1; u16 u1;	
	u16 v2; u16 u2;	

} Spr1;

typedef struct ModifierVolume
{
	f32 x0,y0,z0;
	f32 x1,y1,z1;
	f32 x2,y2,z2;
	u32 ignore[6];

} ModifierVolume, ModVolume, Modifier, Mod, ModVol;


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

		ModVol		mvol;
	};

} VertexParam, Vtx;



// uh huh

struct AllocCtrl
{
	u32	O_OPB	: 2;
	u32	Ra		: 2;
	u32	OM_OPB	: 2;
	u32	Rb		: 2;
	u32	T_OPB	: 2;
	u32	Rc		: 2;
	u32	TM_OPB	: 2;
	u32	Rd		: 2;
	u32	PT_OPB	: 2;
	u32	Re		: 2;
	u32	OPB_Mode: 1;
	u32	Rsvd	:11;

};

extern AllocCtrl * ac;
extern u32 lists_complete;		// TODO, fix this shit, i had it fixed before, check gl plugin ..


#define combine_ac()	\
	if( ac->O_OPB	== 0 ) { lists_complete |= (1<<LT_Opaque);		}	\
	if( ac->T_OPB	== 0 ) { lists_complete |= (1<<LT_Translucent);	}	\
	if( ac->TM_OPB	== 0 ) { lists_complete |= (1<<LT_TransMod);	}	\
	if( ac->OM_OPB	== 0 ) { lists_complete |= (1<<LT_OpaqueMod);	}	\
	if( ac->PT_OPB	== 0 ) { lists_complete |= (1<<LT_PunchThrough);}





typedef enum
{
	PM_Vertex=0,
	PM_Sprite,
	PM_Modifier,

	PM_None,

} TA_PolyMode;


extern TA_PolyMode PolyMode;


enum ParamSize
{
	PS32 = 1,
	PS64 = 2,
};


__inline static bool isPoly64Byte(PCW * pcw)
{
	if((CT_Intensity1 == pcw->Col_Type) && pcw->Offset)
		return true;
	if((CT_Intensity1 == pcw->Col_Type) && pcw->Volume && pcw->Shadow)
		return true;

	return false;
}

__inline static bool isVert64Byte(PCW * pcw)
{
	if((PM_Sprite==PolyMode) || (PM_Modifier==PolyMode))
		return true;

	if(pcw->Texture)
	{
		if(pcw->Volume && pcw->Shadow)	// Textured w/ Two Volumes
			return true;

		if(CT_Float == pcw->Col_Type)	// Textured w/ Floating Point Color
			return true;
	}
	return false;
}

__inline static ParamSize GetParamSize(PCW * pcw)
{
	if(PT_Vertex  == pcw->ParaType && isVert64Byte(pcw))
		return PS64;
	if(PT_Polygon == pcw->ParaType && isPoly64Byte(pcw))
		return PS64;
	return PS32;
}




struct ParamBase
{
	union
	{
		struct 
		{
			PCW pcw;
			u8  Raw[32-4];

		} Base;

		u8 Full[32];
	};
};


struct Vert
{
	f32 xyz[3];
	u32 col;
	f32 uv[4];
};

struct Vertex
{
	u32 TexID;
	u32 ParamID;

	vector<Vert> List;
};



/*
**	Todo: Test Speed on this thing, and add a State format for ogl
**			then add some caching so we can (possibly) save at least
**			states in display lists, and possibly some geometry
*/


class PrimConverter
{
public:
	 PrimConverter() { ClearDCache(); }
	~PrimConverter() { ClearDCache(); }

	ParamSize AppendVert(VertexParam *vp);
	ParamSize AppendParam(GlobalParam *gp);
	ParamSize AppendSprite(VertexParam *vp);


//private:	// i dont like inheritance rules like this ...

	vector<Vertex> Sprites;
	vector<Vertex> OpaqueMods;
	vector<Vertex> TranspMods;
	vector<Vertex> OpaqueVerts;
	vector<Vertex> TranspVerts;
	vector<Vertex> PunchtVerts;

	vector<GlobalParam> GlobalParams;

	void ClearDCache() {
		Sprites.clear();
		OpaqueMods.clear();
		TranspMods.clear();
		OpaqueVerts.clear();
		TranspVerts.clear();
		PunchtVerts.clear();
		GlobalParams.clear();
	}

};



#endif //__TAPARAM_H__

