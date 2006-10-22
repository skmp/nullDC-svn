/*
**	This file must be included in TA_Param.cpp after TA_PARAM_CPP is defined,
**	and after TA_Param.h is included !
*/
#ifndef TA_PARAM_CPP
#error	"VertDecode.h Must be Included in TA_Param.cpp only after TA_PARAM_CPP Is Defined!"
#endif


// A few helper macros 

u32 LastInt = ~0;

S_INLINE float f16(u32 x)
{
	u32 t = x<<16;			// x<<=16
	f32 f = *(float*)(&t);	// return *(float*)(&x);
	return f;
}

S_INLINE u8 NFloat2UB(float NCF)
{
#ifdef DEBUG_LIB
	return (u8)(((NCF > 1.f) ? 1.f : ((NCF < 0.f) ? 0.f : NCF)) * 255.f);
#else
	return (u8)(((NCF > 1.f) ? 1.f:NCF) * 255.f);
#endif
}




S_INLINE void DecodeStrip0(Vert *pVert, VertexParam *vp)
{
	pVert->xyz[0]	= vp->vtx0.xyz[0];
	pVert->xyz[1]	= vp->vtx0.xyz[1];
	pVert->xyz[2]	= (vp->vtx0.xyz[2]>1.f) ? vp->vtx0.xyz[2]/256.f : vp->vtx0.xyz[2];

	memset(pVert->uv, 0, sizeof(float)*4);

	pVert->col		= vp->vtx0.BaseCol;
}


S_INLINE void DecodeStrip1(Vert *pVert, VertexParam *vp)
{
	pVert->xyz[0]	= vp->vtx1.xyz[0];
	pVert->xyz[1]	= vp->vtx1.xyz[1];
	pVert->xyz[2]	= (vp->vtx1.xyz[2]>1.f) ? vp->vtx1.xyz[2]/256.f : vp->vtx1.xyz[2];

	memset(pVert->uv, 0, sizeof(float)*4);

	pVert->col		=	NFloat2UB(vp->vtx1.BaseA) << 24;
	pVert->col		|=	NFloat2UB(vp->vtx1.BaseR) << 0;
	pVert->col		|=	NFloat2UB(vp->vtx1.BaseG) << 8;
	pVert->col		|=	NFloat2UB(vp->vtx1.BaseB) << 16;
}

S_INLINE void DecodeStrip2(Vert *pVert, VertexParam *vp)	// Intensity
{
	lprintf("DecodeStrip2() Intensity Col-Mode : %d\n", vp->pcw.Col_Type);

	pVert->xyz[0]	= vp->vtx2.xyz[0];
	pVert->xyz[1]	= vp->vtx2.xyz[1];
	pVert->xyz[2]	= (vp->vtx2.xyz[2]>1.f) ? vp->vtx2.xyz[2]/256.f : vp->vtx2.xyz[2];

	memset(pVert->uv, 0, sizeof(float)*4);

	u8 tcol			= NFloat2UB(vp->vtx2.BaseInt);
	pVert->col		= LastInt = tcol | (tcol<<8) | (tcol<<16) | (tcol<<24) ;
}

S_INLINE void DecodeStrip3(Vert *pVert, VertexParam *vp)
{
	pVert->xyz[0]	= vp->vtx3.xyz[0];
	pVert->xyz[1]	= vp->vtx3.xyz[1];
	pVert->xyz[2]	= (vp->vtx3.xyz[2]>1.f) ? vp->vtx3.xyz[2]/256.f : vp->vtx3.xyz[2];

	pVert->uv[2]	= ((float)0.f);
	pVert->uv[3]	= pVert->xyz[2];
	pVert->uv[0]	= vp->vtx3.u * pVert->xyz[2];
	pVert->uv[1]	= vp->vtx3.v * pVert->xyz[2];

	pVert->col		= vp->vtx3.BaseCol;
}

S_INLINE void DecodeStrip4(Vert *pVert, VertexParam *vp)
{
	pVert->xyz[0]	= vp->vtx4.xyz[0];
	pVert->xyz[1]	= vp->vtx4.xyz[1];
	pVert->xyz[2]	= (vp->vtx4.xyz[2]>1.f) ? vp->vtx4.xyz[2]/256.f : vp->vtx4.xyz[2];

	pVert->uv[2]	= ((float)0.f);
	pVert->uv[3]	= pVert->xyz[2];
	pVert->uv[0]	= f16(vp->vtx4.u) * pVert->xyz[2];
	pVert->uv[1]	= f16(vp->vtx4.v) * pVert->xyz[2];

	pVert->col		= vp->vtx4.BaseCol;
}

