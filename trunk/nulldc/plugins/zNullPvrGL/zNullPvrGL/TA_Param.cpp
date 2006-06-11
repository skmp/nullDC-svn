/*
**	TA_Param.cpp - David Miller 2006 - PowerVR2 Emulation Library
*/
#include "PowerVR2.h"

PowerVR2 * PvrIf;

AllocCtrl * ac;
u32 lists_complete=0;

TA_PolyMode PolyMode;

vector<ParamBase> ParamFifo;

u32 ProcessParam(ParamBase *pb);



//__forceinline static
u32 ProcessParam(ParamBase *pb)
{
	PCW * pcw = &pb->Base.pcw;

	switch(pcw->ParaType)
	{
	case PT_EndOfList:		// Control: End Object List
		PolyMode = PM_None;

		if(PvrIf->GlobalParams.size() > 0)
		{
			ASSERT_T((LT_Reserved<=PvrIf->GlobalParams[PvrIf->GlobalParams.size()-1].pcw.ListType),"<PVR> EndOfList: Reserved List Type !");

			emuIf.RaiseInterrupt(PvrInts[PvrIf->GlobalParams[PvrIf->GlobalParams.size()-1].pcw.ListType]);
		}
		else
		{
			ASSERT_T((1),"EndOfList, GlobalParamSize==0 \n");

			emuIf.RaiseInterrupt(PvrInts[pcw->ListType]);		// not exactly correct but it'll have to do
		}

	return 1;

	case PT_Polygon:		// Global: Polygon 
//	case PT_Modifier=4,		// Global: Modfifier Volume

		PolyMode = (pcw->Volume && !pcw->Shadow) ? PM_Modifier : PM_Vertex;
//		ASSERT_T((PM_Vertex == PolyMode && isPoly64Byte(pcw)), "<PVR> 64By Global Polygon Param !");

		if(PM_Vertex == PolyMode)
			return PvrIf->AppendParam((GlobalParam*)pcw);
		else
			return isPoly64Byte(pcw) ? PS64 : PS32;


	case PT_Sprite:			// Global: Sprite
		PolyMode = PM_Sprite;
		return PvrIf->AppendParam((GlobalParam*)pcw);


	case PT_Vertex:			// Vertex Parameter
		ASSERT_T((PM_None==PolyMode),		"<PVR> Vertex Recieved After Object List Ended !");

		if(PM_Vertex == PolyMode) {
			return PvrIf->AppendVert((VertexParam*)pcw);
		}
		if(PM_Sprite == PolyMode) {
			return PvrIf->AppendSprite((VertexParam*)pcw);
		}
		if(PM_Modifier == PolyMode) {	// Not Handled Yet *FIXME*
			return PS64;
		}
		ASSERT_T((1), "PT_Vertex: Reached const. return value (ERROR)!");
		return isVert64Byte(pcw)?PS64:PS32;


	case PT_UserTileClip:	// Control: User Tile Clip	*FIXME*
	case PT_ObjectListSet:	// Control: Object List Set
	//	ASSERT_T((1),"<PVR> Unhandled ListType (ObjList || TileClip)!\n");
		return 1;

	case PT_Reserved3:		// Control: Reserved Param
	case PT_Reserved6:		// Global: Reserved Param
		bLogEnabled=true;
		DebugOptions=1;
		ASSERT_T((1),"<PVR> Illegal ListType (Reserved)!\n");
		return 1;
	}

	ASSERT_T((1),"<PVR> ERROR Invalid ListType, Out of Context!\n");
	return 1;
}




void DumpFifo(u32 address, u32* data, u32 size);
void TaFifo(u32 address, u32* data, u32 size)
{
	bool bd=false;
	bool bp=false;

	for(u32 i=0; i<size; i++)
	{
		ParamFifo.push_back(((ParamBase*)data)[i]);

		if(PT_EndOfList == ((ParamBase*)data)[i].Base.pcw.ParaType)
		{
			bp = true;
			//lprintf("<PVR> Processing EndOfList, ParamFifo size: %d\n", ParamFifo.size());

			size_t tmp=0;
			for(size_t p=0; p<ParamFifo.size(); )
			{
				p += ProcessParam(&ParamFifo[p]);
			/*	tmp = ProcessParam(&ParamFifo[p]);

				lprintf("--\t [%06x] + %d: pointero: %X | ParaType: %d \n",
					p, tmp*32, (u32)&ParamFifo[p], ((ParamBase*)&ParamFifo[p])->Base.pcw.ParaType);

				p += tmp;*/
#ifdef DEBUG_LIB
				if(DebugOptions&1 && !bd) {
					DumpFifo(address,data,size);
					DebugOptions &= ~1;
					bd=true;
				}
#endif
			}
			ParamFifo.clear();

			// This is a good hack, saves sword of the berserks ass, but where is the real bug?
#ifdef DEBUG_LIB
		/*	if(size != (i+1)) {
				printf("<PVR> ParamFifo, EndOfList and i+1(%X) != size(%X) !\n", (i+1), size);
			//	return;	// hack?
			}*/
#endif
		}
	}
}










