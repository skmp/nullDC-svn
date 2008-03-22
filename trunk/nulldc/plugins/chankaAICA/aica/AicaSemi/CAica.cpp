#include "..\stdafx.h"
#include "Aicadc.h"

#include "CAica.h"
#include "CSound.h"


#define FRAMESYNC


#ifndef FRAMESYNC
void __cdecl SoundThread(LPVOID Param)
{
	const int Slice=44100/(60);
	CAICA *_this=(CAICA*) Param;
	while(1)
	{
		AICA_DoSamples(Slice);
#ifdef SEPARATE_BUFFERS
		_this->SoundCore.AddChannelData(0,_this->BuffersTemp[0],Slice*2);
		_this->SoundCore.AddChannelData(1,_this->BuffersTemp[1],Slice*2);
#else
		_this->SoundCore.AddChannelData(0,_this->BuffersTemp[0],Slice*4);
#endif
//		Sleep(0);
	}
}
#endif

void CAICA::ProcessSync(int nSamples)
{
#ifndef BAKMODE
#ifdef FRAMESYNC	
	nSamples-=MagicAdjust;	//Misterios del mundo, con esto marca 60 en vez de 55
#ifndef XBOX
	if(GetKeyState(VK_SCROLL)&1)
		DummyGUI.FrameLimit=false;
	else
		DummyGUI.FrameLimit=true;
#endif
	AICA_DoSamples(nSamples);
#ifdef SEPARATE_BUFFERS
	SoundCore.AddChannelData(0,BuffersTemp[0],nSamples*2);
	SoundCore.AddChannelData(1,BuffersTemp[1],nSamples*2);
#else
	SoundCore.AddChannelData(0,BuffersTemp[0],nSamples*4);
#endif
#endif
#else
#ifdef BUFFERLIST	
	
	while ( nSamples )
	{
		int iSamplesToUpdate = nSamples;
		int iFreeSamples = CurrentBufferProcessed->GetFreeSamples();
		if ( iSamplesToUpdate > iFreeSamples )
			iSamplesToUpdate = iFreeSamples;
		//m_Mixer.Process(iSamplesToUpdate,&CurrentBufferProcessed->m_aData[(AICA_MAX_SIZE_SAMPLES_BUFFERPROCESSED-iFreeSamples)<<1]);
		AICA_SetBuffers(&CurrentBufferProcessed->m_aData[(AICA_MAX_SIZE_SAMPLES_BUFFERPROCESSED-iFreeSamples)<<1],NULL);
		AICA_DoSamples(iSamplesToUpdate);

		CurrentBufferProcessed->m_iSamplesProcessed += iSamplesToUpdate;
		if (CurrentBufferProcessed->GetFreeSamples() == 0 )
		{
			BuffersProcessed.InsertBuffer(CurrentBufferProcessed);
			CurrentBufferProcessed = BuffersFree.ExtractBuffer();
			if(!CurrentBufferProcessed)
			{
				CurrentBufferProcessed=new TBufferProcessed;
			}
			CurrentBufferProcessed->m_iSamplesProcessed=0;
		}
		nSamples -= iSamplesToUpdate;
	}



#endif
#endif
}

void CAICA::GetSamples(int nSamples,signed short *buffer)
{
#ifdef BUFFERLIST
	TBufferProcessed* pBufferProcessed=BuffersProcessed.ExtractBuffer();
	if(pBufferProcessed)
	{
		ASSERT(pBufferProcessed->m_iSamplesProcessed == nSamples);
		memcpy(buffer,pBufferProcessed->m_aData,nSamples*2*sizeof(signed short));
		BuffersFree.InsertBuffer(pBufferProcessed);
	}
	else
		memset(buffer,0,nSamples*2*sizeof(signed short));
#else
	AICA_SetBuffers(buffer,NULL);
	AICA_DoSamples(nSamples);
#endif
}


TError CAICA::Init(unsigned char *Registers,unsigned char *RAM)
{
	SampleRate=44100;
	switch(SampleRate)
	{
		case 11025:
			MagicAdjust=7;
			break;
		case 22050:
			MagicAdjust=15;
			break;
		case 44100:
			MagicAdjust=30;
			break;
	}
#ifndef BAKMODE
	DummyGUI.FrameLimit=true;
	DummyGUI.AutoDeLimit=false;

	SoundCore.Init(&DummyGUI,SampleRate,60,5);
	
#ifdef SEPARATE_BUFFERS
	SoundCore.CreateChannel(0,SampleRate,39,0);
	SoundCore.CreateChannel(1,SampleRate,39,255);
#else
	SoundCore.CreateChannelStereo(0,SampleRate,39,0);
#endif
#else
#ifdef BUFFERLIST
	BuffersProcessed.Init();
	BuffersFree.Init();
	CurrentBufferProcessed=NEW(TBufferProcessed);
#endif
#endif


	AICA_Init(SampleRate);
	AICA_SetRAM(RAM,Registers);
#ifdef SEPARATE_BUFFERS
	AICA_SetBuffers(BuffersTemp[0],BuffersTemp[1]);
#else
	AICA_SetBuffers(BuffersTemp[0],NULL);
#endif
#ifndef XBOX
#ifndef FRAMESYNC
	Thread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) SoundThread,(LPVOID) this,0,&ThreadID);
#endif
#endif
	return RET_OK;
}



void CAICA::End()
{
#ifndef XBOX
#ifndef FRAMESYNC
	TerminateThread(Thread,0);
	CloseHandle(Thread);
#endif
#endif
#ifndef BAKMODE
	SoundCore.Destroy();
#endif
}

unsigned int CAICA::GetCurrentPosChannel(int nChannel)
{
	return AICA_GetPlayPos(nChannel);
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
}

int CAICA::GetSuggestedUpdateSize()
{
	return SampleRate/(60*2);
}

float CAICA::GetMixVelocity()
{
#ifndef BAKMODE
	return (float) SampleRate/ 22050.0f;
#else
#ifdef BUFFERLIST
	double m_dIdealVelocity;
	int iBlocks=BuffersProcessed.Size();
	m_dIdealVelocity = (double)(iBlocks+1) / 8.0;
	m_dIdealVelocity = pow(1.0 / m_dIdealVelocity,2);
	return (float) m_dIdealVelocity;
#else
	return (float) SampleRate/ 22050.0f;
#endif
#endif
}