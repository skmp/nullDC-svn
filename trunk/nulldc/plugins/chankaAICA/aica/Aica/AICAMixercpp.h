////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICAMixercpp.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////

#include "AICAMixChannels.h"

////////////////////////////////////////////////////////////////////////////////////////
// Init()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
TError CAICAMixer<T>::Init()
{
  TError Error = RET_OK;

  End();

  for ( int j = 0 ; j < AICAMIXER_CHANNELS && Error == RET_OK ; ++j )
    Error = m_aChannels[i].Init();

  m_bInit = true;

  return Error;
}


////////////////////////////////////////////////////////////////////////////////////////
// End()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMixer<T>::End()
{
  if ( m_bInit )
  {
    m_bInit = false;
  }
}


////////////////////////////////////////////////////////////////////////////////////////
// Process()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMixer<T>::Process(int iSamplesFaltan, signed short* pOutput)
{

#ifdef _SAVE_AICA_DEBUG_FILES_
  static FILE* s_f = 0;
  if ( !s_f )
    s_f = fopen("d:\\AICAMixerProcess.raw","wb");
#endif

  while ( iSamplesFaltan )
  {
    int iSamples = iSamplesFaltan;
    if ( iSamples > AICAMIXER_MIXBUFFERSSIZE)
      iSamples = AICAMIXER_MIXBUFFERSSIZE;

    bool bAdd = false;
    for ( int j = 0 ; j < AICAMIXER_CHANNELS ; ++j )
    {
      if ( ProcessChannel(j,iSamples,bAdd) )
        bAdd = true;
    }

    if ( !bAdd )
      memset(m_aOutputMix,0,iSamples*2*sizeof(T));

    for ( int i = 0 ; i < (int)iSamples*2 ; ++i )
    {
      T Value = m_aOutputMix[i];
      if ( Value > (T)32767 )
        Value = (T)32767;
      else if ( Value < (T)-32768 )
        Value = (T)-32768;

      pOutput[i] = (signed short)Value;

#ifdef _SAVE_AICA_DEBUG_FILES_
      fwrite(&pOutput[i],2,1,s_f);
#endif
    }
    pOutput += iSamples*2;

    iSamplesFaltan -= iSamples;
  }

#ifdef _SAVE_AICA_DEBUG_FILES_
  fflush(s_f);
#endif
}


////////////////////////////////////////////////////////////////////////////////////////
// PlayChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMixer<T>::PlayChannel(int iChannel, void* pData, ESampleDataType eType, int iSamplesSize, int iLoopIni, float fPitchPercent, int iVolume, int iPan)
{
  ASSERT(iChannel<AICAMIXER_CHANNELS);
  m_aChannels[iChannel].SetOn(pData,eType,iSamplesSize,iLoopIni,fPitchPercent,iVolume,iPan);
  /*char szTemp[256];
  sprintf(szTemp,"SetOn(%i)(%i)\n",iChannel,iVolume);
  OutputDebugString(szTemp);*/
}

////////////////////////////////////////////////////////////////////////////////////////
// StopChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMixer<T>::StopChannel(int iChannel)
{
  ASSERT(iChannel<AICAMIXER_CHANNELS);
  m_aChannels[iChannel].Stop();
  /*char szTemp[256];
  sprintf(szTemp,"Stop(%i)\n",iChannel);
  OutputDebugString(szTemp);*/
}


////////////////////////////////////////////////////////////////////////////////////////
// SetPitchChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMixer<T>::SetPitchChannel(int iChannel, float fPitchChannel)
{
  ASSERT(iChannel<AICAMIXER_CHANNELS);
  m_aChannels[iChannel].SetPitchPercent(fPitchChannel);
}

////////////////////////////////////////////////////////////////////////////////////////
// SetVolumeChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMixer<T>::SetVolumeChannel(int iChannel, int iVolume)
{
  ASSERT(iChannel<AICAMIXER_CHANNELS);
  m_aChannels[iChannel].SetVolume(iVolume);
  /*char szTemp[256];
  sprintf(szTemp,"Volume(%i)(%i)\n",iChannel,iVolume);
  OutputDebugString(szTemp);*/
}

////////////////////////////////////////////////////////////////////////////////////////
// SetPanChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMixer<T>::SetPanChannel(int iChannel, int iPan)
{
  ASSERT(iChannel<AICAMIXER_CHANNELS);
  m_aChannels[iChannel].SetPan(iPan);
}


