#include "mmu.h"
#include "dc/sh4/sh4_if.h"
#include "dc/sh4/ccn.h"
#include "dc/sh4/intc.h"
#include "dc/sh4/sh4_registers.h"
#include "plugins/plugin_manager.h"


#include "_vmem.h"

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

void mmu_raise_exeption(u32 mmu_error,u32 address,u32 am)
{
	CCN_TEA=address;
	CCN_PTEH.VPN=address>>10;

	switch(mmu_error)
	{
	//No error
	case MMU_ERROR_NONE:
		printf("Error : mmu_raise_exeption(MMU_ERROR_NONE)\n");
		break;

	//TLB miss
	case MMU_ERROR_TLB_MISS :
		printf("MMU_ERROR_UTLB_MISS 0x%X, handled\n",address);
		if (am==MMU_TT_DWRITE)			//WTLBMISS - Write Data TLB Miss Exception
			sh4_cpu->RaiseExeption(0x60,0x400);
		else if (am==MMU_TT_DREAD)		//RTLBMISS - Read Data TLB Miss Exception
			sh4_cpu->RaiseExeption(0x40,0x400);
		else							//ITLBMISS - Instruction TLB Miss Exception
			sh4_cpu->RaiseExeption(0x40,0x400);

		return;
		break;

	//TLB Multyhit
	case MMU_ERROR_TLB_MHIT :
		printf("MMU_ERROR_TLB_MHIT\n");
		break;

	//Mem is read/write protected (depends on translation type)
	case MMU_ERROR_PROTECTED :
		printf("MMU_ERROR_PROTECTED 0x%X, handled\n",address);
		if (am==MMU_TT_DWRITE)			//WRITEPROT - Write Data TLB Protection Violation Exception
			sh4_cpu->RaiseExeption(0xC0,0x100);
		else if (am==MMU_TT_DREAD)		//READPROT - Data TLB Protection Violation Exception
			sh4_cpu->RaiseExeption(0xA0,0x100);
		return;
		break;

	//Mem is write protected , firstwrite
	case MMU_ERROR_FIRSTWRITE :
		printf("MMU_ERROR_FIRSTWRITE\n");
		break;

	//data read/write missasligned
	case MMU_ERROR_BADADDR :
		printf("MMU_ERROR_BADADDR 0x%X, handled\n",address);
		if (am==MMU_TT_DWRITE)			//WADDERR - Write Data Address Error
			sh4_cpu->RaiseExeption(0x100,0x100);
		else if (am==MMU_TT_DREAD)		//RADDERR - Read Data Address Error
			sh4_cpu->RaiseExeption(0xE0,0x100);
		else							//IADDERR - Instruction Address Error
			sh4_cpu->RaiseExeption(0xE0,0x100);
		return;
		break;

	//Can't Execute
	case MMU_ERROR_EXECPROT :
		printf("MMU_ERROR_EXECPROT 0x%X, handled\n",address);
		//EXECPROT - Instruction TLB Protection Violation Exception
		sh4_cpu->RaiseExeption(0xA0,0x100);
		return;
		break;
	}

	__asm int 3;
}

u32 fast_reg_lut[8]= 
{
	0,0,0,0	//P0-U0
	,1		//P1
	,1		//P2
	,0		//P3
	,1		//P4
};

