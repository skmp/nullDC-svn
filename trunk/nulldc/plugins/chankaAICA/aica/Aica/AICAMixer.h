////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICAMixer.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef AICAMIXER_H_
#define AICAMIXER_H_

#include "AICAChannel.h"
#include "AICADSP.h"

const int AICAMIXER_CHANNELS = 64;
const int AICAMIXER_MIXBUFFERSSIZE = 1024;    // Size in samples

template <typename T>
class CAICAMixer
{
public:
          CAICAMixer          () : m_bInit(false) {}
          ~CAICAMixer         ()                  { End(); }

  // Init/End
  TError  Init                ();
  void    End                 ();

  // Process
  void    Process             (int iSamples, signed short* pOutput);

  // Play/Stop Channels
  void    PlayChannel         (int iChannel, void* pData, ESampleDataType eType, int iSamplesSize, int iLoopIni, float fPitchPercent, int iVolume, int iPan);
  void    StopChannel         (int iChannel);
  void    SetPitchChannel     (int iChannel, float fPitchChannel);
  void    SetVolumeChannel    (int iChannel, int iVolume);
  void    SetPanChannel       (int iChannel, int iPan);
  int     GetCurrentPosChannel(int iChannel) const;

private:
  bool    ProcessChannel      (int iChannel, int iSamples, bool bAdd);
  /*bool    Process16BitChannel (CAICAChannel<T>* pChannel, int iSamples, T* pDest,bool bAdd);
  bool    Process8BitChannel  (CAICAChannel<T>* pChannel, int iSamples, T* pDest,bool bAdd);
  bool    ProcessADPCMChannel (CAICAChannel<T>* pChannel, int iSamples, T* pDest,bool bAdd);*/

  bool                                    m_bInit;
  CAICAChannel<T>                         m_aChannels[AICAMIXER_CHANNELS];
  CAICADSP<T,AICAMIXER_MIXBUFFERSSIZE*2>  m_DSP;
  T                                       m_aOutputMix[AICAMIXER_MIXBUFFERSSIZE*2];
};

#include "AICAMixercpp.h"

#endif AICAMIXER_H_
