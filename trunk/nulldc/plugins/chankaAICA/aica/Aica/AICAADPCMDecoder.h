////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICAADPCMDecoder.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef AICAADPCM_DECODER_H_
#define AICAADPCM_DECODER_H_

template<typename T>
class CAICAADPCMDecoder
{
public:
          CAICAADPCMDecoder   () : m_bInit(false) {}
          ~CAICAADPCMDecoder  ()                  { End(); }

  TError  Init                ();
  void    End                 ();

  int     GetDestSamplesSize  () const;
  int     DecodeSample        (T* pDecodedSample, const BYTE ucCurrentData);

  void    Seek                (int iSamplePos) { m_iBytePos = (iSamplePos+1)&0x1; }

  //void    Decode              (T* pDecodedData);  // test function
  //TError  Init                (const void* pData, int iSizeBytes); // test init

private:
  bool          m_bInit;
  const void*   m_pSrcData; 
  int           m_iSrcSizeBytes;

  int           m_iBytePos;
  float         m_fLastValue;
  float         m_fQuant;
};

#include "AICAADPCMDecodercpp.h"

#endif AICAADPCM_DECODER_H_
