////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICAADPCMDecoder.cpp
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "AICAADPCMDecoder.h"


////////////////////////////////////////////////////////////////////////////////////////
// Init()
////////////////////////////////////////////////////////////////////////////////////////
/*template<typename T>
TError CAICAADPCMDecoder<T>::Init(const void* pData, int iSizeBytes)
{
  TError Error = RET_OK;

  End();

  m_bInit = true;
  m_pSrcData = pData;
  m_iSrcSizeBytes = iSizeBytes/2;
  m_iBytePos = 0;
  m_fLastValue = 0.f;
  m_fQuant = 127.f;

  if ( Error != RET_OK )
    End();

  return Error;
}*/

////////////////////////////////////////////////////////////////////////////////////////
// Init()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
TError CAICAADPCMDecoder<T>::Init()
{
  TError Error = RET_OK;

  End();

  m_bInit = true;
  m_iBytePos = 0;
  m_fLastValue = 0.f;
  m_fQuant = 127.f;

  if ( Error != RET_OK )
    End();

  return Error;
}


////////////////////////////////////////////////////////////////////////////////////////
// End()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAADPCMDecoder<T>::End()
{
  if ( m_bInit )
  {
    m_bInit = false;
  }
}


////////////////////////////////////////////////////////////////////////////////////////
// GetDestSamplesSize()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
int CAICAADPCMDecoder<T>::GetDestSamplesSize() const
{
  return m_iSrcSizeBytes*2;
}


////////////////////////////////////////////////////////////////////////////////////////
// DecodeSample()
////////////////////////////////////////////////////////////////////////////////////////
template<>
int CAICAADPCMDecoder<float>::DecodeSample(float* pDecodedSample, const BYTE ucCurrentData)
{
  BYTE cValue = (ucCurrentData>>(m_iBytePos*4))&0xF;

  float fNewValue = m_fQuant/8.f;
  if ( cValue&1 )
    fNewValue += m_fQuant/4.f;
  if ( cValue&2 )
    fNewValue += m_fQuant/2.f;
  if ( cValue&4 )
    fNewValue += m_fQuant;
  if ( cValue&8 )
    fNewValue = -fNewValue;

  m_fLastValue += fNewValue;
  if ( m_fLastValue > 32767.f )
    m_fLastValue = 32737.f;
  else if ( m_fLastValue < -32768.f )
    m_fLastValue = -32768.f;

  float aTableQuant[8] = { 0.8984375f, 0.8984375f, 0.8984375f, 0.8984375f, 1.19921875f, 1.59765625f, 2.0f, 2.3984375f };
  m_fQuant = (float)((int)(aTableQuant[cValue&0x7] * m_fQuant));
  if ( m_fQuant < 127.f )
    m_fQuant = 127.f;
  if ( m_fQuant > 24576.f )
    m_fQuant = 24576.f;

  *pDecodedSample = m_fLastValue;

  m_iBytePos = (m_iBytePos+1)&0x1;
  return m_iBytePos == 0 ? 1 : 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// Decode()
////////////////////////////////////////////////////////////////////////////////////////
/*template<>
void CAICAADPCMDecoder<float>::Decode(float* pDecodedData)
{
  FILE* fdest = fopen("d:\\chanka5.raw","wb");

  int iBytes = m_iSrcSizeBytes;
  BYTE* paSrcData = (BYTE*)m_pSrcData;
  while ( iBytes-- )
  {
    for ( int i = 0 ; i < 2 ; ++i )
    {
      DecodeSample(pDecodedData,*paSrcData);

      signed short sValue = (signed short)*pDecodedData;
      fwrite(&sValue,2,1,fdest);

      pDecodedData++;
    }
    paSrcData++;
  }

  fclose(fdest);
}*/
