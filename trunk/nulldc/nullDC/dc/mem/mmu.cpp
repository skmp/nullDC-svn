#include "mmu.h"
#include "dc/sh4/sh4_if.h"
#include "dc/sh4/ccn.h"
#include "dc/sh4/intc.h"
#include "dc/sh4/sh4_registers.h"
#include "plugins/plugin_manager.h"


#include "_vmem.h"

//SQ fast remap , mailny hackish , assumes 1 mb pages
//max 64 mb can be remapped on SQ
u32 sq_remap[64];

TLB_Entry UTLB[64];
TLB_Entry ITLB[4];
const u32 mmu_mask[4]= 
{
	((0xFFFFFFFF)>>10)<<10,	//1 kb page
	((0xFFFFFFFF)>>12)<<12,	//4 kb page
	((0xFFFFFFFF)>>16)<<16,	//64 kb page
	((0xFFFFFFFF)>>20)<<20	//1 MB page
};

const u32 fast_reg_lut[8]= 
{
	0,0,0,0	//P0-U0
	,1		//P1
	,1		//P2
	,0		//P3
	,1		//P4
};

const u32 ITLB_LRU_OR[4]=
{
	0x00,//000xxx
	0x20,//1xx00x
	0x14,//x1x1x0
	0x0B,//xx1x11
};
const u32 ITLB_LRU_AND[4]=
{
	0x07,//000xxx
	0x39,//1xx00x
	0x3E,//x1x1x0
	0x3F,//xx1x11
};
u32 ITLB_LRU_USE[64];

//sync mem mapping to mmu , suspend compiled blocks if needed.entry is a UTLB entry # , -1 is for full sync
void UTLB_Sync(u32 entry)
{
	
	if ((UTLB[entry].Address.VPN & (0xFC000000>>10)) == (0xE0000000>>10))
	{
		#ifdef NO_MMU
		u32 vpn_sq=((UTLB[entry].Address.VPN & 0x7FFFF)>>10) &0x3F;//upper bits are allways known [0xE0/E1/E2/E3]
		sq_remap[vpn_sq]=UTLB[entry].Data.PPN<<10;
		printf("SQ remap %d : 0x%X to 0x%X\n",entry,UTLB[entry].Address.VPN<<10,UTLB[entry].Data.PPN<<10);
		#endif
	}
	else
		;//printf("MEM remap %d : 0x%X to 0x%X\n",entry,UTLB[entry].Address.VPN<<10,UTLB[entry].Data.PPN<<10);
}
//sync mem mapping to mmu , suspend compiled blocks if needed.entry is a ITLB entry # , -1 is for full sync
void ITLB_Sync(u32 entry)
{
	//printf("ITLB MEM remap %d : 0x%X to 0x%X\n",entry,ITLB[entry].Address.VPN<<10,ITLB[entry].Data.PPN<<10);
}

u32 mmu_error_TT;
void fastcall mmu_raise_exeption(u32 mmu_error,u32 address,u32 am)
{
	//printf("pc = 0x%X : ",pc);
	CCN_TEA=address;
	CCN_PTEH.VPN=address>>10;

	//save translation type error :)
	mmu_error_TT=am;

	switch(mmu_error)
	{
	//No error
	case MMU_ERROR_NONE:
		printf("Error : mmu_raise_exeption(MMU_ERROR_NONE)\n");
		getc(stdin);
		break;

	//TLB miss
	case MMU_ERROR_TLB_MISS :
		//printf("MMU_ERROR_UTLB_MISS 0x%X, handled\n",address);
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
		printf("MMU_ERROR_TLB_MHIT @ 0x%X\n",address);
		break;

	//Mem is read/write protected (depends on translation type)
	case MMU_ERROR_PROTECTED :
		//printf("MMU_ERROR_PROTECTED 0x%X, handled\n",address);
		if (am==MMU_TT_DWRITE)			//WRITEPROT - Write Data TLB Protection Violation Exception
			sh4_cpu->RaiseExeption(0xC0,0x100);
		else if (am==MMU_TT_DREAD)		//READPROT - Data TLB Protection Violation Exception
			sh4_cpu->RaiseExeption(0xA0,0x100);
		return;
		break;

	//Mem is write protected , firstwrite
	case MMU_ERROR_FIRSTWRITE :
		/*
		printf("MMU_ERROR_FIRSTWRITE\n");
		getc(stdin);
		*/
		verify(am==MMU_TT_DWRITE);
		//FIRSTWRITE - Initial Page Write Exception
		sh4_cpu->RaiseExeption(0x80,0x100);
		
		return;
		break;

	//data read/write missasligned
	case MMU_ERROR_BADADDR :
		//printf("MMU_ERROR_BADADDR 0x%X, handled\n",address);
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
		//printf("MMU_ERROR_EXECPROT 0x%X, handled\n",address);
		//EXECPROT - Instruction TLB Protection Violation Exception
		sh4_cpu->RaiseExeption(0xA0,0x100);
		return;
		break;
	}

	__asm int 3;
}