////////////////////////// below is old code .. looks useable


# define stype(a, b) (((a)<<16) | ((b)&0xFFFF))

__inline static u32 PolyType( PCW *pcw )
{
	//	if( (pcw->ListType == LT_OpaqueMod) || (pcw->ListType == LT_TransMod) )
	//		return 0x0600FF;	// mod volume

	if( !pcw->Texture ) {
		if( !pcw->Volume ) {
			switch( pcw->Col_Type ) {
			case 0:		return stype(0,0);
			case 1:		return stype(0,1);
			case 2:		return stype(1,2);
			case 3:		return stype(0,2);
			}
		} else {
			switch( pcw->Col_Type ) {
			case 0:		return stype(3,9);
			case 1:		return 0xFFFFFFFF;
			case 2:		return stype(4,10);
			case 3:		return stype(3,10);
			}
		}
	} else {
		if( !pcw->Volume ) {
			switch( pcw->Col_Type ) {
			case 0:		return pcw->UVFormat ? stype(0,4) : stype(0,3);
			case 1:		return pcw->UVFormat ? stype(0,6) : stype(0,5);
			case 2:
				if( pcw->Offset )	{ return pcw->UVFormat ? stype(2,8) : stype(2,7) ; }
				else				{ return pcw->UVFormat ? stype(1,8) : stype(1,7) ; }
			
			case 3:		return pcw->UVFormat ? stype(0,8) : stype(0,7);
			}
		} else {
			switch( pcw->Col_Type ) {
			case 0:		return pcw->UVFormat ? stype(3,12) : stype(3,11);
			case 1:		return 0xFFFFFFFF;
			case 2:		return pcw->UVFormat ? stype(4,14) : stype(4,13);
			case 3:		return pcw->UVFormat ? stype(3,14) : stype(3,13);
			}
		}
	}
	return 0xFFFFFFFF;	// fucked
}


/*
*	Watch, my old code used PolyType of the GParam, not the vtx !
*		Should do some ASSERTS to make sure we are always getting
*		the same PCWs ... OK CHANGED, now uses GParam/PolyParam !
*/





__inline float f16(u32 x)
{
	u32 t = x<<16;
	float f = *(float*)(&t);
	return f;
}











/*
**	class PrimConverter
*/



int PrimConverter::AppendParam(GlobalParam *gp)
{
	GlobalParams.push_back(*(GlobalParam*)gp);
	return isPoly64Byte((PCW*)&gp->pcw)?(PS64):(PS32);
}


__forceinline static u8 NFloat2UB(float NCF)
{
#ifdef DEBUG_LIB
	return (u8)(((NCF > 1.f) ? 1.f : ((NCF < 0.f) ? 0.f : NCF)) * 255.f);
#else
	return (u8)(((NCF > 1.f) ? 1.f:NCF) * 255.f);
#endif
}

#define FULL_LIST	// process all verts at once, should be faster

