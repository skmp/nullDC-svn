////////////////////////////////////////////////////////////////////////////////////////
/// @file  cacheTextures.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
#ifndef CACHETEXTURES_H_
#define CACHETEXTURES_H_

#ifndef XBOX
#include<map>
using namespace std;


////////////////////////////////////////////////////////////////////////////////////////
/// @class  CCacheTextures
/// @brief  
////////////////////////////////////////////////////////////////////////////////////////
class CCacheTextures
{
public:
	CCacheTextures () : m_bInit(false)     {}
	~CCacheTextures()                      { CCacheTextures::End(); }

	TError  Init           (IDirect3DDevice9* pDevice);
	bool    IsOk           () const                { return m_bInit; }
	void    End            ();
	

	struct TTextureInfo
	{
		enum 
		{
			F_VQ_TEXTURE = 1,
			F_TWIDLE = 2,
		};

		DWORD uWidth;
		DWORD uHeight;
		DWORD uLevels;
		DWORD uFormatTexture;
		DWORD uFlags;
		unsigned __int64 uHash;		
	};

	IDirect3DTexture9* GetTexture(const TTextureInfo* pTexInfo, void* pBuffer);

	struct TTexture
	{
		enum
		{
			F_DIRTY = 1,			
		};
		DWORD uFlags;
		__int64 uHash;
		DWORD uAddress;
		DWORD uFrame;
    D3DFORMAT uD3DFormat;
		IDirect3DTexture9* pTextureD3D;
    DWORD aOffsets[4];
    DWORD aData[4];
    DWORD aOldData[4];
    void* pBuffer;
	};

	static void InvalidateTexture(void* pData, void* pAux);

	void StartFrame();
	void EndFrame();

  void UpdatePalette(int _iMode, int _iNumEntries, const void* pBuffer);

  void SetStride(DWORD uStride);

  void ReloadTextures();
private:
	bool                m_bInit;
  IDirect3DDevice9*   m_pDevice;
	DWORD		m_uCurrentMemTextures;
  DWORD   m_uStride;

  

  int m_TwiddleTable[1024];
  SHORT m_TableUnpack2x4to2x8[256];
  BYTE m_TableUnpack4to8[16];
  BYTE m_TableUnpack5to8[32];
  BYTE m_TableUnpack6to8[64];

  DWORD m_Palette[256];

  void InitializeDecoder();

  void Detwiddle8(DWORD uWidth, DWORD uHeight, const BYTE* pBufferSrc, BYTE* pBufferDst);
  void Detwiddle16(DWORD uWidth, DWORD uHeight, const WORD* pBufferSrc, WORD* pBufferDst);


	map<unsigned __int64,TTexture> m_hashTextures;	
	list<TTexture*> m_listTexturesDirty;	

	void UpdateTextureVQ_16(IDirect3DTexture9* pTextureD3D, const TTextureInfo* pTexInfo, const void* pBuffer);
	void UpdateTexture_16(IDirect3DTexture9* pTextureD3D, const TTextureInfo* pTexInfo, const void* pBuffer);
  void UpdateTexture_YUV(IDirect3DTexture9* pTextureD3D, const TTextureInfo* pTexInfo, const void* pBuffer);


};

#endif

#endif
