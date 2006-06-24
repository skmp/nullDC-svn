////////////////////////////////////////////////////////////////////////////////////////
/// @file  cacheTextures.cpp
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////
//#include "stdafx.h"

DWORD SH4HWRegistersGetValue(DWORD uAddress,DWORD uMask,DWORD uShift)
{
	DWORD* pBuffer = SH4HWRegistersGetPtr(uAddress);
	if (!pBuffer)
		return 0;
	return (((*pBuffer)&uMask)>>uShift);
}
void SH4WriteLongMemory(const DWORD uAddress,const DWORD uData)
{
	printf("SH4WriteLongMemory(0x%X,0x%X)\n",uAddress,uData);
}

#ifndef XBOX


//extern CVirtualMemHandler* g_pTextureMemoryMemHandler;


//extern CDisplayDevice* g_pDisplayDevice;



void CCacheTextures::InitializeDecoder()
{  
  // intitialize TwiddleTable
  for(int x=0; x<1024; x++)
    m_TwiddleTable[x] = (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)|((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9);

  //tables to speed up decompression
  for (int i=0; i<16; i++)
    for (int j=0; j<16; j++)
      m_TableUnpack2x4to2x8[i*16+j] = (j*255/15) + 256*(i*255/15);
  for (int i=0; i<16; i++)
    m_TableUnpack4to8[i] = i*255/15;
  for (int i=0; i<32; i++)
    m_TableUnpack5to8[i] = i*255/31;
  for (int i=0; i<64; i++)
    m_TableUnpack6to8[i] = i*255/63;
}


////////////////////////////////////////////////////////////////////////////////////////
/// @fn      TError CCacheTextures::Init()
/// @return  RET_OK if successful.
/// @brief   Initializes the class. Required before any other method.
////////////////////////////////////////////////////////////////////////////////////////
TError CCacheTextures::Init(IDirect3DDevice9*   pDevice)
{
	TError Error = RET_OK;

	End();


  m_uStride = 0;

  m_pDevice = pDevice;
	// Begin Reset
	m_hashTextures.clear();
  m_listTexturesDirty.clear();
	m_uCurrentMemTextures = 0;
	// End Reset

  InitializeDecoder();

	// Alloc Stuff

	m_bInit = true;
	if (Error != RET_OK)
		CCacheTextures::End();

	return Error;
}

////////////////////////////////////////////////////////////////////////////////////////
/// @fn      void CCacheTextures::End()
/// @brief   Frees all class memory.
////////////////////////////////////////////////////////////////////////////////////////
void CCacheTextures::End()
{
	if (IsOk())
	{
		// Dispose stuff
		map<unsigned __int64,TTexture>::const_iterator it;
		
		for (it=m_hashTextures.begin();it!=m_hashTextures.end();++it)
		{
			IDirect3DTexture9* pTexture = it->second.pTextureD3D;

			if (pTexture)
				pTexture->Release();
		}

		m_hashTextures.clear();
    m_listTexturesDirty.clear();
		m_bInit = false;
	}
}



void CCacheTextures::InvalidateTexture(void* pData, void* pAux)
{
	CCacheTextures::TTexture* pTexture = (CCacheTextures::TTexture*) pData;
	CCacheTextures* pCacheTextures = (CCacheTextures*) pAux;

	pTexture->uFlags = CCacheTextures::TTexture::F_DIRTY;

	pCacheTextures->m_listTexturesDirty.push_front(pTexture);
}


void CCacheTextures::Detwiddle8(DWORD uWidth, DWORD uHeight, const BYTE* pBufferSrc, BYTE* pBufferDst)
{
  int iMin = min(uWidth, uHeight);
  int iMask = iMin - 1;

  for(DWORD y=0; y<uHeight; y++) 
  {		
    for(DWORD x=0; x<uWidth; x++) 
    {
      int iTwiddleOffset = (m_TwiddleTable[y&iMask]|(m_TwiddleTable[x&iMask]<<1))	+ (x/iMin + y/iMin)*(iMin*iMin);				
      BYTE iColor = *(pBufferSrc + iTwiddleOffset);
      *pBufferDst++ = BYTE(iColor);      
    }
  }		
}

void CCacheTextures::Detwiddle16(DWORD uWidth, DWORD uHeight, const WORD* pBufferSrc, WORD* pBufferDst)
{
  int iMin = min(uWidth, uHeight);
  int iMask = iMin - 1;

  for(DWORD y=0; y<uHeight; y++) 
  {		
    for(DWORD x=0; x<uWidth; x++) 
    {
      int iTwiddleOffset = (m_TwiddleTable[y&iMask]|(m_TwiddleTable[x&iMask]<<1))	+ (x/iMin + y/iMin)*(iMin*iMin);				
      WORD iColor = *(pBufferSrc + iTwiddleOffset);
      *pBufferDst++ = iColor;      
    }
  }		
}

static void SetOffsets(const CCacheTextures::TTextureInfo* pTexInfo,CCacheTextures::TTexture* pTexture)
{

  DWORD uBytes;
  if (pTexInfo->uFormatTexture == PVR_TXRFMT_4BPP_PALETTE)
    uBytes = pTexInfo->uHeight*pTexInfo->uWidth/2;
  else if (pTexInfo->uFormatTexture == PVR_TXRFMT_8BPP_PALETTE)
    uBytes = pTexInfo->uHeight*pTexInfo->uWidth;
  else
    uBytes = pTexInfo->uHeight*pTexInfo->uWidth*2;

  pTexture->aOffsets[0] = 0;
  pTexture->aOffsets[1] = 4;
  pTexture->aOffsets[2] = 8;
  pTexture->aOffsets[3] = 12;

}

IDirect3DTexture9* CCacheTextures::GetTexture(const TTextureInfo* pTexInfo, void* pBuffer)
{
	map<unsigned __int64, TTexture>::iterator it;
	it = m_hashTextures.find(pTexInfo->uHash);
	bool bDirty = false;


  D3DFORMAT d3dFormat;

	IDirect3DTexture9* pTextureD3D = NULL;
	if (it!=m_hashTextures.end())
	{
		TTexture* pTextureAux = &it->second;
		pTextureD3D = pTextureAux->pTextureD3D;		

    d3dFormat = pTextureAux->uD3DFormat;
/*
    int i;
    bDirty = false;			
    for (i=0;!bDirty && i<4;i++)
    {
      if (*((DWORD*)(pTextureAux->aOffsets[i]+(BYTE*)pBuffer)) != pTextureAux->aData[i])
      {
        bDirty =  true;
        pTextureAux->uFlags = 0;
      }
    }*/
    
    
    
    if (pTextureAux->uFlags & TTexture::F_DIRTY)
    {
      list<TTexture*>::iterator it = find(m_listTexturesDirty.begin(),m_listTexturesDirty.end(),pTextureAux);

      ASSERT(it != m_listTexturesDirty.end());
      m_listTexturesDirty.erase(it);			
      pTextureAux->uFlags = 0;
      bDirty = true;			
    }	

	}
	
	if (bDirty || !pTextureD3D)
	{		
		DWORD uBytes = 0;
    DWORD uUsage;

    if (pTexInfo->uLevels)
      uUsage  = D3DUSAGE_AUTOGENMIPMAP;
    else
      uUsage = 0;
    

		if (pTexInfo->uFormatTexture == PVR_TXRFMT_8BPP_PALETTE)
		{
			if (!pTextureD3D)
				m_pDevice->CreateTexture(pTexInfo->uWidth, pTexInfo->uHeight,1,uUsage,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&pTextureD3D,NULL);
			ASSERT(pTextureD3D);

      d3dFormat = D3DFMT_A8R8G8B8;

      if (pTextureD3D)
      {      
        DWORD uOffsetMipMap = 0;

        if (pTexInfo->uLevels)
        {
          DWORD uSize = 3;

          DWORD i;

          i = 1;
          while (i<pTexInfo->uWidth)
          {
            uSize+=i*i;
            i<<=1;
          }

          uOffsetMipMap = uSize;
        }


        int iMin = min(pTexInfo->uWidth, pTexInfo->uHeight);
        int iMask = iMin - 1;

        D3DLOCKED_RECT lockedRect;						

        uBytes = pTexInfo->uWidth*pTexInfo->uHeight;

        pTextureD3D->LockRect(0,&lockedRect,NULL,D3DLOCK_NOSYSLOCK);

        void* _pDest = lockedRect.pBits;

        int iByteProRow = lockedRect.Pitch/4;
        for(DWORD y=0; y<pTexInfo->uHeight; y++) 
        {		
          DWORD *pDest = ((DWORD*)_pDest) + (y * iByteProRow);
          for(DWORD x=0; x<pTexInfo->uWidth; x++) 
          { 									
            int iTwiddleOffset = (m_TwiddleTable[y&iMask]|(m_TwiddleTable[x&iMask]<<1))
              + (x/iMin + y/iMin)*(iMin*iMin);

            BYTE iColor = *((unsigned char*)pBuffer + uOffsetMipMap + iTwiddleOffset);          
            *pDest++ = m_Palette[iColor];
          }
        }


        pTextureD3D->UnlockRect(0);
      }
			
		}
    else if (pTexInfo->uFormatTexture == PVR_TXRFMT_4BPP_PALETTE)
    {      
      if (!pTextureD3D)
        m_pDevice->CreateTexture(pTexInfo->uWidth, pTexInfo->uHeight,1,uUsage,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&pTextureD3D,NULL);

      d3dFormat = D3DFMT_A8R8G8B8;

      if (pTextureD3D)
      {              
        D3DLOCKED_RECT lockedRect;						

        DWORD uOffsetMipMap = 0;

        if (pTexInfo->uLevels)
        {
          DWORD uSize = 1;

          DWORD i;

          i = 1;
          while (i<pTexInfo->uWidth)
          {
            uSize+=i*i;
            i<<=1;
          }

          uOffsetMipMap = uSize/2 + 1;
        }


        uBytes = pTexInfo->uWidth*pTexInfo->uHeight/2;

        pTextureD3D->LockRect(0,&lockedRect,NULL,D3DLOCK_NOSYSLOCK);

        int iMin = min(pTexInfo->uWidth, pTexInfo->uHeight);
        int iMask = iMin - 1;

        void* _pDest = lockedRect.pBits;

        int iByteProRow = lockedRect.Pitch/4;
        for(DWORD y=0; y<pTexInfo->uHeight; y+=2) 
        {
          DWORD *pDest = ((DWORD*)_pDest) + (y * iByteProRow);
          for(DWORD x=0; x<pTexInfo->uWidth; x+=2) 
          {
            int iTwiddleOffset = (m_TwiddleTable[(y>>1)&(iMask>>1)]|(m_TwiddleTable[(x>>1)&(iMask>>1)]<<1))
              + (((x>>1) / (iMin>>1)) + ((y>>1) / (iMin>>1))) * ((iMin>>1)*(iMin>>1));				

            unsigned short iColor = *(unsigned short*)((unsigned char*)pBuffer + uOffsetMipMap + iTwiddleOffset*2);

            pDest[0]			 = (m_Palette[ iColor        & 0x0f]);
            pDest[1]			 = (m_Palette[(iColor >>  8) & 0x0f]);
            pDest[0+iByteProRow] = (m_Palette[(iColor >>  4) & 0x0f]);
            pDest[1+iByteProRow] = (m_Palette[(iColor >> 12) & 0x0f]);

            pDest+=2;
          }
        }

        pTextureD3D->UnlockRect(0);
      }
    }
		else if (pTexInfo->uFormatTexture <= PVR_TXRFMT_YUV422)
		{
	
			if (!pTextureD3D)
			{			
				switch (pTexInfo->uFormatTexture)
				{
				case PVR_TXRFMT_RGB565:
					d3dFormat = D3DFMT_R5G6B5;										break;
				case PVR_TXRFMT_ARGB4444:
					d3dFormat = D3DFMT_A4R4G4B4;									break;
				case PVR_TXRFMT_ARGB15555:
					d3dFormat = D3DFMT_A1R5G5B5;									break;
				case PVR_TXRFMT_YUV422:
					d3dFormat = D3DFMT_UYVY;											break;
          //d3dFormat = D3DFMT_UNKNOWN; break;
				}				

        pTextureD3D = NULL;
				m_pDevice->CreateTexture(pTexInfo->uWidth, pTexInfo->uHeight,1,uUsage,d3dFormat,D3DPOOL_MANAGED,&pTextureD3D,NULL);

        if (!pTextureD3D && pTexInfo->uFormatTexture == PVR_TXRFMT_YUV422)
        {
          d3dFormat = D3DFMT_A8R8G8B8;
          m_pDevice->CreateTexture(pTexInfo->uWidth, pTexInfo->uHeight,1,uUsage,d3dFormat,D3DPOOL_MANAGED,&pTextureD3D,NULL);
        }
			}

			uBytes = pTexInfo->uWidth*pTexInfo->uHeight*2;

      if (pTexInfo->uFlags & TTextureInfo::F_VQ_TEXTURE)
        uBytes = (pTexInfo->uWidth/2)*(pTexInfo->uHeight/2) + 2048;


			ASSERT(pTextureD3D);

			if (pTextureD3D)
			{			
        if (pTexInfo->uFormatTexture == PVR_TXRFMT_YUV422 && d3dFormat != D3DFMT_UYVY)
          UpdateTexture_YUV(pTextureD3D,pTexInfo,pBuffer);
        else
        {
          if (pTexInfo->uFlags & TTextureInfo::F_VQ_TEXTURE)
            UpdateTextureVQ_16(pTextureD3D,pTexInfo,pBuffer);
          else
            UpdateTexture_16(pTextureD3D,pTexInfo,pBuffer);
        }
      }
    }
		else
		{
			ASSERT(false);
			pTextureD3D = NULL;
		}

		if (!bDirty)
		{
			TTexture texture;
			texture.uFlags = 0;
			texture.pTextureD3D = pTextureD3D;
			texture.uHash = pTexInfo->uHash;
      texture.uD3DFormat = d3dFormat;
			m_uCurrentMemTextures+=uBytes;
      

			ASSERT(m_hashTextures.find(pTexInfo->uHash)== m_hashTextures.end());

			m_hashTextures[pTexInfo->uHash] = texture;			
		}

		it = m_hashTextures.find(pTexInfo->uHash);

		TTexture* pTextureAux = &it->second;

    if (!bDirty)
    {
      pTextureAux->pBuffer = pBuffer;
      SetOffsets(pTexInfo,pTextureAux);
    }

		pTextureAux->uFrame = CurrentFrame;
		pTextureAux->uAddress = (DWORD)pBuffer;

		
    /*
    pTextureAux->aData[0] = pTextureAux->uAddress;
    pTextureAux->aData[1] = pTextureAux->uFrame;
    pTextureAux->aData[2] = rand();
    pTextureAux->aData[3] = rand();

    pTextureAux->aOldData[0] = *((DWORD*)(pTextureAux->aOffsets[0]+(BYTE*)pBuffer));
    pTextureAux->aOldData[1] = *((DWORD*)(pTextureAux->aOffsets[1]+(BYTE*)pBuffer));
    pTextureAux->aOldData[2] = *((DWORD*)(pTextureAux->aOffsets[2]+(BYTE*)pBuffer));
    pTextureAux->aOldData[3] = *((DWORD*)(pTextureAux->aOffsets[3]+(BYTE*)pBuffer));
    *((DWORD*)(pTextureAux->aOffsets[0]+(BYTE*)pBuffer)) = pTextureAux->aData[0];
    *((DWORD*)(pTextureAux->aOffsets[1]+(BYTE*)pBuffer)) = pTextureAux->aData[1];
    *((DWORD*)(pTextureAux->aOffsets[2]+(BYTE*)pBuffer)) = pTextureAux->aData[2];
    *((DWORD*)(pTextureAux->aOffsets[3]+(BYTE*)pBuffer)) = pTextureAux->aData[3];*/
    
/*
    pTextureAux->aData[0] = *((DWORD*)(pTextureAux->aOffsets[0]+(BYTE*)pBuffer));
    pTextureAux->aData[1] = *((DWORD*)(pTextureAux->aOffsets[1]+(BYTE*)pBuffer));
    pTextureAux->aData[2] = *((DWORD*)(pTextureAux->aOffsets[2]+(BYTE*)pBuffer));
    pTextureAux->aData[3] = *((DWORD*)(pTextureAux->aOffsets[3]+(BYTE*)pBuffer));
*/

		
		if (pTextureD3D)
    {      
      //FIXME
			//g_pTextureMemoryMemHandler->ProtectMem(pBuffer,uBytes,InvalidateTexture,pTextureAux,this,NULL,true);
    }
	

  
	}

  m_uStride = 0;		

	
	return pTextureD3D;
}

void CCacheTextures::StartFrame()
{
}


void CCacheTextures::EndFrame()
{
 // g_pTextureMemoryMemHandler->Flush();
	
	while (m_listTexturesDirty.size())
	{
		TTexture* pTexture = m_listTexturesDirty.front();
    if(pTexture)
    {
      if(pTexture->pTextureD3D)
      {
        pTexture->pTextureD3D->Release();
        pTexture->pTextureD3D=0;
      }
		  m_hashTextures.erase(pTexture->uHash);
    }
		m_listTexturesDirty.pop_front();
	}

  /*
	if (m_uCurrentMemTextures > 1024*1024*16)// || GetAsyncKeyState('R'))
	{
		m_uCurrentMemTextures = 0;
		map<unsigned __int64,TTexture>::const_iterator it;

		for (it=m_hashTextures.begin();it!=m_hashTextures.end();++it)
		{
			IDirect3DTexture9* pTexture = it->second.pTextureD3D;

			if (pTexture)
      {
        pTexture->Release();

        const TTexture* pTextureAux = &it->second;        
        int i;
        bool bOk = true;
        for (i=0;bOk && i<4;i++)
        {
          if (*((DWORD*)(pTextureAux->aOffsets[i]+(BYTE*)pTextureAux->pBuffer)) != pTextureAux->aData[i])
            bOk = false;
        }

        if (bOk)
        {
          *((DWORD*)(pTextureAux->aOffsets[0]+(BYTE*)pTextureAux->pBuffer)) = pTextureAux->aOldData[0];
          *((DWORD*)(pTextureAux->aOffsets[1]+(BYTE*)pTextureAux->pBuffer)) = pTextureAux->aOldData[1];
          *((DWORD*)(pTextureAux->aOffsets[2]+(BYTE*)pTextureAux->pBuffer)) = pTextureAux->aOldData[2];
          *((DWORD*)(pTextureAux->aOffsets[3]+(BYTE*)pTextureAux->pBuffer)) = pTextureAux->aOldData[3];
        }
      }
				
		}

		m_hashTextures.clear();

	}*/


  if (false)
  {

	  //FIXME
    //g_pTextureMemoryMemHandler->Reset();
    map<unsigned __int64,TTexture>::const_iterator it;

    for (it=m_hashTextures.begin();it!=m_hashTextures.end();++it)
    {
      IDirect3DTexture9* pTexture = it->second.pTextureD3D;

      if (pTexture)
      {
        pTexture->Release();
      }
    }

    m_hashTextures.clear();
  }
}

void CCacheTextures::ReloadTextures()
{
	//FIXME
//  g_pTextureMemoryMemHandler->Reset();
  map<unsigned __int64,TTexture>::const_iterator it;

  for (it=m_hashTextures.begin();it!=m_hashTextures.end();++it)
  {
    IDirect3DTexture9* pTexture = it->second.pTextureD3D;

    if (pTexture)
    {
      pTexture->Release();
    }
  }

  m_listTexturesDirty.clear();
  m_hashTextures.clear();
}


void CCacheTextures::UpdateTextureVQ_16(IDirect3DTexture9* pTextureD3D, const TTextureInfo* pTexInfo, const void* pBuffer)
{
	


	DWORD uOffset = 0;

	if (pTexInfo->uLevels)
	{
		DWORD uSize = 1;

		DWORD i;

		i = 1;
		while (i<pTexInfo->uWidth/2)
		{
			uSize+=i*i;
			i<<=1;
		}

		uOffset = uSize;
	}

	BYTE* pBufferAux; 

	if (pTexInfo->uFlags & TTextureInfo::F_TWIDLE)
	{		

		pBufferAux = NEW(BYTE[(pTexInfo->uWidth/2)*(pTexInfo->uHeight/2)]);
		Detwiddle8(pTexInfo->uWidth/2, pTexInfo->uHeight/2,((const unsigned char*)pBuffer)+2048+uOffset, pBufferAux);
	}
	else
		pBufferAux = ((BYTE*)pBuffer)+2048+uOffset;

	
	DWORD i,j;

	D3DLOCKED_RECT lockedRect;			

	HRESULT hr = pTextureD3D->LockRect(0,&lockedRect,NULL,D3DLOCK_NOSYSLOCK);

	ASSERT(!FAILED(hr));



	const unsigned char* pBufferSrc = pBufferAux;
	for (j=0;j<pTexInfo->uHeight/2;j++)
	{			
		WORD* pBufferDest = (WORD*)((unsigned char*)lockedRect.pBits + j*2*lockedRect.Pitch);
		
		
		for (i=0;i<pTexInfo->uWidth/2;i++)
		{
			DWORD iDx = (DWORD) *pBufferSrc;
			iDx<<=2;

			pBufferDest[0] = ((WORD*)pBuffer)[iDx];
			pBufferDest[1] = ((WORD*)pBuffer)[iDx+2];

			pBufferDest[lockedRect.Pitch/2] = ((WORD*)pBuffer)[iDx+1];
			pBufferDest[lockedRect.Pitch/2+1] = ((WORD*)pBuffer)[iDx+3];

			pBufferDest+=2;
			pBufferSrc++;
		}	
	}

	pTextureD3D->UnlockRect(0);

	if (pTexInfo->uFlags & TTextureInfo::F_TWIDLE)
		DISPOSE_ARRAY(pBufferAux);
}

static DWORD GetOffsetMipMap(const CCacheTextures::TTextureInfo* pTexInfo)
{
  DWORD uBytes = 0;
  if (pTexInfo->uLevels)
  {
    DWORD uSize = 3;

    DWORD i;

    i = 1;
    while (i<pTexInfo->uWidth)
    {
      uSize+=i*i;
      i<<=1;
    }
    uBytes = uSize*2;
  }

  return uBytes;
}

void CCacheTextures::UpdateTexture_16(IDirect3DTexture9* pTextureD3D, const TTextureInfo* pTexInfo, const void* pBuffer)
{
	if (pTexInfo->uFlags & TTextureInfo::F_TWIDLE)	
	{		
		if (pTexInfo->uLevels)
		{
			DWORD uSize = 3;

			DWORD i;

			i = 1;
			while (i<pTexInfo->uWidth)
			{
				uSize+=i*i;
				i<<=1;
			}

			pBuffer = (void*)(((unsigned char*) pBuffer) + uSize*2);
		}

    int iMin = min(pTexInfo->uWidth, pTexInfo->uHeight);
    int iMask = iMin - 1;

    D3DLOCKED_RECT lockedRect;			

    pTextureD3D->LockRect(0,&lockedRect,NULL,D3DLOCK_NOSYSLOCK);


    for(DWORD y=0; y<pTexInfo->uHeight; y++) 
    {		
      WORD* pBufferDest = (WORD*)((unsigned char*)lockedRect.pBits + y*lockedRect.Pitch);
      for(DWORD x=0; x<pTexInfo->uWidth; x++) 
      {
        int iTwiddleOffset = (m_TwiddleTable[y&iMask]|(m_TwiddleTable[x&iMask]<<1))	+ (x/iMin + y/iMin)*(iMin*iMin);				
        unsigned short iColor = *(unsigned short*)((BYTE*)pBuffer + iTwiddleOffset*2);
        *pBufferDest++ = iColor;
        
      }
    }			


		pTextureD3D->UnlockRect(0);
	}
	else
	{		

		DWORD j;

		D3DLOCKED_RECT lockedRect;			


		DWORD uBytes = 0;
		if (pTexInfo->uLevels)
		{
			DWORD uSize = 3;

			DWORD i;

			i = 1;
			while (i<pTexInfo->uWidth)
			{
				uSize+=i*i;
				i<<=1;
			}
			uBytes = uSize*2;
		}

    RECT rect;

    DWORD uWidth = pTexInfo->uWidth;
    if (m_uStride && m_uStride<uWidth)
      uWidth = m_uStride;


    rect.bottom = pTexInfo->uHeight;
    rect.top = 0;
    rect.left = 0;
    rect.right = uWidth;
    pTextureD3D->LockRect(0,&lockedRect,&rect,D3DLOCK_NOSYSLOCK);
		

		const WORD* pBufferSrc = (const WORD*)pBuffer;
		for (j=0;j<pTexInfo->uHeight;j++)
		{			
			WORD* pBufferDest = (WORD*)((unsigned char*)lockedRect.pBits + j*lockedRect.Pitch);
			memcpy(pBufferDest,pBufferSrc+uBytes,uWidth*2);
			pBufferSrc+=uWidth;				
		}

		pTextureD3D->UnlockRect(0);
	}
}


void CCacheTextures::UpdatePalette(int _iMode, int _iNumEntries, const void* pBuffer)
{  

  
  switch(_iMode & 0x03)
  {
    //=============================================================================
    // ARGB1555
    //=============================================================================
  case 0:
    {
      const unsigned short* pPalTable = (const unsigned short*)pBuffer;
      BYTE* pPalette = (BYTE*)&m_Palette[0];

      for(int i=0; i<_iNumEntries; i++) 
      {
        unsigned short iColor = *pPalTable;
        pPalTable += 2;

        *pPalette++ = m_TableUnpack5to8[ iColor     & 0x1f];	   // Blue
        *pPalette++ = m_TableUnpack5to8[(iColor>>5) & 0x1f]; // Green
        *pPalette++ = m_TableUnpack5to8[(iColor>>10)& 0x1f];	   // Red
        *pPalette++ = (iColor & 0x8000) ? 255 : 0;
      }
    }
    break;

    //=============================================================================
    // RGB565
    //=============================================================================
  case 1:
    {
      const unsigned short* pPalTable = (const unsigned short*)pBuffer;
      BYTE* pPalette = (BYTE*)&m_Palette[0];

      for(int i=0; i<_iNumEntries; i++) 
      {
        unsigned short iColor = *pPalTable;
        pPalTable += 2;

        *pPalette++ = m_TableUnpack5to8[ iColor     & 0x1f];	   // Blue
        *pPalette++ = m_TableUnpack6to8[(iColor>>5) & 0x3f]; // Green
        *pPalette++ = m_TableUnpack5to8[(iColor>>11)& 0x1f];	   // Red
        *pPalette++ = 255;
      }
    }
    break;

    //=============================================================================
    // ARGB4444
    //=============================================================================
  case 2:
    {
      const unsigned short* pPalTable = (const unsigned short*)pBuffer;
      SHORT* pPalette = (SHORT*)&m_Palette[0];

      for(int i=0; i<_iNumEntries; i++) 
      {
        unsigned short iColor = *pPalTable;
        pPalTable += 2;

        *pPalette++ = m_TableUnpack2x4to2x8[iColor & 0xFF];//b,g
        *pPalette++ = m_TableUnpack2x4to2x8[(iColor>>8) & 0xFF];//b,g
      }
    }
    break;

    //=============================================================================
    // ARGB8888
    //=============================================================================
  case 3:
    {
      BYTE* pPalTable = (BYTE*)pBuffer;
      BYTE* pPalette = (BYTE*)&m_Palette[0];
      memcpy(pPalette,pPalTable,_iNumEntries*sizeof(DWORD));
    }
    break;  
  }
}


inline int bound(int low, int up, int val)
{
  if (val<low) 
    return low;
  else if (val>up)
    return up;
  else
    return val;
}
void yuv2rgb(int y,int u,int v,int &r, int &g, int &b)
{
  b = bound(0,255,(76283*(y - 16) + 132252*(u - 128))>>16);
  g = bound(0,255,(76283*(y - 16) - 53281 *(v - 128) - 25624*(u - 128))>>16); //last one u?
  r = bound(0,255,(76283*(y - 16) + 104595*(v - 128))>>16);
}


void CCacheTextures::UpdateTexture_YUV(IDirect3DTexture9* pTextureD3D, const TTextureInfo* pTexInfo, const void* _pBuffer)
{
  int iMin = min(pTexInfo->uWidth, pTexInfo->uHeight);
  int iMask = iMin - 1;


  const void* pBuffer = (BYTE*)_pBuffer + GetOffsetMipMap(pTexInfo);

  if (pTexInfo->uFlags & TTextureInfo::F_TWIDLE)	
  {		
    D3DLOCKED_RECT lockedRect;						


    pTextureD3D->LockRect(0,&lockedRect,NULL,D3DLOCK_NOSYSLOCK);

    for(DWORD y=0; y<pTexInfo->uHeight; y++) 
    {		
      BYTE* pDest = ((unsigned char*)lockedRect.pBits + y*lockedRect.Pitch);

      for(DWORD x=0; x<pTexInfo->uWidth; x+=2) 
      {
        int iTwiddleOffset = (m_TwiddleTable[y&iMask]|(m_TwiddleTable[x&iMask]<<1))	+ (x/iMin + y/iMin)*(iMin*iMin);				
        unsigned short iColor = *(unsigned short *)(((unsigned char*) pBuffer) + iTwiddleOffset*2);
        iTwiddleOffset = (m_TwiddleTable[y&iMask]|(m_TwiddleTable[(x+1)&iMask]<<1))	+ ((x+1)/iMin + y/iMin)*(iMin*iMin);
        unsigned short iColor2 = *(unsigned short *)(((unsigned char*) pBuffer) + iTwiddleOffset*2);
        int y0 = (iColor>>8) &0xff;
        int u = iColor&0xFF;
        int y1 = (iColor2>>8)&0xFF;
        int v = iColor2&0xFF;
        int r,g,b;
        yuv2rgb(y0,u,v,r,g,b);
        *pDest++ = b;//b
        *pDest++ = g;//g
        *pDest++ = r;//r
        *pDest++ = 255; //a
        yuv2rgb(y1,u,v,r,g,b);
        *pDest++ = b;//b
        *pDest++ = g;//g
        *pDest++ = r;//r
        *pDest++ = 255; //a
      }
    }	

    pTextureD3D->UnlockRect(0);
  }
  else
  {
    D3DLOCKED_RECT lockedRect;						


    RECT rect;

    DWORD uWidth = pTexInfo->uWidth;
    if (m_uStride && m_uStride<uWidth)
      uWidth = m_uStride;


    rect.bottom = pTexInfo->uHeight;
    rect.top = 0;

    rect.left = 0;
    rect.right = uWidth;
    pTextureD3D->LockRect(0,&lockedRect,&rect,D3DLOCK_NOSYSLOCK);

    for(DWORD y=0; y<pTexInfo->uHeight; y++) 
    {		
      BYTE* pDest = ((unsigned char*)lockedRect.pBits + y*lockedRect.Pitch);
      for(DWORD x=0; x<uWidth; x+=2) 
      {
        int iOffset = (y*uWidth + x) * 2;
        unsigned short iColor = *(unsigned short *)((BYTE*)pBuffer + iOffset);
        unsigned short iColor2 = *(unsigned short *)((BYTE*)pBuffer + iOffset+2);
        int y0 = (iColor>>8) &0xff;
        int u = iColor&0xFF;
        int y1 = (iColor2>>8)&0xFF;
        int v = iColor2&0xFF;
        int r,g,b;
        yuv2rgb(y0,u,v,r,g,b);
        *pDest++ = b;//b
        *pDest++ = g;//g
        *pDest++ = r;//r
        *pDest++ = 255; //a
        yuv2rgb(y1,u,v,r,g,b);
        *pDest++ = b;//b
        *pDest++ = g;//g
        *pDest++ = r;//r
        *pDest++ = 255; //a
      }
    }	

    pTextureD3D->UnlockRect(0);
  }
}


void CCacheTextures::SetStride(DWORD uStride)
{
  m_uStride = uStride;
}

#endif