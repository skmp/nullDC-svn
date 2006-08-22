#pragma once
#include "drkPvr.h"

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


struct PixelBuffer
{
	u32* p_buffer_start;
	u32* p_current_line;
	u32 pixels_per_line;

	void SetPixel(u32 x,u32 y , u32 value)
	{
		p_buffer_start[y*pixels_per_line+x]=value;
	}
	void NextLine()
	{
		p_current_line+=pixels_per_line;
	}
	void SetLinePixel(u32 x  , u32 value)
	{
		p_current_line[x]=value;
	}
};
void fastcall argb1555to8888(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);
void fastcall argb565to8888(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);
void fastcall argb4444to8888(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height);

void fastcall argb1555to8888_TW(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height,u32 U);
void fastcall argb565to8888_TW(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height,u32 U);
void fastcall argb4444to8888_TW(PixelBuffer* p_out,u16* p_in,u32 Width,u32 Height,u32 U);


void fastcall vq_codebook_argb565(u16* p_in);
void fastcall vq_codebook_argb1555(u16* p_in);
void fastcall vq_codebook_argb4444(u16* p_in);
void fastcall vq_TW(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height,u32 U);