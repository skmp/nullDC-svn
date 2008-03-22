////////////////////////////////////////////////////////////////////////////////////////
/// @file  yuvConverter.h
/// @brief 
////////////////////////////////////////////////////////////////////////////////////////

#ifndef _yuvConverter_h
#define _yuvConverter_h

void InitYuvConverter();
void YUVconv420to422(unsigned char* src,unsigned char* dst, int w, int h);
void YUVconv420to422ToTexture(unsigned char* src,unsigned char* dst, int w, int h);

#endif
