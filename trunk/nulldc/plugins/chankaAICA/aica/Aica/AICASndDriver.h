////////////////////////////////////////////////////////////////////////////////////////
/// @file  AICASndDriver.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef AICASNDDRIVER_H_
#define AICASNDDRIVER_H_

class IAICASndDriver
{
public:
  virtual   ~IAICASndDriver     () {}

  class ICallback
  {
  public:
    //virtual void    Process     (int iSamples, signed short* paOutput) = 0;
    virtual void    GetNewBlockData   (signed short* paOutput) = 0;
    virtual void    SetBlockDataSize  (int iSamples) = 0;
  };

  virtual void    SetCallback       (IAICASndDriver::ICallback *pCallback) = 0;
};

/*class IAICASndDriver
{
public:
  class ICallback
  {
    void    Process     (int iSamples, unsigned short* paOutput);
  };

          CAICASndDriver      () : m_bInit(false) {}
          ~CAICASndDriver     ()                  { End(); }

  TError  Init                (ICallback *pCallback);
  void    End                 ();

private:
  bool          m_bInit;
  ICallback*    m_pCallback;
};*/


#endif AICASNDDRIVER_H_
