////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICASndDriverWaveOut.cpp
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#include "..\StdAfx.h"
#include "AICAGlobals.h"
#include "AICASndDriverWaveOut.h"




////////////////////////////////////////////////////////////////////////////////////////
// WAVEOUT CONFIG VALUES
//
// With this values we know that:
//
//    * Secuencing Commands Precision   =   +-(ms of WAVEOUT_CFG_SIZE_BUFFERS)
//    * Sound Output Latency            =   (ms WAVEOUT_CFG_NUM_BUFFERS)*(ms WAVEOUT_CFG_NUM_BUFFERS)
//
const int WAVEOUT_CFG_SIZE_BUFFERS  = 441*2*2; // At 44100hz stereo 16bit = 10 ms
const int WAVEOUT_CFG_NUM_BUFFERS   = 5;       // 
// So, now we have:
//
//    * +-5ms Secuencing Commands precision
//    * 50ms Output Latency

#ifndef XBOX

////////////////////////////////////////////////////////////////////////////////////////
// MyWaveOutProc()
////////////////////////////////////////////////////////////////////////////////////////
static void CALLBACK MyWaveOutProc( HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 )
{
/*#ifdef _SAVE_AICA_DEBUG_FILES_
  static FILE* s_f = 0;
  if ( !s_f )
    s_f = fopen("d:\\AICADrvWaveOut.raw","wb");
#endif*/

  CAICASndDriverWaveOut* pSndDrv = (CAICASndDriverWaveOut*)(*(void**)&dwInstance);

  if ( !pSndDrv->m_bEnding &&
       uMsg == WOM_DONE )
  {
    WAVEHDR* pwh = (WAVEHDR*)(*(void**)&dwParam1);
    waveOutUnprepareHeader(hwo,pwh,sizeof(*pwh));

    int iSamplesToUpdate = pwh->dwBufferLength/4;

    if ( iSamplesToUpdate && pSndDrv->m_pCallback )
      //pSndDrv->m_pCallback->Process(iSamplesToUpdate,(signed short*)pwh->lpData);
      pSndDrv->m_pCallback->GetNewBlockData((signed short*)pwh->lpData);

/*#ifdef _SAVE_AICA_DEBUG_FILES_
    fwrite(pwh->lpData,sizeof(signed short),iSamplesToUpdate*2,s_f);
    fflush(s_f);
#endif*/

    pwh->dwBytesRecorded = 0;
    pwh->dwUser = NULL;
    pwh->dwFlags = 0;
    pwh->dwLoops = 0;

    waveOutPrepareHeader(hwo,pwh,sizeof(*pwh));
    waveOutWrite(hwo,pwh,sizeof(*pwh));
  }
}


////////////////////////////////////////////////////////////////////////////////////////
// Init()
////////////////////////////////////////////////////////////////////////////////////////
TError CAICASndDriverWaveOut::Init()
{
  TError Error = RET_OK;

  //OutputDebugString("Init\n");

  End();

  m_bInit = true;
  m_pCallback = NULL;
  m_bEnding = false;

  // Init Device
  WAVEFORMATEX wf;
  wf.cbSize = sizeof(WAVEFORMATEX);
  wf.wFormatTag = WAVE_FORMAT_PCM;
  wf.nChannels = 2;
  wf.nSamplesPerSec = 44100;
  wf.wBitsPerSample = 16;
  wf.nBlockAlign = (wf.nChannels * wf.wBitsPerSample/8);
  wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
  if ( waveOutOpen(&m_Handle,WAVE_MAPPER,&wf,(DWORD_PTR)MyWaveOutProc,(DWORD_PTR)this,CALLBACK_FUNCTION) != MMSYSERR_NOERROR )
  {
    Error = RET_FAIL;
    m_Handle = NULL;
  }
  else
  {
    for ( int i = 0 ; i < WAVEOUT_CFG_NUM_BUFFERS && Error == RET_OK ; ++i )
    {
      WAVEHDR *wh = new WAVEHDR;
      wh->reserved = 0;
      wh->lpNext = 0;
      wh->dwBufferLength = WAVEOUT_CFG_SIZE_BUFFERS;   
      wh->lpData = (LPSTR)new signed char[wh->dwBufferLength];
      wh->dwBytesRecorded = 0;
      wh->dwUser = NULL;
      wh->dwFlags = 0;
      wh->dwLoops = 0;

      memset(wh->lpData,0,wh->dwBufferLength);

      if ( waveOutPrepareHeader(m_Handle,wh,sizeof(*wh)) != MMSYSERR_NOERROR )
        Error = RET_FAIL;
      else
      {
        if ( waveOutWrite(m_Handle,wh,sizeof(*wh)) != MMSYSERR_NOERROR )
          Error = RET_FAIL;
      }
    }
  }

  ASSERT(Error==RET_OK);

  if ( Error != RET_OK )
    End();

  return Error;
}


////////////////////////////////////////////////////////////////////////////////////////
// End()
////////////////////////////////////////////////////////////////////////////////////////
void CAICASndDriverWaveOut::End()
{
  if ( m_bInit )
  {
    if ( m_Handle )
    {
      m_bEnding = true;
      waveOutReset(m_Handle);
      waveOutClose(m_Handle);
    }

    m_bInit = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////
// SetCallback()
////////////////////////////////////////////////////////////////////////////////////////
void CAICASndDriverWaveOut::SetCallback(IAICASndDriver::ICallback *pCallback)
{ 
  m_pCallback = pCallback; 
  m_pCallback->SetBlockDataSize(WAVEOUT_CFG_SIZE_BUFFERS/4);
}


#else
//#include <Windows.h>
#include <xtl.h>

static DWORD WINAPI threadSoundNull(void* pBuffer)
{
  CAICASndDriverWaveOut* pDriver = (CAICASndDriverWaveOut*) pBuffer;
  while (true)
  {
    Sleep(10);
    if (pDriver->m_pCallback)
    {
      short aTemp[WAVEOUT_CFG_SIZE_BUFFERS];
      IAICASndDriver::ICallback* pCallback = (IAICASndDriver::ICallback*)pDriver->m_pCallback;
      pCallback->GetNewBlockData(aTemp);
    }    

  }
  return 0;
}

TError  CAICASndDriverWaveOut::Init                    ()
{
  m_pCallback = NULL;
  HANDLE hThread = CreateThread(NULL,0,threadSoundNull,this,0,NULL);

  SetThreadPriority(hThread,THREAD_PRIORITY_HIGHEST);
  m_bInit = true;
  return RET_OK;
}
void    CAICASndDriverWaveOut::End                     ()
{
}

void    CAICASndDriverWaveOut::SetCallback             (IAICASndDriver::ICallback *pCallback)
{
  m_pCallback = pCallback;
}




#endif