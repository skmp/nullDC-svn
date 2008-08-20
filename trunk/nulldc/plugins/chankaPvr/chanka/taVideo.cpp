////////////////////////////////////////////////////////////////////////////////////////
/// @file  taVideo.c
/// @brief
////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

  extern bool render_end_pending;
  extern u32 render_end_pending_cycles;
bool      g_bUSE_ZWRITE           = FALSE;
bool      g_bUSE_ALPHATEST_ZWRITE = FALSE;
ESortMode g_eSortMode             = E_SORT_BATCH;



////////////////////////////////////////////////////////////////////////////////////////
//extern  CSH4*   g_pSH4;
//extern  bool    m_bResetList;
//extern  float   g_fMaxW;
extern  bool    g_bWireframe;
extern DWORD    g_dwCreationWidth;
extern DWORD    g_dwCreationHeight;
extern BOOL     g_bCreationFullScreen;
extern  HWND    g_hWnd;
u32 FrameCount=0;
u32 VertexCount=0;
////////////////////////////////////////////////////////////////////////////////////////
void DrawFpsText(char*str);
extern bool g_bShowStats;
void rend_set_fps_text(char* text)
{
	//sprintf(fpsStr,"FPS: %4.2f(%4.2f) Vert : %4.2fM -  Sh4: %4.2f mhz (%4.2f%%) - %s", spd_fps,fullfps,mv, spd_cpu,spd_cpu*100/200,emu_name);
	if (!g_bCreationFullScreen)
		SetWindowText((HWND)Hwnd, text);
	if (g_bShowStats )
		DrawFpsText(text);
}

namespace Unai
{
////////////////////////////////////////////////////////////////////////////////////////
#define STAMP_RENDER 0xabcdefab

////////////////////////////////////////////////////////////////////////////////////////
CCacheTextures  m_CacheTextures;
CDisplayDevice  m_DisplayDevice;

void InvTexture(void* ptex)
{
	m_CacheTextures.InvalidateTexture(ptex,&m_CacheTextures);
}

DWORD m_uNumVerticesRegistered = 0;
DWORD m_uNumPrimitivesRegistered = 0;
float m_fXCoorRatio = 1.0f;
float m_fYCoorRatio = 1.0f;
float m_fMaxW       = 1.0f;
bool  m_bResetList  = true;

bool  g_bDraw = true;
bool  g_bChangeDisplayEnable;

////////////////////////////////////////////////////////////////////////////////////////
void TADoRender();

static void DrawBackground();
////////////////////////////////////////////////////////////////////////////////////////
struct TADreamcast
{
  enum
  {
    CMD_TYPE_END_OF_LIST	= 0,
    CMD_TYPE_POLYGON			= 4,
    CMD_TYPE_SPRITE			  = 5,
    CMD_TYPE_VERTEX				= 7,
    CMD_TYPE_USER_CLIP		= 1,
    CMD_POLYGON_SIZE			= 64,
  };
};


static DWORD m_uListsCompleted;
static DWORD m_uCurrentList;
static DWORD m_uRenderAddress;
static DWORD m_uFlagsRender;
static DWORD m_uTimeStamp;
static DWORD m_uEndFrameTimeStamp;
static IDirect3DTexture9* m_pTextureBlt16;
static IDirect3DTexture9* m_pTextureBlt32;
static DWORD m_uLastFrameRegisteredVertex  = 0;


////////////////////////////////////////////////////////////////////////////////////////
enum
{
  E_PENDING_DATA_NONE,
  E_PENDING_DATA_CMD,
  E_PENDING_DATA_VERTEX_SPRITE,
  E_PENDING_DATA_VERTEX,
};
static DWORD m_uPendingDataState;
static float m_aBaseIntensity[4];
static float m_aOffsetIntensity[4];

#include "taVertexFormats.h"

////////////////////////////////////////////////////////////////////////////////////////
struct TRenderMode
{
  DWORD               m_aCmdParams[4];
  TRenderMode(){}
  TRenderMode(const DWORD* _aCmdParams)  { m_aCmdParams[0]=_aCmdParams[0];m_aCmdParams[1]=_aCmdParams[1];m_aCmdParams[2]=_aCmdParams[2];m_aCmdParams[3]=_aCmdParams[3];}
  inline bool   operator  <  (const TRenderMode& _mode)  const
  {
    if(m_aCmdParams[3]!=_mode.m_aCmdParams[3])
      return  m_aCmdParams[3] < _mode.m_aCmdParams[3];
    else
    if(m_aCmdParams[0]!=_mode.m_aCmdParams[0])
      return  m_aCmdParams[0] < _mode.m_aCmdParams[0];
    else
    if(m_aCmdParams[1]!=_mode.m_aCmdParams[1])
      return  m_aCmdParams[1] < _mode.m_aCmdParams[1];
    else
    if(m_aCmdParams[2]!=_mode.m_aCmdParams[2])
      return  m_aCmdParams[2] < _mode.m_aCmdParams[2];
    else
      return false;
  }
  inline bool   operator  != (const TRenderMode& _mode)  const
  {
    return  (m_aCmdParams[3]!=_mode.m_aCmdParams[3])  ||  (m_aCmdParams[0]!=_mode.m_aCmdParams[0])
          ||(m_aCmdParams[1]!=_mode.m_aCmdParams[1])  ||  (m_aCmdParams[2]!=_mode.m_aCmdParams[2]);
    return false;
  }
  inline bool   operator  == (const TRenderMode& _mode)  const
  {
    return  (m_aCmdParams[3]==_mode.m_aCmdParams[3])  &&  (m_aCmdParams[0]==_mode.m_aCmdParams[0])
          &&(m_aCmdParams[1]==_mode.m_aCmdParams[1])  &&  (m_aCmdParams[2]==_mode.m_aCmdParams[2]);
    return false;
  }
};

struct TVBuffer
{
  IDirect3DVertexBuffer9*       m_pVB;
  unsigned                      m_uFrame;
  unsigned                      m_uSize;
  unsigned                      m_uUsed;
  unsigned                      m_uBase;
  unsigned                      m_uLocked;
  TD3DTL2VERTEXT1*              m_pLockData;

private:
  void  reset()
  {
    m_pVB      = 0;
    m_uFrame   = 0;
    m_uSize    = 0;
    m_uUsed    = 0;
    m_uBase    = 0;
    m_uLocked  = 0;
    m_pLockData= 0;
  }

public:
  TVBuffer(const TVBuffer& _other)
  {
    m_pVB       =_other.m_pVB      ;
    m_uFrame    =_other.m_uFrame   ;
    m_uSize     =_other.m_uSize    ;
    m_uUsed     =_other.m_uUsed    ;
    m_uBase     =_other.m_uBase    ;
    m_uLocked   =_other.m_uLocked  ;
    m_pLockData =_other.m_pLockData;
    const_cast<TVBuffer*>(&_other)->reset();
  }

  TVBuffer& operator = (const TVBuffer& _other)
  {
    m_pVB       =_other.m_pVB      ;
    m_uFrame    =_other.m_uFrame   ;
    m_uSize     =_other.m_uSize    ;
    m_uUsed     =_other.m_uUsed    ;
    m_uBase     =_other.m_uBase    ;
    m_uLocked   =_other.m_uLocked  ;
    m_pLockData =_other.m_pLockData;
    const_cast<TVBuffer*>(&_other)->reset();
  }

  TVBuffer(IDirect3DDevice9*  _pDevice, unsigned _uSize=0)
  {
    m_uSize   = max(65536,_uSize);
    m_uFrame  = 0;
    m_uUsed   = 0;
    m_uBase   = 0;
    m_pVB     = 0;
    m_uLocked = 0;
    m_pLockData=0;
    _pDevice->CreateVertexBuffer(sizeof(TD3DTL2VERTEXT1)*m_uSize,D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY ,MD3DFVF_TL2VERTEXT1,D3DPOOL_DEFAULT,&m_pVB,NULL);
  }

  bool  SetUp(unsigned _uSize);
  void  Close()
  {
    m_uBase = m_uUsed = m_uLocked;
    m_pVB->Unlock();
    m_pLockData = 0;
    m_uLocked   = 0;
  }

  ~TVBuffer()
  {
    SAFE_RELEASE(m_pVB);
  }
};

struct TRenderBatch
{
  static  unsigned              s_uCurrentFrame;
  static  unsigned              s_uExtraVertex;
  static  unsigned              s_uRealVertex;
  static  unsigned              s_uDrawPrim;
  static  float                 s_fLastZMin;
  static  float                 s_fLastZMax;
  static  float                 s_fLastZRng;
  static  float                 s_fCurZMax[2];
  static  float                 s_fCurZMin[2];

  float                         m_fAverageZ;
  unsigned                      m_uCurStrip;
  unsigned                      m_uQueStatus;
  unsigned                      m_uFrame;
  std::vector<TD3DTL2VERTEXT1>  m_aBuffer;
  TRenderBatch()  : m_uQueStatus(0),m_uFrame(0),m_uCurStrip(0),m_fAverageZ(.0f)
{}

  inline  void      BeginStrip    ()  { m_uQueStatus=0; m_uCurStrip=0;}
  inline  void      LinkStrip     ();
  inline  void      AddVertex     (TD3DTL2VERTEXT1& _vertex);
  inline  TVBuffer* FlushBatch    (TVBuffer* _pLast, list<TVBuffer>&  _aHWBuffers);

