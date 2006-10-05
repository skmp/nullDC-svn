/*
**	TA_Texture.cpp	- David Miller 2006 -
*/
#include "PowerVR2.h"




///// temp use table


///////////////////// thX to Optimiz3 for new routine
const static unsigned int lut[4] = { 0, 1, 4, 5 };

__inline static unsigned int
twiddle_optimiz3d(int value, int n)
{
	unsigned int y, x, count, ret = 0, sh = 0;

	y = x = value;
	y >>= n;			// t0 = y coordinate
	x &= ~(-1 << n);	// t1 = x coordinate

	// we process 2 y bits and 2 x bits every iteration (4 bits) it is possible to do more if we expand the lookup table by a power of 2
	for(count = (n + 1) >> 1; count; --count)
	{
		ret |= (lut[y & 3] | lut[x & 3] << 1) << sh;
		y >>= 2;
		x >>= 2;
		sh += 4;
	}

	return ret;
}

# define twop( val, n )	twiddle_optimiz3d( (val), (n) )
///////////////////////////////////////////////////////

/* Use Array for speed later ..
*/
const static u32 MipOffs[] = { 0x18, 0x58, 0x158, 0x558, 0x1558, 0x5558 }; // ...

//# define MIP_OFFSET(u,level)	MipOffs[u-level] // -1 ?

__inline static u32 MipPoint(u32 u)
{
	u32 ret = (8)+(16);

	for( u32 v=0; v<u; v++ )
		ret += ((8<<v)*(8<<v));

	return ret<<1;//2 bytes per pixel
}

__inline static u32 MipPointVQ(u32 u)
{
	u32 ret = (8)+(16);

	for( u32 v=0; v<u; v++ )
		ret += ((8<<v)*(8<<v));

	return ret>>2;//4 pixels per byte
}

////////////////// A FEW PIXEL MACROS .. THESE DIFFER FROM D3D ////////////////////////////
// MAKE SURE ALPHA ? : is in (PARENTHESIS) !!
#define ARGB1555( word )	\
	( ((word&0x8000)?(0xFF000000):(0)) | ((word&0x7C00)>>7) | ((word&0x3E0)<<6) | ((word&0x1F)<<19) )

#define ARGB565( word )		\
	( 0xFF000000 | ((word&0xF800)>>8) | ((word&0x7E0)<<5) | ((word&0x1F)<<19) )

#define ARGB4444( word )	\
	( ((word&0xF000)<<16) | ((word&0xF00)>>4) | ((word&0xF0)<<8) | ((word&0xF)<<20) )

#define ARGB8888( dword )	\
	( (dword&0xFF00FF00) | ((dword>>16)&0xFF) | ((dword&0xFF)<<16) )

/////////////////////////////////////////////////////////////////////////////

static u32 pTempTex[1024*1024];

 
void TexGen(PolyParam *pp, TexEntry *te)
{
	if(R_OPENGL==pvrOpts.GfxApi)
	{
		glGenTextures(1,&te->texID);
		glBindTexture(GL_TEXTURE_2D, te->texID);

		if( pp->param0.tsp.FilterMode > 0 ) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, te->Width, te->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pTempTex);

#ifdef DEBUG_LIB
		int tw=0,th=0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &tw);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);
		ASSERT_T((!glIsTexture(te->texID)),"OpenGL New Texture ID Invalid!");
		ASSERT_T((tw!=te->Width), "OpenGL TexWidth  Mismatch!");
		ASSERT_T((th!=te->Height),"OpenGL TexHeight Mismatch!");
#endif
	}
	else
	{
		LPDIRECT3DTEXTURE9 tex;
		
		if(g_pDev->CreateTexture(te->Width, te->Height, 0, 0/*D3DUSAGE_DYNAMIC*/, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex, NULL) != D3D_OK)
			printf("@@ ERROR: D3D Tex. Couldnt Create !\n" );

		D3DLOCKED_RECT lrect;
		if(tex->LockRect(0, &lrect, NULL, D3DLOCK_DISCARD) != D3D_OK)
			printf("@@ ERROR: D3D Tex. Couldnt lock rect !\n" );

		if(lrect.Pitch != (te->Width<<2))	// texW<<2 = nbytes per line
			printf("@@ ERROR: D3D Tex. Pitch Doesn't match width\n" );

		for(int t=0; t<(te->Width * te->Height); t++)
			((u32*)lrect.pBits)[t] = (pTempTex[t]&0xFF00FF00) | ((pTempTex[t]>>16)&0xFF) | ((pTempTex[t]&0xFF)<<16);

		tex->UnlockRect(0);
		te->texID = (u32)tex;
	}

	printf("TexParams=%X @ %08x texID: %X, %dx%d  Ctrl: %X\n",
		(*((u32*)&pp->param0.tcw) >> 25 & 0x7F),(pp->param0.tcw.TexAddr << 3 &0x7FFFFF),
		te->texID, te->Width, te->Height, (*((u32*)&pp->param0.tcw) >> 25 & 0x7F));
}








void TexDec1555(PolyParam *pp, TexEntry *te)
{
	for(u32 p=0; p<(te->Width * te->Height); p++)
	{
		u16 pval = *(u16*)(emuIf.vram + te->Start + (p<<1));
		pTempTex[p] = ARGB1555(pval) ;
	}

	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}

void TexDec565(PolyParam *pp, TexEntry *te)
{
	for(u32 p=0; p<(te->Width * te->Height); p++)
	{
		u16 pval = *(u16*)(emuIf.vram + te->Start + (p<<1));
		pTempTex[p] = ARGB565(pval) ;
	}

	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}

