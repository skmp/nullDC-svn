// ----------------------------------------------------------------------------------------
// Nombre       : DisplayDevice.cpp
// Descripcion  : 
// ----------------------------------------------------------------------------------------

//#include "stdafx.h"





#define ATI_ID 0x1002

// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::Init()
// Descripcion  : 
// Retorno      : TError 
// Parametros   : DWORD uWidth
//              : DWORD uHeight
// --------------------------------------------------------------------------------
TError CDisplayDevice::Init(DWORD uWidth, DWORD uHeight, void* hWnd, BOOL bFullScreen)
{
  End();

  HRESULT hr;

  m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

  if (m_pD3D == NULL)
    return RET_FAIL;

  DWORD dwBehavior;
  dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
  D3DADAPTER_IDENTIFIER9 aIdentifier[2];
	hr = m_pD3D->GetAdapterIdentifier(0,0,&aIdentifier[0]);
  if (!FAILED(hr))
	{
    if (aIdentifier[0].VendorId == ATI_ID)
      dwBehavior =  D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

  m_bDisplayChange = false;
  m_bFullScreen = bFullScreen;
  m_dwWidth     = uWidth;
  m_dwHeight    = uHeight;
  m_hWnd        = hWnd;

  ZeroMemory( &m_d3dpp, sizeof(m_d3dpp) );
  m_d3dpp.hDeviceWindow = (HWND)m_hWnd;
  m_d3dpp.Windowed = TRUE;
  m_d3dpp.BackBufferCount = 0;
  m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  m_d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
  m_d3dpp.EnableAutoDepthStencil = TRUE;
  m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
  m_d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
  m_d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
  m_d3dpp.BackBufferWidth  = uWidth;
  m_d3dpp.BackBufferHeight = uHeight;
  m_d3dpp.FullScreen_RefreshRateInHz = 0;
  hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND) m_hWnd,  dwBehavior, &m_d3dpp, &m_pd3dDevice);

  if (FAILED(hr))
  {
    dwBehavior =  D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND) hWnd,  dwBehavior, &m_d3dpp, &m_pd3dDevice);
  }

  if (FAILED(hr))
  {
    m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND) hWnd,  dwBehavior, &m_d3dpp, &m_pd3dDevice);
  }

  if (FAILED(hr))
  {     
    m_d3dpp.BackBufferFormat = D3DFMT_R5G6B5;
    hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND) hWnd,  dwBehavior, &m_d3dpp, &m_pd3dDevice);
  }

  if (FAILED(hr))
  {     
    SAFE_RELEASE(m_pD3D);
    return RET_FAIL;
  }

  m_pFont = NEW(CD3DFont("Arial", 8));
  m_pFont->InitDeviceObjects(m_pd3dDevice);

  m_bInit          = true;
  if(RET_FAIL == Reset(m_dwWidth, m_dwHeight, m_bFullScreen))
    End();

  return m_bInit?RET_OK:RET_FAIL;
 }



// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::End()
// Descripcion  : 
// Retorno      : void 
// --------------------------------------------------------------------------------
void CDisplayDevice::End()
{
  if (m_bInit)
  {
    DISPOSE(m_pFont);

    //DeleteDeviceObjects();

    //SAFE_RELEASE(m_pTextureBlt);
    //int iNumRef = m_pd3dDevice->Release();
    SAFE_RELEASE(m_pd3dDevice);
    SAFE_RELEASE(m_pD3D);		
    m_bInit = false;
  }
}

// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::Reset()
// Descripcion  : 
// Retorno      : TError
// --------------------------------------------------------------------------------
TError        CDisplayDevice::Reset       (DWORD uWidth, DWORD uHeight, BOOL bFullScreen)
{
  m_bDisplayChange = true;
  m_uFrames = m_uFramesLatencia = m_uPixelsDibujados = 0;

  m_pFont->InvalidateDeviceObjects();
  HRESULT hr;
  m_d3dpp.BackBufferCount  = 0;
  m_d3dpp.Windowed         = !bFullScreen;
  m_d3dpp.BackBufferWidth  = uWidth;
  m_d3dpp.BackBufferHeight = uHeight;
  hr = m_pd3dDevice->Reset(&m_d3dpp);
  if(FAILED(hr))
  {
    m_d3dpp.Windowed = true;
    hr = m_pd3dDevice->Reset(&m_d3dpp);
  }
  if(FAILED(hr))
  {
    m_d3dpp.Windowed = !bFullScreen;
    m_d3dpp.BackBufferWidth  = m_dwWidth;
    m_d3dpp.BackBufferHeight = m_dwHeight;
    hr = m_pd3dDevice->Reset(&m_d3dpp);
  }
  if(FAILED(hr))
  {
    m_d3dpp.Windowed = true;
    m_d3dpp.BackBufferWidth  = uWidth;
    m_d3dpp.BackBufferHeight = uHeight;
    hr = m_pd3dDevice->Reset(&m_d3dpp);
  }

  if(FAILED(hr))
  {
    m_d3dpp.Windowed = !m_bFullScreen;
    m_d3dpp.BackBufferWidth  = m_dwWidth;
    m_d3dpp.BackBufferHeight = m_dwHeight;
    hr = m_pd3dDevice->Reset(&m_d3dpp);
  }

  if(FAILED(hr))
    return RET_FAIL;

  D3DCAPS9 caps;
  hr = m_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
  if(FAILED(hr))
    return RET_FAIL;

  m_pFont->RestoreDeviceObjects();

  m_fMaxW = caps.MaxVertexW;
  m_dwFlags = 0;
  if (caps.StencilCaps & D3DSTENCILCAPS_TWOSIDED)
    m_dwFlags |= F_TWO_SIDED_STENCIL;

  if (caps.RasterCaps & D3DPRASTERCAPS_SCISSORTEST)
    m_dwFlags |= F_SCISSOR_TEST;

  if (caps.RasterCaps & D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS)
    m_dwFlags |= F_SLOPE_DEPTH_BIAS;


  D3DVIEWPORT9 vp;
  m_pd3dDevice->GetViewport(&vp);
  m_dwWidth     = vp.Width;
  m_dwHeight    = vp.Height;
  m_bFullScreen = !m_d3dpp.Windowed;

  m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

  for(int i=0;i<8;++i)
  {
    m_pd3dDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    m_pd3dDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    m_pd3dDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
  }

  D3DMATERIAL9 mtrl;
  ZEROMEM( &mtrl, sizeof(D3DMATERIAL9) );
  mtrl.Ambient.r = 1.f;
  mtrl.Ambient.g = 1.f;
  mtrl.Ambient.b = 1.f;
  mtrl.Ambient.a = 1.f;
  m_pd3dDevice->SetMaterial(&mtrl);  

  m_bDisplayChange = false;
  return RET_OK;
}

// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::DrawText()
// Descripcion  : 
// Retorno      : void 
// Parametros   : float x
//              : float y
//              : DWORD uColor
//              : const char* pszTexto
//              : ...
// --------------------------------------------------------------------------------
void CDisplayDevice::DrawText(float x, float y, DWORD uColor,const char* pszTexto, ... ) const
{
  if(!m_bDisplayChange)
  {
    char szCadenaMsg[3000];
    va_list lista;	
    va_start(lista,pszTexto);
    vsprintf(szCadenaMsg,pszTexto,lista);
    va_end(lista);
    m_pFont->DrawText(x,y,uColor,szCadenaMsg);
  }
}




// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::BeginScene()
// Descripcion  : 
// Retorno      : bool
// --------------------------------------------------------------------------------
bool CDisplayDevice::BeginScene(float zClear)
{
  if(m_bDisplayChange || !m_bInit)
    return false;

  IDirect3DDevice9* pd3dDevice = GetD3DDevice();
  if((D3D_OK != pd3dDevice->TestCooperativeLevel()) && (RET_FAIL == Reset(GetWidth(), GetHeight(), m_bFullScreen)) )
    return false;

  D3DVIEWPORT9 vp;
  vp.X      = 0;
  vp.Y      = 0;
  vp.Width  = GetWidth();
  vp.Height = GetHeight();
  vp.MinZ   = 0.f;
  vp.MaxZ   = 1.0f;
  pd3dDevice->SetViewport(&vp);

  pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER , D3DCOLOR_XRGB(0,0,0), zClear, 0 );

  return D3D_OK == pd3dDevice->BeginScene();
}

