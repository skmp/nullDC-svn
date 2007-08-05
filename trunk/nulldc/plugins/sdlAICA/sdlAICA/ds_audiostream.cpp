#include <initguid.h>
#include <dsound.h>
#include "audiostream_rif.h"
#include "audiostream.h"

u32 THREADCALL SoundThread(void* param);

IDirectSound8* dsound;
IDirectSoundBuffer8* buffer;
IDirectSoundNotify8* buffer_notify;

cThread sound_th(SoundThread,0);
 HANDLE buffer_events[2];
volatile bool soundthread_running=false;

u32 wait_buffer_size;

void UpdateBuff(u8* pos)
{
	speed_limit.Set();

	u8* buf=GetReadBuffer();
	if (settings.GlobalMute)
	{
		memset(pos,0,wait_buffer_size);
	}
	else
	{
		if (buf==0)
		{
			//printf("GetReadBuffer -- Out Of Buffers\n");
			//memset(pos,0,wait_buffer_size);
		}
		else
		{
			memcpy(pos,buf,wait_buffer_size);
		}
	}
}
u32 THREADCALL SoundThread(void* param)
{
	while(soundthread_running)
	{
		u32 rv = WaitForMultipleObjects(2,buffer_events,FALSE,400);

		LPVOID p1,p2;
		DWORD s1,s2;

	
		if (rv==WAIT_OBJECT_0+1)
		{
			//part 1
			verifyc(buffer->Lock(0,wait_buffer_size,&p1,&s1,&p2,&s2,0));
			
			UpdateBuff((u8*)p1);
			
			verifyc(buffer->Unlock(p1,s1,p2,s2));
		}
		else if (rv==WAIT_OBJECT_0)
		{
			//part 2
			verifyc(buffer->Lock(wait_buffer_size,wait_buffer_size,&p1,&s1,&p2,&s2,0));

			UpdateBuff((u8*)p1);

			verifyc(buffer->Unlock(p1,s1,p2,s2));
		}
	}
	return 0;
}

void ds_InitAudio()
{
	verifyc(DirectSoundCreate8(NULL,&dsound,NULL));

	verifyc(dsound->SetCooperativeLevel((HWND)eminf.GetRenderTarget(),DSSCL_PRIORITY));
	IDirectSoundBuffer* buffer_;

	WAVEFORMATEX wfx; 
	DSBUFFERDESC desc; 

	// Set up WAV format structure. 

	memset(&wfx, 0, sizeof(WAVEFORMATEX)); 
	wfx.wFormatTag = WAVE_FORMAT_PCM; 
	wfx.nChannels = 2; 
	wfx.nSamplesPerSec = 44100; 
	wfx.nBlockAlign = 4; 
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign; 
	wfx.wBitsPerSample = 16; 

	// Set up DSBUFFERDESC structure. 

	memset(&desc, 0, sizeof(DSBUFFERDESC)); 
	desc.dwSize = sizeof(DSBUFFERDESC); 
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY;// _CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY; 
	desc.dwBufferBytes = settings.BufferSize*(2)* wfx.nBlockAlign; 
	desc.lpwfxFormat = &wfx; 

	wait_buffer_size=settings.BufferSize*wfx.nBlockAlign;

	if (settings.HW_mixing==0)
	{
		desc.dwFlags |=DSBCAPS_LOCSOFTWARE;
	}
	else if (settings.HW_mixing==1)
	{
		desc.dwFlags |=DSBCAPS_LOCHARDWARE;
	}
	else if (settings.HW_mixing==2)
	{
		//auto
	}
	else
	{
		die("settings.HW_mixing: Invalid value");
	}

	if (settings.GlobalFocus)
		desc.dwFlags|=DSBCAPS_GLOBALFOCUS;

	verifyc(dsound->CreateSoundBuffer(&desc,&buffer_,0));
	verifyc(buffer_->QueryInterface(IID_IDirectSoundBuffer8,(void**)&buffer));
	buffer_->Release();

	verifyc(buffer->QueryInterface(IID_IDirectSoundNotify8,(void**)&buffer_notify));
	DSBPOSITIONNOTIFY not[2];

	buffer_events[0]=CreateEvent(NULL,FALSE,FALSE,NULL);
	buffer_events[1]=CreateEvent(NULL,FALSE,FALSE,NULL);

	not[0].dwOffset=wfx.nBlockAlign*10;/*(desc.dwBufferBytes/3)&(~(wfx.nBlockAlign-1))*/;		//midle of the first half
	not[0].hEventNotify=buffer_events[0];

	not[1].dwOffset=desc.dwBufferBytes/2 + wfx.nBlockAlign*10;		//midle of the second half
	not[1].hEventNotify=buffer_events[1];

	buffer_notify->SetNotificationPositions(2,not);

	LPVOID p1=0,p2=0;
	DWORD s1=0,s2=0;

	verifyc(buffer->Lock(0,desc.dwBufferBytes,&p1,&s1,&p2,&s2,0));
	verify(p2==0);
	memset(p1,0,s1);
	verifyc(buffer->Unlock(p1,s1,p2,s2));

	soundthread_running=true;
	verify(SetThreadPriority((HANDLE)sound_th.hThread,THREAD_PRIORITY_TIME_CRITICAL));
	sound_th.Start();
	//Play the buffer !
	verifyc(buffer->Play(0,0,DSBPLAY_LOOPING));
	
}
void ds_TermAudio()
{
	buffer->Stop();
	soundthread_running=false;
	sound_th.WaitToEnd(0xFFFFFFFF);

	verify(CloseHandle(buffer_events[0]));
	verify(CloseHandle(buffer_events[1]));

	buffer_notify->Release();
	buffer->Release();
	dsound->Release();
}