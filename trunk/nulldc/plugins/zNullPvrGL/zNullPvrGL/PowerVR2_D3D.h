/*
**	PowerVR2_D3D.h - David Miller 2006 - PowerVR2 Emulation Library
*/

#ifndef __POWERVR2_D3D_H__
#define __POWERVR2_D3D_H__

#include <windows.h>
#include <d3dx9.h>
#include <d3d9.h>



extern float fMaxW;
extern D3DCAPS9 d3dcaps;

extern IDirect3D9 *g_pD3D;					// D3D Object Pointer
extern IDirect3DDevice9 *g_pDev;			// D3D Device
extern IDirect3DVertexBuffer9 *g_pVB;		// Vertex Buffer
extern IDirect3DVertexDeclaration9 *g_pVD;	// Vertex Decl.


#include "zNullPvr.h"

#include "TA_Regs.h"
#include "TA_Param.h"
#include "TA_Texture.h"

extern 
class PowerVR2_D3D : public PowerVR2
{

	bool Init();
	void Term();
	void Render();
	void Resize();

private:

	void RenderSprites(vector<Vertex> &vl);
	void RenderStripList(vector<Vertex> &vl);
//	void RenderStripListRev(vector<Vertex> &vl);
//	void RenderStripListArray(vector<Vertex> &vl);

	void SetRenderMode(u32 ParamID, u32 TexID);
	void SetRenderModeSpr(u32 ParamID, u32 TexID);

} PvrIfD3D;


const static D3DVERTEXELEMENT9 vertel[] =
{
	{ 0,  0, D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },	// f32 xyz[3];
	{ 0, 12, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,	   0 },	// u32 col;
	{ 0, 16, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },	// f32 uv[4];
	D3DDECL_END()
};



#define D3DFVF_VERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE4(0))	//D3DFVF_TEXTUREFORMAT4  is wtf??

const u32 DepthModeDX[] = 
{
    D3DCMP_NEVER,
    D3DCMP_LESS,
    D3DCMP_EQUAL,
    D3DCMP_LESSEQUAL,
    D3DCMP_GREATER,
    D3DCMP_NOTEQUAL,
    D3DCMP_GREATEREQUAL,
    D3DCMP_ALWAYS   
};

const u32 SrcBlendDX[] =
{
    D3DBLEND_ZERO,
    D3DBLEND_ONE,
	D3DBLEND_DESTCOLOR,
	D3DBLEND_INVDESTCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVDESTALPHA
};

const u32 DstBlendDX[] =
{
    D3DBLEND_ZERO,
    D3DBLEND_ONE,
	D3DBLEND_SRCCOLOR,
	D3DBLEND_INVSRCCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVDESTALPHA
};


#endif //__POWERVR2_D3D_H__

