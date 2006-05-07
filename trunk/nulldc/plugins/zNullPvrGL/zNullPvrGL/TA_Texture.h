/*
**	TA_Param.h - David Miller 2006 - PowerVR2 Emulation Library
*/

#ifndef __TATEXTURE_H__
#define __TATEXTURE_H__

#include <vector>
using namespace std;

typedef unsigned int TexID;

struct TexEntry
{
	TexID texID;
	u32 Start, End;
	u32 Width, Height;
	vram_block* lock_block;
	//Flags, Priority, Residence, ...
};


class TextureCache
{
public:


//	TexID GetTexture(u32 Address);
	TexID GetTexture(PolyParam *pp);

	vector<TexEntry> TexList;

	void ClearTCache();
};



#endif //__TATEXTURE_H__