  static  void      NewFrame()
  {
    s_fLastZMax = s_fCurZMax[0];// + (s_fCurZMax[0]-s_fCurZMin[0])*.05f;
    s_fLastZMin = s_fCurZMin[0];// - (s_fCurZMax[0]-s_fCurZMin[0])*.05f;
    if(s_fLastZMax<s_fLastZMin)
      swap(s_fLastZMax,s_fLastZMin);

    if(s_fLastZMax-s_fLastZMin < .2f)
    {
      s_fLastZMax +=.2f;
      s_fLastZMin -=.2f;
    }

    s_fLastZRng = .99f/(s_fLastZMax - s_fLastZMin);
    s_fCurZMin[0] = s_fCurZMin[1] = 100000.0f;
    s_fCurZMax[0] = s_fCurZMax[1] =-100000.0f;
    ++s_uCurrentFrame;
    s_uRealVertex  = 0;
    s_uExtraVertex = 0;
    s_uDrawPrim    = 0;
  }
};
unsigned  TRenderBatch::s_uCurrentFrame = 0;
unsigned  TRenderBatch::s_uExtraVertex  = 0;
unsigned  TRenderBatch::s_uRealVertex   = 0;
unsigned  TRenderBatch::s_uDrawPrim     = 0;
float     TRenderBatch::s_fLastZMin = -.5f;
float     TRenderBatch::s_fLastZMax = 1.5f;
float     TRenderBatch::s_fLastZRng = 2.0f;
float     TRenderBatch::s_fCurZMax[2] = {1.5f,1.5f};
float     TRenderBatch::s_fCurZMin[2] = {-.5f,-.5f};

#define VERTEX_ALLOC  1024


void      TRenderBatch::LinkStrip     ()
{
  if(m_aBuffer.capacity()-m_aBuffer.size() < 16)
    m_aBuffer.reserve(m_aBuffer.capacity()+VERTEX_ALLOC);
  s_uExtraVertex += 2;
  m_aBuffer.push_back(m_aBuffer.back());
  if((m_uCurStrip&1))
    m_aBuffer.push_back(m_aBuffer.back()),s_uExtraVertex++;
  m_uCurStrip = 0;
  m_uQueStatus= 1;
}

void      TRenderBatch::AddVertex     (TD3DTL2VERTEXT1& _vertex)
{
  _vertex.x = (_vertex.x * m_fXCoorRatio) + .5f;
  _vertex.y = (_vertex.y * m_fYCoorRatio) + .5f;
  _vertex.oow = _vertex.z*m_fMaxW*.5f;
  m_fAverageZ = max(m_fAverageZ,_vertex.z);
  if(0)
  {
  //_vertex.z   = zclamptype1(_vertex.z);
  }
  else
  {
    //  ajuste z y zbuffer "dinámico"
    if(_vertex.z<s_fCurZMin[0])
    {
      if(_vertex.z>s_fCurZMin[1])
        s_fCurZMin[0] = _vertex.z;
      else
      if(_vertex.z>-100000.0f)
      {
        s_fCurZMin[0] = s_fCurZMin[1];
        s_fCurZMin[1] =_vertex.z+.00001f;
      }
    }
    else
    if(_vertex.z>s_fCurZMax[0])
    {
      if(_vertex.z<s_fCurZMax[1])
        s_fCurZMax[0] = _vertex.z;
      else
      if(_vertex.z<100000.0f)
      {
        s_fCurZMax[0] = s_fCurZMax[1];
        s_fCurZMax[1] =_vertex.z-.00001f;
      }
    }
    if(_vertex.z<s_fLastZMin) _vertex.z = s_fLastZMin-.001f;
    if(_vertex.z>s_fLastZMax) _vertex.z = s_fLastZMax;
    _vertex.z = .0045f+((_vertex.z-s_fLastZMin)*s_fLastZRng);
  }

  ++s_uRealVertex;
  ++m_uCurStrip;
  m_aBuffer.push_back(_vertex);
  if(m_uQueStatus)
  {
    m_uQueStatus = 0;
    m_aBuffer.push_back(_vertex);
  }
}


bool  TVBuffer::SetUp(unsigned _uSize)
{
  ASSERT(m_uLocked<=m_uSize);
  ASSERT(m_uUsed<=m_uSize);
  if(m_uLocked>m_uUsed && m_uLocked-m_uUsed >= _uSize)
  {
    m_uBase  = m_uUsed;
    m_uUsed += _uSize;
    return true;
  }
  unsigned uNewSize = max(_uSize, 16384/2);
  if(m_uLocked && m_uSize-m_uLocked >= uNewSize)
  {
    m_uBase    = m_uLocked;
    m_uUsed    = m_uBase + _uSize;
    m_uLocked  = m_uBase + uNewSize;
    m_pVB->Unlock();
    m_pVB->Lock(m_uBase*sizeof(TD3DTL2VERTEXT1), uNewSize*sizeof(TD3DTL2VERTEXT1),(void**)&m_pLockData,D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK);
    return true;
  }
  if(!m_uLocked && m_uSize-m_uUsed>= uNewSize)
  {
    m_uBase   = m_uUsed;
    m_uUsed   = m_uBase + _uSize;
    m_uLocked = m_uBase + uNewSize;
    m_pVB->Lock(m_uBase*sizeof(TD3DTL2VERTEXT1), uNewSize*sizeof(TD3DTL2VERTEXT1),(void**)&m_pLockData,D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK);
    return true;
  }

  if(!m_uLocked && m_uFrame!=TRenderBatch::s_uCurrentFrame &&  m_uSize>= uNewSize)
  {
    m_uBase   = 0;
    m_uUsed   = _uSize;
    m_uLocked = uNewSize;
    m_pVB->Lock(m_uBase*sizeof(TD3DTL2VERTEXT1), uNewSize*sizeof(TD3DTL2VERTEXT1),(void**)&m_pLockData,D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
    return true;
  }
  return false;
}

TVBuffer* TRenderBatch::FlushBatch    (TVBuffer* _pLast, list<TVBuffer>&  _aHWBuffers)
{
  TVBuffer* pBuffer = 0;
  if(_pLast && _pLast->SetUp((unsigned)m_aBuffer.size()))
    pBuffer = _pLast;
  else
  {
    for(list<TVBuffer>::iterator it=_aHWBuffers.begin();it!=_aHWBuffers.end();++it)
    {
      TVBuffer& tBuffer = *it;
      if( tBuffer.SetUp((unsigned)m_aBuffer.size()) )
      {
        pBuffer = &tBuffer;
        _aHWBuffers.splice(_aHWBuffers.end(),_aHWBuffers, it);
        break;
      }
    }
  }

  if(!pBuffer)
  {
    _aHWBuffers.push_back(TVBuffer(m_DisplayDevice.GetD3DDevice(),unsigned(m_aBuffer.size())));
    pBuffer = &_aHWBuffers.back();
    pBuffer->m_uFrame=TRenderBatch::s_uCurrentFrame;
    pBuffer->SetUp((unsigned)m_aBuffer.size());
  }
  pBuffer->m_uFrame = TRenderBatch::s_uCurrentFrame;
  
  memcpy(pBuffer->m_pLockData,&m_aBuffer[0],(m_aBuffer.size()-1)*sizeof(TD3DTL2VERTEXT1));
  pBuffer->m_pLockData += m_aBuffer.size();
  return pBuffer;
}



template <typename T>
class Pool
{
  std::list<TRenderBatch*>  m_aFreeList;
  std::list<TRenderBatch>   m_aPool;
public:
  T*  Alloc ()
  {
    if(m_aFreeList.empty())
    {
      m_aPool.push_back(TRenderBatch());
      m_aFreeList.push_back(&m_aPool.back());
    }
    T* ret = m_aFreeList.front();
    m_aFreeList.pop_front();
    ret->m_aBuffer.reserve(VERTEX_ALLOC);
    return ret;
  }

  void  Free(T* _ptr)
  {
    m_aFreeList.push_back(_ptr);
  }
};


class CRenderQue
{
public:
  static  list<TVBuffer>              m_aHWBuffers;
  static  Pool<TRenderBatch>          m_BatchPool;
  std::map<TRenderMode,TRenderBatch*> m_BatchMap;
  TRenderBatch*                       m_pActiveBatch;

public:
  inline  void          NewFrame      ();
  inline  TRenderBatch* AddRenderMode (DWORD* _pBuffer);
  inline  TVBuffer*     FlushBatch    (TVBuffer* _pLast, TRenderBatch* _pBatch);
};
list<TVBuffer>      CRenderQue::m_aHWBuffers;
Pool<TRenderBatch>  CRenderQue::m_BatchPool;


TRenderBatch* CRenderQue::AddRenderMode(DWORD* _pBuffer)
{
  TRenderMode rMode(_pBuffer);
  std::map<TRenderMode,TRenderBatch*>::iterator it = m_BatchMap.find(rMode);

  if(it!=m_BatchMap.end())
  {
    m_pActiveBatch = it->second;
  }
  else
  {
    m_pActiveBatch = m_BatchPool.Alloc();
    m_BatchMap.insert(std::make_pair(rMode,m_pActiveBatch));
  }

  m_pActiveBatch->m_uFrame = TRenderBatch::s_uCurrentFrame;
  if(!m_pActiveBatch->m_aBuffer.size())
  {
    m_pActiveBatch->m_fAverageZ =-1000.0f;
    m_pActiveBatch->BeginStrip();
  }
  ASSERT(m_pActiveBatch);

  return m_pActiveBatch;
}

void  CRenderQue::NewFrame      ()
{
  m_pActiveBatch = 0;
  for(map<TRenderMode,TRenderBatch*>::iterator it=m_BatchMap.begin();it!=m_BatchMap.end();)
  {
    TRenderBatch* batch = it->second;
    size_t newSize=0;
    if(batch->m_uFrame!=TRenderBatch::s_uCurrentFrame)
    {
      m_BatchMap.erase(it++);
      if(/*batch->m_aBuffer.size() < batch->m_aBuffer.capacity()>>4 &&*/ batch->m_aBuffer.capacity()>VERTEX_ALLOC )
      {
        batch->m_aBuffer.clear();
      }
      batch->m_aBuffer.resize(0);
      m_BatchPool.Free(batch);
    }
    else
    {
      ++it;
      batch->m_aBuffer.resize(0);
    }
  }
}

TVBuffer*     CRenderQue::FlushBatch    (TVBuffer* _pLast, TRenderBatch* _pBatch)
{
  return _pBatch->FlushBatch(_pLast, m_aHWBuffers);
}


CRenderQue    g_aRenderList[5];
TRenderBatch* g_pActiveList;


enum
{
  TA_STATE_NONE,
  TA_STATE_REGISTRATION,

  TA_FLAGS_END_REGISTRATION = 1<<0,
  TA_FLAGS_BEGIN_RENDER = 1<<1,
};



static DWORD m_uTAState = TA_STATE_NONE;
static DWORD m_uTAFlags = 0;

void TASetVideoMode(DWORD uWidth,DWORD uHeight, BOOL bFullScreen)
{
  //  Free Resources
  IDirect3DDevice9* pd3dDevice = m_DisplayDevice.GetD3DDevice();
  if(!pd3dDevice)
    return;
  for (int i=0;i<8;i++)
    pd3dDevice->SetTexture(i,NULL);

  pd3dDevice->SetVertexDeclaration(NULL);
  pd3dDevice->SetIndices(NULL);
  pd3dDevice->SetPixelShader(NULL);
  pd3dDevice->SetVertexShader(NULL);
  pd3dDevice->SetStreamSource(0,NULL,0,0);
  SAFE_RELEASE(m_pTextureBlt16);
  SAFE_RELEASE(m_pTextureBlt32);
  CRenderQue::m_aHWBuffers.clear();
  m_CacheTextures.ReloadTextures();

  //  reset
  m_DisplayDevice.Reset(uWidth, uHeight, bFullScreen);

  //  alloc resources
  pd3dDevice = m_DisplayDevice.GetD3DDevice();
  HRESULT hr = pd3dDevice->CreateTexture(640,480,1,D3DUSAGE_DYNAMIC,D3DFMT_R5G6B5,D3DPOOL_DEFAULT,&m_pTextureBlt16,NULL);
  ASSERT(!FAILED(hr));
  hr = pd3dDevice->CreateTexture(640,480,1,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_pTextureBlt32,NULL);
  ASSERT(!FAILED(hr));
  for(int i=0;i<5;++i)
  {
    g_aRenderList[i].NewFrame();
  }
  CRenderQue::m_aHWBuffers.push_back(TVBuffer(pd3dDevice,0));
  CRenderQue::m_aHWBuffers.push_back(TVBuffer(pd3dDevice,0));
  CRenderQue::m_aHWBuffers.push_back(TVBuffer(pd3dDevice,0));
}

void TAReset()
{
  SH4WriteLongMemory(TPVR::PVR_PT_ALPHA_REF, 0xff);
  m_uListsCompleted = 0;
  m_uCurrentList = -1;
  m_uFlagsRender = 0;
  m_uTimeStamp = 0;
  m_uLastFrameRegisteredVertex  = 0;
  m_bResetList = true;
  g_pActiveList = 0;
}

void TAInit()
{
  TError retValue = m_DisplayDevice.Init(g_dwCreationWidth, g_dwCreationHeight, g_hWnd, g_bCreationFullScreen);
  if (retValue != RET_OK)
  {
    MessageBox(g_hWnd, "Can't init D3D. Your graphic card is not DX 9.0 compatible or your desktop resolution color is not 32 bit", 
      "ERROR",MB_OK | MB_ICONERROR);
    exit(0);
  }
  m_CacheTextures.Init(m_DisplayDevice.GetD3DDevice());
  TASetVideoMode(m_DisplayDevice.GetWidth(),m_DisplayDevice.GetHeight(),m_DisplayDevice.GetFullScreen());
  TAReset();
}

void TAEnd()
{  
}


static void DoFlip()
{
  m_DisplayDevice.Flip();
}

void TANotifyDisplayAddressChange()
{
  /*
  if ((m_uFlagsRender & (F_RENDER_DONE | F_FLIP_PENDENT)) == (F_RENDER_DONE 
| F_FLIP_PENDENT))
  {
  if (m_uRenderAddress == SH4HWRegistersGetValue(TPVR::PVR_DISPLAY_ADDR1))
  DoFlip();
  }
  */
}




static bool IsPaletteFormat(const DWORD* pBuffer)
{
  DWORD uFormatTexture = POLYGON_TXRFMT(pBuffer);
  if (uFormatTexture == PVR_TXRFMT_4BPP_PALETTE || uFormatTexture == PVR_TXRFMT_8BPP_PALETTE)
    return true;
  else
    return false;
}


static void SetTextureInfo(CCacheTextures::TTextureInfo* pInfo, const DWORD* pBuffer)
{
  pInfo->uWidth = POLYGON_USIZE(pBuffer);
  pInfo->uHeight = POLYGON_VSIZE(pBuffer);	
  pInfo->uFormatTexture = POLYGON_TXRFMT(pBuffer);
  pInfo->uFlags = 0;
  pInfo->uHash = (unsigned __int64(pBuffer[3])<<32) | unsigned __int64((pBuffer[2])&PVR_TA_PM2_USIZE_MASK) | 
    unsigned __int64((pBuffer[2])&PVR_TA_PM2_VSIZE_MASK);// | __int64((pBuffer[2])PVR_TA_PM2_MIPBIAS_MASK);

  if (!IsPaletteFormat(pBuffer))
  {  
    if (POLYGON_STRIDE(pBuffer)&&POLYGON_NONTWIDDLE(pBuffer)&&!POLYGON_VQ(pBuffer))
      m_CacheTextures.SetStride(SH4HWRegistersGetValue(0x005F80E4)*32);    
  }

  if (POLYGON_VQ(pBuffer))
    pInfo->uFlags |= CCacheTextures::TTextureInfo::F_VQ_TEXTURE;
  if (!POLYGON_NONTWIDDLE(pBuffer) || IsPaletteFormat(pBuffer) )  
  {  
    pInfo->uFlags |= CCacheTextures::TTextureInfo::F_TWIDLE;    
    pInfo->uLevels = POLYGON_MIPMAP(pBuffer);

    if (pInfo->uLevels)
      pInfo->uHeight = pInfo->uWidth;
  }
  else
    pInfo->uLevels = 0;
}

IDirect3DTexture9* GetTexture(const DWORD* pBuffer)
{
  if (!POLYGON_TEXTURE(pBuffer))
    return NULL;

  IDirect3DTexture9* pTexture;
  CCacheTextures::TTextureInfo textureInfo;
  SetTextureInfo(&textureInfo,pBuffer);

  if (!textureInfo.uWidth)
    return NULL;

  //ASSERT(!IsPaletteFormat(pBuffer)) || !POLYGON_STRIDE(pBuffer) || !POLYGON_NONTWIDDLE(pBuffer));

  /*if (POLYGON_STRIDE(pBuffer))
  return NULL;*/

  if (textureInfo.uFormatTexture == PVR_TXRFMT_4BPP_PALETTE)
  {
    m_CacheTextures.UpdatePalette(SH4HWRegistersGetValue(0x005f8108)&3,16, SH4HWRegistersGetPaletteTable()+POLYGON_TXRCLUT4(pBuffer)*16*4);
  }
  else if (textureInfo.uFormatTexture == PVR_TXRFMT_8BPP_PALETTE)
  {
    m_CacheTextures.UpdatePalette(SH4HWRegistersGetValue(0x005f8108)&3,256, SH4HWRegistersGetPaletteTable()+POLYGON_TXRCLUT8(pBuffer)*256*4);
  }

  pTexture = m_CacheTextures.GetTexture(&textureInfo,g_pSH4TextureMemory+COMPUTE_ADDRESS_TXR(pBuffer));

  return pTexture;
}



static DWORD m_uVertexEOS;

static void SetPolygonFillVertexFunc(DWORD* pBuffer)
{
  DWORD uPolygonMod = POLYGON_MODIFIERMODE(pBuffer);
  m_pFillVertexFunc = NULL;


  if (POLYGON_COLOURTYPE(pBuffer) == PVR_CLRFMT_ARGBPACKED)
  {
    if (POLYGON_TEXTURE(pBuffer))
    {
      if (POLYGON_UVFORMAT(pBuffer) == PVR_UVFMT_32BIT)
      {
        if (uPolygonMod)
          m_pFillVertexFunc = TFormatTexPackedColMod::FillVertex;
        else
          m_pFillVertexFunc = TFormatTexPackedCol::FillVertex;
      }
      else
      {
        if (uPolygonMod)
          m_pFillVertexFunc = TFormatTexPackedCol16UVMod::FillVertex;
        else
          m_pFillVertexFunc = TFormatTexPackedCol16UV::FillVertex;
      }
    }
    else
      m_pFillVertexFunc = TFormatNonTexPackedCol::FillVertex;
  }
  else if (POLYGON_COLOURTYPE(pBuffer) == PVR_CLRFMT_4FLOATS)
  {
    if (!POLYGON_TEXTURE(pBuffer))
      m_pFillVertexFunc = TFormatNonTexCol::FillVertex;
    else
    {
      if (POLYGON_UVFORMAT(pBuffer) == PVR_UVFMT_32BIT)
        m_pFillVertexFunc = TFormatTexCol::FillVertex;
      else
        m_pFillVertexFunc = TFormatTexCol16UV::FillVertex;
    }
  }
  else if (POLYGON_COLOURTYPE(pBuffer) == PVR_CLRFMT_INTENSITY)
  {
    if (POLYGON_SPECULAR(pBuffer) || uPolygonMod)
      m_uPendingDataState = E_PENDING_DATA_CMD;
    else
    {
      float* pafBuffer = (float*) pBuffer;
      m_aBaseIntensity[0] = pafBuffer[5]; m_aBaseIntensity[1] = pafBuffer[6];
      m_aBaseIntensity[2] = pafBuffer[7]; m_aBaseIntensity[3] = pafBuffer[4];
    }

    if (POLYGON_TEXTURE(pBuffer))
    {
      if (POLYGON_UVFORMAT(pBuffer) == PVR_UVFMT_32BIT)
      {
        if (uPolygonMod)
          m_pFillVertexFunc = TFormatTexIntensityMod::FillVertex;
        else
          m_pFillVertexFunc = TFormatTexIntensity::FillVertex;
      }
      else
      {
        if (uPolygonMod)
          m_pFillVertexFunc = TFormatTexIntensity16UVMod::FillVertex;
        else
          m_pFillVertexFunc = TFormatTexIntensity16UV::FillVertex;
      }
    }
    else
      m_pFillVertexFunc = TFormatNonTexIntensity::FillVertex;
  }
  else if (POLYGON_COLOURTYPE(pBuffer) == PVR_CLRFMT_INTENSITY_PREV)
  {
    if (POLYGON_TEXTURE(pBuffer))
    {
      if (POLYGON_UVFORMAT(pBuffer) == PVR_UVFMT_32BIT)
      {
        if (uPolygonMod)
          m_pFillVertexFunc = TFormatTexIntensityMod::FillVertex;
        else
          m_pFillVertexFunc = TFormatTexIntensity::FillVertex;
      }
      else
      {
        if (uPolygonMod)
          m_pFillVertexFunc = TFormatTexIntensity16UVMod::FillVertex;
        else
          m_pFillVertexFunc = TFormatTexIntensity16UV::FillVertex;
      }
    }
    else
      m_pFillVertexFunc = TFormatNonTexIntensity::FillVertex;
  }
  else
    ASSERT(false);
  ASSERT(m_pFillVertexFunc);
}



static DWORD m_uBaseSprite;
static DWORD m_uOffsetSprite;
TD3DTL2VERTEXT1 curVertex[4];
void TASendPackedData(DWORD* pBuffer, DWORD uLength)
{
  if (m_uTAState != TA_STATE_REGISTRATION)
    TAStartRegistration();
  
  if (m_uPendingDataState == E_PENDING_DATA_NONE)
  {
    DWORD uCmdType = CMD_TYPE(pBuffer);
    switch (uCmdType)
    {
    case TADreamcast::CMD_TYPE_POLYGON:
      // Aqui vienen las tonterias de definicion de poligono
      m_uNumPrimitivesRegistered++;
      m_uPendingDataState = E_PENDING_DATA_NONE;
      {
        DWORD uStripLength = POLYGON_STRIPLENGTH(pBuffer);
        if (m_bResetList)
        {
          m_uCurrentList = POLYGON_LISTTYPE(pBuffer);
          m_bResetList  = false;
          g_pActiveList = 0;
        }

        IDirect3DTexture9* pTexture;

        if (m_uCurrentList == 1 || m_uCurrentList == 3)
        {
          m_pFillVertexFunc = TFormatShadowVolume::FillVertex;
          pTexture = NULL;
        }
        else
        {
          SetPolygonFillVertexFunc(pBuffer);
          /*
          if (POLYGON_TEXTURE(pBuffer))
            pTexture = GetTexture(pBuffer);
          else
            pTexture = NULL;
            */
        }
      //g_pActiveList = g_aRenderList[m_uCurrentList].AddRenderMode(pTexture,pBuffer);
        g_pActiveList = g_aRenderList[m_uCurrentList].AddRenderMode(pBuffer);
      }
      break;
    case TADreamcast::CMD_TYPE_VERTEX:
      {
        if (m_pFillVertexFunc)
        {
          ASSERT(m_uCurrentList!=-1);
          m_pFillVertexFunc(pBuffer, curVertex);
          if (m_uPendingDataState == E_PENDING_DATA_NONE)
          {
            m_uNumVerticesRegistered++;

            g_pActiveList->AddVertex(curVertex[0]);
            if (VERTEX_EOS(pBuffer))
            {
              g_pActiveList->LinkStrip();
            }
          }
          else
            m_uVertexEOS = VERTEX_EOS(pBuffer);
        }
      }
      break;
    case TADreamcast::CMD_TYPE_END_OF_LIST:
      {
        if (m_uCurrentList != -1)// && uRenderState == E_BEGIN_RENDER)
        {
          g_pActiveList=0;
          m_bResetList = true;
          m_uLastFrameRegisteredVertex = CurrentFrame;
		  
          switch (m_uCurrentList)
          {
          case TPVR::PVR_LIST_OP_POLY:
            //TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_PVR_OPAQUEDONE);
			  param.RaiseInterrupt(holly_OPAQUE);
            m_uListsCompleted|=(1<<m_uCurrentList);
            break;
          case TPVR::PVR_LIST_OP_MOD:
            //TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_PVR_OPAQUEMODDONE);
			  param.RaiseInterrupt(holly_OPAQUEMOD);
            m_uListsCompleted|=(1<<m_uCurrentList);
            break;
          case TPVR::PVR_LIST_TR_POLY:
            //TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_PVR_TRANSDONE);
			  param.RaiseInterrupt(holly_TRANS);
            m_uListsCompleted|=(1<<m_uCurrentList);
            break;
          case TPVR::PVR_LIST_TR_MOD:
            //TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_PVR_TRANSMODDONE);
			  param.RaiseInterrupt(holly_TRANSMOD);
            m_uListsCompleted|=(1<<m_uCurrentList);
            break;
          case TPVR::PVR_LIST_PT_POLY:
			
            //TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_PVR_PTDONE);
			    param.RaiseInterrupt(holly_PUNCHTHRU);
            m_uListsCompleted|=(1<<m_uCurrentList);
            break;
          }
          DWORD uFinalList = 0;

          if (GETVALUEHWREGISTER(TPVR::PVR_OPB_CFG,TPVR::PVR_OPB_CFG_OP))
            uFinalList |= (1<<TPVR::PVR_LIST_OP_POLY);
          if (GETVALUEHWREGISTER(TPVR::PVR_OPB_CFG,TPVR::PVR_OPB_CFG_OM))
            uFinalList |= (1<<TPVR::PVR_LIST_OP_MOD);
          if (GETVALUEHWREGISTER(TPVR::PVR_OPB_CFG,TPVR::PVR_OPB_CFG_TP))
            uFinalList |= (1<<TPVR::PVR_LIST_TR_POLY);
          if (GETVALUEHWREGISTER(TPVR::PVR_OPB_CFG,TPVR::PVR_OPB_CFG_TM))
            uFinalList |= (1<<TPVR::PVR_LIST_TR_MOD);
          if (GETVALUEHWREGISTER(TPVR::PVR_OPB_CFG,TPVR::PVR_OPB_CFG_PT))
            uFinalList |= (1<<TPVR::PVR_LIST_PT_POLY);


          if (uFinalList == m_uListsCompleted)
          {
            m_uTAFlags |= TA_FLAGS_END_REGISTRATION;
            m_uListsCompleted = 0;

            //((TSH4Registers*)(g_pSH4->GetRegisters()))->uCycles=(SH4_CYCLES_PER_FRAME*480)/DC_GLOBALS::PVR2_VLINES;
            if (m_uTAFlags & TA_FLAGS_BEGIN_RENDER)
              TADoRender();
          }
          m_uCurrentList = -1;
        }

      }
      break;
    case TADreamcast::CMD_TYPE_SPRITE:
      {
        m_uNumPrimitivesRegistered++;
        if (m_bResetList)
        {
          m_uCurrentList = POLYGON_LISTTYPE(pBuffer);
          m_bResetList   = false;
          g_pActiveList  = 0;
        }

        m_pFillVertexFunc = TFormatSpriteA::FillVertex;
      //IDirect3DTexture9* pTexture;
      //pTexture = GetTexture(pBuffer);

        m_uBaseSprite   = pBuffer[4];
        m_uOffsetSprite = pBuffer[5];
      //g_pActiveList = g_aRenderList[m_uCurrentList].AddRenderMode(pTexture,pBuffer);
        g_pActiveList = g_aRenderList[m_uCurrentList].AddRenderMode(pBuffer);
      }
      break;

    case TADreamcast::CMD_TYPE_USER_CLIP:
      {
        //m_pFillVertexFunc = NULL;
      }
      break;
    case 2:
      {
        if (m_bResetList)
        {
          m_uCurrentList = POLYGON_LISTTYPE(pBuffer);
          m_bResetList   = false;
          g_pActiveList  = 0;

          g_pActiveList = g_aRenderList[m_uCurrentList].AddRenderMode(pBuffer);
        }
        m_pFillVertexFunc = NULL;
      }
      break;
    case 6:
      {        
        m_pFillVertexFunc = NULL;
      }
      break;

    default:      
      m_pFillVertexFunc = NULL;
      ASSERT(false);
    }
  }
  else
  {
    switch (m_uPendingDataState)
    {
    case E_PENDING_DATA_VERTEX_SPRITE:
      {
        TFormatSpriteB* pSrc =(TFormatSpriteB*)(pBuffer);
        curVertex[2].y = pSrc->cy;
        curVertex[2].z = pSrc->cz;

        curVertex[3].x = pSrc->dx;
        curVertex[3].y = pSrc->dy;
        curVertex[3].z = pSrc->cz;

        float16Bit_To_32bit(pSrc->auv, &curVertex[0].u, &curVertex[0].v);
        float16Bit_To_32bit(pSrc->buv, &curVertex[1].u, &curVertex[1].v);
        float16Bit_To_32bit(pSrc->cuv, &curVertex[2].u, &curVertex[2].v);

        curVertex[3].u = curVertex[0].u;
        curVertex[3].v = curVertex[2].v;

        DWORD i;
        for (i=0;i<4;i++)
        {
          curVertex[i].uiRGBA         = m_uBaseSprite;
          curVertex[i].uiSpecularRGBA = m_uOffsetSprite;
        }
        g_pActiveList->AddVertex(curVertex[2]);
        g_pActiveList->AddVertex(curVertex[3]);
        g_pActiveList->AddVertex(curVertex[1]);
        g_pActiveList->AddVertex(curVertex[0]);
        g_pActiveList->LinkStrip();
      }
      break;
    case E_PENDING_DATA_CMD:
      {
        float* pafBuffer = (float*) pBuffer;

        m_aBaseIntensity[0] = pafBuffer[1]; m_aBaseIntensity[1] = pafBuffer[2];
        m_aBaseIntensity[2] = pafBuffer[3]; m_aBaseIntensity[3] = pafBuffer[0];

        m_aOffsetIntensity[0] = pafBuffer[5]; m_aOffsetIntensity[1] = pafBuffer[6];
        m_aOffsetIntensity[2] = pafBuffer[7]; m_aOffsetIntensity[3] = pafBuffer[4];
      }
      break;
    case E_PENDING_DATA_VERTEX:
      {
        m_uNumVerticesRegistered++;
        m_pFillVertexFunc(pBuffer, curVertex);
        g_pActiveList->AddVertex(curVertex[0]);
        if (m_uVertexEOS)
          g_pActiveList->LinkStrip();
      }
      break;
    default:
      ASSERT(false);
    }

    m_uPendingDataState = E_PENDING_DATA_NONE;
  }

}

static int m_aFrameBufferColBytes[] = {2,2,3,4};


#define VRAM_MASK (8*1024*1024-1)

//Convert offset32 to offset64
u32 vramlock_ConvOffset32toOffset64(u32 offset32)
{
		//64b wide bus is archevied by interleaving the banks every 32 bits
		//so bank is Address<<3
		//bits <4 are <<1 to create space for bank num
		//bank 1 is mapped at 400000 (32b offset) and after
		u32 bank=((offset32>>22)&0x1)<<2;//bank will be used ass uper offset too
		u32 lv=offset32&0x3; //these will survive
		offset32<<=1;
		//       |inbank offset    |       bank id        | lower 2 bits (not changed)
		u32 rv=  (offset32&(VRAM_MASK-7))|bank                  | lv;
 
		return rv;
}
static void FillBltTexture()
{
  if(!m_DisplayDevice.BeginScene() || !m_pTextureBlt32 || !m_pTextureBlt16)
    return;

  DWORD uDispAddr = SH4HWRegistersGetValue(TPVR::PVR_DISPLAY_ADDR1);

  DWORD uModulo = GETVALUEHWREGISTER(TPVR::PVR_DISPLAY_SIZE,TPVR::PVR_DISPLAY_SIZE_MODULO)-1;
  ASSERT(int(uModulo)>=0);

  DWORD uNumLines = GETVALUEHWREGISTER(TPVR::PVR_DISPLAY_SIZE,TPVR::PVR_DISPLAY_SIZE_LPF)+1;

  //uNumLines = 240;

  bool bScanDouble = GETVALUEHWREGISTER(TPVR::PVR_FB_CFG_1,TPVR::PVR_FB_CFG_1_SD) != 0;
  bool bClockDouble = GETVALUEHWREGISTER(TPVR::PVR_FB_CFG_1,TPVR::PVR_FB_CFG_1_C) != 0;
  bool bInterlaced = GETVALUEHWREGISTER(TPVR::PVR_SYNC_CFG,TPVR::PVR_SYNC_CFG_I) != 0;
  bool bLowRes = GETVALUEHWREGISTER(TPVR::PVR_VIDEO_CFG,TPVR::PVR_VIDEO_CFG_LR) != 0;

  if (bInterlaced)
    uNumLines<<=1;


  DWORD uResX = GETVALUEHWREGISTER(TPVR::PVR_DISPLAY_SIZE,TPVR::PVR_DISPLAY_SIZE_PDPL)+1;

  //uResX = 640;

  /*	if (bLowRes)
  uResX<<=1;*/

  DWORD uColFormat = GETVALUEHWREGISTER(TPVR::PVR_FB_CFG_1,TPVR::PVR_FB_CFG_1_COL);

  DWORD j;

  IDirect3DTexture9* pTextureD3D = NULL;

  const void* pSrc = SH4GetVideoRAMPtr(0xa5000000 + uDispAddr);
  ASSERT(pSrc);

  DWORD uBytesPerPixel = m_aFrameBufferColBytes[GETVALUEHWREGISTER(TPVR::PVR_FB_CFG_1,TPVR::PVR_FB_CFG_1_COL)];

  uResX = (uResX / uBytesPerPixel)<<2;

  if (uBytesPerPixel == 2)
    pTextureD3D = m_pTextureBlt16;
  else
    pTextureD3D = m_pTextureBlt32;


  DWORD uNumBytesPerLine = uResX*uBytesPerPixel;

  ASSERT(uResX<=640);
  if(uNumLines>480)
    uNumLines = 480;


  if (uColFormat != TPVR::PVR_FB_CFG_1_COL_RGB888)
  {
    D3DLOCKED_RECT lockedRect;
    RECT rect;
    rect.left = 0; rect.top = 0;
    rect.bottom = uNumLines; rect.right = uResX;

    HRESULT hr = pTextureD3D->LockRect(0,&lockedRect,&rect,D3DLOCK_NOSYSLOCK);
    ASSERT(!FAILED(hr));
    for (j=0;j<uNumLines;j++)
    {
      unsigned char* pBufferDest = (unsigned char*)lockedRect.pBits + j*lockedRect.Pitch;
      DWORD offset = (j*(/*uModulo*uBytesPerPixel +*/ uNumBytesPerLine));

	  /*const char* pBank1 = SH4GetVideoRAMPtr(0xa5000000 + (uDispAddr>>1));
	  const char* pBank2 = SH4GetVideoRAMPtr(0xa5400000 + (uDispAddr>>1));
      //memcpy(pBufferDest,pBufferSrc,uNumBytesPerLine);
	  DWORD bpp=uNumBytesPerLine>>1;
	  bpp>>=2;
	  for (int i=0;i<bpp;i++) 
	  {
		  memcpy(pBufferDest,pBank1+offset,4);pBufferDest+=4;
		  memcpy(pBufferDest,pBank2+offset,4);pBufferDest+=4;
		  offset+=4;
	  }*/
	  for (int i=0;i<uNumBytesPerLine;i+=1)
	  {
		  u32 new_offset= vramlock_ConvOffset32toOffset64(uDispAddr + offset);
		  char* nof=SH4GetVideoRAMPtr(new_offset);
		  *pBufferDest=*nof;
		  pBufferDest++;
		  offset++;
	  }

    }
	pTextureD3D->UnlockRect(0);
  }
  else if (uColFormat == TPVR::PVR_FB_CFG_1_COL_RGB888)
  {
	  D3DLOCKED_RECT lockedRect;
	  RECT rect;
	  rect.left = 0; rect.top = 0;
	  rect.bottom = uNumLines; rect.right = uResX;

	  HRESULT hr = pTextureD3D->LockRect(0,&lockedRect,&rect,D3DLOCK_NOSYSLOCK);
	  ASSERT(!FAILED(hr));
	  for (j=0;j<uNumLines;j++)
	  {
		  unsigned char* pBufferDest = (unsigned char*)lockedRect.pBits + j*lockedRect.Pitch;
		  DWORD offset = (j*(/*uModulo*uBytesPerPixel +*/ uNumBytesPerLine));

		  /*const char* pBank1 = SH4GetVideoRAMPtr(0xa5000000 + (uDispAddr>>1));
		  const char* pBank2 = SH4GetVideoRAMPtr(0xa5400000 + (uDispAddr>>1));
		  //memcpy(pBufferDest,pBufferSrc,uNumBytesPerLine);
		  DWORD bpp=uNumBytesPerLine>>1;
		  bpp>>=2;
		  for (int i=0;i<bpp;i++) 
		  {
		  memcpy(pBufferDest,pBank1+offset,4);pBufferDest+=4;
		  memcpy(pBufferDest,pBank2+offset,4);pBufferDest+=4;
		  offset+=4;
		  }*/
		  for (int i=0;i<uNumBytesPerLine;i+=1)
		  {
			  u32 new_offset= vramlock_ConvOffset32toOffset64(uDispAddr + offset);
			  char* nof=SH4GetVideoRAMPtr(new_offset);
			  *pBufferDest=*nof;
			  pBufferDest++;
			  offset++;
		  }

	  }
	  pTextureD3D->UnlockRect(0);
  }


  DWORD color = 0xffffffff;
  D3DXVECTOR2 v0,v1;

  v0.x = 0-0.5f; v0.y = 0-0.5f;
  v1.x = (640.0f*m_fXCoorRatio)-0.5f; v1.y = (480.0f*m_fYCoorRatio)-0.5f;

  D3DXVECTOR2 uvLL;

  TD3DTLVERTEXT1 v[4];

  uvLL.x = float(uResX) / 640.f;
  uvLL.y = float(uNumLines) / 480.f;



  // 20
  // 31

  v[0].x = v1.x;   v[0].y = v0.y;
  v[0].z = 0.f;    v[0].oow = 1.f;
  v[0].uiRGBA = color;
  v[0].u = uvLL.x; v[0].v = 0.f;



  v[1].x = v1.x;   v[1].y = v1.y;
  v[1].z = 0.f;    v[1].oow = 1.f;
  v[1].uiRGBA = color;
  v[1].u = uvLL.x; v[1].v = uvLL.y;

  v[2].x = v0.x;   v[2].y = v0.y;
  v[2].z = 0.f;    v[2].oow = 1.f;
  v[2].uiRGBA = color;
  v[2].u = 0.f; v[2].v = 0.f;

  v[3].x = v0.x;   v[3].y = v1.y;
  v[3].z = 0.f;    v[3].oow = 1.f;
  v[3].uiRGBA = color;
  v[3].u = 0.f; v[3].v = uvLL.y;


  IDirect3DDevice9* pd3dDevice = m_DisplayDevice.GetD3DDevice();
  pd3dDevice->SetVertexShader(NULL);
  pd3dDevice->SetTexture(0, pTextureD3D);
  pd3dDevice->SetFVF(MD3DFVF_TLVERTEXT1);
  pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
  pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
  pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
  pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
  pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
  pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
  pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,v,sizeof(TD3DTLVERTEXT1));

  m_DisplayDevice.EndScene();
  m_DisplayDevice.Flip();
}

void TAStartVBlank()
{
	CurrentFrame++;
	if (GetAsyncKeyState(VK_DELETE))
	{

	}
	else
	{
  if ((m_uNumVerticesRegistered != 0) || (CurrentFrame-m_uEndFrameTimeStamp)<8)
    return;
	


  if (m_uLastFrameRegisteredVertex > m_uEndFrameTimeStamp)
    return;
}
  DWORD uDisplayAddress = SH4HWRegistersGetValue(TPVR::PVR_DISPLAY_ADDR1);

  DWORD* pVideoMem = (DWORD*) SH4GetVideoRAMPtr(SH4VideoRAM_START+uDisplayAddress);

 // if (pVideoMem && *pVideoMem == STAMP_RENDER)
   // return;    

    *pVideoMem = STAMP_RENDER;
  //return;
  bool bDisplayEnable = GETVALUEHWREGISTER(TPVR::PVR_FB_CFG_1,TPVR::PVR_FB_CFG_1_DE) != 0;
  //bool bDisplayEnable = true;

  if (bDisplayEnable)
    FillBltTexture();
}



void TAContinueRegistration()
{
  m_bResetList = true;
  m_uTAFlags = 0;
  if (m_uTAState != TA_STATE_REGISTRATION)
    TAStartRegistration();
}

void TAResetRegistration()
{
  m_bResetList = true;
  g_pActiveList=0;
  if (m_uTAState == TA_STATE_REGISTRATION)
  {
    m_uTAState = TA_STATE_NONE;
    m_uNumVerticesRegistered = 0;
    m_uNumPrimitivesRegistered = 0;
    m_uTAFlags = 0;
    m_uListsCompleted = 0;
  }
}




void TAStartRegistration()
{
  TAResetRegistration();
  ASSERT(m_uTAState == TA_STATE_NONE);
  m_uNumVerticesRegistered = 0;
  m_uNumPrimitivesRegistered = 0;
  m_uTAFlags = 0;
  m_uListsCompleted = 0;
  m_uTAState = TA_STATE_REGISTRATION;
  m_fXCoorRatio = float(m_DisplayDevice.GetWidth())/640.0f;
  m_fYCoorRatio = float(m_DisplayDevice.GetHeight())/480.0f;
  m_fMaxW       = m_DisplayDevice.GetMaxW();
}


////////////////////////////////////////////////////////////////////////////////////////
/// @struct TOpaqueContext
/// @brief
////////////////////////////////////////////////////////////////////////////////////////
static  const DWORD aDepthModes [] ={ D3DCMP_NEVER,D3DCMP_LESS,D3DCMP_EQUAL,D3DCMP_LESSEQUAL,D3DCMP_GREATER,
                                      D3DCMP_NOTEQUAL,D3DCMP_GREATEREQUAL,D3DCMP_ALWAYS};
static  const DWORD aZWrite     [] ={ D3DZB_TRUE,D3DZB_FALSE};
static  const DWORD aCulling    [] ={ D3DCULL_NONE,D3DCULL_NONE,D3DCULL_CCW,D3DCULL_CW};

static  const DWORD aSrcBlend   [] ={ D3DBLEND_ZERO,D3DBLEND_ONE,D3DBLEND_DESTCOLOR,D3DBLEND_INVDESTCOLOR,
                                      D3DBLEND_SRCALPHA,D3DBLEND_INVSRCALPHA,D3DBLEND_DESTALPHA,D3DBLEND_INVDESTALPHA};

static  const DWORD aDstBlend   [] ={ D3DBLEND_ZERO,D3DBLEND_ONE,D3DBLEND_SRCCOLOR,D3DBLEND_INVSRCCOLOR,
                                      D3DBLEND_SRCALPHA,D3DBLEND_INVSRCALPHA,D3DBLEND_DESTALPHA,D3DBLEND_INVDESTALPHA};

static  const DWORD aColorOp    [] ={ D3DTOP_SELECTARG1, D3DTOP_MODULATE    , D3DTOP_BLENDDIFFUSEALPHA, D3DTOP_MODULATE };
static  const DWORD aAlphaOp    [] ={ D3DTOP_SELECTARG1, D3DTOP_SELECTARG1  , D3DTOP_SELECTARG2,        D3DTOP_MODULATE };

static  const DWORD aUVMode[2][2] = {{D3DTADDRESS_WRAP,  D3DTADDRESS_MIRROR},{D3DTADDRESS_CLAMP ,D3DTADDRESS_CLAMP}};
/*
void SetUVMode(DWORD flip, DWORD clamp)
{
  static const DWORD lookup[2][2] = {{D3DTADDRESS_WRAP,  D3DTADDRESS_MIRROR},{D3DTADDRESS_CLAMP ,D3DTADDRESS_CLAMP}};
  m_DisplayDevice.GetD3DDevice()->SetSamplerState(0,D3DSAMP_ADDRESSV,lookup[clamp&1][flip&1]);
  m_DisplayDevice.GetD3DDevice()->SetSamplerState(0,D3DSAMP_ADDRESSU,lookup[(clamp&2)>>1][(flip&2)>>1]);
}
*/

/*
void SetTextureSampler(DWORD uSampler)
{
  if (uSampler == PVR_FILTER_POINT_SAMPLED)
  {
  m_DisplayDevice.GetD3DDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
  m_DisplayDevice.GetD3DDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
  m_DisplayDevice.GetD3DDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

  }
  else
  {
  m_DisplayDevice.GetD3DDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  m_DisplayDevice.GetD3DDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  m_DisplayDevice.GetD3DDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
  }
}
*/


////////////////////////////////////////////////////////////////////////////////////////
/// @struct TOpaqueContext
/// @brief
////////////////////////////////////////////////////////////////////////////////////////
struct TOpaqueContext
{
  IDirect3DDevice9* m_pDevice;
  DWORD m_uCullMode;
  DWORD m_uDepthMode;
  DWORD m_uDepthWrite;
  DWORD m_uSpecular;
  DWORD m_uTxrShading;
  DWORD m_uUVMode;
  DWORD m_uFilter;
  DWORD m_uTexture;

