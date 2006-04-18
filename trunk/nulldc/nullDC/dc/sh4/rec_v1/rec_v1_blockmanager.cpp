#include "rec_v1_blockmanager.h"

#include <vector>
using namespace std;

vector<rec_v1_BasicBlock*> blocklist;


rec_v1_BasicBlock* rec_v1_FindBlock(u32 address)
{
	for (u32 i=0;i<blocklist.size();i++)
	{
		if (blocklist[i]->start==address)
			return blocklist[i];
	}
}


rec_v1_BasicBlock* rec_v1_AddBlock(u32 address)
{
	rec_v1_BasicBlock* rv=new rec_v1_BasicBlock();
	rv->start=address;
	
	blocklist.push_back(rv);

	return rv;
}