//Do a full lookup on the UTLB entry's
u32 fastcall mmu_full_lookup(u32 va,u32& idx,u32& rv)
{
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
			bool asid_match = (UTLB[i].Data.SH==0) && ( (sr.MD==0) || (CCN_MMUCR.SV == 0) );

			if ( ( asid_match==false ) || (UTLB[i].Address.ASID==CCN_PTEH.ASID) )
			{
				//verify(sz!=0);
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
	
	idx=entry;

	return MMU_ERROR_NONE;
}


u32 fastcall mmu_full_SQ(u32 va,u32& rv)
{
	u32 entry;
	u32 lookup = mmu_full_lookup(va,entry,rv);

	if (lookup!=MMU_ERROR_NONE)
		return lookup;

	u32 md=UTLB[entry].Data.PR>>1;
	
	//Priv mode protection
	if ((md==0) && sr.MD==0)
	{
		return MMU_ERROR_PROTECTED;
	}

	//Write Protection (Lock or FW)
	if ((UTLB[entry].Data.PR&1)==0)
		return MMU_ERROR_PROTECTED;
	else if (UTLB[entry].Data.D==0)
		return MMU_ERROR_FIRSTWRITE;
	
	return MMU_ERROR_NONE;
}


//MMU TLB cache.Internal cache build by the emulator on top of the tlb
//Gets automaticaly sync'd to tlb by the emulator
union mmu_cache_entry
{
	struct
	{
		u32 asid:8;			//asid id for the page
		u32 user:1;			//set to 1 if user mode cannot access
		u32 write:1;		//set to 1 if page cannot be writen
		u32 PPN:19;			//Physical page number :}
		u32 nil0:1;			//set to 1 if kernel cannot read
		u32 nil1:1;			//set to 1 if kernel cannot write
		u32 asid_on:1;		//set to 1 if asid matching is enabled for that entry
	};
	u32 full;
};

#define MMU_ICC_UA (1<<8)	//User access
#define MMU_ICC_WA (1<<9)	//Write access
#define MMU_ICC_AE (1<<31)

mmu_cache_entry mmu_cache[4*1024*1024];

//Data is assumed to be aligned !
template<bool read,bool priv,bool sqmd,bool match_asids>
u32 fastcall mmu_fast_translation_fill(u32 va,u32& rv)
{
	if ((priv==0) && (va&0x80000000)!=0)
	{
		//if SQ disabled , or if if SQ on but out of SQ mem then BAD ADDR ;)
		if ( ((va&0xFC000000)!=0xE0000000) ||  (sqmd==1) )
			return MMU_ERROR_BADADDR;
	}

	if (fast_reg_lut[va>>29]!=0)
	{
		rv=va;
		return MMU_ERROR_NONE;
	}

	u32 entry;
	u32 lookup=mmu_full_lookup(va,entry,rv);
	
	if (lookup!=MMU_ERROR_NONE)
		return lookup;

	u32 md=UTLB[entry].Data.PR>>1;
	
	//0X  & User mode-> protection violation
	//Priv mode protection
	if ((md==0) && sr.MD==0)
	{
		return MMU_ERROR_PROTECTED;
	}

	//X0 -> read olny
	//X1 -> read/write , can be FW

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
template<bool read,bool priv,bool sqmd,bool AT,bool match_asids>
u32 fastcall mmu_fast_translation(u32 va,u32& rv)
{
	if (!AT)
	{
		rv=va;
		return MMU_ERROR_NONE;
	}
	else
	{
		u32 page=va>>10;
		u32 offs=va&0x3FF;
		mmu_cache_entry dst=mmu_cache[page];

		u32 access_mask=0;

		if (!read)
			access_mask|=MMU_ICC_WA;
		if (!priv)
			access_mask|=MMU_ICC_UA;

		if (!(dst.full&access_mask))
		{
			if (match_asids && !priv)
			{
				if (dst.asid==CCN_PTEH.ASID)
				{
					rv=(dst & 0x1FFFFC00)|offs;
					return MMU_ERROR_NONE;
				}
				else
				{
					return mmu_fast_translation_fill<read,priv,sqmd,match_asids>(va,rv);
				}
			}
			else
			{
				rv=(dst & 0x1FFFFC00)|offs;
				return MMU_ERROR_NONE;
			}
		}
		else
		{
			//uhoh
			//need to perform full lookup :p
			return mmu_fast_translation_fill<read,priv,sqmd,match_asids>(va,rv);
		}
	}

}
template<u32 translation_type>
u32 fastcall mmu_data_translation(u32 va,u32& rv)
{
	if ((sr.MD==0) && (va&0x80000000)!=0)
	{
		//if SQ disabled , or if if SQ on but out of SQ mem then BAD ADDR ;)
		if ( ((va&0xFC000000)!=0xE0000000) ||  (CCN_MMUCR.SQMD==1) )
			return MMU_ERROR_BADADDR;
	}

	if ((CCN_MMUCR.AT==0) || (fast_reg_lut[va>>29]!=0))
	{
		rv=va;
		return MMU_ERROR_NONE;
	}
	if ( CCN_CCR.ORA && ((va&0xFC000000)==0x7C000000))
	{
		return va;
	}
	u32 entry;
	u32 lookup=mmu_full_lookup(va,entry,rv);
	
	if (lookup!=MMU_ERROR_NONE)
		return lookup;

	u32 md=UTLB[entry].Data.PR>>1;
	
	//0X  & User mode-> protection violation
	//Priv mode protection
	if ((md==0) && sr.MD==0)
	{
		return MMU_ERROR_PROTECTED;
	}

	//X0 -> read olny
	//X1 -> read/write , can be FW

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

u32 fastcall mmu_instruction_translation(u32 va,u32& rv)
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

	bool mmach=false;
retry_ITLB_Match:
	u32 entry=4;
	u32 nom=0;
	for (u32 i=0;i<4;i++)
	{
		if (ITLB[i].Data.V==0)
			continue;
		u32 sz=ITLB[i].Data.SZ1*2+ITLB[i].Data.SZ0;
		u32 mask=mmu_mask[sz];

		if ( (((ITLB[i].Address.VPN<<10)&mask)==(va&mask)) )
		{
			bool asid_match = (ITLB[i].Data.SH==0) && ( (sr.MD==0) || (CCN_MMUCR.SV == 0) );

			if ( ( asid_match==false ) || (ITLB[i].Address.ASID==CCN_PTEH.ASID) )
			{
				//verify(sz!=0);
				entry=i;
				nom++;
				//VPN->PPN | low bits
				rv=((ITLB[i].Data.PPN<<10)&mask) | (va&(~mask));
			}
		}
	}

	if (entry==4)
	{
		verify(mmach==false);
		u32 lookup=mmu_full_lookup(va,entry,rv);

		if (lookup!=MMU_ERROR_NONE)
			return lookup;
		u32 replace_index=ITLB_LRU_USE[CCN_MMUCR.LRUI];
		verify(replace_index!=0xFFFFFFFF);
		ITLB[replace_index]=UTLB[entry];
		entry=replace_index;
		ITLB_Sync(entry);
		mmach=true;
		goto retry_ITLB_Match;
	}
	else if (nom!=1)
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

	CCN_MMUCR.LRUI&=ITLB_LRU_AND[entry];
	CCN_MMUCR.LRUI|=ITLB_LRU_OR[entry];

	u32 md=ITLB[entry].Data.PR>>1;
	
	//0X  & User mode-> protection violation
	//Priv mode protection
	if ((md==0) && sr.MD==0)
	{
		return MMU_ERROR_PROTECTED;
	}

	return MMU_ERROR_NONE;
}
void MMU_Init()
{
	memset(ITLB_LRU_USE,0xFF,sizeof(ITLB_LRU_USE));
	for (u32 e=0;e<4;e++)
	{
		u32 match_key=((~ITLB_LRU_AND[e])&0x3F);
		u32 match_mask=match_key | ITLB_LRU_OR[e];
		for (u32 i=0;i<64;i++)
		{
			if ((i & match_mask)==match_key)
			{
				verify(ITLB_LRU_USE[i]==0xFFFFFFFF);
				ITLB_LRU_USE[i]=e;
			}
		}
	}
}


void MMU_Reset(bool Manual)
{
	memset(UTLB,0,sizeof(UTLB));
	memset(ITLB,0,sizeof(ITLB));
}

void MMU_Term()
{
}


u8 __fastcall mmu_ReadMem8(u32 adr)
{
	u32 addr;
	u32 tv=mmu_data_translation<MMU_TT_DREAD>(adr,addr);
	if (tv==0)
		return _vmem_ReadMem8(addr);
	else
		mmu_raise_exeption(tv,adr,MMU_TT_DREAD);
}
u16 __fastcall mmu_ReadMem16(u32 adr)
{
	if (adr&1)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,adr,MMU_TT_DREAD);
			return 0;
	}
	u32 addr;
	u32 tv=mmu_data_translation<MMU_TT_DREAD>(adr,addr);
	if (tv==0)
		return _vmem_ReadMem16(addr);
	else
		mmu_raise_exeption(tv,adr,MMU_TT_DREAD);
}
u16 __fastcall mmu_IReadMem16(u32 adr)
{
	if (adr&1)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,adr,MMU_TT_IREAD);
		return 0;
	}
	u32 addr;
	u32 tv=mmu_instruction_translation(adr,addr);
	if (tv==0)
		return _vmem_ReadMem16(addr);
	else
		mmu_raise_exeption(tv,adr,MMU_TT_IREAD);
}

