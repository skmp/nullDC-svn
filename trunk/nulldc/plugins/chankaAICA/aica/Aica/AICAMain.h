////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICAMain.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef AICAMAIN_H_
#define AICAMAIN_H_


#include "AICAGlobals.h"
#include "AICAMixer.h"
#include <list>

class IAICASndDriver;


const int AICA_MAX_SIZE_SAMPLES_BUFFERPROCESSED = 441*2;  // 20ms


struct TBufferProcessed
{
  TBufferProcessed  () : m_iSamplesProcessed(0), m_pNext(NULL) {}

  int                 m_iSamplesProcessed;
  signed short        m_aData[AICA_MAX_SIZE_SAMPLES_BUFFERPROCESSED*2];
  TBufferProcessed*   m_pNext;

  int             GetFreeSamples    ()    { ASSERT(m_iSamplesProcessed <= AICA_MAX_SIZE_SAMPLES_BUFFERPROCESSED); return AICA_MAX_SIZE_SAMPLES_BUFFERPROCESSED-m_iSamplesProcessed; }
};


enum EAICAMixMode
{
  E_AICA_MIXMODE_REAL_EMULATION = 0,
  E_AICA_MIXMODE_INTELLIGENT_EMULATION,
};

template <typename T>
class CAICAMain
{
public:
          CAICAMain         () : m_bInit(false) {}
          ~CAICAMain        ()                  { End(); }

  // Init/End
  TError  Init              (IAICASndDriver* pSndDriver, EAICAMixMode eMode);
  void    End               ();

  // Play/Stop/etc Channels
  void    PlayChannel           (int iChannel, void* pData, ESampleDataType eType, int iSamplesSize, int iLoopIni, float fPitchPercent, int iVolume, int iPan);
  void    StopChannel           (int iChannel);
  void    SetPitchChannel       (int iChannel, float fPitchPercent);
  void    SetVolumeChannel      (int iChannel, int iVolume);
  void    SetPanChannel         (int iChannel, int iPan);
  int     GetCurrentPosChannel  (int iChannel) const;

  void    GetNewBlockData   (signed short* paData);
  void    ProcessSync       (int iSamples);
  void    SetBlockDataSize  (int iSamples);

  double  GetMixVelocity    ();

private:
  TBufferProcessed*     ExtractBufferProcessed  ();
  void                  InsertBufferProcessed   (TBufferProcessed* pBuffer);
  TBufferProcessed*     ExtractBufferFree       ();
  void                  InsertBufferFree        (TBufferProcessed* pBuffer);
  void                  LockBuffersProcessed    ();
  void                  UnlockBuffersProcessed  ();
  void                  LockBuffersFree         ();
  void                  UnlockBuffersFree       ();

  bool                  m_bInit;
  CAICAMixer<T>         m_Mixer;
  IAICASndDriver*       m_pSndDriver;

  std::list<TBufferProcessed*>  m_BuffersProcessed;
  TBufferProcessed*             m_pCurrentBufferProcessed;
  TBufferProcessed*             m_pBuffersProcessed;
  TBufferProcessed*             m_pBuffersFree;
  int                           m_iBlockDataSize;
  CRITICAL_SECTION              m_CSBuffersProcessed;
  CRITICAL_SECTION              m_CSBuffersFree;

  EAICAMixMode                  m_eMode;
  double                        m_dIdealVelocity;
};

#include "AICAMaincpp.h"

#endif AICAMAIN_H_
