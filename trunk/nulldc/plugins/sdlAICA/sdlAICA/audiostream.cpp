#include "audiostream.h"
#include "sdl_audiostream.h"
#include "ds_audiostream.h"


/*
WriteSample_FP* WriteSample;
WriteSamples1_FP* WriteSamples1;
WriteSamples2_FP* WriteSamples2;
*/
cResetEvent speed_limit(true,true);


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
	buff_count=settings.BufferCount+2;
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
	{
		//printf("READED - FAIL\n");
		return 0;
	}
	else
	{
		//printf("READED\n");
		u8* rv=(u8*)WriteBuffer[rbuffer%buff_count]->Data;
		rbuffer++;
		return rv;
	}
}

void WriteSample(s16 r, s16 l)
{
	if (WriteBuffer[wbuffer%buff_count]->WriteSample(r,l))
	{
		//printf("Writen\n");
		//if we have all buffers filled , were gona replace the last one ;)
		if ((wbuffer-rbuffer)<buff_count)
			wbuffer++;
		else
		{
			//printf("Too many samples , waiting for audio ...\n");
			if (settings.LimitFPS)
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
void WriteSamples1(s16* r , s16* l , u32 sample_count)
{
	for (u32 i=0;i<sample_count;i++)
		WriteSample(r[i],l[i]);
}
void WriteSamples2(s16* rl , u32 sample_count)
{
	for (u32 i=0;i<(sample_count*2);i+=2)
		WriteSample(rl[i|0],rl[i|1]);
}

void InitAudio()
{
	InitAudBuffers(settings.BufferSize);
	if (settings.SoundRenderer)
	{
		//DSound
		ds_InitAudio();
	}
	else
	{
		//SDL
		sdl_InitAudio();
	}
}
void TermAudio()
{
	if (settings.SoundRenderer)
	{
		ds_TermAudio();
	}
	else
	{
		sdl_TermAudio();
	}
}