  TOpaqueContext(IDirect3DDevice9* _pDevice)  : m_pDevice(_pDevice)
  {
    //  Defaults
    //m_pDevice->SetVertexShader(NULL);
    //m_pDevice->SetFVF(MD3DFVF_TL2VERTEXT1);
    //  States
    m_pDevice->SetRenderState(D3DRS_ZENABLE,         D3DZB_TRUE);
    m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
    m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE ,FALSE);
    //  Textures
    m_pDevice->SetTexture(0,0);
    m_pDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pDevice->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    //  State Cache
    m_uCullMode   = m_uDepthMode  = m_uDepthWrite = 0xffffffff;
    m_uSpecular   = m_uTxrShading = m_uUVMode     = m_uFilter = 0xffffffff;
    m_uTexture    = 0;
  }

  void Primitive(const TRenderMode& _rMode)
  {
    //  States
    const DWORD* pBuffer = _rMode.m_aCmdParams;
    DWORD uCullMode   = POLYGON_CULLING     (pBuffer);
    DWORD uDepthMode  = POLYGON_DEPTHMODE   (pBuffer);
    DWORD uDepthWrite = POLYGON_DEPTHWRITE  (pBuffer);
    DWORD uSpecular   = POLYGON_OFFSET_ON   (pBuffer);
    DWORD uUVMode     = POLYGON_UVCLAMP     (pBuffer) | (POLYGON_UVFLIP      (pBuffer)<<8);
    DWORD uFilter     = POLYGON_FILTER      (pBuffer);
    DWORD uTxrShading = POLYGON_TXRSHADING  (pBuffer);
    DWORD uTexture    = POLYGON_TEXTURE     (pBuffer) ? pBuffer[3] : 0;
    if(uCullMode    !=m_uCullMode)    m_uCullMode   = uCullMode,    m_pDevice->SetRenderState(D3DRS_CULLMODE,       aCulling    [m_uCullMode]);
    if(uDepthMode   !=m_uDepthMode)   m_uDepthMode  = uDepthMode,   m_pDevice->SetRenderState(D3DRS_ZFUNC,          aDepthModes [m_uDepthMode]);
    if(uDepthWrite  !=m_uDepthWrite)  m_uDepthWrite = uDepthWrite,  m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE,   aZWrite     [m_uDepthWrite]);
    if(uSpecular    !=m_uSpecular)    m_uSpecular   = uSpecular,    m_pDevice->SetRenderState(D3DRS_SPECULARENABLE, m_uSpecular);

