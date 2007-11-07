#ifndef SOUNDBUFFERS_H
#define SOUNDBUFFERS_H

const int AICA_MAX_SIZE_SAMPLES_BUFFERPROCESSED = 441*2;  // 20ms


struct TBufferProcessed
{
	TBufferProcessed  () : m_iSamplesProcessed(0), m_pNext(NULL) {}

	int m_iSamplesProcessed;
	signed short m_aData[AICA_MAX_SIZE_SAMPLES_BUFFERPROCESSED*2];
	TBufferProcessed* m_pNext;

	int GetFreeSamples    ()    { ASSERT(m_iSamplesProcessed <= AICA_MAX_SIZE_SAMPLES_BUFFERPROCESSED); return AICA_MAX_SIZE_SAMPLES_BUFFERPROCESSED-m_iSamplesProcessed; }
};

class CSoundBuffer
{

  TBufferProcessed*             m_pBuffers;
  CRITICAL_SECTION              m_CSBuffers;

public:
	void Init();
	void End();
	TBufferProcessed* ExtractBuffer();
	void InsertBuffer(TBufferProcessed* pBuffer);
	void Lock();
	void Unlock();
	int Size();

};

#endif