void TexDec4444(PolyParam *pp, TexEntry *te)
{
	for(u32 p=0; p<(te->Width * te->Height); p++)
	{
		u16 pval = *(u16*)(emuIf.vram + te->Start + (p<<1));
		pTempTex[p] = ARGB4444(pval) ;
	}

	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}





void TexDec1555_TW(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;
	for(u32 p=0; p<(te->Width * te->Height); p++)
	{
		u16 pval = *(u16*)(emuIf.vram + te->Start + (twop(p,texU)<<1));
		pTempTex[p] = ARGB1555(pval) ;
	}

	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}

void TexDec565_TW(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;
	for(u32 p=0; p<(te->Width * te->Height); p++)
	{
		u16 pval = *(u16*)(emuIf.vram + te->Start + (twop(p,texU)<<1));
		pTempTex[p] = ARGB565(pval) ;
	}

	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}

void TexDec4444_TW(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;
	for(u32 p=0; p<(te->Width * te->Height); p++)
	{
		u16 pval = *(u16*)(emuIf.vram + te->Start + (twop(p,texU)<<1));
		pTempTex[p] = ARGB4444(pval) ;
	}

	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}


void TexDec1555_SR(PolyParam *pp, TexEntry *te)
{
	u32 Stride = (32 * (*pTEXT_CONTROL &31));

	for(u32 y=0; y<te->Height; y++)
	{
		u32 x=0;
		for(x=0; x<Stride; x++)
		{
			u32 p = (y*Stride+x);
			u16 pval = *(u16*)(emuIf.vram + te->Start + (p<<1));
			pTempTex[(y*te->Width+x)] = ARGB1555(pval) ;
		}
#ifdef DEBUG_LIB
		for(; x<te->Width; x++) {
			u32 t = (y*te->Width+x);
			pTempTex[t] = 0xFF00FF00;
		}
#endif //DEBUG_LIB
	}
	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}

void TexDec565_SR(PolyParam *pp, TexEntry *te)
{
	u32 Stride = (32 * (*pTEXT_CONTROL &31));

	for(u32 y=0; y<te->Height; y++)
	{
		u32 x=0;
		for(x=0; x<Stride; x++)
		{
			u32 p = (y*Stride+x);
			u16 pval = *(u16*)(emuIf.vram + te->Start + (p<<1));
			pTempTex[(y*te->Width+x)] = ARGB565(pval) ;
		}
#ifdef DEBUG_LIB
		for(; x<te->Width; x++) {
			u32 t = (y*te->Width+x);
			pTempTex[t] = 0xFF00FF00;
		}
#endif //DEBUG_LIB
	}
	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}

void TexDec4444_SR(PolyParam *pp, TexEntry *te)
{
	u32 Stride = (32 * (*pTEXT_CONTROL &31));

	for(u32 y=0; y<te->Height; y++)
	{
		u32 x=0;
		for(x=0; x<Stride; x++)
		{
			u32 p = (y*Stride+x);
			u16 pval = *(u16*)(emuIf.vram + te->Start + (p<<1));
			pTempTex[(y*te->Width+x)] = ARGB4444(pval) ;
		}
#ifdef DEBUG_LIB
		for(; x<te->Width; x++) {
			u32 t = (y*te->Width+x);
			pTempTex[t] = 0xFF00FF00;
		}
#endif //DEBUG_LIB
	}
	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}






void TexDecYUV_TW(PolyParam *pp, TexEntry *te)	// *FIXME* uh detwiddle it?
{
	s32 R=0, G=0, B=0;
	s32 Y0, Yu, Y1, Yv;

	u32 texU = pp->param0.tsp.TexU+3;
	for(u32 p=0; p<(te->Width * te->Height); p+=2)
	{
		u16 YUV0 = *(u16 *)(emuIf.vram + te->Start + (twop(p, texU)<<1) );
		u16 YUV1 = *(u16 *)(emuIf.vram + te->Start + (twop(p+2, texU)<<1) );

		Y0 = YUV0>>8 &255;//(s32)*(u8*)(emuIf.vram + te->Start + (twop(p + 0, texU)<<1));
		Yu = YUV0>>0 &255;//(s32)*(u8*)(emuIf.vram + te->Start + (twop(p + 1, texU)<<1));
		Y1 = YUV1>>8 &255;//(s32)*(u8*)(emuIf.vram + te->Start + (twop(p + 2, texU)<<1));
		Yv = YUV1>>0 &255;//(s32)*(u8*)(emuIf.vram + te->Start + (twop(p + 3, texU)<<1));

		B = (76283*(Y0 - 16) + 132252*(Yu - 128))>>16;
		G = (76283*(Y0 - 16) - 53281 *(Yv - 128) - 25624*(Yu - 128))>>16;
		R = (76283*(Y0 - 16) + 104595*(Yv - 128))>>16;

		pTempTex[p] = 0xFF000000;
		pTempTex[p] |= ((R>0xFF)?0xFF:(R<0)?0:R);
		pTempTex[p] |= ((G>0xFF)?0xFF:(G<0)?0:G) << 8;
		pTempTex[p] |= ((B>0xFF)?0xFF:(B<0)?0:B) << 16;

		B = (76283*(Y0 - 16) + 132252*(Yu - 128))>>16;
		G = (76283*(Y0 - 16) - 53281 *(Yv - 128) - 25624*(Yu - 128))>>16;
		R = (76283*(Y0 - 16) + 104595*(Yv - 128))>>16;

		pTempTex[p+1] = 0xFF000000;
		pTempTex[p+1] |= ((R>0xFF)?0xFF:(R<0)?0:R);
		pTempTex[p+1] |= ((G>0xFF)?0xFF:(G<0)?0:G) << 8;
		pTempTex[p+1] |= ((B>0xFF)?0xFF:(B<0)?0:B) << 16;
	}

	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}