int PrimConverter::AppendVert(VertexParam *vp)
{
	static u32 LastType=420;
	static Vertex tmpVert;

	PolyParam * pp = &GlobalParams[GlobalParams.size()-1];

	u32 PType = PolyType(&pp->pcw);
	ASSERT_T((420!=LastType && PType != LastType), "<PVR> AppendVert PType != LastType!\n");
	LastType = PType;

	ParamSize VSize = isVert64Byte((PCW*)&pp->pcw) ? (PS64) : (PS32);

#ifdef FULL_LIST
	// should be best looping here..
	int retsize=0;
	VertexParam* ovp=vp;

	do
	{
	vp = (VertexParam*)((int)ovp + retsize*32);
#endif
	u8 tcol = 0;
	Vert vertex;
	vertex.uv[0]  = vertex.uv[1] = 
	vertex.uv[2]  = vertex.uv[3] = 0.f;	// Stop runtime check failure!



	//lprintf("PType: %X (PCW: %02X)\n", PType, *(u8*)&pp->pcw);

//	lprintf("AppendVert: (%f,%f,%f) EOS: %d Type: %X,VSize: %d \n",
//		vertex.xyz[0], vertex.xyz[1], vertex.xyz[2],
//		vp->pcw.EndOfStrip, PType, VSize*32);

	switch(PType&0xFFFF)
	{
	case 0:
		vertex.col		= RGBA(vp->vtx0.BaseCol);
		break;

	case 1:
		vertex.col		=	NFloat2UB(vp->vtx1.BaseA) << 24;
		vertex.col		|=	NFloat2UB(vp->vtx1.BaseR) << 0;
		vertex.col		|=	NFloat2UB(vp->vtx1.BaseG) << 8;
		vertex.col		|=	NFloat2UB(vp->vtx1.BaseB) << 16;
		break;

	case 2:
		tcol			= NFloat2UB(vp->vtx2.BaseInt);
		vertex.col		= tcol | tcol<<8 | tcol<<16 | tcol<<24 ;
		break;

		// kmflight, uses this and gouraud changes it !
	case 3:
		vertex.col		= RGBA(vp->vtx3.BaseCol);

		vertex.uv[0]	= vp->vtx3.u;
		vertex.uv[1]	= vp->vtx3.v;
		break;

	case 4:
		vertex.col		= RGBA(vp->vtx4.BaseCol);

		vertex.uv[0]	= f16(vp->vtx4.u);
		vertex.uv[1]	= f16(vp->vtx4.v);
		break;

	case 5:
		vertex.col		=	NFloat2UB(vp->vtx5.BaseA) << 24;
		vertex.col		|=	NFloat2UB(vp->vtx5.BaseR) << 0;
		vertex.col		|=	NFloat2UB(vp->vtx5.BaseG) << 8;
		vertex.col		|=	NFloat2UB(vp->vtx5.BaseB) << 16;

		vertex.uv[0]	= vp->vtx5.u;
		vertex.uv[1]	= vp->vtx5.v;
		break;

	case 6:
		vertex.col		=	NFloat2UB(vp->vtx6.BaseA) << 24;
		vertex.col		|=	NFloat2UB(vp->vtx6.BaseR) << 0;
		vertex.col		|=	NFloat2UB(vp->vtx6.BaseG) << 8;
		vertex.col		|=	NFloat2UB(vp->vtx6.BaseB) << 16;

		vertex.uv[0]	= f16(vp->vtx6.u);
		vertex.uv[1]	= f16(vp->vtx6.v);
		break;

	case 7:
		tcol			= NFloat2UB(vp->vtx7.BaseInt);
		vertex.col		= tcol | tcol<<8 | tcol<<16 | tcol<<24 ;

		vertex.uv[0]	= vp->vtx7.u;
		vertex.uv[1]	= vp->vtx7.v;
		break;

	case 8:
		tcol			= NFloat2UB(vp->vtx8.BaseInt);
		vertex.col		= tcol | tcol<<8 | tcol<<16 | tcol<<24 ;

		vertex.uv[0]	= f16(vp->vtx8.u);
		vertex.uv[1]	= f16(vp->vtx8.v);
		break;

	case 9:
		vertex.col		= RGBA(vp->vtx9.BaseCol0);
		break;

	case 10:
		tcol			= NFloat2UB(vp->vtx10.BaseInt0);
		vertex.col		= tcol | tcol<<8 | tcol<<16 | tcol<<24 ;
		break;

	case 11:
		vertex.col		= RGBA(vp->vtx11.BaseCol0);

		vertex.uv[0]	= vp->vtx11.u0;
		vertex.uv[1]	= vp->vtx11.v0;
		break;

	case 12:
		vertex.col		= RGBA(vp->vtx12.BaseCol0);

		vertex.uv[0]	= f16(vp->vtx12.u0);
		vertex.uv[1]	= f16(vp->vtx12.v0);
		break;

	case 13:
		tcol			= NFloat2UB(vp->vtx13.BaseInt0);
		vertex.col		= tcol | tcol<<8 | tcol<<16 | tcol<<24 ;

		vertex.uv[0]	= vp->vtx13.u0;
		vertex.uv[1]	= vp->vtx13.v0;
		break;

	case 14:
		tcol			= NFloat2UB(vp->vtx14.BaseInt0);
		vertex.col		= tcol | tcol<<8 | tcol<<16 | tcol<<24 ;

		vertex.uv[0]	= f16(vp->vtx14.u0);
		vertex.uv[1]	= f16(vp->vtx14.v0);
		break;

	default:
		ASSERT_T((1),"Vertex Type OUT OF RANGE!");
		break;
	}
	vertex.xyz[0] = vp->vtx0.xyz[0];
	vertex.xyz[1] = vp->vtx0.xyz[1];
	vertex.xyz[2] = vp->vtx0.xyz[2];

	// thought i got rid of most of these hacks, i guess d3d in the same plugin is a hack in itself
	if(R_D3D==pvrOpts.GfxApi)
		vertex.col = (vertex.col &0xFF00FF00) | ((vertex.col&255)<<16) | ((vertex.col>>16)&255);

	vertex.uv[2] = 0.f;
	vertex.uv[3] = vertex.xyz[2];
	vertex.uv[0] *= vertex.xyz[2];
	vertex.uv[1] *= vertex.xyz[2];

	// *FIXME* HACK - test, does this device before persp correction make text/menus screwd up?
	if(vertex.xyz[2] > 1.f)
		vertex.xyz[2] /= (vertex.xyz[2] > 512.f) ? 10000.f : 256.f;

	// Push it on list
	tmpVert.List.push_back(vertex);

#ifndef FULL_LIST
	if(vp->pcw.EndOfStrip)
	{
		tmpVert.TexID	= (u32)PvrIf->GetTexture(pp);
		tmpVert.ParamID	= (u32)(GlobalParams.size()-1);

		switch(pp->pcw.ListType)
		{
		case LT_Opaque:			OpaqueVerts.push_back(tmpVert);	break;
		case LT_OpaqueMod:		OpaqueMods.push_back(tmpVert);	break;
		case LT_Translucent:	TranspVerts.push_back(tmpVert);	break;
		case LT_TransMod:		TranspMods.push_back(tmpVert);	break;
		case LT_PunchThrough:	PunchtVerts.push_back(tmpVert);	break;
		case LT_Reserved:
			break;
		}
		LastType = 420;
		tmpVert.TexID =
		tmpVert.ParamID = 0;
		tmpVert.List.clear();
	}
#else
		retsize += VSize;

	} while(!vp->pcw.EndOfStrip);

	tmpVert.TexID	= (u32)PvrIf->GetTexture(pp);
	tmpVert.ParamID	= (u32)(GlobalParams.size()-1);

	switch(pp->pcw.ListType)
	{
		case LT_Opaque:			OpaqueVerts.push_back(tmpVert);	break;
		case LT_OpaqueMod:		OpaqueMods.push_back(tmpVert);	break;
		case LT_Translucent:	TranspVerts.push_back(tmpVert);	break;
		case LT_TransMod:		TranspMods.push_back(tmpVert);	break;
		case LT_PunchThrough:	PunchtVerts.push_back(tmpVert);	break;
		case LT_Reserved:
			break;
	}
	LastType = 420;
	tmpVert.TexID =
	tmpVert.ParamID = 0;
	tmpVert.List.clear();

	return retsize;	//VSize;
#endif
	return VSize;
}


