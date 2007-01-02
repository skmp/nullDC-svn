#pragma once
#include "drkPvr.h"

extern u8* vq_codebook;
//Generic texture cache list class =P
template <class TexEntryType>
class TexCacheList
{
	public:
	class TexCacheEntry
	{
	public:
		TexCacheEntry(TexCacheEntry* prevt,TexCacheEntry* nextt,TexEntryType* textt)
		{
			prev=prevt;
			next=nextt;
			if (textt)
				data=*textt;
		}
		TexCacheEntry* prev;
		TexCacheEntry* next;
		TexEntryType data;
	};
	u32 textures;
	TexCacheEntry* pfirst;
	TexCacheEntry* plast;

	TexCacheList()
	{
		pfirst=0;
		plast=0;
		textures=0;
	}


	TexEntryType* Find(u32 address)
	{
		TexCacheEntry* pl= this->pfirst;
		while (pl)
		{
			if (pl->data.Start==address)
			{
				if (pl->prev!=0)//its not the first one
				{
					if (pl->next==0)
					{
						//if last one then , the last one is the previus
						plast=pl->prev;
					}
					
					//remove the texture from the list
					textures++;//one is counted down by remove ;)
					Remove(pl);
					
					//add it on top
					pl->prev=0;			//no prev , we are first
					pl->next=pfirst;	//after us is the old first
					pfirst->prev=pl;	//we are before the old first
					
					//replace pfirst pointer
					pfirst=pl;

				}
				return &pl->data;
			}
			else
				pl=pl->next;
		}

		return 0;
	}

	TexCacheEntry* Add(TexEntryType* text )
	{
		if (plast==0)
		{
			if (pfirst!=0)
			{
				printf("Texture Cache Error , pfirst!=0 && plast==0\n");
			}
			pfirst=plast=new TexCacheEntry(0,0,text);
		}
		else
		{
			plast=new TexCacheEntry(plast,0,text);
			plast->prev->next=plast;
		}

		textures++;
		return plast;
	}
	void Remove(TexCacheEntry* texture)
	{
		textures--;
		if (texture==pfirst)
		{
			if (texture->next)
				pfirst=texture->next;
			else
				pfirst=0;
		}
		if (texture==plast)
		{
			if (texture->prev)
				plast=texture->prev;
			else
				plast=0;
		}

		//if not last one , remove it from next
		if (texture->next!=0)
			texture->next->prev=texture->prev;
		//if not first one , remove it from prev
		if (texture->prev!=0)
			texture->prev->next=texture->next;
	}
};


//Pixel buffer class (realy helpfull ;) )
struct PixelBuffer
{
	u32* p_buffer_start;
	u32* p_current_line;
	u32* p_current_pixel;

	u32 pixels_per_line;

	void prel(u32 x,u32 value)
	{
		p_current_pixel[x]=value;
	}

	void prel(u32 x,u32 y,u32 value)
	{
		p_current_pixel[y*pixels_per_line+x]=value;
	}

	void rmovex(u32 value)
	{
		p_current_pixel+=value;
	}
	void rmovey(u32 value)
	{
		p_current_line+=pixels_per_line*value;
		p_current_pixel=p_current_line;
	}
	void amove(u32 x_m,u32 y_m)
	{
		//p_current_pixel=p_buffer_start;
		p_current_line=p_buffer_start+pixels_per_line*y_m;
		p_current_pixel=p_current_line + x_m;
	}
};

//Generic texture cache / texture format conevertion code / macros
const u32 unpack_5_to_8[32] = 
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

const u32 unpack_6_to_8[64] = 
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

const u32 unpack_1_to_8[2]={0,0xFF};

#define clamp(minv,maxv,x) min(maxv,max(minv,x))

#define ARGB8888(A,R,G,B) \
	PixelPacker::pack(A,R,G,B)

#define ARGB1555( word )	\
	ARGB8888(unpack_1_to_8[(word>>15)&1],unpack_5_to_8[(word>>10) & 0x1F],	\
	unpack_5_to_8[(word>>5) & 0x1F],unpack_5_to_8[word&0x1F])

#define ARGB565( word )		\
	ARGB8888(0xFF,unpack_5_to_8[(word>>11) & 0x1F],	\
	unpack_6_to_8[(word>>5) & 0x3F],unpack_5_to_8[word&0x1F])
	//( 0xFF000000 | unpack_5_to_8[(word>>11) & 0x1F] | unpack_5_to_8[(word>>5) & 0x3F]<<8 | unpack_5_to_8[word&0x1F]<<16 )

#define ARGB4444( word )	\
	ARGB8888( (word&0xF000)>>(12-4),(word&0xF00)>>(8-4),(word&0xF0)>>(4-4),(word&0xF)<<4 )