extern DWORD g_framesLatency;
extern bool g_bShowStats;

DWORD g_dwFrames  = 0;
// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::EndScene()
// Descripcion  : 
// Retorno      : void
// --------------------------------------------------------------------------------
extern int frame_count;
void CDisplayDevice::EndScene()
{
  //FillTexturaBlt();

  if(m_bDisplayChange)
    return;

  IDirect3DDevice9* pd3dDevice = GetD3DDevice();

  static float fFPS = 0.f;



  if (g_framesLatency==0)
    g_framesLatency = 1;

  if (g_bShowStats)
  {  
    DrawText(0,400,0xff00ff00,"%.2f fps",fFPS);
//    DrawText(0,410,0xff00ff00,"CPU: %3d",g_uCPUUsage);
  }

	frame_count++;
  g_framesLatency = 0;

  {
    static FLOAT fLastTime = 0.0f;
    static DWORD dwFrames  = 0;
    FLOAT fTime = float(GetTickCount()) / 1000.f;
    ++dwFrames;

    // Update the scene stats once per second
    if( fTime - fLastTime > 1.0f )
    {
      fFPS    = dwFrames / (fTime - fLastTime);
      fLastTime = fTime;
      dwFrames  = 0;
    }
  }

  pd3dDevice->EndScene();	
}

// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::Flip()
// Descripcion  : 
// Retorno      : void
// --------------------------------------------------------------------------------
void CDisplayDevice::Flip()
{
  IDirect3DDevice9* pd3dDevice = GetD3DDevice();
  if(!m_bDisplayChange && pd3dDevice)
    pd3dDevice->Present( NULL, NULL, NULL, NULL );
}