    //  Textures
    if(uTexture!=m_uTexture) m_uTexture = uTexture, m_pDevice->SetTexture(0,uTexture?GetTexture(pBuffer):0);
    if(uTxrShading != m_uTxrShading)
    {
      m_uTxrShading = uTxrShading;
      m_pDevice->SetTextureStageState(0,D3DTSS_COLOROP,aColorOp[m_uTxrShading]);
    }

    //Mapping
    if(uUVMode!=m_uUVMode)
    {
      m_uUVMode = uUVMode;
      m_pDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,aUVMode[m_uUVMode&1]     [(m_uUVMode>>8)&1]);
      m_pDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,aUVMode[(m_uUVMode&2)>>1][((m_uUVMode>>8)&2)>>1]);
    }

    //Filltering
    if(m_uFilter!=uFilter)
    {
      m_uFilter = uFilter;
      if (m_uFilter == PVR_FILTER_POINT_SAMPLED)
      {
        m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

      }
      else
      {
        m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
      }
    }
  }
};

////////////////////////////////////////////////////////////////////////////////////////
/// @struct TPunchContext
/// @brief
////////////////////////////////////////////////////////////////////////////////////////
struct TPunchContext
{
  IDirect3DDevice9* m_pDevice;
  DWORD m_uCullMode;
  DWORD m_uDepthMode;
  DWORD m_uDepthWrite;
  DWORD m_uSpecular;
  DWORD m_uTxrShading;
  DWORD m_uUVMode;
  DWORD m_uFilter;
  DWORD m_uTexture;