template<class PixelPacker>
u32 YUV422(s32 Y,s32 Yu,s32 Yv)
{
	s32 B = (76283*(Y - 16) + 132252*(Yu - 128))>>16;
	s32 G = (76283*(Y - 16) - 53281 *(Yv - 128) - 25624*(Yu - 128))>>16;
	s32 R = (76283*(Y - 16) + 104595*(Yv - 128))>>16;

	return ARGB8888(255,clamp(0,255,R),clamp(0,255,G),clamp(0,255,B));
}

u32 fastcall twiddle_razi(u32 x,u32 y,u32 x_sz,u32 y_sz);
#define twop twiddle_razi

//pixel packers !
struct pp_dx
{
	__forceinline 
	static u32 pack(u8 A,u8 R,u8 G,u8 B)
	{
		return ( ((A)<<24) | ((R)<<16) | ((G)<<8) | ((B)<<0));
	}
};
struct pp_gl
{
	__forceinline 
	static u32 pack(u8 A,u8 R,u8 G,u8 B)
	{
		return ( ((A)<<24) | ((B)<<16) | ((G)<<8) | ((R)<<0));
	}
};

//pixel convertors !
//Non twiddled
template<class PixelPacker>
struct conv565_PL
{
	static const u32 xpp=4;	//x pixels per pass
	static const u32 ypp=1;	//y pixels per pass
	static void fastcall Convert(PixelBuffer* pb,u8* data)
	{
		//convert 4x1 565 to 4x1 8888
		u16* p_in=(u16*)data;
		//0,0
		pb->prel(0,ARGB565(p_in[0]));
		//1,0
		pb->prel(1,ARGB565(p_in[1]));
		//2,0
		pb->prel(2,ARGB565(p_in[2]));
		//3,0
		pb->prel(3,ARGB565(p_in[3]));
	}
};

template<class PixelPacker>
struct conv1555_PL
{
	static const u32 xpp=4;	//x pixels per pass
	static const u32 ypp=1;	//y pixels per pass
	static void fastcall Convert(PixelBuffer* pb,u8* data)
	{
		//convert 4x1 1555 to 4x1 8888
		u16* p_in=(u16*)data;
		//0,0
		pb->prel(0,ARGB1555(p_in[0]));
		//1,0
		pb->prel(1,ARGB1555(p_in[1]));
		//2,0
		pb->prel(2,ARGB1555(p_in[2]));
		//3,0
		pb->prel(3,ARGB1555(p_in[3]));
	}
};

template<class PixelPacker>
struct conv4444_PL
{
	static const u32 xpp=4;	//x pixels per pass
	static const u32 ypp=1;	//y pixels per pass
	static void fastcall Convert(PixelBuffer* pb,u8* data)
	{
		//convert 4x1 4444 to 4x1 8888
		u16* p_in=(u16*)data;
		//0,0
		pb->prel(0,ARGB4444(p_in[0]));
		//1,0
		pb->prel(1,ARGB4444(p_in[1]));
		//2,0
		pb->prel(2,ARGB4444(p_in[2]));
		//3,0
		pb->prel(3,ARGB4444(p_in[3]));
	}
};

template<class PixelPacker>
struct convYUV_PL
{
	static const u32 xpp=4;	//x pixels per pass
	static const u32 ypp=1;	//y pixels per pass
	static void fastcall Convert(PixelBuffer* pb,u8* data)
	{
		//convert 4x1 4444 to 4x1 8888
		u16* p_in=(u16*)data;

		
		s32 Y0 = (p_in[0]>>8) &255; //
		s32 Yu = (p_in[0]>>0) &255; //p_in[0]
		s32 Y1 = (p_in[1]>>8) &255; //p_in[3]
		s32 Yv = (p_in[1]>>0) &255; //p_in[2]

		//0,0
		pb->prel(0,YUV422<PixelPacker>(Y0,Yu,Yv));
		//1,0
		pb->prel(1,YUV422<PixelPacker>(Y1,Yu,Yv));
		
		//next 4 bytes
		p_in+=2;

		Y0 = (p_in[0]>>8) &255; //
		Yu = (p_in[0]>>0) &255; //p_in[0]
		Y1 = (p_in[1]>>8) &255; //p_in[3]
		Yv = (p_in[1]>>0) &255; //p_in[2]

		//0,0
		pb->prel(2,YUV422<PixelPacker>(Y0,Yu,Yv));
		//1,0
		pb->prel(3,YUV422<PixelPacker>(Y1,Yu,Yv));
	}
};


