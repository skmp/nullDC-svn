#include "..\stdafx.h"

#include "CSound.h"

void CSound::SetVol(int vol)
{
	//vol range 0-39, convert to DSBVOLUME_MIN-DSBVOLUME-MAX
	//convert to attenuation
	int att=78-(2*vol);
//	int err;
	if(!Channels[0])
		return;
	for(int i=0;i<2;++i)
		if(Channels[i])
			Channels[i]->SetVolume(-att*100);
}

void CSound::SetVol(int vol,int chan)
{
	//vol range 0-39, convert to DSBVOLUME_MIN-DSBVOLUME-MAX
	//convert to attenuation
	int att=78-(2*vol);
	int err;
	err=Channels[chan]->SetVolume(-att*100);
}


int CSound::Init(CGUI *_GUI,int Samplerate,float Fps,int bufcount)
{
	DSBUFFERDESC dsbd;
	int err;
	GUI=_GUI;
	SysFPS=Fps;
	buffercount=bufcount;
	SampleRate=Samplerate;
	DEBUGMESSAGE("Initializing Directsound");
	err=DirectSoundCreate(NULL,&lpDS,NULL);
	if(FAILED(err))
	{
		DEBUGMESSAGE("Unable to create DirectSound");
		return 0;
	}
	err=lpDS->SetCooperativeLevel(GUI->GetWindow(),DSSCL_PRIORITY);
	if(FAILED(err))
	{
		DEBUGMESSAGE("Unable to set Cooplevel");
		return 0;
	}
	ZeroMemory(&dsbd,sizeof(dsbd));
	dsbd.dwSize=sizeof(dsbd);
	dsbd.dwFlags=DSBCAPS_CTRLVOLUME;
	err=lpDS->CreateSoundBuffer(&dsbd,&lpDSBPrimary,0);
	if(FAILED(err))
	{
		DEBUGMESSAGE("Unable to create Primary buffer");
		return 0;
	}

	WAVEFORMATEX wfx;
	ZeroMemory(&wfx,sizeof(wfx));
	wfx.wFormatTag=WAVE_FORMAT_PCM;
	wfx.nChannels=2;
	wfx.wBitsPerSample=16;
	wfx.nSamplesPerSec=Samplerate;
	wfx.nBlockAlign = 4;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	err=lpDSBPrimary->SetFormat(&wfx); 
	err=lpDSBPrimary->Play(0,0,DSBPLAY_LOOPING);
	Reinited=true;
	return 1;
}

void CSound::Destroy()
{
	if(!lpDSBPrimary)
		return;
	lpDSBPrimary->Stop();
	lpDSBPrimary->Release();
	lpDS->Release();
}


int CSound::CreateChannel(int nch,int freq,int vol,int pos)
{
	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd,sizeof(dsbd));
	int err;
	dsbd.dwSize=sizeof(dsbd);
#ifdef XBOX
	dsbd.dwFlags=DSBCAPS_CTRLVOLUME;
#else
	dsbd.dwFlags=DSBCAPS_CTRLVOLUME|DSBCAPS_GETCURRENTPOSITION2/*|DSBCAPS_GLOBALFOCUS*/|DSBCAPS_STICKYFOCUS;
#endif
	WAVEFORMATEX wfx;
	ZeroMemory(&wfx,sizeof(wfx));
	wfx.wFormatTag=WAVE_FORMAT_PCM;
	wfx.nChannels=1;
	wfx.wBitsPerSample=16;
	wfx.nSamplesPerSec=freq;
	wfx.nBlockAlign = 2;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	sizebuf=(wfx.nAvgBytesPerSec/int(SysFPS))+16;
	if(sizebuf&1)
		--sizebuf;
	sizebuf = (sizebuf + 15) & ~15;
	nbufs=buffercount;

	bsize=nbufs*sizebuf;

	NGaps=nbufs;
	GapSize=sizebuf;
	NextGap=0;

	vol=39;

	dsbd.dwBufferBytes=bsize;
	
	dsbd.lpwfxFormat=&wfx;
	err=lpDS->CreateSoundBuffer(&dsbd,&(Channels[nch]),0);
	if(FAILED(err))
	{
		DEBUGMESSAGE("Unable to create secondary buffer");
		return 0;
	}
	err=Channels[nch]->SetVolume(DSBVOLUME_MAX);
#ifndef XBOX
	if(pos==0)
		Channels[nch]->SetPan(DSBPAN_LEFT);
	if(pos==255)
		Channels[nch]->SetPan(DSBPAN_RIGHT);
#endif
	poss[nch]=0;
	Running[nch]=0;
	SetVol(vol,nch);
	LastLen=freq;
	SoundLen=sizebuf;
	return sizebuf;
}