  TPunchContext(IDirect3DDevice9* _pDevice, DWORD _uAlphaRef)  : m_pDevice(_pDevice)
  {
    //  Defaults
    //m_pDevice->SetVertexShader(NULL);
    //m_pDevice->SetFVF(MD3DFVF_TL2VERTEXT1);
    //  States
    m_pDevice->SetRenderState(D3DRS_ZENABLE,         D3DZB_TRUE);
    m_pDevice->SetRenderState(D3DRS_ZFUNC,           D3DCMP_GREATEREQUAL);

    m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
    m_pDevice->SetRenderState(D3DRS_ALPHAREF,        _uAlphaRef);
    m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    m_pDevice->SetRenderState(D3DRS_ALPHAFUNC,       D3DCMP_GREATEREQUAL);
    //  Textures
    m_pDevice->SetTexture(0,0);
    m_pDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pDevice->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    //  State Cache
    m_uCullMode   = m_uDepthMode  = m_uDepthWrite = 0xffffffff;
    m_uSpecular   = m_uTxrShading = m_uUVMode     = m_uFilter = 0xffffffff;
    m_uTexture    = 0;
  }

  void Primitive(const TRenderMode& _rMode)
  {
    //  States
    const DWORD* pBuffer = _rMode.m_aCmdParams;
    DWORD uCullMode   = POLYGON_CULLING     (pBuffer);
    DWORD uDepthMode  = POLYGON_DEPTHMODE   (pBuffer);
    DWORD uDepthWrite = POLYGON_DEPTHWRITE  (pBuffer);
    DWORD uSpecular   = POLYGON_OFFSET_ON   (pBuffer);
    DWORD uUVMode     = POLYGON_UVCLAMP     (pBuffer) | (POLYGON_UVFLIP      (pBuffer)<<8);
    DWORD uFilter     = POLYGON_FILTER      (pBuffer);
    DWORD uTxrShading = POLYGON_TXRSHADING  (pBuffer);
    DWORD uTexture    = POLYGON_TEXTURE     (pBuffer) ? pBuffer[3] : 0;
    if(uCullMode    !=m_uCullMode)    m_uCullMode   = uCullMode,    m_pDevice->SetRenderState(D3DRS_CULLMODE,       aCulling    [m_uCullMode]);
    if(uDepthMode   !=m_uDepthMode)   m_uDepthMode  = uDepthMode,   m_pDevice->SetRenderState(D3DRS_ZFUNC,          aDepthModes [m_uDepthMode]);
    if(uDepthWrite  !=m_uDepthWrite)  m_uDepthWrite = uDepthWrite,  m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE,   aZWrite     [m_uDepthWrite]);
    if(uSpecular    !=m_uSpecular)    m_uSpecular   = uSpecular,    m_pDevice->SetRenderState(D3DRS_SPECULARENABLE, m_uSpecular);