S_INLINE void DecodeStrip5(Vert *pVert, VertexParam *vp)
{
	pVert->xyz[0]	= vp->vtx5.xyz[0];
	pVert->xyz[1]	= vp->vtx5.xyz[1];
	pVert->xyz[2]	= (vp->vtx5.xyz[2]>1.f) ? vp->vtx5.xyz[2]/256.f : vp->vtx5.xyz[2];

	pVert->uv[2]	= ((float)0.f);
	pVert->uv[3]	= pVert->xyz[2];
	pVert->uv[0]	= vp->vtx5.u * pVert->xyz[2];
	pVert->uv[1]	= vp->vtx5.v * pVert->xyz[2];

	pVert->col		=	NFloat2UB(vp->vtx5.BaseA) << 24;
	pVert->col		|=	NFloat2UB(vp->vtx5.BaseR) << 0;
	pVert->col		|=	NFloat2UB(vp->vtx5.BaseG) << 8;
	pVert->col		|=	NFloat2UB(vp->vtx5.BaseB) << 16;
}

S_INLINE void DecodeStrip6(Vert *pVert, VertexParam *vp)
{
	pVert->xyz[0]	= vp->vtx6.xyz[0];
	pVert->xyz[1]	= vp->vtx6.xyz[1];
	pVert->xyz[2]	= (vp->vtx6.xyz[2]>1.f) ? vp->vtx6.xyz[2]/256.f : vp->vtx6.xyz[2];

	pVert->uv[2]	= ((float)0.f);
	pVert->uv[3]	= pVert->xyz[2];
	pVert->uv[0]	= f16(vp->vtx6.u) * pVert->xyz[2];
	pVert->uv[1]	= f16(vp->vtx6.v) * pVert->xyz[2];

	pVert->col		=	NFloat2UB(vp->vtx6.BaseA) << 24;
	pVert->col		|=	NFloat2UB(vp->vtx6.BaseR) << 0;
	pVert->col		|=	NFloat2UB(vp->vtx6.BaseG) << 8;
	pVert->col		|=	NFloat2UB(vp->vtx6.BaseB) << 16;
}

S_INLINE void DecodeStrip7(Vert *pVert, VertexParam *vp)	// Intensity
{
	lprintf("DecodeStrip7() Intensity Col-Mode : %d\n", vp->pcw.Col_Type);

	pVert->xyz[0]	= vp->vtx7.xyz[0];
	pVert->xyz[1]	= vp->vtx7.xyz[1];
	pVert->xyz[2]	= (vp->vtx7.xyz[2]>1.f) ? vp->vtx7.xyz[2]/256.f : vp->vtx7.xyz[2];

	pVert->uv[2]	= ((float)0.f);
	pVert->uv[3]	= pVert->xyz[2];
	pVert->uv[0]	= vp->vtx7.u * pVert->xyz[2];
	pVert->uv[1]	= vp->vtx7.v * pVert->xyz[2];

	u8 tcol			= NFloat2UB(vp->vtx7.BaseInt);
	pVert->col		= tcol | tcol<<8 | tcol<<16 | tcol<<24 ;
}

S_INLINE void DecodeStrip8(Vert *pVert, VertexParam *vp)	// Intensity
{
	lprintf("DecodeStrip8() Intensity Col-Mode : %d\n", vp->pcw.Col_Type);

	pVert->xyz[0]	= vp->vtx8.xyz[0];
	pVert->xyz[1]	= vp->vtx8.xyz[1];
	pVert->xyz[2]	= (vp->vtx8.xyz[2]>1.f) ? vp->vtx8.xyz[2]/256.f : vp->vtx8.xyz[2];

	pVert->uv[2]	= ((float)0.f);
	pVert->uv[3]	= pVert->xyz[2];
	pVert->uv[0]	= f16(vp->vtx8.u) * pVert->xyz[2];
	pVert->uv[1]	= f16(vp->vtx8.v) * pVert->xyz[2];

	u8 tcol			= NFloat2UB(vp->vtx8.BaseInt);
	pVert->col		= tcol | tcol<<8 | tcol<<16 | tcol<<24 ;
}

