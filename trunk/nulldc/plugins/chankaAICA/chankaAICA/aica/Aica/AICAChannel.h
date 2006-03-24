////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICAChannel.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef AICACHANNEL_H_
#define AICACHANNEL_H_

#include "AICAADPCMDecoder.h"


enum ESampleDataType
{
  SAMPLEDATA_TYPE_PCM_16 = 0,
  SAMPLEDATA_TYPE_PCM_8,
  SAMPLEDATA_TYPE_ADPCM,
  SAMPLEDATA_TYPE_ADPCM_LOOP
};

template <typename T>
class CAICAChannel
{
public:
  // ADPCM
  struct TADPCMData
  {
    /*T*      m_pData;
    int     m_iSize;    // Size in samples*/

    inline void Reset    ()
    {
      m_iLastDecodedPos = -1;
      m_iCurrentByteToDecode = 0;
      m_Decoder.Init();
    }

    CAICAADPCMDecoder<T>  m_Decoder;
    int                   m_iLastDecodedPos;
    T                     m_LastDecodedValue;
    int                   m_iCurrentByteToDecode;
  };

                    CAICAChannel        () : m_bInit(false) {}
                    ~CAICAChannel       ()                  { End(); }

  TError            Init                ();
  void              End                 ();

  bool              IsOn                () const            { return m_bOn; }
  void              SetOn               (void *pData, ESampleDataType eType, int iSamplesSize, int iLoopIni, float fPitchPercent, int iVolume, int iPan);
  void              Stop                ();

  inline ESampleDataType   GetSampleDataType   () const            { return m_Type; }
  inline DWORD             GetCurrentPlayPos   () const            { return m_uPlayPos; }
  inline void              SetCurrentPlayPos   (DWORD uPos)        { m_uPlayPos = uPos; }
  inline DWORD             GetCountPos         () const            { return m_uCountPos; }
  inline void              SetCountPos         (DWORD uPos)        { m_uCountPos = uPos; }
  inline DWORD             GetSize             () const            { return m_uSize; }
  inline DWORD             GetLoopIni          () const            { ASSERT(LoopOn()); return (DWORD)m_iLoopIni; }
  inline bool              LoopOn              () const            { return m_iLoopIni >= 0; }
  inline void*             GetData             () const            { return m_pData; }
  inline DWORD             GetSpeed            () const            { return m_uSpeed; }
  inline int               GetVolume           () const            { return m_iVolume; }
  inline void              SetVolume           (int iVolume);
  inline int               GetPanL             () const            { return m_aiPan[0]; }
  inline int               GetPanR             () const            { return m_aiPan[1]; }
  inline void              SetPan              (int iPan);
  inline void              SetPitchPercent     (float fPitch);
  inline TADPCMData*       GetADPCMData        ()                  { return &m_ADPCMData; }

  inline const T&          GetLast16BitValue   () const            { return m_Last16BitValue; }
  inline void              SetLast16BitValue   (const T& Val)      { m_Last16BitValue = Val; }

private:
  bool              m_bInit;
  bool              m_bOn;
  void*             m_pData;
  ESampleDataType   m_Type;
  int               m_iLoopIni;
  DWORD             m_uPlayPos;
  DWORD             m_uCountPos;
  DWORD             m_uSize;
  DWORD             m_uSpeed;
  int               m_iVolume;
  int               m_aiPan[2];     // Volume for left&right

  TADPCMData        m_ADPCMData;
  T                 m_Last16BitValue;
};

#include "AICAChannelcpp.h"

#endif AICACHANNEL_H_
