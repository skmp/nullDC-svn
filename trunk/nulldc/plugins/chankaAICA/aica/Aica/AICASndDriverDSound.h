////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICASndDriverDSound.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef AICASNDDRIVER_DSOUND_H_
#define AICASNDDRIVER_DSOUND_H_


#include "AICASndDriver.h"
#ifndef XBOX
#include <mmreg.h>
#endif
#include <dsound.h>

DWORD WINAPI DS8ACSUpdate(void * pData);

class CAICASndDriverDSound : public IAICASndDriver
{
public:
          CAICASndDriverDSound    () : m_bInit(false) {}
          ~CAICASndDriverDSound   ()                  { End(); }

  TError  Init                    (HWND _hWnd);
  void    End                     ();

  void    SetCallback             (IAICASndDriver::ICallback *pCallback);

private:
  friend DWORD WINAPI DS8ACSUpdate(void * pData);
  TError  CreateBuffer            ();

  void    UpdateBuffer            (int iSide);

  bool                        m_bInit;
  IAICASndDriver::ICallback*  m_pCallback;
  LPDIRECTSOUND8              m_pDS;
  LPDIRECTSOUNDBUFFER         m_pDSBuffer;
  HANDLE                      m_aEvents[2];
  volatile bool               m_bWaitingEndThread;
  HANDLE                      m_ThreadFinished;
  unsigned int                m_uBytesBuffer;
};



#endif AICASNDDRIVER_WAVEOUT_H_