void TexDecYUV(PolyParam *pp, TexEntry *te)
{
	s32 R=0, G=0, B=0;
	s32 Y0, Yu, Y1, Yv;

	for(u32 p=0; p<(te->Width * te->Height); p+=2)
	{
		Yu = (s32)*(u8*)(emuIf.vram + te->Start + (p<<1) + 0);
		Y0 = (s32)*(u8*)(emuIf.vram + te->Start + (p<<1) + 1);
		Yv = (s32)*(u8*)(emuIf.vram + te->Start + (p<<1) + 2);
		Y1 = (s32)*(u8*)(emuIf.vram + te->Start + (p<<1) + 3);

		B = (76283*(Y0 - 16) + 132252*(Yu - 128))>>16;
		G = (76283*(Y0 - 16) - 53281 *(Yv - 128) - 25624*(Yu - 128))>>16;
		R = (76283*(Y0 - 16) + 104595*(Yv - 128))>>16;

		pTempTex[p] = 0xFF000000;
		pTempTex[p] |= ((R>0xFF)?0xFF:(R<0)?0:R);
		pTempTex[p] |= ((G>0xFF)?0xFF:(G<0)?0:G) << 8;
		pTempTex[p] |= ((B>0xFF)?0xFF:(B<0)?0:B) << 16;

		B = (76283*(Y0 - 16) + 132252*(Yu - 128))>>16;
		G = (76283*(Y0 - 16) - 53281 *(Yv - 128) - 25624*(Yu - 128))>>16;
		R = (76283*(Y0 - 16) + 104595*(Yv - 128))>>16;

		pTempTex[p+1] = 0xFF000000;
		pTempTex[p+1] |= ((R>0xFF)?0xFF:(R<0)?0:R);
		pTempTex[p+1] |= ((G>0xFF)?0xFF:(G<0)?0:G) << 8;
		pTempTex[p+1] |= ((B>0xFF)?0xFF:(B<0)?0:B) << 16;
	}

	te->End = te->Start + te->Width * te->Height;
	TexGen(pp,te);
}

void TexDecYUV_SR(PolyParam *pp, TexEntry *te)
{
	s32 R=0, G=0, B=0;
	s32 Y0, Yu, Y1, Yv;
	u32 Stride = (32 * (*pTEXT_CONTROL &31));

	for(u32 y=0; y<te->Height; y++)
	{
		u32 x=0;
		for(x=0; x<Stride; x+=2)
		{
			u32 p = (y*Stride+x);
			u32 t = (y*te->Width+x);

			Yu = (s32)*(u8*)(emuIf.vram + te->Start + (p<<1) + 0);
			Y0 = (s32)*(u8*)(emuIf.vram + te->Start + (p<<1) + 1);
			Yv = (s32)*(u8*)(emuIf.vram + te->Start + (p<<1) + 2);
			Y1 = (s32)*(u8*)(emuIf.vram + te->Start + (p<<1) + 3);

			B = (76283*(Y0 - 16) + 132252*(Yu - 128))>>16;
			G = (76283*(Y0 - 16) - 53281 *(Yv - 128) - 25624*(Yu - 128))>>16;
			R = (76283*(Y0 - 16) + 104595*(Yv - 128))>>16;

			pTempTex[t] = 0xFF000000;
			pTempTex[t] |= ((R>0xFF)?0xFF:(R<0)?0:R);
			pTempTex[t] |= ((G>0xFF)?0xFF:(G<0)?0:G) << 8;
			pTempTex[t] |= ((B>0xFF)?0xFF:(B<0)?0:B) << 16;

			B = (76283*(Y0 - 16) + 132252*(Yu - 128))>>16;
			G = (76283*(Y0 - 16) - 53281 *(Yv - 128) - 25624*(Yu - 128))>>16;
			R = (76283*(Y0 - 16) + 104595*(Yv - 128))>>16;

			pTempTex[t+1] = 0xFF000000;
			pTempTex[t+1] |= ((R>0xFF)?0xFF:(R<0)?0:R);
			pTempTex[t+1] |= ((G>0xFF)?0xFF:(G<0)?0:G) << 8;
			pTempTex[t+1] |= ((B>0xFF)?0xFF:(B<0)?0:B) << 16;
		}
#ifdef DEBUG_LIB
		for(; x<te->Width; x++) {
			u32 t = (y*te->Width+x);
			pTempTex[t] = 0xFF00FF00;
		}
#endif //DEBUG_LIB
	}

	te->End = te->Start + te->Width * te->Height;
	TexGen(pp,te);
}

/*	The color format for a palette format texture is specified by the PAL_RAM_CTRL register,
*	and can be selected from among four types: 1555, 565, 4444, and 8888. 
*	When the color format is 8888 mode, texture filtering performance is reduced by half.
*/