    //  Textures
    if(uTexture!=m_uTexture) m_uTexture = uTexture, m_pDevice->SetTexture(0,uTexture?GetTexture(pBuffer):0);
    if(uTxrShading != m_uTxrShading)
    {
      m_uTxrShading = uTxrShading;      
      m_pDevice->SetTextureStageState(0,D3DTSS_COLOROP,aColorOp[m_uTxrShading]);
      m_pDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,aAlphaOp[m_uTxrShading]);
    }

    //Mapping
    if(uUVMode!=m_uUVMode)
    {
      m_uUVMode = uUVMode;
      m_pDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,aUVMode[m_uUVMode&1]     [(m_uUVMode>>8)&1]);
      m_pDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,aUVMode[(m_uUVMode&2)>>1][((m_uUVMode>>8)&2)>>1]);
    }

    //Filltering
    if(m_uFilter!=uFilter)
    {
      m_uFilter = uFilter;
      if (m_uFilter == PVR_FILTER_POINT_SAMPLED)
      {
        m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

      }
      else
      {
        m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
      }
    }
  }
};

////////////////////////////////////////////////////////////////////////////////////////
/// @struct TTransContext
/// @brief
////////////////////////////////////////////////////////////////////////////////////////
struct TTransContext
{
  IDirect3DDevice9* m_pDevice;
  DWORD m_uCullMode;
  DWORD m_uDepthMode;
  DWORD m_uDepthWrite;
  DWORD m_uSrcBlend;
  DWORD m_uDstBlend;
  DWORD m_uSpecular;
  DWORD m_uTxrShading;
  DWORD m_uUVMode;
  DWORD m_uFilter;
  DWORD m_uTexture;

  TTransContext(IDirect3DDevice9* _pDevice)  : m_pDevice(_pDevice)
  {
    m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,      FALSE);
    m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,     TRUE);
    m_pDevice->SetRenderState(D3DRS_ZENABLE,              D3DZB_TRUE);
    m_pDevice->SetRenderState(D3DRS_ZFUNC,                D3DCMP_GREATEREQUAL);

    if(!g_bUSE_ZWRITE)
      m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE,       D3DZB_FALSE);
    else
    {
      m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE,       D3DZB_TRUE);
      if(g_bUSE_ALPHATEST_ZWRITE)
      {
        m_pDevice->SetRenderState(D3DRS_ALPHAREF,         0);
        m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  TRUE);
        m_pDevice->SetRenderState(D3DRS_ALPHAFUNC,        D3DCMP_GREATER);
      }
    }

    //  Textures
    m_pDevice->SetTexture(0,0);
    m_pDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pDevice->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    m_pDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    m_pDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    //  State Cache
    m_uCullMode   = m_uDepthMode  = m_uDepthWrite = m_uUVMode   = 0xffffffff;
    m_uSpecular   = m_uTxrShading = m_uSrcBlend   = m_uDstBlend = m_uFilter = 0xffffffff;
    m_uTexture    = 0;
  }

  void Primitive(const TRenderMode& _rMode)
  {
    //States
    const DWORD* pBuffer = _rMode.m_aCmdParams;
    DWORD uCullMode   = POLYGON_CULLING     (pBuffer);
    DWORD uSrcBlend   = POLYGON_SRCBLEND    (pBuffer);
    DWORD uDstBlend   = POLYGON_DSTBLEND    (pBuffer);
    DWORD uSpecular   = POLYGON_OFFSET_ON   (pBuffer);
    DWORD uUVMode     = POLYGON_UVCLAMP     (pBuffer) | (POLYGON_UVFLIP      (pBuffer)<<8);
    DWORD uFilter     = POLYGON_FILTER      (pBuffer);
    DWORD uTxrShading = POLYGON_TXRSHADING  (pBuffer);
    DWORD uTexture    = POLYGON_TEXTURE     (pBuffer) ? pBuffer[3] : 0;
    if(uCullMode    !=m_uCullMode)    m_uCullMode   = uCullMode,    m_pDevice->SetRenderState(D3DRS_CULLMODE,       aCulling    [m_uCullMode]);
    if(uSrcBlend    !=m_uSrcBlend)    m_uSrcBlend   = uSrcBlend,    m_pDevice->SetRenderState(D3DRS_SRCBLEND ,      aSrcBlend   [m_uSrcBlend]);
    if(uDstBlend    !=m_uDstBlend)    m_uDstBlend   = uDstBlend,    m_pDevice->SetRenderState(D3DRS_DESTBLEND,      aDstBlend   [m_uDstBlend]);
    if(uSpecular    !=m_uSpecular)    m_uSpecular   = uSpecular,    m_pDevice->SetRenderState(D3DRS_SPECULARENABLE, m_uSpecular);

    //  Textures
    if(uTexture!=m_uTexture) m_uTexture = uTexture, m_pDevice->SetTexture(0,uTexture?GetTexture(pBuffer):0);
    if(uTxrShading != m_uTxrShading)
    {
      m_uTxrShading = uTxrShading;
      m_pDevice->SetTextureStageState(0,D3DTSS_COLOROP,aColorOp[m_uTxrShading]);
      m_pDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,aAlphaOp[m_uTxrShading]);
    }

    //Mapping
    if(uUVMode!=m_uUVMode)
    {
      m_uUVMode = uUVMode;
      m_pDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,aUVMode[ m_uUVMode&1    ][(m_uUVMode>>8)&1]);
      m_pDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,aUVMode[(m_uUVMode&2)>>1][((m_uUVMode>>8)&2)>>1]);
    }

    //Filltering
    if(m_uFilter!=uFilter)
    {
      m_uFilter = uFilter;
      if (m_uFilter == PVR_FILTER_POINT_SAMPLED)
      {
        m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

      }
      else
      {
        m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
      }
    }
  }
};