//twiddled 
template<class PixelPacker>
struct conv565_TW
{
	static const u32 xpp=2;	//x pixels per pass
	static const u32 ypp=2;	//y pixels per pass
	static void fastcall Convert(PixelBuffer* pb,u8* data)
	{
		//convert 4x1 565 to 4x1 8888
		u16* p_in=(u16*)data;
		//0,0
		pb->prel(0,0,ARGB565(p_in[0]));
		//0,1
		pb->prel(0,1,ARGB565(p_in[1]));
		//1,0
		pb->prel(1,0,ARGB565(p_in[2]));
		//1,1
		pb->prel(1,1,ARGB565(p_in[3]));
	}
};

template<class PixelPacker>
struct conv1555_TW
{
	static const u32 xpp=2;	//x pixels per pass
	static const u32 ypp=2;	//y pixels per pass
	static void fastcall Convert(PixelBuffer* pb,u8* data)
	{
		//convert 4x1 1555 to 4x1 8888
		u16* p_in=(u16*)data;
		//0,0
		pb->prel(0,0,ARGB1555(p_in[0]));
		//0,1
		pb->prel(0,1,ARGB1555(p_in[1]));
		//1,0
		pb->prel(1,0,ARGB1555(p_in[2]));
		//1,1
		pb->prel(1,1,ARGB1555(p_in[3]));
	}
};

template<class PixelPacker>
struct conv4444_TW
{
	static const u32 xpp=2;	//x pixels per pass
	static const u32 ypp=2;	//y pixels per pass
	static void fastcall Convert(PixelBuffer* pb,u8* data)
	{
		//convert 4x1 4444 to 4x1 8888
		u16* p_in=(u16*)data;
		//0,0
		pb->prel(0,0,ARGB4444(p_in[0]));
		//0,1
		pb->prel(0,1,ARGB4444(p_in[1]));
		//1,0
		pb->prel(1,0,ARGB4444(p_in[2]));
		//1,1
		pb->prel(1,1,ARGB4444(p_in[3]));
	}
};

template<class PixelPacker>
struct convYUV_TW
{
	static const u32 xpp=2;	//x pixels per pass
	static const u32 ypp=2;	//y pixels per pass
	static void fastcall Convert(PixelBuffer* pb,u8* data)
	{
		//convert 4x1 4444 to 4x1 8888
		u16* p_in=(u16*)data;

		
		s32 Y0 = (p_in[0]>>8) &255; //
		s32 Yu = (p_in[0]>>0) &255; //p_in[0]
		s32 Y1 = (p_in[2]>>8) &255; //p_in[3]
		s32 Yv = (p_in[2]>>0) &255; //p_in[2]

		//0,0
		pb->prel(0,0,YUV422<PixelPacker>(Y0,Yu,Yv));
		//1,0
		pb->prel(1,0,YUV422<PixelPacker>(Y1,Yu,Yv));
		
		//next 4 bytes
		//p_in+=2;

		Y0 = (p_in[1]>>8) &255; //
		Yu = (p_in[1]>>0) &255; //p_in[0]
		Y1 = (p_in[3]>>8) &255; //p_in[3]
		Yv = (p_in[3]>>0) &255; //p_in[2]

		//0,1
		pb->prel(0,1,YUV422<PixelPacker>(Y0,Yu,Yv));
		//1,1
		pb->prel(1,1,YUV422<PixelPacker>(Y1,Yu,Yv));
	}
};


template<class PixelPacker>
struct convPAL4_TW
{
	static const u32 xpp=4;	//x pixels per pass
	static const u32 ypp=4;	//y pixels per pass
	static void fastcall Convert(PixelBuffer* pb,u8* data)
	{
		u8* p_in=(u8*)data;
	}
};
template<class PixelPacker>
struct convPAL8_TW
{
	static const u32 xpp=2;	//x pixels per pass
	static const u32 ypp=4;	//y pixels per pass
	static void fastcall Convert(PixelBuffer* pb,u8* data)
	{
		u8* p_in=(u8*)data;
	}
};
//hanlder functions
template<class PixelConvertor>
void fastcall texture_PL(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height)
{
	u32 p=0;
	pb->amove(0,0);

	Height/=PixelConvertor::ypp;
	Width/=PixelConvertor::xpp;

	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u8* p = p_in;
			PixelConvertor::Convert(pb,p);
			p_in+=8;

			pb->rmovex(PixelConvertor::xpp);
		}
		pb->rmovey(PixelConvertor::ypp);
	}
}

template<class PixelConvertor>
void fastcall texture_TW(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height)
{
	u32 p=0;
	pb->amove(0,0);
	
	Height/=PixelConvertor::ypp;
	Width/=PixelConvertor::xpp;

	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u8* p = &p_in[twop(x,y,Width,Height)<<3];
			PixelConvertor::Convert(pb,p);

			pb->rmovex(PixelConvertor::xpp);
		}
		pb->rmovey(PixelConvertor::ypp);
	}
}