/*	Todo, make sure on calcs, and create new spr and add directly to it, ie: get rid of tmpVert
**/
int PrimConverter::AppendSprite(VertexParam *vp)
{
	static Vertex tmpVert;
	PolyParam * pp = &GlobalParams[GlobalParams.size()-1];

	Vert vertex;
	vertex.xyz[0] = vp->spr1.x0;
	vertex.xyz[1] = vp->spr1.y0;
	vertex.xyz[2] = vp->spr1.z0 / 256.f;			// *FIXME*
	vertex.uv[2]  =	0.f;
	vertex.uv[3]  =	vertex.xyz[2];
	vertex.uv[0]  =	f16(vp->spr1.u0) * vertex.xyz[2];
	vertex.uv[1]  =	f16(vp->spr1.v0) * vertex.xyz[2];
	vertex.col	  = RGBA(pp->sprite.BaseCol);
	tmpVert.List.push_back(vertex);

	vertex.xyz[0] = vp->spr1.x1;
	vertex.xyz[1] = vp->spr1.y1;
	vertex.xyz[2] = vp->spr1.z1 / 256.f;			// *FIXME*
	vertex.uv[2]  =	0.f;
	vertex.uv[3]  =	vertex.xyz[2];
	vertex.uv[0]  =	f16(vp->spr1.u1) * vertex.xyz[2];
	vertex.uv[1]  =	f16(vp->spr1.v1) * vertex.xyz[2];
	vertex.col	  = RGBA(pp->sprite.BaseCol);
	tmpVert.List.push_back(vertex);

	vertex.xyz[0] = vp->spr1.x2;
	vertex.xyz[1] = vp->spr1.y2;
	vertex.xyz[2] = vp->spr1.z2 / 256.f;			// *FIXME*
	vertex.uv[2]  =	0.f;
	vertex.uv[3]  =	vertex.xyz[2];
	vertex.uv[0]  =	f16(vp->spr1.u2) * vertex.xyz[2];
	vertex.uv[1]  =	f16(vp->spr1.v2) * vertex.xyz[2];
	vertex.col	  = RGBA(pp->sprite.BaseCol);
	tmpVert.List.push_back(vertex);

	vertex.xyz[0] =  vp->spr1.x3;
	vertex.xyz[1] =  vp->spr1.y3;
	vertex.xyz[2] =  vp->spr1.z0 / 256.f;			// *FIXME* test, or (vp->spr1.z0 + vp->spr1.z0) / 2.f
	vertex.uv[2]  =	0.f;
	vertex.uv[3]  =	vertex.xyz[2];
	vertex.uv[0]  =	f16(vp->spr1.u0) * vertex.xyz[2];	// *FIXME* 
	vertex.uv[1]  =	f16(vp->spr1.v2) * vertex.xyz[2];	// *FIXME* how does it calc the UVs ?
	vertex.col	  = RGBA(pp->sprite.BaseCol);
	tmpVert.List.push_back(vertex);

	tmpVert.TexID	= (u32)PvrIf->GetTexture(pp);
	tmpVert.ParamID	= (u32)(GlobalParams.size()-1);
	Sprites.push_back(tmpVert);
	tmpVert.List.clear();
	return PS64;
}








	/*
	**	Debugging Misc Functions
	*/



