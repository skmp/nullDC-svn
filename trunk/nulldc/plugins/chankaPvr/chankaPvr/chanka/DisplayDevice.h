// ----------------------------------------------------------------------------------------
// Nombre       : DisplayDevice.h
// Descripcion  : 
// ----------------------------------------------------------------------------------------

#ifndef _DisplayDevice_h
#define _DisplayDevice_h



class CD3DFont;

class CDisplayDevice
{
public:
  CDisplayDevice             (): m_bInit(false)        { m_pd3dDevice = 0; m_pD3D=0; }
  ~CDisplayDevice            ()                        { End(); }

  enum
  {
    F_TWO_SIDED_STENCIL     = 0x00001,
    F_SCISSOR_TEST          = 0x00002,
    F_SLOPE_DEPTH_BIAS      = 0x00004,
  };

  TError        Init        (DWORD uWidth, DWORD uHeight, void* hWnd, BOOL bFullScreen);
  void          End         ();

  TError        Reset       (DWORD uWidth, DWORD uHeight, BOOL bFullScreen);
  bool          BeginScene  (float zClear = 1.f);
  void          EndScene    ();
  void          Flip        ();

  void          DrawText    (float x, float y, DWORD uColor,const char* pszTexto, ... ) const;

  IDirect3DDevice9* GetD3DDevice  ()  const { return m_pd3dDevice;}

  DWORD         GetWidth          ()  const { return m_dwWidth;}
  DWORD         GetHeight         ()  const { return m_dwHeight;}
  BOOL          GetFullScreen     ()  const { return m_bFullScreen;}
  BOOL          GetDisplayChange  ()  const { return m_bDisplayChange;  }
  void*         GetHWND           ()  const { return m_hWnd;}

  bool          CanTwoSidedStencil()  const { return (m_dwFlags & F_TWO_SIDED_STENCIL) != 0; }
  bool          CanScissorTest    ()  const { return (m_dwFlags & F_SCISSOR_TEST)      != 0; }
  bool          CanSlopeDepthBias ()  const { return (m_dwFlags & F_SLOPE_DEPTH_BIAS)  != 0; }
  float         GetMaxW           ()  const { return (m_fMaxW);  }


  /*
  DWORD             StartQueryPixels(DWORD* puFrameLatency);
  void              EndQueryPixels();
  LPDIRECT3DQUERY9  GetQuery() const           { return m_pQuery;}
	void              CrearTexturaBlit();
	void              FillTexturaBlt();
  */
  /*
  void          DrawSphere   (const D3DXVECTOR3* pvPos, DWORD uColor = 0xffffffff,
  const D3DXVECTOR3* pvScale = NULL) const;
  void          DrawAABB    (const D3DXVECTOR3* pvMin, const D3DXVECTOR3* pvMax,
  DWORD uiColor = 0xffff00ff) const;

  void          DrawLineList (DWORD uNumVerts, const D3DXVECTOR3* paVerts, DWORD uColor = 0xffffffff)const;

  void          DrawRect(const RECT* pRect, DWORD uColor = 0xffffffff) const;
  void          DrawPolygon(DWORD uNumVertices, const D3DXVECTOR3* paVertices, DWORD uColor = 0xffffffff) const;
  */

  
protected:
  void          PreShutdown();
  void          DeleteDeviceObjects();
  void          SetupDeviceObjets();

  bool                            m_bInit;

  IDirect3D9*                     m_pD3D;
  IDirect3DDevice9*               m_pd3dDevice;
  D3DPRESENT_PARAMETERS           m_d3dpp;
  
  DWORD                           m_dwWidth;
  DWORD                           m_dwHeight;
  BOOL                            m_bFullScreen;
  volatile  BOOL                  m_bDisplayChange;
  
  CD3DFont*                       m_pFont;
  DWORD                           m_uPixelsDibujados;
  DWORD                           m_uFramesLatencia;
  DWORD                           m_uFrames;
  void*                           m_hWnd;
  DWORD                           m_dwFlags;
  float                           m_fMaxW;

  /*
  ID3DXMesh*                      m_pEsfera;
  LPDIRECT3DQUERY9                m_pQuery;
  bool                            m_bQuery;
	IDirect3DTexture9* m_pTextureBlt;
  */
};



#endif