/*
// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::PreShutdown()
// Descripcion  : 
// Retorno      : void 
// --------------------------------------------------------------------------------
void CDisplayDevice::PreShutdown()
{
  int i;
  for (i=0;i<8;i++)
    m_pd3dDevice->SetTexture(i,NULL);
  
  m_pd3dDevice->SetVertexDeclaration(NULL);
  m_pd3dDevice->SetIndices(NULL);
  m_pd3dDevice->SetPixelShader(NULL);
  m_pd3dDevice->SetVertexShader(NULL);
  m_pd3dDevice->SetStreamSource(0,NULL,0,0);
}
*/
/*

// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::DrawSphere()
// Descripcion  : 
// Retorno      : void 
// Parametros   : D3DXVECTOR3* pvPos
//              : DWORD uColor
//              : D3DXVECTOR3* pvScale
// --------------------------------------------------------------------------------
void CDisplayDevice::DrawSphere(const D3DXVECTOR3* pvPos, DWORD uColor,
                                const D3DXVECTOR3* pvScale) const
{
  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  m_pd3dDevice->SetRenderState(D3DRS_AMBIENT, uColor);

  m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

  D3DXMATRIX matWorld;

  if (pvScale)
    D3DXMatrixScaling(&matWorld, pvScale->x, pvScale->y, pvScale->z);
  else
    D3DXMatrixIdentity(&matWorld);

  matWorld._41 = pvPos->x;
  matWorld._42 = pvPos->y;
  matWorld._43 = pvPos->z;

  m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

  m_pd3dDevice->SetTexture(0, NULL);

  m_pd3dDevice->SetFVF(m_pEsfera->GetFVF());

  m_pd3dDevice->SetVertexShader(NULL);
  m_pd3dDevice->SetPixelShader(NULL);


  m_pEsfera->DrawSubset(0);

  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

  D3DXMatrixIdentity(&matWorld);
  m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

  m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
}
*/
/*
// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::DrawAABB()
// Descripcion  : 
// Retorno      : void 
// Parametros   : const D3DXVECTOR3* pvMin
//              : const D3DXVECTOR3* pvMax
// --------------------------------------------------------------------------------
void CDisplayDevice::DrawAABB(const D3DXVECTOR3* pvMin, const D3DXVECTOR3* pvMax,
                              DWORD uiColor) const
{
  TD3DLVERTEX v[8];


  D3DXVECTOR3 vMin, vMax;
      

  vMin = *pvMin;
  vMax = *pvMax;

    
  v[0].x = vMin.x; v[0].y = vMin.y; v[0].z = vMin.z;
  v[1].x = vMin.x; v[1].y = vMax.y; v[1].z = vMin.z;
  v[2].x = vMax.x; v[2].y = vMax.y; v[2].z = vMin.z;
  v[3].x = vMax.x; v[3].y = vMin.y; v[3].z = vMin.z;
  
  v[4].x = vMin.x; v[4].y = vMin.y; v[4].z = vMax.z;
  v[5].x = vMin.x; v[5].y = vMax.y; v[5].z = vMax.z;
  v[6].x = vMax.x; v[6].y = vMax.y; v[6].z = vMax.z;
  v[7].x = vMax.x; v[7].y = vMin.y; v[7].z = vMax.z;
  
  
  v[0].uiRGBA = uiColor; v[1].uiRGBA = uiColor;
  v[2].uiRGBA = uiColor; v[3].uiRGBA = uiColor;
  v[4].uiRGBA = uiColor; v[5].uiRGBA = uiColor;
  v[6].uiRGBA = uiColor; v[7].uiRGBA = uiColor;

    


  unsigned short idsCubo[36] = {0,1,2,0,2,3,0,4,7,0,7,3,3,2,6,3,6,7,
                                0,1,5,0,5,4,4,5,6,4,6,7,1,5,6,1,6,2};


  D3DXMATRIX matWorld;

  D3DXMatrixIdentity(&matWorld);

  m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
 

  m_pd3dDevice->SetVertexShader(NULL);
  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
  m_pd3dDevice->SetTexture(0, NULL);
  m_pd3dDevice->SetFVF(MD3DFVF_LVERTEX);
  m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 8, 12, idsCubo,
    D3DFMT_INDEX16, v, sizeof(TD3DLVERTEX));    

  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
}

*/
#if 0
void CDisplayDevice::DrawLineList(DWORD uNumVerts, const D3DXVECTOR3* paVerts, DWORD uColor)const
{
  TD3DLVERTEX v[100];
  DWORD i;

  for (i=0;i<uNumVerts;i++)
  {
    /*
    v[i*2].x = paVerts[i].x;
    v[i*2].y = paVerts[i].y;
    v[i*2].z = paVerts[i].z;
    v[i*2].uiRGBA = uColor;


    v[i*2+1].x = paVerts[(i+1)%uNumVerts].x;
    v[i*2+1].y = paVerts[(i+1)%uNumVerts].y;
    v[i*2+1].z = paVerts[(i+1)%uNumVerts].z;
    v[i*2+1].uiRGBA = uColor;
    */

    v[i].x = paVerts[i].x;
    v[i].y = paVerts[i].y;
    v[i].z = paVerts[i].z;
    v[i].uiRGBA = uColor;
  }


  D3DXMATRIX matWorld;

  D3DXMatrixIdentity(&matWorld);

  m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

  m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

  m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

  m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  
  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

  
  m_pd3dDevice->SetVertexShader(NULL);  
  m_pd3dDevice->SetTexture(0, NULL);
  m_pd3dDevice->SetPixelShader(NULL);
  m_pd3dDevice->SetFVF(MD3DFVF_LVERTEX);
  //m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST,uNumVerts*2, v, sizeof(TD3DLVERTEX));

  m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,uNumVerts-2, v, sizeof(TD3DLVERTEX));

  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE); 
}
#endif