//Do a full lookup on the UTLB entry's
u32 mmu_full_lookup(u32 va,u32& rv,u32 translation_type)
{
	if ((sr.MD==0) && (va&0x80000000)!=0)
	{
		//if SQ disabled , or if if SQ on but out of SQ mem then BAD ADDR ;)
		if (CCN_MMUCR.SQMD==0 || (va&0xFC000000)!=0xE0000000)
			return MMU_ERROR_BADADDR;
	}

	if ((CCN_MMUCR.AT==0) || (fast_reg_lut[va>>29]!=0))
	{
		rv=va;
		return MMU_ERROR_NONE;
	}
	
	/*if (va==0xC0C110)
	{
		__asm int 3;
		return;
	}*/

	CCN_MMUCR.URC++;
	if (CCN_MMUCR.URB==CCN_MMUCR.URC)
		CCN_MMUCR.URC=0;
	

	u32 entry=0;
	u32 nom=0;


	for (u32 i=0;i<64;i++)
	{
		if (UTLB[i].Data.V==0)
			continue;
		u32 sz=UTLB[i].Data.SZ1*2+UTLB[i].Data.SZ0;
		u32 mask=mmu_mask[sz];

		if ( (((UTLB[i].Address.VPN<<10)&mask)==(va&mask)) )
		{
			bool asid_match = (sr.MD==0) || ( (UTLB[i].Data.SH==0)  && (CCN_MMUCR.SV == 0 ) );

			if ( ( asid_match==false ) || (UTLB[i].Address.ASID==CCN_PTEH.ASID) )
			{
				entry=i;
				nom++;
				//VPN->PPN | low bits
				rv=((UTLB[i].Data.PPN<<10)&mask) | (va&(~mask));
			}
		}
	}

	if (nom!=1)
	{
		if (nom)
		{
			return MMU_ERROR_TLB_MHIT;
		}
		else
		{
			return MMU_ERROR_TLB_MISS;
		}
	}

	u32 md=UTLB[entry].Data.PR>>1;
	
	//Priv mode protection
	if ((md==0) && sr.MD==0)
	{
		if (translation_type==MMU_TT_IREAD)
			return MMU_ERROR_EXECPROT;
		else
			return MMU_ERROR_PROTECTED;
	}

	//Write Protection (Lock or FW)
	if (translation_type==MMU_TT_DWRITE)
	{
		if ((UTLB[entry].Data.PR&1)==0)
			return MMU_ERROR_PROTECTED;
		else if (UTLB[entry].Data.D==0)
			return MMU_ERROR_FIRSTWRITE;
	}
	

	return MMU_ERROR_NONE;
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


u8 __fastcall mmu_ReadMem8(u32 addr)
{
	u32 tv=mmu_full_lookup(addr,addr,MMU_TT_DREAD);
	if (tv==0)
		return _vmem_ReadMem8(addr);
	else
		mmu_raise_exeption(tv,addr,MMU_TT_DREAD);
}
u16 __fastcall mmu_ReadMem16(u32 addr)
{
	if (addr&1)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,addr,MMU_TT_DREAD);
			return 0;
	}
	u32 tv=mmu_full_lookup(addr,addr,MMU_TT_DREAD);
	if (tv==0)
		return _vmem_ReadMem16(addr);
	else
		mmu_raise_exeption(tv,addr,MMU_TT_DREAD);
}
u16 __fastcall mmu_IReadMem16(u32 addr)
{
	if (addr&1)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,addr,MMU_TT_IREAD);
		return 0;
	}
	u32 tv=mmu_full_lookup(addr,addr,MMU_TT_IREAD);
	if (tv==0)
		return _vmem_ReadMem16(addr);
	else
		mmu_raise_exeption(tv,addr,MMU_TT_IREAD);
}

u32 __fastcall mmu_ReadMem32(u32 addr)
{
	if (addr&3)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,addr,MMU_TT_DREAD);
		return 0;
	}
	u32 tv=mmu_full_lookup(addr,addr,MMU_TT_DREAD);
	if (tv==0)
		return _vmem_ReadMem32(addr);
	else
		mmu_raise_exeption(tv,addr,MMU_TT_DREAD);
}

void __fastcall mmu_WriteMem8(u32 addr,u8 data)
{
	u32 tv=mmu_full_lookup(addr,addr,MMU_TT_DWRITE);
	if (tv==0)
	{
		_vmem_WriteMem8(addr,data);
		return;
	}
	else
		mmu_raise_exeption(tv,addr,MMU_TT_DWRITE);
}
void __fastcall mmu_WriteMem16(u32 addr,u16 data)
{
	if (addr&1)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,addr,MMU_TT_DWRITE);
		return;
	}
	u32 tv=mmu_full_lookup(addr,addr,MMU_TT_DWRITE);
	if (tv==0)
	{
		_vmem_WriteMem16(addr,data);
		return;
	}
	else
		mmu_raise_exeption(tv,addr,MMU_TT_DWRITE);
}
void __fastcall mmu_WriteMem32(u32 addr,u32 data)
{
	if (addr&3)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,addr,MMU_TT_DWRITE);
		return;
	}
	u32 tv=mmu_full_lookup(addr,addr,MMU_TT_DWRITE);
	if (tv==0)
	{
		_vmem_WriteMem32(addr,data);
		return;
	}
	else
		mmu_raise_exeption(tv,addr,MMU_TT_DWRITE);
}