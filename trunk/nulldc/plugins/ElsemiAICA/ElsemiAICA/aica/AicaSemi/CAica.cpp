#include "../stdafx.h"
#include "CAica.h"
#include "Aicadc.h"
#include "CSound.h"



/*
void SoundThread(LPVOID Param)
{
	const int Slice=44100/60;
	while(1)
	{
		AICA_DoSamples(Slice);
		//fwrite(BuffersTemp[0],Slice,2,test);
		SoundCore.AddChannelData(0,(UINT16*) BuffersTemp[0],Slice*2);
		//SoundCore.AddChannelData(1,(UINT16*) BuffersTemp[1],Slice*2);
		Sleep(0);
	}
}
*/

void CAICA::ProcessSync(int nSamples)
{
	//nSamples-=MagicAdjust;	//Misterios del mundo, con esto marca 60 en vez de 55
	/*
	if(GetKeyState(VK_SCROLL)&1)
		DummyGUI.FrameLimit=false;
	else
		DummyGUI.FrameLimit=true;
	*/
	AICA_DoSamples(nSamples);
	SoundCore.AddChannelData(0,(UINT16*) BuffersTemp[0],nSamples*2);
	SoundCore.AddChannelData(1,(UINT16*) BuffersTemp[1],nSamples*2);
}

TError CAICA::Init(unsigned char *Registers,unsigned char *RAM)
{
	SampleRate=44100;

	//DummyGUI.FrameLimit=true;
	//DummyGUI.AutoDeLimit=false;

	SoundCore.Init(0,SampleRate,60,5);
	SoundCore.CreateChannel(0,SampleRate,39,0);
	SoundCore.CreateChannel(1,SampleRate,39,255);

	
	AICA_Init(SampleRate);
	AICA_SetRAM(RAM,Registers);

	AICA_SetBuffers(BuffersTemp[0],BuffersTemp[1]);

	//Thread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) SoundThread,0,0,&ThreadID);
	return RET_OK;
}
void CAICA::End()
{
	TerminateThread(Thread,0);
	CloseHandle(Thread);

	SoundCore.Destroy();
//	fclose(test);
}

unsigned int CAICA::GetCurrentPosChannel(int nChannel)
{
	return AICA_GetPlayPos(nChannel);
}

unsigned int CAICA::GetCurrentEnvChannel(int nChannel)
{
	return AICA_GetEnvState(nChannel);
}

void CAICA::UpdateChannelData(unsigned int iAddress)
{
	int s;
	int r;

	if(iAddress<0x1FFC)
	{
		s=iAddress/128;
		r=iAddress%128;
		AICA_UpdateSlotReg(s,r);
	}
	if(iAddress>=0x3000 && iAddress<0x4000)		//DSP
	{
		AICA_UpdateDSP(iAddress);
	}
}

int CAICA::GetSuggestedUpdateSize()
{
	return SampleRate/(60*2);
}

/*float CAICA::GetMixVelocity()
{
	return (float) SampleRate/ 22050.0f;
}*/