////////////////////////////////////////////////////////////////////////////////////////
// GetCurrentPosChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
int CAICAMixer<T>::GetCurrentPosChannel(int iChannel) const
{
  ASSERT(iChannel<AICAMIXER_CHANNELS);
  if ( m_aChannels[iChannel].IsOn())
    return m_aChannels[iChannel].GetCurrentPlayPos();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// ProcessChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
bool CAICAMixer<T>::ProcessChannel(int iChannel, int iSamples, bool bAdd)
{
  ASSERT(iChannel<AICAMIXER_CHANNELS);
  CAICAChannel<T>* pChannel = &m_aChannels[iChannel];

  bool bFilled = false;
  if ( pChannel->IsOn() )
  {
    switch(pChannel->GetSampleDataType())
    {
      case SAMPLEDATA_TYPE_PCM_16:
        bFilled = MixChannel(pChannel,iSamples,m_aOutputMix,bAdd,MacroMixType16BitChannel());
        break;
      case SAMPLEDATA_TYPE_PCM_8:
        bFilled = MixChannel(pChannel,iSamples,m_aOutputMix,bAdd,MacroMixType8BitChannel());
        break;
      case SAMPLEDATA_TYPE_ADPCM:
       /* if ( pChannel->LoopOn() )
          ASSERT(!"Not tested.");*/
        bFilled = MixChannel(pChannel,iSamples,m_aOutputMix,bAdd,MacroMixTypeADPCMChannel());
        break;
      case SAMPLEDATA_TYPE_ADPCM_LOOP:
        bFilled = MixChannel(pChannel,iSamples,m_aOutputMix,bAdd,MacroMixTypeADPCMLoopChannel());
        break;
    }
    //OutputDebugString("\n");
  }
  return bFilled;
}

/*////////////////////////////////////////////////////////////////////////////////////////
// Process16BitChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
bool CAICAMixer<T>::Process16BitChannel(CAICAChannel<T>* pChannel, int iSamples, T* pDest, bool bAdd)
{
  ASSERT(false);
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////
// Process8BitChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
bool CAICAMixer<T>::Process8BitChannel(CAICAChannel<T>* pChannel, int iSamples, T* pDest, bool bAdd)
{
  int iSamplesToFill = iSamples;

  while ( iSamples > 0 )
  {
    DWORD uPlayPos = pChannel->GetCurrentPlayPos();
    DWORD uEndPos = pChannel->GetSize();
    DWORD uCountPos = pChannel->GetCountPos();
    DWORD uSpeed = pChannel->GetSpeed();
    T Volume = (T) pChannel->GetVolume() * 0.2f;

    while ( uPlayPos < uEndPos && iSamples > 0 )
    {
      T vLeft = (T)((signed char*)pChannel->GetData())[uPlayPos] * Volume;
      T vRight = (T)((signed char*)pChannel->GetData())[uPlayPos] * Volume;

      if ( !bAdd )
      {
        pDest[0] = vLeft;
        pDest[1] = vRight;
      }
      else
      {
        pDest[0] += vLeft;
        pDest[1] += vRight;
      }
      pDest += 2;

      uCountPos += uSpeed;
      uPlayPos += (uCountPos>>18);
      uCountPos = (uCountPos&0x3FFFF);

      iSamples--;
    }

    pChannel->SetCurrentPlayPos(uPlayPos);
    pChannel->SetCountPos(uCountPos);
    bool bStopped = false;
    if ( uPlayPos >= uEndPos )
    {
      if ( pChannel->LoopOn()  )
      {
        uPlayPos = pChannel->GetLoopIni();
        pChannel->SetCurrentPlayPos(uPlayPos);
      }
      else
      {
        pChannel->Stop();
        bStopped = true;
      }
    }

    // End of sample, fill with 0s the remainded output samples
    if ( iSamples && bStopped )
    {
      if ( !bAdd )
        memset(pDest,0,iSamples*2*sizeof(T));
      iSamples = 0;
    }
  }

  return iSamplesToFill != 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// ProcessADPCMChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
bool CAICAMixer<T>::ProcessADPCMChannel(CAICAChannel<T>* pChannel, int iSamples, T* pDest, bool bAdd)
{
  ASSERT(false);
  return false;
}*/

