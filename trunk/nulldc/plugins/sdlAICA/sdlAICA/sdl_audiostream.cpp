#include "sdl_audiostream.h"


//Wait Events
typedef void* EVENTHANDLE;
class cResetEvent
{
private:
	EVENTHANDLE hEvent;
public :
	cResetEvent(bool State,bool Auto);
	~cResetEvent();
	void Set();		//Set state to signaled
	void Reset();	//Set state to non signaled
	void Wait(u32 msec);//Wait for signal , then reset[if auto]
	void Wait();	//Wait for signal , then reset[if auto]
};

//cResetEvent Calss
cResetEvent::cResetEvent(bool State,bool Auto)
{
		hEvent = CreateEvent( 
        NULL,             // default security attributes
		Auto?FALSE:TRUE,  // auto-reset event?
		State?TRUE:FALSE, // initial state is State
        NULL			  // unnamed object
        );
}
cResetEvent::~cResetEvent()
{
	//Destroy the event object ?
	 CloseHandle(hEvent);
}
void cResetEvent::Set()//Signal
{
	SetEvent(hEvent);
}
void cResetEvent::Reset()//reset
{
	ResetEvent(hEvent);
}
void cResetEvent::Wait(u32 msec)//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,msec);
}
void cResetEvent::Wait()//Wait for signal , then reset
{
	WaitForSingleObject(hEvent,(u32)-1);
}
//End AutoResetEvent




cResetEvent speed_limit(true,true);

/* Prototype of our callback function */
void sdl_fill_audio(void *userdata, Uint8 *stream, int len);

class SoundBuffer
{
public:
	s16* Data;
	u32 ssz;
	u32 wp;

	bool WriteSample(s16 r, s16 l)
	{
		Data[wp++]=l;
		Data[wp++]=r;
		return IsFull();
	}

	bool IsFull()
	{
		return wp==ssz;
	}

	void Reset()
	{
		wp=0;
	}

	void Init(u32 scount)
	{
		ssz=scount*2;
		Data=(s16*)malloc(ssz*sizeof(s16)*2);
		memset(Data,0,ssz*sizeof(s16)*2);
	}

	void Term()
	{
		free(Data);
	}
};

SoundBuffer* WriteBuffer[60];

u32 wbuffer;
u32 rbuffer;
u32 buff_count=3;
u32 buffersz;

void InitAudBuffers(u32 buff_samples)
{
	wbuffer=0;
	rbuffer=0;
	buffersz=buff_samples*sizeof(s16)*2;
	

	for (u32 i=0;i<buff_count;i++)
	{
		WriteBuffer[i]= new SoundBuffer();
		WriteBuffer[i]->Init(buff_samples);
	}
}
//Return read buffer , 0 if none
u8* GetReadBuffer()
{
	//read buffer must allways be before write buffer
	if (rbuffer>=wbuffer)
		return 0;
	u8* rv=(u8*)WriteBuffer[rbuffer%buff_count]->Data;
	rbuffer++;
	return rv;
}
#define fps_limit (true)
void WriteSample(s16 r, s16 l)
{
	if (WriteBuffer[wbuffer%buff_count]->WriteSample(r,l))
	{
		//if we have all buffers filled , were gona replace the last one ;)
		if ((wbuffer-rbuffer)<buff_count)
			wbuffer++;
		else
		{
			//printf("Too many samples , waiting for audio ...\n");
			if (fps_limit)
			{
				speed_limit.Wait();
				speed_limit.Reset();
				wbuffer++;
			}
		}
		//reset the new write buffer
		WriteBuffer[wbuffer%buff_count]->Reset();
	}
}
void WriteSamples(s16* r , s16* l , u32 sample_count)
{
	for (u32 i=0;i<sample_count;i++)
		WriteSample(r[i],l[i]);
}
void WriteSamples(s16* rl , u32 sample_count)
{
	for (u32 i=0;i<(sample_count*2);i+=2)
		WriteSample(rl[i|0],rl[i|1]);
}
void InitAudio()
{
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE);
	
	SDL_AudioSpec desired;

	//44.1k
	desired.freq=44100;
	//16-bit signed audio
	desired.format=AUDIO_S16LSB;
	//stereo
	desired.channels=2;
	//Large audio buffer reduces risk of dropouts but increases response time
	desired.samples=settings.BufferSize;

	/* Our callback function */
	desired.callback=sdl_fill_audio;

	desired.userdata=NULL;

	/* Open the audio device */
	if ( SDL_OpenAudio(&desired, 0) < 0 ){
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}
	
	InitAudBuffers(desired.samples);
	SDL_PauseAudio(0);
}
void TermAudio()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}

void sdl_fill_audio(void *userdata, Uint8 *stream, int len)
{
	u8* rb=GetReadBuffer();
	speed_limit.Set();
	if (rb)
	{
		memcpy(stream,rb,len);
	}
	else
	{
		memset(stream,0,len);
		//printf("SDLAudioStream : out of buffers ...\n");
	}

	if (len!=buffersz)
		printf("Error [sdl_fill_audio]: len!=buffersz\n");
}