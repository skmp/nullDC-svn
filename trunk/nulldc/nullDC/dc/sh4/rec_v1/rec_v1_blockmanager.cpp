#include "rec_v1_blockmanager.h"

#include <vector>
using namespace std;

vector<rec_v1_BasicBlock*> blocklist[65536];


rec_v1_BasicBlock* rec_v1_FindBlock(u32 address)
{
	vector<rec_v1_BasicBlock*>*blocklst= &blocklist[(address>>3)&0xFFFF];

	for (u32 i=0;i<blocklst->size();i++)
	{
		if ((*blocklst)[i]->start==address)
			return (*blocklst)[i];
	}

	return 0;
}


rec_v1_BasicBlock* rec_v1_AddBlock(u32 address)
{
	rec_v1_BasicBlock* rv=new rec_v1_BasicBlock();
	rv->start=address;
	
	vector<rec_v1_BasicBlock*>*blocklst= &blocklist[(address>>3)&0xFFFF];

	blocklst->push_back(rv);

	return rv;
}