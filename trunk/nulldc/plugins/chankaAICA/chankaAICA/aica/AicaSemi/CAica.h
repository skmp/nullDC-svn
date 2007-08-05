#ifndef CAICA_SEMI_H
#define CAICA_SEMI_H

#include "CSound.h"

class CAICA
{
	FILE *test;
	CGUI DummyGUI;
	CSound SoundCore;
	HANDLE Thread;
	DWORD ThreadID;
	signed short BuffersTemp[2][44100];
	int SampleRate;
public:
	//float GetMixVelocity();
	void ProcessSync(int nSamples);
	TError Init(unsigned char *Registers,unsigned char *RAM);
	void End();
	unsigned int GetCurrentPosChannel(int nChannel);
	unsigned int GetCurrentEnvChannel(int nChannel);
	void UpdateChannelData(unsigned int iAddress);
	int GetSuggestedUpdateSize();
};


#endif