u32 __fastcall mmu_ReadMem32(u32 adr)
{
	if (adr&3)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,adr,MMU_TT_DREAD);
		return 0;
	}
	u32 addr;
	u32 tv=mmu_data_translation<MMU_TT_DREAD>(adr,addr);
	if (tv==0)
		return _vmem_ReadMem32(addr);
	else
		mmu_raise_exeption(tv,adr,MMU_TT_DREAD);
}
void __fastcall mmu_ReadMem64(u32 adr,u32* reg)
{
	if (adr&7)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,adr,MMU_TT_DREAD);
		return;
	}
	u32 addr;
	u32 tv=mmu_data_translation<MMU_TT_DREAD>(adr,addr);
	if (tv==0)
	{
		reg[0]=_vmem_ReadMem32(addr);
		reg[1]=_vmem_ReadMem32(addr+4);
	}
	else
		mmu_raise_exeption(tv,adr,MMU_TT_DREAD);
}

void __fastcall mmu_WriteMem8(u32 adr,u8 data)
{
	u32 addr;
	u32 tv=mmu_data_translation<MMU_TT_DWRITE>(adr,addr);
	if (tv==0)
	{
		_vmem_WriteMem8(addr,data);
		return;
	}
	else
		mmu_raise_exeption(tv,adr,MMU_TT_DWRITE);
}
void __fastcall mmu_WriteMem16(u32 adr,u16 data)
{
	if (adr&1)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,adr,MMU_TT_DWRITE);
		return;
	}
	u32 addr;
	u32 tv=mmu_data_translation<MMU_TT_DWRITE>(adr,addr);
	if (tv==0)
	{
		_vmem_WriteMem16(addr,data);
		return;
	}
	else
		mmu_raise_exeption(tv,adr,MMU_TT_DWRITE);
}
void __fastcall mmu_WriteMem32(u32 adr,u32 data)
{
	if (adr&3)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,adr,MMU_TT_DWRITE);
		return;
	}
	u32 addr;
	u32 tv=mmu_data_translation<MMU_TT_DWRITE>(adr,addr);
	if (tv==0)
	{
		_vmem_WriteMem32(addr,data);
		return;
	}
	else
		mmu_raise_exeption(tv,adr,MMU_TT_DWRITE);
}
void __fastcall mmu_WriteMem64(u32 adr,u32* data)
{
	if (adr&7)
	{
		mmu_raise_exeption(MMU_ERROR_BADADDR,adr,MMU_TT_DWRITE);
		return;
	}
	u32 addr;
	u32 tv=mmu_data_translation<MMU_TT_DWRITE>(adr,addr);
	if (tv==0)
	{
		_vmem_WriteMem32(addr,data[0]);
		_vmem_WriteMem32(addr+4,data[1]);
		return;
	}
	else
		mmu_raise_exeption(tv,adr,MMU_TT_DWRITE);
}

bool __fastcall mmu_TranslateSQW(u32& adr)
{
#ifndef NO_MMU
	u32 addr;
	u32 tv=mmu_full_SQ(adr,addr);
	if (tv!=0)
	{
		mmu_raise_exeption(tv,adr,MMU_TT_DWRITE);
		return true;
	}

	adr=addr;
#else
	//This will olny work for 1 mb pages .. hopefully nothing else is used
	//*FIXME* to work for all page sizes !!
	adr=sq_remap[(adr>>20)&0x3F] | (adr & 0xFFFFF);
#endif
	return false;
}