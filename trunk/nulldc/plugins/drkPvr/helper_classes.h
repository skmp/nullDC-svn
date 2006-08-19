#pragma once

#define LIST_MAX_ALLOC_CHUNK ((1024*1024*4)/sizeof(T))	//max 4 mb alloc
#define LIST_FREE_STEP_CHUNK ((1024*128)/sizeof(T))		//128 kb steps for free :)

template <class T>
class List
{
public :
	T* data;
	u32 used;
	u32 size;

	u32 last_sizes[8];
	u32 last_size_index;
	u32 avg_sz;
	
	__declspec(noinline) void resize()
	{
		u32 new_size=used+4;
		//MAX 4mb increase each time :)
		if (new_size>(LIST_MAX_ALLOC_CHUNK))
			new_size=LIST_MAX_ALLOC_CHUNK;
		resize(used +new_size);
	}
	void resize(u32 new_size)
	{
		data=(T*)realloc(data,new_size*sizeof(T));
		size=new_size;
	}	
	List()
	{
		data=0;
		used=0;
		size=0;
		memset(last_sizes,0,sizeof(last_sizes));
		last_size_index=0;
		avg_sz=0;
	}
	~List()
	{
		if (data)
			free(data);
	}
	List(u32 pre_alloc)
	{
		data=0;
		used=0;
		size=0;
		resize(pre_alloc);
	}
	__forceinline T* Append()
	{
		if (used==size)
			resize();
		return &data[used++];
	}
	void Clear()
	{
		u32 ls=last_sizes[last_size_index];
		last_sizes[last_size_index]=used;
		last_size_index=(last_size_index+1)&7;

		avg_sz-=ls;
		avg_sz+=used;

		u32 real_avg=avg_sz/8;

		u32 avg_chunk_avg=real_avg/LIST_FREE_STEP_CHUNK;
		u32 used_chunk_avg=used/LIST_FREE_STEP_CHUNK;


		if (avg_chunk_avg<=used_chunk_avg)//try to free olny if we used less items this time (if not , we propably start an increase period)
		{
			u32 allocated_chunk=size/LIST_FREE_STEP_CHUNK;
			avg_chunk_avg++;
			if (allocated_chunk>avg_chunk_avg)
				resize(avg_chunk_avg*LIST_FREE_STEP_CHUNK);
		}

		used=0;
	}
	void Free()
	{
		Clear();
		free(data);
		data=0;
	}
};