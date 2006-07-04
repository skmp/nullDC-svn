////////////////////////////////////////////////////////////////////////////////////////
/// @file  taVideo.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef taVideo_h
#define taVideo_h


enum  ESortMode
{
  E_SORT_NONE       = 0,
  E_SORT_BATCH      = 1,
  E_SORT_STRIP      = 2,
  E_SORT_PRIMITIVE  = 3,
  E_SORT_PIXEL      = 4
};

extern  bool      g_bUSE_ZWRITE;
extern  bool      g_bUSE_ALPHATEST_ZWRITE;
extern  ESortMode g_eSortMode;

namespace NewGrx
{
  void TASendPackedData(DWORD* pBuffer, DWORD uLength);

  void TAInit();
  void TAReset();
  void TANotifyDisplayAddressChange();
  void TAStartVBlank();
  void TAContinueRegistration();
  void TAResetRegistration();
  void TAStartRegistration();
  void TAStartRender();
  void TAEnd();
  bool TAIsOldGfx();

  extern  DWORD m_uNumVerticesRegistered;
  extern  DWORD m_uNumPrimitivesRegistered;
  extern  bool  g_bDraw;
  extern  bool  g_bChangeDisplayEnable;
}

namespace Unai
{
  void TASendPackedData(DWORD* pBuffer, DWORD uLength);

  void TAInit   ();
  void TAReset  ();
  void TASetVideoMode(DWORD uWidth,DWORD uHeight, BOOL bFullScreen = false);
  void TANotifyDisplayAddressChange();
  void TAStartVBlank();
  void TAContinueRegistration();
  void TAResetRegistration();
  void TAStartRegistration();
  void TAStartRender();
  void TAEnd();
  bool TAIsOldGfx();

  extern  DWORD m_uNumVerticesRegistered;
  extern  DWORD m_uNumPrimitivesRegistered;
  extern  bool  g_bDraw;
  extern  bool  g_bChangeDisplayEnable;
  void InvTexture(void* ptex);
}

namespace Wip
{
  void TASendPackedData(DWORD* pBuffer, DWORD uLength);

  void TAInit();
  void TAReset();
  void TASetVideoMode(DWORD uWidth,DWORD uHeight, BOOL bFullScreen = false);
  void TANotifyDisplayAddressChange();
  void TAStartVBlank();
  void TAContinueRegistration();
  void TAResetRegistration();
  void TAStartRegistration();
  void TAStartRender();
  void TAEnd();
  bool TAIsOldGfx();

  extern  DWORD m_uNumVerticesRegistered;
  extern  DWORD m_uNumPrimitivesRegistered;
  extern  bool  g_bDraw;
  extern  bool  g_bChangeDisplayEnable;
}

namespace Garrofi
{
  void TASendPackedData(DWORD* pBuffer, DWORD uLength);

  void TAInit();
  void TAReset();
  void TASetVideoMode(DWORD uWidth,DWORD uHeight, BOOL bFullScreen = false);
  void TANotifyDisplayAddressChange();
  void TAStartVBlank();
  void TAContinueRegistration();
  void TAResetRegistration();
  void TAStartRegistration();
  void TAStartRender();
  void TAEnd();
  bool TAIsOldGfx();

  extern  DWORD m_uNumVerticesRegistered;
  extern  DWORD m_uNumPrimitivesRegistered;
  extern  bool  g_bDraw;
  extern  bool  g_bChangeDisplayEnable;
}


#define PVR_TXRFMT_ARGB15555 0
#define PVR_TXRFMT_RGB565 1
#define PVR_TXRFMT_ARGB4444 2
#define PVR_TXRFMT_YUV422 3
#define PVR_TXRFMT_BUMP 4
#define PVR_TXRFMT_4BPP_PALETTE 5
#define PVR_TXRFMT_8BPP_PALETTE 6


//using namespace Wip;
using namespace Unai;
//using namespace Garrofi;



#endif