void TexDecPAL4(PolyParam *pp, TexEntry *te)
{
	u32 palselect = *((u32*)&pp->param0.tcw) >> 21 & 0x3F;

	u16 * pPAL = &((u16*)TA_PalT)[palselect<<5];

	// decode this bullshit first
	u32 texU = pp->param0.tsp.TexU+3;

	switch(*pPAL_RAM_CTRL &3)
	{
	case 0:	// argb155
		for(u32 y=0; y<te->Height; y+=2)
		{
			for(u32 x=0; x<te->Width; x+=2) 
			{
				u16 iColor = *(u16*)(emuIf.vram + te->Start + (twop(((y>>1)*te->Width+(x>>1)),texU)<<1));

				pTempTex[y*te->Width+x]			= ARGB1555(pPAL[(iColor        & 0x0f)*2]);
				pTempTex[y*te->Width+x+1]		= ARGB1555(pPAL[((iColor >>  8) & 0x0f)*2]);
				pTempTex[(y+1)*te->Width+x]		= ARGB1555(pPAL[((iColor >>  4) & 0x0f)*2]);
				pTempTex[(y+1)*te->Width+x+1]	= ARGB1555(pPAL[((iColor >> 12) & 0x0f)*2]);
			}
		}
		break;

	case 1:	// rgb565
		for(u32 y=0; y<te->Height; y+=2)
		{
			for(u32 x=0; x<te->Width; x+=2) 
			{
				u16 iColor = *(u16*)(emuIf.vram + te->Start + (twop(((y>>1)*te->Width+(x>>1)),texU)<<1));

				pTempTex[y*te->Width+x]			= ARGB565(pPAL[( iColor        & 0x0f)*2]);
				pTempTex[y*te->Width+x+1]		= ARGB565(pPAL[((iColor >>  8) & 0x0f)*2]);
				pTempTex[(y+1)*te->Width+x]		= ARGB565(pPAL[((iColor >>  4) & 0x0f)*2]);
				pTempTex[(y+1)*te->Width+x+1]	= ARGB565(pPAL[((iColor >> 12) & 0x0f)*2]);
			}
		}
		break;

	case 2:	// argb4444
		{
			for(u32 y=0; y<te->Height; y+=2)
			{
				for(u32 x=0; x<te->Width; x+=2) 
				{
					u16 iColor = *(u16*)(emuIf.vram + te->Start + (twop(((y>>1)*te->Width+(x>>1)),texU)<<1));

					pTempTex[y*te->Width+x]			= ARGB4444(pPAL[( iColor        & 0x0f)*2]);
					pTempTex[y*te->Width+x+1]		= ARGB4444(pPAL[((iColor >>  8) & 0x0f)*2]);
					pTempTex[(y+1)*te->Width+x]		= ARGB4444(pPAL[((iColor >>  4) & 0x0f)*2]);
					pTempTex[(y+1)*te->Width+x+1]	= ARGB4444(pPAL[((iColor >> 12) & 0x0f)*2]);
				}
			}
		}

		break;

	case 3:	// argb8888
		for(u32 y=0; y<te->Height; y+=2)
		{
			for(u32 x=0; x<te->Width; x+=2) 
			{
				u16 iColor = *(u16*)(emuIf.vram + te->Start + (twop(((y>>1)*te->Width+(x>>1)),texU)<<1));

				pTempTex[y*te->Width+x]			= ARGB8888(*(u32*)&pPAL[( iColor        & 0x0f)*2]);
				pTempTex[y*te->Width+x+1]		= ARGB8888(*(u32*)&pPAL[((iColor >>  8) & 0x0f)*2]);
				pTempTex[(y+1)*te->Width+x]		= ARGB8888(*(u32*)&pPAL[((iColor >>  4) & 0x0f)*2]);
				pTempTex[(y+1)*te->Width+x+1]	= ARGB8888(*(u32*)&pPAL[((iColor >> 12) & 0x0f)*2]);
			}
		}
		break;
	}


	te->End = te->Start + te->Width * te->Height /2;//4bits per pixel
	TexGen(pp,te);
}

void TexDecPAL8(PolyParam *pp, TexEntry *te)
{
	u32 palselect = *((u32*)&pp->param0.tcw) >> 21 & 0x3F;

	u16 * pPAL = &((u16*)TA_PalT)[palselect<<5];

	u32 texU = pp->param0.tsp.TexU+3;

	switch(*pPAL_RAM_CTRL &3)
	{
	case 0:	// argb1555
		for(u32 p=0; p<(te->Width * te->Height); p++)
		{
			u8 pval = *(u8*)(emuIf.vram + te->Start + (twop(p,texU)));
			pTempTex[p] = ARGB1555(pPAL[pval*2]) ;
		}
		break;

	case 1:	// rgb565
		for(u32 p=0; p<(te->Width * te->Height); p++)
		{
			u8 pval = *(u8*)(emuIf.vram + te->Start + (twop(p,texU)));
			pTempTex[p] = ARGB565(pPAL[pval*2]) ;
		}
		break;

	case 2:	// argb4444
		for(u32 p=0; p<(te->Width * te->Height); p++)
		{
			u8 pval = *(u8*)(emuIf.vram + te->Start + (twop(p,texU)));
			pTempTex[p] = ARGB4444(pPAL[pval*2]) ;
		}

		break;

	case 3:	// argb8888
		for(u32 p=0; p<(te->Width * te->Height); p++)
		{
			u8 pval = *(u8*)(emuIf.vram + te->Start + (twop(p,texU)));
			pTempTex[p] = ARGB8888(*(u32*)&pPAL[pval*2]) ;
		}
		break;
	}

	te->End = te->Start + te->Width * te->Height *1;//8bits per pixel
	TexGen(pp,te);
	memset(pTempTex, 0xFF, 1024*1024*4);
}