S_INLINE void DecodeStrip9(Vert *pVert, VertexParam *vp)
{
	pVert->xyz[0]	= vp->vtx9.xyz[0];
	pVert->xyz[1]	= vp->vtx9.xyz[1];
	pVert->xyz[2]	= (vp->vtx9.xyz[2]>1.f) ? vp->vtx9.xyz[2]/256.f : vp->vtx9.xyz[2];

	memset(pVert->uv, 0, sizeof(float)*4);

	pVert->col		= vp->vtx9.BaseCol0;
}

S_INLINE void DecodeStripA(Vert *pVert, VertexParam *vp)	// Intensity
{
	lprintf("DecodeStripA() Intensity Col-Mode : %d\n", vp->pcw.Col_Type);

	pVert->xyz[0]	= vp->vtx10.xyz[0];
	pVert->xyz[1]	= vp->vtx10.xyz[1];
	pVert->xyz[2]	= (vp->vtx10.xyz[2]>1.f) ? vp->vtx10.xyz[2]/256.f : vp->vtx10.xyz[2];

	memset(pVert->uv, 0, sizeof(float)*4);

	u8 tcol			= NFloat2UB(vp->vtx10.BaseInt0);
	pVert->col		= tcol | tcol<<8 | tcol<<16 | tcol<<24 ;
}

S_INLINE void DecodeStripB(Vert *pVert, VertexParam *vp)
{
	pVert->xyz[0]	= vp->vtx11.xyz[0];
	pVert->xyz[1]	= vp->vtx11.xyz[1];
	pVert->xyz[2]	= (vp->vtx11.xyz[2]>1.f) ? vp->vtx11.xyz[2]/256.f : vp->vtx11.xyz[2];

	pVert->uv[2]	= ((float)0.f);
	pVert->uv[3]	= pVert->xyz[2];
	pVert->uv[0]	= vp->vtx11.u0 * pVert->xyz[2];
	pVert->uv[1]	= vp->vtx11.v0 * pVert->xyz[2];

	pVert->col		= vp->vtx11.BaseCol0;
}

S_INLINE void DecodeStripC(Vert *pVert, VertexParam *vp)
{
	pVert->xyz[0]	= vp->vtx12.xyz[0];
	pVert->xyz[1]	= vp->vtx12.xyz[1];
	pVert->xyz[2]	= (vp->vtx12.xyz[2]>1.f) ? vp->vtx12.xyz[2]/256.f : vp->vtx12.xyz[2];

	memset(pVert->uv, 0, sizeof(float)*4);

	pVert->col		= vp->vtx12.BaseCol0;
}

S_INLINE void DecodeStripD(Vert *pVert, VertexParam *vp)	// Intensity
{
	lprintf("DecodeStripD() Intensity Col-Mode : %d\n", vp->pcw.Col_Type);

	pVert->xyz[0]	= vp->vtx13.xyz[0];
	pVert->xyz[1]	= vp->vtx13.xyz[1];
	pVert->xyz[2]	= (vp->vtx13.xyz[2]>1.f) ? vp->vtx13.xyz[2]/256.f : vp->vtx13.xyz[2];

	pVert->uv[2]	= ((float)0.f);
	pVert->uv[3]	= pVert->xyz[2];
	pVert->uv[0]	= vp->vtx13.u0 * pVert->xyz[2];
	pVert->uv[1]	= vp->vtx13.v0 * pVert->xyz[2];

	u8 tcol			= NFloat2UB(vp->vtx13.BaseInt0);
	pVert->col		= tcol | tcol<<8 | tcol<<16 | tcol<<24 ;
}

S_INLINE void DecodeStripE(Vert *pVert, VertexParam *vp)	// Intensity
{
	lprintf("DecodeStripE() Intensity Col-Mode : %d\n", vp->pcw.Col_Type);

	pVert->xyz[0]	= vp->vtx14.xyz[0];
	pVert->xyz[1]	= vp->vtx14.xyz[1];
	pVert->xyz[2]	= (vp->vtx14.xyz[2]>1.f) ? vp->vtx14.xyz[2]/256.f : vp->vtx14.xyz[2];

	pVert->uv[2]	= ((float)0.f);
	pVert->uv[3]	= pVert->xyz[2];
	pVert->uv[0]	= f16(vp->vtx14.u0) * pVert->xyz[2];
	pVert->uv[1]	= f16(vp->vtx14.v0) * pVert->xyz[2];

	u8 tcol			= NFloat2UB(vp->vtx14.BaseInt0);
	pVert->col		= tcol | tcol<<8 | tcol<<16 | tcol<<24 ;
}















