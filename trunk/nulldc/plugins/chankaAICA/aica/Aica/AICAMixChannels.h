////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICAMiChannels.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef AICAMIXCHANNELS_H_
#define AICAMIXCHANNELS_H_


////////////////////////////////////////////////////////////////////////////////////////
// MIX DATA TYPES
class MacroMixType16BitChannel      {};
class MacroMixType8BitChannel       {};
class MacroMixTypeADPCMChannel      {};
class MacroMixTypeADPCMLoopChannel  {};
// ADD TYPES
class MacroAddTypeSet   {};
class MacroAddTypeAdd   {};


////////////////////////////////////////////////////////////////////////////////////////
// ADD TYPES FUNCTIONS
template<typename T,typename ADDTYPE>
void MacroMixFncAdd(T* pOutput, const T* pInput, const ADDTYPE&);

template<typename T>
inline void MacroMixFncAdd(T* pOutput, const T* pInput, const MacroAddTypeSet&)
{
  pOutput[0] = pInput[0];
}

template<typename T>
inline void MacroMixFncAdd(T* pOutput, const T* pInput, const MacroAddTypeAdd&)
{
  pOutput[0] += pInput[0];
}

////////////////////////////////////////////////////////////////////////////////////////
// FILL REMAIN0s FUNCTIONS
template<typename T,typename ADDTYPE>
void FillRemain0Samples(T* pDest,int iBytes, const ADDTYPE&);

template<typename T>
inline void FillRemain0Samples(T* pDest,int iBytes, const MacroAddTypeSet&)
{
  memset(pDest,0,iBytes);
}

template<typename T>
inline void FillRemain0Samples(T* pDest,int iBytes, const MacroAddTypeAdd&)
{
}

////////////////////////////////////////////////////////////////////////////////////////
// DATA TYPE CONVERSION FUNCTIONS
template<typename T, typename DATATYPE>
void ConvertDataType( T* pDest, CAICAChannel<T>* pChannel, DWORD uPos, const DATATYPE&);

template<typename T>
inline void ConvertDataType( T* pDest, CAICAChannel<T>* pChannel, DWORD uPos, const MacroMixType8BitChannel&)
{
  pDest[0] = (T)((signed char*)pChannel->GetData())[uPos] * (T)256;
}

template<typename T>
inline void ConvertDataType( T* pDest, CAICAChannel<T>* pChannel, DWORD uPos, const MacroMixType16BitChannel&)
{
  signed short vValue = ((signed short*)pChannel->GetData())[uPos];
  if ( vValue == -32768 )
    pDest[0] = pChannel->GetLast16BitValue();
  else
    pDest[0] = (T)vValue;
  pChannel->SetLast16BitValue(pDest[0]);
}

template<typename T>
inline void ConvertDataType( T* pDest, CAICAChannel<T>* pChannel, DWORD uPos, const MacroMixTypeADPCMChannel&)
{
  CAICAChannel<T>::TADPCMData* pADPCMData = pChannel->GetADPCMData();

  if ( pADPCMData->m_iLastDecodedPos > (int)uPos )
  {
    ASSERT(pADPCMData->m_iLastDecodedPos == pChannel->GetSize()-1); // by now only pitch < 200% is supported
    ASSERT((int)uPos == pChannel->GetLoopIni());

    pADPCMData->m_iLastDecodedPos = (int)pChannel->GetLoopIni()-1;
    pADPCMData->m_iCurrentByteToDecode = (pADPCMData->m_iLastDecodedPos+1)/2;
    pADPCMData->m_Decoder.Seek(pADPCMData->m_iLastDecodedPos);
  }

  while ( pADPCMData->m_iLastDecodedPos < (int)uPos )
  {
    pADPCMData->m_iCurrentByteToDecode += pADPCMData->m_Decoder.DecodeSample(&pADPCMData->m_LastDecodedValue,((unsigned char*)pChannel->GetData())[pADPCMData->m_iCurrentByteToDecode]);
    pADPCMData->m_iLastDecodedPos++;
  }
  pDest[0] = pADPCMData->m_LastDecodedValue;
}

