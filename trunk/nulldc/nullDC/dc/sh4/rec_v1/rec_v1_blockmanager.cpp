#include "rec_v1_blockmanager.h"

#include <vector>
using namespace std;

#define block_cnt (0x1000) 
#define HASH_BITS 4
vector<rec_v1_BasicBlock*> blocklist[block_cnt];

u8 BitTest[8]={1<<0,1<<1,1<<2,1<<3,1<<4,1<<5,1<<6,1<<7};
u8 BitRes[8]={255-(1<<0),255-(1<<1),255-(1<<2),255-(1<<3),255-(1<<4),255-(1<<5),255-(1<<6),255-(1<<7)};
u8 RamTest[0x1000000>>(HASH_BITS+3)];

rec_v1_BasicBlock* rec_v1_FindBlock(u32 address)
{
	vector<rec_v1_BasicBlock*>*blocklst= &blocklist[(address>>2)&(block_cnt-1)];

	for (u32 i=0;i<blocklst->size();i++)
	{
		if ((*blocklst)[i]->start==address)
			return (*blocklst)[i];
	}

	return 0;
}


rec_v1_BasicBlock* rec_v1_NewBlock(u32 address)
{
	rec_v1_BasicBlock* rv=new rec_v1_BasicBlock();
	rv->start=address;

	return rv;
}

rec_v1_BasicBlock* rec_v1_AddBlock(u32 address)
{
	rec_v1_BasicBlock* rv=rec_v1_NewBlock(address);
	
	vector<rec_v1_BasicBlock*>*blocklst= &blocklist[(address>>2)&(block_cnt-1)];

	blocklst->push_back(rv);

	return rv;
}



void rec_v1_SetBlockTest(u32 addr)
{
	addr&=0xFFFFFF;
	addr>>=HASH_BITS;//32 kb chunks , 
	RamTest[addr>>3]|=BitTest[addr&7];
}

void rec_v1_ResetBlockTest(u32 addr)
{
	addr&=0xFFFFFF;
	addr>>=HASH_BITS;//32 kb chunks , 
	RamTest[addr>>3]&=BitRes[addr&7];
}

void rec_v1_BlockTest(u32 addr)
{
	addr&=0xFFFFFF;
	addr>>=HASH_BITS;//32 kb chunks , 
	if (RamTest[addr>>3]&BitTest[addr&7])
	{
		u32 bas=addr<<(HASH_BITS);
		for (int i=0;i<(1<<(HASH_BITS));i++)
		{
			blocklist[((bas +i)>>2)&(block_cnt-1)].clear();
			rec_v1_ResetBlockTest(bas+i);
		}
		//damn a block is overwrited
	}
}
void rec_v1_NotifyMemWrite(u32 start , u32 size)
{

}