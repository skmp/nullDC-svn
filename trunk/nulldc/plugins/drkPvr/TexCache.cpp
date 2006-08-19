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

u32 unpack_1_to_8[2]={0,0xFF};

#define ARGB8888(A,R,G,B) \
	( ((A)<<24) | ((R)<<0) | ((G)<<8) | ((B)<<16))

#define ARGB1555( word )	\
	ARGB8888(unpack_1_to_8[word>>15],unpack_5_to_8[(word>>10) & 0x1F],	\
	unpack_5_to_8[(word>>5) & 0x1F],unpack_5_to_8[word&0x1F])

#define ARGB565( word )		\
	ARGB8888(0xFF,unpack_5_to_8[(word>>11) & 0x1F],	\
	unpack_5_to_8[(word>>5) & 0x3F],unpack_5_to_8[word&0x1F])
	//( 0xFF000000 | unpack_5_to_8[(word>>11) & 0x1F] | unpack_5_to_8[(word>>5) & 0x3F]<<8 | unpack_5_to_8[word&0x1F]<<16 )


void fastcall argb1555to8888(u32* p_out,u16* p_in,u32 Width,u32 Height)
{
	for(u32 p=0; p<(Width * Height); p++)
	{
		u16 pval = p_in[p];
		p_out[p] = ARGB1555(pval) ;
	}
}
void fastcall argb565to8888(u32* p_out,u16* p_in,u32 Width,u32 Height)
{
	for(u32 p=0; p<(Width * Height); p++)
	{
		u16 pval = p_in[p];
		p_out[p] = ARGB565(pval) ;
	}
}