template<typename T>
inline void ConvertDataType( T* pDest, CAICAChannel<T>* pChannel, DWORD uPos, const MacroMixTypeADPCMLoopChannel&)
{
  CAICAChannel<T>::TADPCMData* pADPCMData = pChannel->GetADPCMData();

  if ( pADPCMData->m_iLastDecodedPos > (int)uPos )
  {
    ASSERT(pADPCMData->m_iLastDecodedPos == pChannel->GetSize()-1); // by now only pitch < 200% is supported
    ASSERT((int)uPos == pChannel->GetLoopIni());

    pADPCMData->m_iLastDecodedPos = (int)pChannel->GetLoopIni()-1;
    pADPCMData->m_iCurrentByteToDecode = (pADPCMData->m_iLastDecodedPos+1)/2;
    pADPCMData->m_Decoder.Seek(pADPCMData->m_iLastDecodedPos);
  }

  while ( pADPCMData->m_iLastDecodedPos < (int)uPos )
  {
    pADPCMData->m_iCurrentByteToDecode += pADPCMData->m_Decoder.DecodeSample(&pADPCMData->m_LastDecodedValue,((unsigned char*)pChannel->GetData())[pADPCMData->m_iCurrentByteToDecode]);
    pADPCMData->m_iLastDecodedPos++;
  }
  pDest[0] = pADPCMData->m_LastDecodedValue;
}

////////////////////////////////////////////////////////////////////////////////////////
// MixChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T,typename DATATYPE>
inline bool MixChannel(CAICAChannel<T>* pChannel, int iSamples, T* pDest, bool bAdd, const DATATYPE& DataType )
{
  bool bFilled = false;
  
  if ( bAdd )
    bFilled = MixChannelWithMacros(pChannel,iSamples,pDest,DataType,MacroAddTypeAdd());
  else
    bFilled = MixChannelWithMacros(pChannel,iSamples,pDest,DataType,MacroAddTypeSet());
  
  return bFilled;
}


////////////////////////////////////////////////////////////////////////////////////////
// MixChannelWithMacros()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T,typename DATATYPE, typename ADDTYPE>
inline bool MixChannelWithMacros(CAICAChannel<T>* pChannel, int iSamples, T* pDest, const DATATYPE& DataType, const ADDTYPE& AddType)
{
  int iSamplesToFill = iSamples;

  while ( iSamples > 0 )
  {
    DWORD uPlayPos = pChannel->GetCurrentPlayPos();
    DWORD uEndPos = pChannel->GetSize();
    DWORD uCountPos = pChannel->GetCountPos();
    DWORD uSpeed = pChannel->GetSpeed();
    T VolumeL = (T)pChannel->GetVolume() * 0.002f; // 0.002f = temporal global amplification
    T VolumeR = VolumeL;

    VolumeL *= (T)pChannel->GetPanL() / (T)255;
    VolumeR *= (T)pChannel->GetPanR() / (T)255;

    while ( uPlayPos < uEndPos && iSamples > 0 )
    {
      T vLeft; ConvertDataType(&vLeft,pChannel,uPlayPos,DataType);
      T vRight = vLeft;

      vLeft *= VolumeL;
      vRight *= VolumeR;

      MacroMixFncAdd(&pDest[0],&vLeft,AddType);
      MacroMixFncAdd(&pDest[1],&vRight,AddType);
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

    // End of sample, fill with 0s the remainded output samples (only when no add!)
    if ( iSamples && bStopped )
    {
      FillRemain0Samples(pDest,iSamples*2*sizeof(T),AddType);
      iSamples = 0;
    }
  }

  return iSamplesToFill != 0;
}




#endif AICAMIXCHANNELS_H_
