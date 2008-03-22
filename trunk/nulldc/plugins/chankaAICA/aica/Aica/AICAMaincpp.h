////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICAMaincpp.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
// Init()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
TError CAICAMain<T>::Init(IAICASndDriver* pSndDriver, EAICAMixMode eMode)
{
  TError Error = RET_OK;

  End();

  m_bInit = true;
  InitializeCriticalSection(&m_CSBuffersProcessed);
  InitializeCriticalSection(&m_CSBuffersFree);
  m_pSndDriver = pSndDriver;
  m_iBlockDataSize = 0;
  m_pCurrentBufferProcessed = new TBufferProcessed;
  m_pBuffersProcessed = NULL;
  m_pBuffersFree = NULL;
  m_eMode = eMode;
  m_dIdealVelocity = 1.0;
  
  return Error;
}


////////////////////////////////////////////////////////////////////////////////////////
// End()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::End()
{
  if ( m_bInit )
  {
    m_bInit = false;
    delete m_pSndDriver;

    TBufferProcessed* pBufferProcessed = ExtractBufferProcessed();
    while (pBufferProcessed)
    {
      TBufferProcessed* pTemp = pBufferProcessed;
      pBufferProcessed = ExtractBufferProcessed();
      delete pTemp;
    }

    TBufferProcessed* pBufferFree = ExtractBufferFree();
    while (pBufferFree)
    {
      TBufferProcessed* pTemp = pBufferFree;
      pBufferFree = ExtractBufferFree();
      delete pTemp;
    }

    DeleteCriticalSection(&m_CSBuffersProcessed);
    DeleteCriticalSection(&m_CSBuffersFree);
  }
}


////////////////////////////////////////////////////////////////////////////////////////
// PlayChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::PlayChannel(int iChannel, void* pData, ESampleDataType eType, int iSamplesSize, int iLoopIni, float fPitchPercent, int iVolume, int iPan)
{
  m_Mixer.PlayChannel(iChannel,pData,eType,iSamplesSize,iLoopIni,fPitchPercent,iVolume,iPan);
}

////////////////////////////////////////////////////////////////////////////////////////
// StopChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::StopChannel(int iChannel)
{
  m_Mixer.StopChannel(iChannel);
}

////////////////////////////////////////////////////////////////////////////////////////
// SetPitchChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::SetPitchChannel(int iChannel, float fPitchPercent)
{
  m_Mixer.SetPitchChannel(iChannel,fPitchPercent);
}

////////////////////////////////////////////////////////////////////////////////////////
// GetCurrentPosChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
int CAICAMain<T>::GetCurrentPosChannel(int iChannel) const
{
  return m_Mixer.GetCurrentPosChannel(iChannel);
}


////////////////////////////////////////////////////////////////////////////////////////
// SetVolumeChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::SetVolumeChannel(int iChannel, int iVolume)
{
  m_Mixer.SetVolumeChannel(iChannel,iVolume);
}

////////////////////////////////////////////////////////////////////////////////////////
// SetPanChannel()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::SetPanChannel(int iChannel, int iVolume)
{
  m_Mixer.SetPanChannel(iChannel,iVolume);
}


