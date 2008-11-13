#include "audiostream.h"
#include "sdl_audiostream.h"
#include "ds_audiostream.h"


/*
WriteSample_FP* WriteSample;
WriteSamples1_FP* WriteSamples1;
WriteSamples2_FP* WriteSamples2;
*/
cResetEvent speed_limit(true,true);

struct SoundSample { s16 l;s16 r; };
SoundSample* RingBuffer;
SoundSample* TempBuffer;
u32 RingBufferByteSize;
u32 RingBufferSampleCount;

volatile u32 WritePtr;	//last WRITEN sample
volatile u32 ReadPtr;	//next sample to read
u32 BufferByteSz;
u32 BufferSampleCount;

#ifdef LOG_SOUND
WaveWriter rawout("d:\\aica_out.wav");
#endif
void InitAudBuffers(u32 buff_samples)
{
	BufferSampleCount=buff_samples;
	BufferByteSz=BufferSampleCount*sizeof(s16)*2;
	RingBufferSampleCount=(settings.BufferCount+2)*BufferSampleCount;
	RingBufferByteSize=RingBufferSampleCount*sizeof(s16)*2;
	WritePtr=0;
	ReadPtr=0;
	RingBuffer=(SoundSample*)malloc(RingBufferByteSize);
	TempBuffer=(SoundSample*)malloc(RingBufferByteSize);

	//first samlpe is actualy used, so make sure its 0 ...
	RingBuffer[0].r=0;
	RingBuffer[0].l=0;
}
u32 asRingUsedCount()
{
	if (WritePtr>=ReadPtr)
		return WritePtr-ReadPtr;
	else
		return RingBufferSampleCount-(ReadPtr-WritePtr);
	//s32 sz=(WritePtr+1)%RingBufferSampleCount-ReadPtr;
	//return sz<0?sz+RingBufferSampleCount:sz;
}
u32 asRingFreeCount()
{
	return RingBufferSampleCount-asRingUsedCount()-1;
}
//Return read buffer , 0 if none
bool asRingRead(u8* dst,u32 sz)
{
	if (sz==0)
		sz=BufferSampleCount;
	if (asRingUsedCount()>=sz)
	{
		const u32 ptr=ReadPtr;
		if ((ReadPtr+sz)<=RingBufferSampleCount)
		{
			//R ... W, just copy the sz :)
			memcpy(dst,&RingBuffer[ptr],sz*sizeof(s16)*2);
		}
		else
		{
			//...W R...., just copy the sz :)
			const u32 szhi=RingBufferSampleCount-ptr;
			const u32 szlow=sz-szhi;

			memcpy(dst,&RingBuffer[ptr],szhi*sizeof(s16)*2);
			dst+=szhi*sizeof(s16)*2;
			memcpy(dst,&RingBuffer[0],szlow*sizeof(s16)*2);
		}
		ReadPtr=(ptr+sz)%RingBufferSampleCount;
		
		speed_limit.Set();
		return true;
	}
	else
	{
		//printf("ONLY %d used, rd=%d, wt=%d\n",asRingUsedCount(),ReadPtr,WritePtr);
		return false;
	}
}
bool asRingStretchRead(u8* dst,u32 sz)
{
	if (sz==1)
		return false; //not supported ...

	if (sz==0)
		sz=BufferSampleCount;

	u32 used=asRingUsedCount();
	if (used==0)
		return false;

	if (settings.LimitFPS)
	{
		if (used>=sz)
			return asRingRead(dst,sz);
		else
		{
			bool rv=asRingRead((u8*)TempBuffer,used);
			verify(rv==true);//can't be false ...
			u32 steps=used*sz*2;
			
			//this does change pitch, but for a few samples it won't matter much ...
			for (u32 cs=sz-1;cs<steps;cs+=used*2)
			{
				u32 srcidx=cs/(sz*2) ;
				verify(srcidx<used);

				*(SoundSample*)dst=TempBuffer[srcidx];
				dst+=sizeof(SoundSample);
			}
			return true;
		}
	}
		return asRingRead(dst,sz);

}
void WriteSample(s16 r, s16 l)
{
	#ifdef LOG_SOUND
	rawout.Write(l,r);
	#endif

	speed_limit.Reset();
	if (!asRingFreeCount())
	{
		if (settings.LimitFPS)
		{
			speed_limit.Wait();
		}
		else
			return;
	}

	const u32 ptr=(WritePtr+1)%RingBufferSampleCount;
	RingBuffer[ptr].r=r;
	RingBuffer[ptr].l=l;
	WritePtr=ptr;
	//if (0==(WritePtr&255))
	//printf("write done %d %d \n",ReadPtr,WritePtr);
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