void TexDecYUV_TW_MM(PolyParam *pp, TexEntry *te)
{
	s32 R=0, G=0, B=0;
	s32 Y0, Yu, Y1, Yv;

	u32 texU = pp->param0.tsp.TexU+3;
	u8* texAddr = (emuIf.vram + te->Start) + MipPoint(texU-3);

	for(u32 p=0; p<(te->Width * te->Height); p+=2)
	{
		u16 YUV0 = *(u16 *)(texAddr + (twop(p, texU)<<1) );
		u16 YUV1 = *(u16 *)(texAddr + (twop(p+2, texU)<<1) );

		Y0 = YUV0>>8 &255;//(s32)*(u8*)(emuIf.vram + te->Start + (twop(p + 0, texU)<<1));
		Yu = YUV0>>0 &255;//(s32)*(u8*)(emuIf.vram + te->Start + (twop(p + 1, texU)<<1));
		Y1 = YUV1>>8 &255;//(s32)*(u8*)(emuIf.vram + te->Start + (twop(p + 2, texU)<<1));
		Yv = YUV1>>0 &255;//(s32)*(u8*)(emuIf.vram + te->Start + (twop(p + 3, texU)<<1));

		B = (76283*(Y0 - 16) + 132252*(Yu - 128))>>16;
		G = (76283*(Y0 - 16) - 53281 *(Yv - 128) - 25624*(Yu - 128))>>16;
		R = (76283*(Y0 - 16) + 104595*(Yv - 128))>>16;

		pTempTex[p] = 0xFF000000;
		pTempTex[p] |= ((R>0xFF)?0xFF:(R<0)?0:R);
		pTempTex[p] |= ((G>0xFF)?0xFF:(G<0)?0:G) << 8;
		pTempTex[p] |= ((B>0xFF)?0xFF:(B<0)?0:B) << 16;

		B = (76283*(Y0 - 16) + 132252*(Yu - 128))>>16;
		G = (76283*(Y0 - 16) - 53281 *(Yv - 128) - 25624*(Yu - 128))>>16;
		R = (76283*(Y0 - 16) + 104595*(Yv - 128))>>16;

		pTempTex[p+1] = 0xFF000000;
		pTempTex[p+1] |= ((R>0xFF)?0xFF:(R<0)?0:R);
		pTempTex[p+1] |= ((G>0xFF)?0xFF:(G<0)?0:G) << 8;
		pTempTex[p+1] |= ((B>0xFF)?0xFF:(B<0)?0:B) << 16;
	}

	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}

// VQ


static u32 lcodebook[256][4];


void TexDec1555_TW_VQ(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;
	u16 tcol;
	u32 texAddr = te->Start;

	for( u32 j=0; j<256;j++) {
		for( u32 h=0; h<4; h++ )	// 2byte texture indices
		{
			*(u32*)&lcodebook[j][3-h] = ARGB1555(*(u16*)(emuIf.vram + texAddr));
			texAddr+=2;
		}
	}

	u32 texoffset=0;

	for(u32 i=0; i<te->Height; i+=2) {	
		for(u32 j=0; j<te->Width; j+=2)
		{
			int texoffset =  (twop(((i>>1)*te->Width+(j>>1)),texU));
			tcol = *(u8*)(emuIf.vram + (texAddr+texoffset));

			pTempTex[(i+1)*te->Width+(j+1)]	= lcodebook[tcol][0];
			pTempTex[i*te->Width+(j+1)]		= lcodebook[tcol][1];
			pTempTex[(i+1)*te->Width+j]		= lcodebook[tcol][2];
			pTempTex[i*te->Width+j]			= lcodebook[tcol][3];
		}			
	}

	te->End = te->Start + te->Width * te->Height +0x800;	// Not Right?
	TexGen(pp,te);
}

void TexDec565_TW_VQ(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;

	u16 tcol;
	u32 texAddr = te->Start;

	for( u32 j=0; j<256;j++) {
		for( u32 h=0; h<4; h++ )	// 2byte texture indices
		{
			*(u32*)&lcodebook[j][3-h] = ARGB565(*(u16*)(emuIf.vram + texAddr));
			texAddr+=2;
		}
	}

	u32 texoffset=0;

	for(u32 i=0; i<te->Height; i+=2) {	
		for(u32 j=0; j<te->Width; j+=2)
		{
			int texoffset =  (twop(((i>>1)*te->Width+(j>>1)),texU));
			tcol = *(u8*)(emuIf.vram + (texAddr+texoffset));

			pTempTex[(i+1)*te->Width+(j+1)]	= lcodebook[tcol][0];
			pTempTex[i*te->Width+(j+1)]		= lcodebook[tcol][1];
			pTempTex[(i+1)*te->Width+j]		= lcodebook[tcol][2];
			pTempTex[i*te->Width+j]			= lcodebook[tcol][3];
		}			
	}

	te->End = te->Start + te->Width * te->Height +0x800;	// Not Right?
	TexGen(pp,te);
}

