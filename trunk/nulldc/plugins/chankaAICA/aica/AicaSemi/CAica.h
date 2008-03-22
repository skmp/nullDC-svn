#ifndef CAICA_SEMI_H
#define CAICA_SEMI_H

//BAKMODE para usar los output drivers de baktery
#define BAKMODE

//para usar buffer list para sincronizar (solo con BAKMODE)
#define BUFFERLIST

#if defined(BUFFERLIST) && !defined(BAKMODE)
#error BUFFERLIST SOLO VA CON BAKMODE
#endif

#ifndef BAKMODE
#include "CSound.h"
#endif

#include <list>

#include "SoundBuffers.h"

class CAICA
{
private:
	FILE *test;
#ifndef BAKMODE
	CGUI DummyGUI;
	CSound SoundCore;
#else

#endif
	HANDLE Thread;
	DWORD ThreadID;
	signed short BuffersTemp[2][44100*2];
	int SampleRate;
	int MagicAdjust;

	CSoundBuffer BuffersProcessed;
	CSoundBuffer BuffersFree;
	TBufferProcessed *CurrentBufferProcessed;
public:
	float GetMixVelocity();
	void ProcessSync(int nSamples);
	void GetSamples(int nSamples,signed short *Buffer);
	TError Init(unsigned char *Registers,unsigned char *RAM);
	void End();
	unsigned int GetCurrentPosChannel(int nChannel);
	void UpdateChannelData(unsigned int iAddress);
	int GetSuggestedUpdateSize();
	friend void __cdecl SoundThread(LPVOID Param);
};


#endif