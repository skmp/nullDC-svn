/*
**	nullRendD3D.cpp,	nullDC::nullPVR	(2007) ZeZu & drk||Raziel
*/

#include "nullPvr.h"
#include "nullRend.h"
#include "pvrMemory.h"
#include "ta_vdec.h"


float fMaxW = 1.f;
D3DCAPS9 d3dcaps;

IDirect3D9 *g_pD3D;					// D3D Object Pointer
IDirect3DDevice9 *g_pDev;			// D3D Device
IDirect3DVertexBuffer9 *g_pVB;		// Vertex Buffer
IDirect3DVertexDeclaration9 *g_pVD;	// Vertex Decl.




#define MAX_VB_SIZE 100000		// *FIXME* shouldn't need this shit now, or ..


s32  FASTCALL InitD3D(void * handle)
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
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND)handle,
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

	if(D3D_OK != g_pDev->CreateVertexDeclaration(vertel, &g_pVD))
		goto fail;

	if(D3D_OK != g_pDev->GetDeviceCaps(&d3dcaps))
		goto fail;

	fMaxW = d3dcaps.MaxVertexW;
	printf("MaxVertexW: %f\n", fMaxW );

	return true;

fail:
	TermD3D(handle);
	printf("D3D9 Failed to Initialize !\n");
	return false;
	return true;
}

void FASTCALL TermD3D(void * handle)
{
//	ClearDCache();
//	TCache.ClearTCache();		// Textures

	if(g_pDev != NULL)	g_pDev->Release();
	if(g_pD3D != NULL)	g_pD3D->Release();
	if(g_pVB  != NULL)	g_pVB->Release();
}

static RECT rClient;
void FASTCALL ResizeD3D(void * handle)
{
	GetClientRect((HWND)handle,&rClient);

}

void FASTCALL RenderD3D(void * buffer)
{
	u32 dwValue = *pVO_BORDER_COL;
	u32	R=((dwValue>>0x10)&0xFF),
		G=((dwValue>>0x08)&0xFF),
		B=((dwValue>>0x00)&0xFF);

	g_pDev->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(R,G,B), 0.f, 0);


	g_pDev->BeginScene();
	g_pDev->SetVertexShader(NULL);
	g_pDev->SetVertexDeclaration(g_pVD);
	//	g_pDev->SetStreamSource(0, g_pVB, 0, sizeof(Vertex));







	g_pDev->EndScene();
	g_pDev->Present(NULL,NULL,NULL,NULL);

//	ClearDCache();
//	TCache.ClearTInvalids();
}
void FASTCALL SetStateD3D(void * state)
{
	g_pDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pDev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);


//	g_pDev->SetRenderState(D3DRS_ZFUNC, DepthModeDX[D3D_GEQUAL]);
	g_pDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	g_pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