void TexDec4444_TW_VQ(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;

	u16 tcol;
	u32 texAddr = te->Start;

	for( u32 j=0; j<256;j++) {
		for( u32 h=0; h<4; h++ )	// 2byte texture indices
		{
			*(u32*)&lcodebook[j][3-h] = ARGB4444(*(u16*)(emuIf.vram + texAddr));
			texAddr+=2;
		}
	}

	u32 texoffset=0;

	for(u32 i=0; i<te->Height; i+=2) {	
		for(u32 j=0; j<te->Width; j+=2)
		{
			int texoffset =  (twop(((i>>1)*te->Width+(j>>1)),texU));
			tcol = *(u8*)(emuIf.vram + (texAddr+texoffset));

			pTempTex[(i+1)*te->Width+(j+1)]	= lcodebook[tcol][0];
			pTempTex[i*te->Width+(j+1)]		= lcodebook[tcol][1];
			pTempTex[(i+1)*te->Width+j]		= lcodebook[tcol][2];
			pTempTex[i*te->Width+j]			= lcodebook[tcol][3];
		}			
	}

	te->End = te->Start + te->Width * te->Height +0x800;	// Not Right?
	TexGen(pp,te);
}



	// MipMap Decoding

void TexDec1555_TW_MM(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;
	u32 MipOffs = MipPoint(texU-3);

	for(u32 p=0; p<(te->Width * te->Height); p++)
	{
		u16 pval = *(u16*)(emuIf.vram + te->Start + MipOffs + (twop(p,texU)<<1));
		pTempTex[p] = ARGB1555(pval) ;
	}

	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}

void TexDec565_TW_MM(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;
	u32 MipOffs = MipPoint(texU-3);

	for(u32 p=0; p<(te->Width * te->Height); p++)
	{
		u16 pval = *(u16*)(emuIf.vram + te->Start + MipOffs + (twop(p,texU)<<1));
		pTempTex[p] = ARGB565(pval) ;
	}

	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}

void TexDec4444_TW_MM(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;
	u32 MipOffs = MipPoint(texU-3);

	for(u32 p=0; p<(te->Width * te->Height); p++)
	{
		u16 pval = *(u16*)(emuIf.vram + te->Start + MipOffs + (twop(p,texU)<<1));
		pTempTex[p] = ARGB4444(pval) ;
	}

	te->End = te->Start + te->Width * te->Height * 2;
	TexGen(pp,te);
}




	// MM + VQ

void TexDec1555_TW_MM_VQ(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;
	u16 tcol;
	u32 texAddr = te->Start;

	for( u32 j=0; j<256;j++) {
		for( u32 h=0; h<4; h++ )	// 2byte texture indices
		{
			*(u32*)&lcodebook[j][3-h] = ARGB1555(*(u16*)(emuIf.vram + texAddr));
			texAddr+=2;
		}
	}

	texAddr += MipPointVQ(texU-3);

	u32 texoffset=0;

	for(u32 i=0; i<te->Height; i+=2) {	
		for(u32 j=0; j<te->Width; j+=2)
		{
			int texoffset =  (twop(((i>>1)*te->Width+(j>>1)),texU));
			tcol = *(u8*)(emuIf.vram + (texAddr+texoffset));

			pTempTex[(i+1)*te->Width+(j+1)]	= lcodebook[tcol][0];
			pTempTex[i*te->Width+(j+1)]		= lcodebook[tcol][1];
			pTempTex[(i+1)*te->Width+j]		= lcodebook[tcol][2];
			pTempTex[i*te->Width+j]			= lcodebook[tcol][3];
		}			
	}

	te->End = te->Start + te->Width * te->Height +0x800;	// Not Right?
	TexGen(pp,te);
}

void TexDec565_TW_MM_VQ(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;

	u16 tcol;
	u32 texAddr = te->Start;

	for( u32 j=0; j<256;j++) {
		for( u32 h=0; h<4; h++ )	// 2byte texture indices
		{
			*(u32*)&lcodebook[j][3-h] = ARGB565(*(u16*)(emuIf.vram + texAddr));
			texAddr+=2;
		}
	}

	texAddr += MipPointVQ(texU-3);

	u32 texoffset=0;

	for(u32 i=0; i<te->Height; i+=2) {	
		for(u32 j=0; j<te->Width; j+=2)
		{
			int texoffset = (twop(((i>>1)*te->Width+(j>>1)),texU));
			tcol = *(u8*)(emuIf.vram + (texAddr+texoffset));

			pTempTex[(i+1)*te->Width+(j+1)]	= lcodebook[tcol][0];
			pTempTex[i*te->Width+(j+1)]		= lcodebook[tcol][1];
			pTempTex[(i+1)*te->Width+j]		= lcodebook[tcol][2];
			pTempTex[i*te->Width+j]			= lcodebook[tcol][3];
		}			
	}

	te->End = te->Start + te->Width * te->Height +0x800;	// Not Right?
	TexGen(pp,te);
}

void TexDec4444_TW_MM_VQ(PolyParam *pp, TexEntry *te)
{
	u32 texU = pp->param0.tsp.TexU+3;

	u16 tcol;
	u32 texAddr = te->Start;

	for( u32 j=0; j<256;j++) {
		for( u32 h=0; h<4; h++ )	// 2byte texture indices
		{
			*(u32*)&lcodebook[j][3-h] = ARGB4444(*(u16*)(emuIf.vram + texAddr));
			texAddr+=2;
		}
	}

	texAddr += MipPointVQ(texU-3);

	u32 texoffset=0;

	for(u32 i=0; i<te->Height; i+=2) {	
		for(u32 j=0; j<te->Width; j+=2)
		{
			int texoffset =  (twop(((i>>1)*te->Width+(j>>1)),texU));
			tcol = *(u8*)(emuIf.vram + (texAddr+texoffset));

			pTempTex[(i+1)*te->Width+(j+1)]	= lcodebook[tcol][0];
			pTempTex[i*te->Width+(j+1)]		= lcodebook[tcol][1];
			pTempTex[(i+1)*te->Width+j]		= lcodebook[tcol][2];
			pTempTex[i*te->Width+j]			= lcodebook[tcol][3];
		}			
	}

	te->End = te->Start + te->Width * te->Height +0x800;	// Not Right?
	TexGen(pp,te);
}