/*
// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::DrawRect()
// Descripcion  : 
// Retorno      : void 
// Parametros   : const RECT* pRect
//              : DWORD uColor
// --------------------------------------------------------------------------------
void CDisplayDevice::DrawRect(const RECT* pRect, DWORD uColor) const
{
  TD3DTLVERTEX aVerts[4];

  aVerts[0].x = float(pRect->left);
  aVerts[0].y = float(pRect->top);

  aVerts[1].x = float(pRect->left);
  aVerts[1].y = float(pRect->bottom);

  aVerts[2].x = float(pRect->right);
  aVerts[2].y = float(pRect->bottom);

  aVerts[3].x = float(pRect->right);
  aVerts[3].y = float(pRect->top);

  int i;

  for (i=0;i<4;i++)
  {
    aVerts[i].uiRGBA = uColor;
    aVerts[i].z = 0.f;
    aVerts[i].oow = 1.f;
  }


  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

  m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

  m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

  m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);


  m_pd3dDevice->SetVertexShader(NULL);  
  m_pd3dDevice->SetTexture(0, NULL);
  m_pd3dDevice->SetPixelShader(NULL);
  m_pd3dDevice->SetFVF(MD3DFVF_TLVERTEX);
  //m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST,uNumVerts*2, v, sizeof(TD3DLVERTEX));

  m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,2, aVerts, sizeof(TD3DTLVERTEX));

  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE); 

}
*/


/*
// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::SetupDeviceObjets()
// Descripcion  : 
// Retorno      : void 
// --------------------------------------------------------------------------------
void CDisplayDevice::SetupDeviceObjets()
{
  m_pFont = NEW(CD3DFont("Arial", 8));
  m_pFont->InitDeviceObjects(m_pd3dDevice);
  m_pFont->RestoreDeviceObjects();

  D3DXCreateSphere(m_pd3dDevice, 1.f, 15, 15, &m_pEsfera, NULL);

  m_pd3dDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION, &m_pQuery);

  m_uFrames = m_uFramesLatencia = m_uPixelsDibujados = 0;
}
*/

/*
// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::DeleteDeviceObjects()
// Descripcion  : 
// Retorno      : void 
// --------------------------------------------------------------------------------
void CDisplayDevice::DeleteDeviceObjects()
{
//SAFE_RELEASE(m_pEsfera);
  DISPOSE(m_pFont);
//SAFE_RELEASE(m_pQuery);
}
*/
/*
// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::DrawPolygon()
// Descripcion  : 
// Retorno      : void 
// Parametros   : DWORD uNumVertices
//              : const D3DXVECTOR3* paVertices
//              : DWORD uColor = 0xffffffff
// --------------------------------------------------------------------------------
void CDisplayDevice::DrawPolygon(DWORD uNumVertices, const D3DXVECTOR3* paVertices, DWORD uColor) const
{
  TD3DLVERTEX v[100];

  DWORD i = 0;
  for (i=0;i<uNumVertices;i++)
  {
    v[i].x = paVertices[i].x;
    v[i].y = paVertices[i].y;
    v[i].z = paVertices[i].z;
    v[i].uiRGBA = uColor;
  }




  D3DXMATRIX matWorld;

  D3DXMatrixIdentity(&matWorld);

  m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);


  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  m_pd3dDevice->SetVertexShader(NULL);
  //m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  m_pd3dDevice->SetTexture(0, NULL);
  m_pd3dDevice->SetFVF(MD3DFVF_LVERTEX);
  m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, uNumVertices-2, v, sizeof(TD3DLVERTEX));
    

  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

}
*/
// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::StartQueryPixels()
// Descripcion  : 
// Retorno      : DWORD 
// Parametros   : DWORD* puFrameLatency
// --------------------------------------------------------------------------------
/*
DWORD CDisplayDevice::StartQueryPixels(DWORD* puFrameLatency)
{
  if (!m_pQuery)
  {
    if (puFrameLatency)
      *puFrameLatency = 0;
    return 0;
  }

  DWORD dwVizData;
  HRESULT hr = m_pQuery->GetData((void *) &dwVizData, sizeof(DWORD), 0);  

  if (hr == S_OK)
  {
    m_uPixelsDibujados = dwVizData;
    m_uFramesLatencia = m_uFrames;    
    m_bQuery = true;
    m_uFrames = 0;
  }
  else
  {
    m_uFrames++;
    m_bQuery = false;
  }

  if (m_bQuery)
    m_pQuery->Issue (D3DISSUE_BEGIN);

  if (puFrameLatency)
    *puFrameLatency = m_uFramesLatencia;

  return m_uPixelsDibujados;
}

// --------------------------------------------------------------------------------
// Nombre       : CDisplayDevice::EndQueryPixels()
// Descripcion  : 
// Retorno      : void 
// --------------------------------------------------------------------------------
void CDisplayDevice::EndQueryPixels()
{
  if (m_pQuery)
  {
    if (m_bQuery)
      m_pQuery->Issue (D3DISSUE_END);
  }
}

*/


