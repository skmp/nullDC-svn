#pragma once
#include "drkPvr.h"

void  ta_alloc_init();
void* ta_alloc(size_t Size,u8 align);
void  ta_alloc_print_stats();
void  ta_alloc_free_all();
void ta_alloc_release_all();

template<class T>
T* falloc()
{
	return (T*)ta_alloc(sizeof(T),1);
}

template<class T>
T* falloc(u8 align)
{
	return (T*)ta_alloc(sizeof(T),align);
}