////////////////////////////////////////////////////////////////////////////////////////
/// @fn      void TAStartRender()
/// @brief
////////////////////////////////////////////////////////////////////////////////////////
void TAStartRender()
{
  m_uTAFlags |= TA_FLAGS_BEGIN_RENDER;
  if (m_uTAFlags & TA_FLAGS_END_REGISTRATION)
    TADoRender();
  else
    //TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_PVR_RENDERDONE);
  {
	param.RaiseInterrupt(holly_RENDER_DONE);
	param.RaiseInterrupt(holly_RENDER_DONE_isp);
	param.RaiseInterrupt(holly_RENDER_DONE_vd);
  }
}


struct  TDrawPrimitive
{
  const TRenderMode*  m_pMode;
  TVBuffer*           m_pBuffer;
  unsigned            m_uBase;
  unsigned            m_uCount;
  float               m_fAverageZ;
  TDrawPrimitive  (){}
  TDrawPrimitive  (const TRenderMode* _pMode, TVBuffer* _pBuffer, unsigned _uBase, unsigned _uCount, float _fAverageZ=.0f)
  {
    m_pMode   = _pMode;
    m_pBuffer = _pBuffer;
    m_uBase   = _uBase;
    m_uCount  = _uCount;
    m_fAverageZ = _fAverageZ;
  }

  inline bool  operator < (const TDrawPrimitive& _other)
  {
    return m_fAverageZ<_other.m_fAverageZ;
  }
};



////////////////////////////////////////////////////////////////////////////////////////
/// @fn      void TADoRender()
/// @brief
////////////////////////////////////////////////////////////////////////////////////////
static bool s_bLost = false;
void TADoRender()
{
	VertexCount+=Unai::m_uNumVerticesRegistered ;
  DWORD uFPU_CFG = SH4HWRegistersGetValue(TPVR::FPU_PARAM_CFG);
  DWORD uISP_CFG = SH4HWRegistersGetValue(TPVR::ISP_FEED_CFG);
  DWORD uSort    = (uFPU_CFG & TPVR::FPU_PARAM_CFG_REGTYPE_MASK)>>TPVR::FPU_PARAM_CFG_REGTYPE_SHIFT;
  bool bAuto_sort;
  if (uSort == 0)
  {
    if (uISP_CFG&1)
    {
      bAuto_sort = false;
    }
    else
    {
      bAuto_sort = true;
    }
  }
  else
  {
    DWORD uRegionBase = SH4HWRegistersGetValue(TPVR::REGION_BASE);
    DWORD* pBuffer = (DWORD*)SH4GetVideoRAMPtr(uRegionBase + SH4VideoRAM_START);
    DWORD uPreSort = (pBuffer[0]>>29)&1;
    bAuto_sort = uPreSort == 0;
  }

  DWORD uAlphaRef = SH4HWRegistersGetValue(TPVR::PVR_PT_ALPHA_REF);
//SH4WriteLongMemory(TPVR::PVR_PT_ALPHA_REF, 0xff);

  //  save sse data
 // g_psh4_dynarec->SaveExtraRegs();
	SaveSSERegs();

  //  prepare d3d objects
  IDirect3DDevice9* pd3dDevice = m_DisplayDevice.GetD3DDevice();
  bool  bDrawOP = g_bDraw && !m_DisplayDevice.GetDisplayChange() && pd3dDevice;
  bool  bDrawPT = g_bDraw && !m_DisplayDevice.GetDisplayChange() && pd3dDevice;
  bool  bDrawTR = g_bDraw && !m_DisplayDevice.GetDisplayChange() && pd3dDevice;
  TVBuffer* pBufferCache = 0;
  std::vector<TDrawPrimitive> aComandsOP;
  std::vector<TDrawPrimitive> aComandsPT;
  std::vector<TDrawPrimitive> aComandsTR;
  aComandsOP.reserve(512);
  aComandsPT.reserve(128);
  aComandsTR.reserve(128);
  //  opacos
  if(bDrawOP)
  {
    for(map<TRenderMode,TRenderBatch*>::iterator it=g_aRenderList[TPVR::PVR_LIST_OP_POLY].m_BatchMap.begin();it!=g_aRenderList[TPVR::PVR_LIST_OP_POLY].m_BatchMap.end();++it)
    {
      if((it->second)->m_aBuffer.empty()) continue;
      TVBuffer* pBuffer = g_aRenderList[TPVR::PVR_LIST_OP_POLY].FlushBatch(pBufferCache, it->second);
      unsigned uBase  = pBuffer->m_uBase;
      unsigned uCount = pBuffer->m_uUsed - pBuffer->m_uBase;
      if(pBuffer!=pBufferCache)
      {
        if(pBufferCache)  pBufferCache->Close();
        pBufferCache = pBuffer;
      }
      if(pBuffer && pBuffer->m_pVB && uCount>3)
        aComandsOP.push_back(TDrawPrimitive(&(it->first),pBuffer,uBase,uCount-3));
	  else
	  {
		  __asm int 3;
	  }
    }
  }
  //  alphatest
  if(bDrawPT)
  {
    for(map<TRenderMode,TRenderBatch*>::iterator it=g_aRenderList[TPVR::PVR_LIST_PT_POLY].m_BatchMap.begin();it!=g_aRenderList[TPVR::PVR_LIST_PT_POLY].m_BatchMap.end();++it)
    {
      if((it->second)->m_aBuffer.empty()) continue;
      TVBuffer* pBuffer = g_aRenderList[TPVR::PVR_LIST_PT_POLY].FlushBatch(pBufferCache, it->second);
      unsigned uBase  = pBuffer->m_uBase;
      unsigned uCount = pBuffer->m_uUsed - pBuffer->m_uBase;
      if(pBuffer!=pBufferCache)
      {
        if(pBufferCache)  pBufferCache->Close();
        pBufferCache = pBuffer;
      }
      if(pBuffer && pBuffer->m_pVB && uCount>3)
        aComandsPT.push_back(TDrawPrimitive(&(it->first),pBuffer,uBase,uCount-3));
	  	  else
	  {
		  __asm int 3;
	  }
    }
  }
  //  trans
  if(bDrawTR)
  {
    for(map<TRenderMode,TRenderBatch*>::iterator it=g_aRenderList[TPVR::PVR_LIST_TR_POLY].m_BatchMap.begin();it!=g_aRenderList[TPVR::PVR_LIST_TR_POLY].m_BatchMap.end();++it)
    {
      if((it->second)->m_aBuffer.empty()) continue;
      TVBuffer* pBuffer = g_aRenderList[TPVR::PVR_LIST_TR_POLY].FlushBatch(pBufferCache, it->second);
      unsigned uBase  = pBuffer->m_uBase;
      unsigned uCount = pBuffer->m_uUsed - pBuffer->m_uBase;
      if(pBuffer!=pBufferCache)
      {
        if(pBufferCache)  pBufferCache->Close();
        pBufferCache = pBuffer;
      }
      if(pBuffer && pBuffer->m_pVB && uCount>3)
        aComandsTR.push_back(TDrawPrimitive(&(it->first),pBuffer,uBase,uCount-3,(it->second)->m_fAverageZ));
	  	  else
	  {
		  __asm int 3;
	  }
    }
  }
  if(pBufferCache)
  {
    pBufferCache->Close();
    pBufferCache = 0;
  }

  //  Begin scene
  bool bBegin = m_DisplayDevice.BeginScene(*(float*)SH4HWRegistersGetPtr(TPVR::PVR_BGPLANE_Z));
  if (g_bDraw && bBegin)
  {
    if (s_bLost)
    {
      s_bLost = false;
      m_CacheTextures.Init(pd3dDevice);
    }

    D3DXMATRIX  mPrjMat;
    D3DXMatrixIdentity(&mPrjMat);
    pd3dDevice->SetTransform(D3DTS_PROJECTION, &mPrjMat);
    pd3dDevice->SetRenderState(D3DRS_CLIPPING,TRUE);
    if (g_bWireframe)
      m_DisplayDevice.GetD3DDevice()->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
    else
      m_DisplayDevice.GetD3DDevice()->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);

    DrawBackground();

    pd3dDevice->SetVertexShader(NULL);
    pd3dDevice->SetFVF(MD3DFVF_TL2VERTEXT1);
    if(bDrawOP)
    {
      TOpaqueContext  context(pd3dDevice);
      for(std::vector<TDrawPrimitive>::iterator it=aComandsOP.begin();it!=aComandsOP.end();++it)
      {
        context.Primitive(*(it->m_pMode));
        if(pBufferCache!=it->m_pBuffer)
          pBufferCache = it->m_pBuffer ,pd3dDevice->SetStreamSource(0,it->m_pBuffer->m_pVB,0,sizeof(TD3DTL2VERTEXT1));
        pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, it->m_uBase,it->m_uCount);
        TRenderBatch::s_uDrawPrim += it->m_uCount;
      }
    }
    if(bDrawPT)
    {
      TPunchContext  context(pd3dDevice,uAlphaRef);
      for(std::vector<TDrawPrimitive>::iterator it=aComandsPT.begin();it!=aComandsPT.end();++it)
      {
        context.Primitive(*(it->m_pMode));
        if(pBufferCache!=it->m_pBuffer)
          pBufferCache = it->m_pBuffer ,pd3dDevice->SetStreamSource(0,it->m_pBuffer->m_pVB,0,sizeof(TD3DTL2VERTEXT1));
        pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, it->m_uBase,it->m_uCount);
        TRenderBatch::s_uDrawPrim += it->m_uCount;
      }
    }
    if(bDrawTR)
    {
      if(g_eSortMode!=E_SORT_NONE)
        std::sort(aComandsTR.begin(),aComandsTR.end());
      TTransContext  context(pd3dDevice);
      for(std::vector<TDrawPrimitive>::iterator it=aComandsTR.begin();it!=aComandsTR.end();++it)
      {
        context.Primitive(*(it->m_pMode));
        if(pBufferCache!=it->m_pBuffer)
          pBufferCache = it->m_pBuffer ,pd3dDevice->SetStreamSource(0,it->m_pBuffer->m_pVB,0,sizeof(TD3DTL2VERTEXT1));
        pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, it->m_uBase,it->m_uCount);
        TRenderBatch::s_uDrawPrim += it->m_uCount;
      }
    }
  
    m_CacheTextures.EndFrame();
    for(int i=0;i<5;++i)
      g_aRenderList[i].NewFrame();
    TRenderBatch::NewFrame();
  }

  if(!bBegin)
    s_bLost = true;

  m_DisplayDevice.EndScene();



  m_uEndFrameTimeStamp = CurrentFrame;

  DWORD uTextValue = SH4HWRegistersGetValue(0x005F8060);

  if (uTextValue<0x1000000)
  {
    DoFlip();
    m_uTAState = TA_STATE_NONE;
    m_uTAFlags = 0;
  }
  FrameCount ++;

  LoadSSERegs();
