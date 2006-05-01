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

	//Flags, Priority, Residence, ...
};


class TextureCache
{
public:


//	TexID GetTexture(u32 Address);
	TexID GetTexture(PolyParam *pp);

	vector<TexEntry> TexList;

	void ClearTCache()	{
		for(size_t i=0; i<TexList.size(); i++)
			glDeleteTextures(1,&TexList[i].texID);
	}

};



#endif //__TATEXTURE_H__