template<class PixelConvertor>
void fastcall texture_VQ(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height)
{
	p_in+=256*4*2;
	u32 p=0;
	pb->amove(0,0);
	
	Height/=PixelConvertor::ypp;
	Width/=PixelConvertor::xpp;

	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u8 p = p_in[twop(x,y,Width,Height)];
			PixelConvertor::Convert(pb,&vq_codebook[p*8]);

			pb->rmovex(PixelConvertor::xpp);
		}
		pb->rmovey(PixelConvertor::ypp);
	}
}

//We ask the compiler to generate the templates here
//;)
//planar formats !
template void fastcall texture_PL<conv4444_PL<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_PL<conv565_PL<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_PL<conv1555_PL<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_PL<convYUV_PL<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);

//twiddled formats !
template void fastcall texture_TW<conv4444_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_TW<conv565_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_TW<conv1555_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_TW<convYUV_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_TW<convPAL4_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_TW<convPAL8_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);

//VQ formats !
template void fastcall texture_VQ<conv4444_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_VQ<conv565_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_VQ<conv1555_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_VQ<convYUV_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_VQ<convPAL4_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_VQ<convPAL8_TW<pp_dx>>(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);

//#defines to keep compat w/ older code
//void fastcall argb1555to8888(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);
#define argb1555to8888 texture_PL<conv1555_PL<pp_dx>>
//void fastcall argb565to8888(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);
#define argb565to8888 texture_PL<conv565_PL<pp_dx>>
//void fastcall argb4444to8888(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);
#define argb4444to8888 texture_PL<conv4444_PL<pp_dx>>
//void fastcall YUV422to8888(PixelBuffer* pb,u16* p_in,u32 Width,u32 Height);
#define YUV422to8888 texture_PL<convYUV_PL<pp_dx>>

//void fastcall argb1555to8888_TW(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);
#define argb1555to8888_TW texture_TW<conv1555_TW<pp_dx>>
//void fastcall argb565to8888_TW(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);
#define argb565to8888_TW texture_TW<conv565_TW<pp_dx>>
//void fastcall argb4444to8888_TW(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);
#define argb4444to8888_TW texture_TW<conv4444_TW<pp_dx>>
//void fastcall YUV422to8888_TW(PixelBuffer* pb,u16* p_in,u32 Width,u32 Height);
#define YUV422to8888_TW texture_TW<convYUV_TW<pp_dx>>
//void fastcall PAL4to8888_TW(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height,u32 pal_base);
#define PAL4to8888_TW texture_TW<convPAL4_TW<pp_dx>>
//void fastcall PAL8to8888_TW(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height,u32 pal_base);
#define PAL8to8888_TW  texture_TW<convPAL8_TW<pp_dx>>

//void fastcall vq_codebook_argb565(u16* p_in);
//void fastcall vq_codebook_argb1555(u16* p_in);
//void fastcall vq_codebook_argb4444(u16* p_in);
//void fastcall vq_codebook_YUV422(u16* p_in);
//void fastcall vq_codebook_PAL4(u16* p_in);
//void fastcall vq_codebook_PAL8(u16* p_in);

//void fastcall argb1555to8888_TW(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);
#define argb1555to8888_VQ texture_VQ<conv1555_TW<pp_dx>>
//void fastcall argb565to8888_TW(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);
#define argb565to8888_VQ texture_VQ<conv565_TW<pp_dx>>
//void fastcall argb4444to8888_TW(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);
#define argb4444to8888_VQ texture_VQ<conv4444_TW<pp_dx>>
//void fastcall YUV422to8888_TW(PixelBuffer* pb,u16* p_in,u32 Width,u32 Height);
#define YUV422to8888_VQ texture_VQ<convYUV_TW<pp_dx>>
//void fastcall PAL4to8888_TW(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height,u32 pal_base);
#define PAL4to8888_VQ texture_VQ<convPAL4_TW<pp_dx>>
//void fastcall PAL8to8888_TW(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height,u32 pal_base);
#define PAL8to8888_VQ  texture_VQ<convPAL8_TW<pp_dx>>

/*
void fastcall texture_VQ_argb565(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
void fastcall texture_VQ_argb1555(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
void fastcall texture_VQ_argb4444(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
void fastcall texture_VQ_YUV422(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
void fastcall texture_VQ_PAL4(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
void fastcall texture_VQ_PAL8(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
*/

void fastcall palette_PAL4(u32 offset);
void fastcall texture_PAL4(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);

void fastcall palette_PAL8(u32 offset);
void fastcall texture_PAL8(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);


