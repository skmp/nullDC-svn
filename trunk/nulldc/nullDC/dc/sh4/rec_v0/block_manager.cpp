#include "block_manager.h"

//4096 entry hash
GrowingList<recBlock> recHash[0x10000];

//hash function
INLINE u32 __fastcall GetHash(u32 value)
{
	//H are the nibles uses for hash
	//0x000H0HH0
	u32 hashv=(value>>2)&0xFFFF;//((value>>4)&0xFF) | ((value>>8)&0xF00);
	return hashv;
}

recBlock* FindBlock(u32 start)
{
	GrowingList<recBlock>* hash =&recHash[GetHash(start)];

	for (u32 i=0;i<hash->items.Size;i++)
	{
		if ((hash->items[i].used) && (*hash)[i].StartAddr==start)
		{
			return &((*hash)[i]);
		}
	}
	return 0;
}

recBlock* AddBlock(u32 start)
{
	GrowingList<recBlock>* hash =&recHash[GetHash(start)];
	recBlock temp;
	temp.StartAddr=start;

	u32 index=hash->Add(temp);
	return &((*hash)[index]);
}

bool RemoveBlock(u32 start)
{
	GrowingList<recBlock>* hash =&recHash[GetHash(start)];

	for (u32 i=0;i<hash->items.Size;i++)
	{
		if ((hash->items[i].used) && ((*hash)[i].StartAddr==start))
		{
			hash->Remove(i);//remove it :P	
			return true;
		}
	}
	return false;
}
void InitHash()
{
	for (int i=0;i<0x10000;i++)
	{
		recHash[i].items.Resize(1,true);
	}
}