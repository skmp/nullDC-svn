/*
**	PowerVR2_D3D.cpp	- David Miller 2006 -
*/
#include "PowerVR2.h"
#include "TA_Param.h"

using namespace PvrIf;






float fMaxW = 1.f;
D3DCAPS9 d3dcaps;

IDirect3D9 *g_pD3D;					// D3D Object Pointer
IDirect3DDevice9 *g_pDev;			// D3D Device
IDirect3DVertexBuffer9 *g_pVB;		// Vertex Buffer
IDirect3DVertexDeclaration9 *g_pVD;	// Vertex Decl.






S_INLINE void SetRenderMode(u32 ParamID, u32 TexID)
{
	GlobalParam * gp = &GlobalParams[ParamID];

    g_pDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pDev->SetRenderState(D3DRS_SHADEMODE, gp->pcw.Gouraud ? D3DSHADE_GOURAUD : D3DSHADE_FLAT);
//	g_pDev->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);

	g_pDev->SetRenderState(D3DRS_ZWRITEENABLE, !gp->isp.ZWriteDis);
	g_pDev->SetRenderState(D3DRS_ZFUNC, DepthModeDX[gp->isp.DepthMode]);

	switch( gp->pcw.ListType )
	{
	case LT_Opaque:
		g_pDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		g_pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		break;

	case LT_Translucent:
	//	g_pDev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		g_pDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		
		if(!gp->param0.tsp.IgnoreTexA)	// in trs type ? alpharef ?
		{
			g_pDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			g_pDev->SetRenderState(D3DRS_ALPHAREF, 0x00);
			g_pDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	
		} else { g_pDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE); }

		if(!gp->param0.tsp.UseAlpha)
			g_pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		else {
			g_pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			g_pDev->SetRenderState(D3DRS_SRCBLEND,  SrcBlendDX[gp->param0.tsp.SrcInstr]);
			g_pDev->SetRenderState(D3DRS_DESTBLEND, DstBlendDX[gp->param0.tsp.DstInstr]);
		}
		break;

	case LT_PunchThrough:
		g_pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		g_pDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		g_pDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		g_pDev->SetRenderState(D3DRS_ALPHAREF, (*pPT_ALPHA_REF &0xFF));
		break;

	case LT_OpaqueMod:return;		// don't care yet
	case LT_TransMod: return;		// don't care yet
	case LT_Reserved: ASSERT_T((1),"BOGUS LIST TYPE IN SetRenderMode()");	return;
	}

	if(gp->isp.Texture || gp->pcw.Texture)
	{
		switch(gp->param0.tsp.ShadInstr)	// these should be correct, except offset
		{
		case 0:	// Decal
			g_pDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			break;
		case 1:	// Modulate
			g_pDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			break;
		case 2:	// Decal Alpha
			g_pDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_BLENDTEXTUREALPHA);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
			break;
		case 3:	// Modulate Alpha
			g_pDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			break;
		default: ASSERT_T((1),"SetRenderMode->TSP.ShadInstr is INVALID !!"); return;
		}

		switch(gp->param0.tsp.FilterMode)
		{
		case 0:	// point sampled
			g_pDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			g_pDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			break;
		case 1:	// bi-linear
			g_pDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			g_pDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			break;
		case 2:	// pass a
		case 3:	// pass b
		//	g_pDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
			g_pDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			g_pDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			break;
		default: ASSERT_T((1),"Unknown Tex Filter Type in SetRenderMode() !!"); break;
		}

	//	D3DTTFF_DISABLE, D3DTTFF_COUNT1|2|3|4  D3DTTFF_PROJECTED 
		g_pDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS,
										D3DTTFF_COUNT4 | D3DTTFF_PROJECTED);
	}