/// [K1,K2,K3,Q] are Poly Offs Color Bytes: [3,2,1,0] - [S,R] are Texel Bytes: [1,0]
/*
*	
*
*
*/

inline static float someshit(u8 S, u8 R, u8 T, u8 Q)
{
	float s = 3.14/2.f * (float)S/256.f;
	float r = 3.14*2.f * (float)R/256.f;
/*
	float Xs = cos(s)*cos(r);
	float Ys = sin(s);
	float Zs = cos(s)*sin(r);
*/
	float t = 3.14/2.f * (float)T/256.f;	// Where the fuck does T come from ?
	float q = 3.14*2.f * (float)Q/256.f;
/*
	float Xl = cos(t)*cos(q);
	float Yl = sin(t);
	float Zl = cos(t)*sin(q);

	float I = Xs*Xl + Ys*Yl + Zs*Zl;
*/

	// Ok, two simplifications :
	//float I = cos(s)*cos(r)*cos(t)*cos(q) + sin(s)*sin(t) + cos(s)*sin(r)*cos(t)*sin(q) ;

	// Even Better

	float I = sin(s)*sin(t) + cos(s)*cos(t)*cos(r-q) ;
}

void TexDecBump(PolyParam *pp, TexEntry *te)
{
	printf(" -------- BUMP MAP -------- ");
}

void TexDecBump_TW(PolyParam *pp, TexEntry *te)
{
	printf(" -------- BUMP MAP TW -------- ");
}





/*	When ScanOrder is 1 MipMap is ignored !
*
*/


TexID TextureCache::GetTexture(PolyParam *pp)
{
	u32 TexAddr = ((pp->param0.tcw.TexAddr << 3) &0x7FFFFF);
	if(0==TexAddr)
		return 0;

	//for(size_t t=0; t<TexList.size(); t++)
	//{
	//	if(TexAddr == TexList[t].Start)
	//		return TexList[t].texID;
	//}

	TexEntry* tad=TexList.Find(TexAddr);
	if (tad)
		return tad->texID;

	TexEntry tex;
	tex.texID	= 0;
	tex.End		= TexAddr;
	tex.Start	= TexAddr;
	tex.Width	= (8 << pp->param0.tsp.TexU);
	tex.Height	= (8 << pp->param0.tsp.TexV);

	u32 tctrl	= *((u32*)&pp->param0.tcw) >> 25 & 0x7F;

	switch(tctrl)
	{
	case 0x00:	TexDec1555_TW(pp,&tex);		break;	// 1555 + Twiddled
	case 0x02:	TexDec1555(pp,&tex);		break;	// 1555
	case 0x03:	TexDec1555_SR(pp,&tex);		break;	// 1555 + StrideRect

	case 0x04:	TexDec565_TW(pp,&tex);		break;	// 565 + Twiddled
	case 0x06:	TexDec565(pp,&tex);			break;	// 565
	case 0x07:	TexDec565_SR(pp,&tex);		break;	// 565 + StrideRect

	case 0x08:	TexDec4444_TW(pp,&tex);		break;	// 4444 + Twiddled
	case 0x0A:	TexDec4444(pp,&tex);		break;	// 4444
	case 0x0B:	TexDec4444_SR(pp,&tex);		break;	// 4444 + StrideRect

	case 0x0C:	TexDecYUV_TW(pp,&tex);		break;	// YUV422 + Twiddled
	case 0x0E:	TexDecYUV(pp,&tex);			break;	// YUV422
	case 0x0F:	TexDecYUV_SR(pp,&tex);		break;	// YUV422 + StrideRect



	case 0x12:	TexDecBump(pp,&tex);		break;	// Bump
	case 0x10:	TexDecBump_TW(pp,&tex);		break;	// Bump + Twiddled
	case 0x13:		// Bump + StrideRect
		goto unhandled_fmt;




	case 0x14:	case 0x15:		// PAL4
	case 0x16:	case 0x17:		// PAL4
		TexDecPAL4(pp,&tex);	break;

	case 0x18:	case 0x19:		// PAL8
	case 0x1A:	case 0x1B:		// PAL8
		TexDecPAL8(pp,&tex);	break;




		// Start VQ //

	case 0x20:	TexDec1555_TW_VQ(pp,&tex);		break;	// 1555 + VQ + Twiddled
	case 0x22:		// 1555 + VQ
	case 0x23:		// 1555 + VQ + StrideRect
		goto unhandled_fmt;

	case 0x24:	TexDec565_TW_VQ(pp,&tex);		break;	// 565 + VQ + Twiddled
	case 0x26:		// 565 + VQ
	case 0x27:		// 565 + VQ + StrideRect
		goto unhandled_fmt;

	case 0x28:	TexDec4444_TW_VQ(pp,&tex);		break;	// 4444 + VQ + Twiddled
	case 0x2A:		// 4444 + VQ
	case 0x2B:		// 4444 + VQ + StrideRect
		goto unhandled_fmt;





		// Start MipMaps //

	case 0x40:	TexDec1555_TW_MM(pp,&tex);		break;	// 1555 + MipMap + Twiddled
	case 0x42:		// 1555 + MipMap
	case 0x43:		// 1555 + MipMap + StrideRect
		goto unhandled_fmt;

	case 0x44:	TexDec565_TW_MM(pp,&tex);		break;	// 565 + MipMap + Twiddled
	case 0x46:		// 565 + MipMap
	case 0x47:		// 565 + MipMap + StrideRect
		goto unhandled_fmt;

	case 0x48:	TexDec4444_TW_MM(pp,&tex);		break;	// 4444 + MipMap + Twiddled
	case 0x4A:		// 4444 + MipMap
	case 0x4B:		// 4444 + MipMap + StrideRect
		goto unhandled_fmt;


	case 0x4C:	TexDecYUV_TW_MM(pp,&tex);		break;	// YUV422 + MipMap + Twiddled




		// Start VQ+MipMaps //

	case 0x60:	TexDec1555_TW_MM_VQ(pp,&tex);		break;	// 1555 + Twiddled
	case 0x64:	TexDec565_TW_MM_VQ(pp,&tex);		break;	// 565 + Twiddled
	case 0x68:	TexDec4444_TW_MM_VQ(pp,&tex);		break;	// 4444 + Twiddled





	case 0x01:		// 1555				(Illegal Twiddle/Stride)
	case 0x05:		// 565				(Illegal Twiddle/Stride)
	case 0x09:		// 4444				(Illegal Twiddle/Stride)
	case 0x0D:		// YUV422			(Illegal Twiddle/Stride)
	case 0x11:		// Bump				(Illegal Twiddle/Stride)
	case 0x1C:		// Reserved
	case 0x1D:		// Reserved
	case 0x1E:		// Reserved
	case 0x1F:		// Reserved
	case 0x21:		// 1555 + VQ		(Illegal Twiddle/Stride)
	case 0x25:		// 565 + VQ			(Illegal Twiddle/Stride)
	case 0x29:		// 4444 + VQ		(Illegal Twiddle/Stride)
	case 0x41:		// 1555 + MipMap	(Illegal Twiddle/Stride)
	case 0x45:		// 565 + MipMap		(Illegal Twiddle/Stride)
	case 0x49:		// 4444 + MipMap	(Illegal Twiddle/Stride)
	default:
		goto unhandled_fmt;
	}

	TexCacheList::TexCacheEntry* tce= TexList.Add(&tex);
	
	tce->text.lock_block=emuIf.vramLock64(tce->text.Start, tce->text.End, tce);
	
	return tex.texID;

unhandled_fmt:
	printf("GetTexture, Addr: %08X, Ctrl: %X  Unhandled!\n", TexAddr, tctrl);

	for(u32 p=0; p<(tex.Width * tex.Height); p++)
		pTempTex[p] = 0xFF00FF00;

	tex.End = tex.Start + tex.Width * tex.Height;	// This might cause trouble but its an error anyways !
	TexGen(pp,&tex);

	tce= TexList.Add(&tex);

	tce->text.lock_block=emuIf.vramLock64(tce->text.Start, tce->text.End, tce);
	return tex.texID;
}


