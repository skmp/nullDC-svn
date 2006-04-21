#include "rec_v1_basicblock.h"

void link_compile_inject_TF_stub(rec_v1_BasicBlock* ptr);
void link_compile_inject_TT_stub(rec_v1_BasicBlock* ptr);

void rec_v1_BasicBlock::Discard()
{
	int lsz=reflist.size();
	for (int i=0;i<lsz;i++)
	{
		rec_v1_BasicBlock* pbl=reflist[i];

		if (pbl->pTF_next_addr==compiled->Code)
		{
			pbl->pTF_next_addr=link_compile_inject_TF_stub;
		}

		if (pbl->pTT_next_addr==compiled->Code)
		{
			pbl->pTT_next_addr=link_compile_inject_TT_stub;
		}
	}

	free(this->compiled->Code);
	delete this->compiled;
}