#include "..\stdafx.h"
#include "SoundBuffers.h"




void CSoundBuffer::Init()
{
	InitializeCriticalSection(&m_CSBuffers);
	m_pBuffers=NULL;
}


void CSoundBuffer::End()
{
	TBufferProcessed* pBufferProcessed = ExtractBuffer();
	while (pBufferProcessed)
	{
		TBufferProcessed* pTemp = pBufferProcessed;
		pBufferProcessed = ExtractBuffer();
		delete pTemp;
	}
}

TBufferProcessed* CSoundBuffer::ExtractBuffer()
{
	TBufferProcessed* pBuffer = NULL;
	Lock();
	if ( m_pBuffers)
	{
		pBuffer = m_pBuffers;
		m_pBuffers=m_pBuffers->m_pNext;
		pBuffer->m_pNext = NULL;
	}
	Unlock();
	return pBuffer;
}

void CSoundBuffer::InsertBuffer(TBufferProcessed *pBuffer)
{
	Lock();
	pBuffer->m_pNext=NULL;
	if ( !m_pBuffers)
	{
		m_pBuffers=pBuffer;
	}
	else
	{
		TBufferProcessed* pBufferAux=m_pBuffers;
		while(pBufferAux->m_pNext)
			pBufferAux=pBufferAux->m_pNext;
		pBufferAux->m_pNext=pBuffer;
	}
	Unlock();
}

void CSoundBuffer::Lock()
{
	EnterCriticalSection(&m_CSBuffers);
}

void CSoundBuffer::Unlock()
{
	LeaveCriticalSection(&m_CSBuffers);
}

int CSoundBuffer::Size()
{
	Lock();
	int iBlocks = 0;
	TBufferProcessed* pTemp = m_pBuffers;
	while (pTemp)
	{
		iBlocks++;
		pTemp = pTemp->m_pNext;
	}
	Unlock();
	return iBlocks;
}