/*

void CDisplayDevice::CrearTexturaBlit()
{
	IDirect3DDevice9* pd3dDevice = GetD3DDevice();
	m_pTextureBlt = NULL;

	//pd3dDevice->CreateTexture(1024,512,1,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_pTextureBlt,NULL);
	HRESULT hr = pd3dDevice->CreateTexture(640,480,1,D3DUSAGE_DYNAMIC,D3DFMT_R5G6B5,D3DPOOL_DEFAULT,&m_pTextureBlt,NULL);

	hr = hr;

}
*/

/*
extern unsigned char* g_pSH4VideoRAM;
void CDisplayDevice::FillTexturaBlt()
{
	D3DLOCKED_RECT lockedRect;
	HRESULT hr;

	hr = m_pTextureBlt->LockRect(0,&lockedRect,NULL,D3DLOCK_DISCARD|D3DLOCK_DONOTWAIT|D3DLOCK_NOSYSLOCK);

	ASSERT(!FAILED(hr));

	DWORD i;


	for (i=0;i<240;i++)
	{
		unsigned char* pDest = (unsigned char*)lockedRect.pBits + i*lockedRect.Pitch;

		//memcpy(pDest,g_pSH4VideoRAM + 0x200000 + 640*4*i,640*4);
		memcpy(pDest,g_pSH4VideoRAM + 320*2*i,320*2);

		//FillMemory(pDest,320*2,0x20);
		

	}

		

	m_pTextureBlt->UnlockRect(0);

	
	DWORD color = 0xffffffff;
	D3DXVECTOR2 v0,v1;

	v0.x = 0; v0.y = 0;
	v1.x = 512; v1.y = 512;

	TD3DTLVERTEXT1 v[4];  



	// 20
	// 31

	v[0].x = v1.x;   v[0].y = v0.y;
	v[0].z = 0.f;    v[0].oow = 1.f;
	v[0].uiRGBA = color;
	v[0].u = 1.f; v[0].v = 0.f;
	


	v[1].x = v1.x;   v[1].y = v1.y;
	v[1].z = 0.f;    v[1].oow = 1.f;
	v[1].uiRGBA = color;
	v[1].u = 1.f; v[1].v = 1.f;

	v[2].x = v0.x;   v[2].y = v0.y;
	v[2].z = 0.f;    v[2].oow = 1.f;
	v[2].uiRGBA = color;
	v[2].u = 0.f; v[2].v = 0.f;

	v[3].x = v0.x;   v[3].y = v1.y;
	v[3].z = 0.f;    v[3].oow = 1.f;
	v[3].uiRGBA = color;
	v[3].u = 0.f; v[3].v = 1.f;


  m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	m_pd3dDevice->SetVertexShader(NULL);	
	m_pd3dDevice->SetTexture(0, m_pTextureBlt);
	m_pd3dDevice->SetFVF(MD3DFVF_TLVERTEXT1);
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,v,sizeof(TD3DTLVERTEXT1));		
}

*/


