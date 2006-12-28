#include "TexCache.h"

//Generic texture cache / texture format conevrtion code
u32 unpack_5_to_8[32] = 
{
	//low 16
	0<<3,1<<3,2<<3,3<<3,
	4<<3,5<<3,6<<3,7<<3,
	8<<3,9<<3,10<<3,11<<3,
	12<<3,13<<3,14<<3,15<<3,

	//hi 16
	16<<3,17<<3,18<<3,19<<3,
	20<<3,21<<3,22<<3,23<<3,
	24<<3,25<<3,26<<3,27<<3,
	28<<3,29<<3,30<<3,31<<3,
};

u32 unpack_6_to_8[64] = 
{
	//low 16
	0<<2,1<<2,2<<2,3<<2,
	4<<2,5<<2,6<<2,7<<2,
	8<<2,9<<2,10<<2,11<<2,
	12<<2,13<<2,14<<2,15<<2,

	//hi 16
	16<<2,17<<2,18<<2,19<<2,
	20<<2,21<<2,22<<2,23<<2,
	24<<2,25<<2,26<<2,27<<2,
	28<<2,29<<2,30<<2,31<<2,

	//low 16
	32<<2,33<<2,34<<2,35<<2,
	36<<2,37<<2,38<<2,39<<2,
	40<<2,41<<2,42<<2,43<<2,
	44<<2,45<<2,46<<2,47<<2,

	//hi 16
	48<<2,49<<2,50<<2,51<<2,
	52<<2,53<<2,54<<2,55<<2,
	56<<2,57<<2,58<<2,59<<2,
	60<<2,61<<2,62<<2,63<<2,
};

u32 unpack_1_to_8[2]={0,0xFF};

#define ARGB8888(A,R,G,B) \
	( ((A)<<24) | ((R)<<16) | ((G)<<8) | ((B)<<0))

#define ARGB1555( word )	\
	ARGB8888(unpack_1_to_8[word>>15],unpack_5_to_8[(word>>10) & 0x1F],	\
	unpack_5_to_8[(word>>5) & 0x1F],unpack_5_to_8[word&0x1F])

#define ARGB565( word )		\
	ARGB8888(0xFF,unpack_5_to_8[(word>>11) & 0x1F],	\
	unpack_6_to_8[(word>>5) & 0x3F],unpack_5_to_8[word&0x1F])
	//( 0xFF000000 | unpack_5_to_8[(word>>11) & 0x1F] | unpack_5_to_8[(word>>5) & 0x3F]<<8 | unpack_5_to_8[word&0x1F]<<16 )

#define ARGB4444( word )	\
	( ((word&0xF000)<<16) | ((word&0xF00)>>4) | ((word&0xF0)<<8) | ((word&0xF)<<20) )

///////////////////// thX to Optimiz3 for new routine
const static unsigned int lut[4] = { 0, 1, 4, 5 };

__inline static u32
twiddle_optimiz3d(u32 value, int n)
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

//input : address in the yyyyyxxxxx format
//output : address in the xyxyxyxy format
//U : x resolution , V : y resolution
//twidle works on 64b words
u32 fastcall twiddle_razi(u32 x,u32 y,u32 x_sz,u32 y_sz)
{
	//u32 rv2=twiddle_optimiz3d(raw_addr,U);
	u32 rv=0;//raw_addr & 3;//low 2 bits are directly passed  -> needs some misc stuff to work.However
			 //Pvr internaly maps the 64b banks "as if" they were twidled :p
	
	//verify(x_sz==y_sz);
	u32 sh=0;
	x_sz>>=1;
	y_sz>>=1;
	while(x_sz!=0 || y_sz!=0)
	{
		if (y_sz)
		{
			u32 temp=y&1;
			rv|=temp<<sh;

			y_sz>>=1;
			y>>=1;
			sh++;
		}
		if (x_sz)
		{
			u32 temp=x&1;
			rv|=temp<<sh;

			x_sz>>=1;
			x>>=1;
			sh++;
		}
	}	
	return rv;
}

//# define twop( val, n )	twidle_razi( (val), (n),(n) )
#define twop twiddle_razi

void fastcall argb4444to8888(PixelBuffer* pb,u16* p_in,u32 Width,u32 Height)
{
	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u16 pval = *p_in++;
			pb->SetLinePixel(x,ARGB4444(pval));
		}
		pb->NextLine();
	}
}
void fastcall argb1555to8888(PixelBuffer* pb,u16* p_in,u32 Width,u32 Height)
{
	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u16 pval = *p_in++;
			pb->SetLinePixel(x,ARGB1555(pval));
		}
		pb->NextLine();
	}
}
void fastcall argb565to8888(PixelBuffer* pb,u16* p_in,u32 Width,u32 Height)
{
	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u16 pval = *p_in++;
			pb->SetLinePixel(x,ARGB565(pval));
		}
		pb->NextLine();
	}
}


void fastcall argb4444to8888_TW(PixelBuffer* pb,u16* p_in,u32 Width,u32 Height)
{
	u32 p=0;
	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u16 pval = p_in[twop(x,y,Width,Height)];
			p++;
			pb->SetLinePixel(x,ARGB4444(pval));
		}
		pb->NextLine();
	}
}
void fastcall argb1555to8888_TW(PixelBuffer* pb,u16* p_in,u32 Width,u32 Height)
{
	u32 p=0;
	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u16 pval = p_in[twop(x,y,Width,Height)];
			p++;
			pb->SetLinePixel(x,ARGB1555(pval));
		}
		pb->NextLine();
	}
}
void fastcall argb565to8888_TW(PixelBuffer* pb,u16* p_in,u32 Width,u32 Height)
{
	u32 p=0;
	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u16 pval = p_in[twop(x,y,Width,Height)];
			p++;
			pb->SetLinePixel(x,ARGB565(pval));
		}
		pb->NextLine();
	}
}


u32 vq_codebook[256][4];
void fastcall vq_codebook_argb565(u16* p_in)
{
	for( u32 i=0; i<256;i++) 
	{
		for( u32 j=0; j<4; j++ )
		{
			vq_codebook[i][j] = ARGB565(*p_in);
			p_in++;
		}
	}
}
void fastcall vq_codebook_argb1555(u16* p_in)
{
	for( u32 i=0; i<256;i++) 
	{
		for( u32 j=0; j<4; j++ )
		{
			vq_codebook[i][j] = ARGB1555(*p_in);
			p_in++;
		}
	}
}
void fastcall vq_codebook_argb4444(u16* p_in)
{
	for( u32 i=0; i<256;i++) 
	{
		for( u32 j=0; j<4; j++ )
		{
			vq_codebook[i][j] = ARGB4444(*p_in);
			p_in++;
		}
	}
}

void fastcall vq_TW(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height)
{
	p_in+=256*4*2;
	u32 p=0;
	//U-=1;//half the height , b/c VQ is 2 pix/ 1 byte on each direction ;)
	Height>>=1;
	Width>>=1;

	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u8 pval = p_in[twop(x,y,Width<<1,Height)];
			p++;
			pb->SetPixel(x*2	,y*2	,	vq_codebook[pval][0]);
			pb->SetPixel(x*2	,y*2+1	,	vq_codebook[pval][1]);
			pb->SetPixel(x*2+1	,y*2	,	vq_codebook[pval][2]);
			pb->SetPixel(x*2+1	,y*2+1	,	vq_codebook[pval][3]);
		}
	}
}