/*
	// test 
	g_pDev->SetRenderState(D3DRS_FOGENABLE, TRUE);
	g_pDev->SetRenderState(D3DRS_FOGCOLOR, D3DCOLOR_XRGB(0xF0,0x0C, 0xC0)); 
	
	float Density = 0.8f;
	g_pDev->SetRenderState(D3DRS_FOGVERTEXMODE, Mode);
    g_pDev->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)(&Density));
*/
}


S_INLINE void SetRenderModeSpr(u32 ParamID, u32 TexID)
{
	GlobalParam * gp = &GlobalParams[ParamID];

    g_pDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pDev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

	//g_pDev->SetRenderState(D3DRS_ZWRITEENABLE, !gp->isp.ZWriteDis);
	g_pDev->SetRenderState(D3DRS_ZFUNC, DepthModeDX[gp->isp.DepthMode]);


	if(LT_Opaque == gp->pcw.ListType)
	{
		g_pDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		g_pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	} else {
		g_pDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		g_pDev->SetRenderState(D3DRS_ALPHAREF, 0x00);
		g_pDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	}

	if(gp->isp.Texture || gp->pcw.Texture)
	{
		switch(gp->param0.tsp.ShadInstr)	// these should be correct, except offset
		{
		case 0:	// Decal
			g_pDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			break;
		case 1:	// Modulate
			g_pDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			break;
		case 2:	// Decal Alpha
			g_pDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_BLENDTEXTUREALPHA);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
			break;
		case 3:	// Modulate Alpha
			g_pDev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			g_pDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			break;
		default: ASSERT_T((1),"SetRenderMode->TSP.ShadInstr is INVALID !!"); return;
		}

		switch(gp->param0.tsp.FilterMode)
		{
		case 0:	// point sampled
			g_pDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			g_pDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			break;
		case 1:	// bi-linear
			g_pDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			g_pDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			break;
		case 2:	// pass a
		case 3:	// pass b
		//	g_pDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
			g_pDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			g_pDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			break;
		default: ASSERT_T((1),"Unknown Tex Filter Type in SetRenderMode() !!"); break;
		}

	//	D3DTTFF_DISABLE, D3DTTFF_COUNT1|2|3|4  D3DTTFF_PROJECTED 
		g_pDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS,
										D3DTTFF_COUNT4 | D3DTTFF_PROJECTED);
	}
}


S_INLINE void RenderStripList(TA_ListType lType)
{
	DCache * pDList = NULL;

	switch(lType)	{
	case LT_Opaque:			pDList = &Opaque;		break;
	case LT_Translucent:	pDList = &Transparent;	break;
	case LT_PunchThrough:	pDList = &PunchThru;	break;
	default: ASSERT_T((1),"RenderStripList() LType Default");	return;
	}

	int cpos=0;
	for(u32 p=0; p<pDList->StripCount; p++)
	{
		int len = pDList->StripList[p].len;

		ASSERT_T((len <= 2), "Render of Strip <= len 2");
		//lprintf("\nVList: %X : size: %d\n", p, vp->Size);

		Vert * vp = &((Vert*)pDList->buff)[cpos];//+v];
		SetRenderMode(pDList->StripList[p].ParamID, pDList->StripList[p].TexID);
		g_pDev->SetTexture(0, (IDirect3DTexture9*)pDList->StripList[p].TexID);
		g_pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, len-2, &vp->xyz, sizeof(Vert));

		cpos += len;
	}
}

/*S_INLINE void RenderStripList(TA_ListType lType)
{
	int nStrips = 0;
	Vertex * pVList = NULL;

	switch(lType)	{
	case LT_Opaque:			nStrips=nOpqStrips;	pVList=(Vertex*)opq;	break;
	case LT_Translucent:	nStrips=nTrsStrips;	pVList=(Vertex*)trs;	break;
	case LT_PunchThrough:	nStrips=nPtuStrips;	pVList=(Vertex*)ptu;	break;
	}

	for(u32 p=0; p<nStrips; p++)
	{
		Vertex * vp = &pVList[p];
		SetRenderMode(vp->ParamID, vp->TexID);
		g_pDev->SetTexture(0, (IDirect3DTexture9*)vp->TexID);
		g_pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, vp->Size-2, &vp->List[0].xyz, sizeof(Vert));
	}
}*/