//  g_psh4_dynarec->RestoreExtraRegs();


  
//((TSH4Registers*)(g_pSH4->GetRegisters()))->uCycles=(SH4_CYCLES_PER_FRAME*500)/DC_GLOBALS::PVR2_VLINES;


  m_uRenderAddress = SH4HWRegistersGetValue(TPVR::PVR_RENDER_ADDR);

  DWORD* pVideoMem = (DWORD*) SH4GetVideoRAMPtr(SH4VideoRAM_START+m_uRenderAddress);

  //if (pVideoMem)
    //*pVideoMem = STAMP_RENDER;


//  TSH4_ASIC::EventCompleted(TSH4_ASIC::ASIC_EVT_PVR_RENDERDONE);
  render_end_pending=true;
  render_end_pending_cycles= m_uNumVerticesRegistered*25;
  if (render_end_pending_cycles<500000)
	  render_end_pending_cycles=500000;
	  /*
  RaiseInterrupt(holly_RENDER_DONE);
  RaiseInterrupt(holly_RENDER_DONE_vd);
  RaiseInterrupt(holly_RENDER_DONE_isp);*/



  if (g_bChangeDisplayEnable)
    g_bDraw = !g_bDraw;
  g_bChangeDisplayEnable = false;

  //g_pSH4->IncrementFrame();
  
//((TSH4Registers*)(g_pSH4->GetRegisters()))->uCycles=(SH4_CYCLES_PER_FRAME*400)/DC_GLOBALS::PVR2_VLINES;

}



////////////////////////////////////////////////////////////////////////////////////////
/// @fn      void DrawBackground()
/// @brief
////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  DWORD		flags1, flags2;
  DWORD		dummy;
  float		x1, y1, z1;
  DWORD		argb1;
  float		x2, y2, z2;
  DWORD		argb2;
  float		x3, y3, z3;
  DWORD		argb3;
} bkg_poly;

/*
float	x, y, z;
float u,v;
DWORD baseColour;*/
typedef struct {
  DWORD		flags1, flags2;
  DWORD		dummy;
  float		x1, y1, z1;
  float		u1,v1;
  DWORD		argb1;
  float		x2, y2, z2;
  float		u2,v2;
  DWORD		argb2;
  float		x3, y3, z3;
  float		u3,v3;
  DWORD		argb3;
} bkg_poly2;

typedef struct {
  DWORD		flags1, flags2;
  DWORD		dummy;
  float		x1, y1, z1;
  float		u1,v1;
  DWORD		argb1;
  DWORD		offset1;
  float		x2, y2, z2;
  float		u2,v2;
  DWORD		argb2;
  DWORD		offset2;
  float		x3, y3, z3;
  float		u3,v3;
  DWORD		argb3;
  DWORD		offset3;
} bkg_poly3;

float Max3(float a, float b, float c)
{
  float fMax = a;

  if (b > fMax)
    fMax = b;

  if (c > fMax)
    fMax = c;

  return fMax;
}

float Min3(float a, float b, float c)
{
  float fMin = a;

  if (b < fMin)
    fMin = b;

  if (c < fMin)
    fMin = c;

  return fMin;
}

////////////////////////////////////////////////////////////////////////////////////////
/// @fn      void DrawBackground()
/// @brief
////////////////////////////////////////////////////////////////////////////////////////
//Convert offset32 to offset64
static DWORD drkConvOffset32toOffset64(DWORD offset32)
{
		//64b wide bus is archevied by interleaving the banks every 32 bits
		//so bank is Address<<3
		//bits <4 are <<1 to create space for bank num
		//bank 1 is mapped at 400000 (32b offset) and after
		DWORD bank=((offset32>>22)&0x1)<<2;//bank will be used ass uper offset too
		DWORD lv=offset32&0x3; //these will survive
		offset32<<=1;
		//       |inbank offset    |       bank id        | lower 2 bits (not changed)
		DWORD rv=  (offset32&(VRAM_MASK-7))|bank                  | lv;
 
		return rv;
}
static void DrawBackground()
{
	return;
  DWORD uValue = SH4HWRegistersGetValue(TPVR::PVR_BGPLANE_CFG);


  DWORD uBaseAddress = ((SH4HWRegistersGetValue(TPVR::PARAM_BASE)>>20) & 0xF)<<20;

  DWORD uAddress = GETVALUEHWREGISTER(TPVR::PVR_BGPLANE_CFG,TPVR::PVR_BGPLANE_CFG_ADDR)<<2;

  DWORD uISP = GETVALUEHWREGISTER(TPVR::PVR_BGPLANE_CFG,TPVR::PVR_BGPLANE_CFG_ISP);

  uAddress+=uBaseAddress;


  DWORD color;
  D3DXVECTOR2 v0,v1;
  D3DXVECTOR2 uv0,uv1;

//TD3DTLVERTEXT1 aVerts[4];

  IDirect3DTexture9* pTextureD3D = NULL;

  float zValue;

  BYTE pDataVideo_[256];// = (BYTE* ) SH4GetVideoRAMPtr(uAddress + SH4VideoRAM_START);
  for (int i=-128;i<128;i++)
  {
		pDataVideo_[i+128]=*SH4GetVideoRAMPtr(drkConvOffset32toOffset64(uAddress+i));
  }
BYTE*pDataVideo=&pDataVideo_[128];
  if (!pDataVideo)
    return;


  if (uISP == 1)
  {
    bkg_poly* pBkg = (bkg_poly*) pDataVideo;

  //v0.x = (pBkg->x1*m_fXCoorRatio)-0.5f; v0.y = (pBkg->y1*m_fYCoorRatio)-0.5f;
  //v1.x = (pBkg->x2*m_fXCoorRatio)-0.5f; v1.y = (pBkg->y3*m_fYCoorRatio)-0.5f;

    color = pBkg->argb1;

    //color = 0xffffffff;
    /*v0.x = 0.f; v0.y = 0.f;
    v1.x = 640.f; v1.y= 480.f;*/

    float fMaxX = Max3(pBkg->x1,pBkg->x2,pBkg->x3);
    float fMaxY = Max3(pBkg->y1,pBkg->y2,pBkg->y3);

    float fMinX = Min3(pBkg->x1,pBkg->x2,pBkg->x3);
    float fMinY = Min3(pBkg->y1,pBkg->y2,pBkg->y3);

    v0.x = (fMinX*m_fXCoorRatio)-0.5f; v0.y = (fMinY*m_fYCoorRatio)-0.5f;
    v1.x = (fMaxX*m_fXCoorRatio)-0.5f; v1.y = (fMaxY*m_fYCoorRatio)-0.5f;

/*
    aVerts[0].x = pBkg->x1; aVerts[0].y = pBkg->y1; aVerts[0].z = pBkg->z1;
    aVerts[1].x = pBkg->x2; aVerts[1].y = pBkg->y2; aVerts[1].z = pBkg->z2;
    aVerts[2].x = pBkg->x3; aVerts[2].y = pBkg->y3; aVerts[2].z = pBkg->z3;
    aVerts[0].oow = aVerts[1].oow = aVerts[2].oow = 1.f;
    aVerts[0].uiRGBA = pBkg->argb1;
    aVerts[1].uiRGBA = pBkg->argb2;
    aVerts[2].uiRGBA = pBkg->argb3;
*/
    zValue = pBkg->z1;

    uv0.x = 0.f; uv0.y = 0.f;
    uv1.x = 0.f; uv1.y = 0.f;
  }
  else if (uISP == 3)
  {
    bkg_poly2* pBkg = (bkg_poly2*) pDataVideo;

    DWORD* pBuffer = (DWORD*) (DWORD*)  pDataVideo-4;;;
    //pBuffer = pBuffer -1;

    pTextureD3D = GetTexture(pBuffer);

    v0.x = (pBkg->x1*m_fXCoorRatio)-0.5f; v0.y = (pBkg->y1*m_fYCoorRatio)-0.5f;
    v1.x = (pBkg->x3*m_fXCoorRatio)-0.5f; v1.y = (pBkg->y3*m_fYCoorRatio)-0.5f;
    
    uv0.x = pBkg->u1; uv0.y = pBkg->v1;
    uv1.x = pBkg->u3; uv1.y = pBkg->v3;

    color = pBkg->argb1;

    zValue = pBkg->z1;
  }
  else if (uISP == 4)
  {
    bkg_poly3* pBkg = (bkg_poly3*) pDataVideo;

    DWORD* pBuffer = (DWORD*)  pDataVideo-4;;
    //pBuffer = pBuffer -1;

    pTextureD3D = GetTexture(pBuffer);

    v0.x = (pBkg->x1*m_fXCoorRatio)-0.5f; v0.y = (pBkg->y1*m_fYCoorRatio)-0.5f;
    v1.x = (pBkg->x3*m_fXCoorRatio)-0.5f; v1.y = (pBkg->y3*m_fYCoorRatio)-0.5f;

    uv0.x = pBkg->u1; uv0.y = pBkg->v1;
    uv1.x = pBkg->u3; uv1.y = pBkg->v3;

    color = pBkg->argb1;

    zValue = pBkg->z1;
  }
  else
  {
    ASSERT(false);
    return;
  }


  TD3DTLVERTEXT1 v[4];

  // 20
  // 31

  v[0].x = v1.x;   v[0].y = v0.y;
  v[0].z = zValue;    v[0].oow = 1.f;
  v[0].uiRGBA = color;
  v[0].u = uv1.x; v[0].v = uv0.y;


  v[1].x = v1.x;   v[1].y = v1.y;
  v[1].z = zValue;    v[1].oow = 1.f;
  v[1].uiRGBA = color;
  v[1].u = uv1.x; v[1].v = uv1.y;

  v[2].x = v0.x;   v[2].y = v0.y;
  v[2].z = zValue;    v[2].oow = 1.f;
  v[2].uiRGBA = color;
  v[2].u = uv0.x; v[2].v = uv0.y;

  v[3].x = v0.x;   v[3].y = v1.y;
  v[3].z = zValue;    v[3].oow = 1.f;
  v[3].uiRGBA = color;
  v[3].u = uv0.x; v[3].v = uv1.y;


  IDirect3DDevice9* pd3dDevice = m_DisplayDevice.GetD3DDevice();

  pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
  pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
  pd3dDevice->SetVertexShader(NULL);
  pd3dDevice->SetTexture(0, pTextureD3D);
  pd3dDevice->SetFVF(MD3DFVF_TLVERTEXT1);
  pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
  pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
  pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);


  pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,v,sizeof(TD3DTLVERTEXT1));

  //pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,1,aVerts,sizeof(TD3DTLVERTEXT1));
}


bool TAIsOldGfx()
{
  return false;
}

}
extern char fps_text_wkl[512];
void DrawFpsText(char*str)
{
	strcpy(fps_text_wkl,str);
}