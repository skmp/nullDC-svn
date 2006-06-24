////////////////////////////////////////////////////////////////////////////////////////
/// @file  yuvConverter.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "yuvConverter.h"


static unsigned char* Clip;

void InitYuvConverter()
{
	int i;

	Clip = NEW(unsigned char[1024]);

	Clip += 384;

	for (i=-384; i<640; i++)
		Clip[i] = (i<0) ? 0 : ((i>255) ? 255 : i);
}



/* vertical 1:2 interpolation filter */
void YUVconv420to422(unsigned char* src,unsigned char* dst, int w, int h)
{
	int i, j, j2;
	int jm6, jm5, jm4, jm3, jm2, jm1, jp1, jp2, jp3, jp4, jp5, jp6, jp7;

	w = w>>1;
	h = h>>1;

	if (true)
	{
		/* intra frame */
		for (i=0; i<w; i++)
		{
			for (j=0; j<h; j++)
			{
				j2 = j<<1;
				jm3 = (j<3) ? 0 : j-3;
				jm2 = (j<2) ? 0 : j-2;
				jm1 = (j<1) ? 0 : j-1;
				jp1 = (j<h-1) ? j+1 : h-1;
				jp2 = (j<h-2) ? j+2 : h-1;
				jp3 = (j<h-3) ? j+3 : h-1;

				/* FIR filter coefficients (*256): 5 -21 70 228 -37 11 */
				/* New FIR filter coefficients (*256): 3 -16 67 227 -32 7 */
				dst[w*j2] =     Clip[(int)(  3*src[w*jm3]
				-16*src[w*jm2]
				+67*src[w*jm1]
				+227*src[w*j]
				-32*src[w*jp1]
				+7*src[w*jp2]+128)>>8];

				dst[w*(j2+1)] = Clip[(int)(  3*src[w*jp3]
				-16*src[w*jp2]
				+67*src[w*jp1]
				+227*src[w*j]
				-32*src[w*jm1]
				+7*src[w*jm2]+128)>>8];
			}
			src++;
			dst++;
		}
	}
	else
	{
		/* intra field */
		for (i=0; i<w; i++)
		{
			for (j=0; j<h; j+=2)
			{
				j2 = j<<1;

				/* top field */
				jm6 = (j<6) ? 0 : j-6;
				jm4 = (j<4) ? 0 : j-4;
				jm2 = (j<2) ? 0 : j-2;
				jp2 = (j<h-2) ? j+2 : h-2;
				jp4 = (j<h-4) ? j+4 : h-2;
				jp6 = (j<h-6) ? j+6 : h-2;

				/* Polyphase FIR filter coefficients (*256): 2 -10 35 242 -18 5 */
				/* New polyphase FIR filter coefficients (*256): 1 -7 30 248 -21 5 */
				dst[w*j2] = Clip[(int)(  1*src[w*jm6]
				-7*src[w*jm4]
				+30*src[w*jm2]
				+248*src[w*j]
				-21*src[w*jp2]
				+5*src[w*jp4]+128)>>8];

				/* Polyphase FIR filter coefficients (*256): 11 -38 192 113 -30 8 */
				/* New polyphase FIR filter coefficients (*256):7 -35 194 110 -24 4 */
				dst[w*(j2+2)] = Clip[(int)( 7*src[w*jm4]
				-35*src[w*jm2]
				+194*src[w*j]
				+110*src[w*jp2]
				-24*src[w*jp4]
				+4*src[w*jp6]+128)>>8];

				/* bottom field */
				jm5 = (j<5) ? 1 : j-5;
				jm3 = (j<3) ? 1 : j-3;
				jm1 = (j<1) ? 1 : j-1;
				jp1 = (j<h-1) ? j+1 : h-1;
				jp3 = (j<h-3) ? j+3 : h-1;
				jp5 = (j<h-5) ? j+5 : h-1;
				jp7 = (j<h-7) ? j+7 : h-1;

				/* Polyphase FIR filter coefficients (*256): 11 -38 192 113 -30 8 */
				/* New polyphase FIR filter coefficients (*256):7 -35 194 110 -24 4 */
				dst[w*(j2+1)] = Clip[(int)( 7*src[w*jp5]
				-35*src[w*jp3]
				+194*src[w*jp1]
				+110*src[w*jm1]
				-24*src[w*jm3]
				+4*src[w*jm5]+128)>>8];

				dst[w*(j2+3)] = Clip[(int)(  1*src[w*jp7]
				-7*src[w*jp5]
				+30*src[w*jp3]
				+248*src[w*jp1]
				-21*src[w*jm1]
				+5*src[w*jm3]+128)>>8];
			}
			src++;
			dst++;
		}
	}
}



void YUVconv420to422ToTexture(unsigned char* src,unsigned char* dst, int w, int h)
{

}