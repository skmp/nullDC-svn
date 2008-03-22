////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICASndDriverWaveOut.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef AICASNDDRIVER_WAVEOUT_H_
#define AICASNDDRIVER_WAVEOUT_H_


#include "AICASndDriver.h"
#include <list>

#ifndef XBOX

#include <mmsystem.h>


static void CALLBACK MyWaveOutProc( HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 );

class CAICASndDriverWaveOut : public IAICASndDriver
{
public:
          CAICASndDriverWaveOut   () : m_bInit(false) {}
          ~CAICASndDriverWaveOut  ()                  { End(); }

  TError  Init                    ();
  void    End                     ();

  void    SetCallback             (IAICASndDriver::ICallback *pCallback);

private:
  friend static void CALLBACK MyWaveOutProc( HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 );
  bool                        m_bInit;
  IAICASndDriver::ICallback*  m_pCallback;

  HWAVEOUT                    m_Handle;
  std::list<WAVEHDR>          m_WaveHdrs;
  volatile bool               m_bEnding;
};

#else

class CAICASndDriverWaveOut : public IAICASndDriver
{
public:
  CAICASndDriverWaveOut   () : m_bInit(false) {}
  ~CAICASndDriverWaveOut  ()                  { End(); }

  TError  Init                    ();
  void    End                     ();

  void    SetCallback             (IAICASndDriver::ICallback *pCallback);
private:
  bool                        m_bInit;
  friend static DWORD WINAPI threadSoundNull(void* pBuffer);
  volatile IAICASndDriver::ICallback*  m_pCallback;
};

#endif


#endif AICASNDDRIVER_WAVEOUT_H_
