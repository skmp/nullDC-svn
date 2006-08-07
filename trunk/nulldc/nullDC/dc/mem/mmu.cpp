#include "mmu.h"

TLB_Entry UTLB[64];
TLB_Entry ITLB[4];
u32 mmu_mask[4]= 
{
	((0xFFFFFFFF)>>10)<<10,	//1 kb page
	((0xFFFFFFFF)>>12)<<12,	//4 kb page
	((0xFFFFFFFF)>>16)<<16,	//64 kb page
	((0xFFFFFFFF)>>20)<<20	//1 MB page
};

//sync mem mapping to mmu , suspend compiled blocks if needed.entry is a UTLB entry # , -1 is for full sync
void UTLB_Sync(u32 entry)
{
}
//sync mem mapping to mmu , suspend compiled blocks if needed.entry is a ITLB entry # , -1 is for full sync
void ITLB_Sync(u32 entry)
{
}

//Do a full lookup on the UTLB entry's
u32 mmu_full_lookup(u32 va)
{
	u32 entry=0;
	u32 rv;
	u32 nom=0;

	for (u32 i=0;i<64;i++)
	{
		if (UTLB[i].Address.V==0)
			continue;
		u32 sz=UTLB[i].Data.SZ1*2+UTLB[i].Data.SZ0;
		u32 mask=mmu_mask[sz];

		if (((UTLB[i].Address.VPN<<10)&mask)==va&mask)
		{
			entry=i;
			nom++;
			//VPN->PPN | low bits
			rv=((UTLB[i].Address.VPN<<10)&mask) | (va&(~mask));
		}
	}

	if (nom!=1)
	{
		if (nom)
			printf("UTLB : mutly hit %d\n",nom);
		else
			printf("UTLB : miss \n");

		return 0;
	}


}

void MMU_Init()
{
}


void MMU_Reset(bool Manual)
{
	memset(UTLB,0,sizeof(UTLB));
	memset(ITLB,0,sizeof(ITLB));
}

void MMU_Term()
{
}