int CSound::CreateChannelStereo(int nch,int freq,int vol,int pos)
{
	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd,sizeof(dsbd));
	int err;
	dsbd.dwSize=sizeof(dsbd);
#ifdef XBOX
	dsbd.dwFlags=DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY;
#else
	dsbd.dwFlags=DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY|DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_GLOBALFOCUS;
#endif
	

	WAVEFORMATEX wfx;
	ZeroMemory(&wfx,sizeof(wfx));
	wfx.wFormatTag=WAVE_FORMAT_PCM;
	wfx.nChannels=2;
	wfx.wBitsPerSample=16;
	wfx.nSamplesPerSec=freq;
	wfx.nBlockAlign = 4;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

//	sizebuf=(wfx.nAvgBytesPerSec/60.0)+16;
	sizebuf=(wfx.nAvgBytesPerSec/int(SysFPS))+16;
	if(sizebuf&1)
		--sizebuf;
	sizebuf = (sizebuf + 15) & ~15;
	nbufs=buffercount;

	bsize=nbufs*sizebuf;

	NGaps=nbufs;
	GapSize=sizebuf;
	NextGap=0;


	dsbd.dwBufferBytes=bsize;
	
	dsbd.lpwfxFormat=&wfx;
	err=lpDS->CreateSoundBuffer(&dsbd,&(Channels[nch]),0);
	err=Channels[nch]->SetVolume(DSBVOLUME_MAX);
	poss[nch]=0;
	Running[nch]=0;
	SetVol(vol,nch);
	LastLen=freq;
	
	err=Channels[nch]->Play(0,0,DSBPLAY_LOOPING);
	Running[nch]=1;
	return sizebuf;
}


void CSound::DeleteChannel(int nch)
{
	if(!Channels[nch])
		return;

	Channels[nch]->Stop();
	Channels[nch]->Release();
	Channels[nch]=NULL;
}

int CSound::GetBufferMaxWrite(int r,int w,int pos)
{
	DWORD dwMaxSize;
	if (pos < r)
    {
		dwMaxSize = r - pos;
    }
    else
    {
        dwMaxSize = bsize - pos + r;
	}
	return dwMaxSize;

}
bool CSound::EnoughSpace(int r,int w,int pos,int len)
{
	
	actpos=pos;
	actrpos=r;
	nextpos=pos+len;
	nextrpos=r+len;

	if(nextpos>=bsize)
		nextpos-=bsize;
	if(nextrpos>=bsize)
		nextrpos-=bsize;
	
	if(nextrpos>r)
	{
		if(pos>=r && pos<nextrpos)
			return false;
		if(nextpos>r && nextpos<=nextrpos)
			return false;
	}
	if(nextrpos<r)
	{
		if(pos>nextpos)
			return false;
		if(pos<nextrpos)
			return false;
		if(nextpos>=r)
			return false;
	}
	return true;
}

void CSound::AddChannelData(int nch,signed short *data,unsigned int len)
{
	DWORD r,w;
	DWORD curpos,writepos;
	int in=poss[nch];
	unsigned char *d=(unsigned char *)data;
	DWORD l1,l2;
	void *w1,*w2;
	int err;
	static int nn=0;
	
	Reinited=false;

	LastLen=len;


	
	if(!nch && Running[nch] && GUI->FrameLimit)
	{
		do
		{
			Channels[nch]->GetCurrentPosition((LPDWORD) &r,(LPDWORD) &w);
//			Sleep(2);
		} while(!EnoughSpace(r,w,in+2,len) && !Reinited);
	}	

	nn++;

	

	Channels[nch]->Lock(in,len,&w1,(LPDWORD) &l1,&w2,(LPDWORD) &l2,0);
	memcpy(w1,d,l1);
	memcpy(w2,d+l1,l2);
	Channels[nch]->Unlock(w1,l1,w2,l2);
	in+=len;
	poss[nch]=(in)%(bsize);

	NextGap++;
	NextGap=NextGap%NGaps;


	if(!Running[nch])
	{
		err=Channels[nch]->Play(0,0,DSBPLAY_LOOPING);
		Running[nch]=1;
		Channels[nch]->GetCurrentPosition((LPDWORD) &curpos,(LPDWORD) &writepos);
		in=writepos;
	}
	
}

void CSound::Mute(bool mute)
{
	static long lastvol;
	if(mute)
	{
		for(int i=0;i<2;++i)
			if(Channels[i])
				Channels[i]->SetVolume(DSBVOLUME_MIN);
	}
	else
	{
		SetVol(vol);
	}
}