void DumpFifo(u32 address, u32* data, u32 size)
{

	printf("\n\n\n Dumping Fifo Write to @%08X \n\n\n", address);

	static u32 n=0;

	FILE * f;
	int ds=0;
	char filename[512];

	sprintf_s(filename, "Fifo[%06X].txt", ++n);
	fopen_s(&f, filename, "wt");

	fprintf(f, "\nDumpFifo %08X - addr: %08X, size: 0x%X\n\n", n, address, size);

	for(u32 i=0; i<size; i++) {
		switch( ((ParamBase*)data)[i].Base.pcw.ParaType ) {
		case PT_EndOfList:		fprintf(f,"[%06X] PT_EndOfList     \n",i);	ds=1;	break;	// *FIXME* ds
		case PT_Polygon:		fprintf(f,"[%06X] PT_Polygon       \n",i);	ds=1;	break;
		case PT_Sprite:			fprintf(f,"[%06X] PT_Sprite        \n",i);	ds=1;	break;
		case PT_Vertex:			fprintf(f,"[%06X] PT_Vertex        \n",i);	ds=1;	break;
		case PT_UserTileClip:	fprintf(f,"[%06X] PT_UserTileClip  \n",i);	ds=1;	break;
		case PT_ObjectListSet:	fprintf(f,"[%06X] PT_ObjectListSet \n",i);	ds=1;	break;
		case PT_Reserved3:		fprintf(f,"[%06X] PT_Reserved3     \n",i);	ds=1;	break;
		case PT_Reserved6:		fprintf(f,"[%06X] PT_Reserved6     \n",i);	ds=1;	break;
		}
		for(int z=0; z<ds; z++) {
			fprintf(f,"%08X %08X %08X %08X\n%08X %08X %08X %08X\n\n",
				((u32*)((ParamBase*)data)[i].Full)[0], ((u32*)((ParamBase*)data)[i].Full)[1],
				((u32*)((ParamBase*)data)[i].Full)[2], ((u32*)((ParamBase*)data)[i].Full)[3],
				((u32*)((ParamBase*)data)[i].Full)[4], ((u32*)((ParamBase*)data)[i].Full)[5],
				((u32*)((ParamBase*)data)[i].Full)[6], ((u32*)((ParamBase*)data)[i].Full)[7]);
		}
	}
	fclose(f);
}






