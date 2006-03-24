////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICADSP.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef AICADSP_H_
#define AICADSP_H_

#include "AICADSPChannel.h"

const int AICADSP_CHANNELS = 16 + 2;

template <typename T,int CHANNELBUFFERSIZE>
class CAICADSP
{
public:
          CAICADSP    () : m_bInit(false) {}
          ~CAICADSP   ()                  { End(); }

  TError  Init        ();
  void    End         ();

private:
  bool                                  m_bInit;
  CAICADSPChannel<T,CHANNELBUFFERSIZE>  m_aChannels[AICADSP_CHANNELS];
};

#include "AICADSPcpp.h"

#endif AICADSP_H_
