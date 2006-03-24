////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICASndDriverDSound.cpp
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#include "..\StdAfx.h"
#include "AICAGlobals.h"
#include "AICASndDriverDSound.h"
#include <process.h>

//=================================================================================
// DS8ACSUpdate()
//=================================================================================
DWORD WINAPI DS8ACSUpdate(void * pData)
{  
  CAICASndDriverDSound* pDriver = (CAICASndDriverDSound*)pData;
  while ( !pDriver->m_bWaitingEndThread )
  {
#ifdef XBOX
    DirectSoundDoWork();
    DWORD uWait = WaitForMultipleObjects(2,pDriver->m_aEvents,FALSE,10);    
#else
    DWORD uWait = WaitForMultipleObjects(2,pDriver->m_aEvents,FALSE,INFINITE);    
#endif

    if (uWait != WAIT_TIMEOUT)
    {
      if ( !pDriver->m_bWaitingEndThread )
      {
        if ( uWait == WAIT_OBJECT_0 )
          pDriver->UpdateBuffer(0);
        else if ( uWait == WAIT_OBJECT_0+1 )
          pDriver->UpdateBuffer(1);
        else
          ASSERT(false);
      }
    }
  }
  SetEvent(pDriver->m_ThreadFinished);

  return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// Init()
////////////////////////////////////////////////////////////////////////////////////////
TError CAICASndDriverDSound::Init(HWND _hWnd)
{
  TError Error = RET_OK;

  //OutputDebugString("Init\n");

  End();

  m_bInit = true;
  m_pCallback = NULL;
  m_pDSBuffer = NULL;
  m_pDSBuffer = NULL;
  m_ThreadFinished = NULL;
  m_bWaitingEndThread = false;
  for ( int i = 0 ; i < 2 ; ++i )
  {
    m_aEvents[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
    ASSERT(m_aEvents[i]);
  }

  // Init DSound8
#ifdef XBOX
  if ( Error == RET_OK && DirectSoundCreate(NULL, &m_pDS, NULL) != DS_OK )
#else
  if ( Error == RET_OK && DirectSoundCreate8(NULL, &m_pDS, NULL) != DS_OK )
#endif
  {
    Error = RET_FAIL;
    m_pDS = NULL;
  }

#ifndef XBOX
  // Set Cooperative Level
  if ( Error == RET_OK && m_pDS->SetCooperativeLevel(_hWnd, DSSCL_PRIORITY) != DS_OK )
    Error = RET_FAIL;
#endif

  // Create Buffer
  if ( Error == RET_OK )
    Error = CreateBuffer();

  // Create the UpdateThread
  if ( Error == RET_OK )
  {
    m_ThreadFinished = CreateEvent(NULL,false,false,NULL);
    if ( m_ThreadFinished )
    {      
      HANDLE hThread = (HANDLE)CreateThread(NULL,0,DS8ACSUpdate,this,0,NULL);

      if ( hThread == (HANDLE)-1L )
      {
        Error = RET_FAIL;
        CloseHandle(m_ThreadFinished);
        m_ThreadFinished = NULL;
      }
      else
        SetThreadPriority(hThread,THREAD_PRIORITY_HIGHEST);
    }
    else
      Error = RET_FAIL;
  }

  ASSERT(Error==RET_OK);

  if ( Error != RET_OK )
    End();

  return Error;
}


////////////////////////////////////////////////////////////////////////////////////////
// End()
////////////////////////////////////////////////////////////////////////////////////////
void CAICASndDriverDSound::End()
{
  if ( m_bInit )
  {
    if ( m_pDSBuffer )
      m_pDSBuffer->Stop();

    // End Thread
    m_bWaitingEndThread = true;
    if ( m_ThreadFinished )
    {
      SetEvent(m_aEvents[0]);
      WaitForSingleObject(m_ThreadFinished,INFINITE);
      CloseHandle(m_ThreadFinished);
      m_ThreadFinished = NULL;
    }

    if ( m_pDS )
    {
      m_pDS->Release();
      m_pDS = NULL;
    }

    for ( int i = 0 ; i < 2 ; ++i )
    {
      if ( m_aEvents[i] )
        CloseHandle(m_aEvents[i]);
    }

    m_bInit = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// SetCallback()
////////////////////////////////////////////////////////////////////////////////////////
void CAICASndDriverDSound::SetCallback(IAICASndDriver::ICallback *pCallback)
{ 
  m_pCallback = pCallback; 
  m_pCallback->SetBlockDataSize(m_uBytesBuffer/8);
  HRESULT hr = m_pDSBuffer->Play(0,0,DSBPLAY_LOOPING);
  ASSERT(hr == DS_OK);
}



////////////////////////////////////////////////////////////////////////////////////////
// CreateBuffer()
////////////////////////////////////////////////////////////////////////////////////////
TError CAICASndDriverDSound::CreateBuffer()
{
  TError Error = RET_OK;

  WAVEFORMATEX wfx;
  memset(&wfx, 0, sizeof(WAVEFORMATEX));
  wfx.wFormatTag = WAVE_FORMAT_PCM;
  wfx.nChannels = 2;
  wfx.nSamplesPerSec = 44100;
  wfx.wBitsPerSample = 16;
  wfx.nBlockAlign = wfx.nChannels*(wfx.wBitsPerSample/8);
  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

  DSBUFFERDESC dsbdesc;
  memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
  dsbdesc.dwSize = sizeof(DSBUFFERDESC);
#ifdef XBOX
  dsbdesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY;
#else
  dsbdesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_LOCSOFTWARE;
#endif
  dsbdesc.dwBufferBytes = (wfx.nChannels * (wfx.wBitsPerSample/8) * 44100 * 40) / 1000; // 20ms
  m_uBytesBuffer = dsbdesc.dwBufferBytes;
  dsbdesc.lpwfxFormat = &wfx;

  if ( m_pDS->CreateSoundBuffer(&dsbdesc, &m_pDSBuffer, NULL) != DS_OK )
  {
    m_pDSBuffer = NULL;
    Error = RET_FAIL;
  }

  // Set Notification positions
  if ( Error == RET_OK )
  {
#ifdef XBOX
    DSBPOSITIONNOTIFY aPositionNotify[2];
    for ( int i = 0 ; i < 2 && Error == RET_OK ; ++i )
    {
      aPositionNotify[i].dwOffset = i == 0 ? dsbdesc.dwBufferBytes/2 : 0;
      aPositionNotify[i].hEventNotify = m_aEvents[i];
    }

    if ( m_pDSBuffer->SetNotificationPositions(2, aPositionNotify) != DS_OK )
      Error = RET_FAIL;      
#else
    LPDIRECTSOUNDNOTIFY8 lpDsNotify = NULL;
    if ( m_pDSBuffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&lpDsNotify) == DS_OK )
    {
      DSBPOSITIONNOTIFY aPositionNotify[2];
      for ( int i = 0 ; i < 2 && Error == RET_OK ; ++i )
      {
        aPositionNotify[i].dwOffset = i == 0 ? dsbdesc.dwBufferBytes/2 : 0;
        aPositionNotify[i].hEventNotify = m_aEvents[i];
      }
      if ( lpDsNotify->SetNotificationPositions(2, aPositionNotify) != DS_OK )
        Error = RET_FAIL;
    }
    else
      Error = RET_FAIL;    
#endif    
  }

  return Error;
}


////////////////////////////////////////////////////////////////////////////////////////
// UpdateBuffer()
////////////////////////////////////////////////////////////////////////////////////////
void CAICASndDriverDSound::UpdateBuffer(int iSide)
{
  void* pDstData = NULL;
  void* pDstData2 = NULL;
  unsigned long uDstBytes = 0;
  unsigned long uDstBytes2 = 0;
  if ( iSide <= 1 &&
      m_pDSBuffer->Lock((m_uBytesBuffer/2)*iSide,m_uBytesBuffer/2,&pDstData,&uDstBytes,&pDstData2,&uDstBytes2,0) == DS_OK )
  {
    ASSERT(!pDstData2);
    ASSERT(!uDstBytes2);
    ASSERT(uDstBytes == (m_uBytesBuffer/2));

    m_pCallback->GetNewBlockData((signed short*)pDstData);

    m_pDSBuffer->Unlock(pDstData,uDstBytes,NULL,NULL);
  }
  else
    ASSERT(false);
}
