////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICADSPChannel.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef AICADSPCHANNEL_H_
#define AICADSPCHANNEL_H_

template <typename T, int BUFFERSIZE>
class CAICADSPChannel
{
public:
          CAICADSPChannel     () : m_bInit(false) {}
          ~CAICADSPChannel    ()                  { End(); }

  TError  Init                ();
  void    End                 ();

private:
  bool    m_bInit;
  T       m_Buffer[BUFFERSIZE*2];   // *2 = stereo
};

#include "AICADSPChannelcpp.h"

#endif AICADSPCHANNEL_H_