S_INLINE void RenderSprites(vector<Vertex> &vl)
{
/*	for(u32 p=0; p<vl.size(); p++) {
		SetRenderModeSpr(vl[p].ParamID, vl[p].TexID);
		g_pDev->SetTexture(0, (IDirect3DTexture9*)vl[p].TexID);
		g_pDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, vl[p].List.size()-2, &vl[p].List[0].xyz, sizeof(Vert));
	}*/
}


void RenderD3D()
{
//	FrameCount++;
	lprintf("Render() (D3D)\n");

	u32 dwValue = *pVO_BORDER_COL;
	u32	R=((dwValue>>0x10)&0xFF),
		G=((dwValue>>0x08)&0xFF),
		B=((dwValue>>0x00)&0xFF);

    g_pDev->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(R,G,B), 0.f, 0);

	g_pDev->BeginScene();
	g_pDev->SetVertexShader(NULL);
	g_pDev->SetVertexDeclaration(g_pVD);
//	g_pDev->SetStreamSource(0, g_pVB, 0, sizeof(Vertex));

	RenderStripList(LT_Opaque);
	RenderStripList(LT_Translucent);
	RenderStripList(LT_PunchThrough);
//	RenderSprites(Sprites);

	g_pDev->EndScene();
	g_pDev->Present(NULL,NULL,NULL,NULL);

	ClearDCache();
	TCache.ClearTInvalids();
}




void ResizeD3D()
{
}



#define MAX_VB_SIZE 100000		// *FIXME* shouldn't need this shit now, or ..

bool InitD3D()
{
	if(FAILED(g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		goto fail;

	D3DDISPLAYMODE d3ddm;
	if(FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
		goto fail;
	
	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed   = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;	// D3DSWAPEFFECT_COPY_VSYNC;
	d3dpp.BackBufferFormat = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	
	if( FAILED(g_pD3D->CreateDevice(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND)emuIf.handle,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE,	// *FIXME* fucking use hardware 
		&d3dpp, &g_pDev ) ) )
		goto fail;


	if( FAILED(g_pDev->CreateVertexBuffer(MAX_VB_SIZE*sizeof(Vertex),
		D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_VERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL)))
	   goto fail;
	else
	{
		D3DXMATRIX mat;

		D3DXMatrixOrthoOffCenterRH(&mat, 0,640, 480,0, 1.f, -1.f);
		g_pDev->SetTransform(D3DTS_PROJECTION, &(D3DMATRIX)mat);

		D3DXMatrixIdentity(&mat);
		g_pDev->SetTransform(D3DTS_WORLD, &(D3DMATRIX)mat);

		D3DXMatrixTranslation(&mat, 0, 0, 0);
		g_pDev->SetTransform(D3DTS_VIEW, &(D3DMATRIX)mat);

		g_pDev->SetRenderState(D3DRS_LIGHTING, FALSE);
		g_pDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE); // D3DZB_USEW
	}

	if(g_pDev->CreateVertexDeclaration(vertel, &g_pVD) != D3D_OK)
		goto fail;

	if( g_pDev->GetDeviceCaps( &d3dcaps ) != D3D_OK )
		goto fail;

	fMaxW = d3dcaps.MaxVertexW;
	printf("MaxVertexW: %f\n", fMaxW );

	return true;

fail:
	Term();
	printf("D3D9 Failed to Initialize !\n");
	return false;
}

void TermD3D()
{
    if(g_pDev != NULL)	g_pDev->Release();
	if(g_pD3D != NULL)	g_pD3D->Release();
	if(g_pVB  != NULL)	g_pVB->Release();

	ClearDCache();
	TCache.ClearTCache();		// Textures
}



