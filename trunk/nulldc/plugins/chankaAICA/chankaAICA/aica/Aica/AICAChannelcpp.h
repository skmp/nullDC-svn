////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICAChannelcpp.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
// Log2Linear()
////////////////////////////////////////////////////////////////////////////////////////
static int Log2Linear(float fVolume)
{
  float fVolumeNew = pow(10.f,fVolume/20.f);
  return (int)(fVolumeNew*255.f);
}

////////////////////////////////////////////////////////////////////////////////////////
// Log2Linear8()
////////////////////////////////////////////////////////////////////////////////////////
static int Log2Linear8(int iVolume)
{
  BYTE uByte = (BYTE)(iVolume&0xFF);
  float fVolume = 0.f;
  if ( uByte&1 )
    fVolume += -0.4f;
  if ( uByte&2 )
    fVolume += -0.8f;
  if ( uByte&4 )
    fVolume += -1.5f;
  if ( uByte&8 )
    fVolume += -3.f;
  if ( uByte&16)
    fVolume += -6.f;
  if ( uByte&32)
    fVolume += -12.f;
  if ( uByte&64)
    fVolume += -24.f;
  if ( uByte&128)
    fVolume += -48.f;

  return Log2Linear(fVolume);
}

////////////////////////////////////////////////////////////////////////////////////////
// Log2Linear4()
////////////////////////////////////////////////////////////////////////////////////////
static int Log2Linear4(int iVolume)
{
  BYTE uByte = (BYTE)(iVolume&0xFF);
  float fVolume = 0.f;
  if ( uByte&1 )
    fVolume += -0.4f;
  if ( uByte&2 )
    fVolume += -0.8f;
  if ( uByte&4 )
    fVolume += -1.5f;
  if ( uByte&8 )
    fVolume += -3.f;
  if ( uByte&16)
    fVolume += -6.f;
  if ( uByte&32)
    fVolume += -12.f;
  if ( uByte&64)
    fVolume += -24.f;
  if ( uByte&128)
    fVolume += -48.f;

  return Log2Linear(fVolume);
}


////////////////////////////////////////////////////////////////////////////////////////
// Init()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
TError CAICAChannel<T>::Init()
{
  TError Error = RET_OK;

  End();

  m_bInit = true;
  m_bOn = false;
  m_pData = NULL;
  m_Type = SAMPLEDATA_TYPE_PCM_16;
  m_iLoopIni = -1;
  m_uPlayPos = 0;
  m_uSpeed = 0;
  m_iSize = 0;
  m_uCountPos = 0;
  m_iVolume = 0;
  m_iPan = 0;

  return Error;
}


////////////////////////////////////////////////////////////////////////////////////////
// End()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAChannel<T>::End()
{
  if ( m_bInit )
  {
    m_bInit = false;
  }
}


////////////////////////////////////////////////////////////////////////////////////////
// SetOn()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAChannel<T>::SetOn(void *pData,ESampleDataType eType, int iSamplesSize, int iLoopIni, float fPitchPercent, int iVolume, int iPan)
{
  /*if ( m_bOn )
    OutputDebugString("SetOn a un canal que estaba ya ON\n");*/
  /*char szTemp[256];
  sprintf(szTemp,"SetOn() Volume(%i)\n",iVolume);
  OutputDebugString(szTemp);*/

  SetPitchPercent(fPitchPercent);
  SetVolume(iVolume);
  SetPan(iPan);
  m_pData = pData;
  m_Type = eType;
  m_iLoopIni = (DWORD)iLoopIni;
  m_uPlayPos = 0;
  m_uCountPos = 0;
  m_uSize = (DWORD)iSamplesSize;
  m_bOn = true;
  ASSERT(m_iLoopIni < (int)m_uSize);
  m_Last16BitValue = (T)0;

  switch(eType)
  {
  case SAMPLEDATA_TYPE_PCM_16:
  case SAMPLEDATA_TYPE_PCM_8:
    /*if ( m_iLoopIni != -1 )
      m_iLoopIni++;*/
    break;
  case SAMPLEDATA_TYPE_ADPCM:
    if ( m_iLoopIni != -1 )
      m_iLoopIni++;
  case SAMPLEDATA_TYPE_ADPCM_LOOP:
    {
      // Temporal, to debug
      /*m_uSize = 0;
      BYTE* pcData = new BYTE[10000000];
      m_pData = pcData;
      FILE* f = fopen("chord.adp","rb");
      while ( fread(pcData,1,1,f) )
      {
        pcData++;
        m_uSize += 2;
      }
      fclose(f);*/      
      //m_iLoopIni = -1;
      /*CAICAADPCMDecoder<T> ADPCMDecoder;
      ADPCMDecoder.Init(pData,iSamplesSize);
      T* pPCMData = new T[ADPCMDecoder.GetDestSamplesSize()];
      ADPCMDecoder.Decode(pPCMData);
      m_pData = pPCMData;*/

      m_ADPCMData.Reset();      
      if (eType == SAMPLEDATA_TYPE_ADPCM_LOOP)
        ASSERT(m_uSize == (m_uSize&0xfffffffc));
    }
    break;
  };

  if ( m_iLoopIni != -1 &&
       m_iLoopIni > (int)m_uSize )
    m_iLoopIni = (int)m_uSize;
}


////////////////////////////////////////////////////////////////////////////////////////
// Stop()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAChannel<T>::Stop()
{
  m_bOn = false;
}


////////////////////////////////////////////////////////////////////////////////////////
// SetPitchPercent()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAChannel<T>::SetPitchPercent(float fPitchPercent)
{
  fPitchPercent *= 12.f;
  int iNote = (int)fPitchPercent;
  int iFineTune = (int)((fPitchPercent - (float)iNote)*127.f);
  iNote = iNote + 48;  // 48 = C-4
  float fPeriod = float(10*12*16*4) - float(iNote*16*4)- float(iFineTune/2);
  if ( fPeriod > 7680.f) 
    fPeriod = 7680.f;
  if ( fPeriod < 0.f) 
    fPeriod = 0.f;
  DWORD uFreq = (DWORD)(44100.f * pow(2.f,(float(6*12*16*4)-fPeriod)/float(12*16*4)));
  float fAbajo = float(uFreq % 44100);
  fAbajo /= 44100.f;
  fAbajo *= 262144.f;
  m_uSpeed = ((uFreq/((44100)))<<18)+((DWORD)(fAbajo));
}

////////////////////////////////////////////////////////////////////////////////////////
// SetVolume()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAChannel<T>::SetVolume(int iVolume)
{ 
  m_iVolume = Log2Linear8(iVolume);
  
  /*char szTemp[256];
  sprintf(szTemp,"Volume(%i)\n",m_iVolume);
  OutputDebugString(szTemp);*/
}

////////////////////////////////////////////////////////////////////////////////////////
// SetPan()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAChannel<T>::SetPan(int iPan)
{
  BYTE aTablePan[16] = { 255, 196, 128, 96, 64, 48, 32, 24, 16, 8, 6, 4, 3, 2, 1, 0 };

  m_aiPan[0] = m_aiPan[1] = 255;
  if ( (iPan&(1<<4)) == 0 )
    m_aiPan[0] = aTablePan[iPan&0xf];
  else
    m_aiPan[1] = aTablePan[iPan&0xf];
}
