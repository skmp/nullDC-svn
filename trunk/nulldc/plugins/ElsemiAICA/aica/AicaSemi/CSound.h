#ifndef SOUND_H
#define SOUND_H

#define DIRECTSOUND_VERSION 0x700
#include "mmsystem.h"
#include "dsound.h"

//#define DEBUGMESSAGE(msg) MessageBox(NULL,msg,"Error",MB_OK);
#define DEBUGMESSAGE(msg)

extern HWND g_hWnd;

class CGUI
{
public:
	bool FrameLimit;
	bool AutoDeLimit;
	HWND GetWindow() { return g_hWnd; };
};

class CSound
{
private:
	//CGUI *GUI;

	float SysFPS;
	int buffercount;
	LPDIRECTSOUND lpDS;
	LPDIRECTSOUNDBUFFER lpDSBPrimary;
	LPDIRECTSOUNDBUFFER Channels[2];
	LPDIRECTSOUNDNOTIFY lpDSN;
	HANDLE Events[2];
	int poss[2];
	bool Running[2];
	int bsize;
	int sizebuf;
	int nbufs;
	int LastLen;
	int NGaps;
	int NextGap;
	int GapSize;
	bool Reinited;
	int nextpos,nextrpos,actpos,actrpos;
	int vol;


	int GetBufferMaxWrite(int r,int w,int pos);
	bool EnoughSpace(int r,int w,int pos,int len);

public:
	signed short Buffer[2][44100*2];
	int SampleRate;
	int SoundLen;

	void SetVol(int vol);
	void SetVol(int vol,int chan);
	int Init(CGUI *_GUI,int Samplerate,float Fps,int bufcount);
	void Destroy();
	int CreateChannel(int nch,int freq,int vol,int pos);
	int CreateChannelStereo(int nch,int freq,int vol,int pos);
	void DeleteChannel(int nch);
	void AddChannelData(int nch,UINT16 *data,UINT32 len);
	void Mute(bool mute);
};

#endif
