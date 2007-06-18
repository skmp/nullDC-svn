#include "TexCache.h"
#include "regs.h"


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

u8* vq_codebook;
u32 palette_index;
u32 palette_lut[1024];
bool pal_needs_update=true;
u32 _pal_rev_256[4]={0};
u32 _pal_rev_16[64]={0};
u32 pal_rev_256[4]={0};
u32 pal_rev_16[64]={0};
void palette_update()
{
	if (pal_needs_update==false)
		return;
	memcpy(pal_rev_256,_pal_rev_256,sizeof(pal_rev_256));
	memcpy(pal_rev_16,_pal_rev_16,sizeof(pal_rev_16));

#define PixelPacker pp_dx
	pal_needs_update=false;
	switch(PAL_RAM_CTRL&3)
	{
	case 0:
		for (int i=0;i<1024;i++)
		{
			palette_lut[i]=ARGB1555(PALETTE_RAM[i]);
		}
		break;

	case 1:
		for (int i=0;i<1024;i++)
		{
			palette_lut[i]=ARGB565(PALETTE_RAM[i]);
		}
		break;

	case 2:
		for (int i=0;i<1024;i++)
		{
			palette_lut[i]=ARGB4444(PALETTE_RAM[i]);
		}
		break;

	case 3:
		for (int i=0;i<1024;i++)
		{
			palette_lut[i]=PALETTE_RAM[i];//argb 8888 :p
		}
		break;
	}

}
/*
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

void fastcall YUV422to8888(PixelBuffer* pb,u16* p_in,u32 Width,u32 Height)
{
	u32 p=0;
	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x+=2)
		{
			u32 YUV0 = p_in[p];p++;
			u32 YUV1 = p_in[p];p++;

			s32 Y0 = (YUV0>>8) &255;
			s32 Yu = (YUV0>>0) &255;
			s32 Y1 = (YUV1>>8) &255;
			s32 Yv = (YUV1>>0) &255;

			pb->SetLinePixel(x,YUV422(Y0,Yu,Yv));

			pb->SetLinePixel(x+1,YUV422(Y1,Yu,Yv));
		}
		pb->NextLine();
	}
}
void fastcall YUV422to8888_TW(PixelBuffer* pb,u16* p_in,u32 Width,u32 Height)
{
	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x+=2)
		{
			u32 YUV0 = p_in[twop(x,y,Width,Height)];
			u32 YUV1 = p_in[twop(x+1,y,Width,Height)];

			s32 Y0 = (YUV0>>8) &255;
			s32 Yu = (YUV0>>0) &255;
			s32 Y1 = (YUV1>>8) &255;
			s32 Yv = (YUV1>>0) &255;

			pb->SetLinePixel(x,YUV422(Y0,Yu,Yv));

			pb->SetLinePixel(x+1,YUV422(Y1,Yu,Yv));
		}
		pb->NextLine();
	}
}

u32 palette_lut[1024];
void fastcall update_palette()
{
	switch(PAL_RAM_CTRL&3)
	{
	case 0:
		for (int i=0;i<1024;i++)
		{
			palette_lut[i]=ARGB1555(PALETTE_RAM[i]);
		}
		break;

	case 1:
		for (int i=0;i<1024;i++)
		{
			palette_lut[i]=ARGB565(PALETTE_RAM[i]);
		}
		break;

	case 2:
		for (int i=0;i<1024;i++)
		{
			palette_lut[i]=ARGB4444(PALETTE_RAM[i]);
		}
		break;

	case 3:
		for (int i=0;i<1024;i++)
		{
			palette_lut[i]=PALETTE_RAM[i];//argb 8888 :p
		}
		break;
	}

}

void fastcall PAL4to8888_TW(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height,u32 pal_base)
{
	update_palette();
	u32* pal=&palette_lut[pal_base];

	Width>>=1;
	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u32 index = p_in[twop(x,y,Width,Height)];

			pb->SetLinePixel(x*2,pal[index&0xF]);
			pb->SetLinePixel(x*2+1,pal[index>>4]);
		}
		pb->NextLine();
	}
}

void fastcall PAL8to8888_TW(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height,u32 pal_base)
{
	update_palette();
	u32* pal=&palette_lut[pal_base];
	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u32 index = p_in[twop(x,y,Width,Height)];

			pb->SetLinePixel(x,pal[index]);
		}
		pb->NextLine();
	}
}
*/

//mnn
//time for better texture code
//wiihhaa

/*

PowerVR internaly handles textures on 64bit each time.Twidlle is on 64bits too

So , it is a good idea to base this code on that.
Generic texture Handlers

struct PixelPacker
{
	void pack(pixelbuffer* pb,u8 R,u8 G,u8 B,u8 A)
};
template<class PixelPacker>
struct PixelConvertor
{
	const u32 xpp=1;	//x pixels per pass
	const u32 ypp=1;	//y pixels per pass
	const bool twidle=true;
	static void fastcall Convert(PixelBuffer* pb,u8* data);
};
*/
/*
void fastcall vq_codebook_argb565(u16* p_in)
{
	for( u32 i=0; i<256;i++) 
	{
		for( u32 j=0; j<4; j++ )
		{
			//vq_codebook[i][j] = ARGB565(*p_in);
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
			//vq_codebook[i][j] = ARGB1555(*p_in);
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
			//vq_codebook[i][j] = ARGB4444(*p_in);
			p_in++;
		}
	}
}
void fastcall vq_codebook_YUV422(u16* p_in)
{
	for( u32 i=0; i<256;i++) 
	{
		for( u32 j=0; j<4; j+=4 )
		{
			{
				u32 YUV0 = *p_in++;//p_in++;//Y0U
				u32 YUV1 = *p_in++;//p_in++;//Y1V
				

				s32 Y0 = (YUV0>>8) &255;
				s32 U = (YUV0>>0) &255;
				s32 Y1 = (YUV1>>8) &255;
				s32 V = (YUV1>>0) &255;
				
				//vq_codebook[i][3]=vq_codebook[i][1]=vq_codebook[i][2]=vq_codebook[i][0] = YUV422(Y1,U,V);
				//vq_codebook[i][j+1]=YUV422(Y1,U,V);
			}
			{
				u32 YUV0 = *p_in++;//p_in++;//Y0U
				u32 YUV1 = *p_in++;//p_in++;//Y1V
				

				s32 Y0 = (YUV0>>8) &255;
				s32 U = (YUV0>>0) &255;
				s32 Y1 = (YUV1>>8) &255;
				s32 V = (YUV1>>0) &255;
				
				//vq_codebook[i][3]=vq_codebook[i][1] = YUV422(Y0,U,V);
				//vq_codebook[i][j+2] = YUV422(Y1,U,V);
			}
		}
	}
}*/
/*
void fastcall texture_VQ(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height)
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
			u8 pval = p_in[twop(x,y,Width,Height)];
			p++;
//			pb->SetPixel(x*2	,y*2	,	vq_codebook[pval][0]);
//			pb->SetPixel(x*2	,y*2+1	,	vq_codebook[pval][1]);
//			pb->SetPixel(x*2+1	,y*2	,	vq_codebook[pval][2]);
//			pb->SetPixel(x*2+1	,y*2+1	,	vq_codebook[pval][3]);
		}
	}
}*/