void DeleteTexture(u32 * Texture)
{
	if(R_OPENGL==pvrOpts.GfxApi)
		glDeleteTextures(1,Texture);
	else
		((IDirect3DTexture9*)*(u32*)Texture)->Release();
	*(u32*)Texture = 0;
}

void vramLockCB(vram_block *bl, u32 addr)
{
	//WARN : removing list items while enumerating could be fatal .. seems to work , needs check
	/*for(size_t t=0; t<TexList.size(); t++) 
	{
		if(addr >= TexList[t].Start && addr < TexList[t].End)	
		{
			PvrIf->InvList.push_back(TexList[t].texID);
			PvrIf->TexList.erase(TexList.begin()+t);
			--t;//removed one
		}
	}*/
	TexCacheList::TexCacheEntry* tce=(TexCacheList::TexCacheEntry*)bl->userdata;

			///// **FIXME** //////
/*	PvrIf->InvList.push_back(tce->text.texID);
	PvrIf->TexList.Remove(tce);*/
	delete tce;
	emuIf.vramUnlock(bl);
}


void TextureCache::ClearTCache()
{
	//WARN : removing list items while enumerating could be fatal .. seems to work , needs check
	/*for(size_t t=0; t<TexList.size(); t++)
	{
		TexEntry temp=TexList[t];
		emuIf.vramUnlock(temp.lock_block);
		DeleteTexture(&temp.texID);
	}
	TexList.clear();*/
	while(TexList.pfirst)
	{
		TexCacheList::TexCacheEntry* tce=TexList.pfirst;
		TexList.Remove(TexList.pfirst);
		DeleteTexture(&tce->text.texID);
		emuIf.vramUnlock(tce->text.lock_block);
		delete tce;
	}
}

void TextureCache::ClearTInvalids()
{
	for(size_t tidx=0; tidx<InvList.size(); tidx++)
		DeleteTexture(&InvList[tidx]);
	InvList.clear();
	//check for texture age too , we want olny old textures wiped out ;) (30 frames +)
	/*
	if (TexList.textures>400)
	{
		int trm=TexList.textures-300;
		for (int i=0;i<trm;i++)
		{
			TexCacheList::TexCacheEntry* tce=TexList.plast;
			TexList.Remove(tce);
			DeleteTexture(&tce->text.texID);
			emuIf.vramUnlock(tce->text.lock_block);
			delete tce;
		}
	}*/
}