////////////////////////////////////////////////////////////////////////////////////////
// ProcessSync()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::ProcessSync(int iSamples)
{
  while ( iSamples )
  {
    int iSamplesToUpdate = iSamples;
    int iFreeSamples = m_pCurrentBufferProcessed->GetFreeSamples();
    if ( iSamplesToUpdate > iFreeSamples )
      iSamplesToUpdate = iFreeSamples;
    m_Mixer.Process(iSamplesToUpdate,&m_pCurrentBufferProcessed->m_aData[(AICA_MAX_SIZE_SAMPLES_BUFFERPROCESSED-iFreeSamples)<<1]);
    
    m_pCurrentBufferProcessed->m_iSamplesProcessed += iSamplesToUpdate;
    if (m_pCurrentBufferProcessed->GetFreeSamples() == 0 )
    {
      InsertBufferProcessed(m_pCurrentBufferProcessed);
      m_pCurrentBufferProcessed = ExtractBufferFree();
      if ( !m_pCurrentBufferProcessed )
      {
        //OutputDebugString("new TBufferProcessed\n");
        m_pCurrentBufferProcessed = new TBufferProcessed;
      }
      m_pCurrentBufferProcessed->m_iSamplesProcessed = 0;
    }

    iSamples -= iSamplesToUpdate;
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// GetNewBlockData()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::GetNewBlockData(signed short* paData)
{
  // Calculate Ideal Mix Velocity
  LockBuffersProcessed();
  int iBlocks = 0;
  TBufferProcessed* pTemp = m_pBuffersProcessed;
  while (pTemp)
  {
    iBlocks++;
    pTemp = pTemp->m_pNext;
  }
  UnlockBuffersProcessed();
  m_dIdealVelocity = (double)(iBlocks+1) / 8.0;
  m_dIdealVelocity = pow(1.0 / m_dIdealVelocity,2);

  /*m_dIdealVelocity = (double)iBlocks+1 / 4.0;
  m_dIdealVelocity = 1.0 / m_dIdealVelocity;
  if ( s_temp++ >= 50 )
  {
    char szTemp[256];
    sprintf(szTemp,"NumBlocks(%i) (%f)\n",iBlocks,(float)m_dIdealVelocity);
    OutputDebugString(szTemp);
    s_temp = 0;
  }*/
  

  TBufferProcessed* pBufferProcessed = ExtractBufferProcessed();
  if ( pBufferProcessed )
  {
    ASSERT(pBufferProcessed->m_iSamplesProcessed == m_iBlockDataSize);
    memcpy(paData,pBufferProcessed->m_aData,m_iBlockDataSize*2*sizeof(signed short));
    InsertBufferFree(pBufferProcessed);
  }
  else
    memset(paData,0,m_iBlockDataSize*2*sizeof(signed short));
}

////////////////////////////////////////////////////////////////////////////////////////
// SetBlockDataSize()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::SetBlockDataSize(int iSamples)
{
  m_iBlockDataSize = iSamples;
}

////////////////////////////////////////////////////////////////////////////////////////
// ExtractBufferProcessed()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
TBufferProcessed* CAICAMain<T>::ExtractBufferProcessed()
{
  TBufferProcessed* pBuffer = NULL;
  LockBuffersProcessed();
  if ( m_pBuffersProcessed )
  {
    pBuffer = m_pBuffersProcessed;
    m_pBuffersProcessed = m_pBuffersProcessed->m_pNext;
    pBuffer->m_pNext = NULL;
  }
  UnlockBuffersProcessed();
  return pBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////
// InsertBufferProcessed()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::InsertBufferProcessed(TBufferProcessed* pBuffer)
{
  LockBuffersProcessed();
  pBuffer->m_pNext = NULL;
  if ( !m_pBuffersProcessed )
  {
    m_pBuffersProcessed = pBuffer;
  }
  else
  {
    TBufferProcessed* pBufferAux = m_pBuffersProcessed;
    while ( pBufferAux->m_pNext )
      pBufferAux = pBufferAux->m_pNext;
    pBufferAux->m_pNext = pBuffer;
  }
  UnlockBuffersProcessed();
}


////////////////////////////////////////////////////////////////////////////////////////
// ExtractBufferFree()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
TBufferProcessed* CAICAMain<T>::ExtractBufferFree()
{
  TBufferProcessed* pBuffer = NULL;
  LockBuffersFree();
  if ( m_pBuffersFree )
  {
    pBuffer = m_pBuffersFree;
    m_pBuffersFree = m_pBuffersFree->m_pNext;
    pBuffer->m_pNext = NULL;
  }
  UnlockBuffersFree();
  return pBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////
// InsertBufferFree()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::InsertBufferFree(TBufferProcessed* pBuffer)
{
  LockBuffersFree();
  pBuffer->m_pNext = NULL;
  if ( !m_pBuffersFree )
  {
    m_pBuffersFree = pBuffer;
  }
  else
  {
    TBufferProcessed* pBufferAux = m_pBuffersFree;
    while ( pBufferAux->m_pNext )
      pBufferAux = pBufferAux->m_pNext;
    pBufferAux->m_pNext = pBuffer;
  }
  UnlockBuffersFree();
}


////////////////////////////////////////////////////////////////////////////////////////
// LockBuffersProcessed()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::LockBuffersProcessed()
{
  EnterCriticalSection(&m_CSBuffersProcessed);
}

////////////////////////////////////////////////////////////////////////////////////////
// UnlockBuffersProcessed()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::UnlockBuffersProcessed()
{
  LeaveCriticalSection(&m_CSBuffersProcessed);
}

////////////////////////////////////////////////////////////////////////////////////////
// LockBuffersFree()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::LockBuffersFree()
{
  EnterCriticalSection(&m_CSBuffersFree);
}

////////////////////////////////////////////////////////////////////////////////////////
// UnlockBuffersFree()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CAICAMain<T>::UnlockBuffersFree()
{
  LeaveCriticalSection(&m_CSBuffersFree);
}

////////////////////////////////////////////////////////////////////////////////////////
// GteMixVelocity()
////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
double CAICAMain<T>::GetMixVelocity()
{
  if ( m_eMode == E_AICA_MIXMODE_INTELLIGENT_EMULATION)
    return m_dIdealVelocity;

  return 1.0;
}
