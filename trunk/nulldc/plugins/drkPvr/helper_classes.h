#pragma once

#define LIST_MAX_ALLOC_CHUNK ((1024*1024*4)/sizeof(T))	//max 4 mb alloc
#define LIST_FREE_STEP_CHUNK ((1024*128*8)/sizeof(T))		//128 kb steps for free :)

template <class T,u32 MaxAllocChunk=LIST_MAX_ALLOC_CHUNK,u32 FreeStepChunk=LIST_FREE_STEP_CHUNK>
class List
{
public :
	T* data;
	u32 used;
	u32 size;

	u32 last_sizes[8];
	u32 last_size_index;
	u32 avg_sz;
	
	__declspec(noinline) void resize(u32 min_size=1)
	{
		//verify(size!=0x000004d7);
		u32 new_size=used+4+min_size;
		//MAX 4mb increase each time :)
		if (new_size>(MaxAllocChunk))
			new_size=MaxAllocChunk;
		resize_2(used +new_size);
	}
	void resize_2(u32 new_size)
	{
		//verify(size!=0x000004d7);
		//printf("resize_2 , size = %d:%d\n",size,new_size);
		data=(T*)realloc(data,new_size*sizeof(T));
		size=new_size;
		//printf("resize_2 , size = %d\n",size);
	}
	void Init(u32 pre_alloc=0)
	{
		data=0;
		used=0;
		size=0;
		memset(last_sizes,0,sizeof(last_sizes));
		last_size_index=0;
		avg_sz=0;
		if (pre_alloc)
			resize(pre_alloc);
	}

	__forceinline T* Append()
	{
		if (used==size)
			resize();
		return &data[used++];
	}
	__forceinline T* Append(u32 count)
	{
		if ((used+count)>=size)
			resize(count);
		T* rv=&data[used];
		used+=count;
		return rv;
	}
	void Clear()
	{
		u32 ls=last_sizes[last_size_index];
		last_sizes[last_size_index]=used;
		last_size_index=(last_size_index+1)&7;

		avg_sz-=ls;
		avg_sz+=used;

		u32 real_avg=avg_sz/8;
		if (used<real_avg)
		{
			//consider resizing ONLY if the used is less than the average
			//if diff is > FreeStepChunk
			u32 used_top=used+FreeStepChunk;

			if (used_top<real_avg)
			{
				resize(real_avg + FreeStepChunk/2);
			}
		}
		//printf("Clear , size = %d:%d\n",size,used);
		used=0;
	}
	void Free()
	{
		Clear();
		free(data);
		data=0;
	}
};
//Windoze code
//Threads
#define THREADCALL __stdcall

typedef  u32 THREADCALL ThreadEntryFP(void* param);
typedef void* THREADHANDLE;

class cThread
{
private:
	ThreadEntryFP* Entry;
	void* param;
	THREADHANDLE hThread;
public :
	cThread(ThreadEntryFP* function,void* param);
	~cThread();
	//Simple thread functions
	void Start();
	void Suspend();
	void WaitToEnd(u32 msec);
};

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