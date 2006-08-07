#pragma once
#include "types.h"
#include "dc\sh4\ccn.h"
struct TLB_AddressEntry
{
	union
	{
		struct
		{
			u32 ASID:8;
			u32 V:1;
			u32 res:1;
			u32 VPN:22;
		};
		u32 full;
	};
};

struct TLB_DataEntry
{
	union
	{
		struct
		{
		u32 WT:1;
		u32 SH:1;
		u32 D :1;
		u32 C :1;
		
		u32 SZ0:1;
		u32 PR :2;
		u32 SZ1:1;

		u32 V:1;
		u32 res_0:1;
		u32 PPN:19;//PPN 10-28
		u32 res_1:3;
		};
		u32 full;
	};
};

struct TLB_Entry
{
	TLB_AddressEntry Address;
	TLB_DataEntry Data;
};

extern TLB_Entry UTLB[64];
extern TLB_Entry ITLB[4];

//sync mem mapping to mmu , suspend compiled blocks if needed.entry is a UTLB entry # , -1 is for full sync
void UTLB_Sync(u32 entry);
//sync mem mapping to mmu , suspend compiled blocks if needed.entry is a ITLB entry # , -1 is for full sync
void ITLB_Sync(u32 entry);

//Do a full lookup on the UTLB entry's
u32 mmu_full_lookup(u32 va);


void MMU_Init();
void MMU_Reset(bool Manual);
void MMU_Term();